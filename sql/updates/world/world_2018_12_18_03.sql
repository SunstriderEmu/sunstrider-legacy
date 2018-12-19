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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
