DROP TABLE IF EXISTS `character_aura`;
CREATE TABLE `character_aura` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier',
  `casterGuid` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'Full Global Unique Identifier',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `effectMask` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `recalculateMask` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `stackCount` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `amount0` int(11) NOT NULL DEFAULT 0,
  `amount1` int(11) NOT NULL DEFAULT 0,
  `amount2` int(11) NOT NULL DEFAULT 0,
  `base_amount0` int(11) NOT NULL DEFAULT 0,
  `base_amount1` int(11) NOT NULL DEFAULT 0,
  `base_amount2` int(11) NOT NULL DEFAULT 0,
  `maxDuration` int(11) NOT NULL DEFAULT 0,
  `remainTime` int(11) NOT NULL DEFAULT 0,
  `remainCharges` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `critChance` float NOT NULL DEFAULT 0,
  `applyResilience` tinyint(3) NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`casterGuid`,`spell`,`effectMask`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Player System';

DROP TABLE IF EXISTS `pet_aura`;
CREATE TABLE `pet_aura` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier',
  `casterGuid` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'Full Global Unique Identifier',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `effectMask` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `recalculateMask` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `stackCount` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `amount0` mediumint(8) NOT NULL,
  `amount1` mediumint(8) NOT NULL,
  `amount2` mediumint(8) NOT NULL,
  `base_amount0` mediumint(8) NOT NULL,
  `base_amount1` mediumint(8) NOT NULL,
  `base_amount2` mediumint(8) NOT NULL,
  `maxDuration` int(11) NOT NULL DEFAULT 0,
  `remainTime` int(11) NOT NULL DEFAULT 0,
  `remainCharges` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `critChance` float NOT NULL DEFAULT 0,
  `applyResilience` tinyint(3) NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`casterGuid`,`spell`,`effectMask`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Pet System';

