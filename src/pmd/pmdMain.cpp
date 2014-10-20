#include "core.h"
#include "osSocket.h"
#include "logger.h"
#include "threadPool.h"
#include "pmdTcpListener.h"
#include "dmsFile.h"

using namespace std;
using namespace myan::utils;

int main()
{
    Logger::getLogger().init("run.log", Logger::LOG_DEBUG_LEVEL, true);

    //init data file
    DMSFILE_PTR dmsFilePtr = new dmsFile();
    dmsFilePtr->initialize("data/myan-bin.data");

    Runnable *tcpListener =  new pmdTcpListener(dmsFilePtr);
    ThreadPool::getThreadPool().start(tcpListener);
    ThreadPool::getThreadPool().joinAll();

    delete  dmsFilePtr;
}
