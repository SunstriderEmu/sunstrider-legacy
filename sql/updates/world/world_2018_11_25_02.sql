replace into `game_event` (`entry`, `start_time`, `end_time`, `occurence`, `length`, `description`, `world_event`) values('80',NULL,NULL,'5184000','2592000','Pirates Day','0');
replace into `game_event` (`entry`, `start_time`, `end_time`, `occurence`, `length`, `description`, `world_event`) values('81',NULL,NULL,'5184000','2592000','Day of the Dead (TLK)','0');

DELETE gec FROM game_event_creature gec JOIN creature_entry ce ON ce.spawnId = gec.guid AND ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);

REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 1 
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);
		
REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 2
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);
		
REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 7
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);

REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 12
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);

REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 26
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);

REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 80
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);

REPLACE INTO game_event_creature (guid, event) 
	SELECT ce.spawnId, 81
		FROM creature_entry ce 
		WHERE ce.entry IN (18927, 19148, 19169, 19171, 19172, 19173, 19175, 19176, 19177, 19178, 20102);
