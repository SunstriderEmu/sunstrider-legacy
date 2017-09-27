ALTER TABLE `character_pet`   
  CHANGE `level` `level` TINYINT(3) UNSIGNED DEFAULT 1  NOT NULL,
  CHANGE `slot` `slot` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `savetime` `savetime` INT(11) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `resettalents_time` `resettalents_time` INT(11) UNSIGNED DEFAULT 0  NOT NULL;

ALTER TABLE `pet_spell`   
  CHANGE `spell` `spell` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL   COMMENT 'Spell Identifier',
  CHANGE `active` `active` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL;
  