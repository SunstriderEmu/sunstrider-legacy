-- Ring of Blood monsters

-- NPC 18400 - Rokdar the Sundered Lord
SET @ENTRY = 18400;
UPDATE creature_template SET AIName='SmartAI', ScriptName='', DamageModifier = 13.368, BaseVariance = 0.4037 WHERE entry = @ENTRY; -- From Bestiary
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Rokdar the Sundered Lord - On Aggro - Say Line 0 '),
(@ENTRY, 0, 2, 0, 0, 0, 100, 0, 5000, 7500, 10000, 20000, 11, 16727, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rokdar the Sundered Lord - In Combat - Cast "<a href="http://wowhead.com/spell=16727">War Stomp</a>" '),
(@ENTRY, 0, 3, 0, 0, 0, 100, 0, 3000, 6000, 10000, 20000, 11, 31389, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rokdar the Sundered Lord - In Combat - Cast "<a href="http://wowhead.com/spell=31389">Knock Away</a>" '),
(@ENTRY, 0, 4, 0, 0, 0, 100, 0, 0, 3000, 3000, 5000, 11, 15976, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rokdar the Sundered Lord - In Combat - Cast "<a href="http://wowhead.com/spell=15976">Puncture</a>" ');


-- NPC 18401 - Skra'gath
SET @ENTRY = 18401;
UPDATE creature_template SET AIName='', ScriptName='npc_skragath', DamageModifier = 16.523, BaseVariance = 0.40085, dmgschool = 5 WHERE entry = @ENTRY; -- From Bestiary
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "Closer... Come closer... See what the void brings!", 14, 0, 100, 0, 0, "Skra'gath"),
(@ENTRY, 1, 0, "%s absorbs the arcane energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - "),
(@ENTRY, 2, 0, "%s absorbs the fire energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - "),
(@ENTRY, 3, 0, "%s absorbs the frost energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - "),
(@ENTRY, 4, 0, "%s absorbs the holy energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - "),
(@ENTRY, 5, 0, "%s absorbs the nature energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - "),
(@ENTRY, 6, 0, "%s absorbs the shadow energy of the attack.", 16, 0, 100, 0, 0, "Skra&#039;gath - ");


-- NPC 18471 - Gurgthock
SET @ENTRY = 18471;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES
(@ENTRY, 12, 0, "$N went down like a sack of orc skulls!", 14, 0, 100, 0, 0, "Gurgthock - On failed quest Warmaul Champion"),
(@ENTRY, 13, 0, "All that's left of $N is a red stain on the floor!", 14, 0, 100, 0, 0, "Gurgthock - On failed quest Brokentoe");


-- NPC 18402 - Warmaul Champion
-- Damaged is based on Bestiary and then boosted as it seemed to have been nerfed in 2.3
SET @ENTRY = 18402;
UPDATE creature_template SET DamageModifier = 20.8795, BaseVariance = 0.40675, AIName='', ScriptName='npc_warmaul_champion', MovementType = 2 WHERE entry = @ENTRY;
DELETE FROM waypoint_data WHERE id = @ENTRY;
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid, comment) VALUES
(@ENTRY,	1,	-706.349,	7866.02,	44.9805,	0,	0,	0,	NULL,	100,	0,	'Warmaul Champion'),
(@ENTRY,	2,	-710.258,	7886.39,	46.4929,	0,	0,	0,	NULL,	100,	0,	'Warmaul Champion'),
(@ENTRY,	3,	-717.591,	7901.46,	49.2525,	0,	0,	0,	NULL,	100,	0,	'Warmaul Champion');
REPLACE INTO waypoint_info (id, pathType, pathDirection) VALUES (@ENTRY, 1,  0);
DELETE FROM creature_template_addon WHERE entry = @ENTRY;
INSERT IGNORE INTO creature_template_addon (entry, path_id) VALUES
(@ENTRY, @ENTRY);
