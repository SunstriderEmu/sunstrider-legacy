-- Remove flight for bats in the Tirisfal glades
UPDATE creature_template_movement SET Flight = 0 WHERE CreatureId IN (1512, 16354);