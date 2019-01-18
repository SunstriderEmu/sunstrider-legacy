-- Fixed visibility for 832, this changed with Cataclysm
UPDATE creature_template_addon SET visibilityDistanceType = 0 WHERE entry = 832;