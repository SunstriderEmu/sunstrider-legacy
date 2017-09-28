ALTER TABLE arena_match ADD team1_members varchar(60) NOT NULL default '' AFTER team2;
ALTER TABLE arena_match ADD team2_members varchar(60) NOT NULL default '' AFTER team1_members;
