#include "TestThread.h"
#include "TestCase.h"

TestThread::TestThread(std::shared_ptr<TestCase> test)
    : _testCase(test), 
    _finished(false), 
    _waitTimer(0)
{
    _future = std::move(std::async(std::launch::async, [this]() { this->Run(); }));
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
    }
    catch (std::exception& e)
    { 
        if (std::string(e.what()) != EXCEPTION_MESSAGE) //make sure that wasn't some other execption
            _testCase->_FailNoException(e.what());
    }

    _finished = true;

    //unlock sleeping TestMgr if needed (we finished test)
    //TC_LOG_TRACE("tests", "Test tread with test name %s will notify 2", _testCase->GetName().c_str());
    std::lock_guard<std::mutex> lk(_m);
    _cv.notify_all();
    //TC_LOG_TRACE("tests", "Test tread with test name %s notify 2", _testCase->GetName().c_str());
}

void TestThread::UpdateWaitTimer(uint32 const diff)
{
    if (!_waitTimer)
        return;
    if (_waitTimer <= diff)
    {
        _SetWait(0);
    }
    else
    {
        _SetWait(_waitTimer - diff);
        //unlock sleeping TestMgr if needed (test is waiting)
    }
}

//This function will be executed while the test is running... care for racing conditions
void TestThread::SleepUntilDoneOrWaiting(std::shared_ptr<TestCase> test)
{
    std::unique_lock<std::mutex> lk(_m);
    if (_waitTimer > 0 || _finished)
        return; //no sleep to do

    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s will now SleepUntilDoneOrWaiting", test->GetName().c_str());
    _cv.wait(lk, [this] {return _finished || _waitTimer > 0; });
    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s has finished waiting", test->GetName().c_str());
}

void TestThread::_SetWait(uint32 ms)
{
    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s will notify 1", _testCase->GetName().c_str());
    std::lock_guard<std::mutex> lk(_m);
    _waitTimer = ms;
    _cv.notify_all();
    //TC_LOG_TRACE("test.unit_test", "Test tread with test name %s notify 1", _testCase->GetName().c_str());
}

void TestThread::Cancel()
{
    _finished = true;
    GetTest()->_FailNoException("Test was canceled (thread)");
    std::lock_guard<std::mutex> lk(_m);
    _cv.notify_all();
    //test mgr will remove the test after this since it is marked as finished
}

bool TestThread::Wait(uint32 ms)
{
    if (_finished)
        return false;

    if (ms == 0)
        return true;

    //see _thisUpdateStartTimeMS comment
    milliseconds nowMS = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    uint32 timeSinceThisUpdateStart = (nowMS - _thisUpdateStartTimeMS).count();

    _SetWait(ms + timeSinceThisUpdateStart);
    std::unique_lock<std::mutex> lk(_m);
    if (_waitTimer == 0)
        return true;

    _cv.wait(lk, [this] { return _finished || _waitTimer == 0; });
    _thisUpdateStartTimeMS = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    return true;
}