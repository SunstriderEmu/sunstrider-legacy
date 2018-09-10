UPDATE gossip_menu_option SET option_text = "more" WHERE menu_id = 20066;
UPDATE gossip_menu_option SET option_text = "back" WHERE menu_id = 20064;

REPLACE INTO gossip_text (ID, comment, text0_0) VALUES (26310, "Beta helper npc", "Welcome $N$B$BLet me help you jump into the action immediately!");
REPLACE INTO gossip_menu VALUES (26310, 26310);
UPDATE creature_template SET gossip_menu_id = 26310 WHERE entry = 26310;

REPLACE INTO gossip_text (ID, comment, text0_0) VALUES (26311, "Beta info npc", "Please join our discord at valoren.org$B$B There you'll find:$B$B - Info on what should be currently tested$B$B- News about about project progress$B$B- Other players and the developers to talk with!");
REPLACE INTO gossip_menu VALUES (26311, 26311);
UPDATE creature_template SET gossip_menu_id = 26311 WHERE entry = 26311;

UPDATE creature_template SET name = "Beta Infos" WHERE entry = 26311;