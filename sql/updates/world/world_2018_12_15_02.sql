UPDATE creature_template SET faction = 35 WHERE entry IN (582, 26311, 26837, 26310);
UPDATE creature_template SET entry = entry + 100000 WHERE entry IN (26311, 26837, 26310);