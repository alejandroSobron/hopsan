#ifndef RELAYHANDLER_H
#define RELAYHANDLER_H

#include <thread>
#include <mutex>
#include <queue>

#include "zmq.hpp"
#include "MessageUtilities.h"

typedef std::lock_guard<std::mutex> scope_lock;

extern zmq::context_t gContext;

class Relay
{
public:
    Relay(const std::string &id, const std::string &endpoint ) : mId(id), mEndpoint(endpoint)
    {
        mHaveFrontMessage.lock();
    }

    bool connectToEndpoint()
    {
        int linger_ms = 1000;
        try
        {
            mpBackendSocket = new zmq::socket_t(gContext, ZMQ_REQ);
            mpBackendSocket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
            mpBackendSocket->connect(mEndpoint.c_str());
            return true;
        }
        catch(zmq::error_t e)
        {
            std::cout << "Error: Relay could not create backend socket" << std::endl;
            return false;
        }
    }

    void disconnectFromEndpoint()
    {
        mpBackendSocket->disconnect(mEndpoint.c_str());
        delete mpBackendSocket;
        mpBackendSocket=nullptr;
    }

    void relayThread()
    {
        while (true)
        {
            mHaveFrontMessage.lock();
            if (mStopThread)
            {
                break;
            }

            zmq::message_t *pFront_msg=nullptr;
            mMutex.lock();
            mHaveFrontMessage.unlock();
            if (!mFronInputQueue.empty())
            {
                pFront_msg = mFronInputQueue.front();
                mFronInputQueue.pop();
            }
            if (mFronInputQueue.empty())
            {
                mHaveFrontMessage.lock();
            }
            mMutex.unlock();

            if (pFront_msg && mpBackendSocket)
            {
                // Send message
                mpBackendSocket->send(*pFront_msg);

                // Wait for response
                zmq::message_t back_msg;
                receiveWithTimeout(*mpBackendSocket, 10000, back_msg); //!< @todo timeout size

                //            size_t offset;
                //            bool unpackok;
                //            size_t id = getMessageId(back_msg, offset, unpackok);
                //            std::cout << "back_msg id:" << id << std::endl;

                //! @todo handle timeout (Send Nack)
                // Now it should be safe to delete teh front message (as we have gotten a reply)
                delete pFront_msg;

                // Push the return message onto queue
                mMutex.lock();
                zmq::message_t *pNewMsg = new zmq::message_t();
                pNewMsg->move(&back_msg);
                mBackInputQueue.push(pNewMsg);
                mHaveResponse = true;
                mMutex.unlock();
            }

            if (mStopThread)
            {
                break;
            }
        }
        mThreadRunning = false;
    }

    void startRelaying()
    {
        if (!mThreadRunning)
        {
            mStopThread= false;
            mThreadRunning = true;
            mpThread = new std::thread(&Relay::relayThread, this);
        }
    }

    void stopRelaying()
    {
        mStopThread = true;
        mHaveFrontMessage.unlock();
        mpThread->join();
        mpThread = nullptr;
    }

    std::string id() const
    {
        return mId;
    }

    bool haveResponse() const
    {
        return mHaveResponse;
    }

    bool threadRunning() const
    {
        return mThreadRunning;
    }

    void popResponse(zmq::message_t &rResponse)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        rResponse.move(mBackInputQueue.front());
        delete mBackInputQueue.front();
        mBackInputQueue.pop();
        if (mBackInputQueue.empty())
        {
            mHaveResponse = false;
        }
    }

    void pushMessage(zmq::message_t &msg)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        zmq::message_t *pNewMsg = new zmq::message_t();
        pNewMsg->move(&msg);
        mFronInputQueue.push(pNewMsg);
        mHaveFrontMessage.unlock();
    }

    bool isFinished() const
    {
        return !mThreadRunning && !mHaveResponse;
    }

private:
    std::string mId;
    std::string mEndpoint;
    std::queue<zmq::message_t*> mFronInputQueue;
    std::queue<zmq::message_t*> mBackInputQueue;
    zmq::socket_t *mpBackendSocket;
    std::thread *mpThread=nullptr;
    std::mutex mMutex;
    std::mutex mHaveFrontMessage;
    bool mStopThread = true;
    bool mHaveResponse = false;
    bool mThreadRunning = false;
};

class RelayHandler
{
public:
    ~RelayHandler()
    {
        // Remove and close all relays
        for (auto it=mRelays.begin(); it!=mRelays.end(); ++it)
        {
            removeRelay(it->first);
            it = mRelays.begin();
        }
        purgeRemoved();
    }

    Relay* addRelay(std::string id, std::string endpoint)
    {
        scope_lock lock(mMutex);
        Relay *pRelay = new Relay(id, endpoint);
        mRelays.insert(std::pair<std::string, Relay*>(id, pRelay));
        return pRelay;
    }

    void removeRelay(const std::string &id)
    {
        scope_lock lock(mMutex);
        auto it = mRelays.find(id);
        if (it != mRelays.end())
        {
            mRemoved.push_back(it->second);
            mRelays.erase(it);
            mRemoved.back()->stopRelaying();
        }
    }

    void purgeRemoved()
    {
        scope_lock lock(mMutex);
        for (auto it=mRemoved.begin(); it!=mRemoved.end(); ++it)
        {
            if ((*it)->isFinished())
            {
                (*it)->disconnectFromEndpoint();
                delete *it;
                mRemoved.erase(it);
                it=mRemoved.begin();
            }
        }
    }

    std::list<Relay*> getRelays()
    {
        scope_lock lock(mMutex);
        std::list<Relay*> out;
        for (auto it=mRelays.begin(); it!=mRelays.end(); ++it)
        {
            out.push_back(it->second);
        }
        return out;
    }

    Relay* getRelay(const std::string &rRelayId)
    {
        scope_lock lock(mMutex);
        auto it = mRelays.find(rRelayId);
        if (it != mRelays.end())
        {
            return it->second;
        }
        return 0;
    }

private:
    std::mutex mMutex;
    std::map<std::string, Relay*> mRelays;
    std::list<Relay*> mRemoved;
};

extern RelayHandler gRelayHandler;

#endif // RELAYHANDLER_H
