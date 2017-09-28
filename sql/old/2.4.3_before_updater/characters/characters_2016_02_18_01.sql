
DROP TABLE IF EXISTS `creature_respawn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `creature_respawn` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `respawnTime` int(10) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(10) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`),
  KEY `idx_instance` (`instanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Grid Loading System';
/*!40101 SET character_set_client = @saved_cs_client */;


DROP TABLE IF EXISTS `gameobject_respawn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `gameobject_respawn` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `respawnTime` int(10) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(10) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`),
  KEY `idx_instance` (`instanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Grid Loading System';
/*!40101 SET character_set_client = @saved_cs_client */;

ALTER TABLE `corpse`   
  CHANGE `instance` `instanceId` INT(11) UNSIGNED DEFAULT 0  NOT NULL;
  
ALTER TABLE `instance_reset`   
  CHANGE `mapid` `mapid` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `difficulty` `difficulty` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL  AFTER `mapid`,
  CHANGE `resettime` `resettime` INT(11) UNSIGNED DEFAULT 0  NOT NULL, 
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`mapid`, `difficulty`);