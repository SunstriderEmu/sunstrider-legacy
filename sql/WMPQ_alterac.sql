-- UPDATE creature_template SET mechanic_immune_mask = 617299803 WHERE entry IN (11946, 11948);

UPDATE creature_template SET scriptname = "AV_Marshals" WHERE entry IN (14762, 14763, 14765, 14764);
UPDATE creature_template SET scriptname = "AV_Warmasters", rank = 1 WHERE entry IN (14773, 14776, 14772, 14777);
UPDATE creature_template SET scriptname = "boss_balinda", mechanic_immune_mask = 113981279 WHERE entry = 11949;
UPDATE creature_template SET scriptname = "mob_water_elemental", mechanic_immune_mask = 131088 WHERE entry = 25040;
UPDATE creature_template SET scriptname = "boss_drekthar", mechanic_immune_mask = 617299807 WHERE entry = 11946;
UPDATE creature_template SET scriptname = "boss_galvangar", mechanic_immune_mask = 80426847 WHERE entry = 11947;
UPDATE creature_template SET scriptname = "boss_vanndar", mechanic_immune_mask = 617299807 WHERE entry = 11948;
UPDATE creature_template SET minmana = 90000, maxmana = 90000 WHERE entry = 11949;
UPDATE creature_template SET minhealth = 96715, maxhealth = 96715 WHERE entry IN (14773, 14776, 14772);
UPDATE creature_template SET minhealth = 125734, maxhealth = 125734 WHERE entry = 14777;
UPDATE creature_template SET minhealth = 125734, maxhealth = 125734 WHERE entry = 14765;
UPDATE creature_template SET minhealth = 70000, maxhealth = 70000 WHERE entry IN (14762, 14763);
UPDATE creature_template SET minhealth = 91650, maxhealth = 91650 WHERE entry = 14764;

DELETE FROM script_texts WHERE entry BETWEEN -2100022 AND -2100000;
INSERT INTO script_texts (entry, content_default, content_loc2, comment) VALUES
(-2100019, "Begone, uncouth scum! The Alliance shall prevail in Alterac Valley!", "Arrière, vermine inculte ! L'Alliance prévaudra dans la Vallée d'Alterac !", "Balinda - YELL_AGGRO"),
(-2100020, "Filthy Frostwolf cowards! If you want a fight, you'll have to come to me!", "Lâches ! Si vous voulez me combattre, venez jusqu'à moi !", "Balinda - YELL_EVADE"),
(-2100021, "Die! Your kind has no place in Alterac Valley!", "Mourrez ! Il n'y a pas de place pour les gens de votre espèce dans la Vallée d'Alterac !", "Galvangar - YELL_AGGRO"),
(-2100022, "I'll never fall for that, fool! If you want a battle it will be on my terms and in my lair!", "Je ne tomberai pas pour ça, fous ! Si vous voulez la bagarre, ça sera selon mes règles et dans mon repaire !", "Galvangar - YELL_EVADE"),
(-2100008, "Soldiers of Stormpike, your General is under attack! I require aid! Come! Come! Slay these mangy Frostwolf dogs.", "Soldats de Foudrepique, votre Général subit une attaque ! J'ai besoin d'aide ! Venez, venez ! Tuez ces chiens galeux de Loup-de-givre !", "Vanndar - YELL_AGGRO"),
(-2100009, "You'll never get me out of me bunker, heathens!", "Vous ne m'attirerez pas hors de mon bâtiment, païens !", "Vanndar - YELL_EVADE"),
(-2100012, "I will tell you this much...Alterac Valley will be ours.", "Je vais vous le dire... La Vallée d'Alterac sera nôtre.", "Vanndar - YELL_RANDOM1"),
(-2100013, "Why don't ya try again without yer cheap tactics, pansies! Or are you too chicken?", "Pourquoi n'essayez-vous pas sans vos faibles tactiques ? Ou peut-être êtes-vous trop pleutres ?", "Vanndar - YELL_RANDOM2"),
(-2100014, "Your attacks are weak! Go practice on some rabbits and come back when you're stronger.", "Vos attaques sont faibles ! Entrainez-vous sur quelques lapins et revenez quand vous serez plus forts.", "Vanndar - YELL_RANDOM3"),
(-2100015, "We will not be swayed from our mission!", "Nous ne serons pas détournés de notre mission !", "Vanndar - YELL_RANDOM4"),
(-2100016, "It'll take more than you rabble to bring me down!", "Il faudra plus que vous pour me descendre !", "Vanndar - YELL_RANDOM5"),
(-2100017, "We, the Alliance, will prevail!", "Nous, l'Alliance, prévaudrons !", "Vanndar - YELL_RANDOM6"),
(-2100018, "Is that the best you can do?", "Est-ce là le mieux que vous puissiez faire ?", "Vanndar - YELL_RANDOM7"),
(-2100000, "Stormpike filth! In my keep?! Slay them all!", "Des ordures de Foudrepique, dans mon donjon ? Tuez-les tous !", "Drekthar - YELL_AGGRO"),
(-2100001, "You seek to draw the General of the Frostwolf legion out from his fortress? PREPOSTEROUS!", "Vous cherchez à faire sortir le Général des Loup-de-givre de sa forteresse ? ABSURDE !", "Drekthar - YELL_EVADE"),
(-2100003, "Your attacks are slowed by the cold, I think!", "Vos attaques sont ralenties par le froid, je crois !", "Drekthar - YELL_RANDOM1"),
(-2100004, "Today, you will meet your ancestors!", "Aujourd'hui, vous allez rencontrer vos ancêtres !", "Drekthar - YELL_RANDOM2"),
(-2100005, "If you will not leave Alterac Valley on your own, then the Frostwolves will force you out!", "Si vous ne quittez pas la Vallée d'Alterac de vous-mêmes, les Loup-de-givre vont vous mettre dehors !", "Drekthar - YELL_RANDOM3"),
(-2100006, "You cannot defeat the Frostwolf clan!", "Vous ne pouvez pas éliminer le clan Loup-de-givre !", "Drekthar - YELL_RANDOM4"),
(-2100007, "You are no match for the strength of the Horde!", "Vous ne pouvez rien face à la puissance de la Horde !", "YELL_RANDOM5");
