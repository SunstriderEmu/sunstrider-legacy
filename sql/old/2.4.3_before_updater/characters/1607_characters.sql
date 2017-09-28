ALTER TABLE `characters` DROP COLUMN `bgid`;
ALTER TABLE `characters` DROP COLUMN `bgteam`;
ALTER TABLE `characters` DROP COLUMN `bgmap`;
ALTER TABLE `characters` DROP COLUMN `bgx`;
ALTER TABLE `characters` DROP COLUMN `bgy`;
ALTER TABLE `characters` DROP COLUMN `bgz`;
ALTER TABLE `characters` DROP COLUMN `bgo`;

DROP TABLE IF EXISTS `character_bgcoord`;
CREATE TABLE `character_bgcoord` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `bgid` int(10) unsigned NOT NULL DEFAULT '0',
  `bgteam` int(10) unsigned NOT NULL DEFAULT '0',
  `bgmap` int(10) unsigned NOT NULL DEFAULT '0',
  `bgx` float NOT NULL DEFAULT '0',
  `bgy` float NOT NULL DEFAULT '0',
  `bgz` float NOT NULL DEFAULT '0',
  `bgo` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Saving char in bg System';