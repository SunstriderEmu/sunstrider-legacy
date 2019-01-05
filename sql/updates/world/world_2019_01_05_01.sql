-- Set Scout Jyoba dead and immune to pc, npc, non attackable
UPDATE creature_template SET dynamicflags = dynamicflags | 0x20, unit_flags = unit_flags | 0x40384 WHERE entry = 18035;