DROP TABLE IF EXISTS `mute`;
CREATE TABLE `sanctions` (
  `acctid` bigint(11) NOT NULL default '0' COMMENT 'Account ID',
  `type` tinyint(3) NOT NULL default '0' COMMENT '0 = mute, 1 = ingame ban, 2 = ip ban, 3 = forum ban',
  `duration` mediumint(8) unsigned NOT NULL default '0' COMMENT 'Mute duration',
  `time` bigint(11) NOT NULL default '0' COMMENT 'Current time',
  `reason` varchar(255) NOT NULL default 'Unspecified'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
