#ifndef TEST_H
#define TEST_H

class Player;
class Creature;

class Testing
{
public:
    // Run Catch main. Can Throw
    static int RunCatch(int argc, char const* const* const argv);

    /*
    We currently use a workaround to get the tests to be correctly linked, else the compiler leave the .obj out
    More about this problem: 
    https://github.com/philsquared/Catch/issues/421
    https://github.com/philsquared/Catch/issues/720
    */
    static void RunAllTouches();

    //if you add one here, don't forget to add it in RunAllTouches
    static void Touch_test_auras();
    static void Touch_test_talent_priest();


    // Utility functions

    // Create a bot as player, as well a training dummy 10 meters in front of it. Spawned in testing map (id 13)
    static void PrepareCasterAndTarget(Player*& caster, Creature*& target);
    static void PrepareCasterAndTarget_cleanup(Player*& caster, Creature*& target);

};

#endif