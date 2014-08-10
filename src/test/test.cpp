#include "core.h"
#include "threadPool.h"
#include "runnable.h"
#include "logger.h"

using namespace myan::utils;

class TestFun : public Runnable
{
    public :
    TestFun(int i)
{

        _i = i;
    }
    void run()
    {
        int sum =0;
        for (int i=0; i<1000000; i++)
        {
            sum += i;
        }
        Logger::getLogger().debug("%d test function by thread, result: %d", _i, sum);
    }
    private:
        int _i;
};

int main()
{
    Logger::getLogger().init("log_test.log", Logger::LOG_DEBUG_LEVEL, true);
    vector<TestFun*> vec;
    std::string sname("threadpool");
    ThreadPool tp(sname, 4);

    for (int i=0; i<10; i++)
    {
        TestFun *ptf = new TestFun(i);
        vec.push_back(ptf);
    }

    for(vector<TestFun*>::iterator iter=vec.begin(); iter!=vec.end(); ++iter)
        tp.start(*iter, "");

    tp.stopAll();

    for (vector<TestFun*>::iterator iter = vec.begin(); iter!=vec.end(); ++iter)
        delete *iter;
}
