#include "SimpleCooldown.h"
#include <stdlib.h>


SimpleCooldown::SimpleCooldown()
{
        this->setMaxCD(0);
        this->setActualCD(0);
        this->setBeginingCooldown(0);
        this->isSetCD=false;
        this->ramdomActivated=false;
    return;
}

SimpleCooldown::SimpleCooldown(int cd)
{
        this->setMaxCD(cd);
        this->setActualCD(cd);
        this->setBeginingCooldown(cd);
        this->ramdomActivated=false;
}

SimpleCooldown::SimpleCooldown(int maxCD, int beginingCD)
{       
        this->setMaxCD(maxCD);
        this->setActualCD(beginingCD);
        this->setBeginingCooldown(beginingCD);
        this->ramdomActivated=false;
}

SimpleCooldown::SimpleCooldown(int maxCD, int beginingCD, bool randomOnReset, int rangeRandomValue, bool scaled, bool isCentered)
{
        this->setMaxCD(maxCD);
        this->setActualCD(beginingCD);
        this->setBeginingCooldown(beginingCD);
        this->ramdomActivated=randomOnReset;
        this->rangeRandomValue= scaled ? rangeRandomValue*maxCD : rangeRandomValue;
        this->rangeRandomValue= isCentered ? this->rangeRandomValue : (this->rangeRandomValue)*2;
}

void SimpleCooldown::setBeginingCooldown(int begCD)
{
    this->beginingCooldown = begCD;
}

int SimpleCooldown::getBeginingCooldown()
{
    return this->beginingCooldown;
}

int SimpleCooldown::getActualCD()
{
    return this->actual_cooldown;
}

int SimpleCooldown::getMaxCD()
{
    return this->max_cooldown;
}

void SimpleCooldown::setMaxCD(int cd)
{
    this->max_cooldown=cd;
        this->isSetCD=true;
}

void SimpleCooldown::setActualCD(int cd)
{
    this->actual_cooldown=cd;
}

bool SimpleCooldown::isReady(int diff)
{
    return this->actual_cooldown<diff;
}

void SimpleCooldown::update(int diff)
{
    this->actual_cooldown-=diff;
}

void SimpleCooldown::reinitCD()
{
        if(ramdomActivated)
        {
            this->actual_cooldown=this->getMaxCD()-((this->rangeRandomValue)/2)+(rand()%(this->rangeRandomValue));
        }
        else
        {
            this->actual_cooldown=this->getMaxCD();
        }
}

void SimpleCooldown::resetAtStart()
{
    this->setActualCD(this->getBeginingCooldown());
}

// Si on peut lancer le sort, on dit oui et on réinit. Sinon, on décrémente.
bool SimpleCooldown::CheckAndUpdate(int diff)
{
        if(!isMaxCDSet())
        {
            return false;
        }
        
    if(this->isReady(diff))
    {
        this->reinitCD();
        return true;
    }
    else
    {
        this->update(diff);
        return false;
    }
}

bool SimpleCooldown::isMaxCDSet()
{
    return isSetCD;
}

bool SimpleCooldown::CheckAndUpdate(int diff, bool additionnalCondition)
{
    if(additionnalCondition)
        return CheckAndUpdate(diff);
    else
        update(diff); // update but don't reset
    return false;
}
