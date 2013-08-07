/*
SQLyog Community v9.63 
MySQL - 5.6.10-log : Database - wrlogs
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `arena_match` */

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
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=latin1;

/*Table structure for table `bg_stats` */

CREATE TABLE `bg_stats` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `mapid` mediumint(8) unsigned NOT NULL,
  `start_time` bigint(20) NOT NULL COMMENT 'Start timestamp',
  `end_time` bigint(20) NOT NULL COMMENT 'End timestamp',
  `winner` tinyint(4) NOT NULL COMMENT '0 = Alliance - 1 = Horde',
  `score_alliance` mediumint(9) NOT NULL,
  `score_horde` mediumint(9) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

/*Table structure for table `boss_down` */

CREATE TABLE `boss_down` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `boss_entry` mediumint(8) unsigned NOT NULL,
  `boss_name` varchar(100) NOT NULL,
  `guild_id` mediumint(8) unsigned NOT NULL,
  `guild_name` varchar(100) NOT NULL,
  `time` bigint(11) NOT NULL,
  `guild_percentage` float NOT NULL,
  `leaderGuid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `idx_boss` (`boss_entry`)
) ENGINE=MyISAM AUTO_INCREMENT=172 DEFAULT CHARSET=utf8;

/*Table structure for table `char_delete` */

CREATE TABLE `char_delete` (
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) NOT NULL DEFAULT '',
  `time` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `idx_acct` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `gm_command` */

CREATE TABLE `gm_command` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` int(11) NOT NULL DEFAULT '0',
  `gmlevel` int(11) NOT NULL DEFAULT '0',
  `time` int(11) NOT NULL DEFAULT '0',
  `map` int(11) NOT NULL DEFAULT '0',
  `selection` int(11) NOT NULL DEFAULT '0',
  `command` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=223354 DEFAULT CHARSET=utf8;

/*Table structure for table `mon_classes` */

CREATE TABLE `mon_classes` (
  `time` int(10) unsigned NOT NULL,
  `class` tinyint(3) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_maps` */

CREATE TABLE `mon_maps` (
  `time` int(10) unsigned NOT NULL,
  `map` mediumint(8) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_players` */

CREATE TABLE `mon_players` (
  `time` int(10) unsigned NOT NULL,
  `active` int(10) unsigned NOT NULL,
  `queued` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_races` */

CREATE TABLE `mon_races` (
  `time` int(10) unsigned NOT NULL,
  `race` tinyint(3) unsigned NOT NULL,
  `players` int(10) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `mon_timediff` */

CREATE TABLE `mon_timediff` (
  `time` int(10) unsigned NOT NULL,
  `diff` mediumint(8) unsigned NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `sanctions` */

CREATE TABLE `sanctions` (
  `acctid` bigint(11) NOT NULL DEFAULT '0' COMMENT 'Account ID',
  `author` bigint(11) NOT NULL DEFAULT '0',
  `type` tinyint(3) NOT NULL DEFAULT '0' COMMENT '0 = mute, 1 = ingame ban, 2 = ip ban, 3 = forum ban',
  `duration` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Mute duration',
  `time` bigint(11) NOT NULL DEFAULT '0' COMMENT 'Current time',
  `reason` varchar(255) NOT NULL DEFAULT 'Unspecified'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `warden_fails` */

CREATE TABLE `warden_fails` (
  `guid` int(11) unsigned NOT NULL,
  `account` int(11) unsigned NOT NULL,
  `check_id` int(4) unsigned NOT NULL,
  `comment` text NOT NULL,
  `time` bigint(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `arena_2v2` */

DROP TABLE IF EXISTS `arena_2v2`;

/*!50001 CREATE TABLE  `arena_2v2`(
 `id` mediumint(8) unsigned ,
 `type` tinyint(3) unsigned ,
 `team1` mediumint(8) unsigned ,
 `team1_members` varchar(177) ,
 `team2` mediumint(8) unsigned ,
 `team2_members` varchar(177) ,
 `start` bigint(20) ,
 `duration` bigint(21) ,
 `winner` mediumint(8) unsigned ,
 `rating_change` tinyint(3) unsigned ,
 `winner_rating` mediumint(8) unsigned ,
 `loser_rating` mediumint(8) unsigned 
)*/;

/*View structure for view arena_2v2 */

/*!50001 DROP TABLE IF EXISTS `arena_2v2` */;
/*!50001 CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `arena_2v2` AS select `arena_match`.`id` AS `id`,`arena_match`.`type` AS `type`,`arena_match`.`team1` AS `team1`,concat(`arena_match`.`team1_member1`,' IP: ',`arena_match`.`team1_member1_ip`,' Heal: ',`arena_match`.`team1_member1_heal`,' Damage: ',`arena_match`.`team1_member1_damage`,' Kills: ',`arena_match`.`team1_member1_kills`,'\n',`arena_match`.`team1_member2`,' IP: ',`arena_match`.`team1_member2_ip`,' Heal: ',`arena_match`.`team1_member2_heal`,' Damage: ',`arena_match`.`team1_member2_damage`,' Kills: ',`arena_match`.`team1_member2_kills`) AS `team1_members`,`arena_match`.`team2` AS `team2`,concat(`arena_match`.`team2_member1`,' IP: ',`arena_match`.`team2_member1_ip`,' Heal: ',`arena_match`.`team2_member1_heal`,' Damage: ',`arena_match`.`team2_member1_damage`,' Kills: ',`arena_match`.`team2_member1_kills`,'\\',`arena_match`.`team2_member2`,' IP: ',`arena_match`.`team2_member2_ip`,' Heal: ',`arena_match`.`team2_member2_heal`,' Damage: ',`arena_match`.`team2_member2_damage`,' Kills: ',`arena_match`.`team2_member2_kills`) AS `team2_members`,`arena_match`.`start_time` AS `start`,(`arena_match`.`end_time` - `arena_match`.`start_time`) AS `duration`,`arena_match`.`winner` AS `winner`,`arena_match`.`rating_change` AS `rating_change`,`arena_match`.`winner_rating` AS `winner_rating`,`arena_match`.`loser_rating` AS `loser_rating` from `arena_match` */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
