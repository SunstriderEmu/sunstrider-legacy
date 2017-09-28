DROP TABLE IF EXISTS `lottery`;
CREATE TABLE `lottery` (
  `guid` int(11) unsigned NOT NULL default '0',
  `accountid` int(11) unsigned NOT NULL default '0',
  `registertime` bigint(20) unsigned NOT NULL default '0',
  `faction` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`accountid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
