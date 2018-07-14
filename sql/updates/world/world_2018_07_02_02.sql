-- Midsummer Celebrant
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22852, npcflag = npcflag | 0x1 WHERE entry = 16781;
-- Menu 22852
DELETE FROM gossip_text WHERE ID = 1871957;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871957, '','The Midsummer Fire Festival has something for everyone to enjoy!', 'The Midsummer Fire Festival has something for everyone to enjoy!');
DELETE FROM gossip_menu WHERE entry = 22852;
INSERT INTO gossip_menu (entry, text_id) VALUES (22852, 1871957);