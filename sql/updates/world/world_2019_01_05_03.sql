-- Add Gurgthock base gossip
REPLACE INTO gossip_text (ID, comment, text0_0, text0_1, BroadcastTextID0, lang0, Probability0, em0_0, em0_1, em0_2, em0_3, em0_4, em0_5, text1_0, text1_1, BroadcastTextID1, lang1, Probability1, em1_0, em1_1, em1_2, em1_3, em1_4, em1_5, text2_0, text2_1, BroadcastTextID2, lang2, Probability2, em2_0, em2_1, em2_2, em2_3, em2_4, em2_5, text3_0, text3_1, BroadcastTextID3, lang3, Probability3, em3_0, em3_1, em3_2, em3_3, em3_4, em3_5, text4_0, text4_1, BroadcastTextID4, lang4, Probability4, em4_0, em4_1, em4_2, em4_3, em4_4, em4_5, text5_0, text5_1, BroadcastTextID5, lang5, Probability5, em5_0, em5_1, em5_2, em5_3, em5_4, em5_5, text6_0, text6_1, BroadcastTextID6, lang6, Probability6, em6_0, em6_1, em6_2, em6_3, em6_4, em6_5, text7_0, text7_1, BroadcastTextID7, lang7, Probability7, em7_0, em7_1, em7_2, em7_3, em7_4, em7_5)  VALUES 
(1872758, 'Gurgthock', "You're definitely strong, $r, but do you have what it takes to become the Warmaul champion?", NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0');
REPLACE INTO gossip_menu (MenuID, TextID)
VALUES (57150, 1872758);

SET @ENTRY = 18471;
DELETE FROM smart_scripts WHERE entryorguid LIKE '18471%' AND source_type IN (0,9);
UPDATE creature_template SET gossip_menu_id = 57150, AIName='', ScriptName='npc_gurgthock' WHERE entry = @ENTRY;
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "Get in the Ring of Blood, $n. The fight is about to start!", 12, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 1, 0, "The battle is about to begin! $n versus the ferocious clefthoof, Brokentoe!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 2, 0, "$n has defeated Brokentoe!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 3, 0, "The battle is about to begin! The unmerciful Murkblood twins versus $n!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 4, 0, "Unbelievable! $n has defeated the Murkblood twins!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 5, 0, "Hailing from the mountains of Blade's Edge comes Rokdar the Sundered Lord! $n is in for the fight of $g his:her; life.", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 6, 0, "$n is victorious once more!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 7, 0, "From parts unknown: Skra'gath! Can $n possibly survive the onslaught of void energies?", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 8, 0, "This is the moment we've all been waiting for! The Warmaul champion is about to make his first showing at the Ring of Blood in weeks! Will $n go down in defeat as easily as the champion's other opponents? We shall see...", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 9, 0, "Mogor has challenged you. You have to accept! Get in the ring if you are ready to fight.", 12, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 10, 0, "For the first time in the Ring of Blood's history, Mogor has chosen to exercise his right of battle! On this wartorn ground, $N will face Mogor, hero of the Warmaul!", 14, 0, 100, 0, 0, "Gurgthock"),
(@ENTRY, 11, 0, "$n has defeated the hero of the Warmaul, Mogor! All hail $n!", 12, 0, 100, 15, 0, "Gurgthock");

-- Mogor
DELETE FROM smart_scripts WHERE entryorguid LIKE '18069%' AND source_type IN (0,9);
UPDATE creature_template SET AIName='', ScriptName='npc_mogor', flags_extra = 0 WHERE entry = 18069;
DELETE FROM waypoint_data WHERE id = 18069;
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(18069,	1,	-715.037,	7922.95,	59.5062,	0,	0,	0,	NULL,	100,	0),
(18069,	2,	-720.283,	7916.27,	57.1464,	0,	0,	0,	NULL,	100,	0),
(18069,	3,	-724.706,	7909.14,	51.8177,	0,	0,	0,	NULL,	100,	0),
(18069,	4,	-717.016,	7896.94,	48.4604,	0,	0,	0,	NULL,	100,	0);

-- Correct spell for Mogor totem
UPDATE creature_template SET modelid1 = 4587, modelid3 = NULL, spell1 = 18978 WHERE entry = 12141;