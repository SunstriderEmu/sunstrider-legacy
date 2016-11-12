/* ScriptData
SDName: Scholomance
SD%Complete: 100
SDComment: Entrance lever...
SDCategory: Scholomance
EndScriptData */


#include "def_scholomance.h"

class ScholomanceTorch : public GameObjectScript
{
public:
    ScholomanceTorch() : GameObjectScript("go_scholo_torch")
    {}

    bool OnGossipHello(Player* pPlayer, GameObject* _GO) override
    {
        if (GameObject *door = pPlayer->FindNearestGameObject(174626, 10.0f)) {
            door->UseDoorOrButton();

            return true;
        }

        return false;
    }
};

void AddSC_scholomance() {
    new ScholomanceTorch();
}
