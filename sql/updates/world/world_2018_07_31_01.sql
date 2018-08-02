-- Allow blackout (priest talent) to proc on dots cast
UPDATE spell_proc SET SpellTypeMask = SpellTypeMask | 0x4 WHERE SpellId = -15268;