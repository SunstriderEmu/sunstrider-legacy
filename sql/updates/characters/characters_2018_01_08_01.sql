-- Remove some invalid saved spells. Those may be existing if characters has been created a while ago
DELETE FROM character_spell WHERE spell IN (5420, 5419, 7376, 7381, 21156, 21178, 33948, 40121);