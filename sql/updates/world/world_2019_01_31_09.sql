SET FOREIGN_KEY_CHECKS=0;

-- remove some invalid orphan entries
DELETE FROM creature_entry where spawnID IN (1012522, 1012523, 1012524, 1012525, 1095739);

ALTER TABLE `creature`  
  DROP FOREIGN KEY `creature_ibfk_1`;
  
ALTER TABLE `creature_entry`  
  ADD CONSTRAINT `creature_entry_ibfk_2` FOREIGN KEY (`spawnID`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE;

SET FOREIGN_KEY_CHECKS=1;
