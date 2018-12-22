-- CREATURE_FLAG_EXTRA_INSTANCE_BIND = 1
UPDATE creature_template SET flags_extra = flags_extra & 1 WHERE entry IN (18473, 20912, 22841);