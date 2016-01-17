/* Copyright (C) 2006 - 2008 TrinityScript <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "ConfigMgr.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Spell.h"
#include "ConfigMgr.h"
#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Player.h"
#include "GossipDef.h"

class CreatureScript;

#define _FULLVERSION "TrinityScript"

#ifndef _TRINITY_SCRIPT_CONFIG
# define _TRINITY_SCRIPT_CONFIG  "worldserver.conf"
#endif // _TRINITY_SCRIPT_CONFIG

// String text additional data, used in TextMap
struct StringTextData
{
    uint32 SoundId;
    uint8  Type;
    uint32 Language;
    uint32 Emote;
};

#define TEXT_SOURCE_RANGE   -1000000                        //the amount of entries each text source has available

// Text Maps
std::unordered_map<int32, StringTextData> TextMap;

void LoadOverridenSQLData();
void LoadOverridenDBCData();

// -- Scripts to be added --

// -- Areatrigger --
extern void AddSC_areatrigger_scripts();

// -- Outdoors dragons --
extern void AddSC_boss_dragonsofnightmare();

// -- Creature --
extern void AddSC_generic_creature();

// -- Custom --
extern void AddSC_npc_rez();
extern void AddSC_training_dummy();
extern void AddSC_zone_silence();
extern void AddSC_custom_example();
extern void AddSC_custom_gossip_codebox();
extern void AddSC_onevents();
extern void AddSC_npc_lottery();
extern void AddSC_theinform();
extern void AddSC_mylittlebombling();
extern void AddSC_firework_controller();
extern void AddSC_npc_interpreter();
extern void AddSC_custom_gnominizer();
extern void AddSC_pack58();

extern void AddSC_catapultmaster();
extern void AddSC_npc_teleporter();
extern void AddSC_npc_teleporter_pvpzone();

// -- GO --
extern void AddSC_go_scripts();

// -- Guard --
extern void AddSC_guards();

// -- Honor --

// -- Item --
extern void AddSC_item_scripts();
extern void AddSC_item_test();

// -- NPC --
extern void AddSC_npc_professions();
extern void AddSC_npcs_special();
extern void AddSC_npc_xp_blocker();

// -- Servers --
extern void AddSC_SmartSCripts();

// -- Misc --
extern void AddSC_arena_spectator_script();
extern void AddSC_hallows_end();
extern void AddSC_arenabeastmaster();

//--------------------
//------ ZONE --------

//Alterac Mountains
extern void AddSC_alterac_mountains();

//Alterac Valley
extern void AddSC_AV_Warmasters();
extern void AddSC_boss_balinda();
extern void AddSC_boss_drekthar();
extern void AddSC_boss_galvangar();
extern void AddSC_boss_vanndar();
extern void AddSC_alterac_bowman();

//Arathi Highlands
extern void AddSC_arathi_highlands();

//Ashenvale Forest
extern void AddSC_ashenvale();

//Aunchindoun
//--Auchenai Crypts
extern void AddSC_boss_exarch_maladaar();
extern void AddSC_boss_shirrak_the_dead_watcher();

//--Mana Tombs
extern void AddSC_boss_nexusprince_shaffar();
extern void AddSC_boss_pandemonius();

//--Sekketh Halls
extern void AddSC_boss_darkweaver_syth();
extern void AddSC_boss_talon_king_ikiss();
extern void AddSC_boss_anzu();
extern void AddSC_instance_sethekk_halls();
extern void AddSC_sethekk_halls();

//--Shadow Labyrinth
extern void AddSC_boss_ambassador_hellmaw();
extern void AddSC_boss_blackheart_the_inciter();
extern void AddSC_boss_grandmaster_vorpil();
extern void AddSC_boss_murmur();
extern void AddSC_instance_shadow_labyrinth();

//Azshara
extern void AddSC_boss_azuregos();
extern void AddSC_azshara();

//Azuremyst Isle
extern void AddSC_azuremyst_isle();

//Badlands
//Barrens
extern void AddSC_the_barrens();

//Black Temple
extern void AddSC_black_temple();
extern void AddSC_boss_illidan();
extern void AddSC_boss_shade_of_akama();
extern void AddSC_boss_supremus();
extern void AddSC_boss_gurtogg_bloodboil();
extern void AddSC_boss_mother_shahraz();
extern void AddSC_boss_reliquary_of_souls();
extern void AddSC_boss_teron_gorefiend();
extern void AddSC_boss_najentus();
extern void AddSC_boss_illidari_council();
extern void AddSC_instance_black_temple();

//Blackfathom Depths
extern void AddSC_blackfathom_deeps();
extern void AddSC_boss_gelihast();
extern void AddSC_boss_kelris();
extern void AddSC_boss_aku_mai();
extern void AddSC_instance_blackfathom_deeps();

//Blackrock Depths
extern void AddSC_blackrock_depths();
extern void AddSC_boss_ambassador_flamelash();
extern void AddSC_boss_anubshiah();
extern void AddSC_boss_draganthaurissan();
extern void AddSC_boss_general_angerforge();
extern void AddSC_boss_gorosh_the_dervish();
extern void AddSC_boss_grizzle();
extern void AddSC_boss_high_interrogator_gerstahn();
extern void AddSC_boss_magmus();
extern void AddSC_boss_moira_bronzebeard();
extern void AddSC_boss_tomb_of_seven();
extern void AddSC_instance_blackrock_depths();

//Blackrock Spire
extern void AddSC_boss_drakkisath();
extern void AddSC_boss_halycon();
extern void AddSC_boss_highlordomokk();
extern void AddSC_boss_mothersmolderweb();
extern void AddSC_boss_overlordwyrmthalak();
extern void AddSC_boss_shadowvosh();
extern void AddSC_boss_thebeast();
extern void AddSC_boss_warmastervoone();
extern void AddSC_boss_quatermasterzigris();
extern void AddSC_boss_pyroguard_emberseer();
extern void AddSC_boss_gyth();
extern void AddSC_boss_rend_blackhand();

//Blackwing lair
extern void AddSC_boss_razorgore();
extern void AddSC_boss_vael();
extern void AddSC_boss_broodlord();
extern void AddSC_boss_firemaw();
extern void AddSC_boss_ebonroc();
extern void AddSC_boss_flamegor();
extern void AddSC_boss_chromaggus();
extern void AddSC_boss_nefarian();
extern void AddSC_boss_victor_nefarius();

//Blade's Edge Mountains
extern void AddSC_blades_edge_mountains();
extern void AddSC_shartuul_event();

//Blasted lands
extern void AddSC_boss_kruul();
extern void AddSC_blasted_lands();

//Bloodmyst Isle
extern void AddSC_bloodmyst_isle();

//Burning steppes
extern void AddSC_burning_steppes();

//Caverns of Time
//--Battle for Mt. Hyjal
extern void AddSC_hyjal();
extern void AddSC_boss_archimonde();
extern void AddSC_instance_mount_hyjal();
extern void AddSC_hyjal_trash();
extern void AddSC_boss_rage_winterchill();
extern void AddSC_boss_anetheron();
extern void AddSC_boss_kazrogal();
extern void AddSC_boss_azgalor();

//--Old Hillsbrad
extern void AddSC_boss_captain_skarloc();
extern void AddSC_boss_epoch_hunter();
extern void AddSC_boss_lieutenant_drake();
extern void AddSC_instance_old_hillsbrad();
extern void AddSC_old_hillsbrad();

//--The Dark Portal
extern void AddSC_boss_aeonus();
extern void AddSC_boss_chrono_lord_deja();
extern void AddSC_boss_temporus();
extern void AddSC_dark_portal();
extern void AddSC_instance_dark_portal();

//Coilfang Resevoir
//--Serpent Shrine Cavern
extern void AddSC_boss_fathomlord_karathress();
extern void AddSC_boss_hydross_the_unstable();
extern void AddSC_boss_lady_vashj();
extern void AddSC_boss_leotheras_the_blind();
extern void AddSC_boss_morogrim_tidewalker();
extern void AddSC_instance_serpentshrine_cavern();
extern void AddSC_boss_the_lurker_below();

//--Slave Pens
extern void AddSC_boss_ahune();
extern void AddSC_instance_slave_pens();
extern void AddSC_slave_pens();

//--Steam Vault
extern void AddSC_boss_hydromancer_thespia();
extern void AddSC_boss_mekgineer_steamrigger();
extern void AddSC_boss_warlord_kalithresh();
extern void AddSC_instance_steam_vault();

//--Underbog
extern void AddSC_boss_hungarfen();
extern void AddSC_boss_the_black_stalker();

//Darkshore
extern void AddSC_darkshore();
//Darnassus
//Deadmines
extern void AddSC_instance_deadmines();

//Deadwind pass
//Desolace
extern void AddSC_desolace();

//Dire Maul
extern void AddSC_instance_dire_maul();
extern void AddSC_dire_maul();
extern void AddSC_boss_pusillin();

//Dun Morogh
extern void AddSC_dun_morogh();

//Durotar
extern void AddSC_durotar();

//Duskwood
//Dustwallow marsh
extern void AddSC_dustwallow_marsh();

//Eversong Woods
extern void AddSC_eversong_woods();

//Exodar
//Eastern Plaguelands
extern void AddSC_eastern_plaguelands();

//Elwynn Forest
extern void AddSC_elwynn_forest();

//Felwood
extern void AddSC_felwood();

//Feralas
extern void AddSC_feralas();

//Ghostlands
extern void AddSC_ghostlands();

//Gnomeregan
//Gruul's Lair
extern void AddSC_boss_gruul();
extern void AddSC_boss_high_king_maulgar();
extern void AddSC_instance_gruuls_lair();

//Hellfire Citadel
//--Blood Furnace
extern void AddSC_boss_broggok();
extern void AddSC_boss_kelidan_the_breaker();
extern void AddSC_boss_the_maker();
extern void AddSC_instance_blood_furnace();

//--Magtheridon's Lair
extern void AddSC_boss_magtheridon();
extern void AddSC_instance_magtheridons_lair();

//--Shattered Halls
extern void AddSC_boss_grand_warlock_nethekurse();
extern void AddSC_boss_warbringer_omrogg();
extern void AddSC_boss_warchief_kargath_bladefist();
extern void AddSC_instance_shattered_halls();

//--Ramparts
extern void AddSC_boss_watchkeeper_gargolmar();
extern void AddSC_boss_omor_the_unscarred();
extern void AddSC_boss_vazruden_the_herald();

//Hellfire Peninsula
extern void AddSC_boss_doomlordkazzak();
extern void AddSC_hellfire_peninsula();

//Hillsbrad Foothills
//Hinterlands
extern void AddSC_hinterlands();

//Ironforge
extern void AddSC_ironforge();

//Isle of Quel'Danas
extern void AddSC_isle_of_queldanas();

//Karazhan
extern void AddSC_boss_attumen();
extern void AddSC_boss_curator();
extern void AddSC_boss_maiden_of_virtue();
extern void AddSC_boss_shade_of_aran();
extern void AddSC_boss_malchezaar();
extern void AddSC_boss_terestian_illhoof();
extern void AddSC_boss_moroes();
extern void AddSC_bosses_opera();
extern void AddSC_boss_netherspite();
extern void AddSC_instance_karazhan();
extern void AddSC_karazhan();
extern void AddSC_boss_nightbane();
extern void AddSC_chess_event();

//Loch Modan
extern void AddSC_loch_modan();

//Lower Blackrock Spire
extern void AddSC_instance_blackrock_spire();

// Magister's Terrace
extern void AddSC_boss_felblood_kaelthas();
extern void AddSC_boss_selin_fireheart();
extern void AddSC_boss_vexallus();
extern void AddSC_boss_priestess_delrissa();
extern void AddSC_instance_magisters_terrace();

//Maraudon
extern void AddSC_boss_celebras_the_cursed();
extern void AddSC_boss_landslide();
extern void AddSC_boss_noxxion();
extern void AddSC_boss_ptheradras();

//Molten core
extern void AddSC_boss_lucifron();
extern void AddSC_boss_magmadar();
extern void AddSC_boss_gehennas();
extern void AddSC_boss_garr();
extern void AddSC_boss_baron_geddon();
extern void AddSC_boss_shazzrah();
extern void AddSC_boss_golemagg();
extern void AddSC_boss_sulfuron();
extern void AddSC_boss_majordomo();
extern void AddSC_boss_ragnaros();
extern void AddSC_instance_molten_core();
extern void AddSC_molten_core();

//Moonglade
extern void AddSC_moonglade();

//Mulgore
extern void AddSC_mulgore();

//Nagrand
extern void AddSC_nagrand();

//Naxxramas
extern void AddSC_boss_anubrekhan();
extern void AddSC_boss_maexxna();
extern void AddSC_boss_patchwerk();
extern void AddSC_boss_razuvious();
extern void AddSC_boss_highlord_mograine();
extern void AddSC_boss_kelthuzad();
extern void AddSC_boss_faerlina();
extern void AddSC_boss_loatheb();
extern void AddSC_boss_noth();
extern void AddSC_boss_gluth();
extern void AddSC_boss_sapphiron();
extern void AddSC_boss_four_horsemen();

//Netherstorm
extern void AddSC_netherstorm();

//Onyxia's Lair
extern void AddSC_boss_onyxia();

//Orgrimmar
extern void AddSC_orgrimmar();

//Ragefire Chasm
//Razorfen Downs
extern void AddSC_boss_tuten_kash();
extern void AddSC_boss_plaguemaw_the_rotting();
extern void AddSC_boss_ragglesnout();
extern void AddSC_boss_glutton();
extern void AddSC_boss_mordresh_fire_eye();
extern void AddSC_boss_amnennar_the_coldbringer();
extern void AddSC_razorfen_downs();
extern void AddSC_instance_razorfen_downs();

//Razorfen Kraul
extern void AddSC_razorfen_kraul();
extern void AddSC_instance_razorfen_kraul();

//Redridge Mountains
extern void AddSC_redridge_mountains();

//Ruins of Ahn'Qiraj
extern void AddSC_boss_kurinnaxx();
extern void AddSC_boss_rajaxx();
extern void AddSC_boss_moam();
extern void AddSC_boss_buru();
extern void AddSC_boss_ayamiss();
extern void AddSC_boss_ossirian();
extern void AddSC_instance_ruins_of_ahnqiraj();
//Scarlet Monastery
extern void AddSC_boss_arcanist_doan();
extern void AddSC_boss_azshir_the_sleepless();
extern void AddSC_boss_bloodmage_thalnos();
extern void AddSC_boss_headless_horseman();
extern void AddSC_boss_herod();
extern void AddSC_boss_high_inquisitor_fairbanks();
extern void AddSC_boss_houndmaster_loksey();
extern void AddSC_boss_interrogator_vishas();
extern void AddSC_boss_scorn();
extern void AddSC_instance_scarlet_monastery();
extern void AddSC_boss_mograine_and_whitemane();

//Scholomance
extern void AddSC_boss_darkmaster_gandling();
extern void AddSC_boss_death_knight_darkreaver();
extern void AddSC_boss_theolenkrastinov();
extern void AddSC_boss_illuciabarov();
extern void AddSC_boss_instructormalicia();
extern void AddSC_boss_jandicebarov();
extern void AddSC_boss_kormok();
extern void AddSC_boss_lordalexeibarov();
extern void AddSC_boss_lorekeeperpolkelt();
extern void AddSC_boss_rasfrost();
extern void AddSC_boss_theravenian();
extern void AddSC_boss_vectus();
extern void AddSC_instance_scholomance();
extern void AddSC_scholomance();

//Searing gorge
extern void AddSC_searing_gorge();

//Shadowfang keep
extern void AddSC_shadowfang_keep();
extern void AddSC_instance_shadowfang_keep();

//Shadowmoon Valley
extern void AddSC_boss_doomwalker();
extern void AddSC_shadowmoon_valley();

//Shattrath
extern void AddSC_shattrath_city();

//Silithus
extern void AddSC_silithus();

//Silvermoon
extern void AddSC_silvermoon_city();

//Silverpine forest
extern void AddSC_silverpine_forest();

//Stockade
//Stonetalon mountains
extern void AddSC_stonetalon_mountains();

//Stormwind City
extern void AddSC_stormwind_city();

//Stranglethorn Vale
extern void AddSC_stranglethorn_vale();

//Stratholme
extern void AddSC_boss_magistrate_barthilas();
extern void AddSC_boss_maleki_the_pallid();
extern void AddSC_boss_nerubenkan();
extern void AddSC_boss_cannon_master_willey();
extern void AddSC_boss_baroness_anastari();
extern void AddSC_boss_ramstein_the_gorger();
extern void AddSC_boss_timmy_the_cruel();
extern void AddSC_boss_postmaster_malown();
extern void AddSC_boss_baron_rivendare();
extern void AddSC_boss_dathrohan_balnazzar();
extern void AddSC_boss_order_of_silver_hand();
extern void AddSC_instance_stratholme();
extern void AddSC_stratholme();

//Sunken Temple
extern void AddSC_instance_sunken_temple();
extern void AddSC_sunken_temple();

//Sunwell Plateau
extern void AddSC_instance_sunwell_plateau();
extern void AddSC_boss_kalecgos();
extern void AddSC_boss_brutallus();
extern void AddSC_boss_felmyst();
extern void AddSC_boss_eredar_twins();
extern void AddSC_boss_muru();
extern void AddSC_boss_kiljaeden_new();
extern void AddSC_sunwell_plateau();

//Swamp of Sorrows
extern void AddSC_swamp_of_sorrows();

//Tanaris
extern void AddSC_tanaris();

//Teldrassil
extern void AddSC_teldrassil();

//Tempest Keep
//--Arcatraz
extern void AddSC_arcatraz();
extern void AddSC_boss_harbinger_skyriss();
extern void AddSC_instance_arcatraz();

//--Botanica
extern void AddSC_boss_high_botanist_freywinn();
extern void AddSC_boss_laj();
extern void AddSC_boss_warp_splinter();

//--The Eye
extern void AddSC_boss_alar();
extern void AddSC_boss_kaelthas();
extern void AddSC_boss_void_reaver();
extern void AddSC_boss_high_astromancer_solarian();
extern void AddSC_instance_the_eye();
extern void AddSC_the_eye();

//--The Mechanar
extern void AddSC_boss_mechanolord_capacitus();
extern void AddSC_boss_gatewatcher_iron_hand();
extern void AddSC_boss_nethermancer_sepethrea();
extern void AddSC_boss_pathaleon_the_calculator();
extern void AddSC_instance_mechanar();
extern void AddSC_mechanar();

//Temple of ahn'qiraj
extern void AddSC_boss_cthun();
extern void AddSC_boss_fankriss();
extern void AddSC_boss_huhuran();
extern void AddSC_bug_trio();
extern void AddSC_boss_sartura();
extern void AddSC_boss_skeram();
extern void AddSC_boss_twinemperors();
extern void AddSC_mob_anubisath_sentinel();
extern void AddSC_instance_temple_of_ahnqiraj();
extern void AddSC_boss_ouro();

//Terokkar Forest
extern void AddSC_terokkar_forest();

//Thousand Needles
extern void AddSC_thousand_needles();

//Thunder Bluff
extern void AddSC_thunder_bluff();

//The Balance of Light and Shadow - Epic priest quest
extern void AddSC_the_balance_of_light_and_shadow();

//Tirisfal Glades
extern void AddSC_tirisfal_glades();

//Uldaman
extern void AddSC_boss_archaedas();
extern void AddSC_boss_ironaya();
extern void AddSC_uldaman();
extern void AddSC_instance_uldaman();

//Undercity
extern void AddSC_undercity();

//Un'Goro Crater
extern void AddSC_ungoro_crater();

//Upper blackrock spire
//Wailing caverns
extern void AddSC_instance_wailing_caverns();
extern void AddSC_wailing_caverns();

//Western plaguelands
extern void AddSC_western_plaguelands();

//Westfall
extern void AddSC_westfall();

//Wetlands
extern void AddSC_wetlands();

//Winterspring
extern void AddSC_winterspring();

//Zangarmarsh
extern void AddSC_zangarmarsh();

//Zul'Farrak
extern void AddSC_zulfarrak();
extern void AddSC_instance_zulfarrak();

//Zul'Gurub
extern void AddSC_boss_jeklik();
extern void AddSC_boss_venoxis();
extern void AddSC_boss_marli();
extern void AddSC_boss_mandokir();
extern void AddSC_boss_gahzranka();
extern void AddSC_boss_thekal();
extern void AddSC_boss_arlokk();
extern void AddSC_boss_jindo();
extern void AddSC_boss_hakkar();
extern void AddSC_boss_grilek();
extern void AddSC_boss_hazzarah();
extern void AddSC_boss_renataki();
extern void AddSC_boss_wushoolay();
extern void AddSC_instance_zulgurub();

//Zul'Aman
extern void AddSC_boss_akilzon();
extern void AddSC_boss_halazzi();
extern void AddSC_boss_hex_lord_malacrass();
extern void AddSC_boss_janalai();
extern void AddSC_boss_nalorakk();
extern void AddSC_boss_zuljin();
extern void AddSC_instance_zulaman();
extern void AddSC_zulaman();

// -------------------
void ScriptMgr::LoadDatabase()
{
    //***Preform all DB queries here***
    QueryResult result;

    // Drop Existing Text Map, only done once and we are ready to add data from multiple sources.
    TextMap.clear();

    // Load Script Text
    TC_LOG_INFO("server.loading","TSCR: Loading Script Texts...");
    LoadTrinityStrings(WorldDatabase,"script_texts",TEXT_SOURCE_RANGE,1+(TEXT_SOURCE_RANGE*2));

    // Gather Additional data from Script Texts
    result = WorldDatabase.PQuery("SELECT entry, sound, type, language, emote FROM script_texts");

    TC_LOG_INFO("server.loading","TSCR: Loading Script Texts additional data...");
    if (result)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();
            StringTextData temp;

            int32 i             = fields[0].GetInt32();
            temp.SoundId        = fields[1].GetInt32();
            temp.Type           = fields[2].GetInt8();
            temp.Language       = fields[3].GetInt8();
            temp.Emote          = fields[4].GetInt16();

            if (i >= 0)
            {
                error_db_log("TSCR: Entry %i in table `script_texts` is not a negative value.",i);
                continue;
            }

            if (i > TEXT_SOURCE_RANGE || i <= TEXT_SOURCE_RANGE*2)
            {
                error_db_log("TSCR: Entry %i in table `script_texts` is out of accepted entry range for table.",i);
                continue;
            }

            if (temp.SoundId)
            {
                if (!GetSoundEntriesStore()->LookupEntry(temp.SoundId))
                    error_db_log("TSCR: Entry %i in table `script_texts` has soundId %u but sound does not exist.",i,temp.SoundId);
            }

            if (!GetLanguageDescByID(temp.Language))
                error_db_log("TSCR: Entry %i in table `script_texts` using Language %u but Language does not exist.",i,temp.Language);

            if (temp.Type >= CHAT_TYPE_END)
                error_db_log("TSCR: Entry %i in table `script_texts` has Type %u but this Chat Type does not exist.",i,temp.Type);

            TextMap[i] = temp;
            ++count;
        } while (result->NextRow());

        TC_LOG_INFO("FIXME","\n>> TSCR: Loaded %u additional Script Texts data.", count);
    }else
    {
        TC_LOG_INFO("FIXME","\n>> Loaded 0 additional Script Texts data. DB table `script_texts` is empty.");
    }
}

ScriptMgr::ScriptMgr()
{
    
}

ScriptMgr::~ScriptMgr()
{
    ClearScripts();
}

void ScriptMgr::ClearScripts()
{
    if(spellSummary)
        delete[] spellSummary;

    //in reload case, delete all previous scripts
    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        if (m_scripts[i])
        {
            delete m_scripts[i];
            m_scripts[i] = nullptr;
        }
    }

    TC_LOG_INFO("scripts", "\n>> ScriptMgr: Cleared all core scripts.");
}

void ScriptMgr::ScriptsInit(char const* cfg_file)
{
    bool CanLoadDB = true;
    
    //Trinity Script startup
    TC_LOG_INFO("server.loading"," _____     _       _ _         ____            _       _");
    TC_LOG_INFO("server.loading","|_   _| __(_)_ __ (_) |_ _   _/ ___|  ___ _ __(_)_ __ | |_ ");
    TC_LOG_INFO("server.loading","  | || '__| | '_ \\| | __| | | \\___ \\ / __| \'__| | \'_ \\| __|");
    TC_LOG_INFO("server.loading","  | || |  | | | | | | |_| |_| |___) | (__| |  | | |_) | |_ ");
    TC_LOG_INFO("server.loading","  |_||_|  |_|_| |_|_|\\__|\\__, |____/ \\___|_|  |_| .__/ \\__|");
    TC_LOG_INFO("server.loading","                          |___/                  |_|        ");
    TC_LOG_INFO("server.loading","Trinity Script initializing %s\n", _FULLVERSION);

    TC_LOG_INFO("server.loading"," ");

    //Load database (must be called after TScriptConfig.SetSource). In case it failed, no need to even try load.
    if (CanLoadDB)
        LoadDatabase();

    TC_LOG_INFO("server.loading","TSCR: Loading C++ scripts\n");

    ClearScripts();

    sScriptMgr->FillSpellSummary();

    // -- Scripts to be added --

    // -- Areatrigger --
    AddSC_areatrigger_scripts();

    // -- Outdoors Dragons --
    AddSC_boss_dragonsofnightmare();

    // -- Creature --
    AddSC_generic_creature();

    // -- Custom --
    AddSC_npc_rez();
    AddSC_training_dummy();
    AddSC_zone_silence();
    AddSC_custom_example();
    AddSC_custom_gossip_codebox();
    AddSC_onevents();
    AddSC_npc_lottery();
    AddSC_theinform();
    AddSC_mylittlebombling();
    AddSC_firework_controller();
    AddSC_npc_interpreter();
    AddSC_custom_gnominizer();
    AddSC_pack58();

    AddSC_npc_teleporter();
    AddSC_npc_teleporter_pvpzone();
    AddSC_catapultmaster();

    // -- GO --
    AddSC_go_scripts();

    // -- Guard --
    AddSC_guards();

    // -- Honor --

    // -- Item --
    AddSC_item_scripts();
    AddSC_item_test();

    // -- NPC --
    AddSC_npc_professions();
    AddSC_npcs_special();
    AddSC_npc_xp_blocker();

    // -- Servers --
    AddSC_SmartSCripts();
    
    // -- Misc --
    AddSC_arena_spectator_script();
    AddSC_hallows_end();
    AddSC_arenabeastmaster();

    //--------------------
    //------ ZONE --------

    //Alterac Mountains
    AddSC_alterac_mountains();
    
    //Alterac Valley
    AddSC_AV_Warmasters();
    AddSC_boss_balinda();
    AddSC_boss_drekthar();
    AddSC_boss_galvangar();
    AddSC_boss_vanndar();
    AddSC_alterac_bowman();

    //Arathi Highlands
    AddSC_arathi_highlands();

    //Ashenvale Forest
    AddSC_ashenvale();

    //Aunchindoun
    //--Auchenai Crypts
    AddSC_boss_exarch_maladaar();
    AddSC_boss_shirrak_the_dead_watcher();

    //--Mana Tombs
    AddSC_boss_nexusprince_shaffar();
    AddSC_boss_pandemonius();

    //--Sekketh Halls
    AddSC_boss_darkweaver_syth();
    AddSC_boss_talon_king_ikiss();
    AddSC_boss_anzu();
    AddSC_instance_sethekk_halls();
    AddSC_sethekk_halls();

    //--Shadow Labyrinth
    AddSC_boss_ambassador_hellmaw();
    AddSC_boss_blackheart_the_inciter();
    AddSC_boss_grandmaster_vorpil();
    AddSC_boss_murmur();
    AddSC_instance_shadow_labyrinth();

    //Azshara
    AddSC_boss_azuregos();
    AddSC_azshara();

    //Azuremyst Isle
    AddSC_azuremyst_isle();

    //Badlands
    //Barrens
    AddSC_the_barrens();

    //Black Temple
    AddSC_black_temple();
    AddSC_boss_illidan();
    AddSC_boss_shade_of_akama();
    AddSC_boss_supremus();
    AddSC_boss_gurtogg_bloodboil();
    AddSC_boss_mother_shahraz();
    AddSC_boss_reliquary_of_souls();
    AddSC_boss_teron_gorefiend();
    AddSC_boss_najentus();
    AddSC_boss_illidari_council();
    AddSC_instance_black_temple();

    //Blackfathom Depths
    AddSC_blackfathom_deeps();
    AddSC_boss_gelihast();
    AddSC_boss_kelris();
    AddSC_boss_aku_mai();
    AddSC_instance_blackfathom_deeps();

    //Blackrock Depths
    AddSC_blackrock_depths();
    AddSC_boss_ambassador_flamelash();
    AddSC_boss_anubshiah();
    AddSC_boss_draganthaurissan();
    AddSC_boss_general_angerforge();
    AddSC_boss_gorosh_the_dervish();
    AddSC_boss_grizzle();
    AddSC_boss_high_interrogator_gerstahn();
    AddSC_boss_magmus();
    AddSC_boss_moira_bronzebeard();
    AddSC_boss_tomb_of_seven();
    AddSC_instance_blackrock_depths();

    //Blackrock Spire
    AddSC_boss_drakkisath();
    AddSC_boss_halycon();
    AddSC_boss_highlordomokk();
    AddSC_boss_mothersmolderweb();
    AddSC_boss_overlordwyrmthalak();
    AddSC_boss_shadowvosh();
    AddSC_boss_thebeast();
    AddSC_boss_warmastervoone();
    AddSC_boss_quatermasterzigris();
    AddSC_boss_pyroguard_emberseer();
    AddSC_boss_gyth();
    AddSC_boss_rend_blackhand();
    AddSC_instance_blackrock_spire();

    //Blackwing lair
    AddSC_boss_razorgore();
    AddSC_boss_vael();
    AddSC_boss_broodlord();
    AddSC_boss_firemaw();
    AddSC_boss_ebonroc();
    AddSC_boss_flamegor();
    AddSC_boss_chromaggus();
    AddSC_boss_nefarian();
    AddSC_boss_victor_nefarius();

    //Blade's Edge Mountains
    AddSC_blades_edge_mountains();
    AddSC_shartuul_event();

    //Blasted lands
    AddSC_boss_kruul();
    AddSC_blasted_lands();

    //Bloodmyst Isle
    AddSC_bloodmyst_isle();

    //Burning steppes
    AddSC_burning_steppes();

    //Caverns of Time
    //--Battle for Mt. Hyjal
    AddSC_hyjal();
    AddSC_boss_archimonde();
    AddSC_instance_mount_hyjal();
    AddSC_hyjal_trash();
    AddSC_boss_rage_winterchill();
    AddSC_boss_anetheron();
    AddSC_boss_kazrogal();
    AddSC_boss_azgalor();

    //--Old Hillsbrad
    AddSC_boss_captain_skarloc();
    AddSC_boss_epoch_hunter();
    AddSC_boss_lieutenant_drake();
    AddSC_instance_old_hillsbrad();
    AddSC_old_hillsbrad();

    //--The Dark Portal
    AddSC_boss_aeonus();
    AddSC_boss_chrono_lord_deja();
    AddSC_boss_temporus();
    AddSC_dark_portal();
    AddSC_instance_dark_portal();

    //Coilfang Resevoir
    //--Serpent Shrine Cavern
    AddSC_boss_fathomlord_karathress();
    AddSC_boss_hydross_the_unstable();
    AddSC_boss_lady_vashj();
    AddSC_boss_leotheras_the_blind();
    AddSC_boss_morogrim_tidewalker();
    AddSC_instance_serpentshrine_cavern();
    AddSC_boss_the_lurker_below();

    //--Slave Pens
    AddSC_boss_ahune();
    AddSC_instance_slave_pens();
    AddSC_slave_pens();
    
    //--Steam Vault
    AddSC_boss_hydromancer_thespia();
    AddSC_boss_mekgineer_steamrigger();
    AddSC_boss_warlord_kalithresh();
    AddSC_instance_steam_vault();

    //--Underbog
    AddSC_boss_hungarfen();
    AddSC_boss_the_black_stalker();

    //Darkshore
    AddSC_darkshore();
    //Darnassus
    //Deadmines
    AddSC_instance_deadmines();

    //Deadwind pass
    //Desolace
    AddSC_desolace();
    
    //Dire Maul
    AddSC_instance_dire_maul();
    AddSC_dire_maul();
    AddSC_boss_pusillin();
    
    //Dun Morogh
    AddSC_dun_morogh();

    //Durotar
    AddSC_durotar();
    
    //Duskwood
    //Dustwallow marsh
    AddSC_dustwallow_marsh();

    //Eversong Woods
    AddSC_eversong_woods();

    //Exodar
    //Eastern Plaguelands
    AddSC_eastern_plaguelands();

    //Elwynn Forest
    AddSC_elwynn_forest();

    //Felwood
    AddSC_felwood();

    //Feralas
    AddSC_feralas();

    //Ghostlands
    AddSC_ghostlands();

    //Gnomeregan
    //Gruul's Lair
    AddSC_boss_gruul();
    AddSC_boss_high_king_maulgar();
    AddSC_instance_gruuls_lair();

    //Hellfire Citadel
    //--Blood Furnace
    AddSC_boss_broggok();
    AddSC_boss_kelidan_the_breaker();
    AddSC_boss_the_maker();
    AddSC_instance_blood_furnace();

    //--Magtheridon's Lair
    AddSC_boss_magtheridon();
    AddSC_instance_magtheridons_lair();

    //--Shattered Halls
    AddSC_boss_grand_warlock_nethekurse();
    AddSC_boss_warbringer_omrogg();
    AddSC_boss_warchief_kargath_bladefist();
    AddSC_instance_shattered_halls();

    //--Ramparts
    AddSC_boss_watchkeeper_gargolmar();
    AddSC_boss_omor_the_unscarred();
    AddSC_boss_vazruden_the_herald();

    //Hellfire Peninsula
    AddSC_boss_doomlordkazzak();
    AddSC_hellfire_peninsula();

    //Hillsbrad Foothills
    //Hinterlands
    AddSC_hinterlands();
    
    //Ironforge
    AddSC_ironforge();

    //Isle of Quel'Danas
    AddSC_isle_of_queldanas();

    //Karazhan
    AddSC_boss_attumen();
    AddSC_boss_curator();
    AddSC_boss_maiden_of_virtue();
    AddSC_boss_shade_of_aran();
    AddSC_boss_malchezaar();
    AddSC_boss_terestian_illhoof();
    AddSC_boss_moroes();
    AddSC_bosses_opera();
    AddSC_boss_netherspite();
    AddSC_instance_karazhan();
    AddSC_karazhan();
    AddSC_boss_nightbane();
    AddSC_chess_event();

    //Loch Modan
    AddSC_loch_modan();

    //Lower Blackrock Spire

    // Magister's Terrace
    AddSC_boss_felblood_kaelthas();
    AddSC_boss_selin_fireheart();
    AddSC_boss_vexallus();
    AddSC_boss_priestess_delrissa();
    AddSC_instance_magisters_terrace();

    //Maraudon
    AddSC_boss_celebras_the_cursed();
    AddSC_boss_landslide();
    AddSC_boss_noxxion();
    AddSC_boss_ptheradras();

    //Molten core
    AddSC_boss_lucifron();
    AddSC_boss_magmadar();
    AddSC_boss_gehennas();
    AddSC_boss_garr();
    AddSC_boss_baron_geddon();
    AddSC_boss_shazzrah();
    AddSC_boss_golemagg();
    AddSC_boss_sulfuron();
    AddSC_boss_majordomo();
    AddSC_boss_ragnaros();
    AddSC_instance_molten_core();
    AddSC_molten_core();

    //Moonglade
    AddSC_moonglade();

    //Mulgore
    AddSC_mulgore();

    //Nagrand
    AddSC_nagrand();

    //Naxxramas
    AddSC_boss_anubrekhan();
    AddSC_boss_maexxna();
    AddSC_boss_patchwerk();
    AddSC_boss_razuvious();
    AddSC_boss_highlord_mograine();
    AddSC_boss_kelthuzad();
    AddSC_boss_faerlina();
    AddSC_boss_loatheb();
    AddSC_boss_noth();
    AddSC_boss_gluth();
    AddSC_boss_sapphiron();
    AddSC_boss_four_horsemen();

    //Netherstorm
    AddSC_netherstorm();

    //Onyxia's Lair
    AddSC_boss_onyxia();

    //Orgrimmar
    AddSC_orgrimmar();

    //Ragefire Chasm
    //Razorfen Downs
    AddSC_boss_tuten_kash();
    AddSC_boss_plaguemaw_the_rotting();
    AddSC_boss_ragglesnout();
    AddSC_boss_glutton();
    AddSC_boss_mordresh_fire_eye();
    AddSC_instance_razorfen_downs();
    AddSC_boss_amnennar_the_coldbringer();
    AddSC_razorfen_downs();

    //Razorfen Kraul
    AddSC_razorfen_kraul();
    AddSC_instance_razorfen_kraul();

    //Redridge Mountains
    AddSC_redridge_mountains();
    
    //Ruins of Ahn'Qiraj
    AddSC_boss_kurinnaxx();
    AddSC_boss_rajaxx();
    AddSC_boss_moam();
    AddSC_boss_buru();
    AddSC_boss_ayamiss();
    AddSC_boss_ossirian();
    AddSC_instance_ruins_of_ahnqiraj();
    //Scarlet Monastery
    AddSC_boss_arcanist_doan();
    AddSC_boss_azshir_the_sleepless();
    AddSC_boss_bloodmage_thalnos();
    AddSC_boss_headless_horseman();
    AddSC_boss_herod();
    AddSC_boss_high_inquisitor_fairbanks();
    AddSC_boss_houndmaster_loksey();
    AddSC_boss_interrogator_vishas();
    AddSC_boss_scorn();
    AddSC_instance_scarlet_monastery();
    AddSC_boss_mograine_and_whitemane();

    //Scholomance
    AddSC_boss_darkmaster_gandling();
    AddSC_boss_death_knight_darkreaver();
    AddSC_boss_theolenkrastinov();
    AddSC_boss_illuciabarov();
    AddSC_boss_instructormalicia();
    AddSC_boss_jandicebarov();
    AddSC_boss_kormok();
    AddSC_boss_lordalexeibarov();
    AddSC_boss_lorekeeperpolkelt();
    AddSC_boss_rasfrost();
    AddSC_boss_theravenian();
    AddSC_boss_vectus();
    AddSC_instance_scholomance();
    AddSC_scholomance();

    //Searing gorge
    AddSC_searing_gorge();

    //Shadowfang keep
    AddSC_shadowfang_keep();
    AddSC_instance_shadowfang_keep();

    //Shadowmoon Valley
    AddSC_boss_doomwalker();
    AddSC_shadowmoon_valley();

    //Shattrath
    AddSC_shattrath_city();

    //Silithus
    AddSC_silithus();

    //Silvermoon
    AddSC_silvermoon_city();

    //Silverpine forest
    AddSC_silverpine_forest();

    //Stockade
    //Stonetalon mountains
    AddSC_stonetalon_mountains();

    //Stormwind City
    AddSC_stormwind_city();

    //Stranglethorn Vale
    AddSC_stranglethorn_vale();

    //Stratholme
    AddSC_boss_magistrate_barthilas();
    AddSC_boss_maleki_the_pallid();
    AddSC_boss_nerubenkan();
    AddSC_boss_cannon_master_willey();
    AddSC_boss_baroness_anastari();
    AddSC_boss_ramstein_the_gorger();
    AddSC_boss_timmy_the_cruel();
    AddSC_boss_postmaster_malown();
    AddSC_boss_baron_rivendare();
    AddSC_boss_dathrohan_balnazzar();
    AddSC_boss_order_of_silver_hand();
    AddSC_instance_stratholme();
    AddSC_stratholme();

    //Sunken Temple
    AddSC_instance_sunken_temple();
    AddSC_sunken_temple();

    //Sunwell Plateau
    AddSC_instance_sunwell_plateau();
    AddSC_boss_kalecgos();
    AddSC_boss_brutallus();
    AddSC_boss_felmyst();
    AddSC_boss_eredar_twins();
    AddSC_boss_muru();
    AddSC_boss_kiljaeden_new();
    AddSC_sunwell_plateau();
    
    //Swamp of Sorrows
    AddSC_swamp_of_sorrows();

    //Tanaris
    AddSC_tanaris();

    //Teldrassil
    AddSC_teldrassil();
    
    //Tempest Keep
    //--Arcatraz
    AddSC_arcatraz();
    AddSC_boss_harbinger_skyriss();
    AddSC_instance_arcatraz();

    //--Botanica
    AddSC_boss_high_botanist_freywinn();
    AddSC_boss_laj();
    AddSC_boss_warp_splinter();

    //--The Eye
    AddSC_boss_alar();
    AddSC_boss_kaelthas();
    AddSC_boss_void_reaver();
    AddSC_boss_high_astromancer_solarian();
    AddSC_instance_the_eye();
    AddSC_the_eye();

    //--The Mechanar
    AddSC_boss_mechanolord_capacitus();
    AddSC_boss_gatewatcher_iron_hand();
    AddSC_boss_nethermancer_sepethrea();
    AddSC_boss_pathaleon_the_calculator();
    AddSC_instance_mechanar();
    AddSC_mechanar();

    //Temple of ahn'qiraj
    AddSC_boss_cthun();
    AddSC_boss_fankriss();
    AddSC_boss_huhuran();
    AddSC_bug_trio();
    AddSC_boss_sartura();
    AddSC_boss_skeram();
    AddSC_boss_twinemperors();
    AddSC_mob_anubisath_sentinel();
    AddSC_instance_temple_of_ahnqiraj();
    AddSC_boss_ouro();

    //Terokkar Forest
    AddSC_terokkar_forest();

    //Thousand Needles
    AddSC_thousand_needles();

    //Thunder Bluff
    AddSC_thunder_bluff();

    // The Balance of Light and Shadow - Epic priest quest
    AddSC_the_balance_of_light_and_shadow();
    
    //Tirisfal Glades
    AddSC_tirisfal_glades();

    //Uldaman
    AddSC_boss_archaedas();
    AddSC_boss_ironaya();
    AddSC_uldaman();
    AddSC_instance_uldaman();

    //Undercity
    AddSC_undercity();

    //Un'Goro Crater
    AddSC_ungoro_crater();

    //Upper blackrock spire
    //Wailing caverns
    AddSC_instance_wailing_caverns();
    AddSC_wailing_caverns();

    //Western plaguelands
    AddSC_western_plaguelands();

    //Westfall
    AddSC_westfall();

    //Wetlands
    AddSC_wetlands();
    
    //Winterspring
    AddSC_winterspring();

    //Zangarmarsh
    AddSC_zangarmarsh();

    //Zul'Farrak
    AddSC_zulfarrak();
    AddSC_instance_zulfarrak();

    //Zul'Gurub
    AddSC_boss_jeklik();
    AddSC_boss_venoxis();
    AddSC_boss_marli();
    AddSC_boss_mandokir();
    AddSC_boss_gahzranka();
    AddSC_boss_thekal();
    AddSC_boss_arlokk();
    AddSC_boss_jindo();
    AddSC_boss_hakkar();
    AddSC_boss_grilek();
    AddSC_boss_hazzarah();
    AddSC_boss_renataki();
    AddSC_boss_wushoolay();
    AddSC_instance_zulgurub();

    //Zul'Aman
    AddSC_boss_akilzon();
    AddSC_boss_halazzi();
    AddSC_boss_hex_lord_malacrass();
    AddSC_boss_janalai();
    AddSC_boss_nalorakk();
    AddSC_boss_zuljin();
    AddSC_instance_zulaman();
    AddSC_zulaman();

    // -------------------

    TC_LOG_INFO("FIXME",">> Loaded %i C++ Scripts.", num_sc_scripts);

    TC_LOG_INFO("FIXME",">> Load Overriden SQL Data.");
    LoadOverridenSQLData();
    TC_LOG_INFO("FIXME",">> Load Overriden DBC Data.");
    LoadOverridenDBCData();
}

//*********************************
//*** Functions used globally ***

//send text from script_texts table. Only there for retrocompatibity, use Unit::Say/Yell/Whispers ... functions instead.
void DoScriptText(int32 textEntry, Unit* pSource, Unit* target)
{
    if (!pSource)
    {
        error_log("TSCR: DoScriptText entry %i, invalid Source pointer.",textEntry);
        return;
    }

    if (textEntry >= 0)
    {
        error_log("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    std::unordered_map<int32, StringTextData>::iterator i = TextMap.find(textEntry);

    if (i == TextMap.end())
    {
        error_log("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) could not find text entry %i.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    if((*i).second.SoundId)
    {
        if( GetSoundEntriesStore()->LookupEntry((*i).second.SoundId) )
        {
            pSource->SendPlaySound((*i).second.SoundId, false);
        }
        else
            error_log("TSCR: DoScriptText entry %i tried to process invalid sound id %u.",textEntry,(*i).second.SoundId);
    }

    if((*i).second.Emote)
    {
        if (pSource->GetTypeId() == TYPEID_UNIT || pSource->GetTypeId() == TYPEID_PLAYER)
        {
            ((Unit*)pSource)->HandleEmoteCommand((*i).second.Emote);
        }
        else
            error_log("TSCR: DoScriptText entry %i tried to process emote for invalid TypeId (%u).",textEntry,pSource->GetTypeId());
    }

    switch((*i).second.Type)
    {
        case CHAT_TYPE_SAY:
            pSource->old_Say(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_YELL:
            pSource->old_Yell(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            pSource->old_TextEmote(textEntry, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            pSource->old_TextEmote(textEntry, target ? target->GetGUID() : 0, true);
            break;
        case CHAT_TYPE_WHISPER:
            {
                if (target && target->GetTypeId() == TYPEID_PLAYER)
                    pSource->old_Whisper(textEntry, target->GetGUID());
                else error_log("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
            }break;
        case CHAT_TYPE_BOSS_WHISPER:
            {
                if (target && target->GetTypeId() == TYPEID_PLAYER)
                    pSource->old_Whisper(textEntry, target->GetGUID(), true);
                else error_log("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
            }break;
    }
}

//*********************************
//*** Functions used internally ***

void ScriptMgr::RegisterScript(Script*& script)
{
    int id = GetScriptId(script->Name.c_str());
    if (id)
    {
        m_scripts[id] = script;
        ++num_sc_scripts;
        return;
    }

    TC_LOG_WARN("scripts", "TrinityScript: RegisterScript, but script named %s does not have ScriptName assigned in database. Script has been removed from memory.", script->Name.c_str());

    delete script;
    script = nullptr;
}

//********************************
//*** Functions to be Exported ***


void ScriptMgr::OnMapUpdate(Map* map, uint32 diff)
{
    ASSERT(map);

    /*
    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnUpdate(map, diff);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnUpdate((InstanceMap*)map, diff);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnUpdate((BattlegroundMap*)map, diff);
    SCR_MAP_END;
    */
}

void ScriptMgr::OnAddPassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

   /* GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddPassenger(transport, player); */
}

void ScriptMgr::OnAddCreaturePassenger(Transport* transport, Creature* creature)
{
    ASSERT(transport);
    ASSERT(creature);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddCreaturePassenger(transport, creature);*/
}

void ScriptMgr::OnRemovePassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRemovePassenger(transport, player);*/
}

void ScriptMgr::OnTransportUpdate(Transport* transport, uint32 diff)
{
    ASSERT(transport);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(transport, diff);*/
}

void ScriptMgr::OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z)
{
/*    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRelocate(transport, waypointId, mapId, x, y, z); */
}

void ScriptMgr::OnPlayerLogin(Player *pPlayer, bool /* firstLogin */)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogin) 
        return;

    tmpscript->pOnLogin(pPlayer);
}

void ScriptMgr::OnSocketOpen(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    //FOREACH_SCRIPT(ServerScript)->OnSocketOpen(socket);
}

void ScriptMgr::OnSocketClose(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    //FOREACH_SCRIPT(ServerScript)->OnSocketClose(socket);
}

void ScriptMgr::OnPlayerSave(Player* player)
{
   // FOREACH_SCRIPT(PlayerScript)->OnSave(player);
}

void ScriptMgr::OnPasswordChange(uint32 accountId)
{
   // FOREACH_SCRIPT(AccountScript)->OnPasswordChange(accountId);
}

void ScriptMgr::OnPacketReceive(WorldSession* session, WorldPacket const& packet)
{
   /* if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketReceive(session, copy);*/
}

void ScriptMgr::OnPacketSend(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

   /* if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketSend(session, copy);*/
}


void ScriptMgr::OnNetworkStart()
{
 //   FOREACH_SCRIPT(ServerScript)->OnNetworkStart();
}

void ScriptMgr::OnNetworkStop()
{
  //  FOREACH_SCRIPT(ServerScript)->OnNetworkStop();
}

void ScriptMgr::OnEmailChange(uint32 accountId)
{
   // FOREACH_SCRIPT(AccountScript)->OnEmailChange(accountId);
}

void ScriptMgr::OnFailedEmailChange(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedEmailChange(accountId);
}

void ScriptMgr::OnFailedPasswordChange(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedPasswordChange(accountId);
}

void ScriptMgr::OnAccountLogin(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnAccountLogin(accountId);
}

void ScriptMgr::OnFailedAccountLogin(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedAccountLogin(accountId);
}

void ScriptMgr::OnPlayerLogout(Player *pPlayer)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogout) return;
    tmpscript->pOnLogout(pPlayer);
}


void ScriptMgr::OnPVPKill(Player *killer, Player *killed)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnPVPKill) return;
    tmpscript->pOnPVPKill(killer, killed);
}

void ScriptMgr::OnUnknownPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

    /*if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnUnknownPacketReceive(session, copy);*/
}

void ScriptMgr::OnPlayerCreate(Player* player)
{
    //FOREACH_SCRIPT(PlayerScript)->OnCreate(player);
}

char const* ScriptMgr::ScriptsVersion()
{
    return "Sunstrider scripting library";
}

bool ScriptMgr::OnGossipHello ( Player * player, Creature *_Creature )
{
    player->PlayerTalkClass->ClearMenus();
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if(!tmpscript || !tmpscript->pOnGossipHello)
        return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pOnGossipHello(player,_Creature);
    
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->getScriptName()];
    if(tmpscript2 && tmpscript2->pOnGossipHello) 
        return tmpscript2->pOnGossipHello(player,_Creature);
}


bool ScriptMgr::OnGossipSelect( Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if(!tmpscript || !tmpscript->pOnGossipSelect)
        return false;
    
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pOnGossipSelect(player,_Creature,sender,action);
     
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->getScriptName()];
    if(tmpscript2 && tmpscript2->pGossipSelect) 
        return tmpscript2->pGossipSelect(player,_Creature,sender,action);
 
    return false;
}


bool ScriptMgr::OnGossipSelectCode( Player *player, Creature *_Creature, uint32 sender, uint32 action, const char* sCode )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
     if(!tmpscript || !tmpscript->pOnGossipSelectCode)
         return false;
     
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pOnGossipSelectCode(player,_Creature,sender,action,sCode);
     
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->GetScriptName()];
    if(tmpscript2 && tmpscript2->pOnGossipSelectCode) 
        return tmpscript2->pOnGossipSelectCode(player,_Creature,sender,action,sCode);

    return false;
}


bool ScriptMgr::OnGossipSelect( Player *player, GameObject *_GO, uint32 sender, uint32 action )
{
    if(!_GO)
    return false;

    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if(!tmpscript || !tmpscript->pOnGossipSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOOnGossipSelect(player,_GO,sender,action);
}


bool ScriptMgr::OnGossipSelectCode( Player *player, GameObject *_GO, uint32 sender, uint32 action, const char* sCode )
{
    if(!_GO)
    return false;

    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if(!tmpscript || !tmpscript->pOnGossipSelectCode) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOOnGossipSelectCode(player,_GO,sender,action,sCode);
}


bool ScriptMgr::QuestAccept( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pOnQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pOnQuestAccept(player,_Creature,_Quest);
}


bool ScriptMgr::QuestSelect( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestSelect(player,_Creature,_Quest);
}


bool ScriptMgr::QuestComplete( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestComplete) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestComplete(player,_Creature,_Quest);
}


bool ScriptMgr::ChooseReward( Player *player, Creature *_Creature, Quest const *_Quest, uint32 opt )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pOnQuestReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pOnQuestReward(player,_Creature,_Quest,opt);
}


uint32 ScriptMgr::GetDialogStatus( Player *player, Creature *_Creature )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pGetDialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGetDialogStatus(player,_Creature);
}


uint32 ScriptMgr::GetDialogStatus( Player *player, GameObject *_GO )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGetDialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOGetDialogStatus(player,_GO);
}


bool ScriptMgr::ItemHello( Player *player, Item *_Item, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Item->GetTemplate()->ScriptId];
    if (!tmpscript || !tmpscript->pItemHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemHello(player,_Item,_Quest);
}


bool ScriptMgr::ItemQuestAccept( Player *player, Item *_Item, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Item->GetTemplate()->ScriptId];
    if (!tmpscript || !tmpscript->pItemQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemQuestAccept(player,_Item,_Quest);
}


bool ScriptMgr::OnGossipHello( Player *player, GameObject *_GO )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pOnGossipHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOOnGossipHello(player,_GO);
}


bool ScriptMgr::OnQuestAccept( Player *player, GameObject *_GO, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pOnQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOOnQuestAccept(player,_GO,_Quest);
}


bool ScriptMgr::OnQuestReward( Player *player, GameObject *_GO, Quest const *_Quest, uint32 opt )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pOnQuestReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOOnQuestReward(player,_GO,_Quest,opt);
}

bool ScriptMgr::OnUse(Player* player, GameObject* go)
{
    Script *tmpscript = m_scripts[go->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGoOnUse) 
        return false;

    return tmpscript->pGoOnUse(player,go);
}

bool ScriptMgr::AreaTrigger( Player *player, AreaTriggerEntry const* atEntry)
{
    Script* tmpscript = m_scripts[GetAreaTriggerScriptId(atEntry->id)];
    if (!tmpscript || !tmpscript->pAreaTrigger) return false;

    return tmpscript->pAreaTrigger(player, atEntry);
}

CreatureAI* ScriptMgr::GetAI(Creature *_Creature, uint32 scriptId)
{
    if (scriptId >= MAX_SCRIPTS)
    {
        TC_LOG_ERROR("scripts", "ScriptMgr::GetAI called with scriptId %u. Max id is %u", scriptId, MAX_SCRIPTS);
        return nullptr;
    }

    if (scriptId == 0)
        scriptId = _Creature->GetScriptId();

    Script* tmpscript = m_scripts[scriptId];
    if (!tmpscript || !tmpscript->GetAI) 
        return nullptr;
    
    return tmpscript->GetAI(_Creature);
}

CreatureAINew* ScriptMgr::getAINew(Creature* creature)
{
    CreatureScriptMap::const_iterator iter = m_creatureScripts.find(creature->getScriptName().c_str());
    if (iter == m_creatureScripts.end())
        return NULL;
    
    return iter->second->getAI(creature);
}

bool ScriptMgr::ItemUse( Player *player, Item* _Item, SpellCastTargets const& targets)
{
    Script *tmpscript = m_scripts[_Item->GetTemplate()->ScriptId];
    if (!tmpscript || !tmpscript->pItemUse) return false;

    return tmpscript->pItemUse(player,_Item,targets);
}


bool ScriptMgr::ReceiveEmote( Player *player, Creature *_Creature, uint32 emote )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pReceiveEmote) return false;

    return tmpscript->pReceiveEmote(player, _Creature, emote);
}

bool ScriptMgr::EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget)
{
    Script *tmpscript = m_scripts[crTarget->GetScriptId()];

    if (!tmpscript || !tmpscript->pEffectDummyCreature) return false;

    return tmpscript->pEffectDummyCreature(caster, spellId, effIndex, crTarget);
}

InstanceScript* ScriptMgr::CreateInstanceData(Map *map)
{
    if (!map->IsDungeon()) return NULL;

    Script *tmpscript = m_scripts[((InstanceMap*)map)->GetScriptId()];
    if (!tmpscript || !tmpscript->GetInstanceData) return NULL;

    return tmpscript->GetInstanceData(map);
}
