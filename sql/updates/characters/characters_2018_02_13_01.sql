
DROP TABLE IF EXISTS `character_spell_cooldown`;
CREATE TABLE `character_spell_cooldown` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier, Low part',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT 0 COMMENT 'Spell Identifier',
  `item` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Item Identifier',
  `time` int(10) unsigned NOT NULL DEFAULT 0,
  `categoryId` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Spell category Id',
  `categoryEnd` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `pet_spell_cooldown`;
CREATE TABLE `pet_spell_cooldown` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier, Low part',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT 0 COMMENT 'Spell Identifier',
  `time` int(10) unsigned NOT NULL DEFAULT 0,
  `categoryId` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Spell category Id',
  `categoryEnd` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
