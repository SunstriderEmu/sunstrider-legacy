ALTER TABLE lottery ADD COLUMN ip VARCHAR(30) not null default '0.0.0.0' AFTER faction;
