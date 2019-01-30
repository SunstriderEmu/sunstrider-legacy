UPDATE creature_formations SET angle = 0.7,  dist = 3 WHERE memberGUID = 1309320;
UPDATE creature_formations SET angle = -0.7, dist = 3 WHERE memberGUID = 1309321;


DELETE FROM creature_addon where spawnID = 135530;