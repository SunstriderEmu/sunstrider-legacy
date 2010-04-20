DELETE FROM creature_template WHERE entry IN (4, 5);
INSERT INTO creature_template(entry, modelid_A, modelid_A2, modelid_H, modelid_H2, name, subname, minlevel, maxlevel, minhealth, maxhealth, faction_A, faction_H, npcflag, unit_flags, flags_extra, Scriptname) VALUES(4, 5566, 5566, 5566, 5566, 'KingPin', 'Bloqueur d\'expérience', 70, 70, 42000, 42000, 35, 35, 1, 2, 2, 'npc_xp_blocker');
INSERT INTO creature_template(entry, modelid_A, modelid_A2, modelid_H, modelid_H2, name, subname, minlevel, maxlevel, minhealth, maxhealth, faction_A, faction_H, npcflag, unit_flags, flags_extra, Scriptname) VALUES(5, 14360, 14360, 14360, 14360, 'KingPin', 'Bloqueur d\'expérience', 70, 70, 42000, 42000, 35, 35, 1, 2, 2, 'npc_xp_blocker');

DELETE FROM npc_gossip WHERE textid = 42;
INSERT INTO npc_gossip VALUES(15424628, 42);
INSERT INTO npc_gossip VALUES(15424629, 42);

DELETE FROM npc_text WHERE id = 42;
INSERT INTO npc_text(id, text2_0) VALUES(42, "Salut à toi, aventurier !$B$BSi tu viens me voir, c'est que tu souhaites geler ton niveau actuel et ne plus progresser en expérience. Est-ce bien cela ?$BRéfléchis bien, car cela te coûtera 10 pièces d'or. Par contre, dégeler ta barre d'expérience est gratuit. La manoeuvre est répétable autant de fois que tu le voudras, mais bien sûr, tu dois t'acquitter de cette somme d'argent à chaque fois.$B$BTu as toutes les cartes en main, fais ton choix...");

DELETE FROM creature WHERE id IN (4, 5);
INSERT INTO creature (guid, id, map, spawnMask, position_x, position_y, position_z, orientation, spawntimesecs, movementtype) VALUES
-- Alliance
(15424628, 4, 0, 1, -8408.185547, 271.648163, 120.885590, 0.745878, 60, 0),
-- Horde
(15424629, 5, 1, 1, 1977.396118, -4789.833008, 55.793800, 5.756231, 60, 0);
