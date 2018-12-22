-- Data if from https://github.com/TrinityCore/TrinityCore/issues/20493 
UPDATE trainer SET Greeting = "Can I teach you how to turn the meat you find on beasts into a feast?" WHERE Id = 202004;
UPDATE trainer SET Greeting = "I can teach you how to use a fishing pole to catch fish." WHERE Id = 202001;
UPDATE trainer SET Greeting = "Here, let me show you how to bind those wounds..." WHERE Id = 202007;
UPDATE trainer SET Greeting = "Enchanting is the art of improving existing items through magic." WHERE Id = 201009 OR Id = 201010;
UPDATE trainer SET Greeting = "Greetings!  Can I teach you how to cut precious gems and craft jewelry?" WHERE Id IN (201024, 201025, 201026);
UPDATE trainer SET Greeting = "You have not lived until you have dug deep into the earth." WHERE Id IN (201033, 201034);
UPDATE trainer SET Greeting = "It requires a steady hand to remove the leather from a slain beast." WHERE Id IN (201036, 201037);
UPDATE trainer SET Greeting = "Engineering is very simple once you grasp the basics." WHERE Id IN (17634, 17637, 18752, 18775, 19576, 201012, 201015, 201016, 201017);
UPDATE trainer SET Greeting = "Greetings!  Can I teach you how to turn beast hides into armor?" WHERE Id IN (201027, 201028, 201030, 201031, 201032);
UPDATE trainer SET Greeting = "Care to learn how to turn the ore that you find into weapons and metal armor?" WHERE Id IN (201004, 201005);
UPDATE trainer SET Greeting = "With alchemy you can turn found herbs into healing and other types of potions." WHERE Id IN (17215, 201001, 201002);
UPDATE trainer SET Greeting = "Searching for herbs requires both knowledge and instinct." WHERE Id IN (201018, 201019);
UPDATE trainer SET Greeting = "Greetings!  Can I teach you how to turn found cloth into cloth armor?" WHERE Id IN (201039, 201040);
UPDATE trainer SET Greeting = "Hello!  Can I teach you something?" WHERE Id IN (202011);
UPDATE trainer SET Greeting = "There's more than one way to cook a fish." WHERE Id IN (19186);
