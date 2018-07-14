-- Delete now invalid pets (we changed the way we store their spell)
DELETE FROM character_pet;
DELETE FROM pet_aura;
DELETE FROM pet_spell;
DELETE FROM pet_spell_cooldown;