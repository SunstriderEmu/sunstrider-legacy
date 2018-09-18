ALTER TABLE `creature_entry`   
  CHANGE `equipment_id` `equipment_id` TINYINT(1) DEFAULT -1  NOT NULL  COMMENT '-1: random, 0: no equipment, >0 equipment id';
