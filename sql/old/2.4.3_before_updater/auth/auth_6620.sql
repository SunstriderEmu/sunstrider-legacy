
DROP TABLE IF EXISTS `account_credits`;

CREATE TABLE `account_credits` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `amount` int(11) unsigned NOT NULL DEFAULT '0',
  `last_update` bigint(20) unsigned NOT NULL DEFAULT '0',
  `from` varchar(16) NOT NULL DEFAULT 'Shop',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;