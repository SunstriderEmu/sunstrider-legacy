DELETE FROM creature_queststarter WHERE id = 6928 AND quest = 2161;

UPDATE quest_template
SET
    ObjectiveText1 = "Put Dathric's Spirit to Rest",
    ReqSpellCast1 = NULL
WHERE entry = 10182;