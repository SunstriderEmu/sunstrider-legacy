UPDATE `creature_template` SET `InhabitType` = '3' WHERE `entry` = 25319;
UPDATE `creature_template` SET `InhabitType` = '3' WHERE `entry` = 25502;
UPDATE `creature_template` SET `InhabitType` = '3' WHERE `entry` = 25770;
UPDATE `creature_template` SET `InhabitType` = '3' WHERE `entry` = 22917;
UPDATE `creature_template` SET `InhabitType` = '3' WHERE `entry` = 25038;

DELETE FROM `creature_scripts` WHERE `entryorguid` = 25038;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'boss_felmyst' WHERE `entry` = 25038;

DELETE FROM `creature_scripts` WHERE `entryorguid` = 25265;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'mob_felmyst_vapor' WHERE `entry` = 25265;

DELETE FROM `creature_scripts` WHERE `entryorguid` = 25267;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'mob_felmyst_trail' WHERE `entry` = 25267;

DELETE FROM `creature_scripts` WHERE `entryorguid` = 25268;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = '' WHERE `entry` = 25268;

UPDATE `spell_template` SET `effectBasePoints1` = '45716' WHERE `id` = 45714;

UPDATE `creature_template` SET `minmana` = '3387', `maxmana` = '3387' WHERE `entry` = 25038;

UPDATE `spell_template` SET `rangeIndex` = 3, `effectRadiusIndex1` = 9 WHERE `id` = 45782;
UPDATE `spell_template` SET `effectRadiusIndex1` = 8, `effectImplicitTargetA1` = 73 WHERE `id` = 45392;
