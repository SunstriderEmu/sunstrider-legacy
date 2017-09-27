DROP TABLE IF EXISTS `wrchat_servers`;
CREATE TABLE `wrchat_servers` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `host` varchar(50) NOT NULL DEFAULT '127.0.0.1',
  `port` int(10) unsigned NOT NULL DEFAULT '6667',
  `ssl` tinyint(4) NOT NULL DEFAULT '0',
  `nick` varchar(50) NOT NULL DEFAULT 'W',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `wrchat_channels`;
CREATE TABLE `wrchat_channels` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `server` int(10) unsigned NOT NULL DEFAULT '0',
  `irc_channel` varchar(50) NOT NULL DEFAULT '',
  `ingame_channel` varchar(50) NOT NULL DEFAULT '',
  `channel_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `join_message` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `server` (`server`),
  CONSTRAINT `wrchat_channels_ibfk_1` FOREIGN KEY (`server`) REFERENCES `wrchat_servers` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
