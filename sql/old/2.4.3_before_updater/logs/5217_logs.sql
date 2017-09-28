DROP TABLE IF EXISTS `char_delete`;
CREATE TABLE `char_delete` (
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) NOT NULL DEFAULT '',
  `time` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `idx_acct` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

