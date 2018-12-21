-- Fix NPC:857 Donal Osgood double spawn
DELETE FROM creature_entry WHERE spawnID=97495;
DELETE FROM creature WHERE spawnID=97495;