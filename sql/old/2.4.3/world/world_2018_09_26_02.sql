ALTER TABLE `creature_template`   
  ADD COLUMN  `speed_walk` float NOT NULL DEFAULT '1' COMMENT 'Result of 2.5/2.5, most common value' AFTER `speed`,
  ADD COLUMN  `speed_run` float NOT NULL DEFAULT '1.14286' COMMENT 'Result of 8.0/7.0, most common value' AFTER `speed_walk`;
  
UPDATE creature_template SET speed_run = speed;
ALTER TABLE `creature_template`   
  DROP COLUMN `speed`;