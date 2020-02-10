#ifndef GUI_H
#define GUI_H

#include "cocos2d.h"
//#include "LightEffect.h"
//#include "EffectSprite.h"
#include "DynamicLight.h"
#include "DynamicLightManager.h"
#include "Dungeon.h"
#include "Actors.h"

void playFootstepSound();

struct Dungeons {
	Dungeons() { ; };

	Dungeon* DUNGEON1 = nullptr;
	Shop* SHOP1 = nullptr;
	SecondFloor* DUNGEON2 = nullptr;
	ThirdFloor* DUNGEON3 = nullptr;
	FirstBoss* BOSS1 = nullptr;
};

class StartScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	void keyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void startGameCallback(cocos2d::Ref* pSender);
	void exitGameCallback(cocos2d::Ref* pSender);

	CREATE_FUNC(StartScene);
private:
	cocos2d::Sprite* playerSprite;
	int id;
	int index = 0;
};

class HUDLayer : public cocos2d::Layer {
public:
	CREATE_FUNC(HUDLayer);
	virtual bool init();
	//HUDLayer();

	void updateHUD(Dungeon &dungeon);
	void showShopHUD(Dungeon &dungeon, int x, int y);
	void updateShopHUD();
	void showBossHP();
	void updateBossHUD(Dungeon &dungeon);

	void NPCInteraction(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon, std::vector<std::string> dialogue);
	void NPCKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	// For second dungeon
	void devilsWaters(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void devilKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void inventoryMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void inventoryMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void weaponMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void weaponMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void itemMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void itemMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void gameOver();
	void gameOver(cocos2d::Scene &scene);
	void gameOverKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void winner();
	void winnerKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites);
	void constructShieldHUD();
	void deconstructShieldHUD();
	void constructItemHUD();
	void deconstructItemHUD();
	void deconstructTrinketHUD();
	void deconstructShopHUD();
	void deconstructBossHUD();

	// used to properly enable the gameplay event listener
	void enableListener();
	void menuKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	cocos2d::Label* itemprice = nullptr;
private:
	std::map<std::string, cocos2d::Sprite*> HUD;
	std::map<std::string, cocos2d::Label*> labels;
	std::map<std::string, cocos2d::Label*> keyLabels;
	std::multimap<std::string, cocos2d::Sprite*> weaponMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> itemMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> inventoryMenuSprites;
	cocos2d::Label* goldcount;
	cocos2d::Label* numericalHP;
	cocos2d::Label* str = nullptr;
	cocos2d::Label* dex;
	cocos2d::Label* intellect;
	cocos2d::Label* armor;
	cocos2d::Label* moneyBonus;
	cocos2d::Label* qKey = nullptr;

	cocos2d::EventListenerKeyboard* activeListener;
	cocos2d::EventListenerKeyboard* release; // used to properly enable the gameplay event listener

	Player p;
	int index = 0;
	int quick = 1; // For quick access slot; 1 = at least one item in player inv, 0 = no items

	// holds information about the player's item
	// 1st string: Item name, 2nd string: Item description
	std::pair<std::string, std::string> inventoryText[14];
	cocos2d::Label* itemName;
	cocos2d::Label* itemDescription;

	std::vector<std::string> m_dialogue; // for npc dialogue
	cocos2d::Label* line;
	int lineIndex = 0;
};

class BackgroundLayer : public cocos2d::Layer {
public:
	CREATE_FUNC(BackgroundLayer);
	virtual bool init();

	void updateBackground();
private:
	std::map<std::string, cocos2d::Sprite*> bg;
};

class Level1Scene : public cocos2d::Scene {
public:
	Level1Scene(HUDLayer* hud);
	static cocos2d::Scene* createScene();
	static Level1Scene* create(HUDLayer* hud);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	//void graySprite(cocos2d::Sprite* sprite);
	//EffectSprite* createEffectSprite(std::string image, int maxrows, int x, int y, int z);

	void Level1KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void pauseMenu();
	void advanceLevel();


	Dungeons m_dungeons;
	Dungeon DUNGEON;

	Player p;
	//cocos2d::Camera* gameCamera;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;

	//cocos2d::TimerTargetCallback* timer;
	//cocos2d::EventListenerCustom* inactionListener;
	
	int bg_music_id;

	// :::: new lighting stuff ::::
	//EffectSprite* m_playerLight;
	//LightEffect* m_lighting;

	DynamicLightManager* lightManager;


	/// unused
	//bool isKeyPressed(cocos2d::EventKeyboard::KeyCode);
	//double keyPressedDuration(cocos2d::EventKeyboard::KeyCode);
	//void moveCallback(cocos2d::Ref* pSender);

	//virtual void update(float delta) override;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	//cocos2d::EventListenerKeyboard* kbListener;
	
	std::vector<cocos2d::Sprite*> money;
	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;
	std::vector<cocos2d::Sprite*> floors;

	// for menu selection
	int index = 0;
};

class Shop1Scene : public cocos2d::Scene {
public:
	Shop1Scene(HUDLayer* hud, Player p, int level);
	static cocos2d::Scene* createScene(Player p, int level);
	static Shop1Scene* create(HUDLayer* hud, Player p, int level);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	void Shop1KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void showShopHUD(Dungeon &dungeon, int x, int y);
	void deconstructShopHUD();
	cocos2d::Label* itemprice = nullptr;

	void pauseMenu();
	void advanceLevel();

	Player p;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;

	int id;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	Dungeons m_dungeons;

	Shop* SHOP1;
	int m_level;

	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;
	
};

class Level2Scene : public cocos2d::Scene {
public:
	Level2Scene(HUDLayer* hud, Player p);
	static cocos2d::Scene* createScene(Player p);
	static Level2Scene* create(HUDLayer* hud, Player p);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	void Level2KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void pauseMenu();
	void advanceLevel();

	Player p;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;

	int id;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	Dungeons m_dungeons;
	SecondFloor* DUNGEON2;

	std::vector<cocos2d::Sprite*> money;
	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;
	std::vector<cocos2d::Sprite*> floors;
};

class Level3Scene : public cocos2d::Scene {
public:
	Level3Scene(HUDLayer* hud, Player p);
	static cocos2d::Scene* createScene(Player p);
	static Level3Scene* create(HUDLayer* hud, Player p);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	void Level3KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void pauseMenu();
	void advanceLevel();

	Player p;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;

	int id;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	Dungeons m_dungeons;
	ThirdFloor* DUNGEON3;

	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> money;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;
	std::vector<cocos2d::Sprite*> floors;
};


//		BOSS 1 SCENE
class Boss1Scene : public cocos2d::Scene {
public:
	Boss1Scene(HUDLayer* hud, Player p);
	static cocos2d::Scene* createScene(Player p);
	static Boss1Scene* create(HUDLayer* hud, Player p);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	void Boss1KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void pauseMenu();
	void advanceLevel();

	Player p;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;

	int id;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	Dungeons m_dungeons;
	FirstBoss* BOSS1;

	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spike_projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;
};



class PauseMenuScene : public cocos2d::Scene {
public:
	CREATE_FUNC(PauseMenuScene);
	static cocos2d::Scene* createScene();
	virtual bool init();

	void helpScreen();

	void pauseMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void helpMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label *label;

	cocos2d::EventListenerKeyboard* pauseSelectListener;
	cocos2d::EventListenerKeyboard* helpSelectListener;

	Player p;
	int index = 0;
};

class HelpScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	void helpSceneKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	CREATE_FUNC(HelpScene);

private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label *label;
};


#endif
