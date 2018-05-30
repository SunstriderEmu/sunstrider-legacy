#include "TestThread.h"
#include "TestCase.h"

TestThread::TestThread(std::shared_ptr<TestCase> test)
    : _testCase(test), 
    _state(STATE_NOT_STARTED),
    _waitTimer(0)
{
}

void TestThread::Start()
{
    _state = STATE_STARTED;
    _future = std::move(std::async(std::launch::async, [this]() { this->Run(); }));
}

bool TestThread::IsStarted() const
{
    return _state >= STATE_STARTED; 
}

bool TestThread::IsFinished() const
{
    return _state == STATE_FINISHED;
}

bool TestThread::IsCanceling() const
{
    return _state == STATE_CANCELING;
}

bool TestThread::IsPaused() const
{
    return _state == STATE_PAUSED; 
}

void TestThread::Run()
{
    _testCase->_SetThread(this);
    try
    {
        bool setupSuccess = _testCase->_InternalSetup();
        if(!setupSuccess)
            _testCase->_Fail("Failed to setup test");
        
        _state = STATE_READY;

        { //Test will actually start the test when map resume execution for the first time
            std::unique_lock<std::mutex> lk(_testCVMutex);
            _testCV.wait(lk, [this] {return _state > STATE_READY; });
        }

        _thisUpdateStartTimeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        _testCase->Test();
        if (_testCase->GetTestCount() == 0)
            _testCase->_Fail("No checks executed in test");
        //Yay time!
        _testCase->Celebrate();
    }
    catch (TestException e)
    {
        _testCase->Sadness();
        //test failed! nothing more to do, failure message is already handled by TestCase
    }
    catch (std::exception& e)
    { 
        //a regular exception happened (not one we triggered). Set its what in failure message
        _testCase->_FailNoException(e.what());
    }

    _testCase->_Cleanup();
    _state = STATE_FINISHED;

    //unlock sleeping TestMgr if needed (we finished test)
    //TC_LOG_TRACE("tests", "Test tread with test name %s will notify 2", _testCase->GetName().c_str());
    WakeUp();
    //TC_LOG_TRACE("tests", "Test tread with test name %s notify 2", _testCase->GetName().c_str());
}

void TestThread::WakeUp()
{
    std::lock_guard<std::mutex> lk(_testCVMutex);
    _testCV.notify_all();
}

void TestThread::ResumeExecution()
{
    ASSERT(_state != STATE_FINISHED && _state != STATE_CANCELING);
    _thisUpdateStartTimeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    //resume execution if wait finished
    if (_waitTimer == 0 || _state == STATE_PAUSED)
    {
        _state = STATE_RUNNING;
        WakeUp();
    }
}

void TestThread::UpdateWaitTimer(uint32 const mapDiff)
{
    TestMap const* testMap = _testCase->GetMap();
    if (!testMap)
        return; //test may not be setup yet
    
    if (!_waitTimer)
        return;
    if (_waitTimer <= mapDiff)
        _SetWait(0);
    else
        _SetWait(_waitTimer - mapDiff);
}

//This function will be executed while the test is running... care for racing conditions
void TestThread::WaitUntilDoneOrWaiting(std::shared_ptr<TestCase> test)
{
    std::unique_lock<std::mutex> lk(_testCVMutex);
    if (_state != STATE_RUNNING && (_waitTimer > 0 || _state == STATE_FINISHED || _state == STATE_CANCELING))
        return; //no sleep to do

    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s will now WaitUntilDoneOrWaiting", test->GetName().c_str());
    _testCV.wait(lk, [this] {return  _state == STATE_FINISHED || _state == STATE_CANCELING || _state == STATE_WAITING || _state == STATE_PAUSED; });
    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s has finished waiting", test->GetName().c_str());
}

void TestThread::_SetWait(uint32 ms)
{
    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s : _SetWait %u", _testCase->GetName().c_str(), ms);
    std::lock_guard<std::mutex> lk(_testCVMutex);
    _waitTimer = ms;
}

void TestThread::Cancel()
{
    _state = STATE_CANCELING;
    GetTest()->_FailNoException("Test was canceled (thread)");
    WakeUp();
    //test mgr will remove the test after this since it is marked as finished
}

void TestThread::Wait(uint32 ms)
{
    if (_state == STATE_FINISHED || _state == STATE_CANCELING)
        throw TestException();

    if (ms == 0)
        return;

    _state = STATE_WAITING;
    _SetWait(ms);
    WakeUp(); //wake up TestMgr which may be waiting in WaitUntilDoneOrWaiting

    //Now pause exec
    std::unique_lock<std::mutex> lk(_testCVMutex);
    _testCV.wait(lk, [this] { return _state == STATE_FINISHED || _state == STATE_CANCELING || _waitTimer == 0; });

    //Resume!
    if(_state == STATE_WAITING)
        _state = STATE_RUNNING;
}

void TestThread::HandleThreadPause()
{
    if (_state == STATE_CANCELING)  //Test was canceled. Message already set, just throw to exit test
        throw TestException();

    if (_state != STATE_RUNNING)
        return;

    //pause if we used too much time in this update
    milliseconds const nowMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds const elapsedTimeMS = nowMS - _thisUpdateStartTimeMS;
    if (elapsedTimeMS.count() > MAX_UPDATE_TIME_MS)
    {
        _state = STATE_PAUSED;
        WakeUp(); //wake up TestMgr which may be waiting in WaitUntilDoneOrWaiting

        //Now pause exec
        std::unique_lock<std::mutex> lk(_testCVMutex);
        _testCV.wait(lk, [this] { return _state != STATE_PAUSED; });
    }
}