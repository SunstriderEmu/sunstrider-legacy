# The Sunstrider Core

[![Build Status](https://travis-ci.org/ValorenWoW/sunstrider-core.svg?branch=master)](https://travis-ci.org/ValorenWoW/sunstrider-core)

Sunstrider is a WoW 2.4.3 emulator used and developed by the Valoren private server.
Development started in 2009 from a [trinitycore1](https://bitbucket.org/KPsN/trinitycore-243) fork as the core started being used on WoWMania, a french server, and was then rebranded "Windrunner", where it was continuously developed for about 5 years. You can find the last Windrunner version [here](https://github.com/kelno/windrunner). Starting with our departure from WoWMania the core was renamed Sunstrider.

## Installation

* [Windows](/doc/install_win.md)
* [Linux](/doc/install_linux.md)

## Main features

* Close to TrinityCore  
You can think of this core as TrinityCore on TBC.  
Over the years all majors systems were rewritten using TC as a base. This allows us to continuously make us of the TrinityCore recent developments and have a dev-friendly core using the well known infrastructure of TC. 
When a system has been imported from TC, differences (either improvements or TBC adaptations) with original TC are usually highlighted by comments.
* Partly open source  
With the exception of instance and creatures scripts, as well as the AntiCheat. The core is not currently meant to be used as is outside of the Valoren server, but you're free to re use parts of it.
* Automated testing system  
Our star feature! Mainly used for spells at the moment.
* [Continuous integration](https://travis-ci.org/ValorenWoW/sunstrider-core)  
Including tests results!
* Dynamic spawn system (see [here](https://github.com/TrinityCore/TrinityCore/pull/19056))
* Scripts and tests can be compiled in dynamic libraries and reloaded on runtime (see [here](https://trinitycore.atlassian.net/wiki/spaces/tc/pages/18874377/Using+the+script+hotswapping+system))
* Improved VMaps and MMaps  
Up to date from TC with some fixes on our own as well as some from Nostalrius.
* Spells
    * "A more powerful spell is active" mechanic
    * Binary spells
    * Heartbeat resist
    * Custom blink and charges
    * A LOT of misc fixes in addition to TC systems
* EventAI has been dropped in favor of SmartAI  
Furthermore SmartAI has been used extensively in the last years of development on Sunstrider and has received various fixes and extensions on our part.
* Reworked map update logic  
Instances and battlegrounds may be updated multiple times while continents are updated, making instance/bgs less sensible to lag when the server is under high load.
* Creature soft evade on target not accessible
* Movemaps on transports!
* Years of services
Lot have been done when the server was live:
    * Large debugged quest base  
A LOT of quests of all levels have been fixed, so we're benefiting from a good base on quests.
    * Good instance and boss scripts  
Most high level instance scripts are decent seeing other BC server ones (but still need work). Black temple and SunWell scripts have been the most worked on and are close to perfect.
    * A lot of possible abuses fixes  
    * Most world creatures use their spells and have correct equipment.

## Additional features (non blizzlike)

* [Playerbots](https://github.com/ike3/mangosbot)
* [Fully reworked stealth mechanics](http://wiki.valoren.org/index.php/Stealth_mechanics)
* Transports includes static npcs (ship crew), purely cosmectic. Selling and combat has been disabled for them.
* Dynamic PvP titles system depending on player rank in arena ladder (Disabled by default)
* ArenaSpectator (Not tested for a long while, we'll probably just trash it)
* Duel zone ([wiki](https://github.com/ValorenWoW/sunstrider-core/wiki/Duel-zone-(custom-feature)))
* Pack58 item packs


And much more. We've also developed some cool dev tools we'll probably share at a later point.

[Wiki](https://github.com/ValorenWoW/sunstrider-core/wiki) (WIP, import in progress)
