/*
SQLyog Community v12.09 (64 bit)
MySQL - 5.5.41-MariaDB-1~wheezy-log : Database - logs
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`logs` /*!40100 DEFAULT CHARACTER SET latin1 */;

/*Table structure for table `arena_match` */

DROP TABLE IF EXISTS `arena_match`;

CREATE TABLE `arena_match` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `team1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `team1_name` varchar(255) NOT NULL DEFAULT '',
  `team1_member1` int(10) unsigned DEFAULT NULL,
  `team1_member1_ip` varchar(30) DEFAULT NULL,
  `team1_member1_heal` mediumint(8) unsigned DEFAULT NULL,
  `team1_member1_damage` mediumint(8) unsigned DEFAULT NULL,
  `team1_member1_kills` tinyint(3) unsigned DEFAULT NULL,
  `team1_member2` int(10) unsigned DEFAULT NULL,
  `team1_member2_ip` varchar(30) DEFAULT NULL,
  `team1_member2_heal` mediumint(8) unsigned DEFAULT NULL,
  `team1_member2_damage` mediumint(8) unsigned DEFAULT NULL,
  `team1_member2_kills` tinyint(3) unsigned DEFAULT NULL,
  `team1_member3` int(10) unsigned DEFAULT NULL,
  `team1_member3_ip` varchar(30) DEFAULT NULL,
  `team1_member3_heal` mediumint(8) unsigned DEFAULT NULL,
  `team1_member3_damage` mediumint(8) unsigned DEFAULT NULL,
  `team1_member3_kills` tinyint(3) unsigned DEFAULT NULL,
  `team1_member4` int(10) unsigned DEFAULT NULL,
  `team1_member4_ip` varchar(30) DEFAULT NULL,
  `team1_member4_heal` mediumint(8) unsigned DEFAULT NULL,
  `team1_member4_damage` mediumint(8) unsigned DEFAULT NULL,
  `team1_member4_kills` tinyint(3) unsigned DEFAULT NULL,
  `team1_member5` int(10) unsigned DEFAULT NULL,
  `team1_member5_ip` varchar(30) DEFAULT NULL,
  `team1_member5_heal` mediumint(8) unsigned DEFAULT NULL,
  `team1_member5_damage` mediumint(8) unsigned DEFAULT NULL,
  `team1_member5_kills` tinyint(3) unsigned DEFAULT NULL,
  `team2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `team2_name` varchar(255) NOT NULL DEFAULT '',
  `team2_member1` int(10) unsigned DEFAULT NULL,
  `team2_member1_ip` varchar(30) DEFAULT NULL,
  `team2_member1_heal` mediumint(8) unsigned DEFAULT NULL,
  `team2_member1_damage` mediumint(8) unsigned DEFAULT NULL,
  `team2_member1_kills` tinyint(3) unsigned DEFAULT NULL,
  `team2_member2` int(10) unsigned DEFAULT NULL,
  `team2_member2_ip` varchar(30) DEFAULT NULL,
  `team2_member2_heal` mediumint(8) unsigned DEFAULT NULL,
  `team2_member2_damage` mediumint(8) unsigned DEFAULT NULL,
  `team2_member2_kills` tinyint(3) unsigned DEFAULT NULL,
  `team2_member3` int(10) unsigned DEFAULT NULL,
  `team2_member3_ip` varchar(30) DEFAULT NULL,
  `team2_member3_heal` mediumint(8) unsigned DEFAULT NULL,
  `team2_member3_damage` mediumint(8) unsigned DEFAULT NULL,
  `team2_member3_kills` tinyint(3) unsigned DEFAULT NULL,
  `team2_member4` int(10) unsigned DEFAULT NULL,
  `team2_member4_ip` varchar(30) DEFAULT NULL,
  `team2_member4_heal` mediumint(8) unsigned DEFAULT NULL,
  `team2_member4_damage` mediumint(8) unsigned DEFAULT NULL,
  `team2_member4_kills` tinyint(3) unsigned DEFAULT NULL,
  `team2_member5` int(10) unsigned DEFAULT NULL,
  `team2_member5_ip` varchar(30) DEFAULT NULL,
  `team2_member5_heal` mediumint(8) unsigned DEFAULT NULL,
  `team2_member5_damage` mediumint(8) unsigned DEFAULT NULL,
  `team2_member5_kills` tinyint(3) unsigned DEFAULT NULL,
  `start_time` bigint(20) NOT NULL DEFAULT '0',
  `end_time` bigint(20) NOT NULL DEFAULT '0',
  `winner` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `rating_change` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `winner_rating` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `loser_rating` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `arena_season_stats` */

DROP TABLE IF EXISTS `arena_season_stats`;

CREATE TABLE `arena_season_stats` (
  `teamid` int(10) unsigned NOT NULL DEFAULT '0',
  `time1` int(10) unsigned DEFAULT '0' COMMENT 'Time spent first in minutes',
  `time2` int(10) unsigned DEFAULT '0' COMMENT 'Time spent second in minutes',
  `time3` int(10) unsigned DEFAULT '0' COMMENT 'Time spent third in minutes',
  PRIMARY KEY (`teamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `arena_team_event` */

DROP TABLE IF EXISTS `arena_team_event`;

CREATE TABLE `arena_team_event` (
  `id` int(10) unsigned NOT NULL,
  `event` tinyint(3) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `player` int(10) unsigned NOT NULL,
  `ip` varchar(16) NOT NULL DEFAULT '0.0.0.0',
  `time` bigint(20) unsigned NOT NULL,
  KEY `idx_arena_team_event_id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `bg_stats` */

DROP TABLE IF EXISTS `bg_stats`;

CREATE TABLE `bg_stats` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `mapid` mediumint(8) unsigned NOT NULL,
  `start_time` bigint(20) NOT NULL COMMENT 'Start timestamp',
  `end_time` bigint(20) NOT NULL COMMENT 'End timestamp',
  `winner` tinyint(4) NOT NULL COMMENT '0 = Alliance - 1 = Horde',
  `score_alliance` mediumint(9) NOT NULL,
  `score_horde` mediumint(9) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `boss_down` */

DROP TABLE IF EXISTS `boss_down`;

CREATE TABLE `boss_down` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `boss_entry` mediumint(8) unsigned NOT NULL,
  `boss_name` varchar(100) NOT NULL,
  `boss_name_fr` varchar(100) NOT NULL,
  `guild_id` mediumint(8) unsigned NOT NULL,
  `guild_name` varchar(100) NOT NULL,
  `time` bigint(11) NOT NULL,
  `guild_percentage` float NOT NULL,
  `leaderGuid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `idx_boss` (`boss_entry`)
) ENGINE=Aria AUTO_INCREMENT=134695 DEFAULT CHARSET=utf8 PAGE_CHECKSUM=1;

/*Table structure for table `char_delete` */

DROP TABLE IF EXISTS `char_delete`;

CREATE TABLE `char_delete` (
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) NOT NULL DEFAULT '',
  `time` bigint(20) NOT NULL DEFAULT '0',
  `ip` varchar(16) NOT NULL DEFAULT '0.0.0.0',
  PRIMARY KEY (`guid`),
  KEY `idx_acct` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `char_rename` */

DROP TABLE IF EXISTS `char_rename`;

CREATE TABLE `char_rename` (
  `account` int(10) unsigned NOT NULL,
  `guid` int(10) unsigned NOT NULL,
  `old_name` varchar(12) NOT NULL,
  `new_name` varchar(12) NOT NULL,
  `time` bigint(20) NOT NULL,
  `ip` varchar(16) NOT NULL DEFAULT '0.0.0.0',
  KEY `idx_char_rename_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `characters_OLD` */

DROP TABLE IF EXISTS `characters_OLD`;

CREATE TABLE `characters_OLD` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `account` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `data` longtext,
  `name` varchar(12) NOT NULL DEFAULT '',
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `playerFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `map` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `instance_id` int(11) unsigned NOT NULL DEFAULT '0',
  `dungeon_difficulty` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `taximask` longtext,
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `cinematic` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `totaltime` int(11) unsigned NOT NULL DEFAULT '0',
  `leveltime` int(11) unsigned NOT NULL DEFAULT '0',
  `logout_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `is_logout_resting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rest_bonus` float NOT NULL DEFAULT '0',
  `resettalents_cost` int(11) unsigned NOT NULL DEFAULT '0',
  `resettalents_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `trans_x` float NOT NULL DEFAULT '0',
  `trans_y` float NOT NULL DEFAULT '0',
  `trans_z` float NOT NULL DEFAULT '0',
  `trans_o` float NOT NULL DEFAULT '0',
  `transguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `extra_flags` int(11) unsigned NOT NULL DEFAULT '0',
  `stable_slots` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `at_login` int(11) unsigned NOT NULL DEFAULT '0',
  `zone` int(11) unsigned NOT NULL DEFAULT '0',
  `death_expire_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `taxi_path` text,
  `arena_pending_points` int(10) unsigned NOT NULL DEFAULT '0',
  `arenaPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `totalHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `todayHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `yesterdayHonorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `totalKills` int(10) unsigned NOT NULL DEFAULT '0',
  `todayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  `yesterdayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  `chosenTitle` int(10) unsigned NOT NULL DEFAULT '0',
  `watchedFaction` int(10) NOT NULL DEFAULT '0',
  `drunk` smallint(5) unsigned NOT NULL DEFAULT '0',
  `health` int(10) unsigned NOT NULL DEFAULT '0',
  `power1` int(10) unsigned NOT NULL DEFAULT '0',
  `power2` int(10) unsigned NOT NULL DEFAULT '0',
  `power3` int(10) unsigned NOT NULL DEFAULT '0',
  `power4` int(10) unsigned NOT NULL DEFAULT '0',
  `power5` int(10) unsigned NOT NULL DEFAULT '0',
  `latency` int(11) unsigned NOT NULL DEFAULT '0',
  `exploredZones` longtext,
  `equipmentCache` longtext,
  `ammoId` int(10) unsigned NOT NULL DEFAULT '0',
  `knownTitles` longtext,
  `actionBars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `xp_blocked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `lastGenderChange` bigint(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `idx_account` (`account`),
  KEY `idx_online` (`online`),
  KEY `idx_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

/*Table structure for table `gm_command` */

DROP TABLE IF EXISTS `gm_command`;

CREATE TABLE `gm_command` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` int(11) NOT NULL DEFAULT '0',
  `gmlevel` int(11) NOT NULL DEFAULT '0',
  `time` int(11) NOT NULL DEFAULT '0',
  `map` int(11) NOT NULL DEFAULT '0',
  `selection` int(11) NOT NULL DEFAULT '0',
  `command` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `item_delete` */

DROP TABLE IF EXISTS `item_delete`;

CREATE TABLE `item_delete` (
  `playerguid` int(11) unsigned NOT NULL COMMENT 'player guid',
  `entry` mediumint(8) unsigned NOT NULL COMMENT 'item entry',
  `count` smallint(5) unsigned NOT NULL DEFAULT '1' COMMENT 'item count',
  `time` bigint(20) unsigned NOT NULL COMMENT 'delete time',
  PRIMARY KEY (`playerguid`,`entry`,`count`,`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `item_mail` */

DROP TABLE IF EXISTS `item_mail`;

CREATE TABLE `item_mail` (
  `senderguid` int(11) unsigned DEFAULT NULL,
  `receiverguid` int(11) unsigned NOT NULL,
  `itemguid` int(11) unsigned NOT NULL,
  `itementry` mediumint(8) unsigned NOT NULL,
  `itemcount` smallint(5) unsigned NOT NULL,
  `time` bigint(20) unsigned NOT NULL COMMENT 'send time'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `mj_manager` */

DROP TABLE IF EXISTS `mj_manager`;

CREATE TABLE `mj_manager` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` varchar(32) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `request` varchar(255) NOT NULL,
  `query` text NOT NULL,
  `logged_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `mon_classes` */

DROP TABLE IF EXISTS `mon_classes`;

CREATE TABLE `mon_classes` (
  `time` int(10) unsigned NOT NULL,
  `class` tinyint(3) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_maps` */

DROP TABLE IF EXISTS `mon_maps`;

CREATE TABLE `mon_maps` (
  `time` int(10) unsigned NOT NULL,
  `map` mediumint(8) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_players` */

DROP TABLE IF EXISTS `mon_players`;

CREATE TABLE `mon_players` (
  `time` int(10) unsigned NOT NULL,
  `active` int(10) unsigned NOT NULL,
  `queued` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_races` */

DROP TABLE IF EXISTS `mon_races`;

CREATE TABLE `mon_races` (
  `time` int(10) unsigned NOT NULL,
  `race` tinyint(3) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_timediff` */

DROP TABLE IF EXISTS `mon_timediff`;

CREATE TABLE `mon_timediff` (
  `time` int(10) unsigned NOT NULL,
  `diff` mediumint(8) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `phishing` */

DROP TABLE IF EXISTS `phishing`;

CREATE TABLE `phishing` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `srcguid` int(11) NOT NULL,
  `dstguid` int(11) NOT NULL,
  `time` bigint(20) NOT NULL,
  `data` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `sanctions` */

DROP TABLE IF EXISTS `sanctions`;

CREATE TABLE `sanctions` (
  `acctid` bigint(11) NOT NULL DEFAULT '0' COMMENT 'Account ID',
  `author` bigint(11) NOT NULL DEFAULT '0',
  `type` tinyint(3) NOT NULL DEFAULT '0' COMMENT '0 = mute, 1 = ingame ban, 2 = ip ban, 3 = forum ban',
  `duration` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Mute duration',
  `time` bigint(11) NOT NULL DEFAULT '0' COMMENT 'Current time',
  `reason` varchar(255) NOT NULL DEFAULT 'Unspecified'
) ENGINE=Aria DEFAULT CHARSET=utf8 PAGE_CHECKSUM=1;

/*Table structure for table `warden_fails` */

DROP TABLE IF EXISTS `warden_fails`;

CREATE TABLE `warden_fails` (
  `guid` int(11) unsigned NOT NULL,
  `account` int(11) unsigned NOT NULL,
  `check_id` int(4) unsigned NOT NULL,
  `comment` text NOT NULL,
  `time` bigint(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
