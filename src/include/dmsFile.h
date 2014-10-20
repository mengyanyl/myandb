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

#ifndef DMSFILE_H
#define DMSFILE_H

#include "core.h"
#include "osMmapFile.h"
#include "dmsRecordID.h"
#include "bson.h"

//4M for one page
#define DMS_PAGESIZE 4194304
//2G for one db file
#define DMS_MAX_PAGES 262144
#define DMS_MAX_RECORD (DMS_PAGESIZE - sizeof(dmsHeader)-sizeof(dmsRecord)-sizeof(SLOTOFF))
typedef unsigned int SLOTOFF;
#define DMS_INVALID_SLOTID      0xFFFFFFFF
#define DMS_INVALID_PAGEID      0xFFFFFFFF
#define DMS_KEY_FIELDNAME       "_id"

#define DMS_RECORD_FLAG_NORMAL      0
#define DMS_RECORD_FLAG_DROPPED     1

struct dmsRecord
{
    unsigned int _size;
    unsigned int _flag;
    char         _data[0];
};

//dms header
#define DMS_HEADER_EYECATCHER "DMSH"
#define DMS_HEADER_EYECATCHER_LEN 4
#define DMS_HEADER_FLAG_NORMAL      0
#define DMS_HEADER_FLAG_DROPPED     1

#define DMS_HEADER_VERSION_0      0
#define DMS_HEADER_VERSION_CURRENT DMS_HEADER_VERSION_0

struct dmsHeader
{
    char            _eyeCatcher[DMS_HEADER_EYECATCHER_LEN];
    unsigned int    _size;
    unsigned int    _flag;
    unsigned int    _version;
};

//page structure
#define DMS_PAGE_EYECATCHER "PAGH"
#define DMS_PAGE_EYECATCHER_LEN 4
#define DMS_PAGE_FLAG_NORMAL    0
#define DMS_PAGE_FLAG_DROPPED   1
#define DMS_SLOT_EMPTY          0xFFFFFFFF

struct dmsPageHeader
{
    char            _eyeCatcher[DMS_PAGE_EYECATCHER_LEN];
    unsigned int    _size;
    unsigned int    _flag;
    unsigned int    _numSlots;
    unsigned int    _slotOffset;
    unsigned int    _freeSpace;
    unsigned int    _freeOffset;
    char            _data[0];
};

#define DMS_FILE_SEGMENT_SIZE   134217728
#define DMS_FILE_HEADER_SIZE    65536
#define DMS_EXTEND_SIZE         DMS_FILE_HEADER_SIZE
#define DMS_PAGES_PER_SEGMENT   (DMS_FILE_SEGMENT_SIZE/DMS_PAGESIZE)
#define DMS_MAX_SEGMENTS        (DMS_MAX_PAGES/DMS_PAGES_PER_SEGMENT)

extern const char *gKeyFieldName;

class dmsFile : public osMmapFile
{
private:
    dmsHeader                   *_header;
    std::vector<char *>         _body;
    std::multimap<unsigned int , PAGEID>    _freeSpaceMap;
    boost::mutex                            _mutex;
    boost::mutex                            _extendMutex;
    char *                      _pFileName;
    //TODO INDEX MANAGEER
    public:
        dmsFile();
        virtual ~dmsFile();
        dmsFile& operator=(const dmsFile& df)
        {
            return *this;
        }
        int initialize(const char *pFileName);
        int insert(bson::BSONObj &record, bson::BSONObj &ourtRecord,
                   dmsRecordID &rid);
        int remove(dmsRecordID &rid);
        int find(dmsRecordID &rid, bson::BSONObj &result);
    protected:
    private:
        int _extnedSegment();
        int _initNew();
        int _extendFile(int size);
        int _extendSegment();
        int _loadData();
        int _searchSlot(char *page, dmsRecordID &rid, SLOTOFF &slot);
        void _recoverSpace(char *page);
        void _updateFreeSpace(dmsPageHeader *header, int changeSize, PAGEID pageID);
        PAGEID _findPage(size_t requireSize);
    public:
        inline unsigned int getNumSegments()
        {
            return _body.size();
        }
        inline unsigned int getNumPages()
        {
            return getNumSegments() * DMS_PAGES_PER_SEGMENT;
        }
        inline char *pageToOffset(PAGEID pageID)
        {
            if (pageID > getNumPages())
            {
                return NULL;
            }
            return _body[pageID/DMS_PAGES_PER_SEGMENT] + DMS_PAGESIZE * (pageID % DMS_PAGES_PER_SEGMENT);
        }
        inline bool validSize(size_t size)
        {

        }
};

typedef dmsFile* DMSFILE_PTR;

#endif // DMSFILE_H
