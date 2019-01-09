-- NPC 18286
SET @ENTRY = 18286;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 37, 0, 100, 0, 0, 0, 0, 0, 58, 6, 35316, 3000, 3500, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mragesh - On AI Initialize - Install Caster SUN Template '),
(@ENTRY, 0, 2, 0, 9, 0, 100, 0, 0, 5, 2500, 7500, 11, 35317, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mragesh - Within 0-5 Range - Cast "<a href="http://wowhead.com/spell=35317">Chilled Blood</a>" ');

-- NPC 18131
SET @ENTRY = 18131;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 0, 0, 75, 0, 5000, 10000, 7500, 10000, 11, 35333, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshfang Slicer - In Combat - Cast "<a href="http://wowhead.com/spell=35333">Tail Swipe</a>" '),
(@ENTRY, 0, 2, 0, 0, 0, 75, 0, 1000, 3000, 5000, 10000, 11, 17008, 0, 0, 0, 0, 0, 2, 32, 0, 0, 0, 0, 0, 0, 0, 'Marshfang Slicer - In Combat - Cast "<a href="http://wowhead.com/spell=17008">Drain Mana</a>" '),
(@ENTRY, 0, 3, 0, 0, 0, 100, 0, 5000, 10000, 5000, 10000, 11, 35334, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshfang Slicer - In Combat - Cast "<a href="http://wowhead.com/spell=35334">Nether Shock</a>" ');


-- Add creatures around Terrorclaw
DELETE FROM creature_entry WHERE spawnID IN (1311717, 1311718, 1311719, 1311720, 1311721, 1311722);
INSERT INTO creature_entry (spawnID, entry, equipment_id) VALUES
(1311717,	20295,	-1),
(1311718,	20294,	-1),
(1311719,	20295,	-1),
(1311720,	20294,	-1),
(1311721,	20295,	-1),
(1311722,	20294,	-1);
INSERT INTO creature (spawnID, map, spawnMask, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, pool_id, ScriptName, patch_min, patch_max) VALUES
(1311717,	530,	1,	0,	404.949,	8328.56,	24.2113,	4.65812,	300,	5,	0,	5341,	0,	0,	0,	0,	'',	0,	10),
(1311718,	530,	1,	0,	418.403,	8348.02,	22.8904,	0.566198,	300,	5,	0,	5341,	0,	0,	0,	0,	'',	0,	10),
(1311719,	530,	1,	0,	420.775,	8372.79,	22.1903,	1.19137,	300,	5,	0,	5527,	0,	0,	0,	0,	'',	0,	10),
(1311720,	530,	1,	0,	403.299,	8379.79,	22.4293,	1.99719,	300,	5,	0,	5527,	0,	0,	0,	0,	'',	0,	10),
(1311721,	530,	1,	0,	378.706,	8360.39,	19.2899,	3.81775,	300,	5,	0,	5527,	0,	0,	0,	0,	'',	0,	10),
(1311722,	530,	1,	0,	377.285,	8345.83,	21.815,	4.62671,	300,	5,	0,	5341,	0,	0,	0,	0,	'',	0,	10);

-- Update Terrorclaw position
UPDATE creature SET position_x=397.131, position_y=8352.3, position_z=22.7498, orientation=3.88311 WHERE spawnID = 86747;
-- NPC 20477
SET @ENTRY = 20477;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 15000, 20000, 11, 40504, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Terrorclaw - In Combat - Cast "<a href="http://wowhead.com/spell=40504">Cleave</a>" '),
(@ENTRY, 0, 1, 0, 2, 0, 100, 0, 0, 50, 5000, 30000, 11, 34259, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Terrorclaw - Between 0-50% HP - Cast "<a href="http://wowhead.com/spell=34259">Fear</a>" ');

-- NPC 18133
SET @ENTRY = 18133;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 10000, 15000, 11, 35329, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshlight Bleeder - In Combat - Cast "<a href="http://wowhead.com/spell=35329">Vibrant Blood</a>" '),
(@ENTRY, 0, 1, 0, 0, 0, 100, 0, 2500, 7500, 15000, 15000, 11, 13443, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshlight Bleeder - In Combat - Cast "<a href="http://wowhead.com/spell=13443">Rend</a>" ');

-- NPC 20198
SET @ENTRY = 20198;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 7500, 12500, 10000, 15000, 11, 35325, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Fenglow Stinger - In Combat - Cast "<a href="http://wowhead.com/spell=35325">Glowing Blood</a>" ');

-- NPC 18121
SET @ENTRY = 18121;
UPDATE creature_template SET AIName='SmartAI', ScriptName='', flags_extra = flags_extra | 0x200000 WHERE entry = @ENTRY; -- Add dual wield
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 37, 0, 100, 0, 0, 0, 0, 0, 58, 6, 9613, 3000, 3500, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Souleater - On AI Initialize - Install Caster SUN Template '),
(@ENTRY, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 11, 35194, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Souleater - On Respawn - Cast "<a href="http://wowhead.com/spell=35194">Shadowform</a>" ');

-- NPC 18120
SET @ENTRY = 18120;
UPDATE creature_template SET AIName='', ScriptName='', flags_extra = flags_extra | 0x200000 WHERE entry = @ENTRY; -- Add dual wield
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;

-- NPC 18160
SET @ENTRY = 18160;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 10000, 15000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - In Combat - Cast "<a href="http://wowhead.com/spell=15496">Cleave</a>" '),
(@ENTRY, 0, 1, 0, 2, 0, 100, 1, 81, 90, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 81-90% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 2, 0, 2, 0, 100, 1, 71, 80, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 71-80% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 3, 0, 2, 0, 100, 1, 61, 70, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 61-70% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 4, 0, 2, 0, 100, 1, 51, 60, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 51-60% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 5, 0, 2, 0, 100, 1, 41, 50, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 41-50% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 6, 0, 2, 0, 100, 1, 31, 40, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 31-40% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 7, 0, 2, 0, 100, 1, 21, 30, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 21-30% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 8, 0, 2, 0, 100, 1, 11, 20, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 11-20% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)'),
(@ENTRY, 0, 9, 0, 2, 0, 100, 1, 0, 10, 0, 0, 11, 33962, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Overlord Gorefist - Between 0-10% HP - Cast "<a href="http://wowhead.com/spell=33962">Toughen</a>" (No Repeat)');

INSERT IGNORE INTO creature_entry (spawnID, entry) VALUES
(64248, 18120),
(64231, 18121),
(64251, 18120),
(64228, 18121),
(64227, 18121),
(64226, 18121),
(64258, 18120);

DELETE FROM waypoint_data WHERE id IN (1483, 1484);
UPDATE creature SET position_x=966.833984, position_y=7373.423828, position_z=29.237190, orientation=2.918122 WHERE spawnID = 67968;
UPDATE creature SET position_x=966.002441, position_y=7369.765137, position_z=29.232878, orientation=2.918122 WHERE spawnID = 67969;
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(1483,	1,	966.846,	7372.78,	29.2574,	0,	0,	0,	NULL,	100,	0),
(1483,	2,	953.248,	7375.67,	27.8889,	0,	0,	0,	NULL,	100,	0),
(1483,	3,	941.762,	7377.91,	23.1664,	0,	0,	0,	NULL,	100,	0),
(1483,	4,	932.359,	7380.44,	19.84,	0,	0,	0,	NULL,	100,	0),
(1483,	5,	917.988,	7389.29,	20.1314,	0,	0,	0,	NULL,	100,	0),
(1483,	6,	904.686,	7402.32,	20.238,	0,	0,	0,	NULL,	100,	0),
(1483,	7,	893.11,	7419.27,	20.4625,	0,	0,	0,	NULL,	100,	0),
(1483,	8,	885.457,	7433.27,	20.6322,	0,	0,	0,	NULL,	100,	0),
(1483,	9,	880.443,	7445.35,	20.9483,	0,	0,	0,	NULL,	100,	0);
REPLACE INTO waypoint_info (id, pathType, pathDirection) VALUES (1483, 2, 0);
UPDATE creature_addon SET path_id = 1483 WHERE spawnID = 67968;
DELETE FROM creature_formations WHERE memberGUID IN (67968, 67969);
INSERT INTO creature_formations (leaderGUID, memberGUID, dist, angle, groupAI, respawn, linkedloot) VALUES
(NULL,	67968,	0,	0,  0,	0,	0),
(67968,	67969,	3.74352,	1.57016,	2,	0,	0);

-- Add gossip to Wanted Poster in Telaar, Nagrand
REPLACE INTO gossip_text (ID, comment, text0_0, text0_1, BroadcastTextID0, lang0, Probability0, em0_0, em0_1, em0_2, em0_3, em0_4, em0_5, text1_0, text1_1, BroadcastTextID1, lang1, Probability1, em1_0, em1_1, em1_2, em1_3, em1_4, em1_5, text2_0, text2_1, BroadcastTextID2, lang2, Probability2, em2_0, em2_1, em2_2, em2_3, em2_4, em2_5, text3_0, text3_1, BroadcastTextID3, lang3, Probability3, em3_0, em3_1, em3_2, em3_3, em3_4, em3_5, text4_0, text4_1, BroadcastTextID4, lang4, Probability4, em4_0, em4_1, em4_2, em4_3, em4_4, em4_5, text5_0, text5_1, BroadcastTextID5, lang5, Probability5, em5_0, em5_1, em5_2, em5_3, em5_4, em5_5, text6_0, text6_1, BroadcastTextID6, lang6, Probability6, em6_0, em6_1, em6_2, em6_3, em6_4, em6_5, text7_0, text7_1, BroadcastTextID7, lang7, Probability7, em7_0, em7_1, em7_2, em7_3, em7_4, em7_5)
VALUES ('1872753', 'Wanted Poster in Telaar, Nagrand', "Registrants are required to speak with Warden Moi'bff Jill before posting any task.$b
Those found making unapproved postings will be prosecuted.$b
Post no bills.", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0');
REPLACE INTO gossip_menu (MenuID, TextID, patch_min, patch_max)
VALUES ('7685', '1872753', '0', '10');

-- NPC 18408 - Warden Iolol becomes Warden Moi'bff Jill in Patch 2.3, so reverting... + event
SET @ENTRY = 18408;
UPDATE creature_template SET AIName='SmartAI', ScriptName='', name="Warden Iolol" WHERE entry = @ENTRY;
UPDATE quest_template SET Objectives = replace(Objectives, "Warden Moi'bff Jill", "Warden Iolol");
DELETE FROM creature_addon WHERE spawnID = 65800;
DELETE FROM creature_template_addon WHERE entry = @ENTRY;
DELETE FROM waypoint_data WHERE id = 1440;

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 1, 0, 100, 0, 10000, 20000, 180000, 240000, 53, 0, 18408, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - Out of Combat - Start Waypoint 18408 '),
(@ENTRY, 0, 1, 0, 40, 0, 100, 0, 3, 18408, 0, 0, 80, 1840801, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - On Waypoint 3 Reached - Run Script <a href="/smartai/script/1840801">1840801</a> '),
(@ENTRY, 0, 2, 0, 40, 0, 100, 0, 5, 18408, 0, 0, 66, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4.69494, 'Warden Iolol - On Waypoint 5 Reached - Set Orientation 4.69494 ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 54, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - In Combat - Pause Waypoint 20000ms '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 17, 173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - In Combat - Set Emote State 173 '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 17000, 17000, 0, 0, 17, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - In Combat - Set Emote State 0 '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 500, 500, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warden Iolol - In Combat - Say Line 1 ');

DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "New posting going up! Adventurers and heroes, gather round the bulletin board!", 12, 7, 100, 0, 0, "Warden Iolol - "),
(@ENTRY, 1, 0, "That should get Telaar the assistance it needs!", 12, 7, 100, 5, 0, "Warden Iolol - ");

DELETE FROM waypoints WHERE entry = @ENTRY;
INSERT INTO waypoints (entry, pointid, position_x, position_y, position_z, point_comment) VALUES
(@ENTRY,	1,	-2568.98,	7271.64,	15.48,	'Warden Iolol'),
(@ENTRY,	2,	-2566.52,	7272.41,	15.48,	'Warden Iolol'),
(@ENTRY,	3,	-2565.77,	7274,	15.48,	'Warden Iolol'),
(@ENTRY,	4,	-2566.69,	7272.65,	15.48,	'Warden Iolol'),
(@ENTRY,	5,	-2568.98,	7271.64,	15.4789,	'Warden Iolol');

-- NPC 18352
SET @ENTRY = 18352;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 32248, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Boulderfist Hunter - On Aggro - Cast "<a href="http://wowhead.com/spell=32248">Spear Throw</a>" ');

-- Lump interrogation
DELETE FROM gossip_text WHERE ID = 9354;
REPLACE INTO gossip_text (ID, comment, text0_0, text0_1, BroadcastTextID0, lang0, Probability0, em0_0, em0_1, em0_2, em0_3, em0_4, em0_5, text1_0, text1_1, BroadcastTextID1, lang1, Probability1, em1_0, em1_1, em1_2, em1_3, em1_4, em1_5, text2_0, text2_1, BroadcastTextID2, lang2, Probability2, em2_0, em2_1, em2_2, em2_3, em2_4, em2_5, text3_0, text3_1, BroadcastTextID3, lang3, Probability3, em3_0, em3_1, em3_2, em3_3, em3_4, em3_5, text4_0, text4_1, BroadcastTextID4, lang4, Probability4, em4_0, em4_1, em4_2, em4_3, em4_4, em4_5, text5_0, text5_1, BroadcastTextID5, lang5, Probability5, em5_0, em5_1, em5_2, em5_3, em5_4, em5_5, text6_0, text6_1, BroadcastTextID6, lang6, Probability6, em6_0, em6_1, em6_2, em6_3, em6_4, em6_5, text7_0, text7_1, BroadcastTextID7, lang7, Probability7, em7_0, em7_1, em7_2, em7_3, em7_4, em7_5) VALUES
(1872754, 'Lump interrogation', "Why you mean to Lump? Lump hungry, dat's all.", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0'),
(1872755, 'Lump interrogation', "OK! Lump tell you anything you want!", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0'),
(1872756, 'Lump interrogation', "Boulderfist hungry! Need food! Warmaul ogre tribe kick Boulderfist out of mound to north. Many more Boulderfist to feed now.", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0'),
(1872757, 'Lump interrogation', "Dat not true. Me just want to nibble. I don't eat all. Ok, maybe I eat da little ones.$b$bDey so delicious!", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0'),
(1872758, 'Lump interrogation', "<Lump shrugs.>$b$bbYou bring da war to da Boulderfist. We put it in da stew.", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0');
REPLACE INTO gossip_menu (MenuID, TextID, patch_min, patch_max) VALUES
(57145, 1872754, '0', '10'),
(57146, 1872755, '0', '10'),
(57147, 1872756, '0', '10'),
(57148, 1872757, '0', '10'),
(57149, 1872758, '0', '10');
REPLACE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max) VALUES
(57145, '0', '0', 'I need answers, ogre!', NULL, '1', '1', 57146, NULL, '0', '0', NULL, NULL, '0', '10'),
(57146, '0', '0', 'Why are the Boulderfist out this far? You know that this is Kurenai territory.', NULL, '1', '1', 57147, NULL, '0', '0', NULL, NULL, '0', '10'),
(57147, '0', '0', "And you think you can just eat anything you want? You're obviously trying to eat the Broken of Telaar.", NULL, '1', '1', 57148, NULL, '0', '0', NULL, NULL, '0', '10'),
(57148, '0', '0', "This means war, Lump! War I say!", NULL, '1', '1', 57149, NULL, '0', '0', NULL, NULL, '0', '10');

-- NPC 18351
SET @ENTRY = 18351;
UPDATE creature_template SET AIName='SmartAI', ScriptName='', gossip_menu_id=57145, flags_extra = flags_extra | 0x200000 WHERE entry = @ENTRY; -- Yep, dual wield with shield
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 80, 1835101, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - On Respawn - Run Script <a href="/smartai/script/1835101">1835101</a> '),
(@ENTRY, 0, 2, 0, 4, 0, 100, 0, 0, 0, 0, 0, 80, 1835102, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - On Aggro - Run Script <a href="/smartai/script/1835102">1835102</a> '),
(@ENTRY, 0, 3, 0, 2, 0, 100, 1, 0, 10, 0, 0, 80, 1835103, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - Between 0-10% HP - Run Script <a href="/smartai/script/1835103">1835103</a> (No Repeat)'),
(@ENTRY, 0, 4, 0, 62, 0, 100, 0, 57148, 0, 0, 0, 33, 18354, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - On Gossip 57148 Option 0 Selected - Quest Credit "<a href="http://wowhead.com/quest=18354">Lump"s Quest Credit</a>" ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 75, 32951, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Add Aura "<a href="http://wowhead.com/spell=32951">Sleeping Sleep</a>" '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 42, 0, 10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Set Invincibility At 0% HP '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 31994, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Cast "<a href="http://wowhead.com/spell=31994">Shoulder Charge</a>" ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+2 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+2, 9, 200, 0, 0, 0, 100, 0, 0, 0, 0, 0, 28, 32951, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Remove Aura "<a href="http://wowhead.com/spell=32951">Sleeping Sleep</a>" '),
(@ENTRY*100+2, 9, 201, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Say Line 0 '),
(@ENTRY*100+2, 9, 202, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 31994, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Cast "<a href="http://wowhead.com/spell=31994">Shoulder Charge</a>" ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+3 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+3, 9, 300, 0, 0, 0, 100, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Evade '),
(@ENTRY*100+3, 9, 301, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Say Line 1 '),
(@ENTRY*100+3, 9, 302, 0, 0, 0, 100, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Set Faction 35 '),
(@ENTRY*100+3, 9, 303, 0, 0, 0, 100, 0, 0, 0, 0, 0, 103, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Set Root On '),
(@ENTRY*100+3, 9, 309, 0, 0, 0, 100, 0, 5000, 5000, 0, 0, 90, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Set Flag Sit Down '),
(@ENTRY*100+3, 9, 310, 0, 0, 0, 100, 0, 0, 0, 0, 0, 41, 60000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lump - In Combat - Despawn In 60000ms ');

DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "In Nagrand, food hunt ogre!", 12, 0, 100, 0, 0, "Lump - "),
(@ENTRY, 0, 1, "You taste good with maybe a little salt and pepper.", 12, 0, 100, 0, 0, "Lump - "),
(@ENTRY, 1, 0, "OK, OK! Lump give up!", 12, 0, 100, 0, 0, "Lump - ");

DELETE FROM creature_template_addon WHERE entry = @ENTRY;









