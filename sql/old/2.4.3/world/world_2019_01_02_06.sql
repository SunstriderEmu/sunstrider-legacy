UPDATE gameobject_template set patch = 0 WHERE `name` LIKE "Dummy TLK %";
DELETE FROM creature_template WHERE entry IN (36296, 34564, 36530, 36885, 38208) AND patch = 5;
UPDATE creature_template set patch = 0 WHERE `name` LIKE "Dummy TLK %";	