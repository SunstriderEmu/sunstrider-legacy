-- MySQL dump 10.11
--
-- Host: localhost    Database: silm_characters
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
-- Table structure for table `recups`
--

DROP TABLE IF EXISTS `recups`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `recups` (
  `id` int(11) NOT NULL auto_increment,
  `account` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `classe` int(11) NOT NULL,
  `faction` int(11) NOT NULL,
  `metier1` int(11) NOT NULL,
  `metier1_level` int(11) NOT NULL,
  `metier2` int(11) NOT NULL,
  `metier2_level` int(11) NOT NULL,
  `reputs` text,
  `phase` int(11) NOT NULL default '1',
  `active` int(11) NOT NULL,
  `stuff` int(11) NOT NULL,
  `nickfofo` text NOT NULL,
  `screenshot1` text NOT NULL,
  `screenshot2` text NOT NULL,
  `screenshot3` text NOT NULL,
  `screenshot4` text NOT NULL,
  `metier3` int(11) NOT NULL,
  `metier3_level` int(11) NOT NULL,
  `comment` text NOT NULL,
  `origserv` text NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `recups`
--

LOCK TABLES `recups` WRITE;
/*!40000 ALTER TABLE `recups` DISABLE KEYS */;
/*!40000 ALTER TABLE `recups` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-11-23 13:56:07
