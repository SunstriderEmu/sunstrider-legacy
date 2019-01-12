-- Fix some error startups

-- Fix Onyxia's Lair spawnmask
CREATE TEMPORARY TABLE temporary_tlk_creature AS SELECT * FROM creature WHERE spawnMask = 3 AND map = 249;
UPDATE temporary_tlk_creature SET patch_min = 5;
INSERT IGNORE INTO creature SELECT * FROM temporary_tlk_creature;
UPDATE creature SET spawnMask = 1 WHERE spawnMask = 3 AND map = 249 AND patch_min = 0;

-- Fix some movementTypes
UPDATE creature SET MovementType = 0 WHERE spawnID IN (57507, 57527, 1058317, 1058318, 12851, 12852, 12853, 12854, 52872, 52889, 57596, 57597, 57793, 57967, 57968, 58449, 58450, 58451, 1013355, 1013356, 1013362, 1013367, 1052488, 1053296, 1053300);
UPDATE creature SET MovementType = 1 WHERE spawnID IN (1311600, 1311601, 1311608, 1311717, 1311718, 1311719, 1311720, 1311721, 1311722);

-- Add Echo of Medivh spawndist
UPDATE creature SET spawndist = 5 WHERE spawnID = 135827;

UPDATE creature_addon SET auras = NULL WHERE spawnID = 64205;

-- Fix Mount visuals for TLK
CREATE TEMPORARY TABLE temporary_tlk_mount AS SELECT * FROM creature_addon WHERE mount = 25278;
UPDATE temporary_tlk_mount SET patch = 5;
INSERT IGNORE INTO creature_addon SELECT * FROM temporary_tlk_mount;
UPDATE creature_addon SET mount = 0 WHERE mount = 25278 AND patch = 0;
