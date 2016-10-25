#ifndef TEST_H
#define TEST_H

namespace Testing
{
    // Run Catch main. Can Throw
    int RunCatch(int argc, char const* const* const argv);


    /*
    We currently use a workaround to get the tests to be correctly linked, else the compiler leave the .obj out
    More about this problem: 
    https://github.com/philsquared/Catch/issues/421
    https://github.com/philsquared/Catch/issues/720
    */
    void RunAllTouches();

    void Touch_test_auras();
}

#endif