ALTER TABLE `addons`   
  ADD COLUMN `gamebuild` INT(10) UNSIGNED DEFAULT 0  NOT NULL AFTER `name`, 
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`name`, `gamebuild`);

