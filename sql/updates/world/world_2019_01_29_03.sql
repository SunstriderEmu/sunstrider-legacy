ALTER TABLE creature_loot_template
DROP INDEX `PRIMARY`,
ADD PRIMARY KEY(`Entry`,`Item`,`patch_min`);

UPDATE creature_loot_template clt
LEFT JOIN item_template it ON it.entry = clt.Item
SET clt.Comment = it.name
WHERE clt.Reference != clt.Item;

UPDATE reference_loot_template rlt
LEFT JOIN item_template it ON it.entry = rlt.Item
SET rlt.Comment = it.name
WHERE rlt.Reference != rlt.Item;

-- Fix Magtheridon's loot
DELETE FROM reference_loot_template WHERE Item IN (29753, 29754, 29755);

INSERT IGNORE INTO reference_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(187912,	29753,	0,	0,	0,	1,	2,	1,	1,	"Chestguard of the Fallen Defender",	0,	10),
(187912,	29754,	0,	0,	0,	1,	2,	1,	1,	"Chestguard of the Fallen Champion",	0,	10),
(187912,	29755,	0,	0,	0,	1,	2,	1,	1,	"Chestguard of the Fallen Hero",	0,	10);

UPDATE reference_loot_template SET GroupId = 1 WHERE Entry = 34039;
UPDATE reference_loot_template SET GroupId = 3 WHERE Entry = 90039;

UPDATE creature_template SET lootid = NULL WHERE entry = 17257;
DELETE FROM creature_loot_template WHERE Entry = 17257;
INSERT INTO creature_loot_template (Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount, Comment, patch_min, patch_max) VALUES
(17257,	29434,	0,	    100,	0,	1,	0,	3,	3,	"Badge of Justice",	4,	10),
(17257,	34845,	0,	    100,	0,	1,	0,	1,	1,	"Pit Lord's Satchel",	4,	10),
(17257,	34039,	34039,	100,	0,	1,	1,	2,	2,	"Hors set",	0,	10),
(17257,	187912,	187912,	100,	0,	1,	2,	1,	1,	'Tokens',	0,	0),
(17257,	187912,	187912,	100,	0,	1,	2,	2,	2,	'Tokens',	1,	3),
(17257,	187912,	187912,	100,	0,	1,	2,	3,	3,	'Tokens',	4,	10),
(17257,	34846,	0,	    100,	0,	1,	0,	1,	1,	"Black Sack of Gems",	4,	10),
(17257,	90039,	90039,	100,	0,	1,	3,	1,	1,	"Magtheridon's Head",	1,	10);
UPDATE creature_template SET lootid = 17257 WHERE entry = 17257;
