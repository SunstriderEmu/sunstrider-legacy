-- Delete defenders in front of dark portal, now summoned by script
DELETE FROM creature WHERE id IN (18970,
18971,
18969,
18950,
18972,
18965,
18949,
18966,
18986,
18948);

DELETE FROM creature WHERE guid IN (98080, 98081);

DELETE FROM smart_scripts WHERE entryorguid = 18965 AND id >= 5;
DELETE FROM waypoints WHERE entry = 18965;

-- Delete wrong spell script
DELETE FROM spell_script_names WHERE spell_id IN (32148);
REPLACE INTO spell_script_names VALUES (32343, "spell_revive_self");

-- REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('3440','13','7','33240','0','0','31','0','3','19179','0','0','0','0','','SPELL_SUMMON_INFERNALS - Target triggers');

-- paths from portals (alliance + horde)
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22703','0','-316.503','1019.75','54.25','0','0','1','0','100','0');
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22703','1','-273.468','1027.44','54.3281','0','0','1','0','100','0');
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22703','2','-262.463','1060.36','54.3082','0','0','1','0','100','0');
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22704','0','-182.73','1018.84','54.29','0','0','1','0','100','0');
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22704','1','-220.3','1026.76','54.3','0','0','1','0','100','0');
replace into `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) values('22704','2','-234.32','1058.55','54.31','0','0','1','0','100','0');

-- Darnassian Archer
SET @ENTRY = 18965;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 37, 0, 100, 0, 0, 80, 2300, 3900, 58, 2, 22121, 2300, 3600, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Darnassian Archer - On AI Initialize - Install Turret Template'),
(@ENTRY, 0, 2, 0, 37, 0, 100, 0, 0, 80, 2300, 3900, 40, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darnassian Archer - On AI Initialize - Set Sheath Ranged'),
(@ENTRY, 0, 3, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darnassian Archer - On Waypoint 0 Ended - Evade');

-- Stormwind Mage
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18949, 0, 3, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stormwind Mage - On Waypoint 0 Ended - Evade');

-- Justinius the Harbinger
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18966, 0, 5, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Justinius the Harbinger - On Waypoint 0 Ended - Evade');

-- Ironforge Paladin
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18986, 0, 2, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironforge Paladin - On Waypoint 0 Ended - Evade');

-- Stormwind Soldier
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18948, 0, 2, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stormwind Soldier - On Waypoint 0 Ended - Evade');

-- Darkspear Axe Thrower
SET @ENTRY = 18970;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 37, 0, 100, 0, 0, 80, 2300, 3900, 58, 2, 10277, 2300, 3600, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkspear Axe Thrower - On AI Initialize - Install Turret Template '),
(@ENTRY, 0, 2, 0, 37, 0, 100, 0, 0, 80, 2300, 3900, 40, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkspear Axe Thrower - On AI Initialize - Set Sheath Ranged'),
(@ENTRY, 0, 3, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkspear Axe Thrower - On Waypoint 0 Ended - Evade');

-- Undercity Mage
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18971, 0, 3, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Undercity Mage - On Waypoint 0 Ended - Evade');

-- Melgromm Highmountain
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18969, 0, 5, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Melgromm Highmountain - On Waypoint 0 Ended - Evade');

-- Orgrimmar Grunt
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18950, 0, 2, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Orgrimmar Grunt - On Waypoint 0 Ended - Evade');

-- Orgrimmar Shaman
REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(18972, 0, 2, 0, 58, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Orgrimmar Shaman - On Waypoint 0 Ended - Evade');

