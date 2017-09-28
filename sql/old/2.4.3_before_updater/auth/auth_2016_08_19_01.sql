ALTER TABLE `account`   
  ADD COLUMN `build` SMALLINT(5) UNSIGNED NULL AFTER `os`;
  
ALTER TABLE `account`   
  CHANGE `sha_pass_hash` `password` VARCHAR(40) CHARSET utf8 COLLATE utf8_general_ci DEFAULT ''  NOT NULL;
