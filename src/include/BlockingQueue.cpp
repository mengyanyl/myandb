#ifndef _BLOCKINGQUEUE_H_
#define _BLOCKINGQUEUE_H_

#include <queue>
#include <boost/thread.hpp>
#include <string>
#include "Logger.h"

namespace myan
{
namespace utils
{

template <class T>
class BlockQueue
{
public:
    BlockQueue(std::string aqueueName="default", int maxCount=50)
    {
        _queueName= aqueueName;
        _maxCount=maxCount;
        ready=true;
        _queueCount=0;
    }

    void setQueueCount(int count)
    {
        this->_queueCount= count;
    }

    virtual ~BlockQueue<T>(void)
    {
    }

    bool offer(T t)
    {
        boost::mutex::scoped_lock scope(_mutex);

        if (_queueCount>=_maxCount)
        {
            //Myan::NewFile::Logger::getLogger().debug("Queue[%s] is full; count=%d; waiting...", _queueName.c_str(), _queueCount);
            _cond.wait(_mutex);
            return false;
        }

        _queue.push(t);

        ++_queueCount;

        _cond.notify_one();

        return true;
    }

    void put(T t)
    {
        boost::mutex::scoped_lock scope(_mutex);

        while(_queueCount>=_maxCount)
        {
//            boost::thread::yield();
            //Myan::NewFile::Logger::getLogger().debug("Queue[%s] is full; count=%d; waiting...", _queueName.c_str(), _queueCount);
            _cond.wait(_mutex);
        }

        _queue.push(t);

        ++_queueCount;

        _cond.notify_one();
    }

    bool poll(T &t)
    {
        boost::mutex::scoped_lock scope(_mutex);

        if (_queue.empty())
        {
            _cond.wait(scope);
            return false;
        }


        t = _queue.front();
        _queue.pop();

        --_queueCount;

        _cond.notify_one();

        return true;
    }

    bool take(T &t)
    {
        boost::mutex::scoped_lock scope(_mutex);
        std::cout << _queueName << " queue size: " <<  _queueCount << "*****************" << std::endl;
        while(_queue.empty())
        {
//            boost::thread::yield();
            _cond.wait(scope);
        }

        t = _queue.front();
        _queue.pop();

        --_queueCount;

        _cond.notify_one();

        return true;
    }

    int getQueueSize()
    {
        boost::mutex::scoped_lock scope(_mutex);
        return _queueCount;
    }

private:
    int _maxCount;
    std::string _queueName;
    int _queueCount;
    bool ready;
    std::queue<T> _queue;
    boost::condition_variable_any _cond;
    boost::mutex _mutex;
};

}
}

#endif
