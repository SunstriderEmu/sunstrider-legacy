DROP TABLE IF EXISTS `completed_quests`;
CREATE TABLE `completed_quests` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `count` tinyint(3) UNSIGNED DEFAULT '0' NOT NULL COMMENT 'Completions this week',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
