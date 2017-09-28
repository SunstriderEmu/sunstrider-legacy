DROP TABLE IF EXISTS `boss_down`;
CREATE TABLE `boss_down` (
  `id` mediumint(8) unsigned NOT NULL auto_increment,
  `boss_entry` mediumint(8) unsigned NOT NULL,
  `boss_name` varchar(100) NOT NULL,
  `guild_id` mediumint(8) unsigned NOT NULL,
  `guild_name` varchar(100) NOT NULL,
  `time` bigint(11) NOT NULL,
  `guild_percentage` float NOT NULL,
  PRIMARY KEY(`id`),
  KEY `idx_boss` (`boss_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
