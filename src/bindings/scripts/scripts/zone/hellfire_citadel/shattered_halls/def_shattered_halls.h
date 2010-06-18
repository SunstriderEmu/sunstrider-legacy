/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SHATTERED_H
#define DEF_SHATTERED_H

enum eShatteredHalls {
    DATA_NETHEKURSE_EVENT   = 0,
    DATA_PORUNG_EVENT       = 1,
    DATA_OMROGG_EVENT       = 2,
    DATA_BLADEFIST_EVENT    = 3,
    DATA_EXECUTIONER_EVENT  = 4,
    DATA_NETHEKURSE_GUID    = 5,
    DATA_EXECUTIONER_GUID   = 6,
    DATA_TIMER_LEFT         = 7,    // Every 30 sec, timer is saved in DB (total time = 55 + 10 + 15 = 80 minutes, first buff at 80' left, second at 25', third at 15')
    
    ENTRY_FIRST_DOOR         = 182539,
    ENTRY_SECOND_DOOR        = 182540
};

enum eSpellsExecutioner {
    EXEC_TIMER_55       = 39288,
    EXEC_TIMER_10       = 39289,
    EXEC_TIMER_15       = 39290
};
#endif

