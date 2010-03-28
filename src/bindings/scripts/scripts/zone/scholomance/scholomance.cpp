/* ScriptData
SDName: Scholomance
SD%Complete: 100
SDComment: Entrance lever...
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

/*######
## go_scholo_torch
######*/

bool GOHello_go_scholo_torch(Player *pPlayer, GameObject* pGo)
{
    if (GameObject *door = pPlayer->FindGOInGrid(174626, 10.0f)) {
        door->UseDoorOrButton();
        
        return true;
    }
    
    return false;
}

void AddSC_scholomance() {
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "go_scholo_torch";
    newscript->pGOHello = &GOHello_go_scholo_torch;
    newscript->RegisterSelf();
}
