#ifndef TEST_H
#define TEST_H

class Player;
class Creature;

/*
Tests instances are defined in scripts project. 
We use dummy "touches" fonctions in each tests files so that they are correctly linked, else the compiler leave the .obj out
More about this problem: 
https://github.com/philsquared/Catch/issues/421
https://github.com/philsquared/Catch/issues/720

*/
namespace Testing
{
    // Run Catch main. Can Throw
    int RunCatch(int argc, char const* const* const argv);
    void ClearAllTests();
};

#endif