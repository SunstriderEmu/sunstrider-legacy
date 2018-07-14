DROP TABLE IF EXISTS `groups`;

CREATE TABLE `groups` (
  `guid` int(10) unsigned NOT NULL,
  `leaderGuid` int(10) unsigned NOT NULL,
  `lootMethod` tinyint(3) unsigned NOT NULL,
  `looterGuid` int(10) unsigned NOT NULL,
  `lootThreshold` tinyint(3) unsigned NOT NULL,
  `icon1` bigint(20) unsigned NOT NULL,
  `icon2` bigint(20) unsigned NOT NULL,
  `icon3` bigint(20) unsigned NOT NULL,
  `icon4` bigint(20) unsigned NOT NULL,
  `icon5` bigint(20) unsigned NOT NULL,
  `icon6` bigint(20) unsigned NOT NULL,
  `icon7` bigint(20) unsigned NOT NULL,
  `icon8` bigint(20) unsigned NOT NULL,
  `groupType` tinyint(3) unsigned NOT NULL,
  `difficulty` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `raidDifficulty` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `masterLooterGuid` int(10) unsigned NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `leaderGuid` (`leaderGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Groups';

DROP TABLE IF EXISTS `group_member`;

CREATE TABLE `group_member` (
  `guid` int(10) unsigned NOT NULL,
  `memberGuid` int(10) unsigned NOT NULL,
  `memberFlags` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `subgroup` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `roles` tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`memberGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Groups';

DROP TABLE IF EXISTS `group_instance`;

CREATE TABLE `group_instance` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0,
  `instance` int(10) unsigned NOT NULL DEFAULT 0,
  `permanent` tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;