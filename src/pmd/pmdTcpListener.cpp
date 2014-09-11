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

#include "pmdTcpListener.h"
#include "threadPool.h"
#include "pmdAgent.h"

pmdTcpListener::pmdTcpListener()
{
    _svrsock.setAddress("127.0.0.1", 9999);
    _svrsock.initSocket();
    _svrsock.bind_listen();
}

pmdTcpListener::~pmdTcpListener()
{
    _svrsock.close();
}

void pmdTcpListener::run()
{
    for(;;)
    {
        SOCKET clientSock;
        socklen_t len;

        int re = _svrsock.accept(&clientSock, NULL, NULL);
        if ( EDB_TIMEOUT == re )
        {
            re = EDB_OK ;
            continue ;
        }

        Runnable *pagent = new pmdAgent(&clientSock);
        ThreadPool::getThreadPool().start(pagent);
    }
}
