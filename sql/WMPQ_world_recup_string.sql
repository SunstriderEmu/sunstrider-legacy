-- MySQL dump 10.11
--
-- Host: localhost    Database: silm_world
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
-- Table structure for table `trinity_string`
--

--
-- Dumping data for table `trinity_string`
--
-- WHERE:  entry in (11002,11004,11005,11006,11007,11008,11009,11010)

LOCK TABLES `trinity_string` WRITE;
/*!40000 ALTER TABLE `trinity_string` DISABLE KEYS */;
INSERT INTO `trinity_string` VALUES (11004,'You do not have any active recovery for this character.',NULL,'Vous n\'avez aucune recuperation active pour ce personnage.',NULL,NULL,NULL,NULL,NULL,NULL),(11005,'Recovery data does not match with character data.',NULL,'Les donnees de la recuperation ne correspondent pas aux donnees du personnage.',NULL,NULL,NULL,NULL,NULL,NULL),(11006,'Recovery data seems to be corrupted.',NULL,'Les donnees de la recuperation sont corrompues.',NULL,NULL,NULL,NULL,NULL,NULL),(11007,'The execution of a recovery command failed.',NULL,'L\'execution d\'une commande de recuperation a echoue.',NULL,NULL,NULL,NULL,NULL,NULL),(11008,'Phase one of recovery succeeded. Please retype the recovery command in order to complete the recovery.',NULL,'La premiere phase de la recuperation s\'est terminee avec succes. Veuillez retaper la commande de recuperation afin de la terminer.',NULL,NULL,NULL,NULL,NULL,NULL),(11009,'The execution of reputation recovery command failed.',NULL,'L\'execution de la commande de mise a niveau des reputations a echoue.',NULL,NULL,NULL,NULL,NULL,NULL),(11010,'Recovery completed successfully.',NULL,'La recuperation s\'est terminee avec succes.',NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `trinity_string` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-11-23 14:24:47
