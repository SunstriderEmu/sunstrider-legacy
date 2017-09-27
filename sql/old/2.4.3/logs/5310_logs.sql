DROP TABLE IF EXISTS `arena_team_event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `arena_team_event` (
  `id` int(10) unsigned NOT NULL,
  `event` tinyint(3) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `player` int(10) unsigned NOT NULL,
  `ip` varchar(16) NOT NULL DEFAULT '0.0.0.0',
  `time` bigint(20) unsigned NOT NULL,
  KEY `idx_arena_team_event_id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

