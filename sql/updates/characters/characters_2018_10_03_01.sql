ALTER TABLE `characters`   
  CHANGE `zone` `zone` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `logout_time` `logout_time` INT(10) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `resettalents_time` `resettalents_time` INT(10) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `transguid` `transguid` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL;
