DROP TABLE IF EXISTS `points_of_interest`;
CREATE TABLE `points_of_interest` (
  `ID` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `PositionX` float NOT NULL DEFAULT 0,
  `PositionY` float NOT NULL DEFAULT 0,
  `Icon` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `Flags` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `Importance` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `Name` text NOT NULL,
  `VerifiedBuild` smallint(5) DEFAULT 0,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `gossip_menu_option`  
  CHARSET=utf8, COLLATE=utf8_general_ci;

ALTER TABLE `gossip_menu_option`   
  CHANGE `action_poi_id` `action_poi_id` MEDIUMINT(8) UNSIGNED,
  CHANGE `OptionBroadcastTextID` `OptionBroadcastTextID` INT(11) UNSIGNED,
  CHANGE `BoxBroadcastTextID` `BoxBroadcastTextID` INT(11) UNSIGNED;
  
UPDATE gossip_menu_option SET action_poi_id = NULL WHERE action_poi_id = 0;
UPDATE gossip_menu_option SET OptionBroadcastTextID = NULL WHERE OptionBroadcastTextID = 0;
UPDATE gossip_menu_option SET BoxBroadcastTextID = NULL WHERE BoxBroadcastTextID = 0;

-- Missing values
replace into `broadcast_text` (`ID`, `Language`, `MaleText`, `FemaleText`, `EmoteID0`, `EmoteID1`, `EmoteID2`, `EmoteDelay0`, `EmoteDelay1`, `EmoteDelay2`, `SoundId`, `Unk1`, `Unk2`, `VerifiedBuild`) values('14217','0','Oh, grandmother, what big ears you have.','Oh, grandmother, what big ears you have.','0','0','0','0','0','0','0','0','1','18019');
replace into `broadcast_text` (`ID`, `Language`, `MaleText`, `FemaleText`, `EmoteID0`, `EmoteID1`, `EmoteID2`, `EmoteDelay0`, `EmoteDelay1`, `EmoteDelay2`, `SoundId`, `Unk1`, `Unk2`, `VerifiedBuild`) values('14219','0','Oh, grandmother, what big eyes you have.','Oh, grandmother, what big eyes you have.','0','0','0','0','0','0','0','0','1','18019');

ALTER TABLE `gossip_menu_option`   
  ADD CONSTRAINT `pk_poi` FOREIGN KEY (`action_poi_id`) REFERENCES `points_of_interest`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `pk_broadcast1` FOREIGN KEY (`OptionBroadcastTextID`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `pk_broadcast2` FOREIGN KEY (`BoxBroadcastTextID`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL;
