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

#ifndef _PMDAGNET_H_
#define _PMDAGENT_H_

#include "core.h"
#include "msg.h"
#include "osSocket.h"

class pmdAgent
{
private:
    osSocket _socket;
    int recvHeader(osSocket &sock, MsgHeader &msgHeader);
    int sendHeader(osSocket &sock, int seq, int len , int opCode);
    int recvBodyAndSend(osSocket &sock, MsgHeader &header);

    inline void pmdSend(char *buffer, int len)
    {
        while(true)
        {
            int rc = _socket.send(buffer, len);
            if (rc == EDB_TIMEOUT) continue;
            break;
        }
    }

    inline void pmdRecv(char *buffer, int len)
    {
        while(true)
        {
            int rc = _socket.recv(buffer, len);
            if (rc == EDB_TIMEOUT) continue;
            break;
        }
    }

public:
    int pmdAgentEntryPoint ( SOCKET &clientSocket ) ;
};


#endif // _PMDEDU_H_
