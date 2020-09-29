#ifndef GUI_H
#define GUI_H

class cocos2d::Ref;
class cocos2d::Scene;
class cocos2d::Sprite;
class cocos2d::Label;
class cocos2d::EventListenerKeyboard;
enum class cocos2d::EventKeyboard::KeyCode;
class cocos2d::Event;
class LevelScene;
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
extern KeyType PAUSE_KEY;

using ButtonType = cocos2d::Controller::Key;

extern ButtonType UP_BUTTON;
extern ButtonType DOWN_BUTTON;
extern ButtonType LEFT_BUTTON;
extern ButtonType RIGHT_BUTTON;
extern ButtonType INTERACT_BUTTON;
extern ButtonType QUICK_BUTTON;
extern ButtonType ACTIVE_BUTTON;
extern ButtonType WEAPON_BUTTON;
extern ButtonType CAST_BUTTON;
extern ButtonType ITEM_BUTTON;
extern ButtonType INVENTORY_BUTTON;
extern ButtonType PAUSE_BUTTON;

extern ButtonType SELECT_BUTTON;
extern ButtonType BACK_BUTTON;

std::string convertKeycodeToStr(KeyType keyCode);
std::string convertButtonToStr(ButtonType button);
KeyType gameplayButtonToKey(ButtonType button);
KeyType menuButtonToKey(ButtonType button);
void registerGamepads();
cocos2d::Controller* fetchGamepad();
static void restartGame(const Player &p);

class MenuScene : public cocos2d::Scene {
public:
	~MenuScene() override;

protected:
	MenuScene();

	void options();

	void addLabel(float x, float y, std::string name, std::string id, float fontSize);
	void updateLabel(std::string id, std::string newText);

	void addSprite(float x, float y, int z, const std::string &image, const std::string &id);
	cocos2d::Sprite* createSelectorSprite(float x, float y);

	void createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node);
	void createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node); // I suspect that the `int` param is actually a ButtonType

	void removeAll();
	void removeLabels();
	void removeSprites();

	void setPrevMenuState();
	void restorePrevMenuState();

	std::map<std::string, cocos2d::Sprite*> sprites;
	std::map<std::string, cocos2d::Label*> labels;

	cocos2d::EventListenerKeyboard* kbListener = nullptr;
	cocos2d::EventListenerController* controllerListener = nullptr;

	int id = -1; // bg music id
	int index = 0; // menu selection

	// int: previous index, Vec2: previous selector position
	std::stack<std::pair<int, cocos2d::Vec2>> prevMenuElements;
	bool m_forward = true; // Flag indicating if the user navigated forward or backward through menus

private:
	void optionKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void optionButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void soundOptions();
	void soundOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void soundOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void videoOptions();
	void videoKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void videoButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);
	void adjustResolution();
	void toggleFullscreen();

	void controlOptions();
	void controlOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void controlOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	// Keyboard controls
	void keyBindings();
	void keyBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void setKey(KeyType keyCode, int index);
	bool keyIsValid(KeyType keyCode, int index); // Checks if there's any conflict with other keys set
	void resetKeyboardBindings();

	// Gamepad Controls
	void controllerBindings();
	void controllerBindingsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);
	void controllerBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void setButton(ButtonType keyCode, int index);
	bool buttonIsValid(ButtonType keyCode, int index);
	void resetControllerBindings();
	
	void languageOptions();
	void languageKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void languageButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	//static std::map<KeyType, std::chrono::high_resolution_clock::time_point> keys;

	std::vector<std::pair<int, int>> resolutions;
	int resolutionIndex = 0; // 
	bool fullScreen = false;

	// For key bindings
	std::vector<KeyType> m_keyboardControls;
	std::vector<ButtonType> m_gamepadControls;
	bool settingKey = false; // False indicates the user is not trying to change the key; True indicates the next keystroke will change the key.
};

class StartScene : public MenuScene {
public:
	static cocos2d::Scene* createScene();

private:
	CREATE_FUNC(StartScene);
	bool init() override;

	void keyPressed(KeyType keyCode, cocos2d::Event* event);

	void characterSelect();
	void characterKeyPressed(KeyType keyCode, cocos2d::Event* event);

	void startGameCallback(cocos2d::Ref* pSender);
	void exitGameCallback(cocos2d::Ref* pSender);

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
	~HUDLayer() override;

	static HUDLayer* create(std::shared_ptr<Player> p);

	void setScene(LevelScene *scene) { m_scene = scene; };

	void updateHUD();
	
	void showBossHP();
	void updateBossHUD();

	void showHUD();
	void hideHUD();

	void NPCInteraction(const NPC &npc);

	// Menus found within the game
	void devilsWaters();
	void shrineChoice(const std::string &shrine);
	void inventoryMenu();
	void itemMenu();
	void gameOver();
	void winner();

private:
	HUDLayer(std::shared_ptr<Player> p);

	bool init() override;

	void NPCKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc);
	void NPCButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc);

	void NPCPrompt(const NPC &npc);
	void NPCPromptKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc);
	void NPCPromptButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc);

	void devilKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void devilButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void shrineKeyPressed(KeyType keyCode, cocos2d::Event* event, int choices);
	void shrineButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, int choices);

	void inventoryMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void inventoryMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void itemMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void itemMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void gameOverKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void gameOverButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void winnerKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void winnerButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);


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

	void createPersistentLabel(float x, float y, const std::string &text, const std::string &id, float fontSize);
	void addLabel(float x, float y, const std::string &text, const std::string &id, float fontSize);
	void updateLabel(const std::string &id, std::string newText);

	void createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node);
	void createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node);

	// Used to properly enable the gameplay event listeners
	void enableGameplayListeners();
	void menuKeyReleased(KeyType keyCode, cocos2d::Event* event);
	void menuButtonReleased(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	LevelScene* m_scene = nullptr;
	std::shared_ptr<Player> m_player;

	std::map<std::string, cocos2d::Sprite*> HUD;
	std::map<std::string, cocos2d::Label*> m_persistentLabels;
	std::map<std::string, cocos2d::Label*> labels;
	std::map<std::string, cocos2d::Label*> keyLabels;
	std::multimap<std::string, cocos2d::Sprite*> itemMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> inventoryMenuSprites;
	std::multimap<std::string, cocos2d::Sprite*> generalSprites; // For general use
	std::multimap<std::string, cocos2d::Sprite*> menuSprites; // For menu boxes, etc.
	
	cocos2d::Label* qKey = nullptr;

	// Used to properly enable the gameplay event listeners.
	cocos2d::EventListenerKeyboard* m_keyRelease;
	cocos2d::EventListenerController* m_buttonRelease;

	// Used for menu selections
	int index = 0;

	// Holds information about the player's items.
	// 1st string: Item name, 2nd string: Item description
	std::pair<std::string, std::string> inventoryText[23];
	cocos2d::Label* m_itemName;
	cocos2d::Label* m_itemDescription;

	// For NPC interactions
	std::vector<std::string> m_dialogue;
	cocos2d::Label* m_line;
	int m_lineIndex = 0;

	cocos2d::Label* m_itemPrice = nullptr; // For shop prices
};

class BackgroundLayer : public cocos2d::Layer {
public:
	CREATE_FUNC(BackgroundLayer);

	void updateBackground();

private:
	bool init() override;

	std::map<std::string, cocos2d::Sprite*> bg;
};

class LevelScene : public cocos2d::Scene {
public:
	~LevelScene() override;

	static cocos2d::Scene* createScene(std::shared_ptr<Player> p, int level);

	void enableListeners() {
		kbListener->setEnabled(true);
		controllerListener->setEnabled(true);
	};
	
	void callNPCInteraction(const NPC &npc) { 
		disableListeners();
		m_hud->NPCInteraction(npc);
	};
	void callDevilsWaters() {
		disableListeners();
		m_hud->devilsWaters();
	};
	void callShrineChoice(const std::string &shrine) { 
		disableListeners();
		m_hud->shrineChoice(shrine);
	};
	void callFactoryTileCreation();

	Dungeon* getCurrentDungeon() const { return m_currentDungeon; };
	int getMusicId() const { return m_musicID; };

	void returnToMainMenu();

	void scheduleTimer();
	void unscheduleTimer();

	cocos2d::Label* getPriceLabel() const { return m_itemPrice; };
	void showShopHUD(int x, int y);
	void deconstructShopHUD();

	//void graySprite(cocos2d::Sprite* sprite);
	EffectSprite* createEffectSprite(std::string image, int x, int y, int z);
	void setLightingOn(EffectSprite* sprite);
	void movePlayerLightTo(float x, float y);

	//cocos2d::Camera* gameCamera;

	/// unused
	//bool isKeyPressed(KeyType);
	//double keyPressedDuration(KeyType);
	//void moveCallback(cocos2d::Ref* pSender);
	//virtual void update(float delta) override;

private:
	LevelScene(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level);

	static LevelScene* create(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level);
	bool init() override;

	void createGameplayListeners();
	void createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node);
	void createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node);

	void disableListeners() {
		kbListener->setEnabled(false);
		controllerListener->setEnabled(false);
	};

	void LevelKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void LevelButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void factoryTileKeyPressed(KeyType keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir);
	void factoryTileButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir);

	void setCurrentDungeon(int level, std::shared_ptr<Player> player);
	void createPlayerSpriteAndCamera();
	void setMusic(int level);

	cocos2d::Sprite* createSprite(std::string image, int x, int y, int z);

	/* Actions with a tag of:
	*  1: Single-use action, it is run one time and may be forced to finish early
	*  5: Single animation that should be allowed to finish naturally
	*  10: Animation, should be run forever */
	cocos2d::Sprite* createAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);

	void createLightEntity();
	void updateLevelLighting();

	float getTimerSpeed();
	void pauseMenu();
	void advanceLevel();

	bool playerAdvanced(int level);

	HUDLayer* m_hud;
	cocos2d::EventListenerKeyboard* kbListener;
	cocos2d::EventListenerController* controllerListener;

	LightEffect* m_lighting;

	Dungeon *m_currentDungeon = nullptr;
	std::shared_ptr<Player> p;
	int m_level;

	int m_musicID;
	cocos2d::Label *m_itemPrice = nullptr;

	// for menu selection
	int index = 0;

	// For changing direction of the player sprite
	// Only takes on values of 'l' and 'r'
	char m_facing = 'r';

	bool m_preventTimerScheduling = false;

	cocos2d::Node* m_cameraLayer;
	cocos2d::Camera* m_gameCamera;
	cocos2d::Follow* m_follow;

	/// Unused
	static std::map<KeyType, std::chrono::high_resolution_clock::time_point> keys;
	cocos2d::Label* label;
};

class PauseMenuScene : public MenuScene {
public:
	static cocos2d::Scene* createScene(LevelScene *levelScene);

private:
	PauseMenuScene(LevelScene *levelScene);

	static PauseMenuScene* create(LevelScene *levelScene);
	bool init() override;

	void resumeGame();

	void helpScreen();

	void pauseMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void pauseMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	void helpMenuKeyPressed(KeyType keyCode, cocos2d::Event* event);
	void helpMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);

	LevelScene *m_levelScene = nullptr;
};


#endif
