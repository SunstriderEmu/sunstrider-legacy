
insert ignore into `creature_template` (`entry`, `name`) values('34564','TLK Dummy - Anub\'arak');
insert ignore into `creature_template` (`entry`, `name`) values('36296','TLK Dummy - Apothecary Hummel');
insert ignore into `creature_template` (`entry`, `name`) values('36530','TLK Dummy - [DND] Valentine Boss - Vial Bunny');
insert ignore into `creature_template` (`entry`, `name`) values('36885','TLK Dummy - Crown Apothecary');
insert ignore into `creature_template` (`entry`, `name`) values('38208','TLK Dummy - Investigator Fezzen Brasstacks');

ALTER TABLE `smart_scripts`   
	ADD COLUMN IF NOT EXISTS `patch_min` TINYINT(2) UNSIGNED DEFAULT 0  NOT NULL AFTER target_o,
    ADD COLUMN IF NOT EXISTS `patch_max` TINYINT(2) UNSIGNED DEFAULT 10  NOT NULL AFTER patch_min;

UPDATE smart_scripts ss LEFT JOIN spell_template st ON st.entry = ss.action_param1 SET ss.patch_min = 5 WHERE ss.action_type = 11 AND st.entry IS NULL;

UPDATE smart_scripts SET event_param1 = 4 WHERE source_type = 1 AND event_type = 104 AND event_param1 = 8;