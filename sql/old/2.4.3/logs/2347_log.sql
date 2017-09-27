DROP TABLE IF EXISTS `gm_command`;
CREATE TABLE `gm_command` (
  `id` int NOT NULL auto_increment,
  `account` int NOT NULL default 0,
  `gmlevel` int NOT NULL default 0,
  `time` int NOT NULL default 0,
  `map` int NOT NULL default 0,
  `selection` int NOT NULL default 0,
  `command` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
