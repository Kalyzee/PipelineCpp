#include <PipelineCpp.h>

using namespace PipelineCpp;

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
    std::vector<bool> outLocked(_outputQueues.size(), false);

    /* Try to lock input queues for poping and output queues for pushing. */

    for(int i=0; i<_inputQueues.size(); i++)
    {
        inLocked[i] = _inputQueues[i]->tryLockPop();
	if(!inLocked[i]) all_locked = false;
    }
    
    for(int i=0; i<_outputQueues.size(); i++)
    {
        outLocked[i] = _outputQueues[i]->tryLockPush();
	if(!outLocked[i]) all_locked = false;
    }

    if(!all_locked) // If some queues are not locked, it failed.
    {

        /* So we unlock successfully locked queues. */

        for(int i=0; i<_inputQueues.size(); i++)
	{
	    if(inLocked[i]) _inputQueues[i]->unlockPop();
	}
    
	for(int i=0; i<_outputQueues.size(); i++)
	{
	    if(outLocked[i]) _outputQueues[i]->unlockPush();
	}

	return false;

    }else{ // Otherwise, it worked.

        return true;

    }

}
