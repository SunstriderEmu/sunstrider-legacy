DELETE ca FROM creature_addon ca LEFT JOIN waypoint_data wd ON wd.id = ca.path_id WHERE wd.id is NULL AND ca.path_id is not NULL;
ALTER TABLE creature_addon
CHANGE path_id path_id int(10) unsigned NULL AFTER spawnID;
UPDATE creature_addon SET path_id = NULL WHERE path_id = 0;
ALTER TABLE creature_addon
ADD FOREIGN KEY (path_id) REFERENCES waypoint_data (id) ON DELETE SET NULL ON UPDATE CASCADE;

DELETE cta FROM creature_template_addon cta LEFT JOIN waypoint_data wd ON wd.id = cta.path_id WHERE wd.id is NULL AND cta.path_id is not NULL;
ALTER TABLE creature_template_addon
CHANGE path_id path_id int(10) unsigned NULL AFTER entry;
UPDATE creature_template_addon SET path_id = NULL WHERE path_id = 0;
ALTER TABLE creature_template_addon
ADD FOREIGN KEY (path_id) REFERENCES waypoint_data (id) ON DELETE SET NULL ON UPDATE CASCADE;