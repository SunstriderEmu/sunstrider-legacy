DELETE cdt, t, ts
FROM creature_default_trainer cdt
LEFT JOIN trainer t ON t.Id = cdt.TrainerId
LEFT JOIN trainer_spell ts ON t.Id = ts.TrainerId
WHERE cdt.patch_min >= 5;

DELETE t, ts
FROM trainer t 
LEFT JOIN trainer_spell ts ON t.Id = ts.TrainerId
WHERE t.Id < 202011;
