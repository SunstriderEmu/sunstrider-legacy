ALTER TABLE `gossip_menu`   
  CHANGE `entry` `MenuID` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `text_id` `TextID` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL;

ALTER TABLE `gossip_menu_option`   
	DROP FOREIGN KEY `pk_action_menuId`,
	DROP FOREIGN KEY `pk_broadcast1`,
	DROP FOREIGN KEY `pk_broadcast2`,
	DROP FOREIGN KEY `pk_menuId`,
	DROP FOREIGN KEY `pk_poi`;

ALTER TABLE `gossip_menu_option`   
  CHANGE `menu_id` `MenuID` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL;
  
ALTER TABLE `gossip_menu_option`   
  CHANGE `id` `OptionID` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `option_icon` `OptionIcon` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `option_text` `OptionText` TEXT CHARSET utf8 COLLATE utf8_general_ci NULL,
  CHANGE `option_id` `OptionType` TINYINT(3) UNSIGNED DEFAULT 1  NOT NULL  COMMENT 'action type (such as gossip, trainer, ...)',
  CHANGE `npc_option_npcflag` `OptionNpcFlag` INT(10) UNSIGNED DEFAULT 1  NOT NULL,
  CHANGE `action_menu_id` `ActionMenuID` SMALLINT(5) UNSIGNED NULL,
  CHANGE `action_poi_id` `ActionPoiID` MEDIUMINT(8) UNSIGNED NULL,
  CHANGE `box_coded` `BoxCoded` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `box_money` `BoxMoney` INT(10) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `box_text` `BoxText` TEXT CHARSET utf8 COLLATE utf8_general_ci NULL;

UPDATE gossip_menu_option SET ActionMenuID = NULL WHERE ActionMenuId = 0;

ALTER TABLE `gossip_menu_option`   
  ADD CONSTRAINT `pk_action_menuId` FOREIGN KEY (`ActionMenuID`) REFERENCES `gossip_menu` (`MenuID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `pk_broadcast1` FOREIGN KEY (`OptionBroadcastTextID`) REFERENCES `broadcast_text` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `pk_broadcast2` FOREIGN KEY (`BoxBroadcastTextID`) REFERENCES `broadcast_text` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `pk_menuId` FOREIGN KEY (`MenuID`) REFERENCES `gossip_menu` (`MenuID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `pk_poi` FOREIGN KEY (`ActionPoiID`) REFERENCES `points_of_interest` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;
	
ALTER TABLE `gossip_menu_option_generic`   
  CHANGE `id` `MenuID` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `option_icon` `OptionIcon` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `option_text` `OptionText` TEXT CHARSET utf8 COLLATE utf8_general_ci NULL,
  CHANGE `option_id` `OptionType` TINYINT(3) UNSIGNED DEFAULT 1  NOT NULL,
  CHANGE `npc_option_npcflag` `OptionNpcFlag` INT(10) UNSIGNED DEFAULT 1  NOT NULL;

ALTER TABLE `gossip_text`   
  CHANGE `prob0` `Probability0` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob1` `Probability1` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob2` `Probability2` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob3` `Probability3` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob4` `Probability4` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob5` `Probability5` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob6` `Probability6` FLOAT DEFAULT 0  NOT NULL,
  CHANGE `prob7` `Probability7` FLOAT DEFAULT 0  NOT NULL;
