UPDATE creature_template SET AIName = "", ScriptName = "npc_arcane_anomaly" WHERE entry = 16488;
DELETE FROM smart_scripts where entryorguid = 16488;