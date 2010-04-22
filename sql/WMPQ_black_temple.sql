-- AOEAI for all Illidan's AOE spells
UPDATE `creature_template` SET `AIName` = 'AOEAI',`flags_extra` = '0',`minlevel` = '70',`maxlevel` = '70',`maxlevel` = '70',`faction_A` = '14',`faction_H` = '14' WHERE `entry` IN ('23336','23069','23259');
UPDATE `creature_template` SET `spell1` = '40029' WHERE `entry` = 23069;
UPDATE `creature_template` SET `spell1` = '40610' WHERE `entry` = 23259;
UPDATE `creature_template` SET `spell1` = '40836' WHERE `entry` = 23336;

-- Supremus emotes
UPDATE script_texts SET content_loc2 = "acquiert une nouvelle cible !" WHERE entry = -1564010;
UPDATE script_texts SET content_loc2 = "frappe le sol avec colère !" WHERE entry = -1564011;
UPDATE script_texts SET content_loc2 = "- Le sol commence à s'ouvrir !" WHERE entry = -1564012;

-- Remove Rapid Shot as it's way too fast and I can't fix it
UPDATE eventai_scripts SET creature_id = 23340 WHERE creature_id = 23339 AND action1_param1 = 41173;

-- Curse of Mending seems to be bugged
UPDATE eventai_scripts SET creature_id = 22856 WHERE id = 2285503;

-- Despawn Teron Gorefiend (too much bugged)
UPDATE creature SET spawnMask = 0 WHERE id = 22871;

-- Up damage of Enslaved Soul
UPDATE creature_template SET mindmg = 12, maxdmg = 62.57, attackpower = 262 WHERE entry = 23469;
