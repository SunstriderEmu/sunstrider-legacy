#ifndef DBCFMT_H
#define DBCFMT_H
 
char const AreaTableEntryfmt[]="niiiixxxxxissssssssssssssssxiiiiifx";
char const AuctionHouseEntryfmt[]="niiixxxxxxxxxxxxxxxxx";
char const AreaTriggerEntryfmt[]="niffffffff";
char const BankBagSlotPricesEntryfmt[]="ni";
char const BattlemasterListEntryfmt[]="niiixxxxxiiiixxssssssssssssssssxx";
#ifdef LICH_KING
char const CharSectionsEntryfmt[] = "diiixxxiii";
char const CharStartOutfitEntryfmt[]="diiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const CharacterFacialHairStylesfmt[] = "iiixxxxx";
#else
char const CharSectionsEntryfmt[] = "diiiiixxxi";
char const CharStartOutfitEntryfmt[] = "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx";
char const CharacterFacialHairStylesfmt[] = "iiixxxxxxxx";
#endif
char const CharTitlesEntryfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char const ChatChannelsEntryfmt[]="iixssssssssssssssssxxxxxxxxxxxxxxxxxx";
                                                            // ChatChannelsEntryfmt, index not used (more compact store)
char const ChrClassesEntryfmt[]="nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix";
char const ChrRacesEntryfmt[]="niixiixxixxxxissssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char const CinematicCameraEntryfmt[] = "nsiffff";
char const CinematicSequencesEntryfmt[] = "nxixxxxxxx";
char const CreatureDisplayInfofmt[]="nixifxxxxxxxxx";
char const CreatureDisplayInfoExtrafmt[]="diixxxxxxxxxxxxxxxxxx";
char const CreatureFamilyfmt[]="nfifiiiissssssssssssssssxx";
#ifdef LICH_KING
char const CreatureModelDatafmt[] ="nixxfxxxxxxxxxxffxxxxxxxxxxx";
char const Emotesfmt[] = "nxxiiixx";
char const LiquidTypefmt[] = "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const LightEntryfmt[] = "nifffxxxxxxxxxx";
#else
char const CreatureModelDatafmt[] = "nixxfxxxxxxxxxxffxxxxxxx";
char const Emotesfmt[] = "nxxiiix";
char const LiquidTypefmt[] = "nxii";
char const LightEntryfmt[] = "nifffxxxxxxx";
#endif
char const CreatureSpellDatafmt[]="nxxxxxxxx";
char const DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
char const DurabilityQualityfmt[]="nf";
char const EmoteEntryfmt[]="nxixxxxxxxxxxxxxxxx";
char const FactionEntryfmt[]="niiiiiiiiiiiiiiiiiissssssssssssssssxxxxxxxxxxxxxxxxxx";
char const FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
char const GameObjectDisplayInfofmt[]="nsxxxxxxxxxxffffff";
char const GemPropertiesEntryfmt[]="nixxi";
char const GtCombatRatingsfmt[]="f";
char const GtChanceToMeleeCritBasefmt[]="f";
char const GtChanceToMeleeCritfmt[]="f";
char const GtChanceToSpellCritBasefmt[]="f";
char const GtChanceToSpellCritfmt[]="f";
char const GtNPCManaCostScalerfmt[] = "f";
char const GtOCTRegenHPfmt[]="f";
//char const GtOCTRegenMPfmt[]="f";
char const GtRegenHPPerSptfmt[]="f";
char const GtRegenMPPerSptfmt[]="f";
char const Itemfmt[]="niii";
//char const ItemCondExtCostsEntryfmt[]="xiii";
char const ItemExtendedCostEntryfmt[]="niiiiiiiiiiiii";
char const ItemRandomPropertiesfmt[]="nxiiixxssssssssssssssssx";
char const ItemRandomSuffixfmt[]="nssssssssssssssssxxiiiiii";
char const ItemSetEntryfmt[]="dssssssssssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
char const LockEntryfmt[]="niiiiiiiiiiiiiiiiiixxxxxxxxxxxxxx";
char const MailTemplateEntryfmt[]="nssssssssssssssssxssssssssssssssssx";
char const MapEntryfmt[]="nxixssssssssssssssssxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiffiixxi";
char const PvPDifficultyfmt[]="fake for bc. Will be harcoded in core as static values";
char const QuestSortEntryfmt[]="nxxxxxxxxxxxxxxxxx";
char const RandomPropertiesPointsfmt[]="niiiiiiiiiiiiiii";
char const SkillLinefmt[]="nixssssssssssssssssxxxxxxxxxxxxxxxxxxi";
char const SkillLineAbilityfmt[]="niiiixxiiiiixxi";
char const SkillRaceClassInfofmt[] = "diiiixix";
char const SkillTiersfmt[] = "nxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiii";
char const SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const SpellCastTimefmt[]="nixx";
char const SpellCategoryfmt[] = "ni";
char const SpellDurationfmt[]="niii";
char const SpellEntryfmt[]="nixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffixiixssssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxxiiii";
char const SpellFocusObjectfmt[]="nxxxxxxxxxxxxxxxxx";
char const SpellItemEnchantmentfmt[]="niiiiiixxxiiissssssssssssssssxiiii";
char const SpellItemEnchantmentConditionfmt[]="nbbbbbxxxxxbbbbbbbbbbiiiiiXXXXX";
char const SpellRadiusfmt[]="nfxf";
char const SpellRangefmt[]="nffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const SpellShapeshiftfmt[]="nxxxxxxxxxxxxxxxxxxiixixxxxxxxxxxxx";
char const StableSlotPricesfmt[] = "ni";
char const SummonPropertiesfmt[] = "niiiii";
char const TalentEntryfmt[]="niiiiiiiixxxxixxixxxi";
char const TalentTabEntryfmt[]="nxxxxxxxxxxxxxxxxxxxiix";
char const TaxiNodesEntryfmt[]="nifffssssssssssssssssxii";
char const TaxiPathEntryfmt[]="niii";
char const TaxiPathNodeEntryfmt[]="diiifffiiii";
char const TotemCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
char const TransportAnimationfmt[] = "diifffx";
char const WorldMapAreaEntryfmt[]="xinxffffi";
char const WMOAreaTableEntryfmt[]="niiixxxxxiixxxxxxxxxxxxxxxxx";
char const WorldSafeLocsEntryfmt[]="nifffxxxxxxxxxxxxxxxxx";

#endif // DBCFMT_H