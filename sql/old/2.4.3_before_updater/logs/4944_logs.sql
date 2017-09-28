ALTER TABLE arena_match ADD COLUMN team1_name varchar(255) not null default '' AFTER team1;
ALTER TABLE arena_match ADD COLUMN team2_name varchar(255) not null default '' AFTER team2;
