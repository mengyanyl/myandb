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

#include "osMmapFile.h"
#include "logger.h"

int osMmapFile::open(const char *pFileName, unsigned int options)
{
    _mutex.lock();

    int rc = _fileOp->Open(pFileName);
    if (rc==EDB_OK)
        _opened = true;
    else
        Logger::getLogger().error("open file error");
    strncpy(_fileName, pFileName, strlen(pFileName));

    _mutex.unlock();
    return rc;
}

int osMmapFile::close()
{
    _fileOp->close();
}

int osMmapFile::map(unsigned long long offset, unsigned int length, void **pAddress)
{
    boost::mutex::scope_lock(_mutex);
    int rc = EDB_OK;
    osMmapSegment seg (0,0,0);
    unsigned long long fileSize = 0;
    void *segment = NULL;
    if (0 == length)
    {
        return rc;
    }
    rc = _fileOp.getSize( (off_t*)&fileSize );
    if (rc)
    {
        Logger::getLogger().error("file get size error");
        return rc;
    }

    if (offset + length > fileSize)
    {

        Logger::getLogger.error("offset if greater than file size");
        return EDB_INVALIDARG;
    }

    segment = mmap(NULL, length, PROT_READ | PROT_WRITE,
                    MAP_SHARED, _fileOp.getHandle(), offset);
    if (MAP_FAILED == segment)
    {
        Logger::getLogger.error("failed to map offset %ld length %d, error no = %d"
                                , offset, length, errno);
        if (ENOMEM == errno)
            rc = EDB_OOM;
        else if (EACCES = errno)
            rc = EDB_PERM;
        else
            rc = EDB_SYS;
    }
    seg._ptr = segment;
    seg._length = length;
    seg._offset = offset;
    segments.push_back(seg);
}
