-- Fixed some invalid non attackable creatures
UPDATE creature_template SET unit_flags = 0 WHERE entry IN (16582);

ALTER TABLE `creature`   
  ADD COLUMN `unit_flags` INT(10) UNSIGNED DEFAULT 0  NOT NULL AFTER `MovementType`;

UPDATE creature SET unit_flags = 0x100 WHERE guid IN (56957, 56958, 56907);