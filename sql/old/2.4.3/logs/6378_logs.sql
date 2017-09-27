/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `arena_season_stats` */

DROP TABLE IF EXISTS `arena_season_stats`;

CREATE TABLE `arena_season_stats` (
  `teamid` int(10) unsigned NOT NULL DEFAULT '0',
  `time1` int(10) unsigned DEFAULT '0' COMMENT 'Time spent first in minutes',
  `time2` int(10) unsigned DEFAULT '0' COMMENT 'Time spent second in minutes',
  `time3` int(10) unsigned DEFAULT '0' COMMENT 'Time spent third in minutes',
  PRIMARY KEY (`teamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
