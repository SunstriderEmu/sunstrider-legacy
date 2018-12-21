-- Patch 2.0
UPDATE `areatrigger_teleport` SET `patch`=0 WHERE  `id`=4535 AND `patch`=0;

-- Patch 2.1
DELETE FROM `game_event_creature` WHERE  `guid`=7426 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=78133 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=78138 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=83719 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=84900 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=94964 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=311075 AND `event`=65;
DELETE FROM `game_event_creature` WHERE  `guid`=311078 AND `event`=65;
DELETE FROM `game_event_gameobject` WHERE  `guid`=17260 AND `event`=65;

UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=7426;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=78133;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=78138;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=83719;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=84900;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=94964;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=311075;
UPDATE `creature` SET `patch_min`=1 WHERE `spawnID`=311078;

UPDATE `gameobject` SET `patch_min`=1 WHERE `guid`=17260;

UPDATE `quest_template` SET `patch`=1 WHERE `entry`=10955 AND `patch`=0;
UPDATE `quest_template` SET `patch`=1 WHERE `entry`=10969 AND `patch`=0;
UPDATE `quest_template` SET `patch`=1 WHERE `entry`=10971 AND `patch`=0;
UPDATE `quest_template` SET `patch`=1 WHERE `entry`=10973 AND `patch`=0;
UPDATE `quest_template` SET `patch`=1 WHERE `entry`=10984 AND `patch`=0;

-- Set correct patch_min for Skettis - Sha'tari Skyguard
UPDATE `game_event` SET `patch_min`=1 WHERE  `entry`=68;

-- Set correct patch_min for Ogri'la
UPDATE `game_event` SET `patch_min`=1 WHERE  `entry`=71;

-- Black Temple opened in 2.1
UPDATE `areatrigger_teleport` SET `patch`=1 WHERE  `id`=4598 AND `patch`=0;

-- Patch 2.2
DELETE FROM `game_event_creature` WHERE  `guid`=6715 AND `event`=76;
DELETE FROM `game_event_creature` WHERE  `guid`=68077 AND `event`=76;
DELETE FROM `game_event_creature` WHERE  `guid`=311076 AND `event`=76;
DELETE FROM `game_event_creature` WHERE  `guid`=311077 AND `event`=76;

UPDATE `creature` SET `patch_min`=2 WHERE `spawnID`=6715;
UPDATE `creature` SET `patch_min`=2 WHERE `spawnID`=68077;
UPDATE `creature` SET `patch_min`=2 WHERE `spawnID`=311076;
UPDATE `creature` SET `patch_min`=2 WHERE `spawnID`=311077;

-- Patch 2.3
DELETE FROM `game_event_creature` WHERE  `guid`=10167 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=12114 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=12223 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=12224 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=12225 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=12226 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=17198 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=78562 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=78564 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=80692 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=83724 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=83725 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=83726 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=84347 AND `event`=66;
DELETE FROM `game_event_creature` WHERE  `guid`=93104 AND `event`=66;

UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=10167;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=12114;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=12223;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=12224;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=12225;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=12226;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=17198;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=78562;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=78564;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=80692;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=83724;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=83725;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=83726;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=84347;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93104;

DELETE FROM `game_event_gameobject` WHERE  `guid`=49870 AND `event`=66;
DELETE FROM `game_event_gameobject` WHERE  `guid`=49871 AND `event`=66;

-- UPDATE `quest_template` SET `patch`=3 WHERE `guid`=49870 AND `patch`=0;
-- UPDATE `quest_template` SET `patch`=3 WHERE `guid`=49871 AND `patch`=0;

-- Zul'aman
DELETE FROM `game_event_creature` WHERE  `guid`=2054 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=68182 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=68183 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=68184 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=90318 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=93080 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=93081 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=93082 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=93084 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=93085 AND `event`=77;
DELETE FROM `game_event_creature` WHERE  `guid`=95176 AND `event`=77;

UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=2054;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=68182;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=68183;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=68184;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=90318;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93080;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93081;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93082;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93084;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=93085;
UPDATE `creature` SET `patch_min`=3 WHERE `spawnID`=95176;

DELETE FROM `game_event_gameobject` WHERE  `guid`=44246 AND `event`=77;
-- UPDATE `quest_template` SET `patch`=3 WHERE `guid`=44246 AND `patch`=0;

DELETE FROM `game_event` WHERE `entry`=77;

UPDATE `areatrigger_teleport` SET `patch`=3 WHERE  `id`=4738 AND `patch`=0;

-- Patch 2.4
DELETE FROM `game_event_creature` WHERE  `guid`=1866 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=43249 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=64944 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=74633 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=94969 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95169 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95170 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95171 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95172 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95173 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95174 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95175 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95294 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=95362 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=311004 AND `event`=67;
DELETE FROM `game_event_creature` WHERE  `guid`=311005 AND `event`=67;

UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=1866;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=43249;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=74633;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=94969;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95169;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95170;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95171;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95172;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95173;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95174;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95175;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95294;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=95362;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=311004;
UPDATE `creature` SET `patch_min`=4 WHERE `spawnID`=311005;

-- Set patch min for Shattered Sun Offensive story line
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=35;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=36;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=37;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=38;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=39;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=40;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=41;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=42;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=43;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=44;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=49;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=45;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=46;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=48;
UPDATE `game_event` SET `patch_min`=4 WHERE  `entry`=47;

-- Sunwell Plateau (Entrance)
UPDATE `areatrigger_teleport` SET `patch`=4 WHERE  `id`=4889 AND `patch`=0;
-- Magisters' Terrace (Entrance)
UPDATE `areatrigger_teleport` SET `patch`=4 WHERE  `id`=4887 AND `patch`=0;
