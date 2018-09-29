#ifndef DBCFMT_H
#define DBCFMT_H
 
char constexpr AreaTableEntryfmt[]="niiiixxxxxissssssssssssssssxiiiiifx";
char constexpr AuctionHouseEntryfmt[]="niiixxxxxxxxxxxxxxxxx";
char constexpr AreaTriggerEntryfmt[]="niffffffff";
char constexpr BankBagSlotPricesEntryfmt[]="ni";
char constexpr BattlemasterListEntryfmt[]="niiixxxxxiiiixxssssssssssssssssxx";
#ifdef LICH_KING
char constexpr CharSectionsEntryfmt[] = "diiixxxiii";
char constexpr CharStartOutfitEntryfmt[]="diiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char constexpr CharacterFacialHairStylesfmt[] = "iiixxxxx";
#else
char constexpr CharSectionsEntryfmt[] = "diiiiixxxi";
char constexpr CharStartOutfitEntryfmt[] = "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx";
char constexpr CharacterFacialHairStylesfmt[] = "iiixxxxxxxx";
#endif
char constexpr CharTitlesEntryfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char constexpr ChatChannelsEntryfmt[]="iixssssssssssssssssxxxxxxxxxxxxxxxxxx";
                                                            // ChatChannelsEntryfmt, index not used (more compact store)
char constexpr ChrClassesEntryfmt[]="nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix";
char constexpr ChrRacesEntryfmt[]="niixiixxixxxxissssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char constexpr CinematicCameraEntryfmt[] = "nsiffff";
char constexpr CinematicSequencesEntryfmt[] = "nxixxxxxxx";
char constexpr CreatureDisplayInfofmt[]="nixifxxxxxxxxx";
char constexpr CreatureDisplayInfoExtrafmt[]="diixxxxxxxxxxxxxxxxxx";
char constexpr CreatureFamilyfmt[]="nfifiiiissssssssssssssssxx";
#ifdef LICH_KING
char constexpr CreatureModelDatafmt[] = "nisxfxxxxxxxxxxffxxxxxxxxxxx";
char constexpr Emotesfmt[] = "nxxiiixx";
char constexpr LiquidTypefmt[] = "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char constexpr LightEntryfmt[] = "nifffxxxxxxxxxx";
#else
char constexpr CreatureModelDatafmt[] = "nisxfxxxxxxxxxxffxxxxxxx";
char constexpr Emotesfmt[] = "nxxiiix";
char constexpr LiquidTypefmt[] = "nxii";
char constexpr LightEntryfmt[] = "nifffxxxxxxx";
#endif
char constexpr CreatureSpellDatafmt[]="nxxxxxxxx";
char constexpr DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
char constexpr DurabilityQualityfmt[]="nf";
char constexpr EmoteEntryfmt[]="nxixxxxxxxxxxxxxxxx";
char constexpr FactionEntryfmt[]="niiiiiiiiiiiiiiiiiissssssssssssssssxxxxxxxxxxxxxxxxxx";
char constexpr FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
char constexpr GameObjectDisplayInfofmt[]="nsxxxxxxxxxxffffff";
char constexpr GemPropertiesEntryfmt[]="nixxi";
char constexpr GtCombatRatingsfmt[]="f";
char constexpr GtChanceToMeleeCritBasefmt[]="f";
char constexpr GtChanceToMeleeCritfmt[]="f";
char constexpr GtChanceToSpellCritBasefmt[]="f";
char constexpr GtChanceToSpellCritfmt[]="f";
char constexpr GtNPCManaCostScalerfmt[] = "f";
char constexpr GtOCTRegenHPfmt[]="f";
//char constexpr GtOCTRegenMPfmt[]="f";
char constexpr GtRegenHPPerSptfmt[]="f";
char constexpr GtRegenMPPerSptfmt[]="f";
char constexpr Itemfmt[]="niii";
//char constexpr ItemCondExtCostsEntryfmt[]="xiii";
char constexpr ItemExtendedCostEntryfmt[]="niiiiiiiiiiiii";
char constexpr ItemRandomPropertiesfmt[]="nxiiixxssssssssssssssssx";
char constexpr ItemRandomSuffixfmt[]="nssssssssssssssssxxiiiiii";
char constexpr ItemSetEntryfmt[]="dssssssssssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
char constexpr LockEntryfmt[]="niiiiiiiiiiiiiiiiiixxxxxxxxxxxxxx";
char constexpr MailTemplateEntryfmt[]="nssssssssssssssssxssssssssssssssssx";
char constexpr MapEntryfmt[]="nxixssssssssssssssssxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiffiixxi";
char constexpr PvPDifficultyfmt[]="fake for bc. Will be harcoded in core as static values";
char constexpr QuestSortEntryfmt[]="nxxxxxxxxxxxxxxxxx";
char constexpr RandomPropertiesPointsfmt[]="niiiiiiiiiiiiiii";
char constexpr SkillLinefmt[]="nixssssssssssssssssxxxxxxxxxxxxxxxxxxi";
char constexpr SkillLineAbilityfmt[]="niiiixxiiiiixxi";
char constexpr SkillRaceClassInfofmt[] = "diiiixix";
char constexpr SkillTiersfmt[] = "nxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiii";
char constexpr SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char constexpr SpellCastTimefmt[]="nixx";
char constexpr SpellCategoryfmt[] = "ni";
char constexpr SpellDurationfmt[]="niii";
char constexpr SpellEntryfmt[]="nixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffixiixssssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxxiiii";
char constexpr SpellFocusObjectfmt[]="nxxxxxxxxxxxxxxxxx";
char constexpr SpellItemEnchantmentfmt[]="niiiiiixxxiiissssssssssssssssxiiii";
char constexpr SpellItemEnchantmentConditionfmt[]="nbbbbbxxxxxbbbbbbbbbbiiiiiXXXXX";
char constexpr SpellRadiusfmt[]="nfxf";
char constexpr SpellRangefmt[]="nffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char constexpr SpellShapeshiftfmt[]="nxxxxxxxxxxxxxxxxxxiixixxxxxxxxxxxx";
char constexpr StableSlotPricesfmt[] = "ni";
char constexpr SummonPropertiesfmt[] = "niiiii";
char constexpr TalentEntryfmt[]="niiiiiiiixxxxixxixxxi";
char constexpr TalentTabEntryfmt[]="nxxxxxxxxxxxxxxxxxxxiix";
char constexpr TaxiNodesEntryfmt[]="nifffssssssssssssssssxii";
char constexpr TaxiPathEntryfmt[]="niii";
char constexpr TaxiPathNodeEntryfmt[]="diiifffiiii";
char constexpr TotemCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
char constexpr TransportAnimationfmt[] = "diifffx";
char constexpr WorldMapAreaEntryfmt[]="xinxffffi";
char constexpr WMOAreaTableEntryfmt[]="niiixxxxxiixxxxxxxxxxxxxxxxx";
char constexpr WorldSafeLocsEntryfmt[]="nifffxxxxxxxxxxxxxxxxx";

#endif // DBCFMT_H