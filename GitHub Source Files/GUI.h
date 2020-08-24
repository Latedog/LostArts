#ifndef GUI_H
#define GUI_H

class cocos2d::Ref;
class cocos2d::Scene;
class cocos2d::Sprite;
class cocos2d::Label;
class cocos2d::EventListenerKeyboard;
enum class cocos2d::EventKeyboard::KeyCode;
class cocos2d::Event;
class EffectSprite;
class LightEffect;
class Dungeon;
class Player;
class NPC;

using KeyType = cocos2d::EventKeyboard::KeyCode;

extern KeyType UP_KEY;
extern KeyType DOWN_KEY;
extern KeyType LEFT_KEY;
extern KeyType RIGHT_KEY;
extern KeyType INTERACT_KEY;
extern KeyType QUICK_KEY;
extern KeyType ACTIVE_KEY;
extern KeyType WEAPON_KEY;
extern KeyType CAST_KEY;
extern KeyType ITEM_KEY;
extern KeyType INVENTORY_KEY;

std::string convertKeycodeToStr(KeyType keyCode);
static void restartGame(const Player &p);

class MenuScene : public cocos2d::Scene {
public:
	MenuScene();
	virtual ~MenuScene();

	static MenuScene* create();
	static cocos2d::Scene* createScene();
	virtual bool init();

	void options();
	void optionKeyPressed(KeyType keyCode, cocos2d::Event* event);

	void soundOptions();
	void soundOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event);

	void keyBindings();
	void keyBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void setKey(KeyType keyCode, int index);
	bool keyIsValid(KeyType keyCode, int index); // Checks if there's any conflict with other keys set
	void resetBindings(); // Reset key binds to the defaults

	void adjustResolution();
	void toggleFullscreen();

	void addLabel(float x, float y, std::string name, std::string id, float fontSize);
	void updateLabel(std::string id, std::string newText);

	void removeAll();
	void removeLabels();
	void removeSprites();

protected:
	std::map<std::string, cocos2d::Sprite*> sprites;
	std::map<std::string, cocos2d::Label*> labels;

	cocos2d::EventListenerKeyboard* kbListener = nullptr;
	int id = -1; // bg music id
	int index = 0; // menu selection

private:
	static std::map<KeyType, std::chrono::high_resolution_clock::time_point> keys;

	std::vector<std::pair<int, int>> resolutions;
	int resolutionIndex = 0; // 
	bool fullScreen = false;

	// For key bindings
	std::vector<KeyType> controls;
	bool settingKey = false; // False indicates the user is not trying to change the key; True indicates the next keystroke will change the key.
};

class StartScene : public MenuScene {
public:
	CREATE_FUNC(StartScene);
	static cocos2d::Scene* createScene();

	virtual bool init();

	void keyPressed(KeyType keyCode, cocos2d::Event* event);

	void characterSelect();
	void characterKeyPressed(KeyType keyCode, cocos2d::Event* event);

	void startGameCallback(cocos2d::Ref* pSender);
	void exitGameCallback(cocos2d::Ref* pSender);

private:
	float degreeToRadian(int degree);
	void polarToCartesian(int degree, float &x, float &y);

	void rotate(bool clockwise);
	void displayCharacterInfo();
	void setAngle(int &angle);
	void setCharacterSelectPositions(); // For the initial positions of the characters on the select screen. Dependent on number of characters unlocked.
	void addCharacterSprite(float x, float y, std::string image, std::string id);

	float spriteScalingFactor1 = 1.5f;
	float spriteScalingFactor2 = 1.0f;
	int degree1;
	int degree2;
	int degree3;
	int degree4;
	int degree5;
	
	int characterID; // character chosen
};

class HUDLayer : public cocos2d::Layer {
public:
	HUDLayer(std::shared_ptr<Player> p);
	~HUDLayer();
	static HUDLayer* create(std::shared_ptr<Player> p);
	virtual bool init();

	void updateHUD(Dungeon &dungeon);
	
	void showBossHP();
	void updateBossHUD(Dungeon &dungeon);

	void showHUD();
	void hideHUD();

	void NPCInteraction(cocos2d::EventListenerKeyboard* listener, NPC &npc, std::vector<std::string> dialogue);
	void NPCPrompt(NPC &npc, std::vector<std::string> choices);

	void devilsWaters(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);

	// Shrine prompts
	void shrineChoice(cocos2d::EventListenerKeyboard* listener, std::string shrine, Dungeon &dungeon);

	void inventoryMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);

	void itemMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);

	void gameOver();
	void gameOver(cocos2d::Scene &scene);
	
	void winner();	

private:
	void NPCKeyPressed(KeyType keyCode, cocos2d::Event* event, NPC *npc);
	void NPCPromptKeyPressed(KeyType keyCode, cocos2d::Event* event, NPC *npc);
	void devilKeyPressed(KeyType keyCode, cocos2d::Event* event, Dungeon *dungeon);
	void shrineKeyPressed(KeyType keyCode, cocos2d::Event* event, Dungeon *dungeon, int choices);
	void inventoryMenuKeyPressed(KeyType keyCode, cocos2d::Event* event, Dungeon *dungeon);
	void itemMenuKeyPressed(KeyType keyCode, cocos2d::Event* event, Dungeon *dungeon);
	void gameOverKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void winnerKeyPressed(KeyType keyCode, cocos2d::Event* event);

	// The menu used for winning, dying, making a choice, etc.
	void constructSelectionMenu();

	void deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites);

	void checkPlayerStats();

	void constructActiveItemHUD();
	void checkActiveItemHUD();
	void updateActiveItemHUD();
	void updateActiveItemBar();
	void deconstructActiveItemHUD();

	void constructItemHUD();
	void checkItemHUD();
	void updateItemHUD();
	void deconstructItemHUD();

	void constructRelicHUD();
	void checkRelicHUD();
	void updateRelicHUD();
	void deconstructRelicHUD();

	void constructWeaponHUD();
	void checkWeaponHUD();
	void updateWeaponHUD();
	void addWeaponCastLabel();
	void removeWeaponCastLabel();

	void deconstructShopHUD();
	void deconstructBossHUD();

	cocos2d::Label* getStatLabel(float x, float y, std::string text, float fontSize);
	void addLabel(float x, float y, std::string text, std::string id, float fontSize);
	//void addKeyLabel(float x, float y, std::string text, std::string id, float fontSize);
	void updateLabel(std::string id, std::string newText);

	// Used to properly enable the gameplay event listener
	void enableListener();
	void menuKeyReleased(KeyType keyCode, cocos2d::Event* event);

	std::map<std::string, cocos2d::Sprite*> HUD;
	std::map<std::string, cocos2d::Label*> labels;
	std::map<std::string, cocos2d::Label*> keyLabels;
	std::multimap<std::string, cocos2d::Sprite*> itemMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> inventoryMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> generalSprites; // For general use
	std::multimap<std::string, cocos2d::Sprite*> menuSprites; // For menu boxes, etc.
	cocos2d::Label* goldcount;
	cocos2d::Label* numericalHP;
	cocos2d::Label* str;
	cocos2d::Label* dex;
	cocos2d::Label* intellect;
	cocos2d::Label* armor;
	cocos2d::Label* moneyBonus;
	cocos2d::Label* qKey = nullptr;

	cocos2d::EventListenerKeyboard* activeListener;
	cocos2d::EventListenerKeyboard* release; // used to properly enable the gameplay event listener

	std::shared_ptr<Player> p;
	int index = 0;
	int quick = 1; // For quick access slot; 1 = at least one item in player inv, 0 = no items

	// holds information about the player's item
	// 1st string: Item name, 2nd string: Item description
	std::pair<std::string, std::string> inventoryText[23];
	cocos2d::Label* itemName;
	cocos2d::Label* itemDescription;

	std::vector<std::string> m_dialogue; // for npc dialogue
	cocos2d::Label* line;
	int lineIndex = 0;

	cocos2d::Label* itemprice = nullptr; // For shop prices
};

class BackgroundLayer : public cocos2d::Layer {
public:
	CREATE_FUNC(BackgroundLayer);
	virtual bool init();

	void updateBackground();

private:
	std::map<std::string, cocos2d::Sprite*> bg;
};

class LevelScene : public cocos2d::Scene {
public:
	LevelScene(HUDLayer* hud, std::shared_ptr<Player> p, int level);
	virtual ~LevelScene();

	static cocos2d::Scene* createScene(std::shared_ptr<Player> p, int level);
	static LevelScene* create(HUDLayer* hud, std::shared_ptr<Player> p, int level);
	virtual bool init();

	void enableListener(bool enable) { kbListener->setEnabled(enable); };
	void callNPCInteraction(NPC &npc, std::vector<std::string> dialogue) { m_hud->NPCInteraction(kbListener, npc, dialogue); };
	void callDevilsWaters() { m_hud->devilsWaters(kbListener, *m_currentDungeon); };
	void callShrineChoice(std::string shrine) { m_hud->shrineChoice(kbListener, shrine, *m_currentDungeon); };

	void returnToMainMenu();

	void scheduleTimer();
	void unscheduleTimer();

	cocos2d::Label* getPriceLabel() const { return m_itemPrice; };
	void showShopHUD(int x, int y);
	void deconstructShopHUD();

	//void graySprite(cocos2d::Sprite* sprite);
	EffectSprite* createEffectSprite(std::string image, int x, int y, int z);

	//cocos2d::Camera* gameCamera;

	/// unused
	//bool isKeyPressed(KeyType);
	//double keyPressedDuration(KeyType);
	//void moveCallback(cocos2d::Ref* pSender);
	//virtual void update(float delta) override;

private:
	void LevelKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void setCurrentDungeon(int level, std::shared_ptr<Player> player);
	void createPlayerSpriteAndCamera();
	void setMusic(int level);

	cocos2d::Sprite* createSprite(std::string image, int x, int y, int z);

	// Actions with a tag of:
	// 1: Single-use action, it is run one time and may be forced to finish early
	// 5: Single animation that should be allowed to finish naturally
	// 10: Animation, should be run forever
	cocos2d::Sprite* createAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);

	void updateLevelLighting();

	float getTimerSpeed();
	void pauseMenu();
	void advanceLevel();

	bool playerAdvanced(int level);

	HUDLayer* m_hud;
	cocos2d::EventListenerKeyboard* kbListener;

	Dungeon *m_currentDungeon = nullptr;
	std::shared_ptr<Player> p;
	cocos2d::Sprite* m_player;
	int m_level;
	int bg_music_id;
	cocos2d::Label *m_itemPrice = nullptr;

	// for menu selection
	int index = 0;

	// For changing direction of the player sprite
	// Only takes on values of 'l' and 'r'
	char m_facing = 'r';

	/// Unused
	static std::map<KeyType, std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	// :::: new lighting stuff ::::
	EffectSprite* m_playerLight;
	LightEffect* m_lighting;
};

class PauseMenuScene : public MenuScene {
public:
	PauseMenuScene(LevelScene *levelScene, std::shared_ptr<Player> p, int id);
	~PauseMenuScene();

	static PauseMenuScene* create(LevelScene *levelScene, std::shared_ptr<Player> p, int id);
	static cocos2d::Scene* createScene(LevelScene *levelScene, std::shared_ptr<Player> p, int id);
	virtual bool init();

	void helpScreen();

	void pauseMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void helpMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);

private:
	LevelScene *m_levelScene = nullptr;

	cocos2d::EventListenerKeyboard* kbListener = nullptr;
	cocos2d::EventListenerKeyboard* pauseSelectListener;

	std::shared_ptr<Player> m_player = nullptr;

	// To adjust the volume of the music if changed
	int music_id;
};


#endif
