-- Navigator Mehran
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22847, npcflag = npcflag | 0x1 WHERE entry = 24837;
-- Menu 22847
DELETE FROM gossip_text WHERE ID = 1871948;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871948, '','I\'ve little time for talk, sorry.', '');
DELETE FROM gossip_menu WHERE entry = 22847;
INSERT INTO gossip_menu (entry, text_id) VALUES (22847, 1871948);

-- Sailor Henders
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22848, npcflag = npcflag | 0x1 WHERE entry = 24838;
-- Menu 22848
DELETE FROM gossip_text WHERE ID = 1871950;
INSERT INTO gossip_text (ID, COMMENT, text0_0, text0_1) VALUES (1871950, '','Fine day fer sailin\', innit?', '');
DELETE FROM gossip_menu WHERE entry = 22848;
INSERT INTO gossip_menu (entry, text_id) VALUES (22848, 1871950);

-- Abe the Cabin Boy
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22849, npcflag = npcflag | 0x1 WHERE entry = 24836;
-- Menu 22849
DELETE FROM gossip_text WHERE ID = 1871952;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871952, '','Cap\'n Stash is the BEST!', '');
DELETE FROM gossip_menu WHERE entry = 22849;
INSERT INTO gossip_menu (entry, text_id) VALUES (22849, 1871952);

-- Sailor Wicks
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22850, npcflag = npcflag | 0x1 WHERE entry = 24839;
-- Menu 22850
DELETE FROM gossip_text WHERE ID = 1871954;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871954, '','Torgoley works us hard, but he\'s a good captain, aye.', '');
DELETE FROM gossip_menu WHERE entry = 22850;
INSERT INTO gossip_menu (entry, text_id) VALUES (22850, 1871954);


-- Captain Stash
-- Link creature to first menu and ensure gossip is enabled
UPDATE creature_template SET gossip_menu_id = 22851, npcflag = npcflag | 0x1 WHERE entry = 24833;
-- Menu 22850
DELETE FROM gossip_text WHERE ID = 1871955;
INSERT INTO gossip_text (ID, comment, text0_0, text0_1) VALUES (1871955, '',"Welcome to the majestic Lady Mehley. I am Captain Torgoley.$b$bIf you've questions about the ship, feel free to ask; otherwise, please, enjoy the journey, as I always do... After all, to quote a favorite scribe of mine: My experience of ships is that on them one makes an interesting discovery about the world. One finds one can do without it completely.", '');
DELETE FROM gossip_menu WHERE entry = 22851;
INSERT INTO gossip_menu (entry, text_id) VALUES (22851, 1871955);