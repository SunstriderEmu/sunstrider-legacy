DELETE FROM command WHERE `name` = 'debug valuessnapshot';
REPLACE INTO command VALUES ('debug snapshot', 3, 'Syntax: .debug snapshot [start|stop|dump]\r\nShow differences in unit updates field between start and stop.');
REPLACE INTO command VALUES ('debug snapshot go', 3, 'Syntax: .debug snapshot go #spawnID [start|stop|dump]\r\nShow differences in gobject updates field between start and stop.');

-- Delete old useless command
DELETE FROM command WHERE name = "debug update";
DELETE FROM trinity_string WHERE entry IN (562, 563);