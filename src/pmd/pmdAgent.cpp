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

#include "pmdAgent.h"
#include "logger.h"
#include <bson/src/util/json.h>
#include "threadPool.h"

using namespace bson;
using namespace myan::utils;

pmdAgent::pmdAgent(SOCKET *pSocket, DMSFILE_PTR &aDmsFilePtr)
{
    this->_dmsFilePtr = aDmsFilePtr;
    this->_socket.setSocket(pSocket);
}

pmdAgent::~pmdAgent()
{
    if (_socket.isConnected())
        this->_socket.close();
}

int pmdAgent::recvHeader(osSocket &sock, MsgHeader &header)
{
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));

    pmdRecv(buffer, sizeof(MsgHeader));

    msgExtractHeader(buffer, header.sequence, header.messageLen, header.opCode);

    if (header.opCode>0)
        Logger::getLogger().debug("[client-->server header]: seqno: %d, bodylen: %d, opcode: %d",
                                  header.sequence,
                                  header.messageLen,
                                  header.opCode);
    return EDB_OK;
}

int pmdAgent::sendHeader(osSocket &sock, int seq, int len , int opCode)
{
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    char *pbuffer = buffer;
    msgBuildHeader(&pbuffer, seq, len, opCode);
    pmdSend( buffer, sizeof(MsgHeader) );
}

int pmdAgent::recvBodyAndSend(osSocket &sock, MsgHeader &header)
{
    char *pBuffer = NULL;
    pBuffer = (char *)malloc( sizeof(char) * header.messageLen);
    memset(pBuffer, 0, header.messageLen);
    char *pResultBuffer = NULL;
    pResultBuffer = (char *)malloc(4096); //sizeof(MsgReply));

    pmdRecv(pBuffer, header.messageLen);

    unsigned int probe = 0 ;
    int rc = EDB_OK;

    if (header.opCode == OP_DISCONNECT)
    {
        sock.close();
        Logger::getLogger().debug("[client-->server disconnect] disconnect");
        return EDB_OK;
    }
    else if (header.opCode == OP_DB_SHUTDOWN)
    {
        sock.close();
        ThreadPool::getThreadPool().stopAll();
        return EDB_OK;
    }
    else if (header.opCode == OP_INSERT)
    {
        try
         {
            BSONObj insertor ( &(pBuffer[sizeof(int)]) )  ;

            // make sure _id is included
            BSONObjIterator it ( insertor ) ;
            BSONElement ele = *it ;
            const char *tmp = ele.fieldName () ;
            rc = strcmp ( tmp, "id" ) ;
            if ( rc )
            {
               Logger::getLogger().error("First element in inserted record is not id" ) ;
               probe = 25 ;
               rc = EDB_NO_ID ;
               return rc;
            }

            Logger::getLogger().debug("[client-->server insert] num: %d, data: %s",
                                      ntohl(*((int*)pBuffer)),
                                      insertor.toString().c_str());
            //insert record
            BSONObj outerobj;
            dmsRecordID rid;
            rc = _dmsFilePtr->insert(insertor, outerobj, rid);
            if (rc)
            {
                Logger::getLogger().debug("insert record error");
                return rc;
            }
            else
                Logger::getLogger().debug("insert ok , outer obj: %s", outerobj.toString().c_str());

            //send reply
            BSONObj retobj = bson::fromjson("{msg:'insert ok'}");
            int replyBodyLen = sizeof(int) * 2 + retobj.objsize();
            sendHeader(_socket, header.sequence, replyBodyLen, OP_REPLY);
            msgBuildReply(&pResultBuffer, rc, retobj);
            pmdSend(pResultBuffer, replyBodyLen);
         }
         catch ( std::exception &e )
         {
            Logger::getLogger().error("Failed to create insertor for insert: %s",
                     e.what() ) ;
            probe = 30 ;
            rc = EDB_INVALIDARG ;
            return rc;
         }

    }
    else if (header.opCode == OP_QUERY)
    {
        char *pdata;
        msgExtractQuery( pBuffer, &pdata );
        BSONObj bsonData(pdata);
        Logger::getLogger().debug("[client-->server query] data: %s", bsonData.toString().c_str());
        free(pdata);

        //send reply
        BSONObj retobj = bson::fromjson("{id:1,name:'testquery',sex:'male',age:'35'}");
        int replyBodyLen = sizeof(int) * 2 + retobj.objsize();
        sendHeader(_socket, header.sequence, replyBodyLen, OP_REPLY);
        msgBuildReply(&pResultBuffer, 0, retobj);
        pmdSend(pResultBuffer, replyBodyLen);
    }
    else if (header.opCode == OP_DELETE)
    {
        char *pdata;
        msgExtractDelete( pBuffer, &pdata );
        BSONObj bsonData(pdata);
        Logger::getLogger().debug("[client-->server delete] data: %s", bsonData.toString().c_str());
        free(pdata);

        //build reply
        BSONObj retobj = bson::fromjson("{msg: 'delete ok'}");
        int replyBodyLen = sizeof(int) * 2 + retobj.objsize();
        sendHeader(_socket, header.sequence, replyBodyLen, OP_REPLY);
        msgBuildReply(&pResultBuffer, 0, retobj);
        pmdSend(pResultBuffer, replyBodyLen);
    }

    free(pBuffer);
    free(pResultBuffer);
}

void pmdAgent::run()
{
    _socket.disableNagle();
    char peerAddress[128];
    _socket.getPeerAddress(peerAddress, sizeof(peerAddress));
    int peerPort = _socket.getPeerPort();
    Logger::getLogger().debug("[client-->server connect] peer address: %s; peer port: %d",
                              peerAddress, peerPort);

    MsgHeader header;
    while(_socket.isConnected())
    {
        recvHeader(_socket, header);
        if (header.opCode>0)
        {
            recvBodyAndSend(_socket, header);
        }
    }
}
