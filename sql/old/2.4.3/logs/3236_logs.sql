DROP TABLE IF EXISTS `arena_match`;

CREATE TABLE `arena_match` (
  `id` mediumint(8) unsigned NOT NULL auto_increment,
  `type` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Arena type : 2, 3 or 5',
  `team1` mediumint(8) unsigned NOT NULL default '0' COMMENT 'Team 1 GUID',
  `team2` mediumint(8) unsigned NOT NULL default '0' COMMENT 'Team 2 GUID',
  `start_time` bigint(11) NOT NULL default '0' COMMENT 'Start time',
  `end_time` bigint(11) NOT NULL default '0' COMMENT 'End time',
  `winner` mediumint(8) unsigned NOT NULL default '0' COMMENT 'Winning team GUID',
  `rating_change` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Rating change for both teams',
  `winner_rating` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Winner new rating',
  `loser_rating` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Loser new rating',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `arena_match_player`;

CREATE TABLE `arena_match_player` (
  `match_id` mediumint(8) unsigned NOT NULL default '0',
  `player_guid` int(11) unsigned NOT NULL default '0',
  `player_name` varchar(12) NOT NULL default '',
  `team` mediumint(8) unsigned NOT NULL default '0',
  `ip` varchar(30) NOT NULL default '127.0.0.1',
  `heal` mediumint(8) unsigned NOT NULL default '0',
  `damage` mediumint(8) unsigned NOT NULL default '0',
  `killing_blows` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY (`match_id`, `player_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
