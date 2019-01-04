-- Add gossip to Wanted Poster in Orebor Harborage, Zangarmarsh
REPLACE INTO gossip_text (ID, comment, text0_0, text0_1, BroadcastTextID0, lang0, Probability0, em0_0, em0_1, em0_2, em0_3, em0_4, em0_5, text1_0, text1_1, BroadcastTextID1, lang1, Probability1, em1_0, em1_1, em1_2, em1_3, em1_4, em1_5, text2_0, text2_1, BroadcastTextID2, lang2, Probability2, em2_0, em2_1, em2_2, em2_3, em2_4, em2_5, text3_0, text3_1, BroadcastTextID3, lang3, Probability3, em3_0, em3_1, em3_2, em3_3, em3_4, em3_5, text4_0, text4_1, BroadcastTextID4, lang4, Probability4, em4_0, em4_1, em4_2, em4_3, em4_4, em4_5, text5_0, text5_1, BroadcastTextID5, lang5, Probability5, em5_0, em5_1, em5_2, em5_3, em5_4, em5_5, text6_0, text6_1, BroadcastTextID6, lang6, Probability6, em6_0, em6_1, em6_2, em6_3, em6_4, em6_5, text7_0, text7_1, BroadcastTextID7, lang7, Probability7, em7_0, em7_1, em7_2, em7_3, em7_4, em7_5)
VALUES ('1872752', 'Wanted Poster in Orebor Harborage, Zangarmarsh', 'This old draenei board has been refurbished by the Kurenai and is now used to post announcements and bounties.', NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0', NULL, NULL, NULL, '0', '0', '0', '0', '0', '0', '0', '0');
REPLACE INTO gossip_menu (MenuID, TextID, patch_min, patch_max)
VALUES ('7945', '1872752', '0', '10');

-- Add gossip action to Timothy Daniels
REPLACE INTO gossip_menu (MenuID, TextID, patch_min, patch_max)
VALUES ('9239', '9239', '0', '10');
REPLACE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max) VALUES
('9238', '0', '1', 'Let me browse your reagents and poison supplies.', NULL, '3', '128', NULL, NULL, '0', '0', NULL, NULL, '0', '10'),
('9238', '1', '0', 'Specialist, eh? Just what kind of specialist are you, anyway?', NULL, '1', '1', '9239', NULL, '0', '0', NULL, NULL, '0', '10');
