-- Importing creature gossip with entry 5566 (Tannysa) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872024', 'Imported from TC', 'I hope that your interruption is for a good cause, I was in the middle of some important work.', 'I hope that your interruption is for a good cause, I was in the middle of some important work.', '3336', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('643', '1872024');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872025', 'Imported from TC', 'Another Herbalist, hmm. I doubt your skill is as great as my own so perhaps I can train you in a few proper techniques.', 'Another Herbalist, hmm. I doubt your skill is as great as my own so perhaps I can train you in a few proper techniques.', '3337', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('643', '1872025');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('643', '0', '3', 'Train me.', '3266', '5', '16', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 643 WHERE entry = 5566;


-- Importing creature gossip with entry 5493 (Arnold Leland) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872026', 'Imported from TC', 'Welcome to Stormwind. Took a long time to rebuild her but it was well worth it. So are you visiting, or are you here on business?', 'Welcome to Stormwind. Took a long time to rebuild her but it was well worth it. So are you visiting, or are you here on business?', '3340', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('645', '1872026');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('645', '0', '3', 'Train me.', '3266', '5', '16', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 645 WHERE entry = 5493;


-- Importing creature gossip with entry 1292 (Maris Granger) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872027', 'Imported from TC', 'Do I look like I have the time or energy to coddle every tourist that wanders through my door? Tell me what you need, and make it quick.', 'Do I look like I have the time or energy to coddle every tourist that wanders through my door? Tell me what you need, and make it quick.', '3342', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('648', '1872027');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('648', '0', '3', 'Train me.', '3266', '5', '16', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 648 WHERE entry = 1292;


-- Importing creature gossip with entry 2327 (Shaina Fuller) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872028', 'Imported from TC', 'Oh, a fellow Physician. I would be glad to teach you anything that would aid your journeys.', 'Oh, a fellow Physician. I would be glad to teach you anything that would aid your journeys.', '3356', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('657', '1872028');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872029', 'Imported from TC', 'Yes my $g son : daughter;? What can I assist you with?', 'Yes my $g son : daughter;? What can I assist you with?', '3357', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('657', '1872029');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872030', 'Imported from TC', 'The esteemed Doctor Gustaf VanHowzen can train you further.  He can be found in Theramore treating the wounded at the barracks.', 'The esteemed Doctor Gustaf VanHowzen can train you further.  He can be found in Theramore treating the wounded at the barracks.', '9699', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('657', '1872030');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('657', '0', '3', 'Train me.', '3266', '5', '16', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 657 WHERE entry = 2327;


-- Importing creature gossip with entry 1309 (Wynne Larson) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872031', 'Imported from TC', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', '3374', '7', '100', '0', '0', '0', '0', '0', '0', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', '3375', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('681', '1872031');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('681', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1309;


-- Importing creature gossip with entry 1318 (Jessara Cordell) with import type GOSSIP
-- Menu 681 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1318;


-- Importing creature gossip with entry 1310 (Evan Larson) with import type GOSSIP
-- Menu 681 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1310;


-- Importing creature gossip with entry 1313 (Maria Lumere) with import type GOSSIP
-- Menu 681 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1313;


-- Importing creature gossip with entry 1308 (Owen Vaughn) with import type GOSSIP
-- Menu 681 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1308;


-- Importing creature gossip with entry 1314 (Duncan Cullen) with import type GOSSIP
-- Menu 681 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 681 WHERE entry = 1314;


-- Importing creature gossip with entry 1304 (Darian Singh) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872032', 'Imported from TC', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', '3374', '7', '100', '0', '0', '0', '0', '0', '0', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', '3375', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('682', '1872032');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('682', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 682 WHERE entry = 1304;


-- Importing creature gossip with entry 5503 (Eldraeith) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872033', 'Imported from TC', 'Made by the finest craftsmen in all of Teldrassil, carried by hand from Kalimdor to Azeroth as a sign of the growing alliance between the people of the Great Tree and the humans of this land. If there is anything you require, you have but to ask.', 'Made by the finest craftsmen in all of Teldrassil, carried by hand from Kalimdor to Azeroth as a sign of the growing alliance between the people of the Great Tree and the humans of this land. If there is anything you require, you have but to ask.', '3373', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('683', '1872033');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('683', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 683 WHERE entry = 5503;


-- Importing creature gossip with entry 1347 (Alexandra Bolero) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872034', 'Imported from TC', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', 'Since the fall of Dalaran and the destruction of Quel\'Thalas, wares of this sort have been hard to come by. But Lucan Cordell and some of our other Tradesman have been doing a fine job of filling our orders with quality merchandise.', '3374', '7', '100', '0', '0', '0', '0', '0', '0', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', 'I sell only the finest arcane gear, made by the crafters and mages here in the city of Stormwind.', '3375', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('684', '1872034');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('684', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 684 WHERE entry = 1347;


-- Importing creature gossip with entry 5494 (Catherine Leland) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872035', 'Imported from TC', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Just browsing my wares or is there something specific I can help you find today?', 'Just browsing my wares or is there something specific I can help you find today?', '3384', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('685', '1872035');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('685', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 685 WHERE entry = 5494;


-- Importing creature gossip with entry 483 (Elaine Trias) with import type GOSSIP
-- Menu 685 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 685 WHERE entry = 483;


-- Importing creature gossip with entry 1299 (Lisbeth Schneider) with import type GOSSIP
-- Menu 685 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 685 WHERE entry = 1299;


-- Importing creature gossip with entry 4981 (Ben Trias) with import type GOSSIP
-- Menu 685 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 685 WHERE entry = 4981;


-- Importing creature gossip with entry 277 (Roberto Pupellyverbos) with import type GOSSIP
-- Menu 685 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 685 WHERE entry = 277;


-- Importing creature gossip with entry 1295 (Lara Moore) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872036', 'Imported from TC', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Just browsing my wares or is there something specific I can help you find today?', 'Just browsing my wares or is there something specific I can help you find today?', '3384', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('686', '1872036');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('686', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 686 WHERE entry = 1295;


-- Importing creature gossip with entry 1298 (Frederick Stover) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872037', 'Imported from TC', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', 'Best deals in all of Stormwind my friend, won\'t find any better. Now, what can I help you with?', '3383', '7', '100', '0', '0', '0', '0', '0', '0', 'Just browsing my wares or is there something specific I can help you find today?', 'Just browsing my wares or is there something specific I can help you find today?', '3384', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('687', '1872037');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('687', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 687 WHERE entry = 1298;


-- Importing creature gossip with entry 1285 (Thurman Mullby) with import type GOSSIP
-- Menu 687 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 687 WHERE entry = 1285;


-- Importing creature gossip with entry 1319 (Bryan Cross) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872038', 'Imported from TC', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', '3387', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('688', '1872038');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('688', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 688 WHERE entry = 1319;


-- Importing creature gossip with entry 1333 (Gerik Koen) with import type GOSSIP
-- Menu 688 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 688 WHERE entry = 1333;


-- Importing creature gossip with entry 1323 (Osric Strang) with import type GOSSIP
-- Menu 688 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 688 WHERE entry = 1323;


-- Importing creature gossip with entry 5565 (Jillian Tanner) with import type GOSSIP
-- Menu 688 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 688 WHERE entry = 5565;


-- Importing creature gossip with entry 1341 (Wilhelm Strang) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872039', 'Imported from TC', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', '3387', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('689', '1872039');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('689', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 689 WHERE entry = 1341;


-- Importing creature gossip with entry 1324 (Heinrich Stone) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872040', 'Imported from TC', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', 'I sell only the best gear, tested and true. And the prices are set, so if you want to haggle then go find a Goblin.', '3387', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('690', '1872040');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('690', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 690 WHERE entry = 1324;


-- Importing creature gossip with entry 1348 (Gregory Ardus) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872041', 'Imported from TC', 'May the light guide your path and aid you in your journeys. If you need any assistance browsing, I will be right over here.', 'May the light guide your path and aid you in your journeys. If you need any assistance browsing, I will be right over here.', '3388', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('691', '1872041');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('691', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 691 WHERE entry = 1348;


-- Importing creature gossip with entry 1349 (Agustus Moulaine) with import type GOSSIP
-- Menu 691 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 691 WHERE entry = 1349;


-- Importing creature gossip with entry 1350 (Theresa Moulaine) with import type GOSSIP
-- Menu 691 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 691 WHERE entry = 1350;


-- Importing creature gossip with entry 1351 (Brother Cassius) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872042', 'Imported from TC', 'May the light guide your path and aid you in your journies. If you need any assistance browsing, I will be right over here.', 'May the light guide your path and aid you in your journies. If you need any assistance browsing, I will be right over here.', '3389', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('692', '1872042');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('692', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 692 WHERE entry = 1351;


-- Importing creature gossip with entry 5510 (Thulman Flintcrag) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872043', 'Imported from TC', 'Don\'t let the humans in the Bazaar fool ye $g lad : lass;, with the subway those gnomes built it\'s easier than ye might think to transport goods from Ironforge. If they try to tack on import fees just tell them yer gonna buy yer gear from me.', 'Don\'t let the humans in the Bazaar fool ye $g lad : lass;, with the subway those gnomes built it\'s easier than ye might think to transport goods from Ironforge. If they try to tack on import fees just tell them yer gonna buy yer gear from me.', '3390', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('693', '1872043');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('693', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 693 WHERE entry = 5510;


-- Importing creature gossip with entry 5512 (Kaita Deepforge) with import type GOSSIP
-- Menu 693 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 693 WHERE entry = 5512;


-- Importing creature gossip with entry 5509 (Kathrum Axehand) with import type GOSSIP
-- Menu 693 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 693 WHERE entry = 5509;


-- Importing creature gossip with entry 1302 (Bernard Gump) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872044', 'Imported from TC', 'We are the only supplier of healthy flowers in all of Stormwind. We provide beautiful blooms for any occasion.', 'We are the only supplier of healthy flowers in all of Stormwind. We provide beautiful blooms for any occasion.', '3406', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('703', '1872044');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('703', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 703 WHERE entry = 1302;


-- Importing creature gossip with entry 1303 (Felicia Gump) with import type GOSSIP
-- Menu 703 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 703 WHERE entry = 1303;


-- Importing creature gossip with entry 1326 (Sloan McCoy) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872045', 'Imported from TC', 'Speak quietly and with great care, the wrong word in these parts could get your throat cut. Now, what is it you need from me?', 'Speak quietly and with great care, the wrong word in these parts could get your throat cut. Now, what is it you need from me?', '3411', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('705', '1872045');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('705', '0', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 705 WHERE entry = 1326;


-- Importing creature gossip with entry 4974 (Aldwin Laughlin) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872046', 'Imported from TC', 'Welcome to the Stormwind Guild Registry offices. What can I help you with today?', 'Welcome to the Stormwind Guild Registry offices. What can I help you with today?', '3414', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('708', '1872046');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 708 WHERE entry = 4974;


-- Importing creature gossip with entry 733 (Sergeant Yohwa) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('733', 'Imported from TC', 'You watch where you step around here, $Glad:lass;.  You might not be a part of our outfit, but that doesn\'t mean I won\'t take a cane to you if you fall out of line!', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('733', '733');
UPDATE creature_template SET gossip_menu_id = 733 WHERE entry = 733;


-- Importing creature gossip with entry 7232 (Borgus Steelhand) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1123', 'Imported from TC', 'Can\'t ye see I\'m busy? This had better be good.', '', '3276', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1041', '1123');
-- Text 1133 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1041', '1133');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1041 WHERE entry = 7232;


-- Importing creature gossip with entry 4944 (Captain Garran Vimes) with import type GOSSIP
-- Text 1793 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1161', '1793');
-- Text 1794 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1162', '1794');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1161', '0', '0', 'What have you heard of the Shady Rest Inn?', '4227', '1', '1', '1162', NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1161 WHERE entry = 4944;


-- Importing creature gossip with entry 2352 (Innkeeper Anderson) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872047', 'Imported from TC', 'Welcome to my Inn, weary traveler. What can I do for you?', 'Welcome to my Inn, weary traveler. What can I do for you?', '16967', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1291', '1872047');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1291', '1', '5', 'Make this inn your home.', '2822', '8', '65536', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872048', 'Imported from TC', 'What can you do at an inn?$B$BWell when you stay at an inn, you rest very comfortably.  Because of this, you will become \"well rested\" much more quickly than you would in the wilderness.  When you are well rested, you learn more from experience.$B$BYou may also speak with any innkeeper to get a hearthstone, and can later use the hearthstone in order to quickly return to that inn.', 'What can you do at an inn?$B$BWell when you stay at an inn, you rest very comfortably.  Because of this, you will become \"well rested\" much more quickly than you would in the wilderness.  When you are well rested, you learn more from experience.$B$BYou may also speak with any innkeeper to get a hearthstone, and can later use the hearthstone in order to quickly return to that inn.', '4309', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1221', '1872048');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1291', '2', '0', 'What can I do at an inn?', '4308', '1', '1', '1221', NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1291', '3', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1291', '0', '0', 'Trick or Treat!', '10693', '1', '1', NULL, NULL, '0', '0', '', NULL);
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1291', '0', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1291', '0', '0', '0', '1', '0', '24755', '0', '0', '1', '0', '0', '', '(autoimported) ');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1291 WHERE entry = 2352;


-- Importing creature gossip with entry 7773 (Marli Wishrunner) with import type GOSSIP
-- Text 2053 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1421', '2053');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1421 WHERE entry = 7773;


-- Importing creature gossip with entry 7772 (Kalin Windflight) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872049', 'Imported from TC', 'To get down safely, I provide wings to those in need. ', '', '4518', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1422', '1872049');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1422', '0', '1', 'Can you help me get down?', '4519', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET gossip_menu_id = 1422 WHERE entry = 7772;


-- Importing creature gossip with entry 8767 (Sah'rhee) with import type GOSSIP
-- Text 2155 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1483', '2155');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1483 WHERE entry = 8767;


-- Importing creature gossip with entry 459 (Drusilla La Salle) with import type GOSSIP
-- Text 2175 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '1503', '2175', '0', '0', '15', '0', '256', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text if player is a Warlock');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1503', '2175');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872050', 'Imported from TC', 'The darkness does not embrace you, $c.  Cease your prattle and remove yourself from my sight!  Be gone!', 'The darkness does not embrace you, $c.  Cease your prattle and remove yourself from my sight!  Be gone!', '8300', '7', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '1503', '1872050', '0', '0', '15', '0', '1279', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text if player is not a Warlock');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1503', '1872050');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1503', '0', '3', 'I am interested in warlock training.', '2544', '5', '16', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872051', 'Imported from TC', 'Through rigorous retraining I have had to break many students of all they had previously learned so that I might teach them anew.  The service I offer becomes increasingly difficult to perform each time it is done.  It is for that reason that it becomes increasingly expensive with each retraining.  Do you desire that I break you of the talents you have learned?', 'Through rigorous retraining I have had to break many students of all they had previously learned so that I might teach them anew.  The service I offer becomes increasingly difficult to perform each time it is done.  It is for that reason that it becomes increasingly expensive with each retraining.  Do you desire that I break you of the talents you have learned?', '8260', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26320', '1872051');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('26320', '0', '1', 'Yes. I do.', '60961', '1', '1', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1503', '1', '0', 'I wish to unlearn my talents.', '62295', '16', '16', '26320', NULL, '0', '0', '', NULL);
-- Text 14391 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26321', '14391');
-- Text 14393 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26322', '14393');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `patch_min`) VALUES ('26321', '0', '0', 'Purchase a Dual Talent Specialization.', '33765', '18', '16', '26322', NULL, '0', '10000000', 'Are you sure you wish to purchase a Dual Talent Specialization?', NULL, '5');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `patch_min`) VALUES ('1503', '2', '0', 'Learn about Dual Talent Specialization.', '33762', '20', '1', '26321', NULL, '0', '0', '', NULL, '5');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1503', '0', '0', '0', '15', '0', '256', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip option if player is a Warlock');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1503', '1', '0', '0', '15', '0', '256', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip option if player is a Warlock');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1503', '2', '0', '0', '15', '0', '256', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip option if player is a Warlock');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1503 WHERE entry = 459;


-- Importing creature gossip with entry 7736 (Innkeeper Shyria) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872052', 'Imported from TC', 'Your body looks worn and your spirit weary. Rest yourself beneath our roof and allow Elune to guide your dreams.', 'Your body looks worn and your spirit weary. Rest yourself beneath our roof and allow Elune to guide your dreams.', '2827', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1581', '1872052');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1581', '2', '5', 'Make this inn your home.', '2822', '8', '65536', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1581', '3', '1', 'I want to browse your goods.', '3370', '3', '128', NULL, NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1581', '0', '0', 'Trick or Treat!', '10693', '1', '1', NULL, NULL, '0', '0', '', NULL);
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1581', '0', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('15', '1581', '0', '0', '0', '1', '0', '24755', '0', '0', '1', '0', '0', '', '(autoimported) ');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1581 WHERE entry = 7736;


-- Importing creature gossip with entry 11103 (Innkeeper Lyshaerya) with import type GOSSIP
-- Menu 1581 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1581 WHERE entry = 11103;


-- Importing creature gossip with entry 6738 (Innkeeper Kimlya) with import type GOSSIP
-- Menu 1581 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1581 WHERE entry = 6738;


-- Importing creature gossip with entry 16458 (Innkeeper Faralia) with import type GOSSIP
-- Menu 1581 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1581 WHERE entry = 16458;


-- Importing creature gossip with entry 3309 (Karus) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872053', 'Imported from TC', 'Besides the Grunts outside, my brothers and I fought at the battle of Hyjal. Any who seek to steal from the bank must face us first.', 'Besides the Grunts outside, my brothers and I fought at the battle of Hyjal. Any who seek to steal from the bank must face us first.', '4617', '1', '1', '0', '0', '0', '0', '0', '0', 'I guarantee this bank\'s security with my own blood, is that good enough for you?', 'I guarantee this bank\'s security with my own blood, is that good enough for you?', '4619', '1', '1', '0', '0', '0', '0', '0', '0', 'Would you rather trust your goods with a goblin?', 'Would you rather trust your goods with a goblin?', '4621', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1623', '1872053');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1623', '0', '6', 'I would like to check my deposit box.', '3398', '9', '131072', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1623 WHERE entry = 3309;


-- Importing creature gossip with entry 3320 (Soran) with import type GOSSIP
-- Menu 1623 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1623 WHERE entry = 3320;


-- Importing creature gossip with entry 1340 (Mountaineer Kadrell) with import type GOSSIP
-- Text 2289 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1632', '2289');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1632 WHERE entry = 1340;


-- Importing creature gossip with entry 9047 (Jenal) with import type GOSSIP
-- Text 2313 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1662', '2313');
-- Text 2314 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1661', '2314');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1662', '0', '0', 'What are you doing out here?', '27428', '1', '1', '1661', NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1662 WHERE entry = 9047;


-- Importing creature gossip with entry 8888 (Franclorn Forgewright) with import type GOSSIP
-- Text 2315 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1663', '2315');
-- Text 2316 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1664', '2316');
-- Text 2317 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1665', '2317');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872054', 'Imported from TC', 'My great hammer, Ironfel, was handed down to the new chief architect, Fineous Darkvire: A man of despicable motive and little talent.$B$BThis is where you come in, $N. I want him stopped. He is too inept to harness the true power of Ironfel, but should one come after him with the aptitude to use Ironfel to its full potential, the destruction they could cause with their creations would be disastrous to this world.$B', '', '4653', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1666', '1872054');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('2319', 'Imported from TC', 'You will find Fineous Darkvire beyond the Ring of Law, in the Hall of Crafting. Kill the miserable cur and recover Ironfel.$B$BTake Ironfel to the Shrine of Thaurissan and place the hammer in its rightful place: In the hands of the statue erected in my honor.$B$BWhen this is done, the compartment in which I stored the master key will open. More importantly, Ironfel will remain forever in my grasp. Should they make an attempt to remove the hammer; both the statue and hammer will shatter, lost forever.', '', '4655', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1667', '2319');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1666', '0', '0', 'Just show me the way, Franclorn.', '4654', '1', '1', '1667', NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1665', '0', '0', 'Fascinating. Please, tell me more.', '4652', '1', '1', '1666', NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1664', '0', '0', 'Continue please.', '21460', '1', '1', '1665', NULL, '0', '0', '', NULL);
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1663', '0', '0', 'Amazing! I\'ve never spoken to a ghost. I wish to learn!', '4648', '1', '1', '1664', NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1663 WHERE entry = 8888;


-- Importing creature gossip with entry 7766 (Tyrion) with import type GOSSIP
-- Text 2393 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1761', '2393');
-- Text 2394 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '1761', '2394', '0', '0', '9', '0', '434', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip menu text if quest 434 is taken');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1761', '2394');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1761 WHERE entry = 7766;


-- Importing creature gossip with entry 233 (Farmer Saldean) with import type GOSSIP
-- Text 2414 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1781', '2414');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1781', '0', '1', 'I\'d like to see what you have to sell.', '4713', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1781 WHERE entry = 233;


-- Importing creature gossip with entry 9467 (Miblon Snarltooth) with import type GOSSIP
-- Text 2496 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1844', '2496');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1844 WHERE entry = 9467;


-- Importing creature gossip with entry 9299 (Gaeriyan) with import type GOSSIP
-- Text 2568 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26323', '2568', '0', '0', '14', '0', '3912', '0', '0', '1', '0', '0', '', '(autoimported) Show gossip text 2568 if quest \'Meet at the Grave\' is taken');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26323', '2568', '0', '0', '14', '0', '3913', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 2568 if quest \'A Grave Situation\' is not taken');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26323', '2568');
-- Text 2567 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26323', '2567', '0', '0', '14', '0', '3912', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 2567 if quest \'Meet at the Grave\' is not taken');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26323', '2567');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('2569', 'Imported from TC', 'I have given you all the knowledge I have, $n. May luck be with you.', '', '4826', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26323', '2569', '0', '0', '14', '0', '3913', '0', '0', '1', '0', '0', '', '(autoimported) Show gossip text 2569 if quest \'A Grave Situation\' is taken');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26323', '2569');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26323 WHERE entry = 9299;


-- Importing creature gossip with entry 7879 (Quintis Jonespyre) with import type GOSSIP
-- Text 2575 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('1922', '2575');
INSERT INTO gossip_menu_option (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES ('1922', '0', '1', 'I\'d like to purchase more Tharlendris seeds.', '4851', '3', '128', NULL, NULL, '0', '0', '', NULL);
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 1922 WHERE entry = 7879;


-- Importing creature gossip with entry 1154 (Marek Ironheart) with import type GOSSIP
-- Text 3494 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2801', '3494', '0', '0', '14', '0', '271', '0', '0', '0', '0', '0', '', '(autoimported) SHOW gossip text 3494 if quest \'Vyrin\'s Revenge\' is not taken');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2801', '3494');
-- Text 3495 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2801', '3495', '0', '0', '8', '0', '271', '0', '0', '0', '0', '0', '', '(autoimported) SHOW gossip text 3495 if quest \'Vyrin\'s Revenge\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2801', '3495');
-- Text 3493 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2801', '3493', '0', '0', '9', '0', '271', '0', '0', '0', '0', '0', '', '(autoimported) SHOW gossip text 3493 if quest \'Vyrin\'s Revenge\' is taken');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2801', '3493', '0', '1', '28', '0', '271', '0', '0', '0', '0', '0', '', '(autoimported) SHOW gossip text 3493 if quest \'Vyrin\'s Revenge\' is complete');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2801', '3493');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 2801 WHERE entry = 1154;


-- Importing creature gossip with entry 10667 (Chromie) with import type GOSSIP
-- Text 3584 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2911', '3584');
-- Text 3585 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2911', '3585', '0', '0', '8', '0', '4972', '0', '0', '0', '0', '0', '', '(autoimported) Show Gossip Menu Text 3585 if Quest \'Counting Out Time\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2911', '3585');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 2911 WHERE entry = 10667;


-- Importing creature gossip with entry 10778 (Janice Felstone) with import type GOSSIP
-- Text 3668 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2961', '3668');
-- Text 3669 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '2961', '3669', '0', '0', '8', '0', '5051', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 3669 if quest \'Two Halves Become One\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('2961', '3669');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 2961 WHERE entry = 10778;


-- Importing creature gossip with entry 11016 (Captured Arko'narin) with import type GOSSIP
-- Text 3865 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3129', '3865');
-- Text 4113 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3129', '4113');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3129 WHERE entry = 11016;


-- Importing creature gossip with entry 11277 (Caer Darrow Citizen) with import type GOSSIP
-- Text 4118 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3362', '4118');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3362 WHERE entry = 11277;


-- Importing creature gossip with entry 11279 (Caer Darrow Guardsman) with import type GOSSIP
-- Text 4119 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3363', '4119');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3363 WHERE entry = 11279;


-- Importing creature gossip with entry 11281 (Caer Darrow Horseman) with import type GOSSIP
-- Menu 3363 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3363 WHERE entry = 11281;


-- Importing creature gossip with entry 11286 (Magistrate Marduke) with import type GOSSIP
-- Text 4130 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3372', '4130');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3372 WHERE entry = 11286;


-- Importing creature gossip with entry 11316 (Joseph Dirte) with import type GOSSIP
-- Text 4133 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3381', '4133');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3381 WHERE entry = 11316;


-- Importing creature gossip with entry 11555 (Gorn One Eye) with import type GOSSIP
-- Text 4399 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3625', '4399');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3625 WHERE entry = 11555;


-- Importing creature gossip with entry 10433 (Marduk Blackpool) with import type GOSSIP
-- Text 4447 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3649', '4447');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3649 WHERE entry = 10433;


-- Importing creature gossip with entry 3692 (Volcor) with import type GOSSIP
-- Text 3213 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3692', '3213');
-- Text 3214 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '3692', '3214', '0', '0', '8', '0', '993', '0', '0', '0', '0', '0', '', '(autoimported) Volcor - Show Gossip Menu Text 3214 if Quest 993 is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3692', '3214');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3692 WHERE entry = 3692;


-- Importing creature gossip with entry 3693 (Terenthis) with import type GOSSIP
-- Text 3334 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3693', '3334');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3693 WHERE entry = 3693;


-- Importing creature gossip with entry 10301 (Jaron Stoneshaper) with import type GOSSIP
-- Text 4573 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3761', '4573');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3761 WHERE entry = 10301;


-- Importing creature gossip with entry 10936 (Joseph Redpath) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('4778', 'Imported from TC', 'A darkness had fallen over my eyes, but you tore me from my unholy reverie.  You saved me, $N.$B$BAnd now I beg you... to forgive me.', '', '7455', '0', '1', '0', '20', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3861', '4778');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3861 WHERE entry = 10936;


-- Importing creature gossip with entry 10945 (Davil Crokford) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('4777', 'Imported from TC', 'The battle is over, and the people of Darrowshire are saved.$B$BThey are saved, $N, because of you.  You are truly a hero of Darrowshire.', '', '7454', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3922', '4777');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3922 WHERE entry = 10945;


-- Importing creature gossip with entry 11626 (Rigger Gizelton) with import type GOSSIP
-- Text 4815 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('3963', '4815');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 3963 WHERE entry = 11626;


-- Importing creature gossip with entry 738 (Private Thorsen) with import type GOSSIP
-- Text 4982 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4086', '4982');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4086 WHERE entry = 738;


-- Importing creature gossip with entry 1215 (Alchemist Mallory) with import type GOSSIP
-- Text 5013 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4110', '5013');
-- Text 5014 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4110', '5014');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4110 WHERE entry = 1215;


-- Importing creature gossip with entry 3603 (Cyndra Kindwhisper) with import type GOSSIP
-- Text 5022 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4111', '5022');
-- Text 5023 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4111', '5023');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4111 WHERE entry = 3603;


-- Importing creature gossip with entry 5500 (Tel'Athir) with import type GOSSIP
-- Text 5019 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4112', '5019');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4112 WHERE entry = 5500;


-- Importing creature gossip with entry 11041 (Milla Fairancora) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872055', 'Imported from TC', '', 'Elune-adore, my $gbrother:sister;. You are welcome here.', '7689', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4114', '1872055');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4114 WHERE entry = 11041;


-- Importing creature gossip with entry 3964 (Kylanna) with import type GOSSIP
-- Text 5051 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4117', '5051');
-- Text 5059 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4117', '5059');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4117 WHERE entry = 3964;


-- Importing creature gossip with entry 11042 (Sylvanna Forestmoon) with import type GOSSIP
-- Text 5064 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4125', '5064');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4125 WHERE entry = 11042;


-- Importing creature gossip with entry 3290 (Deek Fizzlebizz) with import type GOSSIP
-- Text 5115 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4136', '5115');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4136 WHERE entry = 3290;


-- Importing creature gossip with entry 11037 (Jenna Lemkenilli) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872056', 'Imported from TC', '', 'Hello there! My name\'s Jenna! And you? ', '7805', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4138', '1872056');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4138 WHERE entry = 11037;


-- Importing creature gossip with entry 7949 (Xylinnia Starshine) with import type GOSSIP
-- Text 5218 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4164', '5218');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4164 WHERE entry = 7949;


-- Importing creature gossip with entry 3967 (Aayndia Floralwind) with import type GOSSIP
-- Text 5360 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4211', '5360');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4211 WHERE entry = 3967;


-- Importing creature gossip with entry 1300 (Lawrence Schneider) with import type GOSSIP
-- Text 5416 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4262', '5416');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4262 WHERE entry = 1300;


-- Importing creature gossip with entry 4193 (Grondal Moonbreeze) with import type GOSSIP
-- Text 5419 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4263', '5419');
-- Text 5421 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4263', '5421');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4263 WHERE entry = 4193;


-- Importing creature gossip with entry 12656 (Thamarian) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872057', 'Imported from TC', 'On the northern dock, you can board a ship that will carry you to Rut\'theran Village and Darnassus.  From the southern dock, you can find passage across the Great Sea to Stormwind Harbor.  The dock to the west, at the end of the pier, leads to Azuremyst Isle, near the Exodar. Safe journeys to you!', '', '8106', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4308', '1872057');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4308 WHERE entry = 12656;


-- Importing creature gossip with entry 2057 (Huldar) with import type GOSSIP
-- Text 5494 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '4322', '5494', '0', '0', '29', '0', '1379', '20', '0', '0', '0', '0', '', '(autoimported) Show gossip text 5494 if Miran is nearby');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4322', '5494');
-- Text 5501 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '4322', '5501', '0', '0', '29', '0', '1379', '20', '0', '1', '0', '0', '', '(autoimported) Show gossip text 3522 if Miran is not nearby');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4322', '5501');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4322 WHERE entry = 2057;


-- Importing creature gossip with entry 5567 (Sellandus) with import type GOSSIP
-- Text 5519 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4344', '5519');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4344 WHERE entry = 5567;


-- Importing creature gossip with entry 12716 (Decedra Willham) with import type GOSSIP
-- Text 5579 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26324', '5579');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26324 WHERE entry = 12716;


-- Importing creature gossip with entry 12866 (Myriam Moonsinger) with import type GOSSIP
-- Text 5713 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4601', '5713');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4601 WHERE entry = 12866;


-- Importing creature gossip with entry 4578 (Josephine Lister) with import type GOSSIP
-- Text 5918 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4843', '5918');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4843 WHERE entry = 4578;


-- Importing creature gossip with entry 13018 (Nipsy) with import type GOSSIP
-- Text 5920 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('4845', '5920');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 4845 WHERE entry = 13018;


-- Importing creature gossip with entry 10219 (Gwennyth Bly'Leggonde) with import type GOSSIP
-- Text 5943 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26325', '5943');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26325 WHERE entry = 10219;


-- Importing creature gossip with entry 13278 (Duke Hydraxis) with import type GOSSIP
-- Text 6108 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5065', '6108');
-- Text 6109 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5065', '6109');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872058', 'Imported from TC', '$N, you are ready for the ultimate task.  The fiery runes etched on the floor of the molten core are conduits to the realm of fire, and their presence is both an affront to us and a source of power for the Firelords.  They must be extinguished!$B$BTo extinguish a rune, take a vial of aqual quintessence.  Pour it on the rune and its flame will go out, rendering it useless and weakening our enemy!', 'Hey there, friend. My name\'s Remy. I\'m from Redridge to the east, and came here looking for business, looking for business. You got any...got any??', '8703', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5065', '1872058');
-- Text 8541 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5065', '8541');
-- Text 8542 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5065', '8542');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5065 WHERE entry = 13278;


-- Importing creature gossip with entry 13447 (Corporal Noreg Stormpike) with import type GOSSIP
-- Text 6288 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5081', '6288');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5081 WHERE entry = 13447;


-- Importing creature gossip with entry 13322 (Hydraxian Honor Guard) with import type GOSSIP
-- Text 6161 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5109', '6161');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5109 WHERE entry = 13322;


-- Importing creature gossip with entry 13443 (Druid of the Grove) with import type GOSSIP
-- Text 6173 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5141', '6173');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5141 WHERE entry = 13443;


-- Importing creature gossip with entry 13442 (Arch Druid Renferal) with import type GOSSIP
-- Text 6174 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5142', '6174');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5142 WHERE entry = 13442;


-- Importing creature gossip with entry 13434 (Macey Jinglepocket) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872059', 'Imported from TC', 'Season\'s savings to you and yours!', 'Season\'s savings to you and yours!', '8778', '0', '1', '0', '0', '0', '0', '0', '0', 'Fresh from our farm to your plate - it\'s Smokywood Pastures wholesome goodness!', 'Fresh from our farm to your plate - it\'s Smokywood Pastures wholesome goodness!', '8779', '0', '3', '0', '0', '0', '0', '0', '0', '\'Tis the season for great bargains!  Fah la la la la!', '\'Tis the season for great bargains!  Fah la la la la!', '8780', '0', '1', '0', '0', '0', '0', '0', '0', 'Have a wondrous Feast of Winter Veil!  Start it off right with treats from Smokywood Pastures!', 'Have a wondrous Feast of Winter Veil!  Start it off right with treats from Smokywood Pastures!', '8781', '0', '3', '0', '0', '0', '0', '0', '0', 'Be sure to tell Greatfather Winter what secret surprise you\'d like to get this Winter Veil!', 'Be sure to tell Greatfather Winter what secret surprise you\'d like to get this Winter Veil!', '8782', '0', '1', '0', '0', '0', '0', '0', '0', 'Don\'t let the jingle of the bells take away from the jingle in your pocket!  Buy Smokywood Pastures treats for your Winter Veil festivities!', 'Don\'t let the jingle of the bells take away from the jingle in your pocket!  Buy Smokywood Pastures treats for your Winter Veil festivities!', '8783', '0', '3', '0', '0', '0', '0', '0', '0', 'If I had some chestnuts, you\'d better believe I\'d have them roasting over an open fire.  Mmmm!', 'If I had some chestnuts, you\'d better believe I\'d have them roasting over an open fire.  Mmmm!', '8784', '0', '1', '0', '0', '0', '0', '0', '0', 'Don\'t forget to stock up on wrapping paper this holiday season!  Nothing says \"Wondrous Winter Veil\" like wrapped armors and weapons for your friends and family.', 'Don\'t forget to stock up on wrapping paper this holiday season!  Nothing says \"Wondrous Winter Veil\" like wrapped armors and weapons for your friends and family.', '8785', '0', '3', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26326', '1872059');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26326 WHERE entry = 13434;


-- Importing creature gossip with entry 13435 (Khole Jinglepocket) with import type GOSSIP
-- Menu 5181 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5181 WHERE entry = 13435;


-- Importing creature gossip with entry 13577 (Stormpike Ram Rider Commander) with import type GOSSIP
-- Text 6313 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5281', '6313');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5281 WHERE entry = 13577;


-- Importing creature gossip with entry 13617 (Stormpike Stable Master) with import type GOSSIP
-- Text 6316 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5283', '6316');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5283 WHERE entry = 13617;


-- Importing creature gossip with entry 4967 (Archmage Tervosh) with import type GOSSIP
-- Text 6333 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5301', '6333');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5301 WHERE entry = 4967;


-- Importing creature gossip with entry 12238 (Zaetar's Spirit) with import type GOSSIP
-- Text 6336 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5304', '6336');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5304 WHERE entry = 12238;


-- Importing creature gossip with entry 13841 (Lieutenant Haggerdin) with import type GOSSIP
-- Text 6476 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5442', '6476');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5442 WHERE entry = 13841;


-- Importing creature gossip with entry 22628 (Lieutenant Haggerdin (1)) with import type GOSSIP
-- Menu 5442 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5442 WHERE entry = 22628;


-- Importing creature gossip with entry 10929 (Haleh) with import type GOSSIP
-- Text 6534 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5482', '6534');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5482 WHERE entry = 10929;


-- Importing creature gossip with entry 11491 (Old Ironbark) with import type GOSSIP
-- Text 6695 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5602', '6695');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5602 WHERE entry = 11491;


-- Importing creature gossip with entry 14322 (Stomper Kreeg) with import type GOSSIP
-- Text 6894 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26327', '6894');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26327 WHERE entry = 14322;


-- Importing creature gossip with entry 11486 (Prince Tortheldrin) with import type GOSSIP
-- Text 6909 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5736', '6909');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5736 WHERE entry = 11486;


-- Importing creature gossip with entry 14369 (Shen'dralar Zealot) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872060', 'Imported from TC', 'We mustn\'t anger the Master. His is a wrath that is cruel beyond measure.', 'We mustn\'t anger the Master. His is a wrath that is cruel beyond measure.', '9408', '0', '1', '1', '274', '0', '1', '2', '0', 'Keep your voice down, stranger. You do not want to disrupt the Master\'s meditations.', 'Keep your voice down, stranger. You do not want to disrupt the Master\'s meditations.', '9409', '0', '1', '20', '0', '0', '0', '0', '0', 'Many have come to this sanctuary, few have left.', 'Many have come to this sanctuary, few have left.', '9410', '0', '1', '1', '0', '0', '0', '0', '0', 'The Lorekeeper is an ancient. He has served under our glorious Queen!', 'The Lorekeeper is an ancient. He has served under our glorious Queen!', '9412', '0', '1', '1', '5', '0', '0', '0', '0', 'Books have been disappearing as of late. The Lorekeeper and his assistants are in a frenzy. They suspect foul play within Eldre\'Thalas.', 'Books have been disappearing as of late. The Lorekeeper and his assistants are in a frenzy. They suspect foul play within Eldre\'Thalas.', '9466', '0', '1', '1', '1', '0', '0', '0', '0', 'Has the Prince recanted the tale of the Ashbringer? It is our most favorite of his stories. His favorite is the followup to the Ashbringer. Of course, nobody has ever been able to get that story out of him.', 'Has the Prince recanted the tale of the Ashbringer? It is our most favorite of his stories. His favorite is the followup to the Ashbringer. Of course, nobody has ever been able to get that story out of him.', '9467', '0', '1', '1', '1', '0', '0', '0', '0', 'I suspect that the imps are stealing knowledge.', 'I suspect that the imps are stealing knowledge.', '9468', '0', '1', '1', '0', '0', '0', '0', '0', 'The Master holds great interest in the lair of Nefarian. If only someone would defeat the dragon and his minions and retell the tale to the Prince. I bet that would loosen the Prince\'s lips. The Ashbringer could be found!', 'The Master holds great interest in the lair of Nefarian. If only someone would defeat the dragon and his minions and retell the tale to the Prince. I bet that would loosen the Prince\'s lips. The Ashbringer could be found!', '9469', '0', '1', '1', '1', '5', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5737', '1872060');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5737 WHERE entry = 14369;


-- Importing creature gossip with entry 14374 (Scholar Runethorn) with import type GOSSIP
-- Text 6917 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5741', '6917');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5741 WHERE entry = 14374;


-- Importing creature gossip with entry 14324 (Cho'Rush the Observer) with import type GOSSIP
-- Text 6918 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5742', '6918');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5742 WHERE entry = 14324;


-- Importing creature gossip with entry 11444 (Gordok Mage-Lord) with import type GOSSIP
-- Text 6922 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5746', '6922');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5746 WHERE entry = 11444;


-- Importing creature gossip with entry 11441 (Gordok Brute) with import type GOSSIP
-- Menu 5746 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5746 WHERE entry = 11441;


-- Importing creature gossip with entry 14368 (Lorekeeper Lydros) with import type GOSSIP
-- Text 6925 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5747', '6925');
UPDATE creature_template SET gossip_menu_id = 5747 WHERE entry = 14368;


-- Importing creature gossip with entry 14383 (Lorekeeper Kildrath) with import type GOSSIP
-- Text 6927 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5748', '6927');
UPDATE creature_template SET gossip_menu_id = 5748 WHERE entry = 14383;


-- Importing creature gossip with entry 14381 (Lorekeeper Javon) with import type GOSSIP
-- Text 6939 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5755', '6939');
UPDATE creature_template SET gossip_menu_id = 5755 WHERE entry = 14381;


-- Importing creature gossip with entry 14382 (Lorekeeper Mykos) with import type GOSSIP
-- Text 6940 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5756', '6940');
UPDATE creature_template SET gossip_menu_id = 5756 WHERE entry = 14382;


-- Importing creature gossip with entry 20724 (Herald Amorlin) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872061', 'Imported from TC', 'Greetings and salutations, hero!  I have the latest news from both continents and points beyond for your consideration.', 'Greetings and salutations, hero!  I have the latest news from both continents and points beyond for your consideration.', '9551', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5782', '1872061');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5782 WHERE entry = 20724;


-- Importing creature gossip with entry 14305 (Human Orphan) with import type GOSSIP
-- Text 6992 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26328', '6992');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26328 WHERE entry = 14305;


-- Importing creature gossip with entry 14444 (Orcish Orphan) with import type GOSSIP
-- Text 6993 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5820', '6993');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5820 WHERE entry = 14444;


-- Importing creature gossip with entry 3305 (Grisha) with import type GOSSIP
-- Text 7123 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('5968', '7123');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 5968 WHERE entry = 3305;


-- Importing creature gossip with entry 22736 (Corporal Teeka Bloodsnarl (1)) with import type GOSSIP
-- Text 7434 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6261', '7434');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6261 WHERE entry = 22736;


-- Importing creature gossip with entry 22759 (Sergeant Durgen Stormpike (1)) with import type GOSSIP
-- Text 7435 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6262', '7435');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6262 WHERE entry = 22759;


-- Importing creature gossip with entry 13777 (Sergeant Durgen Stormpike) with import type GOSSIP
-- Menu 6262 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6262 WHERE entry = 13777;


-- Importing creature gossip with entry 14983 (Field Marshal Oslight) with import type GOSSIP
-- Text 7516 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6323', '7516');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6323 WHERE entry = 14983;


-- Importing creature gossip with entry 14984 (Sergeant Maclear) with import type GOSSIP
-- Text 7517 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6324', '7517');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6324 WHERE entry = 14984;


-- Importing creature gossip with entry 15022 (Deathstalker Mortis) with import type GOSSIP
-- Text 7555 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6362', '7555');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6362 WHERE entry = 15022;


-- Importing creature gossip with entry 15077 (Riggle Bassbait) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872062', 'Imported from TC', 'We already have a winner for our contest, but Jang is willing to purchase any Tastyfish that you have fished up!$B$BThere is always next Sunday for you to show your Angling skill!', '', '10600', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6421', '1872062');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6421 WHERE entry = 15077;


-- Importing creature gossip with entry 15079 (Fishbot 5000) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872063', 'Imported from TC', '*Whirr* *CLANK*$B$BI have been instructed to accept high quality rare fish and return valuable Nat Pagle fishing accessories.   Fish Requested.$B$B*THUMP* *Buzz*$B', '', '10493', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6447', '1872063');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6447 WHERE entry = 15079;


-- Importing creature gossip with entry 2804 (Kurden Bloodclaw) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872064', 'Imported from TC', 'I am sorry $c, but you are not seasoned enough yet to enter Warsong Gulch.', 'I am sorry $c, but you are not seasoned enough yet to enter Warsong Gulch.', '10471', '0', '1', '1', '274', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6462', '1872064');
-- Text 7655 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6462', '7655');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6462 WHERE entry = 2804;


-- Importing creature gossip with entry 14982 (Lylandris) with import type GOSSIP
-- Text 7599 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6463', '7599');
-- Text 7656 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6463', '7656');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6463 WHERE entry = 14982;


-- Importing creature gossip with entry 14942 (Kartra Bloodsnarl) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872065', 'Imported from TC', 'I am sorry $c, but you need to be more experienced before you can survive in Alterac Valley.', 'I am sorry $c, but you need to be more experienced before you can survive in Alterac Valley.', '10481', '0', '1', '1', '274', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6467', '1872065');
-- Text 7660 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6467', '7660');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6467 WHERE entry = 14942;


-- Importing creature gossip with entry 15006 (Deze Snowbane) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872066', 'Imported from TC', 'I am sorry $c, but you need to be more powerful before I can allow you to enter Arathi Basin.', 'I am sorry $c, but you need to be more powerful before I can allow you to enter Arathi Basin.', '10495', '0', '1', '1', '274', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6470', '1872066');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6470 WHERE entry = 15006;


-- Importing creature gossip with entry 15116 (Grinkle) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872067', 'Imported from TC', 'Heya Heya Heya!   If you want to test your fishing skills then I have just the place for you!   This Sunday we have a fishing tournament in Stranglethorn!$B$B', '', '10548', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '6476', '1872067', '0', '0', '27', '0', '35', '3', '0', '0', '0', '0', '', '(autoimported) Gossip text 1872067 for NPC Grinkle requires level to be 35 or higher');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6476', '1872067');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872068', 'Imported from TC', 'Hey Hey Hey!  I am here to promote a fabulous fishing tournament in Stranglethorn but you need to be tough enough to survive there and have enough skill in fishing to fish those waters.$B$BCome back later when you are more qualified!$B$B', '', '10547', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '6476', '1872068', '0', '0', '27', '0', '35', '2', '0', '0', '0', '0', '', '(autoimported) Gossip text 1872068 for NPC Grinkle requires level to be lower than 35');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6476', '1872068');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6476 WHERE entry = 15116;


-- Importing creature gossip with entry 22647 (Frostwolf Emissary (1)) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872069', 'Imported from TC', 'You need to be stronger, $c before you can help us in Alterac Valley.', 'You need to be stronger, $c before you can help us in Alterac Valley.', '10557', '1', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6484', '1872069');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872070', 'Imported from TC', 'Hail, $c.  There is a great need and even greater honor and renown to be had in Alterac Valley at this time. Will you join our fight against the Stormpike Guard?', 'Hail, $c.  There is a great need and even greater honor and renown to be had in Alterac Valley at this time. Will you join our fight against the Stormpike Guard?', '10558', '1', '1', '0', '1', '0', '1', '0', '1', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6484', '1872070');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872071', 'Imported from TC', 'Strength and honor, $c. We need eager soldiers like you in Alterac Valley; the territory of the Frostwolves must be defended! Will you return to Azeroth and lend your aid?', 'Strength and honor, $c. We need eager soldiers like you in Alterac Valley; the territory of the Frostwolves must be defended! Will you return to Azeroth and lend your aid?', '19756', '1', '1', '0', '1', '0', '1', '0', '1', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6484', '1872071');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6484 WHERE entry = 22647;


-- Importing creature gossip with entry 15106 (Frostwolf Emissary) with import type GOSSIP
-- Menu 6484 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6484 WHERE entry = 15106;


-- Importing creature gossip with entry 15119 (Barrus) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872072', 'Imported from TC', 'Well met master of Rod and Reel!   If you be willing to test your skills against the best fisher-folk in the lands then listen up!   On this very Sunday there be a contest in Stranglethorn where the master angler will be chosen!$B$B', '', '10595', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26329', '1872072', '0', '0', '27', '0', '35', '3', '0', '0', '0', '0', '', '(autoimported) Gossip text 7677 for NPC Barrus requires level to be 35 or higher');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26329', '1872072');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('7711', 'Imported from TC', 'Greetings!  I am here to promote a tournament to test your fishing skill in Stranglethorn.  To participate you need enough might to survive there and have enough skill in fishing to coax the fish from the water.$B$B Come back later when you are more qualified!', '', '10596', '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26329', '7711', '0', '0', '27', '0', '35', '2', '0', '0', '0', '0', '', '(autoimported) Gossip text 7676 for NPC Barrus requires level to be lower than 35');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26329', '7711');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26329 WHERE entry = 15119;


-- Importing creature gossip with entry 15078 (Jang) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872073', 'Imported from TC', '', 'Fishmaster Bassbait has had the waters here seeded with high quality Tastyfish so he can have his tournament.$B$BOnce he gets his winner, I will give you silver for each five Tastyfish you bring me!$B', '10571', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26330', '1872073');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26330 WHERE entry = 15078;


-- Importing creature gossip with entry 14733 (Sentinel Farsong) with import type GOSSIP
-- Text 7725 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6523', '7725');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6523 WHERE entry = 14733;


-- Importing creature gossip with entry 15197 (Darkcaller Yanka) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872074', 'Imported from TC', '', 'Welcome to the Wickerman Festival, $c!$B$BOn the observance of Hallow\'s End, the Forsaken burn a wickerman in honor of this, the most revered of occasions.  It was on this day that the Banshee Queen herself delivered us from the clutches of the Lich King and the Scourge.  We have remained free ever since.  Those who would have seen us fall are repaid in full... with vengeance!$B$BThe burning of the Wickerman begins at 8:00 PM!', '10671', '0', '1', '0', '4', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6537', '1872074');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6537 WHERE entry = 15197;


-- Importing creature gossip with entry 15199 (Sergeant Hartman) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872075', 'Imported from TC', 'You there!  You look able bodied... we need your aid in defending Southshore!$B$BThe blasted Forsaken are all riled up because it\'s Hallow\'s End.  Their \"celebration\" will no doubt be leading them right to our doorstep.  If you\'ve got the time, we really could use your help!', '', '10673', '0', '1', '0', '5', '0', '1', '0', '1', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6538', '1872075');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6538 WHERE entry = 15199;


-- Importing creature gossip with entry 15293 (Aendel Windspear) with import type GOSSIP
-- Text 7783 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6568', '7783');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6568 WHERE entry = 15293;


-- Importing creature gossip with entry 15503 (Kandrostrasz) with import type GOSSIP
-- Text 7900 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6645', '7900');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6645 WHERE entry = 15503;


-- Importing creature gossip with entry 15504 (Vethsera) with import type GOSSIP
-- Text 7911 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6655', '7911');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6655 WHERE entry = 15504;


-- Importing creature gossip with entry 15380 (Arygos) with import type GOSSIP
-- Text 8111 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26331', '8111');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26331 WHERE entry = 15380;


-- Importing creature gossip with entry 15379 (Caelestrasz) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8112', 'Imported from TC', 'Our freedom would have meant nothing if C\'Thun had survived. You are truly a champion of prophecy.', '', '11391', '0', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6794', '8112');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6794 WHERE entry = 15379;


-- Importing creature gossip with entry 15378 (Merithra of the Dream) with import type GOSSIP
-- Text 8113 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6795', '8113');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6795 WHERE entry = 15378;


-- Importing creature gossip with entry 11558 (Kernda) with import type GOSSIP
-- Text 8124 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6801', '8124');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6801 WHERE entry = 11558;


-- Importing creature gossip with entry 15909 (Fariel Starsong) with import type GOSSIP
-- Text 8213 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6924', '8213');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6924 WHERE entry = 15909;


-- Importing creature gossip with entry 15906 (Ironforge Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8219', 'Imported from TC', 'The people in Nighthaven sure know how to party!', 'The people in Nighthaven sure know how to party!', '11669', '0', '1', '0', '5', '0', '0', '0', '0', 'These fireworks sure make a nice BOOM, don\'t you think?', 'These fireworks sure make a nice BOOM, don\'t you think?', '11670', '0', '1', '0', '5', '0', '0', '0', '0', 'This moonlight will send you to Ironforge.  But who\'d want to leave this party?', 'This moonlight will send you to Ironforge.  But who\'d want to leave this party?', '11672', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6929', '8219');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6929 WHERE entry = 15906;


-- Importing creature gossip with entry 15905 (Darnassus Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872076', 'Imported from TC', 'The lunar celebration this year is quite magnificent!', 'The lunar celebration this year is quite magnificent!', '11673', '0', '1', '0', '5', '0', '0', '0', '0', 'They say these fireworks use the power of the moon!', 'They say these fireworks use the power of the moon!', '11674', '0', '1', '0', '5', '0', '0', '0', '0', 'This beam of greater moonlight will send you to Darnassus.', 'This beam of greater moonlight will send you to Darnassus.', '11675', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6930', '1872076');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6930 WHERE entry = 15905;


-- Importing creature gossip with entry 15694 (Stormwind Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8218', 'Imported from TC', 'They know how to throw a party here in Moonglade.  Who would have thought so?', 'They know how to throw a party here in Moonglade.  Who would have thought so?', '11665', '0', '1', '0', '1', '0', '0', '0', '0', 'Try these fireworks launchers.  They\'re a blast!', 'Try these fireworks launchers.  They\'re a blast!', '11666', '0', '1', '0', '5', '0', '0', '0', '0', 'This beam of moonlight leads to Stormwind.  Use your invitation when you\'re ready.', 'This beam of moonlight leads to Stormwind.  Use your invitation when you\'re ready.', '11667', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6931', '8218');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6931 WHERE entry = 15694;


-- Importing creature gossip with entry 15908 (Orgrimmar Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872077', 'Imported from TC', 'If I didnt know better, I\'d say this party was organized by orcs!', 'If I didnt know better, I\'d say this party was organized by orcs!', '11676', '0', '1', '0', '5', '0', '0', '0', '0', 'Did you see?  That last firework almost caught that dwarf\'s shirt on fire!  Hah!', 'Did you see?  That last firework almost caught that dwarf\'s shirt on fire!  Hah!', '11677', '0', '1', '0', '153', '0', '0', '0', '0', 'This moon beam will send you to Orgrimmar.  Use your invitation when you want to go.', 'This moon beam will send you to Orgrimmar.  Use your invitation when you want to go.', '11678', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6932', '1872077');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6932 WHERE entry = 15908;


-- Importing creature gossip with entry 15719 (Thunder Bluff Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872078', 'Imported from TC', 'I love the Lunar Festival!  It\'s a great place to let your hooves kick up dust!', 'I love the Lunar Festival!  It\'s a great place to let your hooves kick up dust!', '11682', '0', '1', '0', '5', '0', '0', '0', '0', 'I don\'t know how these fireworks work, but they sure are pretty!', 'I don\'t know how these fireworks work, but they sure are pretty!', '11683', '0', '1', '0', '5', '0', '0', '0', '0', 'This moonlight will take you to Thunder Bluff.  Use it when you want to leave the festival.', 'This moonlight will take you to Thunder Bluff.  Use it when you want to leave the festival.', '11684', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26332', '1872078');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26332 WHERE entry = 15719;


-- Importing creature gossip with entry 15907 (Undercity Reveler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872079', 'Imported from TC', 'There\'s nothing like a little Lunar celebration to warm your bones!', 'There\'s nothing like a little Lunar celebration to warm your bones!', '11679', '0', '1', '0', '5', '0', '0', '0', '0', 'You have to be careful lighting fireworks when you can\'t feel your fingers...', 'You have to be careful lighting fireworks when you can\'t feel your fingers...', '11680', '0', '1', '0', '1', '0', '0', '0', '0', 'This beam of moonlight leads to the Undercity.  Retire when ready.', 'This beam of moonlight leads to the Undercity.  Retire when ready.', '11681', '0', '1', '0', '1', '0', '0', '0', '0', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', 'Don\'t try to teleport to one of your enemies\' cities.  It won\'t work - we may all be friendly here, but back home old grudges remain...', '11668', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6934', '1872079');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6934 WHERE entry = 15907;


-- Importing creature gossip with entry 16002 (Colara Dean) with import type GOSSIP
-- Text 8243 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6981', '8243');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8282', 'Imported from TC', '', 'You look like you\"ve had your heart broken. Come back when you\"re not so sad.', '11790', '0', '1', '0', '25', '0', '153', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6981', '8282');
-- Text 8285 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('6981', '8285');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 6981 WHERE entry = 16002;


-- Importing creature gossip with entry 15471 (Lieutenant General Andorov) with import type GOSSIP
-- Text 7883 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7048', '7883');
-- Text 8304 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7048', '8304');
-- Text 8305 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7048', '8305');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7048 WHERE entry = 15471;


-- Importing creature gossip with entry 16075 (Kwee Q. Peddlefeet) with import type GOSSIP
-- Text 8313 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7054', '8313');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7054 WHERE entry = 16075;


-- Importing creature gossip with entry 16013 (Deliana) with import type GOSSIP
-- Text 8333 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7071', '8333');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7071 WHERE entry = 16013;


-- Importing creature gossip with entry 16016 (Anthion Harmon) with import type GOSSIP
-- Text 8334 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7072', '8334', '0', '0', '8', '0', '8946', '0', '0', '1', '0', '0', '', '(autoimported) Show gossip text 8334 if quest \'Proof of Life\' is not rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7072', '8334');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8337', 'Imported from TC', 'Ysida lives thanks to your courage, $n.  I am deeply in your debt and shall assist you as much as it is in my power.', '', '11933', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7072', '8337', '0', '0', '8', '0', '8946', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8337 if quest \'Proof of Life\' is rewarded');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7072', '8337', '0', '0', '8', '0', '9015', '0', '0', '1', '0', '0', '', '(autoimported) Show gossip text 8337 if quest \'The Challenge\' is NOT rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7072', '8337');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8338', 'Imported from TC', 'I guess it\'s all just about over, right $n? I\'m going to miss you... it\'ll be pretty lonely up here without someone to talk to.$B$BBut that doesn\'t mean that you can\'t come back to see me in the future!', '', '11912', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7072', '8338', '0', '0', '8', '0', '9015', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8338 if quest \'The Challenge\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7072', '8338');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7072 WHERE entry = 16016;


-- Importing creature gossip with entry 16031 (Ysida Harmon) with import type GOSSIP
-- Text 8344 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7091', '8344');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7091 WHERE entry = 16031;


-- Importing creature gossip with entry 16240 (Arcanist Janeda) with import type GOSSIP
-- Text 8470 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26333', '8470');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26333 WHERE entry = 16240;


-- Importing creature gossip with entry 16291 (Magister Quallestis) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872080', 'Imported from TC', 'Where could she possibly be? I sent my apprentice, Shatharia, to the Underlight Mines to the west quite some time ago. We need those ore samples back, and fast or our experiment is going to fail!', '', '12183', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26334', '1872080', '0', '0', '8', '0', '9207', '0', '0', '1', '0', '0', '', '(autoimported) Gossip text requires quest Underlight Ore Samples NOT rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26334', '1872080');
-- Text 8475 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '26334', '8475', '0', '0', '8', '0', '9207', '0', '0', '0', '0', '0', '', '(autoimported) Gossip text requires quest Underlight Ore Samples rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26334', '8475');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26334 WHERE entry = 16291;


-- Importing creature gossip with entry 16203 (Ranger Vynna) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872081', 'Imported from TC', '', 'Sin\'dorei have lost far too much in the past years.  Our land, our homes, even our identities...$B$BIt\'s about time we started taking it all back.', '12185', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26335', '1872081');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26335 WHERE entry = 16203;


-- Importing creature gossip with entry 16365 (Master Craftsman Omarion) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8506', 'Imported from TC', 'There are those who would say that the Argent Crusade is a thing of the past.  I am one of them, but not because the Scourge have fallen.$b$bNo... it is because of their stubborn ways.  They\'ve become too much like the church that they seceded from: too rigid, too governed by honor and order.  This is what I\'m trying to tell Barthalomew, anyhow.$b$bSure, it may sound a bit jingoistic, but it makes for a good debate.', '', '12246', '0', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7215', '8506');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7215 WHERE entry = 16365;


-- Importing creature gossip with entry 16399 (Bloodsail Traitor) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872082', 'Imported from TC', 'Gangway, ye scurvy dog!  Revilgaz has spared me from the gallows but my punishment might be nearly as bad.', 'Gangway, ye scurvy dog!  Revilgaz has spared me from the gallows but my punishment might be nearly as bad.', '12341', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7236', '1872082');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7236 WHERE entry = 16399;


-- Importing creature gossip with entry 16418 (Mupsi Shacklefridd) with import type GOSSIP
-- Text 8537 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26336', '8537');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26336 WHERE entry = 16418;


-- Importing creature gossip with entry 16204 (Magister Idonis) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872083', 'Imported from TC', 'Dar\'Khan thinks we\'ll roll over for the Scourge without putting up a fight.  We shall prove him wrong, $N.', '', '12394', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26337', '1872083');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26337 WHERE entry = 16204;


-- Importing creature gossip with entry 16480 (Apprentice Vor'el) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872084', 'Imported from TC', 'You don\'t see too many apprentices around these parts, do you?  I\'m beginning to think it\'s because the magisters give us all the dangerous work they\'d rather not do themselves.', '', '12447', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7259', '1872084');
UPDATE creature_template SET gossip_menu_id = 7259 WHERE entry = 16480;


-- Importing creature gossip with entry 16862 (Silanna) with import type GOSSIP
-- Text 8684 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7316', '8684');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7316 WHERE entry = 16862;


-- Importing creature gossip with entry 16817 (Festival Loremaster) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872085', 'Imported from TC', 'Enjoying the festival, $c?$b$bWhile the Flame Wardens tend to the fires of the present, I am more of a historian, keeping close the festivals past. I\'m also, of course, documenting this year\'s festivities. Things are going well thus far, don\'t you think?$b$bYou know, $n, there is power inherent in all festival fires burning throughout the holiday. We\'re taking care of ours, but I\'m certain there are sacred flames burning deep within our enemy\'s cities...', 'Enjoying the festival, $c?$b$bWhile the Flame Wardens tend to the fires of the present, I am more of a historian, keeping close the festivals past. I\'m also, of course, documenting this year\'s festivities. Things are going well thus far, don\'t you think?$b$bYou know, $n, there is power inherent in all festival fires burning throughout the holiday. We\'re taking care of ours, but I\'m certain there are sacred flames burning deep within our enemy\'s cities...', '12860', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7326', '1872085');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7326 WHERE entry = 16817;


-- Importing creature gossip with entry 16858 (Grelag) with import type GOSSIP
-- Text 8767 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7348', '8767');
-- Text 9619 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7348', '9619');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7348 WHERE entry = 16858;


-- Importing creature gossip with entry 17103 (Emissary Taluun) with import type GOSSIP
-- Text 8796 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7363', '8796');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7363 WHERE entry = 17103;


-- Importing creature gossip with entry 17106 (Vindicator Palanaar) with import type GOSSIP
-- Text 8802 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7367', '8802', '0', '0', '16', '0', '767', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8802 if player is not a Draenei');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7367', '8802');
-- Text 8803 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7367', '8803', '0', '0', '16', '0', '1024', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8803 if player is a Draenei');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7367', '8803');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7367 WHERE entry = 17106;


-- Importing creature gossip with entry 17127 (Anchorite Avuun) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8812', 'Imported from TC', 'Greetings.  I am Anchorite Avuun of the draenei.  I am here studying the Lost Ones of the Fallow Sanctuary in the hope of being able to find a cure for their condition.', '', '13504', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7374', '8812');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8814', 'Imported from TC', 'Once more, thank you for your assistance with the cursed Lost Ones.  Without your help I would never have been able to continue my research.', '', '13506', '0', '1', '1', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7374', '8814', '0', '0', '8', '0', '9448', '0', '0', '0', '0', '0', '', '(autoimported) Show Gossip Menu Text 8814 if Quest \'Mercy for the Cursed\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7374', '8814');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7374 WHERE entry = 17127;


-- Importing creature gossip with entry 17406 (Artificer) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872086', 'Imported from TC', 'I\'m very busy right now, young $c. Please excuse me.', 'I\'m very busy right now, young $c. Please excuse me.', '13568', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26338', '1872086');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26338 WHERE entry = 17406;


-- Importing creature gossip with entry 17223 (Ambassador Rualeth) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872087', 'Imported from TC', 'Good day to you, $c.$B$BI\'m waiting to hear from one of the High Thane\'s advisors about an audience. It was a struggle just to get the opportunity to present my credentials.$B$BI\'ve heard the Wildhammer described as \'wary of outsiders,\' but that\'s proven to be quite an understatement.  Hopefully, things will improve once I\'ve had the opportunity to meet with Featherbeard, whom I\'m told favors closer ties with the Alliance.', '', '13628', '0', '1', '0', '2', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7386', '1872087');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7386 WHERE entry = 17223;


-- Importing creature gossip with entry 16798 (Provisioner Anir) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872088', 'Imported from TC', '', 'Do you need something, $c?  Perhaps I can assist you.', '13663', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7396', '1872088');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7396 WHERE entry = 16798;


-- Importing creature gossip with entry 16239 (Magister Kaendris) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872089', 'Imported from TC', 'The Ghostlands will not be won back by military force alone, $N.  That\'s where we magisters come in.$B$BOur guile and knowledge of the arcane are unmatched.', '', '13697', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26339', '1872089');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26339 WHERE entry = 16239;


-- Importing creature gossip with entry 17291 (Architect Nemos) with import type GOSSIP
-- Text 8873 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26340', '8873');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26340 WHERE entry = 17291;


-- Importing creature gossip with entry 17287 (Sentinel Luciel Starwhisper) with import type GOSSIP
-- Text 8874 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7405', '8874');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7405 WHERE entry = 17287;


-- Importing creature gossip with entry 3848 (Kayneth Stillwind) with import type GOSSIP
-- Text 8875 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26341', '8875');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26341 WHERE entry = 3848;


-- Importing creature gossip with entry 17303 (Vindicator Vedaar) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8876', 'Imported from TC', 'Can you feel the evil to the south?  By the Hand, it must be cleansed!', '', '13766', '0', '1', '0', '6', '0', '274', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7407', '8876');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8878', 'Imported from TC', 'While you\'ve dealt the demonic evil from the south a serious blow, I still sense trouble from that region.', '', '13769', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7407', '8878', '0', '0', '8', '0', '9516', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8878 if quest \'Destroy the Legion\' is rewarded');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7407', '8878', '0', '0', '8', '0', '9522', '0', '0', '1', '0', '0', '', '(autoimported) Show gossip text 8878 if quest \'Never Again!\' is NOT rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7407', '8878');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8877', 'Imported from TC', 'Now that the demons have been dealt with, we can proceed with the establishment of a new base here.', '', '13767', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7407', '8877', '0', '0', '8', '0', '9516', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8877 if quest \'Destroy the Legion\' is rewarded');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7407', '8877', '0', '0', '8', '0', '9522', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8877 if quest \'Never Again!\' is rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7407', '8877');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7407 WHERE entry = 17303;


-- Importing creature gossip with entry 17412 (Phaedra) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('8939', 'Imported from TC', '', 'Welcome to Forest Song, $c. If there is anything I can do for you, please let me know.', '13959', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7410', '8939');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7410 WHERE entry = 17412;


-- Importing creature gossip with entry 17493 (Stone Guard Stok'ton) with import type GOSSIP
-- Text 8962 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7420', '8962');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7420 WHERE entry = 17493;


-- Importing creature gossip with entry 17443 (Kurz the Revelator) with import type GOSSIP
-- Text 8983 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7432', '8983');
-- Text 8994 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7432', '8994', '0', '0', '8', '0', '9544', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8994 if player has quest 9544 rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7432', '8994');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7432 WHERE entry = 17443;


-- Importing creature gossip with entry 17445 (Stillpine the Younger) with import type GOSSIP
-- Text 8983 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7433', '8983');
-- Text 8996 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '7433', '8996', '0', '0', '8', '0', '9544', '0', '0', '0', '0', '0', '', '(autoimported) Show gossip text 8996 if player has quest 9544 rewarded');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7433', '8996');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7433 WHERE entry = 17445;


-- Importing creature gossip with entry 17686 (Researcher Cornelius) with import type GOSSIP
-- Text 9045 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7466', '9045');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7466 WHERE entry = 17686;


-- Importing creature gossip with entry 17637 (Mack Diver) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('9110', 'Imported from TC', 'Say, you happen to see that arch in the lake to the west?  I wonder where that goes?  I should go divin\" over there.', '', '14546', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7513', '9110');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7513 WHERE entry = 17637;


-- Importing creature gossip with entry 17885 (Earthbinder Rayge) with import type GOSSIP
-- Text 9121 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7521', '9121');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7521 WHERE entry = 17885;


-- Importing creature gossip with entry 17827 (Claw) with import type GOSSIP
-- Text 9125 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7525', '9125');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7525 WHERE entry = 17827;


-- Importing creature gossip with entry 17982 (Demolitionist Legoso) with import type GOSSIP
-- Text 9142 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7538', '9142');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7538 WHERE entry = 17982;


-- Importing creature gossip with entry 17923 (Fahssn) with import type GOSSIP
-- Text 9203 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26342', '9203');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26342 WHERE entry = 17923;


-- Importing creature gossip with entry 18013 (Shadow Hunter Denjai) with import type GOSSIP
-- Text 9248 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7601', '9248');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7601 WHERE entry = 18013;


-- Importing creature gossip with entry 18014 (Witch Doctor Tor'gash) with import type GOSSIP
-- Text 9249 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7602', '9249');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7602 WHERE entry = 18014;


-- Importing creature gossip with entry 18221 (Holaaru) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('9270', 'Imported from TC', 'Hello, $c.  I am what is referred to as one of the Broken.  Like the Lost Ones, though not as badly as they, we were afflicted with fel energies that transformed us from our original draenei forms.', '', '14982', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7618', '9270');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7618 WHERE entry = 18221;


-- Importing creature gossip with entry 18066 (Farseer Kurkush) with import type GOSSIP
-- Text 9271 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7619', '9271');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7619 WHERE entry = 18066;


-- Importing creature gossip with entry 18229 (Saurfang the Younger) with import type GOSSIP
-- Text 9275 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7621', '9275');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7621 WHERE entry = 18229;


-- Importing creature gossip with entry 18067 (Farseer Corhuk) with import type GOSSIP
-- Text 9276 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7622', '9276');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7622 WHERE entry = 18067;


-- Importing creature gossip with entry 18068 (Farseer Margadesh) with import type GOSSIP
-- Text 9277 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7623', '9277');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7623 WHERE entry = 18068;


-- Importing creature gossip with entry 18278 (Pilot Marsha) with import type GOSSIP
-- Text 9290 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7628', '9290');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7628 WHERE entry = 18278;


-- Importing creature gossip with entry 18294 (Kristen Dipswitch) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('9292', 'Imported from TC', '', 'I am SO glad that evil Gankly Rottenfist got his just desserts!  I have lots of skins if you want some to give to Harold.', '15113', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7629', '9292');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7629 WHERE entry = 18294;


-- Importing creature gossip with entry 18302 (Matron Drakia) with import type GOSSIP
-- Text 9297 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7630', '9297');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7630 WHERE entry = 18302;


-- Importing creature gossip with entry 20812 (Corki) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872090', 'Imported from TC', 'Get me out of here, $r!', '', '15291', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7674', '1872090');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7674 WHERE entry = 20812;


-- Importing creature gossip with entry 18407 (Warden Bullrok) with import type GOSSIP
-- Text 9375 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7686', '9375');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7686 WHERE entry = 18407;


-- Importing creature gossip with entry 18414 (Elder Yorley) with import type GOSSIP
-- Text 9376 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7687', '9376');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7687 WHERE entry = 18414;


-- Importing creature gossip with entry 18415 (Elder Ungriz) with import type GOSSIP
-- Text 9377 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7688', '9377');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7688 WHERE entry = 18415;


-- Importing creature gossip with entry 18566 (Shadowstalker Kaide) with import type GOSSIP
-- Text 9495 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7754', '9495');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7754 WHERE entry = 18566;


-- Importing creature gossip with entry 18720 (Shadowmaster Grieve) with import type GOSSIP
-- Text 9524 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7776', '9524');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7776 WHERE entry = 18720;


-- Importing creature gossip with entry 19341 (Grutah) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872091', 'Imported from TC', 'Have you honed your skills enough to learn what I have to teach?', 'Have you honed your skills enough to learn what I have to teach?', '15847', '0', '1', '0', '396', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7809', '1872091');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7809 WHERE entry = 19341;


-- Importing creature gossip with entry 16615 (Novia) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872092', 'Imported from TC', 'Welcome to the Royal Exchange Bank, $g sir : ma\'am;. How may I help you?', 'Welcome to the Royal Exchange Bank, $g sir : ma\'am;. How may I help you?', '15868', '0', '100', '0', '0', '0', '0', '0', '0', 'Greetings. How may we of the Royal Exchange Bank assist you today?', 'Greetings. How may we of the Royal Exchange Bank assist you today?', '15872', '0', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7811', '1872092');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7811 WHERE entry = 16615;


-- Importing creature gossip with entry 16616 (Periel) with import type GOSSIP
-- Menu 7811 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7811 WHERE entry = 16616;


-- Importing creature gossip with entry 17631 (Ceera) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872093', 'Imported from TC', 'Thank you for choosing the Bank of Silvermoon. What may I do for you today?', 'Thank you for choosing the Bank of Silvermoon. What may I do for you today?', '15869', '0', '100', '0', '0', '0', '0', '0', '0', 'Welcome to the Bank of Silvermoon, $g sir : ma\'am;. Would you care to access the depository?', 'Welcome to the Bank of Silvermoon, $g sir : ma\'am;. Would you care to access the depository?', '15873', '0', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7812', '1872093');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7812 WHERE entry = 17631;


-- Importing creature gossip with entry 17632 (Elana) with import type GOSSIP
-- Menu 7812 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7812 WHERE entry = 17632;


-- Importing creature gossip with entry 18775 (Lebowski) with import type GOSSIP
-- Text 9546 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7820', '9546');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7820 WHERE entry = 18775;


-- Importing creature gossip with entry 17558 (Caza'rez) with import type GOSSIP
-- Text 9574 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7825', '9574');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7825 WHERE entry = 17558;


-- Importing creature gossip with entry 18790 (Gatewatcher Aendor) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872094', 'Imported from TC', 'There is nothing to see here. Move along.', 'There is nothing to see here. Move along.', '15904', '1', '1', '0', '0', '0', '0', '0', '0', 'This does not concern you. Be on your way.', 'This does not concern you. Be on your way.', '15905', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7828', '1872094');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7828 WHERE entry = 18790;


-- Importing creature gossip with entry 18792 (Harassed Citizen) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872095', 'Imported from TC', 'Please, don\'t say anything to me. I don\'t want to get into any more trouble with them.', 'Please, don\'t say anything to me. I don\'t want to get into any more trouble with them.', '15937', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7834', '1872095');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7834 WHERE entry = 18792;


-- Importing creature gossip with entry 18816 (Chief Researcher Amereldine) with import type GOSSIP
-- Text 9595 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7838', '9595');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7838 WHERE entry = 18816;


-- Importing creature gossip with entry 18817 (Chief Researcher Kartos) with import type GOSSIP
-- Text 9596 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7839', '9596');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7839 WHERE entry = 18817;


-- Importing creature gossip with entry 18439 (Garokk) with import type GOSSIP
-- Text 9637 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26343', '9637');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26343 WHERE entry = 18439;


-- Importing creature gossip with entry 19169 (Blood Elf Commoner) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872096', 'Imported from TC', 'If you find the lunar festival revelers in Silvermoon, they can transport you to Moonglade to partake in the celebration.', 'If you find the lunar festival revelers in Silvermoon, they can transport you to Moonglade to partake in the celebration.', '16189', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7921', '1872096');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7921 WHERE entry = 19169;


-- Importing creature gossip with entry 19175 (Orc Commoner) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('9704', 'Imported from TC', 'The largest celebration of the Lunar Festival is taking place in Moonglade. If that\'s too far to travel to, there are druids offering transport from the Valley of Wisdom.', 'The largest celebration of the Lunar Festival is taking place in Moonglade. If that\'s too far to travel to, there are druids offering transport from the Valley of Wisdom.', '16193', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7925', '9704');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7925 WHERE entry = 19175;


-- Importing creature gossip with entry 19177 (Troll Commoner) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872097', 'Imported from TC', 'The most amazing of the Lunar Festival parties is in Moonglade. Don\'t walk there, though, the druids in the Valley of Wisdom can transport you.', 'The most amazing of the Lunar Festival parties is in Moonglade. Don\'t walk there, though, the druids in the Valley of Wisdom can transport you.', '16217', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26344', '1872097');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26344 WHERE entry = 19177;


-- Importing creature gossip with entry 19158 (Garadar Guard Captain) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872098', 'Imported from TC', 'Draenei have come back! This group jumped two of my best wolf riders as they were out patrolling the western roads. Needless to say, we were victorious in that skirmish.$B$BNow I\'m not sure what\'s going on but from what my scouts tell me, there are blood elves and draenei all over Halaa. I\'m no blood elf lover but if it\'s one thing I hate it\'s a draenei. If you see draenei or any of their allies at Halaa, be sure to put a boot in their backsides for me.$B$BHalaa\'s to the west... In case you want to go now.', 'Draenei have come back! This group jumped two of my best wolf riders as they were out patrolling the western roads. Needless to say, we were victorious in that skirmish.$B$BNow I\'m not sure what\'s going on but from what my scouts tell me, there are blood elves and draenei all over Halaa. I\'m no blood elf lover but if it\'s one thing I hate it\'s a draenei. If you see draenei or any of their allies at Halaa, be sure to put a boot in their backsides for me.$B$BHalaa\'s to the west... In case you want to go now.', '16248', '0', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7944', '1872098');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7944 WHERE entry = 19158;


-- Importing creature gossip with entry 19273 (Forward Commander To'arch) with import type GOSSIP
-- Text 9771 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7955', '9771');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7955 WHERE entry = 19273;


-- Importing creature gossip with entry 19332 (Stone Guard Ambelan) with import type GOSSIP
-- Text 9777 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7958', '9777');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7958 WHERE entry = 19332;


-- Importing creature gossip with entry 19344 (Legassi) with import type GOSSIP
-- Text 9781 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7960', '9781');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7960 WHERE entry = 19344;


-- Importing creature gossip with entry 19367 ("Screaming" Screed Luckheed) with import type GOSSIP
-- Text 9789 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26345', '9789');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26345 WHERE entry = 19367;


-- Importing creature gossip with entry 19375 (Eli Thunderstrike) with import type GOSSIP
-- Text 9805 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7973', '9805');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7973 WHERE entry = 19375;


-- Importing creature gossip with entry 19496 (Spy Grik'tha) with import type GOSSIP
-- Text 9827 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7980', '9827');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7980 WHERE entry = 19496;


-- Importing creature gossip with entry 19542 (Field Commander Mahfuun) with import type GOSSIP
-- Text 9833 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7983', '9833');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7983 WHERE entry = 19542;


-- Importing creature gossip with entry 19569 (Netherologist Coppernickels) with import type GOSSIP
-- Text 9841 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('7991', '9841');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 7991 WHERE entry = 19569;


-- Importing creature gossip with entry 16829 (Magus Zabraxis) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872099', 'Imported from TC', '', 'Are you the correspondent from the Gadgetzan Times? If you just follow me, I\'ll show you my experimental apparatus...$B$B<You tell the magus that you\'re not the reporter she\'s expecting.>$B$BOh, you\'re not? Well then, if you need reagents, find what you need and don\'t tarry. I\'m expecting someone.', '17083', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8002', '1872099');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8002 WHERE entry = 16829;


-- Importing creature gossip with entry 18063 (Garrosh) with import type GOSSIP
-- Text 9886 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8019', '9886');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8019 WHERE entry = 18063;


-- Importing creature gossip with entry 19644 (Image of Archmage Vargoth) with import type GOSSIP
-- Text 9901 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8024', '9901');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8024 WHERE entry = 19644;


-- Importing creature gossip with entry 19840 (Caledis Brightdawn) with import type GOSSIP
-- Text 9925 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8038', '9925');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8038 WHERE entry = 19840;


-- Importing creature gossip with entry 19879 (Horvon the Armorer) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872100', 'Imported from TC', 'It\'s my passion, you know. In life and in death, armorcrafting is what I love. Please, treat me as if I were a living draenei, stranger. Let me know what it is to feel again... Allow me the honor of repairing your items.', '', '17480', '0', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8047', '1872100');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8047 WHERE entry = 19879;


-- Importing creature gossip with entry 19908 (Su'ura Swiftarrow) with import type GOSSIP
-- Text 7599 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8077', '7599');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872101', 'Imported from TC', '$C, we are not ashamed to say that we are in need of all of the assistance that we can get in our fight at Warsong Gulch.  But there is greater honor to be had now than at any time in the past if you so choose to lend us your skills.  As well, the Silverwing Sentinels are sure to look more favorably upon your efforts on our behalf.  Will you join the battle?', '$C, we are not ashamed to say that we are in need of all of the assistance that we can get in our fight at Warsong Gulch.  But there is greater honor to be had now than at any time in the past if you so choose to lend us your skills.  As well, the Silverwing Sentinels are sure to look more favorably upon your efforts on our behalf.  Will you join the battle?', '10581', '7', '1', '0', '1', '0', '1', '0', '6', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8077', '1872101');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8077 WHERE entry = 19908;


-- Importing creature gossip with entry 19907 (Grumbol Grimhammer) with import type GOSSIP
-- Text 7616 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8078', '7616');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('9984', 'Imported from TC', 'What are ye doin\" here?! Get yer chatty self ta Alterac Vallery, where ye\"re needed!', '', '17748', '7', '1', '1', '5', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8078', '9984');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8078 WHERE entry = 19907;


-- Importing creature gossip with entry 17310 (Gnarl) with import type GOSSIP
-- Text 9986 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8080', '9986');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8080 WHERE entry = 17310;


-- Importing creature gossip with entry 20278 (Vixton Pinchwhistle) with import type GOSSIP
-- Text 10062 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8124', '10062');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8124 WHERE entry = 20278;


-- Importing creature gossip with entry 20518 (Image of Wind Trader Marid) with import type GOSSIP
-- Text 9971 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26346', '9971');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26346 WHERE entry = 20518;


-- Importing creature gossip with entry 20612 (Sorim Lightsong) with import type GOSSIP
-- Text 10079 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8140', '10079');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8140 WHERE entry = 20612;


-- Importing creature gossip with entry 16426 (Bennett) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10120', 'Imported from TC', 'I don\'t have time for idle chitchat. I have men and women to train, and a tower to watch.', '', '18423', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8173', '10120');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8173 WHERE entry = 16426;


-- Importing creature gossip with entry 16811 (Sebastian) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10122', 'Imported from TC', 'Greetings! Behold one of the finest musical instruments in all of Lordaeron. It\'s a true honor to perform for the Master and all his guests. Take a seat in the audience, I hear a show is starting soon.', '', '18424', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8174', '10122');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8174 WHERE entry = 16811;


-- Importing creature gossip with entry 16806 (Ebonlocke) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10123', 'Imported from TC', 'This party is pointless. Those foolish nobles will never get to see Medivh. Can\'t they see this? I should have just stayed in Darkshire with my family instead of wasting my time here.', '', '18426', '0', '1', '1', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8175', '10123');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8175 WHERE entry = 16806;


-- Importing creature gossip with entry 16814 (Gradav) with import type GOSSIP
-- Text 10124 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8176', '10124');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8176 WHERE entry = 16814;


-- Importing creature gossip with entry 16815 (Kamsis) with import type GOSSIP
-- Text 10125 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8177', '10125');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8177 WHERE entry = 16815;


-- Importing creature gossip with entry 16813 (Wravien) with import type GOSSIP
-- Text 10126 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8178', '10126');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8178 WHERE entry = 16813;


-- Importing creature gossip with entry 20876 (Human Refugee) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10201', 'Imported from TC', '...they told us not to go... but we went anyway... they told us not to go... but we went anyway...', '...they told us not to go... but we went anyway... they told us not to go... but we went anyway...', '18578', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8206', '10201');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8206 WHERE entry = 20876;


-- Importing creature gossip with entry 20920 (Magister Theledorn) with import type GOSSIP
-- Text 10211 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26347', '10211');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26347 WHERE entry = 20920;


-- Importing creature gossip with entry 21007 (Sergeant Chawni) with import type GOSSIP
-- Text 10236 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26348', '10236');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26348 WHERE entry = 21007;


-- Importing creature gossip with entry 16695 (Gurak) with import type GOSSIP
-- Text 7616 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8250', '7616');
-- Text 10269 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8250', '10269');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8250 WHERE entry = 16695;


-- Importing creature gossip with entry 16696 (Krukk) with import type GOSSIP
-- Text 7599 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8256', '7599');
-- Text 10277 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8256', '10277');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8256 WHERE entry = 16696;


-- Importing creature gossip with entry 20120 (Tolo) with import type GOSSIP
-- Text 7642 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8257', '7642');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10278', 'Imported from TC', 'Our Alliance allies are assisting the Leage of Arathor in their defense of Arathi Basin.  They could use your aid against the undead Defilers, $c.', '', '18827', '7', '1', '1', '66', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '255', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8257', '10278');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8257 WHERE entry = 20120;


-- Importing creature gossip with entry 20276 (Wolf-Sister Maka) with import type GOSSIP
-- Text 7683 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8260', '7683');
-- Text 10282 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8260', '10282');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8260 WHERE entry = 20276;


-- Importing creature gossip with entry 20269 (Montok Redhands) with import type GOSSIP
-- Text 7599 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8261', '7599');
-- Text 10283 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8261', '10283');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8261 WHERE entry = 20269;


-- Importing creature gossip with entry 20274 (Keldor the Lost) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872102', 'Imported from TC', 'Thank you for the offer, $c.  Unfortunately you are not yet experienced enough to weather the hazards of Arathi Basin.', 'Thank you for the offer, $c.  Unfortunately you are not yet experienced enough to weather the hazards of Arathi Basin.', '10574', '7', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8262', '1872102');
-- Text 10284 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8262', '10284');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8262 WHERE entry = 20274;


-- Importing creature gossip with entry 20271 (Haelga Slatefist) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872103', 'Imported from TC', 'I\'m afraid ye would not survive long in Alterac Valley, $c.  Come back and see us when you have a few more seasons under your belt.', 'I\'m afraid ye would not survive long in Alterac Valley, $c.  Come back and see us when you have a few more seasons under your belt.', '10568', '7', '1', '0', '1', '0', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8263', '1872103');
-- Text 10285 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8263', '10285');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8263 WHERE entry = 20271;


-- Importing creature gossip with entry 20272 (Lylandor) with import type GOSSIP
-- Text 7599 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8264', '7599');
-- Text 10286 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8264', '10286');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8264 WHERE entry = 20272;


-- Importing creature gossip with entry 20339 (Oric Coe) with import type GOSSIP
-- Text 10288 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8266', '10288');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8266 WHERE entry = 20339;


-- Importing creature gossip with entry 21279 (Apothecary Albreck) with import type GOSSIP
-- Text 10301 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8272', '10301');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8272 WHERE entry = 21279;


-- Importing creature gossip with entry 16703 (Amin) with import type GOSSIP
-- Text 10454 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8379', '10454');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8379 WHERE entry = 16703;


-- Importing creature gossip with entry 21950 (Garm Wolfbrother) with import type GOSSIP
-- Text 10508 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8406', '10508');
UPDATE creature_template SET gossip_menu_id = 8406 WHERE entry = 21950;


-- Importing creature gossip with entry 21383 (Wyrmcult Acolyte) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872104', 'Imported from TC', 'Hello, overseer.  Ready for the attack?', 'Hello, overseer.  Ready for the attack?', '19713', '0', '1', '0', '6', '0', '0', '0', '0', 'Something you need, sir?', 'Something you need, sir?', '19714', '0', '1', '0', '6', '0', '0', '0', '0', 'You don\'t look well, overseer.', 'You don\'t look well, overseer.', '19715', '0', '1', '0', '1', '0', '0', '0', '0', 'I heard that Kolphis Darkscale has been looking for you.', 'I heard that Kolphis Darkscale has been looking for you.', '19716', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8446', '1872104');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8446 WHERE entry = 21383;


-- Importing creature gossip with entry 21637 (Wyrmcult Scout) with import type GOSSIP
-- Menu 8446 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8446 WHERE entry = 21637;


-- Importing creature gossip with entry 22024 (Parshah) with import type GOSSIP
-- Text 10556 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8449', '10556');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8449 WHERE entry = 22024;


-- Importing creature gossip with entry 22107 (Captain Darkhowl) with import type GOSSIP
-- Text 10610 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8495', '10610');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8495 WHERE entry = 22107;


-- Importing creature gossip with entry 22211 (Battlemage Vyara) with import type GOSSIP
-- Text 10625 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8500', '10625');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8500 WHERE entry = 22211;


-- Importing creature gossip with entry 22231 (Zezzak) with import type GOSSIP
-- Text 10636 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8505', '10636');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8505 WHERE entry = 22231;


-- Importing creature gossip with entry 22004 (Leoroxx) with import type GOSSIP
-- Text 10645 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8511', '10645');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8511 WHERE entry = 22004;


-- Importing creature gossip with entry 22423 (Evergrove Druid) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872105', 'Imported from TC', 'Death\'s Door is a portal that opens upon the world that the Burning Legion uses to breed their fel hounds and other canine-like terrors.  Any time that you see one of those creatures, that is where it came from, and it passes through here, first.$B$BLately, we became aware that the Burning Legion have ramped up their summoning of these creatures through the portal.$B$BYou must shut down Death\'s Door before it becomes an inexorable tide, and all is lost!', 'Death\'s Door is a portal that opens upon the world that the Burning Legion uses to breed their fel hounds and other canine-like terrors.  Any time that you see one of those creatures, that is where it came from, and it passes through here, first.$B$BLately, we became aware that the Burning Legion have ramped up their summoning of these creatures through the portal.$B$BYou must shut down Death\'s Door before it becomes an inexorable tide, and all is lost!', '20251', '0', '1', '0', '396', '500', '1', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8536', '1872105');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8536 WHERE entry = 22423;


-- Importing creature gossip with entry 22817 (Blood Elf Orphan) with import type GOSSIP
-- Text 10732 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8563', '10732');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8563 WHERE entry = 22817;


-- Importing creature gossip with entry 22818 (Draenei Orphan) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10734', 'Imported from TC', '', 'Why isn\'t the Outland called Draenor anymore?', '20462', '0', '1', '0', '1', '0', '0', '0', '0', '', 'Is the Twisting Nether shaped like a pretzel?  And just what is a Nether?  The orphan matron said I shouldn\'t ask people about their nethers.', '20470', '0', '1', '0', '1', '0', '0', '0', '0', '', 'Are we there yet?', '64242', '0', '1', '0', '1', '0', '0', '0', '0', '', 'If I hold my breath, what color would I turn?', '20684', '0', '1', '0', '1', '0', '0', '0', '0', '', 'If a horse gets horseshoes, will I get draenei shoes?', '20685', '0', '1', '0', '1', '0', '0', '0', '0', '', 'If the Burning Legion is such a problem, why not just throw water on them?  Then they\'ll just be a Legion.', '20686', '0', '1', '0', '1', '0', '0', '0', '0', '', 'How do you drink and eat so much, and not get fat?', '20687', '0', '1', '0', '1', '0', '0', '0', '0', '', 'Does a naaru turn off the Light when it wants to sleep?', '20688', '0', '1', '0', '1', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26349', '10734');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26349 WHERE entry = 22818;


-- Importing creature gossip with entry 22919 (Image of Commander Ameer) with import type GOSSIP
-- Text 10791 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8614', '10791');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8614 WHERE entry = 22919;


-- Importing creature gossip with entry 23101 (Challe) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10837', 'Imported from TC', '', 'Shh! Keep your voice down. The babes are sleeping.', '20923', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8639', '10837');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8639 WHERE entry = 23101;


-- Importing creature gossip with entry 23052 (Bloodmaul Supplicant) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10838', 'Imported from TC', 'Yes, $g king : queen;?', '', '20950', '0', '1', '0', '1', '0', '0', '0', '0', 'You kill gronn!  Now you fight things from sky?', '', '20951', '0', '1', '0', '15', '0', '0', '0', '0', 'Me life for $N!', '', '20952', '0', '1', '0', '15', '0', '0', '0', '0', 'Dis drink good!', '', '20953', '0', '1', '0', '1', '0', '0', '0', '0', 'If $G King : Queen; $N dance, me dance!', '', '20954', '0', '1', '0', '1', '0', '0', '0', '0', '$G King : Queen; not angry with me?', '', '20955', '0', '1', '0', '1', '0', '0', '0', '0', 'Why you look at me like dat?  You not gonna kiss me, right!?', '', '20956', '0', '1', '0', '1', '0', '0', '0', '0', 'Now all Sons of Gruul dead!  Now new $g king : queen; lead all ogres to Ogri\'la!', '', '20957', '0', '1', '0', '15', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8640', '10838');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8640 WHERE entry = 23052;


-- Importing creature gossip with entry 23053 (Bladespire Supplicant) with import type GOSSIP
-- Menu 8640 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8640 WHERE entry = 23053;


-- Importing creature gossip with entry 23140 (Taskmaster Varkule Dragonbreath) with import type GOSSIP
-- Text 10864 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8652', '10864');
UPDATE creature_template SET gossip_menu_id = 8652 WHERE entry = 23140;


-- Importing creature gossip with entry 23141 (Yarzill the Merc) with import type GOSSIP
-- Text 10892 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26350', '10892');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26350 WHERE entry = 23141;


-- Importing creature gossip with entry 23268 (Seer Jovar) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10893', 'Imported from TC', 'My sight is filled with visions of events taking place throughout the world and ones that may yet happen. Many of them are grim, but we needn\'t believe they are incapable of changing.', '', '21292', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8663', '10893');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8663 WHERE entry = 23268;


-- Importing creature gossip with entry 23291 (Chief Overseer Mudlump) with import type GOSSIP
-- Text 10896 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8665', '10896');
UPDATE creature_template SET gossip_menu_id = 8665 WHERE entry = 23291;


-- Importing creature gossip with entry 22433 (Ja'y Nosliw) with import type GOSSIP
-- Text 10904 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8668', '10904');
UPDATE creature_template SET gossip_menu_id = 8668 WHERE entry = 22433;


-- Importing creature gossip with entry 23316 (Torkus) with import type GOSSIP
-- Text 10916 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '10916');
-- Text 10917 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8678', '10917', '0', '0', '8', '0', '11030', '0', '0', '0', '0', '0', '', '(autoimported) Torkus - Show different gossip if player has rewarded quest 11030');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '10917');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('10918', 'Imported from TC', 'The boy\'s too stupid still to say it -- not enough crystal exposure yet -- but, he\'s thankful for what you did in getting him those flasks.$B$BNow, only nine more sons to go.  Gah!$B$B<Both of Torkus\'s heads sigh.>$B$BWant to take one of them off of our hands?  We\'ll sell him to you cheap.', '', '21395', '0', '1', '0', '1', '0', '5', '0', '6', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8678', '10918', '0', '0', '8', '0', '11061', '0', '0', '0', '0', '0', '', '(autoimported) Torkus - Show different gossip if player has rewarded quest 11061');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '10918');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11013', 'Imported from TC', 'It\'s an honor to have a $r such as yourself aiding us ogres.  You\'re keeping our hope of a better future alive.$B$BNow if we could just get rid of these headaches.  Are you sure you don\'t want one of our sons?', '', '21595', '0', '1', '0', '1', '0', '6', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8678', '11013', '0', '0', '5', '0', '1038', '32', '0', '0', '0', '0', '', '(autoimported) Torkus - Show different gossip if player is honored with Ogri\'la');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '11013');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11015', 'Imported from TC', 'It\'s amazing how much you\'ve helped us out.  We dare say that if it weren\'t for you, we\'d already have been overrun by the demons, or fried to a crisp by the Black Dragonflight!$B$BOn a side note, we did mention that we\'d sell you one of our sons, cheap, right?  No, not interested?', '', '21597', '0', '1', '0', '1', '0', '6', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8678', '11015', '0', '0', '5', '0', '1038', '64', '0', '0', '0', '0', '', '(autoimported) Torkus - Show different gossip if player is revered with Ogri\'la');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '11015');
-- Text 11018 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8678', '11018', '0', '0', '5', '0', '1038', '128', '0', '0', '0', '0', '', '(autoimported) Torkus - Show different gossip if player is exalted with Ogri\'la');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8678', '11018');
UPDATE creature_template SET gossip_menu_id = 8678 WHERE entry = 23316;


-- Importing creature gossip with entry 23376 (Dragonmaw Foreman) with import type GOSSIP
-- Text 10937 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8693', '10937');
UPDATE creature_template SET gossip_menu_id = 8693 WHERE entry = 23376;


-- Importing creature gossip with entry 23149 (Mistress of the Mines) with import type GOSSIP
-- Text 10939 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8694', '10939');
UPDATE creature_template SET gossip_menu_id = 8694 WHERE entry = 23149;


-- Importing creature gossip with entry 23164 (Toranaku) with import type GOSSIP
-- Text 10947 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8702', '10947');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8702 WHERE entry = 23164;


-- Importing creature gossip with entry 16583 (Rohok) with import type GOSSIP
-- Text 9546 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26351', '9546');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26351 WHERE entry = 16583;


-- Importing creature gossip with entry 23723 (Sergeant Lukas) with import type GOSSIP
-- Text 11243 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26352', '11243');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26352 WHERE entry = 23723;


-- Importing creature gossip with entry 23568 (Captain Darill) with import type GOSSIP
-- Text 11245 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8788', '11245');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8788 WHERE entry = 23568;


-- Importing creature gossip with entry 5086 (Captain Wymor) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11264', 'Imported from TC', 'Welcome to Sentry Point, $c.', '', '22257', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26353', '11264');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26353 WHERE entry = 5086;


-- Importing creature gossip with entry 16639 (Galana) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11459', 'Imported from TC', '', 'Is there a pattern or two I can show you today?', '22513', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8829', '11459');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8829 WHERE entry = 16639;


-- Importing creature gossip with entry 23835 (Sergeant Amelyn) with import type GOSSIP
-- Text 11406 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8831', '11406');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8831 WHERE entry = 23835;


-- Importing creature gossip with entry 23951 (Lieutenant Aden) with import type GOSSIP
-- Text 11424 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8834', '11424');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8834 WHERE entry = 23951;


-- Importing creature gossip with entry 23949 (Lieutenant Nath) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11426', 'Imported from TC', 'Lieutenant Aden always volunteers for gate duty. That\'s fine by me.$B$BI\'ve got a post that allows me a good view of my guards AND the daughters of the tradesmen at the same time.', '', '22479', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26354', '11426');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26354 WHERE entry = 23949;


-- Importing creature gossip with entry 23950 (Lieutenant Khand) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('11428', 'Imported from TC', 'Are you here to relieve me? I\'m certain it\'s almost time. Well, if you want to relieve me anyway, let me know. I could use a drink or three.', '', '22480', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26355', '11428');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26355 WHERE entry = 23950;


-- Importing creature gossip with entry 4921 (Guard Byron) with import type GOSSIP
-- Text 11457 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8840', '11457');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8840 WHERE entry = 4921;


-- Importing creature gossip with entry 24370 (Nether-Stalker Mah'duun) with import type GOSSIP
-- Text 12046 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8948', '12046');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8948 WHERE entry = 24370;


-- Importing creature gossip with entry 19171 (Draenei Commoner) with import type GOSSIP
-- Text 11956 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11956', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11956', '0', '0', '31', '1', '3', '19169', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11956');
-- Text 11957 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11957', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11957', '0', '0', '31', '1', '3', '19171', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11957');
-- Text 11958 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11958', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11958', '0', '0', '31', '1', '3', '19148', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11958');
-- Text 11959 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11959', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11959', '0', '0', '31', '1', '3', '19172', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11959');
-- Text 11961 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11961', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11961', '0', '0', '31', '1', '3', '18927', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11961');
-- Text 11962 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11962', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11962', '0', '0', '31', '1', '3', '19173', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11962');
-- Text 11963 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11963', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11963', '0', '0', '31', '1', '3', '19175', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11963');
-- Text 11964 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11964', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11964', '0', '0', '31', '1', '3', '19176', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11964');
-- Text 11965 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11965', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11965', '0', '0', '31', '1', '3', '19177', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11965');
-- Text 11966 already present in Sun DB
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11966', '0', '0', '12', '0', '12', '0', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO conditions (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES ('14', '8988', '11966', '0', '0', '31', '1', '3', '19178', '0', '0', '0', '0', '', '(autoimported) ');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '11966');
-- Text 12134 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('8988', '12134');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19171;


-- Importing creature gossip with entry 19176 (Tauren Commoner) with import type GOSSIP
-- Menu 8988 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19176;


-- Importing creature gossip with entry 19148 (Dwarf Commoner) with import type GOSSIP
-- Menu 8988 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19148;


-- Importing creature gossip with entry 19172 (Gnome Commoner) with import type GOSSIP
-- Menu 8988 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19172;


-- Importing creature gossip with entry 19173 (Night Elf Commoner) with import type GOSSIP
-- Menu 8988 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19173;


-- Importing creature gossip with entry 19178 (Forsaken Commoner) with import type GOSSIP
-- Menu 8988 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 8988 WHERE entry = 19178;


-- Importing creature gossip with entry 24727 (Caylee Dak) with import type GOSSIP
-- Text 12162 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9004', '12162');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9004 WHERE entry = 24727;


-- Importing creature gossip with entry 24866 (Lakoor) with import type GOSSIP
-- Text 12208 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9030', '12208');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9030 WHERE entry = 24866;


-- Importing creature gossip with entry 24840 (Sailor Vines) with import type GOSSIP
-- Text 12210 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9032', '12210');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9032 WHERE entry = 24840;


-- Importing creature gossip with entry 24881 (Karrtog) with import type GOSSIP
-- Text 12213 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9035', '12213');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9035 WHERE entry = 24881;


-- Importing creature gossip with entry 24835 (First Mate Kowalski) with import type GOSSIP
-- Text 12214 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26356', '12214');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26356 WHERE entry = 24835;


-- Importing creature gossip with entry 24842 (Marine Anderson) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872106', 'Imported from TC', 'The Lady Mehley is in Theramore enough that we\'ve been assigned to guard her passengers.', 'The Lady Mehley is in Theramore enough that we\'ve been assigned to guard her passengers.', '23959', '7', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26357', '1872106');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26357 WHERE entry = 24842;


-- Importing creature gossip with entry 24841 (Marine Halters) with import type GOSSIP
-- Menu 9037 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9037 WHERE entry = 24841;


-- Importing creature gossip with entry 24937 (Magistrix Seyla) with import type GOSSIP
-- Text 12239 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26358', '12239');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26358 WHERE entry = 24937;


-- Importing creature gossip with entry 25021 (Mariner Moonblade) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12243', 'Imported from TC', 'We must make sure not to lose any more souls to the seas.', 'We must make sure not to lose any more souls to the seas.', '24026', '7', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26359', '12243');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26359 WHERE entry = 25021;


-- Importing creature gossip with entry 25024 (Mariner Softsong) with import type GOSSIP
-- Menu 9054 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9054 WHERE entry = 25024;


-- Importing creature gossip with entry 25022 (Mariner Stillwake) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12244', 'Imported from TC', 'Forgive me $c, but I must concentrate on my job.', 'Forgive me $c, but I must concentrate on my job.', '24030', '7', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26360', '12244');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26360 WHERE entry = 25022;


-- Importing creature gossip with entry 25023 (Mariner Everwatch) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872107', 'Imported from TC', 'Many have been lost to the seas.  I, and the wisps, are charged with looking for those souls.  ', 'Many have been lost to the seas.  I, and the wisps, are charged with looking for those souls.', '24031', '7', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9057', '1872107');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9057 WHERE entry = 25023;


-- Importing creature gossip with entry 24998 (Mariner Farsight) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12246', 'Imported from TC', 'The wisps and I watch the seas for any lost souls.', 'The wisps and I watch the seas for any lost souls.', '24033', '7', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9058', '12246');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9058 WHERE entry = 24998;


-- Importing creature gossip with entry 24996 (Mariner Bladewhisper) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12247', 'Imported from TC', 'The seas are a dangerous place, $c.  We must be alert.', 'The seas are a dangerous place, $c.  We must be alert.', '24034', '7', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9059', '12247');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9059 WHERE entry = 24996;


-- Importing creature gossip with entry 24997 (Mariner Swiftstar) with import type GOSSIP
-- Menu 9059 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9059 WHERE entry = 24997;


-- Importing creature gossip with entry 25007 (Mariner Evenmist) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12248', 'Imported from TC', 'I\'m afraid I cannot speak with you at the moment.', 'I\'m afraid I cannot speak with you at the moment.', '24035', '7', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9060', '12248');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9060 WHERE entry = 25007;


-- Importing creature gossip with entry 25025 (Captain Idrilae) with import type GOSSIP
-- Text 12275 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26361', '12275');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26361 WHERE entry = 25025;


-- Importing creature gossip with entry 24456 (Captain Taldar Windsinger) with import type GOSSIP
-- Text 12276 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9078', '12276');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9078 WHERE entry = 24456;


-- Importing creature gossip with entry 25053 (Mariner Farseeker) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12280', 'Imported from TC', '', 'I\'m sorry, I\'ve little time for speaking to anyone other than the wisps and the captain to report my findings.  Forgive me.', '24135', '7', '1', '2', '2', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9079', '12280');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9079 WHERE entry = 25053;


-- Importing creature gossip with entry 25093 (First Mate Masker) with import type GOSSIP
-- Text 12281 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9083', '12281');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9083 WHERE entry = 25093;


-- Importing creature gossip with entry 25099 (Jonathan Garrett) with import type GOSSIP
-- Text 12282 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9084', '12282');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9084 WHERE entry = 25099;


-- Importing creature gossip with entry 25088 (Captain Valindria) with import type GOSSIP
-- Text 12287 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9088', '12287');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9088 WHERE entry = 25088;


-- Importing creature gossip with entry 25169 (Archmage Ne'thul) with import type GOSSIP
-- Text 12309 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9105', '12309');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9105 WHERE entry = 25169;


-- Importing creature gossip with entry 25081 (Grunt Ounda) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872108', 'Imported from TC', 'We protect this airship on its way from Orgrimmar to Grom\'gol base camp in Stranglethorn Vale.', 'We protect this airship on its way from Orgrimmar to Grom\'gol base camp in Stranglethorn Vale.', '24260', '1', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9108', '1872108');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9108 WHERE entry = 25081;


-- Importing creature gossip with entry 25105 (Sky-Captain Cableclamp) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872109', 'Imported from TC', '', 'Welcome aboard $c.  Take a load off and enjoy the trip.  Aboard \"The Purple Princess\" we want you to relax.  We have vending machines down below if there\'s anything you might need while aboard.', '24282', '1', '1', '0', '1', '4', '2', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9116', '1872109');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9116 WHERE entry = 25105;


-- Importing creature gossip with entry 25100 (Chief Officer Hammerflange) with import type GOSSIP
-- Text 12328 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26362', '12328');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26362 WHERE entry = 25100;


-- Importing creature gossip with entry 25104 (Navigator Hatch) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12329', 'Imported from TC', 'The captain\'s great!  We get time to relax when we dock.  Those are the times I look forward to the most.', '', '24284', '1', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9118', '12329');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9118 WHERE entry = 25104;


-- Importing creature gossip with entry 25107 (Watcher Umjin) with import type GOSSIP
-- Text 12331 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9120', '12331');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9120 WHERE entry = 25107;


-- Importing creature gossip with entry 25102 (Crewman Spinshaft) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872110', 'Imported from TC', 'I\'m busy right now.', 'I\'m busy right now.', '24287', '1', '1', '0', '274', '0', '0', '0', '0', 'You need something fixed? Use the Vend-O-Tron.', 'You need something fixed? Use the Vend-O-Tron.', '24288', '1', '1', '0', '396', '0', '0', '0', '0', 'I don\'t have time to talk right now.', 'I don\'t have time to talk right now.', '24289', '1', '1', '0', '274', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9121', '1872110');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9121 WHERE entry = 25102;


-- Importing creature gossip with entry 25103 (Crewman Boltshine) with import type GOSSIP
-- Menu 9121 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9121 WHERE entry = 25103;


-- Importing creature gossip with entry 25101 (Crewman Cutpipe) with import type GOSSIP
-- Menu 9121 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9121 WHERE entry = 25101;


-- Importing creature gossip with entry 25106 (Watcher Du'una) with import type GOSSIP
-- Text 12333 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9122', '12333');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9122 WHERE entry = 25106;


-- Importing creature gossip with entry 25202 (Arcanist Myndimendez) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872111', 'Imported from TC', 'Did you manage to see their show in Terokkar Forest? It was amazing!', 'Did you manage to see their show in Terokkar Forest? It was amazing!', '24298', '0', '1', '0', '0', '0', '0', '0', '0', 'The Tauren Chieftains must be the best-known minstrels in all of Azeroth!', 'The Tauren Chieftains must be the best-known minstrels in all of Azeroth!', '24299', '0', '1', '0', '0', '0', '0', '0', '0', 'I wonder how much longer they\'ll be in Silvermoon City. I hope they play here!', 'I wonder how much longer they\'ll be in Silvermoon City. I hope they play here!', '24300', '0', '1', '0', '0', '0', '0', '0', '0', 'I\'d give nearly anything for a chance to go up there and speak with them...', 'I\'d give nearly anything for a chance to go up there and speak with them...', '24301', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9125', '1872111');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9125 WHERE entry = 25202;


-- Importing creature gossip with entry 25207 (Roitau) with import type GOSSIP
-- Text 12341 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26363', '12341');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26363 WHERE entry = 25207;


-- Importing creature gossip with entry 25200 (Steven Thomas) with import type GOSSIP
-- Text 12342 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9129', '12342');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9129 WHERE entry = 25200;


-- Importing creature gossip with entry 20124 (Kradu Grimblade) with import type GOSSIP
-- Text 12347 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26364', '12347');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26364 WHERE entry = 20124;


-- Importing creature gossip with entry 20125 (Zula Slagfury) with import type GOSSIP
-- Text 12348 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9132', '12348');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9132 WHERE entry = 20125;


-- Importing creature gossip with entry 17718 (Magister Astalor Bloodsworn) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12370', 'Imported from TC', 'Kael\'thas and his felbloods be damned! All we\'ve worked for is gone, but they\'re fools to think we won\'t fight back. Already, Lady Liadrin seeks a new source of power for our order. We will not be destroyed so easily.', '', '24451', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9142', '12370');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9142 WHERE entry = 17718;


-- Importing creature gossip with entry 26221 (Earthen Ring Elder) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872112', 'Imported from TC', 'The Twilight Cult seeks to undo everything we fight to preserve. It is our sworn duty to prevent this calamity from coming to pass, $r.', 'The Twilight Cult seeks to undo everything we fight to preserve. It is our sworn duty to prevent this calamity from coming to pass, $r.', '24551', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9157', '1872112');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9157 WHERE entry = 26221;


-- Importing creature gossip with entry 25890 (Blasted Lands Flame Warden) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('12504', 'Imported from TC', 'Welcome, $c. Have you come to honor this flame?$b$bRemember: Our festival fires burn in every land with an Alliance settlement. Make time to honor each of them, if you can.$b$bThe Horde, too, burns fires of their own. You would do well to desecrate such unworthy efforts when you see them.', 'Welcome, $c. Have you come to honor this flame?$b$bRemember: Our festival fires burn in every land with an Alliance settlement. Make time to honor each of them, if you can.$b$bThe Horde, too, burns fires of their own. You would do well to desecrate such unworthy efforts when you see them.', '25101', '7', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9203', '12504');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25890;


-- Importing creature gossip with entry 25894 (Desolace Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25894;


-- Importing creature gossip with entry 25898 (Elwynn Forest Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25898;


-- Importing creature gossip with entry 25902 (Loch Modan Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25902;


-- Importing creature gossip with entry 25906 (Teldrassil Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25906;


-- Importing creature gossip with entry 25910 (Westfall Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25910;


-- Importing creature gossip with entry 25914 (Silithus Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25914;


-- Importing creature gossip with entry 25887 (Arathi Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25887;


-- Importing creature gossip with entry 25891 (Bloodmyst Isle Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25891;


-- Importing creature gossip with entry 25895 (Dun Morogh Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25895;


-- Importing creature gossip with entry 25899 (Feralas Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25899;


-- Importing creature gossip with entry 25903 (Nagrand Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25903;


-- Importing creature gossip with entry 25907 (Terokkar Forest Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25907;


-- Importing creature gossip with entry 25911 (Wetlands Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25911;


-- Importing creature gossip with entry 25915 (Stranglethorn Vale Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25915;


-- Importing creature gossip with entry 25888 (Azuremyst Isle Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25888;


-- Importing creature gossip with entry 25892 (Burning Steppes Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25892;


-- Importing creature gossip with entry 25896 (Duskwood Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25896;


-- Importing creature gossip with entry 25900 (Hellfire Peninsula Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25900;


-- Importing creature gossip with entry 25904 (Redridge Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25904;


-- Importing creature gossip with entry 25908 (The Hinterlands Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25908;


-- Importing creature gossip with entry 25912 (Zangarmarsh Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25912;


-- Importing creature gossip with entry 25916 (Tanaris Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25916;


-- Importing creature gossip with entry 25889 (Blade's Edge Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25889;


-- Importing creature gossip with entry 25893 (Darkshore Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25893;


-- Importing creature gossip with entry 25897 (Dustwallow Marsh Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25897;


-- Importing creature gossip with entry 25901 (Hillsbrad Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25901;


-- Importing creature gossip with entry 25905 (Shadowmoon Valley Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25905;


-- Importing creature gossip with entry 25909 (Western Plaguelands Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25909;


-- Importing creature gossip with entry 25913 (Netherstorm Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25913;


-- Importing creature gossip with entry 25917 (Winterspring Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25917;


-- Importing creature gossip with entry 25883 (Ashenvale Flame Warden) with import type GOSSIP
-- Menu 9203 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9203 WHERE entry = 25883;


-- Importing creature gossip with entry 26113 (Master Flame Eater) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872113', 'Imported from TC', 'Behold, the Flames of Summer!', 'Behold, the Flames of Summer!', '25105', '0', '1', '0', '5', '0', '0', '0', '0', 'Careful, don\'t get burned!', 'Careful, don\'t get burned!', '25106', '0', '1', '0', '25', '0', '0', '0', '0', 'Enjoying the Fire Festival?', 'Enjoying the Fire Festival?', '25107', '0', '1', '0', '1', '0', '0', '0', '0', 'Welcome! Have you yet honored this land\'s flame?', 'Welcome! Have you yet honored this land\'s flame?', '25108', '0', '1', '0', '3', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9204', '1872113');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9204 WHERE entry = 26113;


-- Importing creature gossip with entry 25962 (Fire Eater) with import type GOSSIP
-- Menu 9204 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9204 WHERE entry = 25962;


-- Importing creature gossip with entry 25975 (Master Fire Eater) with import type GOSSIP
-- Menu 9204 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9204 WHERE entry = 25975;


-- Importing creature gossip with entry 25994 (Flame Eater) with import type GOSSIP
-- Menu 9204 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9204 WHERE entry = 25994;


-- Importing creature gossip with entry 25918 (Netherstorm Flame Keeper) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872114', 'Imported from TC', 'Welcome, $c. Have you come to honor this flame?$b$bRemember: Our festival fires burn in every land with a Horde settlement. Make time to honor each of them, if you can.$b$bThe Alliance, too, burns fires of their own. You would do well to desecrate such unworthy efforts when you see them.', 'Welcome, $c. Have you come to honor this flame?$b$bRemember: Our festival fires burn in every land with a Horde settlement. Make time to honor each of them, if you can.$b$bThe Alliance, too, burns fires of their own. You would do well to desecrate such unworthy efforts when you see them.', '25402', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9278', '1872114');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25918;


-- Importing creature gossip with entry 25922 (Winterspring Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25922;


-- Importing creature gossip with entry 25927 (Burning Steppes Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25927;


-- Importing creature gossip with entry 25931 (Eversong Woods Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25931;


-- Importing creature gossip with entry 25935 (Hillsbrad Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25935;


-- Importing creature gossip with entry 25939 (Silverpine Forest Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25939;


-- Importing creature gossip with entry 25943 (The Barrens Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25943;


-- Importing creature gossip with entry 25947 (Zangarmarsh Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25947;


-- Importing creature gossip with entry 25884 (Ashenvale Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25884;


-- Importing creature gossip with entry 25919 (Silithus Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25919;


-- Importing creature gossip with entry 25923 (Arathi Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25923;


-- Importing creature gossip with entry 25928 (Desolace Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25928;


-- Importing creature gossip with entry 25932 (Feralas Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25932;


-- Importing creature gossip with entry 25936 (Mulgore Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25936;


-- Importing creature gossip with entry 25940 (Stonetalon Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25940;


-- Importing creature gossip with entry 25944 (The Hinterlands Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25944;


-- Importing creature gossip with entry 25920 (Stranglethorn Vale Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25920;


-- Importing creature gossip with entry 25925 (Badlands Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25925;


-- Importing creature gossip with entry 25929 (Durotar Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25929;


-- Importing creature gossip with entry 25933 (Ghostlands Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25933;


-- Importing creature gossip with entry 25937 (Nagrand Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25937;


-- Importing creature gossip with entry 25941 (Swamp of Sorrows Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25941;


-- Importing creature gossip with entry 25945 (Thousand Needles Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25945;


-- Importing creature gossip with entry 25921 (Tanaris Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25921;


-- Importing creature gossip with entry 25926 (Blade's Edge Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25926;


-- Importing creature gossip with entry 25930 (Dustwallow Marsh Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25930;


-- Importing creature gossip with entry 25934 (Hellfire Peninsula Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25934;


-- Importing creature gossip with entry 25938 (Shadowmoon Valley Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25938;


-- Importing creature gossip with entry 25942 (Terokkar Forest Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25942;


-- Importing creature gossip with entry 25946 (Tirisfal Glades Flame Keeper) with import type GOSSIP
-- Menu 9278 is already imported
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9278 WHERE entry = 25946;


-- Importing creature gossip with entry 16818 (Festival Talespinner) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872115', 'Imported from TC', 'Enjoying the festival, $c?$b$bWhile the Flame Keepers tend to the fires of the present, I am more of a historian, keeping close the festivals past. I\'m also, of course, documenting this year\'s festivities. Things are going well thus far, don\'t you think?$b$bYou know, $n, there is power inherent in all festival fires burning throughout the holiday. We\'re taking care of ours, but I\'m certain there are sacred flames burning deep within our enemy\'s cities...', 'Enjoying the festival, $c?$b$bWhile the Flame Keepers tend to the fires of the present, I am more of a historian, keeping close the festivals past. I\'m also, of course, documenting this year\'s festivities. Things are going well thus far, don\'t you think?$b$bYou know, $n, there is power inherent in all festival fires burning throughout the holiday. We\'re taking care of ours, but I\'m certain there are sacred flames burning deep within our enemy\'s cities...', '25566', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9298', '1872115');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9298 WHERE entry = 16818;


-- Importing creature gossip with entry 27215 (Boxey Boltspinner) with import type GOSSIP
-- Text 12744 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26365', '12744');
UPDATE creature_template SET gossip_menu_id = 26365 WHERE entry = 27215;


-- Importing creature gossip with entry 27398 (Gilbarta Grandhammer) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872116', 'Imported from TC', 'You know of the Spirits of Competition, yes?$B$BEvery two years they draw close to the mortal realm. During such times, members of both the Alliance and Horde wear Competitor\'s Tabards into the Battlegrounds to honor the spirits, seeking to defeat one another and earn their favor.$B$BI stockpile souvenirs from such turbulent times. If you\'ve learned the proper word, I might be convinced to part with one...', 'You know of the Spirits of Competition, yes?$B$BEvery two years they draw close to the mortal realm. During such times, members of both the Alliance and Horde wear Competitor\'s Tabards into the Battlegrounds to honor the spirits, seeking to defeat one another and earn their favor.$B$BI stockpile souvenirs from such turbulent times. If you\'ve learned the proper word, I might be convinced to part with one...', '26512', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26366', '1872116');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26366 WHERE entry = 27398;


-- Importing creature gossip with entry 27216 (Bizzle Quicklift) with import type GOSSIP
-- Text 12913 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26367', '12913');
UPDATE creature_template SET gossip_menu_id = 26367 WHERE entry = 27216;


-- Importing creature gossip with entry 28126 (Don Carlos) with import type GOSSIP
-- Text 13079 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26368', '13079');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26368 WHERE entry = 28126;


-- Importing creature gossip with entry 28209 (Mizli Crankwheel) with import type GOSSIP
-- Text 13093 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9665', '13093');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9665 WHERE entry = 28209;


-- Importing creature gossip with entry 28210 (Ognip Blastbolt) with import type GOSSIP
-- Text 13094 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('9666', '13094');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 9666 WHERE entry = 28210;


-- Importing creature gossip with entry 5875 (Gan'rul Bloodeye) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('13911', 'Imported from TC', 'What is it?', '', '72223', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26369', '13911');
UPDATE creature_template SET gossip_menu_id = 26369 WHERE entry = 5875;


-- Importing creature gossip with entry 20735 (Archmage Lan'dalock) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('13979', 'Imported from TC', 'Amongst the Kirin Tor, it is my duty to see to the strategic interests of the magocracy.$B$BThere is much yet to be done this week. Will you assist me, $N?', '', '32119', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10061', '13979');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10061 WHERE entry = 20735;


-- Importing creature gossip with entry 16128 (Rhonin) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14161', 'Imported from TC', 'Welcome to the home of the Kirin Tor.', '', '32523', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10140', '14161');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10140 WHERE entry = 16128;


-- Importing creature gossip with entry 3443 (Grub) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14313', 'Imported from TC', 'Dig rats make some surprisingly tasty stew.', '', '33345', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10311', '14313');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10311 WHERE entry = 3443;


-- Importing creature gossip with entry 8582 (Kadrak) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14344', 'Imported from TC', 'What is it, $c?', '', '69232', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10331', '14344');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10331 WHERE entry = 8582;


-- Importing creature gossip with entry 5193 (Rebecca Laughlin) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872117', 'Imported from TC', 'Welcome to the Stormwind Guild Registry offices. What can I help you with today?', 'Welcome to the Stormwind Guild Registry offices. What can I help you with today?', '3414', '7', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10392', '1872117');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10392 WHERE entry = 5193;


-- Importing creature gossip with entry 12616 (Vhulgra) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872118', 'Imported from TC', 'Where would you like to fly to?', 'Where would you like to fly to?', '10753', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10434', '1872118');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10434 WHERE entry = 12616;


-- Importing creature gossip with entry 18911 (Juno Dufrain) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872119', 'Imported from TC', 'I can teach you fishing skills.', 'I can teach you fishing skills.', '9557', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10437', '1872119');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10437 WHERE entry = 18911;


-- Importing creature gossip with entry 11701 (Mor'vek) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14474', 'Imported from TC', 'It takes a lot of dedication to train and ride a Venomhide.', '', '34297', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10438', '14474');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10438 WHERE entry = 11701;


-- Importing creature gossip with entry 6091 (Dellylah) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14562', 'Imported from TC', '', 'Another new face? It\'s so good to meet you. I\'m Dellylah.', '34761', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10526', '14562');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10526 WHERE entry = 6091;


-- Importing creature gossip with entry 3467 (Baron Longshore) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14623', 'Imported from TC', 'What\'re ye starin\' at, $r? Come ta torment the condemned?', '', '35048', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10567', '14623');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14624', 'Imported from TC', '<The Baron lifts his manacles.>$B$BCome on then, $c. Do ye trust me?', '', '35049', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10567', '14624');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14625', 'Imported from TC', 'Well? Don\'t ye have heads ta roll an\' treasure ta dig up?', '', '35053', '0', '1', '1', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10567', '14625');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10567 WHERE entry = 3467;


-- Importing creature gossip with entry 5395 (Felgur Twocuts) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14932', 'Imported from TC', 'Make it quick, $r.', '', '36298', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10771', '14932');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10771 WHERE entry = 5395;


-- Importing creature gossip with entry 2055 (Master Apothecary Faranell) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('14940', 'Imported from TC', 'It\'s true our new... guests are limiting our work somewhat, but it is only natural to be wary when a genocidal attempt is made. Perhaps in time they will be convinced that Putress was an unfortunate anomaly and allow us to continue our work in peace.', '', '36321', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10775', '14940');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10775 WHERE entry = 2055;


-- Importing creature gossip with entry 8586 (Haggrum Bloodfist) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15021', 'Imported from TC', 'I remember when my brother and I would camp alone in the Azshara wilderness. Look at it now. ', '', '36578', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10832', '15021');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10832 WHERE entry = 8586;


-- Importing creature gossip with entry 8416 (Felix Whindlebolt) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15139', 'Imported from TC', 'Oh dear.  Things aren\'t looking good for Felix.', '', '37060', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10898', '15139');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10898 WHERE entry = 8416;


-- Importing creature gossip with entry 6782 (Hands Springsprocket) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15145', 'Imported from TC', 'How do you do, $c?', '', '37081', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('10900', '15145');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 10900 WHERE entry = 6782;


-- Importing creature gossip with entry 3418 (Kirge Sternhorn) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15302', 'Imported from TC', 'A foul wind blows from the East, $c.', '', '38248', '0', '1', '1', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11004', '15302');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11004 WHERE entry = 3418;


-- Importing creature gossip with entry 2981 (Chief Hawkwind) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872120', 'Imported from TC', 'Many are the paths of the Earth Mother.  May your ancestors watch over you $c.', 'Many are the paths of the Earth Mother.  May your ancestors watch over you $c.', '8294', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11006', '1872120');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11006 WHERE entry = 2981;


-- Importing creature gossip with entry 1519 (Deathguard Simmer) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15512', 'Imported from TC', '<Deathguard Simmer looks at you expectantly.>', '', '39113', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11145', '15512');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11145 WHERE entry = 1519;


-- Importing creature gossip with entry 10666 (Gordo) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15531', 'Imported from TC', 'Simmer say Gordo got guts, but got no brains.  Why he mean?', '', '39172', '0', '1', '0', '0', '0', '0', '0', '0', 'Gordo like picking flowers.  Pick flowers kind of like killing.', '', '39173', '0', '1', '0', '0', '0', '0', '0', '0', 'It drafty out here.', '', '39174', '0', '1', '0', '0', '0', '0', '0', '0', 'Johaan say Gordo need a tailor to fix Gordo tummy.', '', '39175', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11159', '15531');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11159 WHERE entry = 10666;


-- Importing creature gossip with entry 10676 (Raider Jhash) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15572', 'Imported from TC', 'Need something?', '', '39288', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26370', '15572');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26370 WHERE entry = 10676;


-- Importing creature gossip with entry 3293 (Rezlak) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15664', 'Imported from TC', 'Try to ignore the firm, feathery behinds of the harpies... the creatures are really quite deadly!', '', '39590', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11241', '15664');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11241 WHERE entry = 3293;


-- Importing creature gossip with entry 3936 (Shandris Feathermoon) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15719', 'Imported from TC', '', 'The forces of the Sentinel Army hold back the Horde across all of Kalimdor. While the war continues in Ashenvale and Stonetalon, our forces are also needed here, to protect these remaining wilds and prevent our enemies from gaining further footholds.', '39743', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11284', '15719');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11284 WHERE entry = 3936;


-- Importing creature gossip with entry 7937 (High Tinker Mekkatorque) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15863', 'Imported from TC', 'Welcome to Tinker Town!', '', '40441', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11390', '15863');
UPDATE creature_template SET gossip_menu_id = 11390 WHERE entry = 7937;


-- Importing creature gossip with entry 11390 (Skullsplitter Speaker) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15868', 'Imported from TC', '', 'It\'s my job to ensure that this whole assault runs like clockwork.$B$BHigh Tinker Mekkatorque is out in the field now, but he\'s scheduled to come back to get some more troops any time now.  $B$BHe\'s never late, so just stick around and he\'ll be here.', '40462', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11393', '15868');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('15870', 'Imported from TC', '', 'Hurry! You should talk to Mekkatorque! If we don\'t start the assault soon, we\'re going to be late.$B$BAnd my boss HATES it when we\'re late!', '40466', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11393', '15870');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11393 WHERE entry = 11390;


-- Importing creature gossip with entry 25697 (Luma Skymother) with import type GOSSIP
-- Text 12390 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11417', '12390');
UPDATE creature_template SET gossip_menu_id = 11417 WHERE entry = 25697;


-- Importing creature gossip with entry 3411 (Denni'ka) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16157', 'Imported from TC', '', 'What you want?', '66076', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11573', '16157');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11573 WHERE entry = 3411;


-- Importing creature gossip with entry 9540 (Enohar Thunderbrew) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('1872121', 'Imported from TC', '', 'Things can get a little stuffy here in Nethergarde.  Sometimes a dwarf just needs a brew to take her mind off her duty for a bit.', '42235', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11618', '1872121');
UPDATE creature_template SET gossip_menu_id = 11618 WHERE entry = 9540;


-- Importing creature gossip with entry 2464 (Commander Aggro'gosh) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16334', 'Imported from TC', 'Welcome to Grom\'gol, $c.  Before we get off on the wrong foot, I\'ll have you know that I won\'t have slackers in my Base Camp.  I expect a healthy $r like you to pull your own weight around here.', '', '42910', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11673', '16334');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11673 WHERE entry = 2464;


-- Importing creature gossip with entry 2465 (Far Seer Mok'thardin) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16335', 'Imported from TC', 'Even in this remote corner of the world, know that Hellscream\'s eyes are upon you.', '', '42911', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11674', '16335');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11674 WHERE entry = 2465;


-- Importing creature gossip with entry 2497 (Nimboya) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16336', 'Imported from TC', 'The trolls of Stranglethorn don\'t take too kindly to us Darkspear.  I\'m not afraid of no Bloodscalp or Skullsplitter, but still... Grom\'gol\'s high walls make me feel safe.', '', '42912', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11675', '16336');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11675 WHERE entry = 2497;


-- Importing creature gossip with entry 2491 (Whiskey Slim) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16418', 'Imported from TC', 'Here\'s to you!  Here\'s to me!  May we never disagree.$b$bBut if we do... BLAST YOU!  Here\'s to me.', '', '43444', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11727', '16418');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11727 WHERE entry = 2491;


-- Importing creature gossip with entry 14395 (Griniblix the Spectator) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16432', 'Imported from TC', 'Woohoo! They are into it now!', '', '9504', '0', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11738', '16432');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11738 WHERE entry = 14395;


-- Importing creature gossip with entry 2663 (Narkk) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16440', 'Imported from TC', 'Care to purchase some piratey items?', '', '43575', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('26371', '16440');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 26371 WHERE entry = 2663;


-- Importing creature gossip with entry 2487 (Fleet Master Seahorn) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16441', 'Imported from TC', 'Welcome to Booty Bay, $r.', '', '43587', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11744', '16441');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11744 WHERE entry = 2487;


-- Importing creature gossip with entry 4265 (Nyoma) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16510', 'Imported from TC', '', 'Welcome to Dolanaar, $c. Let me know if you need any recipes or basic cooking ingredients.', '44301', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11778', '16510');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11778 WHERE entry = 4265;


-- Importing creature gossip with entry 2153 (Terl Arakor) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16528', 'Imported from TC', 'This is MY WAGON!$B$BMINE!', '', '44358', '0', '100', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11788', '16528');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11788 WHERE entry = 2153;


-- Importing creature gossip with entry 3139 (Gar'Thok) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16535', 'Imported from TC', 'Keep your eyes open, $c. Even Durotar is not safe from our enemies.', '', '44382', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11790', '16535');
UPDATE creature_template SET gossip_menu_id = 11790 WHERE entry = 3139;


-- Importing creature gossip with entry 10665 (Junior Apothecary Holland) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16583', 'Imported from TC', 'I must have lived a charmed life, $N.  To return to the world as a member of the Forsaken and the Apothecary Society... I couldn\'t have asked for a more suitable fate.', '', '44545', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11827', '16583');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11827 WHERE entry = 10665;


-- Importing creature gossip with entry 1747 (Anduin Wrynn) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16641', 'Imported from TC', 'Welcome to Stormwind, $c.', '', '44742', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11874', '16641');
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16642', 'Imported from TC', '<Anduin sizes you up.>$B$B$N, is it? A few of your triumphs have been the talk of the court.$B$BWhat was it like to serve in Northrend? Have you seen the World Tree? Are the stories about Vashj\'ir true? I have so many questions!', '', '44743', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11874', '16642');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11874 WHERE entry = 1747;


-- Importing creature gossip with entry 7770 (Winkey) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16644', 'Imported from TC', '', 'Hey there, cutie!  Mind rubbing some lotion on my exposed areas?', '44756', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11876', '16644');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11876 WHERE entry = 7770;


-- Importing creature gossip with entry 2215 (High Executor Darthalia) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16667', 'Imported from TC', '', 'You will speak when spoken to.', '44831', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11886', '16667');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11886 WHERE entry = 2215;


-- Importing creature gossip with entry 2439 (Major Samuelson) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16719', 'Imported from TC', '<The Major nods curtly.>', '', '45129', '0', '1', '0', '273', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('11909', '16719');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 11909 WHERE entry = 2439;


-- Importing creature gossip with entry 3322 (Kaja) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('16901', 'Imported from TC', '', 'You won\'t find a finer firearm anywhere else in Orgrimmar.', '45865', '0', '1', '0', '1', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12056', '16901');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12056 WHERE entry = 3322;


-- Importing creature gossip with entry 15188 (Cenarion Emissary Blackhoof) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17031', 'Imported from TC', 'Are you here to help the cause? Mount Hyjal burns!', '', '46529', '0', '1', '0', '6', '0', '5', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12125', '17031');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12125 WHERE entry = 15188;


-- Importing creature gossip with entry 15187 (Cenarion Emissary Jademoon) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17035', 'Imported from TC', '', 'Are you here to help the cause? Mount Hyjal burns!', '46543', '0', '1', '0', '6', '0', '5', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12129', '17035');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12129 WHERE entry = 15187;


-- Importing creature gossip with entry 2785 (Theldurin the Lost) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17038', 'Imported from TC', 'You lookin\' for me?', '', '46592', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12131', '17038');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12131 WHERE entry = 2785;


-- Importing creature gossip with entry 2920 (Lucien Tosselwrench) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17140', 'Imported from TC', 'You\'ve got to try some of this beer.', '', '47168', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12202', '17140');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12202 WHERE entry = 2920;


-- Importing creature gossip with entry 4618 (Martek the Exiled) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17151', 'Imported from TC', 'What brings you to a bunch of old, washed-up desert dogs like us?', '', '47240', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12209', '17151');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12209 WHERE entry = 4618;


-- Importing creature gossip with entry 12636 (Georgia) with import type GOSSIP
-- Text 7778 is already imported
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12237', '7778');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12237 WHERE entry = 12636;


-- Importing creature gossip with entry 5843 (Slave Worker) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17235', 'Imported from TC', 'Who are you?', 'Who are you?', '65124', '0', '1', '0', '6', '0', '0', '0', '0', 'Get out of here, before the slavers see you!', 'Get out of here, before the slavers see you!', '47633', '0', '1', '0', '5', '0', '0', '0', '0', 'Are you... here to save me?', 'Are you... here to save me?', '47634', '0', '1', '0', '6', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12271', '17235');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12271 WHERE entry = 5843;


-- Importing creature gossip with entry 14625 (Overseer Oilfist) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17268', 'Imported from TC', 'Welcome to Thorium Point.  As Overseer, I\'m responsible for all the activities of the Thorium Brotherhood.  While that mainly involves mining, smithing, and some minor guard duty, we\'ve lately become more of a martial organization... out of necessity.', '', '47801', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12293', '17268');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12293 WHERE entry = 14625;


-- Importing creature gossip with entry 2397 (Derak Nightfall) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17511', 'Imported from TC', 'Derak Nightfall, at your service!', '', '48868', '0', '1', '0', '2', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12449', '17511');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12449 WHERE entry = 2397;


-- Importing creature gossip with entry 11079 (Wynd Nightchaser) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17586', 'Imported from TC', 'Welcome to Starfall Village, traveler. Have you come to assist us?', '', '49569', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12502', '17586');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12502 WHERE entry = 11079;


-- Importing creature gossip with entry 28572 (Mason Goldgild) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17733', 'Imported from TC', '', 'That statue\'s provin\' ta be quite the pain.  I thought we could lift it up an\' outta the water with a crane an\' some well timed explosives.  But Wick thought we\'d be safer an\' do less damage by rolling it up this ramp.$B$BNow look at it...  We\'ve got it angled all wrong.  An\' once we get it to the top, then what?$B$BI\'ve gotta bad feeling we\'re gonna to be here a long time.  A real long time...', '50269', '7', '1', '0', '1', '3', '5', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12598', '17733');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12598 WHERE entry = 28572;


-- Importing creature gossip with entry 28571 (Foreman Wick) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17735', 'Imported from TC', 'You think after doing such good work on Stormwind Harbor I\'d get a bit of slack from this dwarf...$B$BI just want to get this statue up the ramp and back in place as fast as possible.  But she\'s got a complaint every step of the way.  Her solution to every problem is to blow it up.  It feels like I\'m working with a gnome or a goblin.  But at least they\'d be sober.$B$BLook at our work crew, no discipline.  You know why?  Because she undermines me!$B$BA bad attitude, a lazy crew... This isn\'t going to be a quick fix at all.', '', '50270', '0', '1', '0', '1', '3', '5', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12600', '17735');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12600 WHERE entry = 28571;


-- Importing creature gossip with entry 2771 (Drum Fel) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17772', 'Imported from TC', 'We\'re glad to have your help.', '', '50624', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12636', '17772');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12636 WHERE entry = 2771;


-- Importing creature gossip with entry 9177 (Oralius) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17804', 'Imported from TC', 'What?', 'What?', '72224', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12662', '17804');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12662 WHERE entry = 9177;


-- Importing creature gossip with entry 10260 (Kibler) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17805', 'Imported from TC', 'What?', 'What?', '72224', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12663', '17805');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12663 WHERE entry = 10260;


-- Importing creature gossip with entry 3233 (Lorekeeper Raintotem) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17838', 'Imported from TC', 'Only the most valiant and honorable amongst the tribe can earn the honor of being laid to rest at Red Rocks.', '', '50751', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12707', '17838');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12707 WHERE entry = 3233;


-- Importing creature gossip with entry 6301 (Gorbold Steelhand) with import type GOSSIP
-- Text 3218 already present in Sun DB
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12726', '3218');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12726 WHERE entry = 6301;


-- Importing creature gossip with entry 8320 (Sprok) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('17880', 'Imported from TC', 'This teleporter will transport you to Grom\'gol in Northern Stranglethorn.$B$BUsually.$B$BI mean, sometimes it\'ll just disintegrate you, or reduce all your soft tissues into a greenish, foul-smelling mist, but mostly it\'s safe!$B$B...mostly.', '', '50915', '1', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12741', '17880');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12741 WHERE entry = 8320;


-- Importing creature gossip with entry 4941 (Caz Twosprocket) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('18070', 'Imported from TC', 'With all the Siege Engines nearly complete, I\'m getting a bit bored!\r\n\r\n\r\n\r\nYou\'re not an engineer, are you? Care to compare blueprints...?', '', '51958', '0', '1', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('12846', '18070');
UPDATE creature_template SET npcflag = (npcflag | 1), gossip_menu_id = 12846 WHERE entry = 4941;


-- Importing creature gossip with entry 2993 (Baine Bloodhoof) with import type GOSSIP
INSERT INTO gossip_text (`ID`, `comment`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `em0_0`, `em0_1`, `em0_2`, `em0_3`, `em0_4`, `em0_5`, `text1_0`, `text1_1`, `BroadcastTextID1`, `lang1`, `Probability1`, `em1_0`, `em1_1`, `em1_2`, `em1_3`, `em1_4`, `em1_5`, `text2_0`, `text2_1`, `BroadcastTextID2`, `lang2`, `Probability2`, `em2_0`, `em2_1`, `em2_2`, `em2_3`, `em2_4`, `em2_5`, `text3_0`, `text3_1`, `BroadcastTextID3`, `lang3`, `Probability3`, `em3_0`, `em3_1`, `em3_2`, `em3_3`, `em3_4`, `em3_5`, `text4_0`, `text4_1`, `BroadcastTextID4`, `lang4`, `Probability4`, `em4_0`, `em4_1`, `em4_2`, `em4_3`, `em4_4`, `em4_5`, `text5_0`, `text5_1`, `BroadcastTextID5`, `lang5`, `Probability5`, `em5_0`, `em5_1`, `em5_2`, `em5_3`, `em5_4`, `em5_5`, `text6_0`, `text6_1`, `BroadcastTextID6`, `lang6`, `Probability6`, `em6_0`, `em6_1`, `em6_2`, `em6_3`, `em6_4`, `em6_5`, `text7_0`, `text7_1`, `BroadcastTextID7`, `lang7`, `Probability7`, `em7_0`, `em7_1`, `em7_2`, `em7_3`, `em7_4`, `em7_5`) VALUES ('50000', 'Imported from TC', 'The land has been good to our people, $c. We must be thankful for our good fortune.', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0', '', '', NULL, '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO gossip_menu (`MenuID`, `TextID`) VALUES ('50000', '50000');
UPDATE creature_template SET gossip_menu_id = 50000 WHERE entry = 2993;


