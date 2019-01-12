ALTER TABLE `creature`   
  CHANGE `spawntimesecs` `spawntimesecs` INT(10) UNSIGNED DEFAULT 300  NOT NULL,
  ADD COLUMN `spawntimesecs_max` INT(10) UNSIGNED DEFAULT 0  NOT NULL  COMMENT 'If specified, rand between spawntimesecs and spawntimesecs_max' AFTER `spawntimesecs`;
