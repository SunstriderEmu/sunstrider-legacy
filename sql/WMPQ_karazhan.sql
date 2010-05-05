-- Curator's adds health
UPDATE creature_template SET minhealth = 12000, maxhealth = 12000 WHERE entry = 17096;

-- Terestian's damage
-- Original values :
-- +-------------------+-------------+--------+--------+
-- | name              | attackpower | mindmg | maxdmg |
-- +-------------------+-------------+--------+--------+
-- | Terestian Illhoof |       21332 |   2003 |   4093 | 
-- +-------------------+-------------+--------+--------+

UPDATE creature_template SET attackpower = 25375, mindmg = 3375, maxdmg = 3875 WHERE entry = 15688;

-- Terestian's Demon Chains
UPDATE creature_template SET minhealth = 13000, maxhealth = 13000 WHERE entry = 17248;

-- Malchezaar Axes
UPDATE creature_template SET mindmg = 113, maxdmg = 263, attackpower = 1312 WHERE entry = 17650;

-- Aran
UPDATE creature_template SET mechanic_immune_mask = 650854235 WHERE entry = 16524; -- Immune to interrupt
UPDATE creature_template SET minhealth = 10600, maxhealth = 10600 WHERE entry = 17167; -- Water elem
