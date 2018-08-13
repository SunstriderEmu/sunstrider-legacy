DELETE FROM creature WHERE id = 15;

UPDATE gossip_text SET text0_0 = "Greetings, adventurer!$B$BI can freeze your XP bar for the modest sum of 10 gold. You can then come to me anytime to unfreeze it for free." WHERE ID = 42;
DELETE FROM gossip_text WHERE ID IN (6, 5, 10);