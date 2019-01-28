-- Fix Warrant Officer Tracy Proudwell not offering her goods
UPDATE gossip_menu_option SET OptionText = "<back>" WHERE MenuID = 20064 AND OptionID = 0;
INSERT IGNORE INTO gossip_menu_option (MenuID, OptionID, OptionIcon, OptionText, OptionBroadcastTextID, OptionType, OptionNpcFlag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, BoxBroadcastTextID, patch_min, patch_max)
VALUES ('9656', '0', '1', 'I have marks to redeem!', NULL, '3', '128', NULL, NULL, '0', '0', NULL, NULL, '0', '10');
