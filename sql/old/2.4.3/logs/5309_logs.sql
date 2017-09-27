DROP TABLE IF EXISTS `char_rename`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `char_rename` (
  `account` int(10) unsigned NOT NULL,
  `guid` int(10) unsigned NOT NULL,
  `old_name` varchar(12) NOT NULL,
  `new_name` varchar(12) NOT NULL,
  `time` bigint(20) NOT NULL,
  `ip` varchar(16) NOT NULL DEFAULT '0.0.0.0',
  KEY `idx_char_rename_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

