-- spawn_group_template (1 entries)
SET @FIRSTGROUPID := 5;
-- Boss state associations
SET @MAGTHERIDON := 0; -- boss state that controls group "Magtheridon"
 
-- Insert spawn group templates
DELETE FROM `spawn_group_template` WHERE `groupId` BETWEEN @FIRSTGROUPID+0 AND @FIRSTGROUPID+0;
INSERT INTO `spawn_group_template` (`groupId`,`groupName`,`groupFlags`) VALUES
    (@FIRSTGROUPID+0, "Magtheridon's Lair - Magtheridon", 0x04);
	
 -- Insert spawn group data
DELETE FROM `spawn_group` WHERE `groupId` BETWEEN @FIRSTGROUPID+0 AND @FIRSTGROUPID+0;
INSERT INTO `spawn_group` (`groupId`,`spawnType`,`spawnId`) VALUES
    (@FIRSTGROUPID+0, 0, 89603),
    (@FIRSTGROUPID+0, 0, 89604),
    (@FIRSTGROUPID+0, 0, 89605),
    (@FIRSTGROUPID+0, 0, 89606),
    (@FIRSTGROUPID+0, 0, 89607),
    (@FIRSTGROUPID+0, 0, 89608),
    (@FIRSTGROUPID+0, 0, 89609),
    (@FIRSTGROUPID+0, 0, 89610),
    (@FIRSTGROUPID+0, 0, 89611),
    (@FIRSTGROUPID+0, 0, 89862),
    (@FIRSTGROUPID+0, 0, 89863),
    (@FIRSTGROUPID+0, 0, 89864);
	
 -- Link information for instance script
DELETE FROM `instance_spawn_groups` WHERE `instanceMapId` = 544;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
    (544,@MAGTHERIDON,0x17,@FIRSTGROUPID+0,0x1); -- Spawn Magtheridon if state is not DONE
	