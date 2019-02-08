-- Fix Gruul's Loot
DELETE FROM reference_loot_template WHERE Item IN (29765, 29766, 29767);

INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187913,	29765,	0,	0,	0,	1,	2,	1,	1,	"Leggings of the Fallen Hero",	0,	10),
(187913,	29766,	0,	0,	0,	1,	2,	1,	1,	"Leggings of the Fallen Champion",	0,	10),
(187913,	29767,	0,	0,	0,	1,	2,	1,	1,	"Leggings of the Fallen Defender",	0,	10);

UPDATE reference_loot_template SET GroupId = 1 WHERE Entry = 34051;

DELETE FROM creature_loot_template WHERE Entry = 19044;
INSERT IGNORE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(19044,	29434,	    0,	100,	0,	1,	0,	3,	3,	'Badge of Justice', 4,	10),
(19044,	31750,	    0,	100,	1,	1,	0,	1,	1,	'Earthen Signet',	0,	10),
(19044,	34051,	34051,	100,	0,	1,	1,	2,	2,	'Hors set',	0,	10),
(19044,	187913,	187913,	100,	0,	1,	2,	1,	1,	'Tokens',	0,	0),
(19044,	187913,	187913,	100,	0,	1,	2,	2,	2,	'Tokens',	1,	3),
(19044,	187913,	187913,	100,	0,	1,	2,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 19044 WHERE entry = 19044;

-- Fix HKM's Loot
UPDATE reference_loot_template SET GroupId = 2 WHERE Entry = 34050;

DELETE FROM creature_loot_template WHERE Entry = 18831;
INSERT IGNORE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(18831,	29434,	    0,	100,	0,	1,	0,	2,	2,	'Badge of Justice',	4,	10),
(18831,	28795,	    0,	0,	    0,	1,	1,	1,	1,	'Bladespire Warbands',	0,	10),
(18831,	28796,	    0,	0,	    0,	1,	1,	1,	1,	'Malefic Mask of the Shadows',	0,	10),
(18831,	28797,	    0,	0,	    0,	1,	1,	1,	1,	'Brute Cloak of the Ogre-Magi',	0,	10),
(18831,	28799,	    0,	0,	    0,	1,	1,	1,	1,	'Belt of Divine Inspiration',	0,	10),
(18831,	28800,	    0,	0,	    0,	1,	1,	1,	1,	'Hammer of the Naaru',	0,	10),
(18831,	28801,	    0,	0,	    0,	1,	1,	1,	1,	"Maulgar's Warhelm",	0,	10),
(18831,	34050,	34050,	100,	0,	1,	2,	1,	1,	'Tokens',	0,	0),
(18831,	34050,	34050,	100,	0,	1,	2,	2,	2,	'Tokens',	1,	3),
(18831,	34050,	34050,	100,	0,	1,	2,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 18831 WHERE entry = 18831;

-- Patch 2.3: Badges to Karazhan
UPDATE creature_loot_template SET patch_min = 3 WHERE Item = 29434 AND Entry IN (
    15550, -- Attumen
    15687, -- Moroes
    17521, -- The Big Bad Wolf
    18168, -- The Crone
    17533, -- Romulo
    17534, -- Julianne
    16457, -- Maiden of Virtue
    15691, -- The Curator
    15688, -- Terestian Illhoof
    16524, -- Shade of Aran
    15689, -- Netherspite
    17225, -- Nightbane
    15690 -- Prince Malchezaar
);

-- The Curator
DELETE FROM creature_loot_template WHERE Item IN (29756, 29757, 29758);
INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187914,	29756,	0,	0,	0,	1,	1,	1,	1,	"Gloves of the Fallen Hero",	0,	10),
(187914,	29757,	0,	0,	0,	1,	1,	1,	1,	"Gloves of the Fallen Champion",	0,	10),
(187914,	29758,	0,	0,	0,	1,	1,	1,	1,	"Gloves of the Fallen Defender",	0,	10);
INSERT IGNORE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(15691,	187914,	    0,	100,	0,	1,	1,	1,	1,	'Tokens',	0,	0),
(15691,	187914,	    0,	100,	0,	1,	1,	2,	2,	'Tokens',	1,	3),
(15691,	187914,	    0,	100,	0,	1,	1,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 15691 WHERE entry = 15691;

-- Prince Malchezaar
DELETE FROM creature_loot_template WHERE Item IN (29759, 29760, 29761);
INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187915,	29759,	0,	0,	0,	1,	1,	1,	1,	"Helm of the Fallen Hero",	0,	10),
(187915,	29760,	0,	0,	0,	1,	1,	1,	1,	"Helm of the Fallen Champion",	0,	10),
(187915,	29761,	0,	0,	0,	1,	1,	1,	1,	"Helm of the Fallen Defender",	0,	10);
INSERT IGNORE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(15690,	187915,	    0,	100,	0,	1,	1,	1,	1,	'Tokens',	0,	0),
(15690,	187915,	    0,	100,	0,	1,	1,	2,	2,	'Tokens',	1,	3),
(15690,	187915,	    0,	100,	0,	1,	1,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 15690 WHERE entry = 15690;

-- Patch 2.4: all raid bosses drops badges and 3 tokens (Maggy, Gruul & HKM already done)
UPDATE creature_loot_template SET patch_min = 4 WHERE Item = 29434 AND Entry IN (
    -- SSC
    21216, -- Hydross the Unstable
    21217, -- The Lurker Below
    21215, -- Leotheras the Blind
    21214, -- Fathom-Lord Karathress
    21213, -- Morogrim Tidewalker
    21212, -- Lady Vashj
    -- The Eye
    19514, -- Al'ar
    19516, -- Void Reaver
    18805, -- High Astromancer Solarian
    -- Hyjal Summit
    17767, -- Rage Winterchill
    17808, -- Anetheron
    17888, -- Kaz'rogal
    -- Black Temple
    22887, -- High Warlord Naj'entus
    22898, -- Supremus
    22841, -- Shade of Akama
    22871, -- Teron Gorefiend
    22948, -- Gurtogg Bloodboil
    23418, -- Reliquary of Souls
    22950 -- Zerevor
);

-- Void Reaver
DELETE FROM creature_loot_template WHERE Entry = 19516 AND Reference = 34054;
INSERT IGNORE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(19516,	34054,	    0,	100,	0,	1,	1,	1,	1,	'Tokens',	0,	0),
(19516,	34054,	    0,	100,	0,	1,	1,	2,	2,	'Tokens',	1,	3),
(19516,	34054,	    0,	100,	0,	1,	1,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 19516 WHERE entry = 19516;

-- Kael'thas
DELETE FROM reference_loot_template WHERE Item IN (30236, 30237, 30238);
INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187916,	30236,	0,	0,	0,	1,	1,	1,	1,	"Chestguard of the Vanquished Champion",	0,	10),
(187916,	30237,	0,	0,	0,	1,	1,	1,	1,	"Chestguard of the Vanquished Defender",	0,	10),
(187916,	30238,	0,	0,	0,	1,	1,	1,	1,	"Chestguard of the Vanquished Hero",	0,	10);
DELETE FROM creature_loot_template WHERE Entry = 19622 AND Reference = 90056;
REPLACE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(19622,	29434,	    0,	100,	0,	1,	0,	3,	3,	'Badge of Justice',	4,	10),
(19622,	187916,	    0,	100,	0,	1,	1,	1,	1,	'Tokens',	0,	0),
(19622,	187916,	    0,	100,	0,	1,	1,	2,	2,	'Tokens',	1,	3),
(19622,	187916,	    0,	100,	0,	1,	1,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 19622 WHERE entry = 19622;

-- Az'galor
UPDATE reference_loot_template SET GroupId = 2 WHERE Entry = 34063;
UPDATE reference_loot_template SET GroupId = 3 WHERE Entry = 34067;
REPLACE INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(17842,	29434,	0,	    100,	0,	1,	0,	2,	2,	'Badge of Justice',	4,	10),
(17842,	30896,	0,	    0,	    0,	1,	1,	1,	1,	'Glory of the Defender',	0,	10),
(17842,	30897,	0,	    0,	    0,	1,	1,	1,	1,	'Girdle of Hope',	0,	10),
(17842,	30898,	0,	    0,	    0,	1,	1,	1,	1,	"Shady Dealer\'s Pantaloons",	0,	10),
(17842,	30899,	0,	    0,	    0,	1,	1,	1,	1,	"Don Rodrigo\'s Poncho",	0,	10),
(17842,	30900,	0,	    0,	    0,	1,	1,	1,	1,	'Bow-stitched Leggings',	0,	10),
(17842,	30901,	0,	    0,	    0,	1,	1,	1,	1,	'Boundless Agony',	0,	10),
(17842,	34063,	34063,	2,	    0,	1,	2,	1,	1,	'Gems Design',	0,	10),
(17842,	34067,	34067,	100,	0,	1,	3,	2,	2,	'Tokens',	1,	3),
(17842,	34067,	34067,	100,	0,	1,	3,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 17842 WHERE entry = 17842;

-- Archimonde
DELETE FROM creature_loot_template WHERE Entry = 17968 AND Item IN (32285, 32289, 32295, 32296, 32297, 32298, 32303, 32307);
DELETE FROM reference_loot_template WHERE Item IN (31095, 31096, 31097);
UPDATE reference_loot_template SET GroupId = 1 WHERE Entry = 34068;
INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187917,	31095,	0,	0,	0,	1,	3,	1,	1,	"Helm of the Forgotten Protector",	0,	10),
(187917,	31096,	0,	0,	0,	1,	3,	1,	1,	"Helm of the Forgotten Vanquisher",	0,	10),
(187917,	31097,	0,	0,	0,	1,	3,	1,	1,	"Helm of the Forgotten Conqueror",	0,	10);
DELETE FROM creature_loot_template WHERE Entry = 17968;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(17968,	29434,	0,	    100,	0,	1,	0,	3,	3,	'Badge of Justice',	4,	10),
(17968,	34068,	34068,	100,	0,	1,	1,	1,	2,	'Hors set',	0,	10),
(17968,	34063,	34063,	2,	    0,	1,	2,	1,	1,	'Gems Design',	0,	10),
(17968,	187917,	187917,	100,	0,	1,	3,	2,	2,	'Tokens',	1,	3),
(17968,	187917,	187917,	100,	0,	1,	3,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 17968 WHERE entry = 17968;

-- Mother Sha'raz
UPDATE reference_loot_template SET GroupId = 2 WHERE Entry = 34069;
UPDATE reference_loot_template SET GroupId = 3 WHERE Entry = 34076;
DELETE FROM creature_loot_template WHERE Entry = 22947;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(22947,	29434,	    0,	100,	0,	1,	0,	2,	2,	'Badge of Justice',	4,	10),
(22947,	32365,	    0,	0,	    0,	1,	1,	1,	1,	'Heartshatter Breastplate',	0,	10),
(22947,	32366,	    0,	0,	    0,	1,	1,	1,	1,	"Shadowmaster\'s Boots",	0,	10),
(22947,	32367,	    0,	0,	    0,	1,	1,	1,	1,	'Leggings of Devastation',	0,	10),
(22947,	32368,	    0,	0,	    0,	1,	1,	1,	1,	'Tome of the Lightbringer',	0,	10),
(22947,	32369,	    0,	0,	    0,	1,	1,	1,	1,	'Blade of Savagery',	0,	10),
(22947,	32370,	    0,	0,	    0,	1,	1,	1,	1,	"Nadina\'s Pendant of Purity",	0,	10),
(22947,	34069,	34069,	2,	    0,	1,	2,	1,	1,	'Plans & Patterns',	0,	10),
(22947,	34076,	34076,	100,	0,	1,	3,	2,	2,	'Tokens',	1,	3),
(22947,	34076,	34076,	100,	0,	1,	3,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 22947 WHERE entry = 22947;

-- Illidari Council
UPDATE reference_loot_template SET GroupId = 2 WHERE Entry = 12008;
DELETE FROM creature_loot_template WHERE Item IN (31098, 31099, 31100);
REPLACE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187918,	31098,	0,	0,	0,	1,	1,	1,	1,	"Leggings of the Forgotten Conqueror",	0,	10),
(187918,	31099,	0,	0,	0,	1,	1,	1,	1,	"Leggings of the Forgotten Vanquisher",	0,	10),
(187918,	31100,	0,	0,	0,	1,	1,	1,	1,	"Leggings of the Forgotten Protector",	0,	10);

-- Zerevor has Badges & items
UPDATE creature_template SET lootid = 22950 WHERE entry = 22950;
-- Veras
DELETE FROM creature_loot_template WHERE Entry = 22952;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(22952,	187918,	187918,	100,	0,	1,	1,	1,	1,	"Tokens",	0,	10);
UPDATE creature_template SET lootid = 22952 WHERE entry = 22952;
-- Gathios
DELETE FROM creature_loot_template WHERE Entry = 22949;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(22949,	187918,	187918,	100,	0,	1,	1,	1,	1,	"Tokens",	0,	10);
UPDATE creature_template SET lootid = 22949 WHERE entry = 22949;
-- Malande
DELETE FROM creature_loot_template WHERE Entry = 22951;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(22951,	187918,	187918,	100,	0,	1,	1,	1,	1,	"Tokens",	4,	10),
(22951,	12008,	12008,	100,	0,	1,  2,	1,	1,	"Gems",	4,	10);
UPDATE creature_template SET lootid = 22951 WHERE entry = 22951;

-- Illidan
DELETE FROM reference_loot_template WHERE Item IN (31089, 31090, 31091);
REPLACE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187919,	31089,	0,	0,	0,	1,	4,	1,	1,	"Chestguard of the Forgotten Conqueror",	0,	10),
(187919,	31090,	0,	0,	0,	1,	4,	1,	1,	"Chestguard of the Forgotten Vanquisher",	0,	10),
(187919,	31091,	0,	0,	0,	1,	4,	1,	1,	"Chestguard of the Forgotten Protector",	0,	10);
UPDATE reference_loot_template SET GroupId = 3 WHERE Entry = 34077;
DELETE FROM creature_loot_template WHERE Entry = 22917;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(22917,	29434,	    0,	100,	0,	1,	0,	3,	3,	'Badge of Justice',	4,	10),
(22917,	32837,	    0,	4,	    0,	1,	1,	1,	1,	'Warglaive of Azzinoth',	0,	10),
(22917,	32838,	    0,	4,	    0,	1,	1,	1,	1,	'Warglaive of Azzinoth',	0,	10),
(22917,	34069,	34069,	2,	    0,	1,	2,	1,	1,	'Plans & Patterns',	0,	10),
(22917,	34077,	34077,	100,	0,	1,	3,	2,	2,	'Hors set',	0,	10),
(22917,	187919,	187919,	100,    0,	1,	4,	2,	2,	'Tokens',	1,	3),
(22917,	187919,	187919,	100,    0,	1,	4,	3,	3,	'Tokens',	4,	10);
UPDATE creature_template SET lootid = 22917 WHERE entry = 22917;

-- Misc fixes
DELETE FROM creature_loot_template WHERE Reference = 90069;