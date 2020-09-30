

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

extern float GLOBAL_SPRITE_SCALE;

const std::string TEXT_FONT = "fonts/SquadaOne-Regular.ttf";

const std::string EN_US_FILE = "enUS.json";
const std::string EN_UK_FILE = "enUK.txt";
const std::string FR_EU_FILE = "frEU.txt";

const int ID_ADVENTURER = 1;
const int ID_SPELLCASTER = 2;
const int ID_MADMAN = 3;
const int ID_SPELUNKER = 4;
const int ID_ACROBAT = 5;

const char WIND_UP = 's'; // wind up is used for items that have a turn of priming

const int TUTORIAL = -1;
const int WORLD_HUB = 0;
const int FIRST_FLOOR = 1;
const int SECOND_FLOOR = 3;
const int THIRD_FLOOR = 5;
const int FOURTH_FLOOR = 7;
const int FIFTH_FLOOR = 9;
const int SIXTH_FLOOR = 11;
const int SEVENTH_FLOOR = 13;
const int EIGHTH_FLOOR = 15;
const int NINTH_FLOOR = 17;

const int FIRST_BOSS = 8;

const int FIRST_SHOP = 4;
const int SECOND_SHOP = 8;
const int THIRD_SHOP = 12;
const int FOURTH_SHOP = 16;

const int FIRST_SHRINE = 2;
const int SECOND_SHRINE = 6;
const int THIRD_SHRINE = 10;
const int FOURTH_SHRINE = 14;

const char c_PLAYER = '@';
const char SMASHER = '%';

// Sprite Z Ordering
const int Z_HUD_LABEL = 220;
const int Z_HUD_SPRITE = 210;
const int Z_HUD_ELEMENT = 200;
const int Z_FLOATERS = 30;
const int Z_PROJECTILE = 20;
const int Z_WALL = 10;
const int Z_ACTOR = 10;
const int Z_CHEST = 10;
const int Z_ITEM = 5;
const int Z_MONEY = 0;
const int Z_TRAP_TOP = -5;
const int Z_TRAP_MIDDLE = -10;
const int Z_TRAP_BOTTOM = -15;
const int Z_EXIT = -20;
const int Z_FLOOR = -100;

// Self-explanatory
const std::string SPAWN_DISALLOWED = "Nothing is allowed to spawn here";
const std::string NPC_PROMPT = "NPC_PROMPT";
const std::string CHEST_PURCHASED = "Chest Purchased";

const std::string PURCHASE = "purchase";
const std::string EMPTY = "";
const std::string STAIRCASE = "Stairs";
const std::string LOCKED_STAIRCASE = "Locked Stairs";
const std::string DOOR = "Door";
const std::string BUTTON_UNPRESSED = "Button Unpressed";
const std::string BUTTON_PRESSED = "Button Pressed";
const std::string REG_WALL = "Breakable Wall";
const std::string UNB_WALL = "Unbreakable Wall";
const std::string BOUNDARY = "Boundary";
const std::string RANDOM_WALL = "Random Breakable Wall"; // when a level generates, this spot has a random chance to be a wall or an empty space
const std::string DOOR_VERTICAL = "Vertical Door";
const std::string DOOR_HORIZONTAL = "Horizontal Door";
const std::string DOOR_VERTICAL_OPEN = "Vertical Door Open";
const std::string DOOR_HORIZONTAL_OPEN = "Horizontal Door Open";
const std::string DOOR_VERTICAL_LOCKED = "Vertical Door Locked";
const std::string DOOR_HORIZONTAL_LOCKED = "Horizontal Door Locked";
const std::string FOUNTAIN = "Fountain";

const char SPACE = ' ';
const char DOOR_V = 'V';
const char DOOR_H = 'H';
const char STAIRS = 'S';

// For Shrines
const std::string CHEST_CHOICE = "Chests";
const std::string RELIC_CHOICE = "Relics";
const std::string HEALTH_CHOICE = "Health";

// PLAYERS
const std::string PLAYER = "Player";
const std::string ADVENTURER = "Adventurer";
const std::string SPELLCASTER = "Spellcaster";
const std::string MADMAN = "The Madman";
const std::string SPELUNKER = "The Spelunker";
const std::string ACROBAT = "The Acrobat";

// NPCS
const std::string CREATURE_LOVER = "Creature Lover";
const std::string MEMORIZER = "Memorizer";
const std::string INJURED_EXPLORER = "Injured Explorer";
const std::string PORTAL_RESEARCHER = "Portal Researcher";
const std::string SHOPKEEPER = "Shopkeeper";
const std::string BLACKSMITH = "Blacksmith";
const std::string ENCHANTER = "Enchanter";
const std::string TRADER = "Trader";

const std::string TUTORIAL_NPC1 = "Tutorial NPC1";
const std::string TUTORIAL_NPC2 = "Tutorial NPC2";
const std::string OUTSIDE_WOMAN1 = "Alert Woman";


// MONSTERS
const std::string RANDOM_MONSTER = "Random Monster";
const std::string FORGOTTEN_SPIRIT = "Forgotten Spirit";
const std::string GHOST = "Ghastly Spectre";
const std::string FLARE = "Mini Flare";
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
const std::string ROUNDABOUT = "Roundabout";
const std::string BROUNDABOUT = "Broundabout";
const std::string FIRE_ROUNDABOUT = "Fire Roundabout";
const std::string PIKEMAN = "Pikeman";
const std::string SEEKER = "Seeker";
const std::string DEAD_SEEKER = "Dead Seeker";
const std::string ITEM_THIEF = "Item Thief";
const std::string GOO_SACK = "Goo Sack";
const std::string CHARGER = "Charger";
const std::string RAT = "Filthy Rat";
const std::string TOAD = "Filthy Toad";
const std::string WATER_SPIRIT = "Water Spirit";
const std::string FIREFLY = "Firefly";
const std::string TRI_HORN = "Tri-Horned Gissel";
const std::string TUMBLE_SHROOM = "Tumble Shroom";
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
const std::string POISON_BUBBLE = "Poison Bubble";
const std::string PIRANHA = "Piranha";
const std::string ANGLED_BOUNCER = "Angled Bouncer";
const std::string WRIGGLER = "Wriggler";
const std::string BARBED_CATERPILLAR = "Barbed Caterpillar";
const std::string CRYSTAL_TURTLE = "Crystal Turtle";
const std::string CRYSTAL_HEDGEHOG = "Crystal Hedgehog";
const std::string CRYSTAL_SHOOTER = "Crystal Shooter";
const std::string CRYSTAL_BEETLE = "Crystal Beetle";
const std::string CRYSTAL_WARRIOR = "Crystal Warrior";
const std::string RABBIT = "Rabbit";
const std::string BENEVOLENT_BARK = "Benevolent Bark";
const std::string TICK = "Tick";
const std::string EXOTIC_FELINE = "Exotic Feline";
const std::string OLD_SMOKEY = "Old Smokey";
const std::string JEWELED_SCARAB = "Jeweled Scarab";
const std::string ARMORED_BEETLE = "Armored Beetle";
const std::string SPECTRAL_SWORD = "Spectral Sword";
const std::string SAND_CENTIPEDE = "Sand Centipede";
const std::string SAND_BEAVER = "Sand Beaver";
const std::string SAND_ALBATROSS = "Sand Albatross";
const std::string SHRINEKEEPER = "Shrinekeeper";
const std::string WATCHER = "Watcher";
const std::string HOWLER = "Howler";
const std::string WISP = "Wisp";
const std::string LIGHTNING_WISP = "Lightning Wisp";
const std::string EVIL_SHADOW = "Sinister Shadow";
const std::string GRABBER = "Grabber";
const std::string ABYSS_SUMMONER = "Abyss Summoner";
const std::string CRAWLING_SPINE = "Crawling Spine";
const std::string MAGICAL_BERSERKER = "Magical Berserker";
const std::string COMBUSTION_GOLEM = "Combustion Golem";
const std::string CONSTRUCTOR_DEMON = "Dirt Demon";
const std::string FACELESS_HORROR = "Faceless Horror";
const std::string TOWERING_BRUTE = "Towering Brute";
const std::string SWAPPER = "Swapper";
const std::string SKY_CRASHER = "Meteor Man";
const std::string STEAM_BOT = "Steam Bot";
const std::string SHIELD_MASTER = "Shield Master";
const std::string PSEUDO_DOPPEL = "Pseudoppel";
const std::string BLADE_PSYCHIC = "Blade Psychic";
const std::string ELECTROMAGNETIZER = "Electromagnetizer";
const std::string DISABLER = "Disabler";
const std::string INCENDIARY_INFUSER = "Incendiary Infuser";
const std::string DASH_MASTER = "Dash Master";
const std::string ACIDIC_BEAST = "Acidic Beast";
const std::string DARK_CANINE = "Dark Canine";
const std::string LIGHTNING_STRIKER = "Lightning Archaic";
const std::string MASTER_CONJURER = "Master Conjurer";
const std::string FLAME_ARCHAIC = "Flame Archaic";
const std::string ADVANCED_ROCK_SUMMONER = "Advanced Rock Summoner";
const std::string ASCENDED_SHOT = "Ascended Shot";
const std::string ROYAL_SWORDSMAN = "Royal Swordsman";
const std::string LIGHT_ENTITY = "Being of Light";
const std::string DARK_ENTITY = "Being of Dark";

// BREAKABLE OBJECTS
const std::string WEAK_CRATE = "Weak Crate";
const std::string WEAK_BARREL = "Weak Barrel";
const std::string WEAK_SIGN = "Weak Sign";
const std::string WEAK_ARROW_SIGN = "Weak Arrow Sign";
const std::string WEAK_POT = "Weak Pot";
const std::string WEAK_LARGE_POT = "Large Weak Pot";
const std::string EXPLOSIVE_BARREL = "Explosive Barrel";
const std::string CHARRED_WOOD = "Charred Wood";

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
const std::string RANDOM_TRINKET = "Random Relic";
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
const std::string POISON_BOMB = "Poison Bomb";
const std::string ROTTEN_MEAT = "Rotten Meat";
const std::string SHIELD_REPAIR = "Shield Repair";
const std::string MATTOCK_DUST = "Mattock Dust";
const std::string SKELETON_KEY = "Skeleton Key";
const std::string MATCHES = "Matches";
const std::string FIRECRACKERS = "Firecrackers";
const std::string SMOKE_BOMB = "Smoke Bomb";
const std::string TELEPORT = "Scroll of Unpredictable Travel";
const std::string WILD_MUSHROOM = "Wild Mushroom";
const std::string MAGMA_HEART = "Magma Heart";
const std::string CACTUS_WATER = "Cactus Water";
const std::string SUPER_ROOT = "Super Root";
const std::string RPG_IN_A_BOTTLE = "RPG In A Bottle";
const std::string MONSTER_TRANSFORM = "Monster Transform";
const std::string SLEDDING_SCARF = "Sledding Scarf";
const std::string BANANA = "Banana";
const std::string PEELED_BANANA = "Peeled Banana";
const std::string HALF_EATEN_BANANA = "Half Eaten Banana";
const std::string BANANA_PEEL = "Banana Peel";
const std::string BANANA_BUNCH = "Banana Bunch";

const std::string TELEPORTER = "Functional Teleporter";
const std::string ROCKS = "Rock";
const std::string MOBILITY = "Acrobatics";

// CHESTS
const std::string TREASURE_CHEST = "Treasure Chest";
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
const std::string FREEZE_SPELL = "Freeze Spell";
const std::string ICE_SHARD_SPELL = "Ice Shards Spell";
const std::string HAIL_STORM_SPELL = "Hail Storm Spell";
const std::string EARTHQUAKE_SPELL = "Earthquake Spell";
const std::string ROCK_SUMMON_SPELL = "Rock Summon Spell";
const std::string SHOCKWAVE_SPELL = "Shockwave Spell";
const std::string FIREBLAST_SPELL = "Fire Blast Spell";
const std::string FIRE_CASCADE_SPELL = "Fire Cascade Spell";
const std::string FIRE_EXPLOSION_SPELL = "Fiery Explosion Spell";
const std::string WHIRLWIND_SPELL = "Whirlwind Spell";
const std::string WIND_BLAST_SPELL = "Wind Blast Spell";
const std::string WIND_VORTEX_SPELL = "Wind Vortex Spell";
const std::string THUNDER_CLOUD_SPELL = "Thunder Cloud Spell";
const std::string INVISIBILITY_SPELL = "Invisibility Spell";
const std::string ETHEREAL_SPELL = "Ethereal Spell";

// PASSIVES
const std::string BAT_WING = "Bat Wing";
const std::string LIFE_ELIXIR = "Life Elixir";
const std::string MAGIC_ESSENCE = "Magic Essence";
const std::string FLYING = "Flying";
const std::string STEEL_PUNCH = "Steel Punch";
const std::string IRON_CLEATS = "Iron Cleats";
const std::string LUCKY_PIG = "Lucky Pig";
const std::string LIFE_STEAL = "Life Steal";
const std::string BERSERK = "Berserk";
const std::string VULCAN_RUNE = "Vulcan Rune";
const std::string POISON_TOUCH = "Poison Touch";
const std::string FIRE_TOUCH = "Fire Touch";
const std::string FROST_TOUCH = "Frozen Touch";
const std::string RAINBOW_TOUCH = "Rainbow Touch";
const std::string POISON_IMMUNE = "Poison Immune";
const std::string FIRE_IMMUNE = "Fire Immune";
const std::string BOMB_IMMUNE = "Bomb Immune";
const std::string POTION_ALCHEMY = "Potion Alchemy";
const std::string SOUL_SPLIT = "Soul Split";
const std::string HEAVY = "Heavy";
const std::string BRICK_BREAKER = "Brick Breaker";
const std::string SUMMON_NPCS = "Summon NPCs";
const std::string CHEAP_SHOPS = "Cheap Shops";
const std::string BETTER_RATES = "Better Rates";
const std::string TRAP_ILLUMINATION = "Trap Illumination";
const std::string ITEM_ILLUMINATION = "Item Illumination";
const std::string MONSTER_ILLUMINATION = "Monster Illumination";
const std::string RESONANT_SPELLS = "Resonant Spells";
const std::string FAT_STACKS = "Fat Stacks";
const std::string SCAVENGER = "Scavenger";

// RELICS
const std::string CURSED_STRENGTH = "Cursed Strength";
const std::string BRIGHT_STAR = "Jar of Fireflies";
const std::string DARK_STAR = "60 Watt";
const std::string RICHES = "Gold Torus";
const std::string SUPER_MAGIC_ESSENCE = "Super Magic Essence";
const std::string MATRIX_VISION = "Matrix Vision";
const std::string PROTECTION = "Guardian's Blessing";

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
const std::string FROZEN_PUDDLE = "Frozen Puddle";
const std::string WATER = "Water";
const std::string POISON_WATER = "Poison Water";
const std::string ICE = "Ice";
const std::string POISON_MISTER = "Poison Mister";
const std::string STALACTITE = "Stalactite";
const std::string GIANT_CRYSTAL = "Giant Crystal";
const std::string BROKEN_CRYSTALS = "Broken Crystals";
const std::string TREE = "Tree";
const std::string TREE_ROOT = "Tree Root";
const std::string MALEVOLENT_PLANT = "Malevolent Plant";
const std::string LAVA = "Lava";
const std::string LAZE = "Laze";
const std::string MAGMA_TIDE = "Magma Tide";
const std::string MOLTEN_PILLAR = "Molten Pillar";
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
const std::string ACTIVE_FIRE_BOMB = "Active Fire Bomb";
const std::string ACTIVE_POISON_BOMB = "Poison Mine";
const std::string POISON_CLOUD = "Poison Cloud";
const std::string BEAR_TRAP = "Bear Trap";
const std::string CRUMBLE_PIT = "Crumble Pit";
const std::string CRUMBLE_LAVA = "Crumble Lava";
const std::string EMBER = "Ember";
const std::string WEB = "Spider Web";
const std::string WIND_TUNNEL = "Wind Tunnel";
const std::string SAND = "Sand";
const std::string QUICKSAND = "Quicksand";
const std::string CACTUS = "Cactus";
const std::string LIGHT_ABSORBER = "Light Absorber";
const std::string MOVING_TILE = "Moving Tile";
const std::string WATCHER_STATUE = "Watcher Statue";
const std::string LAVA_GRATING = "Lava Grating";
const std::string FLARE_CANDLE = "Flare Candle";
const std::string ACID = "Acid";
const std::string DNA_SPLITTER = "DNA Splitter";
const std::string LIGHTNING_ROD = "Lightning Rod";

const std::string SPORES = "Poison Spores";
const std::string DUST_DEVIL = "Dust Devil";
const std::string ENERGY_HELIX = "Wispcharge";
const std::string ABYSSAL_MAW = "Abyssal Maw";
const std::string GOOP = "Sticky Goop";
const std::string MINI_ERUPTION = "Mini Eruption";
const std::string COMBUSTION = "Combustion";
const std::string DIRT_MOUND = "Dirt Mound";
const std::string FACELESS_MASK = "Faceless Mask";
const std::string PSYCHIC_SLASH = "Psychic Slash";
const std::string DISABLING_FIELD = "Disabling Field";
const std::string FLAME_ARCHAIC_FIRE_PILLARS = "Flame Archaic Fire Pillars";
const std::string MEGA_ROCK = "A Giant Rock";
const std::string REFLECTIVE_SHOT = "Reflective Projectile";
const std::string LIGHT_BEAM = "Light Beam";

const std::string ROTTING_DECOY = "Rotten Meat Decoy";
const std::string FACTORY_TILE = "Factory Tile";
const std::string FIRE_PILLARS = "Fire Pillars";
const std::string ICE_SHARDS = "Ice Shards";
const std::string HAIL_STORM = "Hail Storm";
const std::string SHOCKWAVES = "Shockwaves";
const std::string ROCK_SUMMON = "Rock Summon";
const std::string WIND_VORTEX = "Wind Vortex";
const std::string THUNDER_CLOUD = "Thunder Cloud";

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
const std::string FRAGILE = "Fragile";
const std::string INCENDIARY = "Incendiary";
const std::string BLINDNESS = "Blindness";
const std::string DISABLED = "Disabled";
const std::string WET = "Wet";
const std::string SLIPPED = "Slipped";
const std::string TICKED = "Ticked";

const std::string TIMED_BUFF = "Timed Buff";
const std::string TIMED_HEAL = "Timed Heal";
const std::string XP_GAIN = "XP Gain";
const std::string AFFLICTION_IMMUNITY = "Affliction Immunity";


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
const std::string m_k = PIKEMAN;
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
const std::string t_cf = CRUMBLE_PIT;
const std::string t_cl = CRUMBLE_LAVA;

#endif