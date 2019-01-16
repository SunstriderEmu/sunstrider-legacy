ALTER TABLE `pet_levelstats`   
  ADD COLUMN `patch` TINYINT(0) UNSIGNED DEFAULT 0  NOT NULL AFTER `creature_entry`, 
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`creature_entry`, `patch`, `level`);
