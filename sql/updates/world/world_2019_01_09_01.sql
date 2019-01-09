-- Fix Quest: What We Don't Know...

-- Exarch Admetius
REPLACE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max) VALUES
('9022', '0', '0', 'I need the illusion cast upon me again, Admetius.', NULL, '1', '1', NULL, NULL, '0', '0', NULL, NULL, '0', '10');
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 9022;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment, patch_min, patch_max) VALUES
(15,	9022,	0,	0,	0,	9,	0,	9756,	0,	0,	0,	0,	0,	'',	'Gossip option on quest taken 9756',	0,	10),
(15,	9022,	0,	0,	0,	28,	0,	9756,	0,	0,	1,	0,	0,	'',	'Gossip option on quest incomplete 9756',	0,	10),
(15,	9022,	0,	0,	0,	1,	0,	31609,	0,	0,	1,	0,	0,	'',	'Gossip option on not aura 31609',	0,	10);

REPLACE INTO gameobject (guid, id, map, spawnMask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, ScriptName) VALUES
(151223,	182012,	530,    1,  -1937.6,	-11847.1,	50.6868,	2.68781,	0,	0,	0.97437,	0.22495,	120,	0,	1,	'');

-- NPC 17658
SET @ENTRY = 17658;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 19, 0, 100, 0, 9756, 0, 0, 0, 11, 31609, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Exarch Admetius - On Quest "<a href="http://wowhead.com/quest=9756">What We Don"t Know...</a>" Accepted - Cast "<a href="http://wowhead.com/spell=31609">Exarch"s Enchantment</a>" '),
(@ENTRY, 0, 2, 0, 62, 0, 100, 0, 9022, 0, 0, 0, 11, 31609, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Exarch Admetius - On Gossip 9022 Option 0 Selected - Cast "<a href="http://wowhead.com/spell=31609">Exarch"s Enchantment</a>" ');

-- Captured Sunhawk Agent
REPLACE INTO gossip_menu (MenuID, TextID) VALUES
(9136, 9136),
(9137, 9137),
(9138, 9138),
(9139, 9139),
(9140, 9140),
(9141, 9141);

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 9136 AND ConditionTypeOrReference = 28 AND ConditionValue1 = 9756;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment, patch_min, patch_max) VALUES
(14,	9136,	0,	0,	0,	28,	0,	9756,	0,	0,	1,	0,	0,	'',	'Gossip option on quest taken 9756',	0,	10);

REPLACE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max) VALUES
(9136, '0', '0', "I'm a prisoner, what does it look like? The draenei filth captured me as I exited the sun gate. They killed our portal controllers and destroyed the gate. The Sun King will be most displeased with this turn of events.", NULL, '1', '1', 9137, NULL, '0', '0', NULL, NULL, '0', '10'),
(9137, '0', '0', "Ah yes, Sironas. I had nearly forgotten that Sironas was here. I served under Sironas back on Outland. I hadn't heard of this abomination, though; those damnable draenei captured me before I even fully materialized on this world.", NULL, '1', '1', 9138, NULL, '0', '0', NULL, NULL, '0', '10'),
(9138, '0', '0', "Incredible. How did Sironas accomplish such a thing?", NULL, '1', '1', 9139, NULL, '0', '0', NULL, NULL, '0', '10'),
(9139, '0', '0', "Sironas is an eredar... I mean, yes, obviously.", NULL, '1', '1', 9140, NULL, '0', '0', NULL, NULL, '0', '10'),
(9140, '0', '0', "The Vector Coil is massive. I hope we have more than one abomination guarding the numerous weak points.", NULL, '1', '1', 9141, NULL, '0', '0', NULL, NULL, '0', '10'),
(9141, '0', '0', "I did and you believed me. Thank you for the information, blood elf. You have helped us more than you could know.", NULL, '1', '1', NULL, NULL, '0', '0', NULL, NULL, '0', '10');

-- NPC 17824
SET @ENTRY = 17824;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 64, 0, 100, 0, 0, 0, 0, 0, 98, 9136, 9136, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Sunhawk Agent - On Gossip Hello - Send Gossip Menu 9136 '),
(@ENTRY, 0, 2, 0, 62, 0, 100, 0, 9141, 0, 0, 0, 80, 1782401, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Sunhawk Agent - On Gossip 9141 Option 0 Selected - Run Script <a href="/smartai/script/1782401">1782401</a> ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 33, 17974, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Sunhawk Agent - In Combat - Quest Credit "<a href="http://wowhead.com/quest=17974">Captured Sunhawk Agent Invisible Trigger</a>" '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Sunhawk Agent - In Combat - Close Gossip '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Sunhawk Agent - In Combat - Say Line 0 ');

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 22 AND SourceGroup = 2 AND SourceEntry = @ENTRY AND ConditionTypeOrReference = 28 AND ConditionValue1 = 9756;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment, patch_min, patch_max) VALUES
(22,	2,	@ENTRY,	0,	0,	28,	0,	9756,	0,	0,	1,	0,	0,	'',	'Gossip option on quest 9756 incomplete',	0,	10);

SET @ENTRY = 17824;
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "Treacherous whelp! Sironas will destroy you and your people!", 12, 10, 100, 5, 0, "Captured Sunhawk Agent - ");
