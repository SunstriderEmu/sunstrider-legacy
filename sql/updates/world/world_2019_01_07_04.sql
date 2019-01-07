ALTER TABLE `spell_group_stack_rules`  
  DROP FOREIGN KEY `spell_group_stack_rules_ibfk_1`;

REPLACE INTO spell_group_stack_rules VALUES (1147, 1);
UPDATE spell_group SET id = 1147 WHERE id = 3;

REPLACE INTO spell_group_stack_rules VALUES (1148, 1);
UPDATE spell_group SET id = 1148 WHERE id = 4;

REPLACE INTO spell_group_stack_rules VALUES (1021, 4);
REPLACE INTO spell_group_stack_rules VALUES (1027, 4);
REPLACE INTO spell_group_stack_rules VALUES (1064, 4);
DELETE FROM spell_group WHERE id IN (1023, 1024, 1030, 1028, 1031, 1034, 1037, 1050, 1052, 1057, 1063, 1066, 1067, 1068, 1069, 1070, 1071, 1075, 1078, 1079, 1091, 1092);
REPLACE INTO spell_group_stack_rules VALUES (1102, 1);
REPLACE INTO spell_group_stack_rules VALUES (1103, 1);

ALTER TABLE `spell_group`  
  ADD CONSTRAINT `fk_group` FOREIGN KEY (`id`) REFERENCES `spell_group_stack_rules`(`group_id`) ON UPDATE CASCADE ON DELETE CASCADE;
