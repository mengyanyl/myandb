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

#ifndef MSG_HPP__
#define MSG_HPP__

#include "bson.h"

#define OP_REPLY                   1
#define OP_INSERT                  2
#define OP_DELETE                  3
#define OP_QUERY                   4
#define OP_COMMAND                 5
#define OP_DISCONNECT              6
#define OP_CONNECT                 7
#define OP_SNAPSHOT                8

#define RETURN_CODE_STATE_OK       1

using namespace bson;

struct MsgHeader
{
   int sequence;
   int messageLen ;
   int opCode ;
} ;

//   MsgHeader header ;
struct MsgReply
{
   int       returnCode ;
   int       numReturn ;
   char      data[0] ;
} ;

//   MsgHeader header ;
struct MsgInsert
{
   int       numInsert ;
   char      data[0] ;
} ;

//   MsgHeader header ;
struct MsgDelete
{
   char      key[0] ;
} ;

//   MsgHeader header ;
struct MsgQuery
{
   char      key[0] ;
} ;

struct MsgCommand
{
   MsgHeader header ;
   int       numArgs ;
   char      data[0] ;
} ;

int msgBuildReply(char **pResultBuffer, int returnCode, char **ppContent);

int msgBuildReply1(char **pResultBuffer, int returnCode, char *pContent);

int msgBuildReply(char **pResultBuffer, int returnCode, BSONObj &obj);

int msgExtractReply(char *pBuffer, int &returnCode, int &numReturn, char **pData);

int msgBuildHeader(char **pBuffer, int seq, int len, int opCode);

int msgExtractHeader(char *pBuffer, int &seq, int &len, int &opCode);

int msgBuildInsert(char **pBuffer, int numInsert, char **pData);

int msgBuildInsert(char **pBuffer, int numInsert, BSONObj &obj);

int msgExtractInsert(char *pBuffer, int &numInsert, char **pData);

int msgBuildQuery(char **pBuffer, char **pData);

int msgBuildQuery(char **pBuffer, BSONObj &obj);

int msgExtractQuery(char *pBuffer, char **pData);

int msgBuildDelete(char **pBuffer, char **pData);

int msgBuildDelete(char **pBuffer, BSONObj &obj);

int msgExtractDelete(char *pBuffer, char **pData);

#endif
