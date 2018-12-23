-- 2.1
UPDATE creature SET patch_max = 1 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = -65);
UPDATE creature SET patch_min = 1 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = 65);
	
-- 2.2
UPDATE creature SET patch_max = 2 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = -76);
UPDATE creature SET patch_min = 2 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = 76);
	
-- 2.3
UPDATE creature SET patch_max = 3 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = -66);
UPDATE creature SET patch_min = 3 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = 66);
	
-- 2.4
UPDATE creature SET patch_max = 4 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = -67);
UPDATE creature SET patch_min = 4 WHERE spawnID IN 
	(SELECT guid FROM game_event_creature WHERE event = 67);

DELETE FROM game_event_creature WHERE event IN (65, 76, 66, 67, -65, -76, -66, -67);


-- 2.1
UPDATE gameobject SET patch_max = 1 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = -65);
UPDATE gameobject SET patch_min = 1 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = 65);
	
-- 2.2
UPDATE gameobject SET patch_max = 2 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = -76);
UPDATE gameobject SET patch_min = 2 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = 76);
	
-- 2.3
UPDATE gameobject SET patch_max = 3 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = -66);
UPDATE gameobject SET patch_min = 3 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = 66);
	
-- 2.4
UPDATE gameobject SET patch_max = 4 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = -67);
UPDATE gameobject SET patch_min = 4 WHERE guid IN 
	(SELECT guid FROM game_event_gameobject WHERE event = 67);
	
DELETE FROM game_event_gameobject WHERE event IN (65, 76, 66, 67, -65, -76, -66, -67);
