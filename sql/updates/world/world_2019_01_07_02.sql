-- Judgement of light spell_proc should affect higher ranks
UPDATE spell_proc SET SpellId = -20185 WHERE SpellId = 20185;

-- Fixed random warning
UPDATE spell_proc SET SpellId = 16880 WHERE SpellId = -16880;