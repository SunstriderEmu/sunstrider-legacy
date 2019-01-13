ALTER TABLE `waypoints`   
  DROP COLUMN `id`, 
  DROP INDEX `id`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`entry`, `pointid`);