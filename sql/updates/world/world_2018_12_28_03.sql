
/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `pool_quest` */

DROP TABLE IF EXISTS `pool_quest`;

CREATE TABLE `pool_quest` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `pool_entry` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `description` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`entry`),
  KEY `idx_guid` (`entry`),
  KEY `fk_pooltemplate` (`pool_entry`),
  CONSTRAINT `fk_pooltemplate` FOREIGN KEY (`pool_entry`) REFERENCES `pool_template` (`entry`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_quest` FOREIGN KEY (`entry`) REFERENCES `quest_template` (`entry`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('352','1','The Rokk <Master of Cooking> - Daily Quests');
insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('353','1','Old Man Barlowned - Daily Quests');
insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('356','1','Wind Trader Zhareem - Daily Quests');
insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('357','1','Nether-Stalker Mah\'duun - Daily Quests');
insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('700','1','BG Daily Quests - Alliance');
insert ignore into `pool_template` (`entry`, `max_limit`, `description`) values('701','1','BG Daily Quests - Horde');

/*Data for the table `pool_quest` */

insert ignore into `pool_quest`(`entry`,`pool_entry`,`description`) values 
(11354,356,'Wanted: Nazan\'s Riding Crop'),
(11362,356,'Wanted: Keli\'dan\'s Feathered Stave'),
(11363,356,'Wanted: Bladefist\'s Seal'),
(11364,357,'Wanted: Shattered Hand Centurions'),
(11368,356,'Wanted: The Heart of Quagmirran'),
(11369,356,'Wanted: A Black Stalker Egg'),
(11370,356,'Wanted: The Warlord\'s Treatise'),
(11371,357,'Wanted: Coilfang Myrmidons'),
(11372,356,'Wanted: The Headfeathers of Ikiss'),
(11373,356,'Wanted: Shaffar\'s Wondrous Pendant'),
(11374,356,'Wanted: The Exarch\'s Soul Gem'),
(11375,356,'Wanted: Murmur\'s Whisper'),
(11376,357,'Wanted: Malicious Instructors'),
(11377,352,'Revenge is Tasty'),
(11378,356,'Wanted: The Epoch Hunter\'s Head'),
(11379,352,'Super Hot Stew'),
(11380,352,'Manalicious'),
(11381,352,'Soup for the Soul'),
(11382,356,'Wanted: Aeonus\'s Hourglass'),
(11383,357,'Wanted: Rift Lords'),
(11384,356,'Wanted: A Warp Splinter Clipping'),
(11385,357,'Wanted: Sunseeker Channelers'),
(11386,356,'Wanted: Pathaleon\'s Projector'),
(11387,357,'Wanted: Tempest-Forge Destroyers'),
(11388,356,'Wanted: The Scroll of Skyriss'),
(11389,357,'Wanted: Arcatraz Sentinels'),
(11499,356,'Wanted: The Signet Ring of Prince Kael\'thas'),
(11500,357,'Wanted: Sisters of Torment'),
(11665,353,'Crocolisks in the City'),
(11666,353,'Bait Bandits'),
(11667,353,'The One That Got Away'),
(11668,353,'Shrimpin Aint Easy'),
(11669,353,'Felblood Fillet'),

(11339,701,'Call to Arms: Arathi Basin'),

(11340,701,'Call to Arms: Alterac Valley'),

(11341,701,'Call to Arms: Eye of the Storm'),

(11342,701,'Call to Arms: Warsong Gulch'),


(11335,700,'Call to Arms: Arathi Basin'),

(11336,700,'Call to Arms: Alterac Valley'),

(11337,700,'Call to Arms: Eye of the Storm'),

(11338,700,'Call to Arms: Warsong Gulch');


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

DROP TABLE IF EXISTS quest_pool;
DROP TABLE IF EXISTS quest_pool_current;

DELETE FROM command WHERE name = "heroday";
