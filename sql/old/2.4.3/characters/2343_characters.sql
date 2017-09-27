DROP TABLE IF EXISTS `channel_ban`;
CREATE TABLE `channel_ban` (
  `accountid` int(11) unsigned NOT NULL default '0',
  `expire` int(11) unsigned NOT NULL default '0',
  `channel` varchar(20) NOT NULL default '',
  `reason` varchar(50) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
