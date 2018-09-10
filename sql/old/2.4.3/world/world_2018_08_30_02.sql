-- Dark portal defenders
UPDATE creature_template SET speed = 1.05, ArmorModifier = 1 WHERE entry = 18972;
UPDATE creature_template SET BaseVariance = 1 WHERE entry IN (18970, 18971, 18969, 18950, 18972, 18965, 18949, 18966, 18986, 18948);
UPDATE creature_template SET HealthModifier = 2, DamageModifier = 4.5 WHERE entry IN (18970, 18971, 18969, 18950, 18972, 18965, 18949, 18966, 18986, 18948);
UPDATE creature_template SET HealthModifier = 20 WHERE entry IN (18966, 18969);

-- Dark portal attackers
UPDATE creature_template SET DamageModifier = 5 WHERE entry = 18944;
UPDATE creature_template SET DamageModifier = 10 WHERE entry = 19005;
UPDATE creature_template SET DamageModifier = 15 WHERE entry = 18945;
UPDATE creature_template SET mechanic_immune_mask = 650854271 WHERE entry IN (18944, 18945, 19005);
