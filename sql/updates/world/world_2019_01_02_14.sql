DELETE FROM creature_loot_template WHERE Entry IN (832, 1494, 1768, 1908, 2120, 3251, 3252, 3454, 3455, 4054, 4377, 4510, 4661, 5440, 5760, 6196, 6388, 6492, 7051, 7885, 7886, 7915, 8581, 8608, 9118, 9176, 10680, 10681, 10987, 11600, 11602, 11701, 13256, 13419, 13534, 13535, 13536, 13537, 13538, 13539, 13540, 13541, 13542, 13543, 13544, 13545, 13547, 13548, 13549, 13550, 13551, 13552, 13553, 13554, 13555, 13556, 13557, 13776, 13816, 13841, 14285, 14763, 14770, 14771, 14772, 14774, 14775, 14777, 15449, 16156, 19208, 21425, 21627, 24179, 24180);

DELETE FROM reference_loot_template WHERE Entry IN (10006, 11001, 11002, 11003, 11004, 11005, 11006, 11007, 11008, 11009, 11010, 11011, 11012, 11013, 11014, 11015, 11016, 11017, 11099, 11100, 12018, 25003, 89040, 187892);

UPDATE creature SET spawndist = 5 WHERE spawnID IN (58902, 58903, 59464, 67188, 67189, 67190, 67191, 67192, 67194, 67195, 67196, 67197, 67199, 67200, 67202, 67203, 67204, 67205, 67206, 67207, 67208, 67213, 67215, 67216, 67218, 67220, 67224);

UPDATE creature SET MovementType = 0, spawndist = 0 WHERE spawnID = 57912;
UPDATE creature SET MovementType = 0, spawndist = 0 WHERE spawnID = 57913;
UPDATE creature SET MovementType = 0, spawndist = 0 WHERE spawnID = 57914;
UPDATE creature SET MovementType = 0, spawndist = 0 WHERE spawnID = 58429;
UPDATE creature SET MovementType = 0, spawndist = 0 WHERE spawnID = 58435;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87977;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87978;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87979;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87980;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87981;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87982;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87983;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87984;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87985;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87986;
UPDATE creature SET MovementType = 1, spawndist = 5 WHERE spawnID = 87987;
UPDATE creature SET MovementType = 1, spawndist = 15 WHERE spawnID = 87988;

replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('76058','0',NULL,'0','0','0','1','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('76114','0','218010','0','0','33554432','1','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88163','0',NULL,'0','0','0','257','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88474','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88475','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88476','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88477','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88478','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88479','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88480','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88481','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88482','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88483','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88484','0',NULL,'0','0','1','0','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('91694','0',NULL,'0','0','0','1','0','0',NULL);

replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('76058','5',NULL,'0','0','0','1','0','0','43570');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('76114','5','218010','0','0','33554432','1','0','0','55971');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88163','5',NULL,'29102','0','0','257','0','0',NULL);
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88474','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88475','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88476','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88477','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88478','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88479','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88480','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88481','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88482','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88483','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('88484','5',NULL,'0','0','1','0','0','0','45319');
replace into `creature_addon` (`spawnID`, `patch`, `path_id`, `mount`, `bytes0`, `bytes1`, `bytes2`, `emote`, `moveflags`, `auras`) values('91694','5',NULL,'0','0','0','1','0','0','66515');
