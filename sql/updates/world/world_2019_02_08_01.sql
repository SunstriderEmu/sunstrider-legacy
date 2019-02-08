ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_1`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_1` FOREIGN KEY (`lootid`) REFERENCES `creature_loot_template`(`Entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_10`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_10` FOREIGN KEY (`spell6`) REFERENCES `spell_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_11`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_11` FOREIGN KEY (`spell7`) REFERENCES `spell_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_12`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_12` FOREIGN KEY (`spell8`) REFERENCES `spell_template`(`entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_13`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_13` FOREIGN KEY (`modelid1`) REFERENCES `creature_model_info`(`modelid`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_14`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_14` FOREIGN KEY (`modelid2`) REFERENCES `creature_model_info`(`modelid`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_15`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_15` FOREIGN KEY (`modelid3`) REFERENCES `creature_model_info`(`modelid`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_16`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_16` FOREIGN KEY (`modelid4`) REFERENCES `creature_model_info`(`modelid`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_2`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_2` FOREIGN KEY (`pickpocketloot`) REFERENCES `pickpocketing_loot_template`(`Entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_3`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_3` FOREIGN KEY (`skinloot`) REFERENCES `skinning_loot_template`(`Entry`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_4`;

ALTER TABLE `creature_template` ADD CONSTRAINT `creature_template_ibfk_4` FOREIGN KEY (`gossip_menu_id`) REFERENCES `gossip_menu`(`MenuID`) ON UPDATE CASCADE ON DELETE RESTRICT;

ALTER TABLE `creature_template` DROP FOREIGN KEY `creature_template_ibfk_5`;

Alter table `creature_template` add constraint `creature_template_ibfk_5` foreign key (`spell1`) references `spell_template`(`entry`) on update Cascade on delete RESTRICT;

Alter table `creature_template` drop foreign key `creature_template_ibfk_6`;

Alter table `creature_template` add constraint `creature_template_ibfk_6` foreign key (`spell2`) references `spell_template`(`entry`) on update Cascade on delete RESTRICT;

Alter table `creature_template` drop foreign key `creature_template_ibfk_7`;

Alter table `creature_template` add constraint `creature_template_ibfk_7` foreign key (`spell3`) references `spell_template`(`entry`) on update Cascade on delete RESTRICT;

Alter table `creature_template` drop foreign key `creature_template_ibfk_8`;

Alter table `creature_template` add constraint `creature_template_ibfk_8` foreign key (`spell4`) references `spell_template`(`entry`) on update Cascade on delete RESTRICT;

Alter table `creature_template` drop foreign key `creature_template_ibfk_9`;

Alter table `creature_template` add constraint `creature_template_ibfk_9` foreign key (`spell5`) references `spell_template`(`entry`) on update Cascade on delete RESTRICT;
