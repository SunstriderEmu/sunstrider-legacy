/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
TODO:
 - Implement in instance_karazhan dead chess_piece count per side to allow Medivh to cheat when is loosing.(Or different terms was used as a treshold for cheat ?)
 - Set proper position on left or right site from chess board for killed units.
 - Half turn if no trigger in arc (facing external side of the board)
 - Sounds
EndScriptData */

#include "def_karazhan.h"
#include "Util.h"

#define A_FACTION               1690
#define H_FACTION               1689

#define GOSSIP_POSSES           20028 // Actually each piece has "Control <<piece name>>" as gossip option

#define NPC_ATTACK_RADIUS       6
#define AGGRO_RANGE             6
#define TRIGGER_ID              22519
#define SEARCH_RANGE            5
#define DUST_COVERED_CHEST      185119

#define BOTH 536 // TEAM_HORDE + TEAM_ALLIANCE

float playerTeleportPosition[4] = { -11107.241211, -1842.897461, 229.625198, 5.385472 };

typedef struct boardCell
{
    uint64 triggerGUID;
    uint64 pieceGUID;
    uint32 pieceEntry;
    uint8 row;
    uint8 col;
    
    void setData(uint64 _triggerGUID, uint8 _row, uint8 _col)
    {
        triggerGUID = _triggerGUID;
        row = _row;
        col = _col;
    }
    
    void reset()
    {
        pieceGUID = 0;
        pieceEntry = 0;
    }
    
    void setPiece(Creature* piece)
    {
        pieceGUID = piece->GetGUID();
        pieceEntry = piece->GetEntry();
    }
} BoardCell;

typedef enum chessOrientationType
{
    ORI_SE  = 0,    // Horde start
    ORI_SW  = 1,
    ORI_NW  = 2,    // Alliance start
    ORI_NE  = 3
} ChessOrientationType;

float orientations[4] = { 3.809080, 2.235102, 0.661124, 5.385472 };

/*
x = case (0, 0) - y = case (7, 7)
    --------------------
        TEAM_HORDE PIECES  y

X

     x  ALLY  PIECES
    --------------------

*/

enum eScriptTexts {
    SCRIPTTEXT_AT_EVENT_START   =  -1650000,
    SCRIPTTEXT_LOSE_KNIGHT_P    =  -1650001,
    SCRIPTTEXT_LOSE_KNIGHT_M    =  -1650002,
    SCRIPTTEXT_LOSE_PAWN_P_1    =  -1650003,
    SCRIPTTEXT_LOSE_PAWN_P_2    =  -1650004,
    SCRIPTTEXT_LOSE_PAWN_P_3    =  -1650005,
    SCRIPTTEXT_LOSE_PAWN_M_1    =  -1650006,
    SCRIPTTEXT_LOSE_PAWN_M_2    =  -1650007,
    SCRIPTTEXT_LOSE_PAWN_M_3    =  -1650008,
    SCRIPTTEXT_LOSE_QUEEN_P     =  -1650009,
    SCRIPTTEXT_LOSE_QUEEN_M     =  -1650010,
    SCRIPTTEXT_LOSE_BISHOP_P    =  -1650011,
    SCRIPTTEXT_LOSE_BISHOP_M    =  -1650012,
    SCRIPTTEXT_LOSE_ROOK_P      =  -1650013,
    SCRIPTTEXT_LOSE_ROOK_M      =  -1650014,
    SCRIPTTEXT_PLAYER_CHECK     =  -1650015,
    SCRIPTTEXT_MEDIVH_CHECK     =  -1650016,
    SCRIPTTEXT_PLAYER_WIN       =  -1650017,
    SCRIPTTEXT_MEDIVH_WIN       =  -1650018,
    SCRIPTTEXT_MEDIVH_CHEAT_1   =  -1650019,
    SCRIPTTEXT_MEDIVH_CHEAT_2   =  -1650020,
    SCRIPTTEXT_MEDIVH_CHEAT_3   =  -1650021
};

enum eSpells {
    BISHOP_HEAL_H  = 37456,
    BISHOP_HEAL_A  = 37455,
    SPELL_MOVE_1   = 37146, // 8y
    SPELL_MOVE_2   = 30012, // Unlimited
    SPELL_MOVE_3   = 37144, // 15y
    SPELL_MOVE_4   = 37148, // 20y
    SPELL_MOVE_5   = 37151, // 8y
    SPELL_MOVE_6   = 37152, // 8y
    SPELL_MOVE_7   = 37153, // 8y

    SPELL_CHANGE_FACING         = 30284,
    SPELL_MOVE_MARKER           = 32261,
    SPELL_POSSESS_CHESSPIECE    = 30019,
    SPELL_RECENTLY_INGAME       = 30529,
    SPELL_HAND_OF_MEDIVH        = 39339, // 1st cheat: AOE spell burn cell under enemy chesspieces.
    SPELL_FURY_OF_MEDIVH        = 39383  // 2nd cheat: Berserk own chesspieces.
    // 3rd cheat: set own creatures to max health
};

enum deadTeam
{
    DEAD_ALLIANCE   = 0,
    DEAD_HORDE      = 1
};

struct npc_echo_of_medivhAI : public ScriptedAI
{
    npc_echo_of_medivhAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    BoardCell* board[8][8];
    
    uint32 cheatTimer;
    uint32 deadCount[2];
    
    InstanceScript* pInstance;
    
    void EnterCombat(Unit* who) override {}
    
    void Reset()
    override {
        cheatTimer = 80000 + rand()%20000;
        
        if (pInstance && pInstance->GetData(DATA_CHESS_EVENT) == DONE)
            pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
    }
    
    void RemoveCheats()
    {
        // Buffs
        for (auto & row : board) {
            for (uint8 col = 0; col < 8; col++) {
                if (uint64 guid = row[col]->pieceGUID) {
                    if (Creature* piece = ObjectAccessor::GetCreature(*me, guid))
                        piece->RemoveAurasDueToSpell(39339);
                }
            }
        }
    }
    
    void SetupBoard()
    {
        if (pInstance)
            pInstance->SetData(DATA_CHESS_CHECK_PIECES_ALIVE, 0);
        // Cleanup needed?
        if (Creature* trigger = me->FindNearestCreature(TRIGGER_ID, 15.0f, true)) {
            for (auto & row : board) {
                for (uint8 col = 0; col < 8; col++) {
                    if (Creature* cellTrigger = ObjectAccessor::GetCreature(*me, row[col]->triggerGUID))
                        cellTrigger->ForcedDespawn();
                        
                    delete row[col];
                }
                
                //delete[] board[row];
            }
            
            //delete[] board;
        }

        for (uint8 row = 0; row < 8; row++) {
            for (uint8 col = 0; col < 8; col++) {
                auto  cell = new BoardCell;
                board[row][col] = cell;

                if (Creature* trigger = me->SummonCreature(TRIGGER_ID, (-11108.099609 + (3.49f * col) + (4.4f * row)), (-1872.910034 - (4.4f * col) + (3.45f * row)), 220.667f, 0, TEMPSUMMON_MANUAL_DESPAWN, 0)) {
                    cell->setData(trigger->GetGUID(), row, col);
                    HandleCellInitialData(row, col, trigger, cell);
                }
            }
        }
        
        deadCount[DEAD_ALLIANCE] = 0;
        deadCount[DEAD_HORDE] = 0;
    }
    
    void HandleCellInitialData(uint8 row, uint8 col, Creature* trigger, BoardCell* cell)
    {
        switch (row) {
        case 0: // Alliance first row
            switch (col) {
            case 0:
            case 7: // Rook
                if (Creature* rook = trigger->FindNearestCreature(NPC_ROOK_A, 40.0f, true))
                    cell->setPiece(rook);
                break;
            case 1:
            case 6: // Knight
                if (Creature* knight = trigger->FindNearestCreature(NPC_KNIGHT_A, 40.0f, true))
                    cell->setPiece(knight);
                break;
            case 2:
            case 5: // Bishop
                if (Creature* bishop = trigger->FindNearestCreature(NPC_BISHOP_A, 40.0f, true))
                    cell->setPiece(bishop);
                break;
            case 3: // Queen
                if (Creature* queen = trigger->FindNearestCreature(NPC_QUEEN_A, 40.0f, true))
                    cell->setPiece(queen);
                break;
            case 4: // King
                if (Creature* king = trigger->FindNearestCreature(NPC_KING_A, 40.0f, true))
                    cell->setPiece(king);
                break;
            }
            break;
        case 1: // Alliance second row
            // All pawns
            if (Creature* pawn = trigger->FindNearestCreature(NPC_PAWN_A, 40.0f, true))
                cell->setPiece(pawn);
            break;
        case 6: // Horde second row
            // All pawns
            if (Creature* pawn = trigger->FindNearestCreature(NPC_PAWN_H, 40.0f, true))
                cell->setPiece(pawn);
            break;
        case 7: // Horde first row
            switch (col) {
            case 0:
            case 7: // Rook
                if (Creature* rook = trigger->FindNearestCreature(NPC_ROOK_H, 40.0f, true))
                    cell->setPiece(rook);
                break;
            case 1:
            case 6: // Knight
                if (Creature* knight = trigger->FindNearestCreature(NPC_KNIGHT_H, 40.0f, true))
                    cell->setPiece(knight);
                break;
            case 2:
            case 5: // Bishop
                if (Creature* bishop = trigger->FindNearestCreature(NPC_BISHOP_H, 40.0f, true))
                    cell->setPiece(bishop);
                break;
            case 3: // Queen
                if (Creature* queen = trigger->FindNearestCreature(NPC_QUEEN_H, 40.0f, true))
                    cell->setPiece(queen);
                break;
            case 4: // King
                if (Creature* king = trigger->FindNearestCreature(NPC_KING_H, 40.0f, true))
                    cell->setPiece(king);
                break;
            }
            break;
        default:
            //TC_LOG_INFO("Default for %u %u", row, col);
            cell->reset();
        }
    }
    
    bool IsFriendlyPiece(uint32 entry)
    {
        if (!pInstance)
            return false;

        switch (entry) {
        case NPC_PAWN_H:
        case NPC_KNIGHT_H:
        case NPC_QUEEN_H:
        case NPC_BISHOP_H:
        case NPC_ROOK_H:
        case NPC_KING_H:
            return (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_ALLIANCE);
        case NPC_PAWN_A:
        case NPC_KNIGHT_A:
        case NPC_QUEEN_A:
        case NPC_BISHOP_A:
        case NPC_ROOK_A:
        case NPC_KING_A:
            return (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_HORDE);
        }

        return false;
    }
    
    /*void ReceiveEmote(Player* player, uint32 text_emote)
    {
        TC_LOG_INFO("ChessPhase %u", chessPhase);
        return;
        for (uint8 row = 0; row < 8; row++) {
            for (uint8 col = 0; col < 8; col++) {
                TC_LOG_INFO("%u %u: "I64FMTD, row, col, board[row][col]->triggerGUID);
            }
        }
    }*/
    
    Creature* GetTargetFor(Creature* piece, ChessOrientationType orientation)
    {
        Creature* target = nullptr;
        BoardCell* targetCell = nullptr;

        for (uint8 row = 0; row < 8 && !targetCell; row++) {
            for (uint8 col = 0; col < 8 && !targetCell; col++) {
                BoardCell* cell = board[row][col];
                if (cell && cell->pieceGUID == piece->GetGUID()) {
                    switch (orientation) {
                    case ORI_SE:
                        if (row != 0)
                            targetCell = board[row-1][col];
                        break;
                    case ORI_SW:
                        if (col != 0)
                            targetCell = board[row][col-1];
                        break;
                    case ORI_NW:
                        if (row != 7)
                            targetCell = board[row+1][col];
                        break;
                    case ORI_NE:
                        if (col != 7)
                            targetCell = board[row][col+1];
                        break;
                    }
                }
            }
        }
        
        if (targetCell && targetCell->pieceGUID)
            target = ObjectAccessor::GetCreature(*me, targetCell->pieceGUID);

        return target;
    }
    
    void HandlePieceDeath(Creature* piece)
    {
        switch (piece->GetFaction()) {
        case A_FACTION:
        {
            float baseX = -11078.116211;
            float baseY = -1908.443115;
            float deltaX = 2.148438;
            float deltaY = 1.723755;
            float extraX = 2.416992;
            float extraY = -2.889649;
            
            float offset = 1.3f * (deadCount[DEAD_ALLIANCE] % 8);

            float finalX = baseX + offset * deltaX + (deadCount[DEAD_ALLIANCE] >= 8 ? 1 : 0) * extraX;
            float finalY = baseY + offset * deltaY + (deadCount[DEAD_ALLIANCE] >= 8 ? 1 : 0) * extraY;
            piece->Relocate(finalX, finalY, 221, orientations[ORI_SW]);
            ++deadCount[DEAD_ALLIANCE];

            piece->CombatStop();
            piece->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            piece->SendMovementFlagUpdate();
            piece->SetDeathState(JUST_RESPAWNED);
            piece->SetHealth(piece->GetMaxHealth());
            break;
        }
        case H_FACTION:
            float baseX = -11081.393555;
            float baseY = -1844.194092;
            float deltaX = -2.148438;
            float deltaY = -1.723755;
            float extraX = -2.416992;
            float extraY = 2.889649;
            
            float offset = 1.3f * (deadCount[DEAD_HORDE] % 8);

            float finalX = baseX + offset * deltaX + (deadCount[DEAD_HORDE] >= 8 ? 1 : 0) * extraX;
            float finalY = baseY + offset * deltaY + (deadCount[DEAD_HORDE] >= 8 ? 1 : 0) * extraY;
            piece->Relocate(finalX, finalY, 221, orientations[ORI_NE]);
            ++deadCount[DEAD_HORDE];

            piece->CombatStop();
            piece->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            piece->SendMovementFlagUpdate();
            piece->SetDeathState(JUST_RESPAWNED);
            piece->SetHealth(piece->GetMaxHealth());
            break;
        }
        
        for (auto & row : board) {
            for (uint8 col = 0; col < 8; col++) {
                if (row[col]->pieceGUID == piece->GetGUID())
                    row[col]->reset();
            }
        }
        
        if (!pInstance)
            return;
        
        if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_HORDE) {
            switch (piece->GetEntry()) {
            case NPC_ROOK_H:   DoScriptText(SCRIPTTEXT_LOSE_ROOK_P, me);     break;
            case NPC_ROOK_A:   DoScriptText(SCRIPTTEXT_LOSE_ROOK_M, me);     break;
            case NPC_QUEEN_H:  DoScriptText(SCRIPTTEXT_LOSE_QUEEN_P, me);    break;
            case NPC_QUEEN_A:  DoScriptText(SCRIPTTEXT_LOSE_QUEEN_M, me);    break;
            case NPC_BISHOP_H: DoScriptText(SCRIPTTEXT_LOSE_BISHOP_P, me);   break;
            case NPC_BISHOP_A: DoScriptText(SCRIPTTEXT_LOSE_BISHOP_M, me);   break;
            case NPC_KNIGHT_H: DoScriptText(SCRIPTTEXT_LOSE_KNIGHT_P, me);   break;
            case NPC_KNIGHT_A: DoScriptText(SCRIPTTEXT_LOSE_KNIGHT_M, me);   break;
            case NPC_PAWN_H:
                DoScriptText(RAND(SCRIPTTEXT_LOSE_PAWN_P_1, SCRIPTTEXT_LOSE_PAWN_P_2, SCRIPTTEXT_LOSE_PAWN_P_3), me); break;
            case NPC_PAWN_A:
                DoScriptText(RAND(SCRIPTTEXT_LOSE_PAWN_M_1, SCRIPTTEXT_LOSE_PAWN_M_2, SCRIPTTEXT_LOSE_PAWN_M_3), me); break;
            case NPC_KING_H:
                DoScriptText(SCRIPTTEXT_MEDIVH_WIN, me);
                pInstance->SetData(DATA_CHESS_EVENT, NOT_STARTED);
                me->RemoveAurasDueToSpell(39331);
                pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                pInstance->SetData(DATA_CHESS_GAME_PHASE, NOTSTARTED);
                break;
            case NPC_KING_A:
                DoScriptText(SCRIPTTEXT_PLAYER_WIN, me);
                if (pInstance->GetData(DATA_CHESS_EVENT) == IN_PROGRESS) {
                    pInstance->SetData(DATA_CHESS_EVENT, DONE);
                    me->RemoveAurasDueToSpell(39331);
                    pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
                    me->SummonGameObject(DUST_COVERED_CHEST, Position(-11058, -1903, 221, 2.24), G3D::Quat(), 7200000);
                    pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                }
                else if (pInstance->GetData(DATA_CHESS_EVENT) == SPECIAL) {
                    pInstance->SetData(DATA_CHESS_EVENT, DONE);
                    me->RemoveAurasDueToSpell(39331);
                    pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                    pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
                }
                break;
            default: break;
            }
        }
        else if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_ALLIANCE) {
            switch(piece->GetEntry()) {
            case NPC_ROOK_A:   DoScriptText(SCRIPTTEXT_LOSE_ROOK_P, me);     break;
            case NPC_ROOK_H:   DoScriptText(SCRIPTTEXT_LOSE_ROOK_M, me);     break;
            case NPC_QUEEN_A:  DoScriptText(SCRIPTTEXT_LOSE_QUEEN_P, me);    break;
            case NPC_QUEEN_H:  DoScriptText(SCRIPTTEXT_LOSE_QUEEN_M, me);    break;
            case NPC_BISHOP_A: DoScriptText(SCRIPTTEXT_LOSE_BISHOP_P, me);   break;
            case NPC_BISHOP_H: DoScriptText(SCRIPTTEXT_LOSE_BISHOP_M, me);   break;
            case NPC_KNIGHT_A: DoScriptText(SCRIPTTEXT_LOSE_KNIGHT_P, me);   break;
            case NPC_KNIGHT_H: DoScriptText(SCRIPTTEXT_LOSE_KNIGHT_M, me);   break;
            case NPC_PAWN_A:
                DoScriptText(RAND(SCRIPTTEXT_LOSE_PAWN_P_1, SCRIPTTEXT_LOSE_PAWN_P_2, SCRIPTTEXT_LOSE_PAWN_P_3), me); break;
            case NPC_PAWN_H:
                DoScriptText(RAND(SCRIPTTEXT_LOSE_PAWN_M_1, SCRIPTTEXT_LOSE_PAWN_M_2, SCRIPTTEXT_LOSE_PAWN_M_3), me); break;
            case NPC_KING_A:
                DoScriptText(SCRIPTTEXT_MEDIVH_WIN, me);
                pInstance->SetData(DATA_CHESS_EVENT, NOT_STARTED);
                me->RemoveAurasDueToSpell(39331);
                pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                pInstance->SetData(DATA_CHESS_GAME_PHASE, NOTSTARTED);
                break;
            case NPC_KING_H:
                DoScriptText(SCRIPTTEXT_PLAYER_WIN, me);
                if (pInstance->GetData(DATA_CHESS_EVENT) == IN_PROGRESS) {
                    pInstance->SetData(DATA_CHESS_EVENT, DONE);
                    me->RemoveAurasDueToSpell(39331);
                    pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
                    me->SummonGameObject(DUST_COVERED_CHEST, Position(-11058, -1903, 221, 2.24), G3D::Quat(), 7200000);
                    pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                }
                else if (pInstance->GetData(DATA_CHESS_EVENT) == SPECIAL) {
                    pInstance->SetData(DATA_CHESS_EVENT, DONE);
                    me->RemoveAurasDueToSpell(39331);
                    pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                    pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
                }
                break;
            default: break;
            }
        }
        else {
            switch(piece->GetEntry()) {
            case NPC_KING_H:
            case NPC_KING_A:
                pInstance->SetData(DATA_CHESS_EVENT, DONE);
                me->RemoveAurasDueToSpell(39331);
                pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
                pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);
                break;
            }
        }
    }
    
    int HandlePieceRotate(Creature* piece, uint64 trigger)
    {
        uint8 pieceRow = 8, pieceCol = 8, targetRow, targetCol;
        for (uint8 row = 0; row < 8; row++) {
            for (uint8 col = 0; col < 8; col++) {
                BoardCell* cell = board[row][col];
                if (cell->triggerGUID == trigger) {
                    targetRow = row;
                    targetCol = col;
                }
                if (cell->pieceGUID == piece->GetGUID()) {
                    pieceRow = row;
                    pieceCol = col;
                }
            }
        }

        int8 deltaRow = pieceRow - targetRow;
        int8 deltaCol = pieceCol - targetCol;
        
        if (fabs((float)deltaRow) + fabs((float)deltaCol) > 1)
            return -1;

        if (deltaRow == 1 && deltaCol == 0)
            return ORI_SE;
        if (deltaRow == 0 && deltaCol == 1)
            return ORI_SW;
        if (deltaRow == -1 && deltaCol == 0)
            return ORI_NW;
        if (deltaRow == 0 && deltaCol == -1)
            return ORI_NE;

        return 0;
    }
    
    bool HandlePieceMove(Creature* piece, uint64 trigger)
    {
        bool res = false;
        bool foundOld = false;
        bool foundNew = false;
        uint8 oldCol = 8, oldRow = 8, newCol, newRow;
        for (uint8 row = 0; row < 8; row++) {
            for (uint8 col = 0; col < 8; col++) {
                BoardCell* cell = board[row][col];
                if(!foundNew)
                {
                    if (cell->triggerGUID == trigger) {
                        //is there already a piece on this cell ?
                        if (cell->pieceGUID) {
                            return false;
                        } else {
                            newCol = col;
                            newRow = row;
                            res = true;
                        }
                        foundNew = true;
                    }
                }
                if(!foundOld)
                {
                    if (cell->pieceGUID == piece->GetGUID()) {
                        oldCol = col;
                        oldRow = row;
                        foundOld = true;
                    }
                }
                if(foundNew && foundOld)
                    break;
            }
        }
        
        if (res) {
            uint8 deltaRow = fabs((float)oldRow - newRow);
            uint8 deltaCol = fabs((float)oldCol - newCol);
            switch (piece->GetEntry()) {
            case NPC_PAWN_H:
            case NPC_PAWN_A:
            case NPC_BISHOP_H:
            case NPC_BISHOP_A:
            case NPC_ROOK_H:
            case NPC_ROOK_A:
            case NPC_KING_H:
            case NPC_KING_A:
                if (deltaRow > 1 || deltaCol > 1) {
                    TC_LOG_DEBUG("misc","Target cell too far away for %s (%u) - Starting (%u, %u) - Ending (%u, %u)", piece->GetName().c_str(), piece->GetDBTableGUIDLow(), oldRow, oldCol, newRow, newCol);
                    res = false;
                }
                break;
            case NPC_QUEEN_H:
            case NPC_QUEEN_A:
                if (deltaRow > 3 || deltaCol > 3) // FIXME: not correct
                    res = false;
                break;
            case NPC_KNIGHT_H:
            case NPC_KNIGHT_A:
                if (deltaRow > 2 || deltaCol > 2) // FIXME: not correct
                    res = false;
                break;
            default:
                break;
            }
        }
        
        if (res) {
            board[newRow][newCol]->triggerGUID = trigger;
            board[newRow][newCol]->setPiece(piece);
            if (oldRow != 8 && oldCol != 8)
                board[oldRow][oldCol]->reset();
        }

        return res;
    }
    
    bool HandlePieceMoveByAI(Creature* piece, ChessOrientationType orientation)
    {
        uint8 pieceRow = 0, pieceCol = 0;
        bool found = false;
        for (uint8 row = 0; row < 8 && !found; row++) {
            for (uint8 col = 0; col < 8 && !found; col++) {
                if (board[row][col]->pieceGUID == piece->GetGUID()) {
                    pieceRow = row;
                    pieceCol = col;
                    found = true; break;
                }
            }
        }
        // Force half-turn?
        if (orientation == ORI_SE && pieceRow == 0) {
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow+1][pieceCol]->triggerGUID))
                piece->CastSpell(trigger, SPELL_CHANGE_FACING, false);
            return true;
        }
        else if (orientation == ORI_SW && pieceCol == 0) {
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow][pieceCol+1]->triggerGUID))
                piece->CastSpell(trigger, SPELL_CHANGE_FACING, false);
            return true;
        }
        else if (orientation == ORI_NW && pieceRow == 7) {
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow-1][pieceCol]->triggerGUID))
                piece->CastSpell(trigger, SPELL_CHANGE_FACING, false);
            return true;
        }
        else if (orientation == ORI_NE && pieceCol == 7) {
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow][pieceCol-1]->triggerGUID))
                piece->CastSpell(trigger, SPELL_CHANGE_FACING, false);
            return true;
        }
        else if (rand()%2) { // 1/3 chance to check near enemies
            if (pieceRow > 0 && pieceRow < 7 && pieceCol > 0 && pieceCol < 7) {
                if (Creature* neightboor = ObjectAccessor::GetCreature(*me, board[pieceRow-1][pieceCol]->pieceGUID)) {
                    if (!piece->IsFriendlyTo(neightboor)) {
                        piece->CastSpell(neightboor, SPELL_CHANGE_FACING, false);
                        return true;
                    }
                }
                else if (Creature* neightboor = ObjectAccessor::GetCreature(*me, board[pieceRow+1][pieceCol]->pieceGUID)) {
                    if (!piece->IsFriendlyTo(neightboor)) {
                        piece->CastSpell(neightboor, SPELL_CHANGE_FACING, false);
                        return true;
                    }
                }
                else if (Creature* neightboor = ObjectAccessor::GetCreature(*me, board[pieceRow][pieceCol-1]->pieceGUID)) {
                    if (!piece->IsFriendlyTo(neightboor)) {
                        piece->CastSpell(neightboor, SPELL_CHANGE_FACING, false);
                        return true;
                    }
                }
                else if (Creature* neightboor = ObjectAccessor::GetCreature(*me, board[pieceRow][pieceCol+1]->pieceGUID)) {
                    if (!piece->IsFriendlyTo(neightboor)) {
                        piece->CastSpell(neightboor, SPELL_CHANGE_FACING, false);
                        return true;
                    }
                }
            }
        }
        else if (rand()%4 != 0)
            return true;

        switch (orientation) { // Here we shouldn't be facing the edges, check in the 4 first if statements
        case ORI_SE:
        {
            int randomCol;
            switch (rand()%3) {
            case 0:
                randomCol = pieceCol-1;
                break;
            case 1:
                randomCol = pieceCol;
                break;
            case 2:
                randomCol = pieceCol+1;
                break;
            }

            if (randomCol > 7)
                randomCol = 7;
            if (randomCol < 0)
                randomCol = 0;

            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow-1][randomCol]->triggerGUID))
                piece->CastSpell(trigger, SPELL_MOVE_1, false);
            break;
            //return HandlePieceMove(piece, board[pieceRow-1][pieceCol]->triggerGUID);
        }
        case ORI_SW:
        {
            int randomRow;
            switch (rand()%3) {
            case 0:
                randomRow = pieceRow-1;
                break;
            case 1:
                randomRow = pieceRow;
                break;
            case 2:
                randomRow = pieceRow+1;
                break;
            }

            if (randomRow > 7)
                randomRow = 7;
            if (randomRow < 0)
                randomRow = 0;
                
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[randomRow][pieceCol-1]->triggerGUID))
                piece->CastSpell(trigger, SPELL_MOVE_1, false);
            break;
            //return HandlePieceMove(piece, board[pieceRow][pieceCol-1]->triggerGUID);
        }
        case ORI_NW:
        {
            int randomCol;
            switch (rand()%3) {
            case 0:
                randomCol = pieceCol-1;
                break;
            case 1:
                randomCol = pieceCol;
                break;
            case 2:
                randomCol = pieceCol+1;
                break;
            }

            if (randomCol > 7)
                randomCol = 7;
            if (randomCol < 0)
                randomCol = 0;
                
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[pieceRow+1][randomCol]->triggerGUID))
                piece->CastSpell(trigger, SPELL_MOVE_1, false);
            break;
            //return HandlePieceMove(piece, board[pieceRow+1][pieceCol]->triggerGUID);
        }
        case ORI_NE:
        {
            int randomRow;
            switch (rand()%3) {
            case 0:
                randomRow = pieceRow-1;
                break;
            case 1:
                randomRow = pieceRow;
                break;
            case 2:
                randomRow = pieceRow+1;
                break;
            }

            if (randomRow > 7)
                randomRow = 7;
            if (randomRow < 0)
                randomRow = 0;
                
            if (Creature* trigger = ObjectAccessor::GetCreature(*me, board[randomRow][pieceCol+1]->triggerGUID))
                piece->CastSpell(trigger, SPELL_MOVE_1, false);
            break;
            //return HandlePieceMove(piece, board[pieceRow][pieceCol+1]->triggerGUID);
        }
        }
        
        return true;
    }
    
    void HandleCheat()
    {
        switch (rand()%3) {
        case 0: // Heal king
        {
            if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_ALLIANCE) {
                if (Creature* king = me->FindNearestCreature(NPC_KING_H, 80.0f, true)) {
                    if (king->IsAlive())
                        king->SetHealth(king->GetMaxHealth());
                }
            }
            else if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_HORDE) {
                if (Creature* king = me->FindNearestCreature(NPC_KING_A, 80.0f, true)) {
                    if (king->IsAlive())
                        king->SetHealth(king->GetMaxHealth());
                }
            }
            
            DoScriptText(SCRIPTTEXT_MEDIVH_CHEAT_1, me);

            break;
        }
        case 1: // Fire
        {
            std::list<Creature*> targetList;
            for (auto & row : board) {
                for (uint8 col = 0; col < 8; col++) {
                    BoardCell* cell = row[col];
                    if (!cell->pieceGUID)
                        continue;
                    
                    if (IsFriendlyPiece(cell->pieceEntry))
                        continue;
                        
                    if (Creature* trigger = ObjectAccessor::GetCreature(*me, cell->triggerGUID))
                        targetList.push_back(trigger);
                }
            }
            
            Trinity::Containers::RandomResizeList(targetList, ((targetList.size() >= 3) ? 3 : targetList.size()));
            for (auto & itr : targetList)
                DoCast(itr, 39345, true);
                
            DoScriptText(SCRIPTTEXT_MEDIVH_CHEAT_2, me);

            break;
        }
        case 2: // Buff
        {                
            std::list<Creature*> targetList;
            for (auto & row : board) {
                for (uint8 col = 0; col < 8; col++) {
                    BoardCell* cell = row[col];
                    if (!cell->pieceGUID)
                        continue;
                    
                    if (!IsFriendlyPiece(cell->pieceEntry))
                        continue;
                        
                    if (Creature* piece = ObjectAccessor::GetCreature(*me, cell->pieceGUID))
                        targetList.push_back(piece);
                }
            }
            
            Trinity::Containers::RandomResizeList(targetList, 1);
            for (auto & itr : targetList)
                DoCast(itr, 39339, true);
                
            DoScriptText(SCRIPTTEXT_MEDIVH_CHEAT_3, me);

            break;
        }
        }
    }
    
    void HandleShowDebug(Creature* piece)
    {
        for (uint8 row = 0; row < 8; row++) {
            for (uint8 col = 0; col < 8; col++) {
                if (board[row][col]->pieceGUID == piece->GetGUID())
                    TC_LOG_INFO("scripts","On cell %u %u", row, col);
            }
        }
    }
    
    void UpdateAI(uint32 const diff)
    override {
        if (!pInstance)
            return;
            
        uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);
            
        if (chessPhase != INPROGRESS_PVE)
            return;
        
        if (cheatTimer <= diff) {
            HandleCheat();
            
            cheatTimer = 80000 + rand()%20000;
        }
        else
            cheatTimer -= diff;
    }
};

struct npc_chesspieceAI : public ScriptedAI
{
    npc_chesspieceAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)me->GetInstanceScript());
        
        switch (me->GetFaction()) {
        case A_FACTION:
            currentOrientation = ORI_NW;
            break;
        case H_FACTION:
            currentOrientation = ORI_SE;
            break;
        }
        
        NextMoveTimer = 4000 + rand()%1000;
        SetCombatMovementAllowed(false);
    }

    InstanceScript* pInstance;
    
    uint32 Heal_Timer;
    uint32 NextMoveTimer;
    uint32 AttackTimer;
    
    float destX, destY;
    
    ChessOrientationType currentOrientation;
    
    void EnterEvadeMode(EvadeReason /* why */)
    override {
        // Just stay in place
        me->RemoveAllAuras();
        Reset();
    }
    
    void EnterCombat(Unit* pWho) // TODO
    override {
        Unit* npc_medivh = ObjectAccessor::GetUnit(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH));
        
        if (npc_medivh) {
            switch (pInstance->GetData(CHESS_EVENT_TEAM)) {
            case TEAM_ALLIANCE:
                if (me->GetEntry() == NPC_KING_H)
                    DoScriptText(SCRIPTTEXT_MEDIVH_CHECK, npc_medivh);
                else if (me->GetEntry() == NPC_KING_A)
                    DoScriptText(SCRIPTTEXT_PLAYER_CHECK, npc_medivh);
            break;
            case TEAM_HORDE:
                if (me->GetEntry() == NPC_KING_A)
                    DoScriptText(SCRIPTTEXT_MEDIVH_CHECK, npc_medivh);
                else if (me->GetEntry() == NPC_KING_H)
                    DoScriptText(SCRIPTTEXT_PLAYER_CHECK, npc_medivh);
            break;
            }
        }
    }
    
    /*void ReceiveEmote(Player* player, uint32 text_emote)
    {
        if (Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH)))
            ((npc_echo_of_medivhAI*)medivh->AI())->HandleShowDebug(me);
    }*/
    
    void Reset()
    override {
        Heal_Timer = 7000;
        NextMoveTimer = 4000 + rand()%1000; // wait 4.5s for first moves
    
        me->ApplySpellImmune(0, IMMUNITY_ID, 39331, true);
        AttackTimer = me->GetAttackTime(BASE_ATTACK);
        me->SetSheath(SHEATH_STATE_MELEE);

        me->SetInCombatState(false); //avoid regen hp when ooc
    }
    
    void MovementInform(uint32 MovementType, uint32 Data)
    override {
        if (MovementType != POINT_MOTION_TYPE)
            return;
        
        me->SetOrientation(orientations[currentOrientation]);
        me->Relocate(destX, destY, 220.667f);
        destX = 0;
        destY = 0;
        me->SendMovementFlagUpdate();
        if (!me->IsCharmed())
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        
        AttackTimer = 1;
    }
    
    void AttackStart(Unit* who) override {}
    
    void OnCharmed(Unit* charmer, bool apply)
    override {
        if (!pInstance)
            return;

        if (!charmer || charmer->GetTypeId() != TYPEID_PLAYER)
            return;
            
        if (apply)
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        else
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_ALLIANCE)
            me->SetFaction(A_FACTION);
        else if (pInstance->GetData(CHESS_EVENT_TEAM) == TEAM_HORDE)
            me->SetFaction(H_FACTION);
        else
            me->SetFaction(me->GetCreatureTemplate()->faction);
    }
    
    void SetData(uint32 type, uint32 value, Unit* setter)
    override {
        if (type == 0) {
            float x, y, z, o;
            me->GetHomePosition(x, y, z, o);
            destX = x;
            destY = y;
        }
        else if (type == 1) {
            switch (me->GetFaction()) {
            case A_FACTION:
                currentOrientation = ORI_NW;
                break;
            case H_FACTION:
                currentOrientation = ORI_SE;
                break;
            }
        }
    }
    
    void JustDied(Unit* pKiller)
    override { 
        Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_IMAGE_OF_MEDIVH) : 0);
        ((npc_echo_of_medivhAI*)medivh->AI())->HandlePieceDeath(me);
        
        if (me->IsCharmed()) {
            if (Unit* charmer = me->GetCharmer())
                charmer->RemoveAurasDueToSpell(30019);
            me->RemoveCharmedBy(me->GetCharmer());
        }
    }
    
    void OnSpellFinish(Unit* caster, uint32 spellId, Unit* target, bool ok)
    override {
        if (spellId != SPELL_MOVE_1 && spellId != SPELL_MOVE_2 && spellId != SPELL_MOVE_3 && spellId != SPELL_MOVE_4 && 
                spellId != SPELL_MOVE_5 && spellId != SPELL_MOVE_6 && spellId != SPELL_MOVE_7 && spellId != SPELL_CHANGE_FACING) {
            me->SetOrientation(orientations[currentOrientation]);
            me->SendMovementFlagUpdate();
        }
    }
    
    void KilledUnit(Unit* victim)
    override {

    }

    void MoveInLineOfSight(Unit* who) override {}

    void UpdateAI(const uint32 diff)
    override {
        me->SetReactState(REACT_PASSIVE);
        
        if (!pInstance)
            return;
            
        uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);
            
        if (chessPhase != INPROGRESS_PVE && chessPhase != INPROGRESS_PVP)
            return;

        if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            return;
        
        if (me->IsNonMeleeSpellCast(false))
            return;

        if (AttackTimer <= diff) {
            Creature* piece = nullptr;
            if (Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH)))
                piece = ((npc_echo_of_medivhAI*)medivh->AI())->GetTargetFor(me, currentOrientation);

            if (piece && !me->IsFriendlyTo(piece))
                me->Attack(piece, false);

            AttackTimer = me->GetAttackTime(BASE_ATTACK);
        }
        else
            AttackTimer -= diff;

        if (!me->IsCharmed()) {
            if (NextMoveTimer <= diff) {
                if (Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH))) {
                    if (((npc_echo_of_medivhAI*)medivh->AI())->HandlePieceMoveByAI(me, currentOrientation))
                        NextMoveTimer = 5000 + rand()%2000;
                    else
                        NextMoveTimer = 1000 + rand()%1000;
                }
            }
            else
                NextMoveTimer -= diff;
            
            // Special handling -> Move to subclasses?
            switch(me->GetEntry()) {
            case NPC_BISHOP_A:
                if (Heal_Timer <= diff) {
                    if (Unit* target = DoSelectLowestHpFriendly(25.0f, 5000))
                        DoCast(target, BISHOP_HEAL_A);
                
                    Heal_Timer = 15000 + rand()%5000;
                }
                else
                    Heal_Timer -= diff;
                break;
            case NPC_BISHOP_H:
                if (Heal_Timer <= diff) {
                    if (Unit* target = DoSelectLowestHpFriendly(25.0f, 5000))
                        DoCast(target, BISHOP_HEAL_H);
                
                    Heal_Timer = 15000 + rand()%5000;
                }
                else
                    Heal_Timer -= diff;
                break;
            default:
                break;
            }
        }
        
        if (me->GetVictim())
            DoMeleeAttackIfReady();
    }
    
    void SpellHitTarget(Unit *target, const SpellInfo* spell)
    override {
        if (target->GetEntry() != TRIGGER_ID)
            return;

        if ((spell->Id == SPELL_MOVE_1
           || spell->Id == SPELL_MOVE_2 || spell->Id == SPELL_MOVE_3 || spell->Id == SPELL_MOVE_4
           || spell->Id == SPELL_MOVE_5 || spell->Id == SPELL_MOVE_6 || spell->Id == SPELL_MOVE_7)) {
            if (Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH))) {
                if (((npc_echo_of_medivhAI*)medivh->AI())->HandlePieceMove(me, target->GetGUID())) {
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    destX = target->GetPositionX();
                    destY = target->GetPositionY();
                    me->GetMotionMaster()->MovePoint(0, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                    target->CastSpell(target, SPELL_MOVE_MARKER, false);
                }
            }
        }
        else if (spell->Id == SPELL_CHANGE_FACING) {
            if (Creature* medivh = ObjectAccessor::GetCreature(*me, pInstance->GetData64(DATA_IMAGE_OF_MEDIVH))) {
                int result = ((npc_echo_of_medivhAI*)medivh->AI())->HandlePieceRotate(me, target->GetGUID());
                if (result != -1) {
                    me->SetOrientation(orientations[result]);
                    me->SendMovementFlagUpdate();
                    currentOrientation = ChessOrientationType(result);
                }
            }
        }
    }
};

bool GossipHello_npc_chesspiece(Player* player, Creature* creature)
{
    if (player->HasAuraEffect(SPELL_RECENTLY_INGAME))
        return true;
        
    InstanceScript* pInstance = ((InstanceScript*)creature->GetInstanceScript());
    if (!pInstance)
        return true;
        
    uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);

    if (player->GetTeam() == TEAM_ALLIANCE && creature->GetFaction() != A_FACTION && chessPhase < PVE_FINISHED)
        return true;

    if (player->GetTeam() == TEAM_HORDE && creature->GetFaction() != H_FACTION && chessPhase < PVE_FINISHED)
        return true;
        
    bool ok = true;
    
    uint32 textID = 0;

    switch (creature->GetEntry()) {
    case NPC_PAWN_H:
    case NPC_PAWN_A:
    case NPC_KNIGHT_H:
    case NPC_KNIGHT_A:
    case NPC_QUEEN_H:
    case NPC_QUEEN_A:
    case NPC_BISHOP_H:
    case NPC_BISHOP_A:
    case NPC_ROOK_H:
    case NPC_ROOK_A:
        if (chessPhase != INPROGRESS_PVE && chessPhase != INPROGRESS_PVP)
            ok = false;
            
        break;
    case NPC_KING_H:
    case NPC_KING_A:
        if (chessPhase != INPROGRESS_PVE && chessPhase != INPROGRESS_PVP && chessPhase != PVE_WARMUP && chessPhase != PVP_WARMUP)
            ok = false;
            
        break;
    default:
        ok = false;
        break;
    }

    switch (creature->GetEntry()) {
    case NPC_PAWN_H:   textID = 20021; break;
    case NPC_PAWN_A:   textID = 20027; break;
    case NPC_KNIGHT_H: textID = 20019; break;
    case NPC_KNIGHT_A: textID = 20025; break;
    case NPC_QUEEN_H:  textID = 20017; break;
    case NPC_QUEEN_A:  textID = 20023; break;
    case NPC_BISHOP_H: textID = 20018; break;
    case NPC_BISHOP_A: textID = 20023; break;
    case NPC_ROOK_H:   textID = 20020; break;
    case NPC_ROOK_A:   textID = 20026; break;
    case NPC_KING_H:   textID = 20016; break;
    case NPC_KING_A:   textID = 20028; break;
    default:           textID = 8990;  break;
    }

    if (ok) {
        player->ADD_GOSSIP_ITEM_TEXTID(0, GOSSIP_POSSES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->RemoveAurasDueToSpell(30019);
    }
    player->SEND_GOSSIP_MENU_TEXTID(textID, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_chesspiece(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1) {
        player->CastSpell(creature, SPELL_POSSESS_CHESSPIECE, false);
        player->TeleportTo(532, playerTeleportPosition[0], playerTeleportPosition[1], playerTeleportPosition[2], playerTeleportPosition[3]);
        
        InstanceScript* pInstance = ((InstanceScript*)creature->GetInstanceScript());
        if (!pInstance)
            return true;
            
        uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);
        
        if (chessPhase == PVE_WARMUP)
            pInstance->SetData(DATA_CHESS_GAME_PHASE, INPROGRESS_PVE);
        else if (chessPhase == PVP_WARMUP)
            pInstance->SetData(DATA_CHESS_GAME_PHASE, INPROGRESS_PVP);
    }

    player->CLOSE_GOSSIP_MENU();

    return true;
}

enum medivhGossipOptions
{
    MEDIVH_GOSSIP_START_PVE,
    MEDIVH_GOSSIP_RESTART,
    MEDIVH_GOSSIP_START_PVP
};

bool GossipHello_npc_echo_of_medivh(Player* player, Creature* creature)
{
    InstanceScript* pInstance = ((InstanceScript*)creature->GetInstanceScript());
    
    if (!pInstance)
        return true;
        
    uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);

    if (chessPhase == FAILED)
        pInstance->SetData(DATA_CHESS_GAME_PHASE, NOTSTARTED);
    
    if (pInstance->GetData(DATA_CHESS_EVENT) == DONE && chessPhase == NOTSTARTED)
        pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_FINISHED);

    if (chessPhase == NOTSTARTED)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nous souhaitons jouer une partie contre vous !", GOSSIP_SENDER_MAIN, MEDIVH_GOSSIP_START_PVE);

    if (chessPhase == INPROGRESS_PVE || chessPhase == INPROGRESS_PVP)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nous souhaitons recommencer.", GOSSIP_SENDER_MAIN, MEDIVH_GOSSIP_RESTART);
        
    if (chessPhase == PVE_FINISHED)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nous souhaitons jouer entre nous.", GOSSIP_SENDER_MAIN, MEDIVH_GOSSIP_START_PVP);

    player->SEND_GOSSIP_MENU_TEXTID(20015, creature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_echo_of_medivh(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    InstanceScript* pInstance = ((InstanceScript*)creature->GetInstanceScript());
    
    if (!pInstance)
        return true;
        
    uint32 chessPhase = pInstance->GetData(DATA_CHESS_GAME_PHASE);
    
    if (chessPhase < PVE_FINISHED)
        pInstance->SetData(CHESS_EVENT_TEAM, player->GetTeam());
    else
        pInstance->SetData(CHESS_EVENT_TEAM, BOTH);
    
    switch (action) {
    case MEDIVH_GOSSIP_START_PVE:
        pInstance->SetData(DATA_CHESS_GAME_PHASE, PVE_WARMUP);
        ((npc_echo_of_medivhAI*)(creature->AI()))->SetupBoard();
        pInstance->SetData(DATA_CHESS_EVENT, IN_PROGRESS);
        creature->CastSpell(creature, 39331, true);
        DoScriptText(SCRIPTTEXT_AT_EVENT_START, creature);
        break;
    case MEDIVH_GOSSIP_RESTART:
        pInstance->SetData(DATA_CHESS_GAME_PHASE, FAILED);
        pInstance->SetData(DATA_CHESS_REINIT_PIECES, 0);
        ((npc_echo_of_medivhAI*)creature->AI())->deadCount[DEAD_ALLIANCE] = 0;
        ((npc_echo_of_medivhAI*)creature->AI())->deadCount[DEAD_HORDE] = 0;
        //((npc_echo_of_medivhAI*)creature->AI())->RemoveCheats();
        if (pInstance->GetData(DATA_CHESS_EVENT) == IN_PROGRESS)
            pInstance->SetData(DATA_CHESS_EVENT, NOT_STARTED);
        else if (pInstance->GetData(DATA_CHESS_EVENT) == SPECIAL)
            pInstance->SetData(DATA_CHESS_EVENT, DONE);
        creature->RemoveAurasDueToSpell(39331);
        break;
    case MEDIVH_GOSSIP_START_PVP:
        pInstance->SetData(DATA_CHESS_GAME_PHASE, PVP_WARMUP);
        ((npc_echo_of_medivhAI*)(creature->AI()))->SetupBoard();
        pInstance->SetData(DATA_CHESS_EVENT, SPECIAL);
        creature->CastSpell(creature, 39331, true);
        break;
    default:
        TC_LOG_ERROR("scripts","Chess event: unknown action %u", action);
        break;
    }

    player->CLOSE_GOSSIP_MENU();
    
    return true;
}

struct chess_move_triggerAI : public ScriptedAI
{
    chess_move_triggerAI(Creature* c) : ScriptedAI(c) 
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SetCombatMovementAllowed(false);
    }
    
    void AttackStart(Unit* who)
    override {
        return;
    }
    
    void SpellHit(Unit* caster, const SpellInfo* spell)
    override {
        
    }
    
    void UpdateAI(uint32 const diff)
    override {
        //me->CombatStop();
    }
};

CreatureAI* GetAI_npc_chesspiece(Creature* pCreature)
{
    return new npc_chesspieceAI(pCreature);
}

CreatureAI* GetAI_npc_echo_of_medivh(Creature* pCreature)
{
    return new npc_echo_of_medivhAI(pCreature);
}

CreatureAI* GetAI_chess_move_trigger(Creature* creature)
{
    return new chess_move_triggerAI(creature);
}

void AddSC_chess_event()
{
    OLDScript* newscript;

    newscript = new OLDScript;
    newscript->Name = "npc_chesspiece";
    newscript->GetAI = &GetAI_npc_chesspiece;
    newscript->OnGossipHello = &GossipHello_npc_chesspiece;
    newscript->OnGossipSelect = &GossipSelect_npc_chesspiece;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "npc_echo_of_medivh";
    newscript->OnGossipHello = &GossipHello_npc_echo_of_medivh;
    newscript->OnGossipSelect = &GossipSelect_npc_echo_of_medivh;
    newscript->GetAI = &GetAI_npc_echo_of_medivh;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name = "chess_move_trigger";
    newscript->GetAI = &GetAI_chess_move_trigger;
    sScriptMgr->RegisterOLDScript(newscript);
}
