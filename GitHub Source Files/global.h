﻿#include <string>

	//#define RES_1920X1080 1
	//#define RES_1600X900 1
	//#define RES_1366X768 1
	//#define RES_1280X720 1

#if RES_1920X1080

/* Resolution: 1920*1080
*/

const int X_OFFSET = 560;// 840;
const int Y_OFFSET = 170;// 255;
const int SPACING_FACTOR = 44;
const int MENU_SPACING = 45;
const float RES_ADJUST = 1.5;
const float HP_BAR_ADJUST = -130;
const float HP_ADJUST = -14;
const float SP_ADJUST = -14;

#elif RES_1600X900

const int X_OFFSET = 560;
const int Y_OFFSET = 170;
const int SPACING_FACTOR = 37;
const int MENU_SPACING = 37;
const float RES_ADJUST = 1.23;
const float HP_BAR_ADJUST = -102;
const float HP_ADJUST = -14;
const float SP_ADJUST = -14;

#elif RES_1366X768

const int X_OFFSET = 560;
const int Y_OFFSET = 170;
const int SPACING_FACTOR = 60;
const int MENU_SPACING = 60;
const float RES_ADJUST = 1.08f;
const float HP_BAR_ADJUST = -2;
const float HP_ADJUST = 0;
const float SP_ADJUST = 0;

#elif RES_1280X720
/* Resolution: 1280*720
*
*  const int X_OFFSET = 560;
*  const int Y_OFFSET = 170;
*  const int SPACING_FACTOR = 56;
*  const int MENU_SPACING = 56;
*/

const int X_OFFSET = 560;
const int Y_OFFSET = 170;
const int SPACING_FACTOR = 56;
const int MENU_SPACING = 56;
const float RES_ADJUST = 1;
const float HP_BAR_ADJUST = 0;
const float HP_ADJUST = 0;
const float SP_ADJUST = 0;

#endif

#ifndef GLOBAL_H
#define GLOBAL_H

extern int X_OFFSET;
extern int Y_OFFSET;
extern float SPACING_FACTOR;
extern int MENU_SPACING;
extern float RES_ADJUST;
extern float HP_BAR_ADJUST;
extern float HP_ADJUST;
extern float SP_ADJUST;

extern float GLOBAL_MUSIC_VOLUME;
extern float GLOBAL_SOUND_VOLUME;
extern float GLOBAL_UI_VOLUME;

const float GLOBAL_SPRITE_SCALE = 0.8f;

const int ID_ADVENTURER = 1;
const int ID_SPELLCASTER = 2;
const int ID_MADMAN = 3;
const int ID_SPELUNKER = 4;
const int ID_ACROBAT = 5;

const char WIND_UP = 's'; // wind up is used for items that have a turn of priming

const char c_PLAYER = '@';
const char SMASHER = '%';

// Self-explanatory
const std::string SPAWN_DISALLOWED = "Nothing is allowed to spawn here";
const std::string NPC_PROMPT = "NPC Prompt";
const std::string CHEST_PURCHASED = "Chest Purchased";

// PLAYERS
const std::string PLAYER = "Player";
const std::string ADVENTURER = "Adventurer";
const std::string SPELLCASTER = "Spellcaster";
const std::string MADMAN = "The Madman";
const std::string SPELUNKER = "The Spelunker";
const std::string ACROBAT = "The Acrobat";

// NPCS
const std::string CREATURE_LOVER = "Lionel";
const std::string MEMORIZER = "Stewart";
const std::string SHOPKEEPER = "Shopkeeper";

const std::string OUTSIDE_MAN1 = "Tired Man";
const std::string OUTSIDE_MAN2 = "Other Tired Man";
const std::string OUTSIDE_WOMAN1 = "Alert Woman";


// MONSTERS
const std::string RANDOM_MONSTER = "Random Monster";
const std::string GOBLIN = "Goblin";
const std::string ARCHER = "Archer";
const std::string WANDERER = "Wanderer";
const std::string RABID_WANDERER = "Rabid Wanderer";
const std::string FLAME_WANDERER = "Flame Wanderer";
const std::string PROVOCABLE_WANDERER = "Provocable Wanderer";
const std::string SLEEPING_WANDERER = "Sleeping Wanderer";
const std::string ZAPPER = "Zapper";
const std::string SPINNER = "Spinner";
const std::string BOMBEE = "Bombee";
const std::string CHARRED_BOMBEE = "Charred Bombee";
const std::string BOMBEE_ACTIVE = "Bombee Bomb";
const std::string ROUNDABOUT = "Roundabout";
const std::string BROUNDABOUT = "Broundabout";
const std::string FIRE_ROUNDABOUT = "Fire Roundabout";
const std::string MOUNTED_KNIGHT = "Knight";
const std::string SEEKER = "Seeker";
const std::string DEAD_SEEKER = "Dead Seeker";
const std::string ITEM_THIEF = "Item Thief";
const std::string GOO_SACK = "Goo Sack";
const std::string CHARGER = "Charger";
const std::string RAT = "Filthy Rat";
const std::string TOAD = "Filthy Toad";
const std::string GHOST = "Ghastly Spectre";
const std::string PUFF = "Puff";
const std::string GUSTY_PUFF = "Gusty Puff";
const std::string STRONG_GUSTY_PUFF = "Strong Gusty Puff";
const std::string INVERTED_PUFF = "Inverted Puff";
const std::string SPIKED_INVERTED_PUFF = "Spiked Inverted Puff";
const std::string GUSTY_SPIKED_INVERTED_PUFF = "Gusty Spiked Inverted Puff";
const std::string SPIDER = "Spider";
const std::string SHOOTING_SPIDER = "Shooting Spider";
const std::string POUNCING_SPIDER = "Pouncing Spider";
const std::string SERPENT = "Serpent";
const std::string ARMORED_SERPENT = "Armored Serpent";
const std::string SERPENT_TAIL = "Serpent's Tail";


// BREAKABLE OBJECTS
const std::string WEAK_CRATE = "Frail Crate";
const std::string WEAK_BARREL = "Weak Barrel";
const std::string WEAK_SIGN = "Weak Sign";
const std::string WEAK_ARROW_SIGN = "Weak Arrow Sign";
const std::string WEAK_POT = "Weak Pot";
const std::string WEAK_LARGE_POT = "Large Weak Pot";


const std::string PURCHASE = "purchase";
const std::string EMPTY = "";
const std::string STAIRCASE = "Stairs";
const std::string LOCKED_STAIRCASE = "Locked Stairs";
const std::string DOOR = "Door";
const std::string BUTTON_UNPRESSED = "Button Unpressed";
const std::string BUTTON_PRESSED = "Button Pressed";
const std::string REG_WALL = "Breakable Wall";
const std::string UNB_WALL = "Unbreakable Wall";
const std::string RANDOM_WALL = "Random Breakable Wall"; // when a level generates, this spot has a random chance to be a wall or an empty space
const std::string DOOR_VERTICAL = "Vertical Door";
const std::string DOOR_HORIZONTAL = "Horizontal Door";
const std::string DOOR_VERTICAL_OPEN = "Vertical Door Open";
const std::string DOOR_HORIZONTAL_OPEN = "Horizontal Door Open";
const std::string DOOR_VERTICAL_LOCKED = "Vertical Door Locked";
const std::string DOOR_HORIZONTAL_LOCKED = "Horizontal Door Locked";
const std::string FOUNTAIN = "Fountain";

const char SPACE = ' ';
const char IDOL = 'I';
const char DOOR_V = 'V';
const char DOOR_H = 'H';
const char STAIRS = 'S';

// WEAPONS
const std::string HANDS = "Empty-Handed";
const std::string SHORT_SWORD = "Short Sword";
const std::string RADIANT_SHORT_SWORD = "Radiant Short Sword";
const std::string CARBON_FIBER_SHORT_SWORD = "Carbon Fiber Short Sword";
const std::string ENCHANTED_SHORT_SWORD = "Enchanted Short Sword";
const std::string LUCKY_SHORT_SWORD = "Lucky Short Sword";
const std::string BLOOD_SHORT_SWORD = "Blood Short Sword";
const std::string GOLDEN_SHORT_SWORD = "Golden Short Sword";
const std::string GOLDEN_LONG_SWORD = "Golden Long Sword";
const std::string KATANA = "Katana";
const std::string GREATER_KATANA = "Greater Katana";
const std::string SUPERIOR_KATANA = "Superior Katana";
const std::string RUSTY_CUTLASS = "Rusty Cutlass";
const std::string BONE_AXE = "Bone Axe";
const std::string BRONZE_DAGGER = "Bronze Dagger";
const std::string IRON_LANCE = "Iron Lance";
const std::string PIKE = "Pike";
const std::string RADIANT_PIKE = "Radiant Pike";
const std::string CARBON_FIBER_PIKE = "Carbon Fiber Pike";
const std::string ENCHANTED_PIKE = "Enchanted Pike";
const std::string LUCKY_PIKE = "Lucky Pike";
const std::string IRON_LONG_SWORD = "Iron Long Sword";
const std::string WOOD_BOW = "Wood Bow";
const std::string IRON_BOW = "Reinforced Bow";
const std::string VULCAN_SWORD = "Vulcan Sword";
const std::string VULCAN_DAGGER = "Vulcan Dagger";
const std::string VULCAN_HAMMER = "Vulcan Hammer";
const std::string VULCAN_BOW = "Vulcan Bow";
const std::string HAMMER = "Hammer";
const std::string GREATER_HAMMER = "Greater Hammer";
const std::string SUPERIOR_HAMMER = "Superior Hammer";
const std::string ARCANE_STAFF = "Arcane Staff";
const std::string BO_STAFF = "Bo Staff";
const std::string GREATER_BO_STAFF = "Greater Bo Staff";
const std::string SUPERIOR_BO_STAFF = "Superior Bo Staff";
const std::string NUNCHUKS = "Nunchuks";
const std::string GREATER_NUNCHUKS = "Greater Nunchuks";
const std::string SUPERIOR_NUNCHUKS = "Superior Nunchuks";
const std::string ESTOC = "Estoc";
const std::string GREATER_ESTOC = "Greater Estoc";
const std::string SUPERIOR_ESTOC = "Superior Estoc";
const std::string JIAN = "Jian";
const std::string GREATER_JIAN = "Greater Jian";
const std::string SUPERIOR_JIAN = "Superior Jian";
const std::string ZWEIHANDER = "Zweihander";
const std::string GREATER_ZWEIHANDER = "Greater Zweihander";
const std::string SUPERIOR_ZWEIHANDER = "Superior Zweihander";
const std::string CLAW = "Claw";
const std::string MATTOCK = "Mattock";
const std::string MATTOCK_HEAD = "Mattock Head";
const std::string BOOMERANG = "Magic Boomerang";
const std::string PISTOL = "Damaged Pistol";
const std::string WHIP = "Trusty Whip";


// PICKUPS
const std::string RANDOM_WEAPON = "Random Weapon";
const std::string RANDOM_ITEM = "Random Item";
const std::string RANDOM_CHEST = "Random Chest";
const std::string RANDOM_TRINKET = "Random Trinket";
const std::string RANDOM_SHIELD = "Random Shield";
const std::string GOLD = "Gold";
const std::string RANDOM_GEM = "Random Gem";
const std::string HEART_POD = "Heart Pod";
const std::string LIFEPOT = "Life Potion";
const std::string BIG_LIFEPOT = "Big Life Potion";
const std::string STEADY_LIFEPOT = "Steady Life Potion";
const std::string HALF_LIFE_POTION = "Half-Life Potion";
const std::string SOUL_POTION = "Soul Potion";
const std::string BINARY_LIFE_POTION = "Binary Life Potion";
const std::string ARMOR = "Armor";
const std::string STATPOT = "Stat Potion";
const std::string ROTTEN_APPLE = "Rotten Apple";
const std::string ANTIDOTE = "Antidote";
const std::string DIZZY_ELIXIR = "Dizzy Elixir";
const std::string BOMB = "Bomb";
const std::string POISON_CLOUD = "Poison Bomb";
const std::string ROTTEN_MEAT = "Rotten Meat";
const std::string SHIELD_REPAIR = "Shield Repair";
const std::string MATTOCK_DUST = "Mattock Dust";
const std::string SKELETON_KEY = "Skeleton Key";
const std::string MATCHES = "Matches";
const std::string FIRECRACKERS = "Firecrackers";
const std::string SMOKE_BOMB = "Smoke Bomb";

const std::string TELEPORTER = "Functional Teleporter";
const std::string ROCKS = "Rock";

// CHESTS
const std::string LIFE_CHEST = "Life Chest";
const std::string BROWN_CHEST = "Brown Chest";
const std::string SILVER_CHEST = "Silver Chest";
const std::string GOLDEN_CHEST = "Gold Chest";
const std::string HAUNTED_CHEST = "Haunted Chest";
const std::string TELEPORTING_CHEST = "Teleporting Chest";
const std::string EXPLODING_CHEST = "Exploding Chest";
const std::string INFINITY_BOX = "Infinity Box";

// SHIELDS
const std::string WOOD_SHIELD = "Wood Shield";
const std::string IRON_SHIELD = "Iron Shield";
const std::string THORNED_SHIELD = "Thorned Shield";
const std::string FROST_SHIELD = "Frost Shield";
const std::string REFLECT_SHIELD = "Reflect Shield";

// SPELLS
const std::string FREEZE_SPELL = "Freeze";
const std::string ICE_SHARD_SPELL = "Ice Shards";
const std::string HAIL_STORM_SPELL = "Hail Storm";
const std::string EARTHQUAKE_SPELL = "Earthquake";
const std::string ROCK_SUMMON_SPELL = "Rock Summon";
const std::string SHOCKWAVE_SPELL = "Shockwave";
const std::string FIREBLAST_SPELL = "Fire Blast";
const std::string FIRE_CASCADE_SPELL = "Fire Cascade";
const std::string FIRE_EXPLOSION_SPELL = "Fiery Explosion";
const std::string WHIRLWIND_SPELL = "Whirlwind";
const std::string WIND_BLAST_SPELL = "Wind Blast";
const std::string WIND_VORTEX_SPELL = "Wind Vortex";
const std::string INVISIBILITY_SPELL = "Invisibility Spell";
const std::string ETHEREAL_SPELL = "Ethereal Spell";
const std::string TELEPORT = "Scroll of Unpredictable Travel";

// PASSIVES
const std::string BATWING = "Dismembered Wing";
const std::string LIFE_ELIXIR = "Life Elixir";
const std::string MAGIC_ESSENCE = "Magic Essence";
const std::string FLYING = "Levitation";
const std::string STEEL_PUNCH = "Steel Punch";
const std::string IRON_CLEATS = "Iron Cleats";
const std::string POISON_TOUCH = "Poison Touch";
const std::string FIRE_TOUCH = "Thermal Energy";
const std::string FROST_TOUCH = "Frozen Touch";
const std::string RAINBOW_TOUCH = "Overwhelming Energy";
const std::string POISON_IMMUNE = "Poison Immune";
const std::string FIRE_IMMUNE = "Fire Immune";
const std::string BOMB_IMMUNE = "Explosion Immunity";
const std::string POTION_ALCHEMY = "Potion Potency";
const std::string SOUL_SPLIT = "Soul Split";
const std::string HEAVY = "Heaviness";
const std::string BRICK_BREAKER = "Brick Breaker";
const std::string SUMMON_NPCS = "Call on Help";
const std::string CHEAP_SHOPS = "Discount!";
const std::string BETTER_RATES = "Less Penalty";
const std::string TRAP_ILLUMINATION = "Divine Vision";
const std::string ITEM_ILLUMINATION = "Item Vision";
const std::string MONSTER_ILLUMINATION = "Monster Vision";
const std::string RESONANT_SPELLS = "Resonant Spells";


// TRINKETS
const std::string DARK_STAR = "60 Watt";
const std::string LUCKY_PIG = "Lucky Pig";
const std::string GOLD_POT = "Golden Ring";
const std::string RING_OF_CASTING = "Spellcaster's Cap";
const std::string VULCAN_RUNE = "Rune of Vulcan";
const std::string BRIGHT_STAR = "Jar of Fireflies";
const std::string BLOODRITE = "Bloodrite";
const std::string BLOODLUST = "Bloodlust";
const std::string BATFANG = "Wanderer Fang";
const std::string CURSED_STRENGTH = "Cursed Strength";

// TRAPS
const std::string BRAZIER = "Brazier";
const std::string DEVILS_WATER = "Devil's Water";
const std::string SPIKES = "Spikes";
const std::string FALLING_SPIKE = "Falling Spike";
const std::string AUTOSPIKE_ACTIVE = "Auto Spiketrap Active";
const std::string AUTOSPIKE_PRIMED = "Auto Spiketrap Primed";
const std::string AUTOSPIKE_DEACTIVE = "Auto Spiketrap Deactive";
const std::string TRIGGERSPIKE_ACTIVE = "Trigger Spiketrap Active";
const std::string TRIGGERSPIKE_DEACTIVE = "Trigger Spiketrap Active";
const std::string PIT = "Pit";
const std::string FIREBAR = "Firebar";
const std::string DOUBLE_FIREBAR = "Double Firebar";
const std::string PUDDLE = "Puddle";
const std::string POISON_PUDDLE = "Poisonous Puddle";
const std::string LAVA = "Lava";
const std::string SPRING = "Normal Spring";
const std::string LEFT_SPRING = "Normal Left Spring";
const std::string RIGHT_SPRING = "Normal Right Spring";
const std::string UP_SPRING = "Normal Up Spring";
const std::string DOWN_SPRING = "Normal Down Spring";
const std::string BR_SPRING = "Normal BR Spring";
const std::string BL_SPRING = "Normal BL Spring";
const std::string TR_SPRING = "Normal TR Spring";
const std::string TL_SPRING = "Normal TL Spring";
const std::string MULTI_SPRING = "Multi Spring";
const std::string DIAGONAL_SPRING = "Diagonal Spring";
const std::string CARDINAL_SPRING = "Cardinal Spring";
const std::string TURRET = "Turret";
const std::string LEFT_TURRET = "Left Turret";
const std::string RIGHT_TURRET = "Right Turret";
const std::string UP_TURRET = "Up Turret";
const std::string DOWN_TURRET = "Down Turret";
const std::string MOVING_BLOCK = "Moving Block";
const std::string MOVING_BLOCK_H = "Moving Block Horizontal";
const std::string MOVING_BLOCK_V = "Moving Block Vertical";
const std::string ACTIVE_BOMB = "Active Bomb";
const std::string ACTIVE_MEGA_BOMB = "Active Mega Bomb";
const std::string POISON_BOMB = "Poison Mine";
const std::string BEAR_TRAP = "Bear Trap";
const std::string CRUMBLE_FLOOR = "Crumble Floor";
const std::string CRUMBLE_LAVA = "Crumble Lava";
const std::string EMBER = "Ember";
const std::string WEB = "Spider Web";
const std::string ROTTING_DECOY = "Rotten Meat Decoy";
const std::string FIRE_PILLARS = "Fire Pillars";
const std::string ICE_SHARDS = "Ice Shards";
const std::string HAIL_STORM = "Hail Storm";
const std::string SHOCKWAVES = "Shockwaves";
const std::string ROCK_SUMMON = "Rock Summon";
const std::string WIND_VORTEX = "Wind Vortex";

// Afflictions
const std::string HEAL_OVER_TIME = "Heal Over Time";
const std::string BURN = "Burn";
const std::string BLEED = "Bleed";
const std::string STUN = "Stun";
const std::string FREEZE = "Freeze";
const std::string POISON = "Poison";
const std::string INVISIBILITY = "Invisility";
const std::string ETHEREALITY = "Ethereality";
const std::string INVULNERABILITY = "Invulnerablity";
const std::string CONFUSION = "Confusion";
const std::string BUFF = "Buff";
const std::string STUCK = "Stuck";
const std::string POSSESSED = "Possessed";
const std::string CRIPPLE = "Cripple";



// ================================================
//		:::: SHORTHAND FOR LEVEL GENERATION ::::
// ================================================

const std::string _x_ = SPAWN_DISALLOWED;
const std::string xx = SPAWN_DISALLOWED;
const std::string _ply = PLAYER;
const std::string _ext = STAIRCASE;
const std::string lkex = LOCKED_STAIRCASE;
const std::string wll = REG_WALL;
const std::string unb = UNB_WALL;
const std::string r_wl = RANDOM_WALL;
const std::string sp = EMPTY;
const std::string _dv = DOOR_VERTICAL;
const std::string _dh = DOOR_HORIZONTAL;
const std::string _dw = DEVILS_WATER;
const std::string ftn = FOUNTAIN;

// Items
const std::string i_bc = BROWN_CHEST;
const std::string i_sc = SILVER_CHEST;
const std::string i_gc = GOLDEN_CHEST;
const std::string i_hp = HEART_POD;
const std::string i_lp = LIFEPOT;
const std::string i_blp = BIG_LIFEPOT;
const std::string i_b = BOMB;
const std::string i_ar = ARMOR;
const std::string i_gd = GOLD;
const std::string i_rg = RANDOM_GEM;
const std::string i_rw = RANDOM_WEAPON;
const std::string i_ri = RANDOM_ITEM;
const std::string i_rc = RANDOM_CHEST;
const std::string i_rs = RANDOM_SHIELD;
const std::string i_rt = RANDOM_TRINKET;

// Monsters
const std::string m_g = GOBLIN;
const std::string m_se = SEEKER;
const std::string m_ds = DEAD_SEEKER;
const std::string m_b = BOMBEE;
const std::string m_it = ITEM_THIEF;
const std::string m_gs = GOO_SACK;
const std::string m_sp = SPINNER;
const std::string m_z = ZAPPER;
const std::string m_r = ROUNDABOUT;
const std::string m_br = BROUNDABOUT;
const std::string m_fr = FIRE_ROUNDABOUT;
const std::string m_k = MOUNTED_KNIGHT;
const std::string m_w = WANDERER;
const std::string m_a = ARCHER;
const std::string m_rm = RANDOM_MONSTER;

// Traps
const std::string t_bu = BUTTON_UNPRESSED;
const std::string t_pt = PIT;
const std::string t_lv = LAVA;
const std::string t_s = SPRING;
const std::string t_sl = LEFT_SPRING;
const std::string t_sr = RIGHT_SPRING;
const std::string t_su = UP_SPRING;
const std::string t_sd = DOWN_SPRING;
const std::string t_sbr = BR_SPRING;
const std::string t_sbl = BL_SPRING;
const std::string t_str = TR_SPRING;
const std::string t_stl = TL_SPRING;
const std::string t_f = FIREBAR;
const std::string t_df = DOUBLE_FIREBAR;
const std::string t_p = PUDDLE;
const std::string t_tt = TURRET; // Random turret
const std::string t_tl = LEFT_TURRET;
const std::string t_tr = RIGHT_TURRET;
const std::string t_tu = UP_TURRET;
const std::string t_td = DOWN_TURRET;
const std::string t_mb = MOVING_BLOCK;
const std::string t_mbh = MOVING_BLOCK_H;
const std::string t_mbv = MOVING_BLOCK_V;
const std::string t_as = AUTOSPIKE_DEACTIVE;
const std::string t_ts = TRIGGERSPIKE_DEACTIVE;
const std::string t_sk = SPIKES;
const std::string t_cf = CRUMBLE_FLOOR;
const std::string t_cl = CRUMBLE_LAVA;

#endif