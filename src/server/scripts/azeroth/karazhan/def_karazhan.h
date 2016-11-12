/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_KARAZHAN_H
#define DEF_KARAZHAN_H

#define DATA_ATTUMEN_EVENT               1
#define DATA_MOROES_EVENT                2
#define DATA_MAIDENOFVIRTUE_EVENT        3
#define DATA_OPTIONAL_BOSS_EVENT         4
#define DATA_OPERA_EVENT                 5
#define DATA_CURATOR_EVENT               6
#define DATA_SHADEOFARAN_EVENT           7
#define DATA_TERESTIAN_EVENT             8
#define DATA_NETHERSPITE_EVENT           9
#define DATA_CHESS_EVENT                10
#define DATA_MALCHEZZAR_EVENT           11
#define DATA_NIGHTBANE_EVENT            12
#define DATA_OPERA_PERFORMANCE          13
#define DATA_OPERA_OZ_DEATHCOUNT        14
#define DATA_KILREK                     15
#define DATA_TERESTIAN                  16
#define DATA_MOROES                     17
#define DATA_GAMEOBJECT_CURTAINS        18
#define DATA_GAMEOBJECT_STAGEDOORLEFT   19
#define DATA_GAMEOBJECT_STAGEDOORRIGHT  20
#define DATA_GAMEOBJECT_LIBRARY_DOOR    21
#define DATA_GAMEOBJECT_MASSIVE_DOOR    22
#define DATA_GAMEOBJECT_NETHER_DOOR     23
#define DATA_GAMEOBJECT_GAME_DOOR       24
#define DATA_GAMEOBJECT_GAME_EXIT_DOOR  25
#define DATA_IMAGE_OF_MEDIVH            26
#define DATA_MASTERS_TERRACE_DOOR_1     27
#define DATA_MASTERS_TERRACE_DOOR_2     28
#define DATA_CHESS_ECHO_OF_MEDIVH       29
#define CHESS_EVENT_TEAM                30
#define DATA_CHESS_REINIT_PIECES        31
#define DATA_CHESS_CHECK_PIECES_ALIVE   32
#define DATA_IMAGE_OF_MEDIVH_NIGHTBANE  33
#define DATA_CHESS_GAME_PHASE           34
#define DATA_MALCHEZAAR                 35

// Opera Performances
#define EVENT_OZ        1
#define EVENT_HOOD      2
#define EVENT_RAJ       3

#define ERROR_INST_DATA(a)     error_log("TSCR: Instance Data for Karazhan not set properly. Encounter for Creature Entry %u may not work properly.", a->GetEntry());

enum eNPCs { 
    NPC_MEDIVH   = 16816,
    NPC_PAWN_H   = 17469,
    NPC_PAWN_A   = 17211,
    NPC_KNIGHT_H = 21748,
    NPC_KNIGHT_A = 21664,
    NPC_QUEEN_H  = 21750,
    NPC_QUEEN_A  = 21683,
    NPC_BISHOP_H = 21747,
    NPC_BISHOP_A = 21682,
    NPC_ROOK_H   = 21726,
    NPC_ROOK_A   = 21160,
    NPC_KING_H   = 21752,
    NPC_KING_A   = 21684
};

typedef enum gamePhase
{
    NOTSTARTED      = 0,
    PVE_WARMUP      = 1, // Medivh has been spoken too but king isn't controlled yet
    INPROGRESS_PVE  = 2,
    FAILED          = 4,
    PVE_FINISHED    = 5,
    PVP_WARMUP      = 6,
    INPROGRESS_PVP  = 7  // Get back to PVE_FINISHED after that
} GamePhase;
#endif

