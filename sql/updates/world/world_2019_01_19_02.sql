ALTER TABLE `areatrigger_involvedrelation` DROP FOREIGN KEY `areatrigger_involvedrelation_ibfk_2`;

ALTER TABLE `areatrigger_involvedrelation` ADD CONSTRAINT `areatrigger_involvedrelation_ibfk_2` FOREIGN KEY (`quest`) REFERENCES `quest_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `battlemaster_entry` DROP FOREIGN KEY `battlemaster_entry_ibfk_2`;

ALTER TABLE `battlemaster_entry` ADD CONSTRAINT `battlemaster_entry_ibfk_2` FOREIGN KEY (`entry`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `battlemaster_entry` DROP FOREIGN KEY `battlemaster_entry_ibfk_3`;

ALTER TABLE `battlemaster_entry` ADD CONSTRAINT `battlemaster_entry_ibfk_3` FOREIGN KEY (`bg_template`) REFERENCES `battleground_template`(`id`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature` DROP FOREIGN KEY `creature_ibfk_1`;

ALTER TABLE `creature` ADD CONSTRAINT `creature_ibfk_1` FOREIGN KEY (`spawnID`) REFERENCES `creature_entry`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature` DROP FOREIGN KEY `creature_ibfk_2`;

ALTER TABLE `creature` ADD CONSTRAINT `creature_ibfk_2` FOREIGN KEY (`modelid`) REFERENCES `creature_model_info`(`modelid`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_addon` DROP FOREIGN KEY `creature_addon_ibfk_1`;

ALTER TABLE `creature_addon` ADD CONSTRAINT `creature_addon_ibfk_1` FOREIGN KEY (`spawnID`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_addon` DROP FOREIGN KEY `creature_addon_ibfk_2`;

ALTER TABLE `creature_addon` ADD CONSTRAINT `creature_addon_ibfk_2` FOREIGN KEY (`path_id`) REFERENCES `waypoint_data`(`id`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_battleground` DROP FOREIGN KEY `creature_battleground_ibfk_1`;

ALTER TABLE `creature_battleground` ADD CONSTRAINT `creature_battleground_ibfk_1` FOREIGN KEY (`spawnID`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_default_trainer` DROP FOREIGN KEY `creature_default_trainer_ibfk_1`;

ALTER TABLE `creature_default_trainer` ADD CONSTRAINT `creature_default_trainer_ibfk_1` FOREIGN KEY (`CreatureId`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_entry` DROP FOREIGN KEY `creature_entry_ibfk_1`;

ALTER TABLE `creature_entry` ADD CONSTRAINT `creature_entry_ibfk_1` FOREIGN KEY (`entry`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_equip_template` DROP FOREIGN KEY `creature_equip_template_ibfk_1`;

ALTER TABLE `creature_equip_template` ADD CONSTRAINT `creature_equip_template_ibfk_1` FOREIGN KEY (`creatureID`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_formations` DROP FOREIGN KEY `creature_formations_ibfk_1`;

ALTER TABLE `creature_formations` ADD CONSTRAINT `creature_formations_ibfk_1` FOREIGN KEY (`leaderGUID`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_formations` DROP FOREIGN KEY `creature_formations_ibfk_3`;

ALTER TABLE `creature_formations` ADD CONSTRAINT `creature_formations_ibfk_3` FOREIGN KEY (`memberGUID`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_movement_override` DROP FOREIGN KEY `creature_movement_override_ibfk_1`;

ALTER TABLE `creature_movement_override` ADD CONSTRAINT `creature_movement_override_ibfk_1` FOREIGN KEY (`SpawnId`) REFERENCES `creature`(`spawnID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_onkill_reputation` DROP FOREIGN KEY `creature_onkill_reputation_ibfk_1`;

ALTER TABLE `creature_onkill_reputation` ADD CONSTRAINT `creature_onkill_reputation_ibfk_1` FOREIGN KEY (`creature_id`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_questender` DROP FOREIGN KEY `creature_questender_ibfk_4`;

ALTER TABLE `creature_questender` ADD CONSTRAINT `creature_questender_ibfk_4` FOREIGN KEY (`id`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_questender` DROP FOREIGN KEY `creature_questender_ibfk_5`;

ALTER TABLE `creature_questender` ADD CONSTRAINT `creature_questender_ibfk_5` FOREIGN KEY (`quest`) REFERENCES `quest_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_queststarter` DROP FOREIGN KEY `creature_queststarter_ibfk_3`;

ALTER TABLE `creature_queststarter` ADD CONSTRAINT `creature_queststarter_ibfk_3` FOREIGN KEY (`id`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_queststarter` DROP FOREIGN KEY `creature_queststarter_ibfk_4`;

ALTER TABLE `creature_queststarter` ADD CONSTRAINT `creature_queststarter_ibfk_4` FOREIGN KEY (`quest`) REFERENCES `quest_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template_addon` DROP FOREIGN KEY `creature_template_addon_ibfk_2`;

ALTER TABLE `creature_template_addon` ADD CONSTRAINT `creature_template_addon_ibfk_2` FOREIGN KEY (`entry`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template_addon` DROP FOREIGN KEY `creature_template_addon_ibfk_3`;

ALTER TABLE `creature_template_addon` ADD CONSTRAINT `creature_template_addon_ibfk_3` FOREIGN KEY (`path_id`) REFERENCES `waypoint_data`(`id`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template_movement` DROP FOREIGN KEY `creature_template_movement_ibfk_1`;

ALTER TABLE `creature_template_movement` ADD CONSTRAINT `creature_template_movement_ibfk_1` FOREIGN KEY (`CreatureId`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_text` DROP FOREIGN KEY `creature_text_ibfk_2`;

ALTER TABLE `creature_text` ADD CONSTRAINT `creature_text_ibfk_2` FOREIGN KEY (`CreatureID`) REFERENCES `creature_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

