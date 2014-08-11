#include "core.h"
#include "osSocket.h"
#include "logger.h"
#include "threadPool.h"
#include "pmdTcpListener.h"

using namespace std;
using namespace myan::utils;

int main()
{
    Logger::getLogger().init("run.log", Logger::LOG_DEBUG_LEVEL, true);

    Runnable *tcpListener =  new pmdTcpListener();
    ThreadPool::getThreadPool().start(tcpListener);
}
