-- 
-- area trigger teleport
-- 
UPDATE `areatrigger_teleport` SET `patch`=1 WHERE  `id`=4311 AND `patch`=0; -- Battle Of Mount Hyjal, Alliance Base (Entrance)
UPDATE `areatrigger_teleport` SET `patch`=1 WHERE  `id`=4312 AND `patch`=0; -- Battle Of Mount Hyjal, Horde Base (Entrance)
UPDATE `areatrigger_teleport` SET `patch`=1 WHERE  `id`=4313 AND `patch`=0; -- Battle Of Mount Hyjal, Night Elf Base (Entrance)

-- Despawn Magister's Terrace until 2.4
UPDATE `creature` SET `patch_min`=4 WHERE `map`=585;
UPDATE `instance_template` SET `patch`=4 WHERE  `map`=585;

-- Despawn The Sunwell until 2.4
UPDATE `creature` SET `patch_min`=4 WHERE `map`=580;
UPDATE `instance_template` SET `patch`=4 WHERE  `map`=580;

-- Despawn Black Temple until 2.1
UPDATE `creature` SET `patch_min`=1 WHERE `map`=564;
UPDATE `instance_template` SET `patch`=1 WHERE  `map`=564;

-- Despawn The Battle for Mount Hyjal until 2.1
UPDATE `creature` SET `patch_min`=1 WHERE `map`=534;
UPDATE `instance_template` SET `patch`=1 WHERE  `map`=534;

-- Despawn Zul'Aman until 2.3
UPDATE `creature` SET `patch_min`=3 WHERE `map`=568;
UPDATE `instance_template` SET `patch`=3 WHERE  `map`=568;

-- Shattrath - G'eras
UPDATE `creature` SET `patch_min`=2 WHERE  `spawnID`=96654;
UPDATE `creature` SET `patch_min`=2 WHERE  `spawnID`=1310600;

-- 2.1
UPDATE `game_event` SET `patch_min`=1 WHERE  `entry`=65;
-- 2.2
UPDATE `game_event` SET `patch_min`=2 WHERE  `entry`=76;
-- 2.3
UPDATE `game_event` SET `patch_min`=3 WHERE  `entry`=66;
-- 2.4
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=67;
