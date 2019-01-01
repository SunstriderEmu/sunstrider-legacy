UPDATE creature SET patch_min = 5 WHERE map IN (571, 631);

DELETE FROM creature_addon WHERE spawnID IN (83999, 84002, 84003, 84004, 84005, 84006);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('83999','0','839990','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84002','0','840020','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84003','0','840030','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84004','0','840040','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84005','0','840050','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84006','0','840060','0','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('83999','5','839990','28417','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84002','5','840020','28417','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84003','5','840030','28417','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84004','5','840040','28417','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84005','5','840050','28417','0','0','1','0','0',NULL);
insert into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('84006','5','840060','28417','0','0','1','0','0',NULL);

UPDATE quest_template qt JOIN quest_template_addon qta ON qt.entry = qta.ID SET QuestFlags = QuestFlags - 1 WHERE QuestFlags & 1 AND NOT (SpecialFlags & 0x00001000);