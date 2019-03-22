# The Sunstrider Core

[![CircleCI](https://circleci.com/gh/kelno/sunstrider-core/tree/master.svg?style=svg&circle-token=935a3eadba63f221fd0f770768d2347437f4c260)](https://circleci.com/gh/kelno/sunstrider-core/tree/master)

Sunstrider is a WoW 2.4.3 emulator.
Development started in 2009 from a [trinitycore1](https://bitbucket.org/KPsN/trinitycore-243) fork and the core was being used on WoWMania, a french server. It was then rebranded to "Windrunner", where it was continuously developed for about 5 years. You can find the last Windrunner version [here](https://github.com/kelno/windrunner). Starting with our departure from WoWMania the core was renamed to Sunstrider.

## Installation

* [Windows](/doc/install_win.md)
* [Linux](/doc/install_linux.md)
 
## Main features

* Close to TrinityCore  
You can think of this core as TrinityCore for TBC.  
Over the years all major systems were rewritten using TC as a base. This allows us to continuously make use of the TrinityCore's recent developments and have a dev-friendly core using the well known infrastructure of TC. 
When a system has been imported from TC, differences (either improvements or TBC adaptations) from original TC are usually highlighted by comments.
* Automated testing system  
Our star feature! Mainly used for spells at the moment.
* [Continuous integration](https://travis-ci.org/sunstrider/sunstrider-core)  
Including tests results!
* Dynamic spawn system (see [here](https://github.com/TrinityCore/TrinityCore/pull/19056))
* Scripts and tests can be compiled in dynamic libraries and reloaded on runtime (see [here](https://trinitycore.atlassian.net/wiki/spaces/tc/pages/18874377/Using+the+script+hotswapping+system))
* Improved VMaps and MMaps  
Up to date with TC with some fixes of our own as well as some from Nostalrius.
* Spells
    * "A more powerful spell is active" mechanic
    * Binary spells
    * Heartbeat resist
    * Custom blinks and charges
    * A LOT of misc fixes in addition to TC systems
* EventAI has been dropped in favor of SmartAI  
Furthermore SmartAI has been used extensively in the last years of development on Sunstrider and has received various fixes and extensions on our part.
* Reworked map update logic  
Instances and battlegrounds may be updated multiple times while continents are updated, making instances/bgs less sensible to lag when the server is under high load.
* Creature soft evade on inaccessible targets
* Movemaps on transports!
* Years of service
A lot of improvements were made when the server was live:
    * Large debugged quest base  
A LOT of quests of all levels have been fixed, so we're benefiting from a good base on quests.
    * Good instance and boss scripts  
Most high level instance scripts are decent seeing other BC server ones (but still need work). Black temple and Sunwell scripts have been the most worked on and are close to perfect.
    * A lot of abuse prevention fixes  
    * Most world creatures use correct spells and have correct equipment

## Additional features (non blizzlike)

* [Playerbots](https://github.com/ike3/mangosbot)
* [Fully reworked stealth mechanics](http://wiki.valoren.org/index.php/Stealth_mechanics)
* Transports including static npcs (ship crew), purely cosmectic. Selling and combat is disabled for them.
* Dynamic PvP titles system depending on player rank in arena ladder (Disabled by default)
* ArenaSpectator (Not tested for a long while, we'll probably just trash it)
* Duel zone ([wiki](https://github.com/ValorenWoW/sunstrider-core/wiki/Duel-zone-(custom-feature)))
* Pack58 item packs


And much more. We've also developed some cool dev tools we'll probably share at a later point.

[Wiki](https://github.com/ValorenWoW/sunstrider-core/wiki) (WIP, import in progress)
