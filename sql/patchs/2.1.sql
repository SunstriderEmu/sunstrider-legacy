# PATCH 2.1

# Source - Explanation
# SQL

# PatchNote - Heroic difficulty instances end bosses will now always drop an epic item.
UPDATE creature_loot_template SET ChanceOrQuestChance = 100 WHERE item IN (187894,187895,187896,187897,187898,187900,187901,43006,187902,187903,187904,187905,187906,187907);
UPDATE gameobject_loot_template SET ChanceOrQuestChance = 100 WHERE item IN (187908);