/* ScriptData
SDName: Instance_Blackrock_Spire
SD%Complete: n/a
SDComment:
SDCategory: Blackrock Depths
EndScriptData */


#include "def_blackrock_spire.h"
#include "GameEventMgr.h"

enum BlackrockSpireData
{
    MAX_ENCOUNTER           = 1,
};

//dunno what is the status of this but this wasn't used

#ifdef DISABLED
class instance_blackrock_spire : public InstanceMapScript
{
public:
    instance_blackrock_spire() : InstanceMapScript("instance_blackrock_spire", FIXME) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_blackrock_spire_script(map);
    }

    struct instance_blackrock_spire_script : public InstanceScript
    {
        instance_blackrock_spire_script(Map* map) : InstanceScript(map) {}

        uint32 Encounters[MAX_ENCOUNTER];

        uint64 firstDoorGUID;

        uint64 runesGUID[7];

        bool isEventActive()
        {
            const GameEventMgr::ActiveEvents& activeEvents = sGameEventMgr->GetActiveEventList();
            bool active = activeEvents.find(57) != activeEvents.end();

            return active;
        }

        void Initialize()
            override {
            firstDoorGUID = 0;

            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                Encounters[i] = NOT_STARTED;
        }

        void RefreshData() {
            if (Encounters[0] == DONE)
                HandleGameObject(firstDoorGUID, true);
        }

        bool IsEncounterInProgress() const override
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i) {
                if (Encounters[i] == IN_PROGRESS)
                    return true;
            }

            return false;
        }

        void OnCreatureCreate(Creature *creature, uint32 entry) override
        {
            switch (entry) {
            case 10899:
                if (isEventActive())
                    creature->SetDisplayId(15760);

                break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
            case 175194:    runesGUID[0] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175195:    runesGUID[1] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175196:    runesGUID[2] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175197:    runesGUID[3] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175198:    runesGUID[4] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175199:    runesGUID[5] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 175200:    runesGUID[6] = go->GetGUID(); go->UseDoorOrButton(); break;
            case 184247:    firstDoorGUID = go->GetGUID(); if (GetData(DATA_FIRSTROOM_RUNES) == DONE) HandleGameObject(firstDoorGUID, true); break;
            default: break;
            }
        }

        uint32 GetData(uint32 identifier) const override
        {
            switch (identifier)
            {
            case DATA_FIRSTROOM_RUNES:  return Encounters[0];
            default: break;
            }

            return 0;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type) {
            case DATA_FIRSTROOM_RUNES:
                Encounters[DATA_FIRSTROOM_RUNES] = data;
                if (data == DONE)
                    HandleGameObject(firstDoorGUID, true);
                break;
            default: break;
            }

            if (data == DONE)
                SaveToDB();
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream stream;
            stream << Encounters[0]/* << " "  << Encounters[1] << " "  << Encounters[2] << " "  << Encounters[3] << " "
                << Encounters[4] << " "  << Encounters[5] << " "  << Encounters[6] << " "  << Encounters[7] << " "
                << Encounters[8] << " "  << Encounters[9] << " "  << Encounters[10] << " "  << Encounters[11]*/;

            OUT_SAVE_INST_DATA_COMPLETE
                return stream.str();
        }

        void Load(const char* in) override
        {
            if (!in) {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);
            std::istringstream stream(in);
            stream >> Encounters[0]/* >> Encounters[1] >> Encounters[2] >> Encounters[3]
                >> Encounters[4] >> Encounters[5] >> Encounters[6] >> Encounters[7]
                >> Encounters[8] >> Encounters[9] >> Encounters[10] >> Encounters[11]*/;

            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i) {
                if (Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                    Encounters[i] = NOT_STARTED;
            }

            //RefreshData();
            SetData(DATA_FIRSTROOM_RUNES, DONE);

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        bool CheckRunes()
        {
            Player* player = GetPlayer();

            if (!player)
                return false;

            for (uint8 i = 0; i < 7; i++) {
                if (GameObject* rune = GameObject::GetGameObject(*player, runesGUID[i])) {
                    float dist = (i == 5) ? 3.0f : 5.0f;
                    if (Creature* cre = rune->FindNearestCreature(9819, dist, true))
                        return false;
                    if (Creature* cre = rune->FindNearestCreature(9818, dist, true))
                        return false;
                }
            }

            return true;
        }

        void Update(uint32 diff) override
        {
            if (!GetPlayer())
                return;

            if (CheckRunes())
                SetData(DATA_FIRSTROOM_RUNES, DONE);
        }
    };
};
#endif
void AddSC_instance_blackrock_spire()
{
    //new instance_blackrock_spire();
}
