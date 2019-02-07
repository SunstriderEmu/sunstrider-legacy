DELETE gec FROM game_event_gameobject gec 
JOIN gameobject g ON gec.guid = g.guid
JOIN gameobject g2 ON g2.position_x < g.position_x + 1 AND g2.position_x > g.position_x - 1 AND g2.position_y < g.position_y + 1 AND g2.position_y > g.position_y - 1 AND g2.position_z < g.position_z + 1 AND g2.position_z > g.position_z - 1 
JOIN gameobject_template gt2 ON gt2.entry = g2.id AND gt2.type = 6 AND gt2.data3 = 7897
WHERE g.id = 2061;

DELETE g FROM gameobject g
JOIN gameobject g2 ON g2.position_x < g.position_x + 1 AND g2.position_x > g.position_x - 1 AND g2.position_y < g.position_y + 1 AND g2.position_y > g.position_y - 1 AND g2.position_z < g.position_z + 1 AND g2.position_z > g.position_z - 1 
JOIN gameobject_template gt2 ON gt2.entry = g2.id AND gt2.type = 6 AND gt2.data3 = 7897
WHERE g.id = 2061;