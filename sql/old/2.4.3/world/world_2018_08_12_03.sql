SET @GROUPID := 6;
SET @BOSS_STATE_ID := 4;
SET @MAP_ID := 550;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "The Eye - Solarian", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 1;
SET @MAP_ID := 550;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "The Eye - Al'ar", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 11;
SET @MAP_ID := 550;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "The Eye - Void Reaver", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 6;
SET @MAP_ID := 550;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "The Eye - Kael'Thas", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 3;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Gurtogg", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 5;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Naj'entus", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 7;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Illidan", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 8;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Illidari Council", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 14;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Mother Sharaz", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 15;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Reliquary of Souls", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 17;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Shade of Akama", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 19;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Supremus", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 20;
SET @MAP_ID := 564;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Black Temple - Teron Gorefiend", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
-- Fixed one wrong entry
DELETE FROM creature_encounter_respawn WHERE guid = 12825;
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) VALUES (@GROUPID, 0, 12825);
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 0;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Kalecgos", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 1;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Brutallus", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 2;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Felmyst", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 3;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Gauntlet Event", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 4;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Eredar Twins", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 5;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Muru", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 6;
SET @MAP_ID := 580;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "Sunwell Plateau - Kil'Jaeden", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 3;
SET @MAP_ID := 548;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "SSC - Hydross", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
-- Fix some wrong entries
UPDATE creature_encounter_respawn SET eventid = 5 WHERE eventid = 19;
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 5;
SET @MAP_ID := 548;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "SSC - Karathress", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 8;
SET @MAP_ID := 548;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "SSC - Lady Vash'j", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 9;
SET @MAP_ID := 548;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "SSC - Leotheras", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
SET @GROUPID := @GROUPID + 1;
SET @BOSS_STATE_ID := 10;
SET @MAP_ID := 548;
REPLACE INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@GROUPID, "SSC - Morogrim", 0x04);
REPLACE INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) 
    SELECT @GROUPID, 0, cer.guid 
	FROM creature_encounter_respawn cer
	JOIN creature c ON c.guid = cer.guid
	WHERE c.map = @MAP_ID AND cer.eventId = @BOSS_STATE_ID;
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = @MAP_ID AND `bossStateId` = @BOSS_STATE_ID;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (@MAP_ID,@BOSS_STATE_ID,0x17,@GROUPID,0x1); -- Spawn if state is not DONE
	
-- Cleaning up!
DROP TABLE IF EXISTS creature_encounter_respawn;
DELETE FROM command WHERE `name` = "npc seteventid";