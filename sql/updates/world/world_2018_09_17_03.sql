START TRANSACTION;

DROP TABLE creature_entry;
CREATE TABLE `creature_entry`(  
  `spawnID` INT(10) UNSIGNED NOT NULL,
  `entry` MEDIUMINT(8) UNSIGNED NOT NULL,
  `equipment_id` TINYINT(1) DEFAULT 0 NOT NULL,
  PRIMARY KEY (`spawnID`, `entry`)
);

INSERT INTO creature_entry SELECT guid, id, equipment_id FROM creature;

ALTER TABLE `creature`   
  DROP COLUMN `id`, 
  DROP INDEX `idx_id`,
  CHANGE `guid` `spawnID` INT(10) UNSIGNED NOT NULL  COMMENT '';
  
ALTER TABLE `creature_addon`   
  CHANGE `guid` `spawnID` INT(10) UNSIGNED DEFAULT 0  NOT NULL;

COMMIT;