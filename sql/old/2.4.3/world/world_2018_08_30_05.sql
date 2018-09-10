-- Fixed some invalid non attackable creatures
UPDATE creature_template SET unit_flags = 0 WHERE entry IN (16580, 19255, 16842, 19762);