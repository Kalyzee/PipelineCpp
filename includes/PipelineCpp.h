 #ifndef PIPELINECPP_H
#define PIPELINECPP_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>
#include <typeinfo>

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
typedef unsigned int Qid;

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

    virtual std::string type() = 0;
    
    protected:

    std::mutex _accessmutex;
    unsigned int _maxSize;
    std::mutex _notEmpty;
    std::mutex _spaceLeft;
    bool _notEmpty_locked;
    bool _spaceLeft_locked;

};


template<typename T>
class ConcreteQueue: public Queue{

    public:
    
    ConcreteQueue(unsigned int maxSize = 1): Queue(maxSize){}
    ~ConcreteQueue(){}
    
    virtual std::string type(){ return typeid(T).name(); }
    
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
    ProcessingUnit(std::string name):_name(name){}
    virtual ~ProcessingUnit(){}

    virtual void execute() = 0;
    
    bool checkInputs(){ return (_inputTypes.size() == _inputQueues.size()); }
    
    bool checkOutputs(){ return (_outputTypes.size() == _outputQueues.size()); }

    bool tryLockQueues();
    
    std::string name(){ return _name; }
    
    template<typename Tin>
    void addInType(){ _inputTypes.push_back(typeid(Tin).name()); }

    template<typename Tout>
    void addOutType(){ _outputTypes.push_back(typeid(Tout).name()); }
    
    std::string inType(Qid in)
    {
        if(in<_inputTypes.size())
            return _inputTypes[in];
        else
            throw PipelineException("Input queue id out of bounds.");
    }
    
    std::string outType(Qid out)
    {
        if(out<_outputTypes.size())
            return _outputTypes[out];
        else
            throw PipelineException("Output queue id out of bounds.");
    }

    template<typename Tin>
    void inQueue(Queue* q){

        ConcreteQueue<Tin>* cq = dynamic_cast< ConcreteQueue<Tin>* >(q);
        if(cq!=0)
            _inputQueues.push_back(q);
        else
            throw PipelineException("Base Queue object cannot be downcast to this concrete queue type.");

    }

    template<typename Tout>
    void outQueue(Queue* q){

        ConcreteQueue<Tout>* cq = dynamic_cast< ConcreteQueue<Tout>* >(q);
        if(cq!=0)
            _outputQueues.push_back(q);
	    else
            throw PipelineException("Base Queue object cannot be downcast to this concrete queue type.");

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

    std::string _name;
    std::vector<Queue*> _inputQueues;
    std::vector<std::string> _inputTypes;
    std::vector<Queue*> _outputQueues;
    std::vector<std::string> _outputTypes;
    std::mutex _workingmutex;

};

class ProcessingWorker{

    public:

    ProcessingWorker(std::vector<ProcessingUnit*>& processingUnits, unsigned int id):
    _processingUnits(processingUnits), _id(id), _stop(false){}

    ~ProcessingWorker(){}

    void processUnits();

    void start(){_stop = false; _thread = new std::thread(&ProcessingWorker::processUnits, this);}
    void join(){_stop = true; _thread->join();}

    protected:

    std::vector<ProcessingUnit*>& _processingUnits;
    bool _stop;
    std::thread* _thread;
    unsigned int _id;

};

template<typename Tin, typename Tout>
class Pipeline{

    public:

    Pipeline(unsigned int nbWorkers = 1): _nbWorkers(nbWorkers)
    {
        _inputQueue = new ConcreteQueue<Tin>();
	_outputQueue = new ConcreteQueue<Tout>();

	for(int i=0; i<_nbWorkers; i++)
	    _workers.push_back(ProcessingWorker(_processingUnits, (unsigned int) i));

    }
    ~Pipeline(){}

    template<typename PType>
    PToken insert(PType* p){
        
        ProcessingUnit* pu = dynamic_cast<ProcessingUnit*>(p);
        if(pu==0)
	        throw PipelineException("Invalid processing unit (not derived from ProcessingUnit class).");
	
        _processingUnits.push_back(pu);

        return _processingUnits.size()-1;

    }

    template<typename PType>
    PToken create(){
        
        return insert<PType>(new PType());
        
    }

    template<typename T>
    void plug(PToken tk1, Qid out, PToken tk2, Qid in)
    {

        ConcreteQueue<T> * cqueue;
        if(_processingUnits[tk1]->outType(out) == _processingUnits[tk2]->inType(in) && _processingUnits[tk1]->outType(out) == typeid(T).name())
            cqueue = new ConcreteQueue<T>();
        else throw PipelineException("Non matching queue types.");
	
        _queues.push_back(cqueue);

        _processingUnits[tk2]->inQueue<T>(cqueue);
        _processingUnits[tk1]->outQueue<T>(cqueue);

    }

    void plugInput(PToken tk)
    {
        if(_processingUnits[tk]->outType(0) == typeid(Tin).name())
            _processingUnits[tk]->inQueue<Tin>(_inputQueue);
        else throw PipelineException("Non matching queue types.");
    }

    std::string inputType(){ return typeid(Tin).name(); }
    
    void plugOutput(PToken tk)
    {
        if(_processingUnits[tk]->inType(0) == typeid(Tout).name())
            _processingUnits[tk]->outQueue<Tout>(_outputQueue);
        else throw PipelineException("Non matching queue types.");
    }

    std::string outputType(){ return typeid(Tout).name(); }
    
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
            _workers[i].start();

        result = pop();

        for(int i=0; i<_nbWorkers; i++)
	    _workers[i].join();
	
        return result;

    }

    void check();
    
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
