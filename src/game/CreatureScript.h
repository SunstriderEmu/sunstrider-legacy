
#ifndef WR_CREATURESCRIPT_H
#define WR_CREATURESCRIPT_H

#include "SharedDefines.h"

#include <string>

class Creature;
class CreatureAINew;
class Player;

class CreatureScriptWR
{
    public:
        CreatureScriptWR(std::string name) : m_name(name)//, OnGossipHello(0), OnGossipSelect(0)
        {}
        
        std::string getName() { return m_name; }
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_CREATURE; }
        
        virtual CreatureAINew* getAI(Creature* creature) { return NULL; }
    
        //Methods to be scripted
        bool (*OnGossipHello         )(Player*, Creature*);
        bool (*OnGossipSelect        )(Player*, Creature*, uint32 , uint32 );
    protected:
        std::string m_name;
};

#endif
