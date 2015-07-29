#ifndef PIPELINECPP_H
#define PIPELINECPP_H

#include <vector>
#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <exception>

namespace PipelineCpp
{

class PipelineException: public std::exception
{

    public:
    PipelineException(const char* reason):_reason(reason){}

    virtual const char* what() const throw()
    {
        return _reason;
    }

    protected:
    const char* _reason;

};

typedef unsigned int PToken;
typedef unsigned int QToken;

class ProcessingWorker;

class Queue{

    public:
    
    Queue(unsigned int maxSize = 1):_maxSize(maxSize),_notEmpty_locked(true),_spaceLeft_locked(false)
    {
        _notEmpty.lock();
    }
    virtual ~Queue(){_notEmpty.unlock();}

    /* Preemptive push-locking method. */
    bool tryLockPush(){ 
        return _spaceLeft.try_lock();
    }

    /* Preemptive pop-locking method. */
    bool tryLockPop(){
        return _notEmpty.try_lock();
    }

    /* Unlock Push method */
    void unlockPush(){
        _spaceLeft.unlock();
    }

    /* Unlock Pop method */
    void unlockPop(){
        _notEmpty.unlock();
    }

    protected:

    boost::mutex _accessmutex;
    unsigned int _maxSize;
    boost::mutex _notEmpty;
    boost::mutex _spaceLeft;
    bool _notEmpty_locked;
    bool _spaceLeft_locked;

};


template<typename T>
class ConcreteQueue: public Queue{

    public:
    
    ConcreteQueue(unsigned int maxSize = 1): Queue(maxSize){}
    ~ConcreteQueue(){}
    
    /* Standard push method */
    void push(T resource)
    {

        _spaceLeft.lock(); //Wait until there's space left on the queue (if there isn't).

        _accessmutex.lock(); // Lock access to the queue.
        _queue.push(resource); // Safely push the resource.

        if(_notEmpty_locked)
	{
            _notEmpty.unlock(); // No that there's something more in the queue, we're sure it's not empty.
            _notEmpty_locked = false;
        }

        if(_queue.size()<_maxSize) _spaceLeft.unlock(); // If there is space left in the queue, others can push in it.
        else _spaceLeft_locked = true;

        _accessmutex.unlock(); // Unlock access to the queue

    }
    
    /* Standard pop method. */
    T pop()
    {   

        _notEmpty.lock();

        _accessmutex.lock(); //Wait until there's space left on the queue (if there isn't).
	T resource = _queue.front(); // Lock access to the queue.
	_queue.pop(); // Safely pop the resource.

        if(_spaceLeft_locked){
            _spaceLeft.unlock(); // No that there's something less in the queue, we're sure it's not full.
            _spaceLeft_locked = false;
        }

        if(_queue.size()>0) _notEmpty.unlock(); // If there is resource left in the queue, others can pop from it.
        else _notEmpty_locked = true;

	_accessmutex.unlock(); // Unlock access to the queue

	return resource;

    }

    /* Nonblocking push method. */
    bool tryPush(T resource)
    {

        if(!_spaceLeft.try_lock()) return false; //Wait until there's space left on the queue (if there isn't).

        _accessmutex.lock(); // Lock access to the queue.
        _queue.push(resource); // Safely push the resource.

        if(_notEmpty_locked)
	{
            _notEmpty.unlock(); // No that there's something more in the queue, we're sure it's not empty.
            _notEmpty_locked = false;
        }

        if(_queue.size()<_maxSize) _spaceLeft.unlock(); // If there is space left in the queue, others can push in it.
        else _spaceLeft_locked = true;

        _accessmutex.unlock(); // Unlock access to the queue

	return true;

    }
    
    /* Nonblocking pop method. */
    bool tryPop(T& resource)
    {

        if(!_notEmpty.try_lock()) return false;

        _accessmutex.lock(); //Wait until there's space left on the queue (if there isn't).
	resource = _queue.front(); // Lock access to the queue.
	_queue.pop(); // Safely pop the resource.

        if(_spaceLeft_locked){
            _spaceLeft.unlock(); // No that there's something less in the queue, we're sure it's not full.
            _spaceLeft_locked = false;
        }

        if(_queue.size()>0) _notEmpty.unlock(); // If there is resource left in the queue, others can pop from it.
        else _notEmpty_locked = true;

	_accessmutex.unlock(); // Unlock access to the queue

	return true;

    }

    /* Push method when already locked (after preemptive lock). */
    void pushLocked(T resource){

        _accessmutex.lock(); // Lock access to the queue.
        _queue.push(resource); // Safely push the resource.

        if(_notEmpty_locked)
	{
            _notEmpty.unlock(); // No that there's something more in the queue, we're sure it's not empty.
            _notEmpty_locked = false;
        }

        if(_queue.size()<_maxSize) _spaceLeft.unlock(); // If there is space left in the queue, others can push in it.
        else _spaceLeft_locked = true;

        _accessmutex.unlock(); // Unlock access to the queue

    }

    /* Pop method when already locked (after preemptive lock). */
    T popLocked(){

        _accessmutex.lock(); //Wait until there's space left on the queue (if there isn't).
	T resource = _queue.front(); // Lock access to the queue.
	_queue.pop(); // Safely pop the resource.

        if(_spaceLeft_locked){
            _spaceLeft.unlock(); // No that there's something less in the queue, we're sure it's not full.
            _spaceLeft_locked = false;
        }

        if(_queue.size()>0) _notEmpty.unlock(); // If there is resource left in the queue, others can pop from it.
        else _notEmpty_locked = true;

	_accessmutex.unlock(); // Unlock access to the queue

	return resource;

    }

    protected:

    std::queue<T> _queue;

};

class ProcessingUnit{

    public:
    ProcessingUnit(){}
    virtual ~ProcessingUnit(){}

    virtual void execute() = 0;

    bool tryLockQueues();

    template<typename Tin>
    void inQueue(Queue* q){

        ConcreteQueue<Tin>* cq = dynamic_cast< ConcreteQueue<Tin>* >(q);
        if(cq!=0)
            _inputQueues.push_back(q);
        else
            std::cout << "Error: Base Queue object cannot be downcast to this concrete queue type." << std::endl;

    }

    template<typename Tout>
    void outQueue(Queue* q){

        ConcreteQueue<Tout>* cq = dynamic_cast< ConcreteQueue<Tout>* >(q);
        if(cq!=0)
            _outputQueues.push_back(q);
	    else
            std::cout << "Error: Base Queue object cannot be downcast to this concrete queue type." << std::endl;

    }

    template<typename Tin>
    Tin popIn(QToken qk){

        return static_cast<PipelineCpp::ConcreteQueue<Tin>*>(_inputQueues[qk])
        ->popLocked();

    }

    template<typename Tout>
    void pushOut(QToken qk, Tout resource){

        static_cast<PipelineCpp::ConcreteQueue<Tout>*>(_outputQueues[qk])
        ->pushLocked(resource);

    }
    
    void lock(){
        _workingmutex.lock();
    }

    void unlock(){
        _workingmutex.unlock();
    }

    bool try_lock(){
        return _workingmutex.try_lock();
    }

    protected:

    std::vector<Queue*> _inputQueues;
    std::vector<Queue*> _outputQueues;
    boost::mutex _workingmutex;

};

class ConcreteProcessingUnit: public ProcessingUnit{

    public:
    ConcreteProcessingUnit(){}
    ~ConcreteProcessingUnit(){}

    virtual void execute() = 0;

};

class ProcessingWorker{

    public:

    ProcessingWorker(std::vector<ProcessingUnit*>& processingUnits, unsigned int id):
    _processingUnits(processingUnits), _id(id), _stop(false){}

    ~ProcessingWorker(){}

    void processUnits();

    void start(){_thread = new boost::thread(&ProcessingWorker::processUnits, this);}
    void join(){_stop = true; _thread->join();}

    protected:

    std::vector<ProcessingUnit*>& _processingUnits;
    bool _stop;
    boost::thread* _thread;
    unsigned int _id;

};

template<typename Tin, typename Tout>
class Pipeline{

    public:

    Pipeline(unsigned int nbWorkers = 1): _nbWorkers(nbWorkers)
    {
        _inputQueue = new ConcreteQueue<Tin>();
	_outputQueue = new ConcreteQueue<Tout>();
    }
    ~Pipeline(){}

    template<typename PType>
    PToken create(){
        
        ProcessingUnit* pu = dynamic_cast<ProcessingUnit*>(new PType());
        if(pu==0)
	    throw PipelineException("Invalid processing unit (not derived from ProcessingUnit class).");
	
        _processingUnits.push_back(pu);

        return _processingUnits.size()-1;

    }

    template<typename T>
    void plug(PToken tk1, PToken tk2)
    {

        ConcreteQueue<T> * cqueue = new ConcreteQueue<T>();
	
        _queues.push_back(cqueue);

        _processingUnits[tk2]->inQueue<T>(cqueue);
        _processingUnits[tk1]->outQueue<T>(cqueue);

    }

    void plugInput(PToken tk)
    {
        _processingUnits[tk]->inQueue<Tin>(_inputQueue);
    }

    void plugOutput(PToken tk)
    {
        _processingUnits[tk]->outQueue<Tout>(_outputQueue);
    }

    void push(Tin resource)
    {
        _inputQueue->push(resource);
    }

    Tout pop()
    {
        return _outputQueue->pop();
    }

    Tout feed(Tin resource)
    {

        Tout result;
        push(resource);

	for(int i=0; i<_nbWorkers; i++)
	    _workers.push_back(ProcessingWorker(_processingUnits, (unsigned int) i));

	for(int i=0; i<_nbWorkers; i++)
	    _workers[i].start();

        result = pop();

	for(int i=0; i<_nbWorkers; i++)
	    _workers[i].join();
	
	return result;

    }

    protected:

    unsigned int _nbWorkers;
    ConcreteQueue<Tin>* _inputQueue;
    ConcreteQueue<Tout>* _outputQueue;
    std::vector< ProcessingWorker > _workers;
    std::vector<ProcessingUnit*> _processingUnits;
    std::vector<Queue*> _queues;

};

}

#endif	/* PIPELINECPP_H */
