-- Fixed some triggers having random movement
UPDATE creature c 
JOIN creature_template ct ON c.id = ct.entry
SET c.MovementType = 0, c.spawndist = 0
WHERE ct.flags_extra & 0x80 AND c.spawndist != 0;