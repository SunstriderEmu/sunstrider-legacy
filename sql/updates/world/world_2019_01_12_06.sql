-- Set Doomwalker respawn timer to 2-4 days
UPDATE creature SET spawntimesecs = 172800, spawntimesecs_max = 345600 WHERE spawnID = 84633;

-- Set Doom Lord Kazzak respawn timer to 4-7 days
UPDATE creature SET spawntimesecs = 345600, spawntimesecs_max = 604800 WHERE spawnID = 96999;
