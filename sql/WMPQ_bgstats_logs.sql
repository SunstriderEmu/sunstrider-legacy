DROP TABLE IF EXISTS `bg_stats`;
CREATE TABLE `bg_stats` (
  `id` mediumint unsigned NOT NULL auto_increment,
  `mapid` mediumint unsigned NOT NULL,
  `start_time` bigint NOT NULL COMMENT 'Start timestamp',
  `end_time` bigint NOT NULL COMMENT 'End timestamp',
  `winner` tinyint NOT NULL COMMENT '0 = Alliance - 1 = Horde',
  `score_alliance` mediumint NOT NULL,
  `score_horde` mediumint NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
