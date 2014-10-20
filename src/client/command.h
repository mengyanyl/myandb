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
#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include "core.h"
#include "osSocket.h"
#define COMMAND_QUIT         "quit"
#define COMMAND_INSERT       "insert"
#define COMMAND_QUERY        "query"
#define COMMAND_DELETE       "delete"
#define COMMAND_HELP         "help"
#define COMMAND_CONNECT      "connect"
#define COMMAND_TEST         "test"
#define COMMAND_SNAPSHOT     "snapshot"
#define COMMAND_DB_SHUTDOWN     "shutdown"

#define RECV_BUF_SIZE     4096
#define SEND_BUF_SIZE     4096

#define EDB_QUERY_INVALID_ARGUMENT     -101
#define EDB_INSERT_INVALID_ARGUMENT    -102
#define EDB_DELETE_INVALID_ARGUMENT    -103

#define EDB_INVALID_RECORD              -104
#define EDB_RECV_DATA_LENGTH_ERROR     -107

#define EDB_SOCK_INIT_FAILED           -113
#define EDB_SOCK_CONNECT_FAILED        -114
#define EDB_SOCK_NOT_CONNECT           -115
#define EDB_SOCK_REMOTE_CLOSED         -116
#define EDB_SOCK_SEND_FAILD            -117

#define EDB_MSG_BUILD_FAILED           -119


class ICommand
{
   public:
      ICommand()
      {
          memset(_sendBuf, 0, SEND_BUF_SIZE);
          memset(_recvBuf, 0, RECV_BUF_SIZE);
      }
      virtual int execute( osSocket & sock, std::vector<std::string> & argVec)
      {
          return EDB_OK;
      };
      int         getError(int code);
   protected:
      virtual int handleReply() { return EDB_OK; }
       int recvReply( osSocket & sock );
       int sendOrder( osSocket & sock, int opCode, int len );
   protected:
      char _recvBuf[RECV_BUF_SIZE];
      char _sendBuf[SEND_BUF_SIZE];
      std::string _jsonString;
   private:
       unsigned int getSequence();
};

class ConnectCommand : public ICommand
{
   public:
      int execute ( osSocket & sock, std::vector<std::string> & argVec );
   private:
      std::string _address;
      int         _port;
};

class QuitCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
   protected:
      int handleReply();
};

class DBShutDownCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
   protected:
      int handleReply();
};

class InsertCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
   protected:
      int handleReply();
};

class QueryCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
   protected:
      int handleReply();
};

class DeleteCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
   protected:
      int handleReply();
};

class HelpCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
};

class TestCommand : public ICommand
{
   public:
      int execute(osSocket & sock, std::vector<std::string> & argVec );
};

#endif
