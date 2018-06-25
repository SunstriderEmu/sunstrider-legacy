#ifndef TESTDEFINES_H
#define TESTDEFINES_H

enum TestStatus
{
    //Test section is completed and should pass. A failure means a regression.
    STATUS_PASSING,
    //Test section is completed, but failure is expected.
    STATUS_KNOWN_BUG,
    //Test section is not completed but should pass, it still miss some features (handled as STATUS_PASSING by the core, this is just a way to mark test as "need to be refined")
    STATUS_PASSING_INCOMPLETE, //legacy, remove when global test status are removed
    //Test section is not yet completed and will be ignored unless test was directly called
    STATUS_WIP,
};

#define TEST_CREATURE_ENTRY 8
#define TEST_BOSS_ENTRY 28

#endif