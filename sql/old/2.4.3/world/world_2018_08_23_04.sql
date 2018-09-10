ALTER TABLE `recups_data`   
  CHANGE `classe` `classe` TINYINT(3) NOT NULL,
  CHANGE `faction` `faction` TINYINT(3) NOT NULL,
  CHANGE `phase` `phase` TINYINT(3) NOT NULL,
  CHANGE `metier` `metier` TINYINT(3) NOT NULL,
  CHANGE `stuff` `stuff` TINYINT(3) NOT NULL,
  CHANGE `stufflevel` `tier` TINYINT(3) NOT NULL;
  
DELETE FROM game_event WHERE entry = 78;