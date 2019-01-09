-- Remove flight for bats in the Tirisfal glades
UPDATE creature_template_movement SET Flight = 0 WHERE CreatureId = 1512;