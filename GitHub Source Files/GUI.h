#ifndef GUI_H
#define GUI_H

#include "cocos2d.h"
#include "Dungeon.h"
#include "Actors.h"

class StartScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	void startGameCallback(cocos2d::Ref* pSender);
	void exitGameCallback(cocos2d::Ref* pSender);

	CREATE_FUNC(StartScene);
private:
	cocos2d::Sprite* playerSprite;
	int id;
};

class HUDLayer : public cocos2d::Layer {
public:
	CREATE_FUNC(HUDLayer);
	virtual bool init();
	//HUDLayer();

	void updateHUD(Dungeon &dungeon);
	void showBossHP();
	void updateBossHUD(Dungeon &dungeon);

	// For second dungeon
	void devilsWaters(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void devilKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);


	void weaponMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void weaponMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void itemMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon);
	void itemMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon);

	void gameOver();
	void gameOverKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void winner();
	void winnerKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites);
	void deconstructBossHUD();

	// used to properly enable the gameplay event listener
	void enableListener();
	void menuKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
private:
	std::map<std::string, cocos2d::Sprite*> HUD;
	std::map<std::string, cocos2d::Label*> labels;
	std::multimap<std::string, cocos2d::Sprite*> weaponMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> itemMenuSprites;

	cocos2d::EventListenerKeyboard* activeListener;
	cocos2d::EventListenerKeyboard* release; // used to properly enable the gameplay event listener

	Player p;
	int index = 0;
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
	//CREATE_FUNC(Level1Scene);

	Level1Scene(HUDLayer* hud);
	static cocos2d::Scene* createScene();
	static Level1Scene* create(HUDLayer* hud);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);

	// gameplay functions
	void Level1KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void pauseMenu();
	void advanceLevel();

	Dungeon dungeon;
	Player p;
	//cocos2d::Camera* gameCamera;
	cocos2d::Sprite* m_player;
	HUDLayer* m_hud;
	cocos2d::RenderTexture* renderTexture;
	cocos2d::Sprite* gameboard;

	cocos2d::EventListenerKeyboard* kbListener;
	int id;

	// unused
	bool isKeyPressed(cocos2d::EventKeyboard::KeyCode);
	double keyPressedDuration(cocos2d::EventKeyboard::KeyCode);
	void moveCallback(cocos2d::Ref* pSender);

	//virtual void update(float delta) override;
private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;

	//cocos2d::EventListenerKeyboard* kbListener;
	
	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;

	int index = 0;
};

class Level2Scene : public cocos2d::Scene {
public:
	//CREATE_FUNC(Level2Scene);
	Level2Scene(HUDLayer* hud);
	static cocos2d::Scene* createScene();
	static Level2Scene* create(HUDLayer* hud);
	virtual bool init();
	void renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y); // shows dungeon for the first time
	cocos2d::Sprite* createSprite(std::string image, int maxrows, int x, int y, int z);
	void Level2KeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void pauseMenu();
	void advanceLevel();

	bool win(Dungeon &dungeon);

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

	//cocos2d::EventListenerKeyboard* kbListener;

	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
};


//		LEVEL 3 SCENE
class Level3Scene : public cocos2d::Scene {
public:
	Level3Scene(HUDLayer* hud);
	static cocos2d::Scene* createScene();
	static Level3Scene* create(HUDLayer* hud);
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

	std::vector<cocos2d::Sprite*> monsters;
	std::vector<cocos2d::Sprite*> items;
	std::vector<cocos2d::Sprite*> traps;
	std::vector<cocos2d::Sprite*> projectiles;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_buddies;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sparks;
	std::vector<cocos2d::Sprite*> walls;
	std::vector<cocos2d::Sprite*> doors;

};


//		BOSS 1 SCENE
class Boss1Scene : public cocos2d::Scene {
public:
	Boss1Scene(HUDLayer* hud);
	static cocos2d::Scene* createScene();
	static Boss1Scene* create(HUDLayer* hud);
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

class WeaponMenuScene : public cocos2d::Scene {
public:
	WeaponMenuScene(Player player);
	static cocos2d::Scene* createScene(Player player);
	static WeaponMenuScene* create(Player player);

	virtual bool init();

	void weaponMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//CREATE_FUNC(WeaponMenuScene);

private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label *label;

	Player p;
	int index = 0;
};

class ItemMenuScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	void itemMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	CREATE_FUNC(ItemMenuScene);

private:
	static std::map<cocos2d::EventKeyboard::KeyCode,
		std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label *label;

	Player p;
	int index = 0;
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
