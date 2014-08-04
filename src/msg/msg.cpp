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
#include "core.h"
#include "msg.h"
#include "logger.h"

int msgBuildReply(char **pResultBuffer, int returnCode, char **ppContent)
{
    MsgReply *pReply = (MsgReply *)(*pResultBuffer);
    pReply->returnCode = htonl(returnCode);
    pReply->numReturn = htonl( ((*ppContent)==NULL)?0:1 );
    strncpy(&pReply->data[0], *ppContent, strlen(*ppContent));
    return EDB_OK;
}

int msgBuildReply1(char **pResultBuffer, int returnCode, char *pContent)
{
    MsgReply *pReply = (MsgReply *)(*pResultBuffer);
    pReply->returnCode = htonl(returnCode);
    pReply->numReturn = htonl( ((pContent)==NULL)?0:1 );
    memcpy(&pReply->data[0], pContent, strlen(pContent));
    return EDB_OK;
}

int msgBuildReply(char **pResultBuffer, int returnCode, BSONObj &obj)
{
    MsgReply *pReply = (MsgReply *)(*pResultBuffer);
    pReply->returnCode = htonl(returnCode);
    pReply->numReturn = htonl( ((&obj)==NULL)?0:1 );
    memcpy(&pReply->data[0], obj.objdata(), obj.objsize());
    return EDB_OK;
}

int msgExtractReply(char *pBuffer, int &returnCode, int &numReturn, char **pData)
{
    MsgReply *pMsgResply = (MsgReply *)pBuffer;
    returnCode = ntohl(pMsgResply->returnCode);
    numReturn = ntohl(pMsgResply->numReturn);

    BSONObj obj(&pMsgResply->data[0]);
    *pData = (char *)malloc(obj.objsize());

    memcpy( *pData, &pMsgResply->data[0], obj.objsize() );
    return EDB_OK;
}

int msgBuildHeader(char **pBuffer, int seq, int len, int opCode)
{
    MsgHeader *pMsgHeader = (MsgHeader *)(*pBuffer);
    pMsgHeader->sequence = htonl(seq);
    pMsgHeader->messageLen = htonl(len);
    pMsgHeader->opCode = htonl(opCode);
    return EDB_OK;
}

int msgExtractHeader(char *pBuffer, int &seq, int &len, int &opCode)
{
    MsgHeader *pMsgHeader = (MsgHeader *)pBuffer;
    seq = ntohl(pMsgHeader->sequence);
    len = ntohl(pMsgHeader->messageLen);
    opCode = ntohl(pMsgHeader->opCode);
    return EDB_OK;
}

int msgBuildInsert(char **pBuffer, int numInsert, char **pData)
{
    MsgInsert *pMsgInsert = (MsgInsert *)*pBuffer;
    pMsgInsert->numInsert = htonl(numInsert);
    strncpy(&pMsgInsert->data[0], *pData, strlen(*pData));
    return EDB_OK;
}

int msgBuildInsert(char **pBuffer, int numInsert, BSONObj &obj)
{
    MsgInsert *pMsgInsert = (MsgInsert *)*pBuffer;
    pMsgInsert->numInsert = htonl(numInsert);
    memcpy(&pMsgInsert->data[0], obj.objdata(), obj.objsize());
}

int msgExtractInsert(char *pBuffer, int &numInsert, char **pData)
{
    MsgInsert *pMsgInsert = (MsgInsert *)pBuffer;
    numInsert = ntohl(pMsgInsert->numInsert);
    int len = strlen( &(pMsgInsert->data[0]) );
    *pData = (char *)malloc(len + 1);
    memset(*pData, 0, len + 1);
    strncpy(*pData, &pMsgInsert->data[0], len);
    return EDB_OK;
}

int msgBuildQuery(char **pBuffer, char **pData)
{
    MsgQuery *pMsgQuery = (MsgQuery *)(*pBuffer);
    strncpy(&pMsgQuery->key[0], *pData, strlen(*pData));
    return EDB_OK;
}

int msgBuildQuery(char **pBuffer, BSONObj &obj)
{
    MsgQuery *pMsgQuery = (MsgQuery *)(*pBuffer);
    memcpy(&pMsgQuery->key[0], obj.objdata(), obj.objsize());
    return EDB_OK;
}

int msgExtractQuery(char *pBuffer, char **pData)
{
    MsgQuery *pMsgQuery = (MsgQuery *)pBuffer;

    BSONObj obj(&pMsgQuery->key[0]);
    *pData = (char *)malloc(obj.objsize());

    memcpy(*pData, &pMsgQuery->key[0], obj.objsize());
    return EDB_OK;
}

int msgBuildDelete(char **pBuffer, char **pData)
{
    MsgDelete *pMsgDelete = (MsgDelete *)*pBuffer;
    strncpy(&pMsgDelete->key[0], *pData, strlen(*pData));
    return EDB_OK;
}

int msgBuildDelete(char **pBuffer, BSONObj &obj)
{
    MsgDelete *pMsgDelete = (MsgDelete *)*pBuffer;
    memcpy(&pMsgDelete->key[0], obj.objdata(), obj.objsize());
    return EDB_OK;
}


int msgExtractDelete(char *pBuffer, char **pData)
{
    MsgDelete *pMsgDelete = (MsgDelete *)pBuffer;

    BSONObj obj(&pMsgDelete->key[0]);
    *pData = (char *)malloc(obj.objsize());

    memcpy(*pData, &pMsgDelete->key[0], obj.objsize());
    return EDB_OK;
}
