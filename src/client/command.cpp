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
#include "command.h"
#include "commandFactory.h"
#include "msg.h"
#include "logger.h"
#include <bson/src/util/json.h>

using namespace myan::utils;

COMMAND_BEGIN
COMMAND_ADD(COMMAND_CONNECT,ConnectCommand)
COMMAND_ADD(COMMAND_INSERT,InsertCommand)
COMMAND_ADD(COMMAND_QUERY,QueryCommand)
COMMAND_ADD(COMMAND_DELETE,DeleteCommand)
COMMAND_ADD(COMMAND_QUIT, QuitCommand)
COMMAND_ADD(COMMAND_HELP, HelpCommand)
COMMAND_END

extern int gQuit;

int ICommand::getError(int code)
{
    switch(code)
    {
    case EDB_OK:
        break;
    case EDB_IO:
        std::cout << "io error is occurred" << std::endl;
        break;
    case EDB_INVALIDARG:
        std::cout << "invalid argument" << std::endl;
        break;
    case EDB_PERM:
        std::cout << "edb_perm" << std::endl;
        break;
    case EDB_OOM:
        std::cout << "edb_oom" << std::endl;
        break;
    case EDB_SYS:
        std::cout << "system error is occurred." << std::endl;
        break;
    case EDB_QUIESCED:
        std::cout << "EDB_QUIESCED" << std::endl;
        break;
    case EDB_NETWORK_CLOSE:
        std::cout << "net work is closed." << std::endl;
        break;
    case EDB_HEADER_INVALID:
        std::cout << "record header is not right." << std::endl;
        break;
    case EDB_IXM_ID_EXIST:
        std::cout << "record key is exist." << std::endl;
        break;
    case EDB_IXM_ID_NOT_EXIST:
        std::cout << "record is not exist" << std::endl;
        break;
    case EDB_NO_ID:
        std::cout << "_id is needed" << std::endl;
        break;
    case EDB_QUERY_INVALID_ARGUMENT:
        std::cout << "invalid query argument" << std::endl;
        break;
    case EDB_INSERT_INVALID_ARGUMENT:
        std::cout <<  "invalid insert argument" << std::endl;
        break;
    case EDB_DELETE_INVALID_ARGUMENT:
        std::cout << "invalid delete argument" << std::endl;
        break;
    case EDB_INVALID_RECORD:
        std::cout << "invalid record string" << std::endl;
        break;
    case EDB_SOCK_NOT_CONNECT:
        std::cout << "sock connection does not exist" << std::endl;
        break;
    case EDB_SOCK_REMOTE_CLOSED:
        std::cout << "remote sock connection is closed" << std::endl;
        break;
    case EDB_MSG_BUILD_FAILED:
        std::cout << "msg build failed" << std::endl;
        break;
    case EDB_SOCK_SEND_FAILD:
        std::cout << "sock send msg faild" << std::endl;
        break;
    case EDB_SOCK_INIT_FAILED:
        std::cout << "sock init failed" << std::endl;
        break;
    case EDB_SOCK_CONNECT_FAILED:
        std::cout << "sock connect remote server failed" << std::endl;
        break;
    default :
        break;
    }
    return code;
}
int ICommand::recvReply( osSocket & sock )
{
    // define message data length.
    int ret = EDB_OK;
    // fill receive buffer with 0.
    memset(_recvBuf, 0, RECV_BUF_SIZE);
    if( !sock.isConnected() )
    {
        return getError(EDB_SOCK_NOT_CONNECT);
    }
    while(1)
    {
        // receive data from the server.first receive the length of the data.
        ret = sock.recv(_recvBuf, sizeof(MsgHeader));
        if( EDB_TIMEOUT == ret )
        {
            continue;
        }
        else if( EDB_NETWORK_CLOSE == ret )
        {
            return getError(EDB_SOCK_REMOTE_CLOSED);
        }
        else
        {
            break;
        }
    }
    MsgHeader *header = (MsgHeader*)_recvBuf;
    int seq=-1, length=0, opcode=-1;
    // get header
    msgExtractHeader(_recvBuf, seq, length, opcode);
    printf("[client<--server header]: seqno: %d, bodylen: %d, opcode: %d\n",
           seq, length, opcode);
    // judge the length is valid or not.
    if(length > RECV_BUF_SIZE)
    {
        return getError(EDB_RECV_DATA_LENGTH_ERROR);
    }

    //fill recvbuf with 0
    memset(_recvBuf, 0, sizeof(_recvBuf));
    // receive data from the server.second receive the last data.
    while(1)
    {
        ret = sock.recv(_recvBuf, length);
        if(ret == EDB_TIMEOUT)
        {
            continue;
        }
        else if(EDB_NETWORK_CLOSE == ret)
        {
            return getError(EDB_SOCK_REMOTE_CLOSED);
        }
        else
        {
            break;
        }
    }
    return ret;
}

unsigned int _sequence=0;

int ICommand::sendOrder( osSocket & sock, int opCode, int len )
{
    int ret = EDB_OK;
    memset(_sendBuf, 0, SEND_BUF_SIZE);
    char *pSendBuf = _sendBuf;
    msgBuildHeader(&pSendBuf, this->getSequence(), len, opCode);
    ret = sock.send(_sendBuf, sizeof(MsgHeader));
    return ret;
}

unsigned int ICommand::getSequence()
{
    if (_sequence<99999999)
    {
        _sequence++;
    }
    else
    {
        _sequence = 1;
    }

    return _sequence;
}


/******************************ConnectCommand****************************************/
int ConnectCommand::execute( osSocket & sock, std::vector<std::string> & argVec )
{
    int ret = EDB_OK;
    if(argVec.size() < 2)
    {
        printf("too little argument for fuction: ConnectCommand::execute()\n");
        return getError(EDB_QUERY_INVALID_ARGUMENT);
    }
    _address = argVec[0];
    _port = atoi(argVec[1].c_str());
    sock.close();
    sock.setAddress(_address.c_str(), _port);
    ret = sock.initSocket();
    if(ret)
    {
        return getError(EDB_SOCK_INIT_FAILED);
    }
    ret = sock.connect();
    if(ret)
    {
        return getError(EDB_SOCK_CONNECT_FAILED);
    }

    return ret;
}

/******************************QuitCommand**********************************************/
int QuitCommand::handleReply()
{
    int ret = EDB_OK;
    gQuit = 1;
    return ret;
}

int QuitCommand::execute( osSocket & sock, std::vector<std::string> & argVec )
{
    int ret = EDB_OK;
    if( !sock.isConnected() )
    {
        return getError(EDB_SOCK_NOT_CONNECT);
    }
    ret = sendOrder( sock, OP_DISCONNECT, 0 );
    sock.close();
    ret = handleReply();
    return ret;
}

/******************************InsertCommand**********************************************/
int InsertCommand::execute( osSocket &sock, std::vector<std::string> &argVec )
{
    int ret = EDB_OK;
    if ( !sock.isConnected() )
    {
        return getError(EDB_SOCK_CONNECT_FAILED);
    }

    //select {id:1}
    _jsonString = argVec[0];
    char *pSendBuf = _sendBuf;

    bson::BSONObj bsonData;
    try
    {
        bsonData = bson::fromjson(_jsonString.c_str());
    }
    catch( std::exception & e)
    {
        return getError(EDB_INVALID_RECORD);
    }

    int bodyLen = sizeof(int) + bsonData.objsize();
    ret = sendOrder(sock, OP_INSERT, bodyLen);
    if (ret != EDB_OK)
        Logger::getLogger().error("[client-->server] send insert command error");

    msgBuildInsert(&pSendBuf, 1, bsonData);
    sock.send(_sendBuf, bodyLen);

    ret = recvReply(sock);

    ret = handleReply();
}

int InsertCommand::handleReply()
{
    char *pdata;
    int returnCode, numReturn;
    msgExtractReply(_recvBuf, returnCode, numReturn, &pdata);

    bson::BSONObj bsonData(pdata)  ;

    printf("[client<---server] resultCode: %d, msg: %s\n", returnCode, bsonData.toString().c_str());
    free(pdata);
    int ret = getError(returnCode);
    return ret;
}

/******************************QueryCommand**********************************************/
int QueryCommand::execute( osSocket &sock, std::vector<std::string> &argVec )
{
    int ret = EDB_OK;
    if ( !sock.isConnected() )
    {
        return getError(EDB_SOCK_CONNECT_FAILED);
    }

    _jsonString = argVec[0];
    bson::BSONObj bsonData;
    try
    {
        bsonData = bson::fromjson(_jsonString);
    }
    catch( std::exception & e)
    {
        return getError(EDB_INVALID_RECORD);
    }

    int bodyLen = bsonData.objsize();
    ret = sendOrder(sock, OP_QUERY, bodyLen);
    if (ret != EDB_OK)
        Logger::getLogger().error("[client-->server] send query command error");

    char *pSendBuf = _sendBuf;
    msgBuildQuery(&pSendBuf, bsonData);
    sock.send(_sendBuf, bodyLen);

    ret = recvReply(sock);

    ret = handleReply();
}

int QueryCommand::handleReply()
{
    char *pdata;
    int returnCode, numReturn;
    msgExtractReply(_recvBuf, returnCode, numReturn, &pdata);

    bson::BSONObj bsonData(pdata);

    printf("[client<---server] resultCode: %d, msg: %s\n", returnCode, bsonData.toString().c_str());
    free(pdata);
    int ret = getError(returnCode);
    return ret;
}

/******************************DeleteCommand**********************************************/
int DeleteCommand::execute( osSocket &sock, std::vector<std::string> &argVec )
{
    int ret = EDB_OK;
    if ( !sock.isConnected() )
    {
        return getError(EDB_SOCK_CONNECT_FAILED);
    }

    _jsonString = argVec[0];
    bson::BSONObj bsonData;
    try
    {
        bsonData = bson::fromjson(_jsonString);
    }
    catch( std::exception & e)
    {
        return getError(EDB_INVALID_RECORD);
    }
    int bodyLen = bsonData.objsize();

    ret = sendOrder(sock, OP_DELETE, bodyLen);
    if (ret != EDB_OK)
        Logger::getLogger().error("[client-->server] send query command error");

    char *pSendBuf = _sendBuf;
    msgBuildDelete(&pSendBuf, bsonData);
    sock.send(_sendBuf, bodyLen);

    ret = recvReply(sock);

    ret = handleReply();
}

int DeleteCommand::handleReply()
{
    char *pdata;
    int returnCode, numReturn;
    msgExtractReply(_recvBuf, returnCode, numReturn, &pdata);
    bson::BSONObj bsonData(pdata);
    printf("[client<---server] resultCode: %d, msg: %s\n", returnCode, bsonData.toString().c_str());
    free(pdata);
    int ret = getError(returnCode);
    return ret;
}

/******************************HelpCommand**********************************************/
int HelpCommand::execute( osSocket & sock, std::vector<std::string> & argVec )
{
    int ret = EDB_OK;
    printf("List of classes of commands:\n\n");
    printf("%s [server] [port]-- connecting myandb server\n", COMMAND_CONNECT);
    printf("%s -- sending a insert command to myandb server\n", COMMAND_INSERT);
    printf("%s -- sending a query command to myandb server\n", COMMAND_QUERY);
    printf("%s -- sending a delete command to myandb server\n", COMMAND_DELETE);
    printf("%s [number]-- sending a test command to myandb server\n", COMMAND_TEST);
    printf("%s -- providing current number of record inserting\n", COMMAND_SNAPSHOT);
    printf("%s -- quitting command\n\n", COMMAND_QUIT);
    printf("Type \"help\" command for help\n");
    return ret;
}

int TestCommand::execute( osSocket &sock, std::vector<std::string> &argVec)
{

}
