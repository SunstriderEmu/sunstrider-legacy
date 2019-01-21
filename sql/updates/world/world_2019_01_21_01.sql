
CREATE TEMPORARY TABLE temporary_tlk_petlevelstats AS SELECT * FROM pet_levelstats WHERE creature_entry = 510;
UPDATE temporary_tlk_petlevelstats SET patch = 5;
REPLACE INTO pet_levelstats SELECT * FROM temporary_tlk_petlevelstats;
UPDATE pet_levelstats SET mana = mana * 0.6975 WHERE creature_entry = 510 AND patch = 0;
