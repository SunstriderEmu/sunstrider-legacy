DROP TABLE IF EXISTS `quest_bugs`;
CREATE TABLE `quest_bugs` (
  `entry` smallint unsigned NOT NULL default '0',
  `completecount` mediumint unsigned NOT NULL default '0',
  `bugged` tinyint unsigned NOT NULL default '0',
  `comment` varchar(50),
  PRIMARY KEY (entry)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO command VALUES ("quest report", 2, "Utilisation: .quest report <numero (cf wowhead)> <commentaire>");
