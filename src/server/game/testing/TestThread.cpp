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
    _future = std::move(std::async(std::launch::async, [this]() { this->Run(); }));
    _state = STATE_RUNNING;
}

bool TestThread::IsStarted() const
{
    return _state > STATE_NOT_STARTED;
}

bool TestThread::IsFinished() const
{
    return _state == STATE_FINISHED;
}

void TestThread::Run()
{
    try
    {
        _testCase->_SetThread(this);
        bool setupSuccess = _testCase->_InternalSetup();
        if(!setupSuccess)
            _testCase->_Fail("Failed to setup test");

        _thisUpdateStartTimeMS = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

        _testCase->Test();
        if (_testCase->GetTestCount() == 0)
            _testCase->_Fail("No checks executed in test");
        //Yay time!
        _testCase->Celebrate();
    }
    catch (TestException e)
    {
        //test failed! nothing more to do, failure message is already handled by TestCase
    }
    catch (std::exception& e)
    { 
        //a regular exception happened (not one we triggered). Set its what in failure message
        _testCase->_FailNoException(e.what());
    }

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
    _state = STATE_RUNNING;
    WakeUp();
}

void TestThread::UpdateWaitTimer(uint32 const diff)
{
    if (!_waitTimer)
        return;
    if (_waitTimer <= diff)
        _SetWait(0);
    else
        _SetWait(_waitTimer - diff);
}

//This function will be executed while the test is running... care for racing conditions
void TestThread::WaitUntilDoneOrWaiting(std::shared_ptr<TestCase> test)
{
    std::unique_lock<std::mutex> lk(_testCVMutex);
    if (_state != STATE_RUNNING && (_waitTimer > 0 || _state == STATE_FINISHED))
        return; //no sleep to do

    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s will now WaitUntilDoneOrWaiting", test->GetName().c_str());
    _testCV.wait(lk, [this] {return  _state == STATE_FINISHED || _state == STATE_WAITING; });
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
    _state = STATE_FINISHED;
    GetTest()->_FailNoException("Test was canceled (thread)");
    WakeUp();
    //test mgr will remove the test after this since it is marked as finished
}

bool TestThread::Wait(uint32 ms)
{
    if (_state == STATE_FINISHED)
        return false;

    if (ms == 0)
        return true;

    //see _thisUpdateStartTimeMS comment
    milliseconds nowMS = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    uint32 timeSinceThisUpdateStart = (nowMS - _thisUpdateStartTimeMS).count();

    _state = STATE_WAITING;
    _SetWait(ms + timeSinceThisUpdateStart);
    WakeUp(); //wake up TestMgr will may be waiting in WaitUntilDoneOrWaiting

    //Now pause exec
    std::unique_lock<std::mutex> lk(_testCVMutex);
    _testCV.wait(lk, [this] { return _state == STATE_FINISHED || _waitTimer == 0; });
    _thisUpdateStartTimeMS = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    //Resume!
    if(_state == STATE_WAITING)
        _state = STATE_RUNNING;
    return true;
}
