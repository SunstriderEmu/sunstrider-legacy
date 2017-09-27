ALTER TABLE `characters` ADD COLUMN `gender` TINYINT UNSIGNED NOT NULL default '0' AFTER `class`;
UPDATE characters SET gender = CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ',  37), ' ', -1) AS UNSIGNED) >> 16 & 0xff;
