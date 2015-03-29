#ifndef SC_SIMPLECOOLDOWN_H
#define SC_SIMPLECOOLDOWN_H

// N'A PAS ENCORE ÉTÉ TESTÉ

// General warning : negative cooldown (or another value) are ALLOWED !
// So, be cautious...

class SimpleCooldown
{    
public :
    // Constructors
    SimpleCooldown(); // Only create the objectt with cd=0 and with isSetCD=false
    SimpleCooldown(int cd); // Create and initialize variables correctly
        SimpleCooldown(int maxCD, int beginingCD); //like SimpleCD(int cd) but the FIRST actualCD is different
        
        //scaled=true => rangeRandomValue is a precentage of maxCD (1=100%).
        // Moreover, randrange is, by default, centered. it means that if you put 1 at this value, and 3 to maxCD, actualCD can take from 2.5 to 3.5 values
        SimpleCooldown(int maxCD, int beginingCD, bool randomOnReset, int rangeRandomValue, bool scaled=false , bool isCentered=true);
        
        
        ////Public Methods////
        
        //get() Methods
    int getActualCD();
        int getMaxCD();
        int getBeginingCooldown();
        
        //set() Methods
    void setActualCD(int cd);
        void setMaxCD(int cd);
        void setBeginingCooldown(int BegCD);
        
        //Bolean methods
    bool isReady(int diff);
    bool CheckAndUpdate(int diff); // Check if cd is up. Is not, actual_cd goes down
    bool CheckAndUpdate(int diff, bool additionnalCondition); // Same thing but don't reset (but update) if additionnalCondition is false
        bool isMaxCDSet();
        
        //Void methods
    void update(int diff);// Goes down the actual CD
    void reinitCD();// set actualCD to maxCD
        void resetAtStart();
        
private :
        
        // Private variables
    int max_cooldown;
    int actual_cooldown;
        int beginingCooldown;
        bool isSetCD;
        int rangeRandomValue;
        bool ramdomActivated;
};

#endif