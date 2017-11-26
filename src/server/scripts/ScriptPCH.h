
#ifndef SC_PRECOMPILED_H
#define SC_PRECOMPILED_H

#include <memory>
#include "CreatureAIImpl.h"
#include "Util.h"
#include "Transaction.h"
#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Creature.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Unit.h"
#include "GameObject.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Player.h"
#include "GossipDef.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "MotionMaster.h"
#include "Map.h"

#ifdef TESTS
    #include "TestCase.h"
    #include "TestPlayer.h"
    #include "ClassSpells.h"
    #include "Pet.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#endif

