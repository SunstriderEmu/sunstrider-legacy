ALTER TABLE `pool_template`   
  ADD COLUMN `max_limit_percent` FLOAT DEFAULT 0  NOT NULL  COMMENT 'Max number of objects in percent, relative to object count in pool. (0) is no limit' AFTER `max_limit`;
