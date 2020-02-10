#include <string>

	//#define RES_1920X1080 1
	//#define RES_1600X900 1
	#define RES_1366X768 1
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
const float RES_ADJUST = 1.08;
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

const char WIND_UP = 's'; // wind up is used for items that have a turn of priming

const char ESC = 27;
const char PAUSE = 'p';

const char c_PLAYER = '@';
const char c_GOBLIN = 'G';
const char c_ARCHER = 'A';
const char c_WANDERER = 'W';
const char c_ZAPPER = 'Z';
const char c_SPINNER = 'S';
const char c_BOMBEE = 'B';
const char c_BOMBEE_ACTIVE = 235;
const char c_ROUNDABOUT = 233;
const char c_MOUNTED_KNIGHT = 'K';
const char c_SEEKER = 'C';
const char SMASHER = '%';

//
const std::string SPAWN_DISALLOWED = "Nothing is allowed to spawn here";


// ACTORS
const std::string PLAYER = "Player";

// NPCS
const std::string CREATURE_LOVER = "Lionel";

// MONSTERS
const std::string RANDOM_MONSTER = "Random Monster";
const std::string GOBLIN = "Goblin";
const std::string ARCHER = "Archer";
const std::string WANDERER = "Wanderer";
const std::string ZAPPER = "Zapper";
const std::string SPINNER = "Spinner";
const std::string BOMBEE = "Bombee";
const std::string BOMBEE_ACTIVE = "Bombee Bomb";
const std::string ROUNDABOUT = "Roundabout";
const std::string BROUNDABOUT = "Broundabout";
const std::string FIRE_ROUNDABOUT = "Fire Roundabout";
const std::string MOUNTED_KNIGHT = "Knight";
const std::string SEEKER = "Seeker";
const std::string DEAD_SEEKER = "Dead Seeker";
const std::string ITEM_THIEF = "Item Thief";
const std::string GOO_SACK = "Goo Sack";

const std::string PURCHASE = "purchase";
const std::string EMPTY = "";
const std::string STAIRCASE = "Stairs";
const std::string LOCKED_STAIRCASE = "Locked Stairs";
const std::string DOOR = "Door";
const std::string BUTTON_UNPRESSED = "Button Unpressed";
const std::string BUTTON_PRESSED = "Button Pressed";
const std::string REG_WALL = "Breakable Wall";
const std::string UNB_WALL = "Unbreakable Wall";
const std::string RANDOM_WALL = "Randoml Breakable Wall"; // when a level generates, this spot has a random chance to be a wall or an empty space
const std::string DOOR_VERTICAL = "Vertical Door";
const std::string DOOR_HORIZONTAL = "Horizontal Door";
const std::string FOUNTAIN = "Fountain";
const char SPACE = ' ';
const char IDOL = 234;
const char KEY = 230;
const char UNBREAKABLE_WALL = 219;
const char WALL = 176;
const char DOOR_V = 186;
const char DOOR_V_OPEN = 249;
const char DOOR_V_LOCKED = 215;
const char DOOR_H = 205;
const char DOOR_H_OPEN = 250;
const char DOOR_H_LOCKED = 216;
const char c_FOUNTAIN = 157;
const char STAIRS = 240;
const char LOCKED_STAIRS = 241;
const char BUTTON = 167;
const char c_LAVA = 247;
//const char DEVILS_WATER = 178;

// WEAPONS
//const char CUTLASS = 244;
//const char BONEAXE = 226;
//const char BRONZE_DAGGER = 191;
//const char IRON_LANCE = 140;
const std::string SHORT_SWORD = "Short Sword";
const std::string BLOOD_SHORT_SWORD = "Blood Short Sword";
const std::string GOLDEN_SHORT_SWORD = "Golden Short Sword";
const std::string GOLDEN_LONG_SWORD = "Golden Long Sword";
const std::string KATANA = "Katana";
const std::string RUSTY_CUTLASS = "Rusty Cutlass";
const std::string BONE_AXE = "Bone Axe";
const std::string BRONZE_DAGGER = "Bronze Dagger";
const std::string IRON_LANCE = "Iron Lance";
const std::string IRON_LONG_SWORD = "Iron Long Sword";
const std::string WOOD_BOW = "Wood Bow";
const std::string IRON_BOW = "Reinforced Bow";
const std::string VULCAN_SWORD = "Vulcan Sword";
const std::string VULCAN_DAGGER = "Vulcan Dagger";
const std::string VULCAN_HAMMER = "Vulcan Hammer";
const std::string VULCAN_BOW = "Vulcan Bow";
const std::string ARCANE_STAFF = "Arcane Staff";

// PICKUPS
const std::string RANDOM_WEAPON = "Random Weapon";
const std::string RANDOM_ITEM = "Random Item";
const std::string RANDOM_CHEST = "Random Chest";
const std::string RANDOM_TRINKET = "Random Trinket";
const std::string RANDOM_SHIELD = "Random Shield";
const char c_HEART_POD = 248;
const char c_LIFEPOT = 231;
const char c_ARMOR = 131;
const char c_STATPOT = 225;
const char c_BOMB = 229;
const std::string GOLD = "Gold";
const std::string RANDOM_GEM = "Random Gem";
const std::string HEART_POD = "Heart Pod";
const std::string LIFEPOT = "Life Potion";
const std::string BIG_LIFEPOT = "Big Life Potion";
const std::string ARMOR = "Armor";
const std::string STATPOT = "Stat Potion";
const std::string DIZZY_ELIXIR = "Dizzy Elixir";
const std::string BOMB = "Bomb";
const std::string POISON_CLOUD = "Poison Bomb";
const std::string SHIELD_REPAIR = "Shield Repair";
const std::string SKELETON_KEY = "Skeleton Key";
//const std::string LITBOMB = 149;
//const char SHIELD_REPAIR = 'r';
const char LITBOMB = 149;
//const char SKELETON_KEY = 224;

const char c_BROWN_CHEST = 239;
const char c_SILVER_CHEST = 227;
const char c_GOLDEN_CHEST = 232;
const char c_INFINITY_BOX = 236;
const std::string BROWN_CHEST = "Brown Chest";
const std::string SILVER_CHEST = "Silver Chest";
const std::string GOLDEN_CHEST = "Gold Chest";
const std::string INFINITY_BOX = "Infinity Box";

//const char WOOD_SHIELD = 'a';
//const char IRON_SHIELD = 'b';
const std::string WOOD_SHIELD = "Wood Shield";
const std::string IRON_SHIELD = "Iron Shield";
const std::string THORNED_SHIELD = "Thorned Shield";
const std::string FROST_SHIELD = "Frost Shield";
const std::string REFLECT_SHIELD = "Reflect Shield";

const std::string FREEZE_SPELL = "Frost";
const std::string EARTHQUAKE_SPELL = "Earthquake";
const std::string FIREBLAST_SPELL = "Fire Blast";
const std::string WIND_SPELL = "Wind Spell";
const std::string INVISIBILITY_SPELL = "Invisibility Spell";
const std::string ETHEREAL_SPELL = "Ethereal Spell";
const std::string TELEPORT = "Scroll of Unpredictable Travel";

const std::string DARK_STAR = "60 Watt";
const std::string LUCKY_PIG = "Lucky Pig";
const std::string GOLD_POT = "Golden Ring";
const std::string RING_OF_CASTING = "Spellcaster's Cap";
const std::string VULCAN_RUNE = "Rune of Vulcan";
const std::string BRIGHT_STAR = "Jar of Fireflies";
const std::string BLOODRITE = "Bloodrite";
const std::string BLOODLUST = "Bloodlust";

// TRAPS
const char c_SPIKE = 'v';
const char SPIKETRAP_ACTIVE = '!';
const char SPIKETRAP_PRIMED = 177;
const char SPIKETRAP_DEACTIVE = '_';
const char TRIGGER_SPIKE_DEACTIVE = 246;
const char TRIGGER_SPIKE_ACTIVE = '^';
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
const std::string LAVA = "Lava";
const std::string SPRING = "Normal Spring";
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



// ================================================
//		:::: SHORTHAND FOR LEVEL GENERATION ::::
// ================================================

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
const std::string t_tt = TURRET; // Random turret
const std::string t_lv = LAVA;
const std::string t_s = SPRING;
const std::string t_f = FIREBAR;
const std::string t_df = DOUBLE_FIREBAR;
const std::string t_p = PUDDLE;
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