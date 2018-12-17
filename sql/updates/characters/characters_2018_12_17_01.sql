ALTER TABLE `pet_spell`   
  CHANGE `spell` `spell` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL  COMMENT 'Spell Identifier',
  CHANGE `active` `active` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL,
  DROP COLUMN `slot`;
