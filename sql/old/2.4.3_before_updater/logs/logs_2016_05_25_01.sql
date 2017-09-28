
DROP TABLE IF EXISTS `account_ip`;

CREATE TABLE `account_ip` (
  `id` int(10) unsigned NOT NULL,
  `time` int(11) unsigned NOT NULL,
  `ip` varchar(15) CHARACTER SET utf8 NOT NULL,
  `gm_involved` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`id`,`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
