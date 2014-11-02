/*******************************************************************************
   Copyright (C) 2014 MyanDB Software Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.
*******************************************************************************/

#include "dmsFile.h"
#include "logger.h"
#include "osPrimitiveFileOp.h"

using namespace bson;
using namespace myan::utils;

dmsFile::dmsFile():_header(NULL), _pFileName(NULL)
{
    //ctor
}

dmsFile::~dmsFile()
{
    if (_pFileName)
        free(_pFileName);
    close();
}

int dmsFile::initialize(const char *pFileName)
{
    offsetType offset = 0;
    int rc = EDB_OK;
    this->_pFileName = strdup(pFileName);
    if (!_pFileName)
    {
        Logger::getLogger().error("duplicate file name error");
        return EDB_OOM;
    }

    rc = open(_pFileName, OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS);
    if (rc)
    {
        Logger::getLogger().error("failed to open file %s, rc=%d",
                                  _pFileName, rc);
        return rc;
    }

getfilesize:
    rc = _fileOp.getSize(&offset);

    //it's new file
    if (!offset)
    {
        _initNew();
        goto getfilesize;
    }
    _loadData();

    return rc;
}

int dmsFile::_initNew()
{
    int rc = EDB_OK;
    rc = _extendFile(DMS_FILE_HEADER_SIZE);
    if (rc!=EDB_OK)
    {
        Logger::getLogger().error("failed to extend file, rc=%d", rc);
        return rc;
    }
    rc = map(0, DMS_FILE_HEADER_SIZE, (void**)&_header);

    strcpy(_header->_eyeCatcher, DMS_HEADER_EYECATCHER);
    _header->_size = 0;
    _header->_flag = DMS_HEADER_FLAG_NORMAL;
    _header->_version = DMS_HEADER_VERSION_CURRENT;

    return rc;
}

int dmsFile::_extendFile(int size)
{
    int rc = EDB_OK;
    char temp[DMS_EXTEND_SIZE] = {0};
    memset(temp, 0, DMS_EXTEND_SIZE);
    if (size % DMS_EXTEND_SIZE != 0)
    {
        rc = EDB_SYS;
        Logger::getLogger().error("invalid extend size, must be multiple of %d", DMS_EXTEND_SIZE);
        return rc;
    }
    for (int i=0; i<size; i+=DMS_EXTEND_SIZE)
    {
        _fileOp.seekToEnd();
        rc  = _fileOp.Write(temp, DMS_EXTEND_SIZE);
        if (rc)
            Logger::getLogger().error("failed to write to file, rc=%d", rc);
    }
    return rc;
}

int dmsFile::_loadData()
{
    int rc = EDB_OK;
    unsigned int numPages = 0;
    unsigned int numSegments = 0;
    char *data = NULL;
    dmsPageHeader *pageHeader = NULL;
    unsigned int slotOffset = 0;
    BSONObj bson;
    SLOTID slotID = 0;

    if (!_header)
    {
        rc = map(0, DMS_FILE_HEADER_SIZE, (void**)&_header);
    }
    numPages = _header->_size;
    if (numPages % DMS_PAGES_PER_SEGMENT)
    {
        rc = EDB_SYS;
        Logger::getLogger().error("failed to load data, header->size error");
        return rc;
    }

    numSegments = numPages / DMS_PAGES_PER_SEGMENT;
//    if (numSegments < 1)
//    {
//        rc = EDB_SYS;
//        Logger::getLogger().error("failed to load data, num of segments is error, numSegments=%d", numSegments);
//        return rc;
//    }

    for (int i=0; i<numSegments; ++i)
    {
        //put every segments into memory
        rc = map(DMS_FILE_HEADER_SIZE + i * DMS_FILE_SEGMENT_SIZE,
                    DMS_FILE_SEGMENT_SIZE, (void **)&data);
        if (rc != EDB_OK)
        {
            Logger::getLogger().error("map segments error");
        }
        _body.push_back(data);
        for (unsigned int k=0; k<DMS_PAGES_PER_SEGMENT; k++)
        {
            pageHeader = (dmsPageHeader *) (data + k * DMS_PAGESIZE);
            _freeSpaceMap.insert(pair<unsigned int , PAGEID>(pageHeader->_freeSpace, k));
            slotID = (SLOTID)pageHeader->_numSlots;
            //map every record into memory
            for (unsigned int j=0; j<slotID; j++)
            {
                slotOffset = *(SLOTOFF *)(data + k * DMS_PAGESIZE + sizeof(dmsPageHeader)
                    + j*sizeof(SLOTID));
                if (DMS_SLOT_EMPTY == slotOffset) continue;
                bson = BSONObj(data + k * DMS_PAGESIZE + slotOffset + sizeof(dmsRecord));
                Logger::getLogger().info("load bson string: %s", bson.toString().c_str());
            }
        }
    }

    return rc;
}

int dmsFile::find(dmsRecordID &rid, BSONObj &result)
{
    int rc = EDB_OK;
    SLOTOFF slot = 0;
    char *page = NULL;
    dmsRecord *recordHeader = NULL;

    boost::mutex::scoped_lock lock(_mutex);

    page = pageToOffset(rid._pageID);
    if (!page)
    {
        Logger::getLogger().error("[find] get page is null");
        return EDB_SYS;
    }

    rc = _searchSlot(page, rid, slot);
    if (rc)
    {
        Logger::getLogger().error("failed to search slot, rc=%d", rc);
        return rc;
    }

    recordHeader = (dmsRecord *)(page + slot);
    if (recordHeader->_flag == DMS_RECORD_FLAG_DROPPED)
    {
        Logger::getLogger().error("this record is already deleted");
        return EDB_SYS;
    }

    result = BSONObj(page + slot + sizeof(dmsRecord)).copy();
    //TODO

    return rc;
}

int dmsFile::_searchSlot(char *page, dmsRecordID &rid, SLOTOFF &slotOff)
{
    if (page==NULL)
    {
        Logger::getLogger().error("[searchSlot] page is null");
        return EDB_SYS;
    }
    if (rid._pageID<0 || rid._slotID<0)
    {
        Logger::getLogger().error("dmsRecord is error, pageid or slotid < 0");
        return EDB_SYS;
    }
    dmsPageHeader *pageHeader = (dmsPageHeader*)page;
    if ( (pageHeader->_numSlots) < (rid._slotID) )
    {
        Logger::getLogger().error("slot is out of range");
        return EDB_SYS;
    }
    slotOff = *(SLOTOFF *)(page + sizeof(dmsPageHeader) + rid._slotID * sizeof(SLOTOFF));

    return EDB_OK;
}

int dmsFile::remove(dmsRecordID &rid)
{
    SLOTOFF slot = 0;
    int rc = EDB_OK;
    char *page = NULL;
    dmsRecord *recordHeader = NULL;
    dmsPageHeader *pageHeader = NULL;

    boost::mutex::scoped_lock lock(this->_mutex);

    page = pageToOffset(rid._pageID);
    if (!page)
    {
        Logger::getLogger().error("[remove] get page is null");
        return EDB_SYS;
    }
    pageHeader = (dmsPageHeader *)page;

    rc = _searchSlot(page, rid, slot);
    if (rc)
    {
        Logger::getLogger().error("[remove] failed to search slot, rc=%d", rc);
        return rc;
    }
    if (slot == DMS_SLOT_EMPTY)
    {
        Logger::getLogger().error("[remove] the record is dropped");
        return EDB_SYS;
    }

    *((SLOTOFF *)(page + sizeof(dmsPageHeader) + rid._slotID * sizeof(SLOTOFF))) = DMS_SLOT_EMPTY;
    recordHeader = (dmsRecord *)(page + slot);
    recordHeader->_flag = DMS_RECORD_FLAG_DROPPED;
    _updateFreeSpace(pageHeader, recordHeader->_size , rid._pageID);
}

int dmsFile::insert(BSONObj &record, BSONObj &outRecord, dmsRecordID &rid)
{
    int rc = EDB_OK;
    int recordSize = 0;
    PAGEID pageID;
    dmsPageHeader *pageHeader;
    dmsRecord recordHeader;
    SLOTOFF offsetTemp = 0;
    char *page = NULL;

    recordSize = record.objsize();

    if (recordSize > DMS_PAGESIZE)
    {
        Logger::getLogger().error("record cannot bigger than 4MB");
        return EDB_SYS;
    }

retry:
    _mutex.lock();
    pageID = _findPage(recordSize);
    if (pageID == DMS_INVALID_PAGEID)
    {
        _mutex.unlock();
        if (_extendMutex.try_lock())
        {
            rc = _extendSegment();
            if (rc)
            {
                Logger::getLogger().error("failed to extend segment");
                return EDB_SYS;
            }
            _extendMutex.unlock();
        }
        else
        {
            //maybe someone is extending sgemenmt, do it again
            _extendMutex.lock();
        }
        _extendMutex.unlock();
        goto retry;
    }
    page = pageToOffset(pageID);
    if (!page)
    {
        Logger::getLogger().error("failed to find page");
        return EDB_SYS;
    }
    pageHeader = (dmsPageHeader *)page;
    if (
        (pageHeader->_freeSpace >
            (pageHeader->_freeOffset - pageHeader->_slotOffset))
        &&
        ((pageHeader->_slotOffset + recordSize + sizeof(dmsRecord) + sizeof(SLOTID)) >
            pageHeader->_freeOffset)
        )
    {
        _recoverSpace(page);
    }

    if (
        (pageHeader->_freeSpace < (recordSize + sizeof(dmsRecord) + sizeof(SLOTID)))
        ||
        ((pageHeader->_freeOffset - pageHeader->_slotOffset)
         <
         (recordSize + sizeof(dmsRecord) + sizeof(SLOTID)))
        )
    {
        Logger::getLogger().error("page is not enough size");
        return EDB_SYS;
    }

    recordHeader._size = recordSize + sizeof(dmsRecord);
    recordHeader._flag = DMS_RECORD_FLAG_NORMAL;
    offsetTemp = pageHeader->_freeOffset - recordSize - sizeof(dmsRecord);
    *(SLOTOFF *)(page + sizeof(dmsPageHeader) + pageHeader->_numSlots * sizeof(SLOTOFF)) = offsetTemp;
    //copy record header
    memcpy(page + offsetTemp, ( char*)&recordHeader, sizeof(dmsRecord));
    //copy record body
    memcpy(page + offsetTemp + sizeof(dmsRecord),
            record.objdata(), recordSize);
    outRecord = BSONObj(page + offsetTemp + sizeof(dmsRecord));
    rid._pageID = pageID;
    rid._slotID = pageHeader->_numSlots;
    //change pageheader info
    pageHeader->_numSlots++;
    pageHeader->_slotOffset += sizeof(SLOTID);
    pageHeader->_freeOffset = offsetTemp;

    _updateFreeSpace(pageHeader, -(recordSize + sizeof(SLOTID) +sizeof(dmsRecord)), pageID);

    _mutex.unlock();

    return rc;
}

PAGEID dmsFile::_findPage(size_t requireSize)
{
    std::multimap<unsigned int , PAGEID>::iterator iter = _freeSpaceMap.upper_bound(requireSize);
    if (iter!=_freeSpaceMap.end())
    {
        return iter->second;
    }
    return DMS_INVALID_PAGEID;
}

int dmsFile::_extendSegment()
{
    int rc = EDB_OK;
    offsetType offset = 0;
    char *data = NULL;
    dmsPageHeader pageHeader;
    int freeMapSize = 0;

    //get file ended cursor
    rc = _fileOp.getSize(&offset);
    if (rc)
    {
        Logger::getLogger().error("[extendSegment] failed to get file size, rc=%d", rc);
        return rc;
    }

    //extend file
    rc = _extendFile(DMS_FILE_SEGMENT_SIZE);
    if (rc)
    {
        Logger::getLogger().error("[extendSegment] failed to extend file rc=%d", rc);
        return rc;
    }

    //map new segment
    rc = map(offset, DMS_FILE_SEGMENT_SIZE, (void **)&data);
    if (rc)
    {
        Logger::getLogger().error("[extendSegment] failed to map new segment");
        return rc;
    }
    strcpy(pageHeader._eyeCatcher, DMS_PAGE_EYECATCHER);
    pageHeader._size = DMS_PAGESIZE;
    pageHeader._flag = DMS_PAGE_FLAG_NORMAL;
    pageHeader._numSlots = 0;
    pageHeader._freeSpace = DMS_PAGESIZE - sizeof(dmsPageHeader);
    pageHeader._freeOffset = DMS_PAGESIZE;
    pageHeader._slotOffset = sizeof(dmsPageHeader) + pageHeader._numSlots * sizeof(SLOTOFF);

    for (int i=0; i<DMS_PAGES_PER_SEGMENT; i+=DMS_PAGESIZE)
    {
        memcpy(data + i, (char *)(&pageHeader), sizeof(pageHeader));
    }

    this->_mutex.lock();
    freeMapSize = _freeSpaceMap.size();
    for (int i=0; i<DMS_PAGES_PER_SEGMENT; i++)
    {
        _freeSpaceMap.insert(pair<unsigned int , PAGEID>(pageHeader._freeSpace, i+freeMapSize));
    }
    _body.push_back(data);
    _header->_size += DMS_PAGES_PER_SEGMENT;
    this->_mutex.unlock();

    return rc;
}

void dmsFile::_updateFreeSpace(dmsPageHeader *pageHeader, int changeSize, PAGEID pageID)
{
    unsigned int freeSpace = pageHeader->_freeSpace ;
    std::pair<std::multimap<unsigned int, PAGEID>::iterator,
             std::multimap<unsigned int, PAGEID>::iterator> ret ;
    ret = _freeSpaceMap.equal_range(freeSpace);
    for (std::multimap<unsigned int , PAGEID>::iterator it = ret.first; it != ret.second; ++it)
    {
        if (it->second == pageID)
        {
            _freeSpaceMap.erase(pageID);
            break;
        }
    }
    freeSpace += changeSize;
    pageHeader->_freeSpace = freeSpace;
    //insert new freespace
    _freeSpaceMap.insert(std::pair<unsigned int , PAGEID>(freeSpace, pageID));
}

void dmsFile::_recoverSpace(char *page)
{
    dmsPageHeader *pageHeader = NULL;
    SLOTOFF slot = 0;
    char *pLeft = NULL;
    char *pRight = NULL;
    bool isRecoverd = false;
    dmsRecord *pRecord;

    pageHeader = (dmsPageHeader *)page;
    pLeft = page + sizeof(dmsPageHeader);
    pRight = page + DMS_PAGESIZE;

    for (int i=0; i<pageHeader->_numSlots; i++)
    {
        slot = *((SLOTOFF *)(pLeft + i * sizeof(SLOTID)));
        if (slot == DMS_SLOT_EMPTY)
        {
            pRecord = (dmsRecord *)(page + slot);
            pRight -= pRecord->_size;
            if (isRecoverd)
            {
                memmove(pRight, page + slot, pRecord->_size);
                *((SLOTOFF *)(pLeft + i * sizeof(SLOTID))) = pRight - page;
            }
        }
        else
        {
            isRecoverd = true;
        }
    }
    pageHeader->_freeOffset = pRight - page;
}

