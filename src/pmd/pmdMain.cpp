#include "core.h"

#include "osSocket.h"
#include "logger.h"
#include "msg.h"
#include "pmdAgent.h"

using namespace std;
using namespace myan::utils;


int pmdTcpListenerEntryPoint()
{
    osSocket svrsock(8888);
    svrsock.initSocket();
    svrsock.bind_listen();

    pmdAgent agent;
    for(;;)
    {
        SOCKET clientSock;
        socklen_t len;

        int re = svrsock.accept(&clientSock, NULL, NULL);
        if ( EDB_TIMEOUT == re )
        {
            re = EDB_OK ;
            continue ;
        }

        agent.pmdAgentEntryPoint(clientSock);
    }
}

int main()
{
    Logger::getLogger().init("run.log", Logger::LOG_DEBUG_LEVEL, true);
    pmdTcpListenerEntryPoint();
}
