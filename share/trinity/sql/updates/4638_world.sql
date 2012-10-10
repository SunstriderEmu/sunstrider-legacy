DELETE FROM command WHERE name = "guild rename";
INSERT INTO command (name, security, help) VALUES
("guild rename", 3, ".guild rename $id $nom - renomme une guilde");
