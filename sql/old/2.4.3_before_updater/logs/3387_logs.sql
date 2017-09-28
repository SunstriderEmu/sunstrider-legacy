DROP TABLE IF EXISTS `phishing`;
CREATE TABLE `phishing` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `srcguid` int(11) NOT NULL,
  `dstguid` int(11) NOT NULL,
  `time` bigint(20) NOT NULL,
  `data` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
