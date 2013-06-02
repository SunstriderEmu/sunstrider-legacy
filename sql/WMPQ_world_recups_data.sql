-- MySQL dump 10.11
--
-- Host: localhost    Database: world2
-- ------------------------------------------------------
-- Server version	5.0.70-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES latin1 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `recups_data`
--

DROP TABLE IF EXISTS `recups_data`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `recups_data` (
  `classe` int(11) NOT NULL default '0',
  `faction` int(11) NOT NULL default '0',
  `phase` int(11) NOT NULL default '0',
  `command` text,
  `metier` int(11) NOT NULL,
  `stuff` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `recups_data`
--

LOCK TABLES `recups_data` WRITE;
/*!40000 ALTER TABLE `recups_data` DISABLE KEYS */;
INSERT INTO `recups_data` VALUES (1,1,2,'additemset 590;additem 32508 1;additem 31255 1;additem 31284 1;additem 31151 1;additem 31581 1;additem 24087 1;additem 27416 1;additem 31291 1',0,0),(1,2,2,'additemset 588;additem 32508 1;additem 31255 1;additem 31284 1;additem 31151 1;additem 31581 1;additem 24087 1;additem 27416 1;additem 31291 1',0,0),(2,1,2,'additemset 607;additem 32508 1;additem 31255 1;additem 31284 1;additem 31151 1;additem 31276 1;additem 24087 1;additem 27416 1;additem 31291 1',0,0),(2,2,2,'additemset 608;additem 32508 1;additem 31255 1;additem 31284 1;additem 31151 1;additem 31276 1;additem 24087 1;additem 27416 1;additem 31291 1',0,0),(3,1,2,'additemset 595;additem 32508 1;additem 31255 1;additem 24451 1;additem 27646 1;additem 25941 1;additem 28246 1;additem 27416 1;additem 28946 2;additem 31303 1',0,0),(3,2,2,'additemset 596;additem 32508 1;additem 31255 1;additem 24451 1;additem 27646 1;additem 25941 1;additem 28246 1;additem 27416 1;additem 28921 2;additem 31303 1',0,0),(4,1,2,'additemset 605;additem 32508 1;additem 31255 1;additem 31175 1;additem 27637 1;additem 25946 1;additem 24087 1;additem 27416 1;additem 28955 2',0,0),(4,2,2,'additemset 606;additem 32508 1;additem 31255 1;additem 31175 1;additem 27637 1;additem 25946 1;additem 24087 1;additem 27416 1;additem 28930 2',0,0),(5,1,2,'additemset 597;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 28247 1;additem 26055 1;additem 31308 1;additem 25295 1',0,0),(5,2,2,'additemset 598;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 28247 1;additem 26055 1;additem 31308 1;additem 25295 1',0,0),(7,1,2,'additemset 593;additem 25562 1;additem 31255 1;additem 24451 1;additem 27521 1;additem 30953 1;additem 31729 1;additem 27416 1;additem 28953 1;additem 28947 1',0,0),(7,2,2,'additemset 594;additem 25562 1;additem 31255 1;additem 24451 1;additem 27521 1;additem 30953 1;additem 31729 1;additem 27416 1;additem 28928 1;additem 28922 1',0,0),(8,1,2,'additemset 599;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 27780 1;additem 26055 1;additem 31308 1;additem 25939 1',0,0),(8,2,2,'additemset 600;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 27780 1;additem 26055 1;additem 31308 1;additem 25939 1',0,0),(9,1,2,'additemset 591;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 28247 1;additem 26055 1;additem 31308 1;additem 25939 1',0,0),(9,2,2,'additemset 592;additem 31692 1;additem 24252 1;additem 27517 1;additem 27638 1;additem 31230 1;additem 28247 1;additem 26055 1;additem 31308 1;additem 25939 1',0,0),(11,1,2,'additemset 601;additem 25562 1;additem 28032 1;additem 30944 1;additem 27645 1;additem 25946 1;additem 30860 1;additem 25937 1;additem 25761 1',0,0),(11,2,2,'additemset 602;additem 25562 1;additem 28032 1;additem 30944 1;additem 27645 1;additem 25946 1;additem 30860 1;additem 25937 1;additem 25761 1',0,0),(3,0,1,'additem 21841 3;additem 11362 1;additem 28056 600',0,0),(0,0,1,'additem 21841 4',0,0),(0,0,2,'money 30000000;learn 34090',0,0),(0,0,2,'learn 2259;setskill 171',1,0),(0,0,2,'learn 3908;setskill 197',2,0),(0,0,2,'learn 2366;setskill 182',3,0),(0,0,2,'learn 8613;setskill 393',4,0),(0,0,2,'learn 7411;setskill 333',5,0),(0,0,2,'learn 2018;setskill 164',6,0),(0,0,2,'learn 4036;setskill 202',7,0),(0,0,2,'learn 25229;setskill 755',8,0),(0,0,2,'learn 2575;setskill 186',9,0),(0,0,2,'learn 2108;setskill 165',10,0),(5,1,2,'additemset 691;additem 31691 1;additem 25805 1;additem 31531 1;additem 30463 1;additem 31312 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(5,2,2,'additemset 692;additem 31691 1;additem 25805 1;additem 31531 1;additem 30463 1;additem 31312 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(11,1,2,'additemset 688;additem 31691 1;additem 25805 1;additem 29955 1;additem 27645 1;additem 31312 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(11,2,2,'additemset 689;additem 31691 1;additem 25805 1;additem 29955 1;additem 27645 1;additem 31312 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(2,1,2,'additemset 693;additem 31691 1;additem 25805 1;additem 25623 1;additem 31202 1;additem 31276 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(2,2,2,'additemset 694;additem 31691 1;additem 25805 1;additem 25623 1;additem 31202 1;additem 31276 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(7,1,2,'additemset 695;additem 31691 1;additem 25805 1;additem 25592 1;additem 27643 1;additem 29313 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(7,2,2,'additemset 696;additem 31691 1;additem 25805 1;additem 25592 1;additem 27643 1;additem 29313 1;additem 29814 1;additem 25634 1;additem 31304 1',0,1),(1,1,2,'additem 30016 1;additem 31696 1;additem 29316 1;additem 29777 1;additem 25819 1;additem 30225 1;additem 30264 1;additem 30371 1;additem 31226 1;additem 29325 1;additem 27436 1;additem 30300 1;additem 31234 1;additem 31200 1',0,2),(1,2,2,'additem 30016 1;additem 31696 1;additem 29316 1;additem 29777 1;additem 25819 1;additem 30225 1;additem 30264 1;additem 30371 1;additem 31226 1;additem 29325 1;additem 27436 1;additem 30300 1;additem 31234 1;additem 31200 1',0,2),(1,0,3,'learn 5011;learn 264;learn 266;learn 200;learn 2567;learn 15590;learn 5009;learn 227;learn 1180;learn 202;learn 201;learn 197;learn 2764;learn 198;learn 3018;learn 750;learn 20252;learn 2458;learn 355;learn 7386;learn 71',0,0),(2,0,3,'learn 750;learn 196;learn 197;learn 198;learn 199;learn 200;learn 201;learn 202;learn 7328',0,0),(3,0,3,'learn 8737;learn 5011;learn 15590;learn 200;learn 201;learn 2567;learn 196;learn 227;learn 197;learn 202;learn 264;learn 266;learn 1180;learn 674;learn 5149;learn 982;learn 6991;learn 23357;learn 2641;learn 23356;learn 1515;learn 883',0,0),(4,0,3,'learn 201;learn 264;learn 5011;learn 198;learn 266;learn 674;learn 1180;learn 15590;learn 2567;learn 1804;learn 2842',0,0),(5,0,3,'learn 227;learn 1180;learn 198',0,0),(7,0,3,'learn 8737;learn 196;learn 1180;learn 15590;learn 674;learn 198;learn 199;learn 227;learn 197;learn 8071;learn 5394;learn 3599',0,0),(8,0,3,'learn 1180;learn 201',0,0),(9,0,3,'learn 227;learn 201;learn 1180;learn 1122;learn 23161;learn 691;learn 712;learn 697',0,0),(11,0,3,'learn 1180;learn 198;learn 227;learn 199;learn 15590;learn 5487;learn 6795;learn 6807;learn 8946;learn 1066',0,0),(2,1,2,'additem 30016 1;additem 31696 1;additem 29316 1;additem 29777 1;additem 30296 1;additem 30225 1;additem 29959 1;additem 30371 1;additem 31226 1;additem 29325 1;additem 31078 1;additem 30300 1;additem 31234 1;additem 31200 1',0,2),(2,2,2,'additem 30016 1;additem 31696 1;additem 29316 1;additem 29777 1;additem 30296 1;additem 30225 1;additem 29959 1;additem 30371 1;additem 31226 1;additem 29325 1;additem 31078 1;additem 30300 1;additem 31234 1;additem 31200 1',0,2),(0,0,2,'learn 3273;setskill 129',11,0);
/*!40000 ALTER TABLE `recups_data` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-11-24 19:34:29
