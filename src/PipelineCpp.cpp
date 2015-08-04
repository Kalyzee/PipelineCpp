#include <PipelineCpp.hpp>

using namespace PipelineCpp;
using namespace std;

void ProcessingWorker::processUnits()
{
    
    while(!_stop)
    {
        for(int i=0; i<_processingUnits.size(); i++)
	    {   
	        if(_processingUnits[i]->try_lock())
	        {
	            if(_processingUnits[i]->tryLockQueues())
		        {
	                    _processingUnits[i]->execute();
		        }
		        _processingUnits[i]->unlock();
	        } 
	    }
    }

}

bool ProcessingUnit::tryLockQueues()
{
    
    bool all_locked = true;
    std::vector<bool> inLocked(_inputQueues.size(), false);
    std::vector< std::vector<bool> > outLocked(_outputQueues.size());
    for(int i=0; i<_outputQueues.size(); i++)
        outLocked[i].push_back(false);

    /* Try to lock input queues for poping and output queues for pushing. */

    for(int i=0; i<_inputQueues.size(); i++)
    {
        inLocked[i] = _inputQueues[i]->tryLockPop();
	if(!inLocked[i]) all_locked = false;
    }
    
    for(int i=0; i<_outputQueues.size(); i++)
    for(int j=0; j<_outputQueues[i].size(); j++)
    {
        outLocked[i][j] = _outputQueues[i][j]->tryLockPush();
	if(!outLocked[i][j]) all_locked = false;
    }

    if(!all_locked) // If some queues are not locked, it failed.
    {

        /* So we unlock successfully locked queues. */

        for(int i=0; i<_inputQueues.size(); i++)
	{
	    if(inLocked[i]) _inputQueues[i]->unlockPop();
	}
    
	for(int i=0; i<_outputQueues.size(); i++)
	for(int j=0; j<_outputQueues[i].size(); j++)
	{
	    if(outLocked[i][j]) _outputQueues[i][j]->unlockPush();
	}

	return false;

    }else{ // Otherwise, it worked.

        return true;

    }

}
bool ProcessingUnit::checkInputs(){

    if(_inputTypes.size() != _inputQueues.size()) return false;

    for(int i=0; i<_inputQueues.size(); i++)
        if(_inputQueues[i] == NULL) return false;

    return true;

}
    
bool ProcessingUnit::checkOutputs(){

    if(_outputTypes.size() != _outputQueues.size()) return false;

    for(int i=0; i<_outputQueues.size(); i++)
        for(int j=0; j<_outputQueues[i].size(); j++)
            if(_outputQueues[i][j] == NULL) return false;

    return true;

}

template<typename Tin, typename Tout>
void Pipeline<Tin,Tout>::check()
{
    
    for(int i=0; i<_processingUnits.size(); i++)
    {
        
        if(!_processingUnits[i]->checkInputs())
            throw PipelineException(string(string("Invalid number of inputs on '")+_processingUnits[i]->name()+string("' Processing Unit.")).c_str());
        if(!_processingUnits[i]->checkOutputs())
            throw PipelineException(string(string("Invalid number of outputs on '")+_processingUnits[i]->name()+string("' Processing Unit.")).c_str());
        
    }
    
}
