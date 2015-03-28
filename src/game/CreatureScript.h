
#ifndef WR_CREATURESCRIPT_H
#define WR_CREATURESCRIPT_H

#include "SharedDefines.h"

#include <string>

class Creature;
class CreatureAINew;
class Player;

class CreatureScript
{
    public:
        CreatureScript(std::string name) : m_name(name)//, pGossipHello(0), pGossipSelect(0), pGossipSelectWithCode(0)
        {}
        
        std::string getName() { return m_name; }
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_CREATURE; }
        
        virtual CreatureAINew* getAI(Creature* creature) { return NULL; }
    
        //Methods to be scripted
        bool (*pGossipHello         )(Player*, Creature*);
        bool (*pGossipSelect        )(Player*, Creature*, uint32 , uint32 );
        bool (*pGossipSelectWithCode)(Player*, Creature*, uint32 , uint32 , const char* );
    protected:
        std::string m_name;
};

#endif
