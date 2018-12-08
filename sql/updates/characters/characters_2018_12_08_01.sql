DROP TABLE IF EXISTS `character_queststatus_rewarded`;

CREATE TABLE `character_queststatus_rewarded` (
  `guid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier',
  `quest` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Quest Identifier',
  `active` tinyint(10) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`guid`,`quest`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Player System';

INSERT INTO character_queststatus_rewarded
	SELECT guid, quest, 1 FROM character_queststatus WHERE rewarded = 1;

ALTER TABLE character_queststatus
	DROP COLUMN rewarded;
	
ALTER TABLE character_queststatus
	ADD COLUMN `playercount` smallint(5) unsigned NOT NULL DEFAULT 0;
