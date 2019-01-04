-- PATCH 2.3
--
-- Source - Explanation
-- SQL

-- Warden Iolol becomes Warden Moi'bff Jill
UPDATE creature_template SET name = "Warden Moi'bff Jill" WHERE entry = 18408;
UPDATE gossip_text SET text_0 = "I used to be known as a Iolol until a secret vote was passed around town that resulted in me having to change my name. It was either change it or go the way of Corki. You would have done the same. What do you think of my new name? Huge improvement, right?$b
By the way, check the bulletin board if you're looking for work." WHERE ID = 9375;
UPDATE quest_template SET Objectives = replace(Objectives, "Warden Iolol", "Warden Moi'bff Jill");
