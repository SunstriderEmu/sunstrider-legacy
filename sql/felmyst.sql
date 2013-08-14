DELETE FROM `creature_scripts` WHERE `entryorguid` = 25038;
DELETE FROM `creature_scripts` WHERE `entryorguid` = 25265;
DELETE FROM `creature_scripts` WHERE `entryorguid` = 25267;
DELETE FROM `creature_scripts` WHERE `entryorguid` = 25268;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'boss_felmyst' WHERE `entry` = 25038;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'mob_felmyst_vapor' WHERE `entry` = 25265;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'mob_felmyst_trail' WHERE `entry` = 25267;

UPDATE `spell_template` SET `effectBasePoints1` = '45716' WHERE `id` = 45714;

