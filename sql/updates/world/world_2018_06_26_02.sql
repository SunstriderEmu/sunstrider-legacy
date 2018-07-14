-- "The Bravery" gossip texts

-- Navigator Landerson
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22840, npcflag = npcflag | 0x1 WHERE entry = 25015;
-- Menu 22840
DELETE FROM gossip_text WHERE ID = 1871931;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871931, '','Sorry, but we\'ll have to speak some other time. Farewell.', '');
DELETE FROM gossip_menu WHERE entry = 22840;
INSERT INTO gossip_menu (entry, text_id) VALUES (22840, 1871931);

-- Captain Angelina Soluna
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22840, npcflag = npcflag | 0x1 WHERE entry = 25009;
-- Menu 22840
DELETE FROM gossip_text WHERE ID = 1871932;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871932, '','', 'Welcome to The Bravery, $gsir:madame;. Now if you\'ll excuse me...');
DELETE FROM gossip_menu WHERE entry = 22840;
INSERT INTO gossip_menu (entry, text_id) VALUES (22840, 1871932);

-- Sailor Wills
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22840, npcflag = npcflag | 0x1 WHERE entry = 25016;
-- Menu 22840
DELETE FROM gossip_text WHERE ID = 1871933;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871933, '','This greyin\' seadog may be the oldest of the crew, but he\'s also the toughest of the lot! Eh? Eh? Arms like an ogre!', '');
DELETE FROM gossip_menu WHERE entry = 22840;
INSERT INTO gossip_menu (entry, text_id) VALUES (22840, 1871933);

-- First Mate Wavesinger
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22840, npcflag = npcflag | 0x1 WHERE entry = 25011;
-- Menu 22840
DELETE FROM gossip_text WHERE ID = 1871934;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871934, '','The Bravery\'s crew will do its best to make your journey as swift and pleasant as possible.', '');
DELETE FROM gossip_menu WHERE entry = 22840;
INSERT INTO gossip_menu (entry, text_id) VALUES (22840, 1871934);

-- Sailor Stoneheel
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22841, npcflag = npcflag | 0x1 WHERE entry = 25018;
-- Menu 22841
DELETE FROM gossip_text WHERE ID = 1871936;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871936, '','', 'Unlucky name fer a sailor, innit?');
DELETE FROM gossip_menu WHERE entry = 22841;
INSERT INTO gossip_menu (entry, text_id) VALUES (22841, 1871936);

-- Sailor Fairfolk
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22842, npcflag = npcflag | 0x1 WHERE entry = 25017;
-- Menu 22842
DELETE FROM gossip_text WHERE ID = 1871938;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871938, '','', 'I\'m almost jealous of our Mr. Wavesinger. Why, he may well be prettier than me!');
DELETE FROM gossip_menu WHERE entry = 22842;
INSERT INTO gossip_menu (entry, text_id) VALUES (22842, 1871938);

-- Sentinel Winterdew
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22843, npcflag = npcflag | 0x1 WHERE entry = 25014;
-- Menu 22843
DELETE FROM gossip_text WHERE ID = 1871940;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871940, '','', 'We\'ve sworn to do our very best to protect the passengers of The Bravery. The sea lane between Auberdine to Stormwind Harbor must remain safe.');
DELETE FROM gossip_menu WHERE entry = 22843;
INSERT INTO gossip_menu (entry, text_id) VALUES (22843, 1871940);

-- Sentinel Brightgrass
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22843, npcflag = npcflag | 0x1 WHERE entry = 25013;

