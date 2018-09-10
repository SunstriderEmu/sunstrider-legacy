-- Warlord Dar'toon
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 26312, npcflag = npcflag | 0x1 WHERE entry = 19254;
-- Menu 26312
DELETE FROM gossip_text WHERE ID = 1871959;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871959, '','The ground of this place is still warm from freshly shed blood.  Ah, I would have given my own eye to be here when the demons tore through that portal and clashed with Azeroth\'s heroes!  What a battle it was!$B$BBut now, even more fierce fighting rages on the other side!', '');
DELETE FROM gossip_menu WHERE entry = 26312;
INSERT INTO gossip_menu (entry, text_id) VALUES (26312, 1871959);