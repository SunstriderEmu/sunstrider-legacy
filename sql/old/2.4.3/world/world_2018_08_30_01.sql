-- Remove incorrect flag UNIT_FLAG_IN_COMBAT from DB
UPDATE creature_template SET unit_flags = unit_flags & ~(0x80000) WHERE unit_flags & 0x80000;

-- Round some stats for all creatures
UPDATE creature_template SET BaseVariance = ROUND(BaseVariance, 1);
UPDATE creature_template SET HealthModifier = ROUND(HealthModifier, 1);
UPDATE creature_template SET ManaModifier = ROUND(ManaModifier, 1);