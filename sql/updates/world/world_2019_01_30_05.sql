ALTER TABLE `gameobject_queststarter`   
  ADD COLUMN `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL AFTER `quest`,
  ADD COLUMN `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL AFTER `patch_min`;

ALTER TABLE `gameobject_questender`   
  ADD COLUMN `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL AFTER `quest`,
  ADD COLUMN `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL AFTER `patch_min`;

ALTER TABLE `creature_questender`   
  ADD COLUMN `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL AFTER `quest`,
  ADD COLUMN `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL AFTER `patch_min`;
