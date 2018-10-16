ALTER TABLE waypoint_scripts DROP FOREIGN KEY waypoint_scripts_ibfk_1;

ALTER TABLE waypoint_data
DROP FOREIGN KEY waypoint_data_ibfk_1,
ADD FOREIGN KEY (action) REFERENCES waypoint_scripts (id) ON DELETE SET NULL ON UPDATE CASCADE; 