TRUNCATE TABLE recups;
ALTER TABLE recups ADD stufflevel INT NOT NULL AFTER origserv;
ALTER TABLE recups DROP screenshot1;
ALTER TABLE recups DROP screenshot2;
ALTER TABLE recups DROP screenshot3;
ALTER TABLE recups DROP screenshot4;
ALTER TABLE recups ADD screenshots TEXT NOT NULL AFTER nickfofo;
