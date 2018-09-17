START TRANSACTION;

UPDATE creature_template ct
JOIN creature_model_info cmi ON ct.modelid3 != 0 AND cmi.modelid_other_gender = ct.modelid3 AND modelid1 = cmi.modelid
SET ct.modelid3 = 0;

COMMIT;