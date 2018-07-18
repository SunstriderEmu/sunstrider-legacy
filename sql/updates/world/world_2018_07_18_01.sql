-- Shadow vulnerability, fixed redundancy
DELETE FROM spell_proc WHERE SpellId IN (17800, 17794, 17797, 17798, 17799);
REPLACE INTO spell_proc VALUES (-17794, 32, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0);

REPLACE INTO spell_script_names VALUES (-17794, "spell_warl_shadow_vulnerability");

-- Add ranks, currently the core only knows the Improved Shadowbolt ranks. (and yes, the id are not in incremental order)
REPLACE INTO spell_ranks VALUES(17794, 17794, 1), (17794, 17798, 2), (17794, 17797, 3), (17794, 17799, 4), (17794, 17800, 5);