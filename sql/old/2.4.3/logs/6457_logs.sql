/*
SQLyog Community v9.63 
MySQL - 5.5.34-MariaDB : Database - wrlogs
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `item_delete` */

DROP TABLE IF EXISTS `item_delete`;

CREATE TABLE `item_delete` (
  `playerguid` int(11) unsigned NOT NULL COMMENT 'player guid',
  `entry` mediumint(8) unsigned NOT NULL COMMENT 'item entry',
  `count` smallint(5) unsigned NOT NULL DEFAULT '1' COMMENT 'item count',
  `time` bigint(20) unsigned NOT NULL COMMENT 'delete time',
  PRIMARY KEY (`playerguid`,`entry`,`count`,`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `item_mail` (
  `senderguid` int(11) unsigned DEFAULT NULL,
  `receiverguid` int(11) unsigned NOT NULL,
  `itemguid` int(11) unsigned NOT NULL,
  `itementry` mediumint(8) unsigned NOT NULL,
  `itemcount` smallint(5) unsigned NOT NULL,
  `time` bigint(20) unsigned NOT NULL COMMENT 'send time'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
