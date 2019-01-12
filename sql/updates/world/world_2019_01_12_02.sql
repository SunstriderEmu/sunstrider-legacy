-- Update boss armor

-- Warriors
-- 7700 = 6792 * 1,133686
UPDATE creature_template SET ArmorModifier = 1 WHERE entry IN (
    18831, -- HKM
    19044, -- Gruul
    17257, -- Magtheridon
    23576, -- Nalorakk
    23578, -- Jan'alai
    23577, -- Halazzi
    23863, -- Zul'jin
    21216, -- Hydross
    21217, -- Lurker
    21215, -- Leotheras
    21213, -- Morogrim
    19514, -- Al'ar
    22887, -- Naj'entus
    22898, -- Supremus
    22841, -- Akama
    22948, -- Gurtogg
    23419, -- RoS 2
    23420, -- RoS 3
    22917, -- Illidan
    24882, -- Brutallus
    25741 -- M'uru
);

-- Paladins
-- 7700 = 5474 * 1.406649
UPDATE creature_template SET ArmorModifier = 1 WHERE entry = 23574; -- Akil'zon
-- 6200 = 5474 * 1.1326269
UPDATE creature_template SET ArmorModifier = 1 WHERE entry IN (
    24239, -- Malacrass
    21214, -- Karathress
    21212, -- Vashj
    18805, -- Solarian
    19622, -- KT
    17767, -- Rage Winterchill
    17808, -- Anetheron
    17888, -- Kaz'rogal
    17842, -- Azgalor
    17968, -- Archimonde
    22871, -- Teron
    22947, -- Shahraz
    22949, -- Gathios (Council)
    24850, -- Kalecgos
    24892, -- Sathrovarr
    25038, -- Felmyst
    25166, -- Alythess
    25165 -- Sacrolash
);
-- 8800 = 6193 * 1.42
UPDATE creature_template SET ArmorModifier = 1.42 WHERE entry = 19516; -- Void Reaver

UPDATE creature_template SET ArmorModifier = 0 WHERE entry = 23418; -- RoS 1

UPDATE creature_template SET ArmorModifier = 1 WHERE entry = 25799; -- Shadowsword Fury Mage
UPDATE creature_template SET ArmorModifier = 1 WHERE entry = 25798; -- Shadowsword Fury Mage
UPDATE creature_template SET ArmorModifier = 1 WHERE entry = 25772; -- Void Sentinel
