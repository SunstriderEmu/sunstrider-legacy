-- Delete higher ranks of Hunter Ferocious Inspiration
DELETE FROM spell_affect WHERE entry IN (34460, 34459);
REPLACE INTO spell_template_override (entry, DurationIndex, Comment) VALUES(34457, 21, "Passive Ferocious Inspiration duration should be infinite");