-- NPC 18117
SET @ENTRY = 18117;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 13, 0, 100, 0, 10000, 15000, 0, 0, 11, 11978, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Ogre - On Victim Casting "<a href="http://wowhead.com/spell=10000">undefined</a>" - Cast "<a href="http://wowhead.com/spell=11978">Kick</a>" '),
(@ENTRY, 0, 1, 0, 2, 0, 100, 1, 0, 25, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Ogre - Between 0-25% HP - Cast "<a href="http://wowhead.com/spell=8599">Enrage</a>" (No Repeat)'),
(@ENTRY, 0, 2, 0, 2, 0, 100, 1, 0, 25, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Ogre - Between 0-25% HP - Say Line 0 (No Repeat)'),
(@ENTRY, 0, 3, 0, 4, 0, 20, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Ogre - On Aggro - Say Line 1 ');

DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "%s becomes enraged!", 16, 0, 100, 0, 0, "Ango'rosh Ogre"),
(@ENTRY, 1, 0, "You no take mushroom!", 12, 0, 100, 0, 0, "Ango&#039;rosh Ogre - ");

-- NPC 18118
SET @ENTRY = 18118;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 37, 0, 100, 0, 0, 0, 0, 0, 58, 6, 9532, 3000, 3500, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Shaman - On AI Initialize - Install Caster SUN Template '),
(@ENTRY, 0, 1, 0, 2, 0, 100, 1, 0, 50, 0, 0, 11, 23381, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Shaman - Between 0-50% HP - Cast "<a href="http://wowhead.com/spell=23381">Healing Touch</a>" (No Repeat)'),
(@ENTRY, 0, 2, 0, 0, 0, 100, 1, 5000, 10000, 60000, 90000, 11, 6742, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ango&#039;rosh Shaman - In Combat - Cast "<a href="http://wowhead.com/spell=6742">Bloodlust</a>" (No Repeat)');

DELETE FROM waypoint_data WHERE id = 12949730;
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(12949730,	1,	1001.36,	7678.71,	22.7008,	0,	0,	0,	NULL,	100,	0),
(12949730,	2,	1007.78,	7614.64,	21.9057,	0,	0,	0,	NULL,	100,	0),
(12949730,	3,	978.356,	7681.66,	22.2043,	0,	0,	0,	NULL,	100,	0),
(12949730,	4,	979.644,	7695.08,	22.564,	0,	0,	0,	NULL,	100,	0),
(12949730,	5,	999.253,	7752.24,	22.7444,	0,	0,	0,	NULL,	100,	0),
(12949730,	6,	1002.47,	7788.38,	20.6907,	0,	0,	0,	NULL,	100,	0),
(12949730,	7,	1012.76,	7807.07,	20.2083,	0,	0,	0,	NULL,	100,	0),
(12949730,	8,	1001.94,	7789.48,	20.9952,	0,	0,	0,	NULL,	100,	0),
(12949730,	9,	997.858,	7750.67,	22.8615,	0,	0,	0,	NULL,	100,	0),
(12949730,	10,	989.192,	7709.19,	23.3519,	0,	0,	0,	NULL,	100,	0);
REPLACE INTO creature_addon (spawnID, patch, path_id, mount, bytes0, bytes1, bytes2, emote, moveflags, auras) VALUES
(64205,	0,	12949730,	0,	512,	0,	4097,	0,	0,	'(null)');

DELETE FROM creature_entry WHERE spawnID = 64209;
INSERT IGNORE INTO creature_entry (spawnID, entry) VALUES
(64203, 18117),
(64204, 18118),
(64159, 18118),
(64163, 18118);

DELETE FROM waypoint_data WHERE id = 12949731;
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(12949731,	1,	985.97,	7837.71,	21.6726,	0,	0,	0,	NULL,	100,	0),
(12949731,	2,	1014.79,	7875.76,	23.1061,	0,	0,	0,	NULL,	100,	0),
(12949731,	3,	1030.36,	7906.97,	17.4092,	0,	0,	0,	NULL,	100,	0),
(12949731,	4,	1033.48,	7933.51,	22.0233,	0,	0,	0,	NULL,	100,	0),
(12949731,	5,	1027.38,	7952.51,	21.4702,	0,	0,	0,	NULL,	100,	0),
(12949731,	6,	1027.14,	7988.63,	21.8539,	0,	0,	0,	NULL,	100,	0),
(12949731,	7,	1022.25,	8016.93,	23.371,	0,	0,	0,	NULL,	100,	0);
REPLACE INTO waypoint_info (id, pathType, pathDirection) VALUES ('12949731', '2', '0');

-- NPC 18116
SET @ENTRY = 18116;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 1, 0, 0, 100, 0, 4000, 4000, 10000, 10000, 11, 35204, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Daggerfen Assassin - In Combat - Cast "<a href="http://wowhead.com/spell=35204">Toxic Fumes</a>" '),
(@ENTRY, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Daggerfen Assassin - In Combat - Say Line 0 '),
(@ENTRY, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Daggerfen Assassin - On Reset - Cast "<a href="http://wowhead.com/spell=22766">Sneak</a>" ');
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "%s crushes a vial of green gas.", 16, 0, 100, 0, 0, "Daggerfen Assassin - ");

-- NPC 19174
SET @ENTRY = 19174;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 0, 0, 100, 0, 10000, 15000, 20000, 30000, 11, 35205, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Chieftain Mummaki - In Combat - Cast "<a href="http://wowhead.com/spell=35205">Vanish</a>" '),
(@ENTRY, 0, 2, 0, 0, 0, 100, 0, 5000, 10000, 20000, 25000, 11, 30984, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Chieftain Mummaki - In Combat - Cast "<a href="http://wowhead.com/spell=30984">Wound Poison</a>" '),
(@ENTRY, 0, 3, 0, 67, 0, 100, 0, 3000, 7500, 0, 0, 11, 7159, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Chieftain Mummaki - On Behind Target - Cast "<a href="http://wowhead.com/spell=7159">Backstab</a>" ');

-- NPC 19733
SET @ENTRY = 19733;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 1, 7500, 15000, 0, 0, 11, 35207, 0, 0, 0, 0, 0, 10, 1, 68586, 19174, 0, 0, 0, 0, 0, 'Daggerfen Servant - In Combat - Cast "<a href="http://wowhead.com/spell=35207">Bandage</a>" (No Repeat)');

REPLACE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max) VALUES
('9236', '0', '1', 'Show me your wares, Maktu.', NULL, '3', '128', NULL, NULL, '0', '0', NULL, NULL, '0', '10');

