
DROP TABLE IF EXISTS `anticheat_movement`;

CREATE TABLE `anticheat_movement` (
  `time` int(10) unsigned NOT NULL,
  `player` int(11) unsigned NOT NULL,
  `account` int(11) unsigned NOT NULL,
  `reason` varchar(25) NOT NULL,
  `speed` float DEFAULT NULL,
  `opcode` varchar(50) NOT NULL,
  `val1` float DEFAULT NULL,
  `val2` int(11) unsigned DEFAULT NULL,
  `mapid` smallint(5) unsigned NOT NULL,
  `posX` float NOT NULL,
  `posY` float NOT NULL,
  `posZ` float NOT NULL,
  `oldPosX` float DEFAULT NULL,
  `oldPosY` float DEFAULT NULL,
  `oldPosZ` float DEFAULT NULL,
  `level` tinyint(3) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
