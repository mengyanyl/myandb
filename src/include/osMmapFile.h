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

#ifndef OSMMAPFILE_H
#define OSMMAPFILE_H

#include "core.h"
#include "osPrimitiveFileOp.h"

using namespace std;
using namespace boost;

class _osMmapFile
{
    protected:
        class _osMmapSegment
        {
        public:
            void*               _ptr;
            unsigned int        _length;
            unsigned long  long _offset;
            _osMmapSegment(void *ptr, unsigned int length, unsigned long long offset)
            {
                _ptr = ptr;
                _length = length;
                _offset = offset;
            }
        };
        typedef _osMmapSegment osMmapSegment;
        osPrimitiveFileOp   _fileOp;
        boost::mutex        _mutex;
        bool                _opened;
        vector<osMmapSegment>  _segments;
        char                _fileName[OSS_MAX_PATHSIZE];
    public:
        _osMmapFile()
        {
            _opened = false ;
            memset ( _fileName, 0, sizeof(_fileName) ) ;
        }
        virtual ~_osMmapFile()
        {
            close();
        }
        int open(const char* pFileName, unsigned int options);
        int close();
        int map(unsigned long long offset, unsigned int length, void **pAdress);
    public:
        typedef vector<osMmapSegment>::const_iterator CONST_ITR;

        inline CONST_ITR begin()
        {
            return _segments.begin();
        }

        inline CONST_ITR end()
        {
            return _segments.end();
        }

        inline unsigned int size()
        {
            return _segments.size();
        }

};
typedef class _osMmapFile osMmapFile;

#endif // OSMMAPFILE_H
