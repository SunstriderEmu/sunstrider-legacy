DROP TABLE IF EXISTS `mute`;
CREATE TABLE `mute` (
  `acctid` bigint(11) NOT NULL default '0' COMMENT 'Account ID',
  `duration` mediumint(8) unsigned NOT NULL default '0' COMMENT 'Mute duration',
  `time` bigint(11) NOT NULL default '0' COMMENT 'Current time',
  `reason` varchar(255) NOT NULL default 'Unspecified'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
