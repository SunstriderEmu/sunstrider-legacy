-- Elune's blessing crew gossips

-- Mariner Frostnight
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22844, npcflag = npcflag | 0x1 WHERE entry = 25055;
-- Menu 22844
DELETE FROM gossip_text WHERE ID = 1871942;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871942, '','Elune has blessed us many times over with all the discoveries we have made.  But we must remain vigilant, $c.', 'Elune has blessed us many times over with all the discoveries we have made.  But we must remain vigilant, $c.');
DELETE FROM gossip_menu WHERE entry = 22844;
INSERT INTO gossip_menu (entry, text_id) VALUES (22844, 1871942);

-- Mariner Keenstar
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22844, npcflag = npcflag | 0x1 WHERE entry = 25054;

-- Mariner Stillglider
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22845, npcflag = npcflag | 0x1 WHERE entry = 25056;
-- Menu 22845
DELETE FROM gossip_text WHERE ID = 1871944;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871944, '','', 'Forgive me, $r, I must focus on my job.');
DELETE FROM gossip_menu WHERE entry = 22845;
INSERT INTO gossip_menu (entry, text_id) VALUES (22845, 1871944);

-- Captain Galind Windsword
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22846, npcflag = npcflag | 0x1 WHERE entry = 25050;
-- Menu 22846
DELETE FROM gossip_text WHERE ID = 1871946;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871946, '','Watch your step aboard Elune\'s Blessing, $r.  Though Elune\'s blessed us with the discovery of our new friends, we\'ve also had our share of hardships at sea.$b$bHowever, recovering souls lost at sea or lost in a new world is truly a blessing from Elune.', '');
DELETE FROM gossip_menu WHERE entry = 22846;
INSERT INTO gossip_menu (entry, text_id) VALUES (22846, 1871946);