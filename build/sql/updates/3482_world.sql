ALTER TABLE creature_formations CHANGE COLUMN dist dist_min float unsigned NOT NULL default 0;
ALTER TABLE creature_formations ADD COLUMN dist_max float unsigned NOT NULL DEFAULT 0 after dist_min;
