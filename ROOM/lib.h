// Copyright 2026 by HDS OS d.o.o.

#ifndef room_lib_h
#define room_lib_h

#include <cstddef>

#include "placement.h"

template <size_t RegionSize>
class Allocator {
public:
    void* malloc(size_t size) {
        if (size == 0 || size > RegionSize || used + size > RegionSize) {
            return nullptr;
        }
        void* ptr = storage + used;
        used += size;
        return ptr;
    }

private:
    alignas(8) char storage[RegionSize]{};
    size_t used = 0;
};

#include "api/kernel.h"
#include "api/sync.h"
#include "api/periodic_task.h"
#include "api/sporadic_task.h"
#include "api/stack.h"

using Common::Status;

namespace ROOM {

typedef unsigned char sharedid_t;

constexpr sharedid_t nullsid = -1;
typedef unsigned char coreid_t;
constexpr coreid_t nullcoreid = 0;
constexpr coreid_t coreCM7 = 1;
constexpr coreid_t coreCM4 = 2;


inline bool areRemote (coreid_t core1, coreid_t core2) {
    return (core1!=nullcoreid) && (core2!=nullcoreid) && (core1!=core2);
}


class Mutex {
public:
    virtual ~Mutex() {}
    virtual void acquire () = 0;
    virtual void release () = 0;
};


class LocalMutex : public Mutex {
public:
    virtual void acquire () override { mx.acquire(); }
    virtual void release () override { mx.release(); }
private:
    Kernel::Mutex mx;
};




class Actor;
class PortBase;


class ActorTask : public Kernel::Runnable {
public:
    virtual ~ActorTask () {}
    virtual Status start() { return Status::ok; }

    virtual void signal () {}

protected:
    ActorTask (Actor* a) : myActor(a) {}
    Actor* getActor () const { return myActor; }

private:
    Actor* myActor;
};


class PeriodicActorTask : public ActorTask {
public:
    PeriodicActorTask (Actor* a, Kernel::Stack* s, Kernel::PeriodicParams* pp, Kernel::TaskConfig cfg = {})
        : ActorTask(a), myTask(*this, s, pp, cfg) {}

    virtual Status start() override { return myTask.start(); }

protected:
    virtual void activate () override;

private:
    Kernel::PeriodicTask myTask;
};


class SporadicActorTask : public ActorTask {
public:
    SporadicActorTask (Actor* owner, Kernel::Stack* s, const Kernel::SporadicParams* sp, Kernel::TaskConfig cfg = {});

    virtual void signal () override { myTask.signal(); }

    virtual Status start() override { return myTask.start(); }

    Kernel::SporadicTask& getKernelTask() { return myTask; }

protected:
    virtual void activate () override;

private:
    Kernel::SporadicTask myTask;
};


class RemoteActorTask : public ActorTask {
public:
    RemoteActorTask (Actor* a) : ActorTask(a) {}

    virtual void signal () override;

protected:
    virtual void activate () override {}
};




class Actor {
public:
    virtual ~Actor () {}
    
    coreid_t   getCore () const { return myCore; }
    sharedid_t getSID ()  const { return mySID; }

    inline void addEndPort (PortBase*);

    inline bool process ();

    inline void signal () { if (myTask)  myTask->signal();  }

    inline void build  (coreid_t host);
    inline void start  ();

    virtual bool doesReceiveRemoteMsg () const;
    virtual bool hasSporadicBehavior () const { return false; }

    virtual SporadicActorTask* getTask () const { return nullptr; }

protected:
    Actor (Actor* owner = nullptr, coreid_t target = nullcoreid) : myCore(target) { if (owner) owner->addPart(this); }

    inline  void lock   () { if (myMutex) myMutex->acquire(); }
    inline  void unlock () { if (myMutex) myMutex->release(); }

    virtual bool scanPorts () { return true; }
    virtual void readPorts ();
    virtual void behavior  () {}
    
    inline void addPart (Actor*);
    void setSID () { if (this->doesReceiveRemoteMsg()) this->mySID = getNewSid(); }
    
    virtual ActorTask* createTask ();

    ActorTask* hGetTask () const { return myTask; }

private:
    template<class, class, size_t> friend class ServicePort;
    coreid_t myCore = nullcoreid;
    sharedid_t mySID = nullsid;
    Mutex* myMutex = nullptr;
    ActorTask* myTask = nullptr;

    PortBase* myInputEndPortsHead = nullptr;
    Actor* myPartsHead = nullptr;
    Actor* next = nullptr;
    
    inline static sharedid_t sidCounter = 0;
    static sharedid_t getNewSid () { return sidCounter++; }

};

// Application-side: timing params, priority, and stack allocator for an actor type.
#define DEFINE_PERIODIC_TASK(STACK_SIZE, ACTOR_TYPE, PERIOD, PRIORITY) \
    static Kernel::PeriodicParams ACTOR_TYPE##Params = [] {            \
        Kernel::PeriodicParams params{};                               \
        params.period = PERIOD;                                        \
        return params;                                                 \
    }();                                                               \
    PeriodicActorTask* ACTOR_TYPE::createTask () {                     \
        return new (tasks) PeriodicActorTask(                      \
            this, new (stacks) Kernel::StackMem<STACK_SIZE>(), tp_ptr, cfg); \
    }                                                                  \
    Kernel::PeriodicParams* ACTOR_TYPE::tp_ptr = &ACTOR_TYPE##Params;  \
    Kernel::TaskConfig ACTOR_TYPE::cfg = Kernel::makeTaskCfg(PRIORITY);


#define DEFINE_SPORADIC_TASK(STACK_SIZE, ACTOR_TYPE, PRIORITY)         \
    static Kernel::SporadicParams ACTOR_TYPE##Params{};                \
    SporadicActorTask* ACTOR_TYPE::createTask () {                     \
        return new (tasks) SporadicActorTask(                          \
            this, new (stacks) Kernel::StackMem<STACK_SIZE>(), tp_ptr, cfg); \
    }                                                                  \
    Kernel::SporadicParams* ACTOR_TYPE::tp_ptr = &ACTOR_TYPE##Params;  \
    Kernel::TaskConfig ACTOR_TYPE::cfg = Kernel::makeTaskCfg(PRIORITY); 




class PortBase {
public:
    virtual ~PortBase () {}

    Actor*   getOwner () const { return myActor; }
    coreid_t getCore  () const  { return this->getOwner()->getCore(); }

    virtual bool doesReceiveRemoteMsg () const { return false; }

    virtual bool read () { return false; }
    virtual void createBuffer (coreid_t host) {} 

protected:
    PortBase (Actor* owner) : myActor(owner) {}

private:
    friend class Actor;

    Actor* myActor;
    PortBase* next;
};




template<class, class, size_t> class ServicePort;

template<class ProtocolIn, class ProtocolOut>
class Port : public PortBase {
public:
    using SignalIn  = typename ProtocolIn::Signal;
    using SignalOut = typename ProtocolOut::Signal;

    using MessageIn  = typename ProtocolIn::Message;
    using MessageOut = typename ProtocolOut::Message;

    Port (Actor* owner) : PortBase(owner) {}

    void setInTarget  (Port<ProtocolIn,ProtocolOut>* p) { _inTarget = p; }
    void setOutTarget (Port<ProtocolOut,ProtocolIn>* p) { this->getInTarget()->_outTarget = p->getInTarget(); }

protected:
    friend class Port<ProtocolOut,ProtocolIn>;

    virtual Port<ProtocolIn,ProtocolOut>* getInTarget () {
      return _inTarget?_inTarget->getInTarget():this;
    }

    virtual Port<ProtocolOut,ProtocolIn>* getOutTarget () const {
        return _outTarget;
    }

    virtual void receive (const MessageIn& m) {
        auto p = this->getInTarget();
        if (p && p != this) p->receive(m);
    }

    void send (const MessageOut& m) const {
        if (auto p = this->getOutTarget()) p->receive(m);
    }

private:
    Port<ProtocolOut,ProtocolIn>* _outTarget = nullptr;
    Port<ProtocolIn,ProtocolOut>* _inTarget = nullptr;
};




template<typename Msg, size_t CAPACITY>
class PortMsgQueue {
public:
    PortMsgQueue () {}

    void  put (const Msg&);
    bool  read ();
    bool  hasNewMsg () const { return count > 0; }
    const Msg* msg () const { return hasRead ? &readBuf : nullptr; }

private:
    Msg    buffer[CAPACITY];
    Msg    readBuf;
    size_t head = 0, tail = 0, count = 0;
    bool   hasRead = false;
};


template<typename Msg, size_t CAPACITY>
void PortMsgQueue<Msg,CAPACITY>::put (const Msg& m) {
    buffer[tail] = m;
    tail = (tail + 1) % CAPACITY;
    if (count < CAPACITY) {
        count++;
    } else {
        head = tail;
    }
}


template<typename Msg, size_t CAPACITY>
bool PortMsgQueue<Msg,CAPACITY>::read () {
    if (count == 0) return false;
    readBuf = buffer[head];
    head = (head + 1) % CAPACITY;
    count--;
    hasRead = true;
    return true;
}


template<typename Msg>
class PortMsgQueue<Msg, 1> {
public:
    PortMsgQueue () {}

    bool  hasNewMsg () const  { return pending; }
    const Msg* msg () const   { return readIdx == npos ? nullptr : &buffer[readIdx]; }

    void  put (const Msg& m) { 
        buffer[writeIdx] = m; 
        pending = true; 
    }

    bool  read () { 
        if (!pending) return false; 
        readIdx = writeIdx; 
        writeIdx = 1 - writeIdx; 
        pending = false; 
        return true; 
    }

private:
    static constexpr size_t npos = size_t(-1);
    Msg    buffer[2];
    size_t writeIdx = 0, readIdx = npos;
    bool   pending  = false;
};




template<class ProtocolIn, class ProtocolOut, size_t CAPACITY>
class ServicePort : public Port<ProtocolIn,ProtocolOut> {
public:
    using MessageIn  = typename Port<ProtocolIn,ProtocolOut>::MessageIn;
    using MessageOut = typename Port<ProtocolIn,ProtocolOut>::MessageOut;

    ServicePort (Actor* owner) : Port<ProtocolIn,ProtocolOut>(owner) {
        if (this->doesReceiveMsg()) owner->addEndPort(this);
    }

    static constexpr bool doesReceiveMsg () { return MessageIn::hasMsg(); }

    bool doesReceiveRemoteMsg () const override {
        if constexpr (!doesReceiveMsg()) return false;
        else if (auto p = this->getOutTarget())
            return areRemote(this->getCore(),p->getCore());
        else
            return false;
    }

    void send (const MessageOut& m) const { Port<ProtocolIn,ProtocolOut>::send(m); }

    bool  hasNewMsg () const { return myQue?myQue->hasNewMsg():false; }
    bool  read () override { return myQue?myQue->read():false; }
    const MessageIn* msg () const { return myQue?myQue->msg():nullptr; }

protected:
    virtual void receive (const MessageIn& m) override;
    
    virtual void createBuffer (coreid_t host) override;

private:
    Actor* myOwner;
    PortMsgQueue<MessageIn,CAPACITY>* myQue = nullptr;
};


template<class ProtocolIn, class ProtocolOut, size_t CAPACITY>
inline void ServicePort<ProtocolIn,ProtocolOut,CAPACITY>::receive (const MessageIn& m) {
    if (!this->myQue) return;
    Actor* myOwner = this->getOwner();
    myOwner->lock();
    this->myQue->put(m);
    myOwner->unlock();
    myOwner->signal();
}


template<class ProtocolIn, class ProtocolOut, size_t CAPACITY>
inline void ServicePort<ProtocolIn,ProtocolOut,CAPACITY>::createBuffer (coreid_t host) {
    if (!this->getOutTarget()) return;
    coreid_t myCore = this->getCore();
    coreid_t peerCore = this->getOutTarget()->getCore();


    if (areRemote(myCore, peerCore)) {
        this->myQue = new (sharedBuffers) PortMsgQueue<MessageIn,CAPACITY>;
    } else if (myCore == host) {
        this->myQue = new (localBuffers) PortMsgQueue<MessageIn,CAPACITY>;
    }
}


inline void Actor::addEndPort (PortBase* p) {
    p->next = this->myInputEndPortsHead;
    this->myInputEndPortsHead = p;
}


inline void Actor::addPart (Actor* p) {
    p->next = this->myPartsHead;
    this->myPartsHead = p;
}


inline bool Actor::process () {
    bool hasMsg = false;
    this->lock();
    if (this->scanPorts()) {
        this->readPorts();
        hasMsg = true;
    }
    this->unlock();

    if (hasMsg)
        this->behavior();
    return hasMsg;
}


inline void Actor::readPorts () {
    for (PortBase* p = this->myInputEndPortsHead; p; p = p->next)
        p->read();
}


inline bool Actor::doesReceiveRemoteMsg () const {
    bool ret = false;
    for (PortBase* p = this->myInputEndPortsHead; !ret && p; p = p->next)
        ret = (ret || p->doesReceiveRemoteMsg());
    return ret;
}


inline ActorTask* Actor::createTask () {
    if (this->doesReceiveRemoteMsg() && this->hasSporadicBehavior())
        return new (tasks) RemoteActorTask(this);
    else
        return nullptr;
}


inline void Actor::build (coreid_t host) {
    this->setSID();

    for (PortBase* p = this->myInputEndPortsHead; p; p = p->next)
        p->createBuffer(host);

    this->myTask = this->createTask();

    if (!this->doesReceiveRemoteMsg()) {
        this->myMutex = new (actors) LocalMutex();
    }

    for (Actor* a = this->myPartsHead; a; a = a->next)
        a->build(host);
}


inline void Actor::start () {
    for (Actor* a = this->myPartsHead; a; a = a->next)
        a->start();

    if (myTask) {
        myTask->start();
    }
}


inline SporadicActorTask::SporadicActorTask (Actor* owner, Kernel::Stack* s, const Kernel::SporadicParams* sp, Kernel::TaskConfig cfg)
    : ActorTask(owner), myTask(*this, s, sp, cfg) {
}


inline void RemoteActorTask::signal () {
}


inline void PeriodicActorTask::activate () {
    getActor()->process();
}


inline void SporadicActorTask::activate () {
    getActor()->process();
}




} // namespace ROOM

using ROOM::PortMsgQueue;

#define START(Actor, host, target) \
    do { \
        resetSharedBuffersAllocator(); \
        ROOM::Actor* root = Actor##::create((host), (target)); \
        root->build(host); \
        root->start(); \
    } while (0);

#endif
