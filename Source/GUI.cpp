<<<<<<< HEAD
#include "cocos2d.h"
#include "GUI.h"
#include "AppDelegate.h"
#include "AudioEngine.h"
#include "ui/cocosGUI.h"
#include "global.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "FX.h"
#include "Actors.h"
#include "LightEffect.h"
#include "EffectSprite.h"
#include <cstdlib>
#include <cmath>
#include <vector>

// 1366x768
//int X_OFFSET = 560;
//int Y_OFFSET = 170;
//float SPACING_FACTOR = 51.1f;// 60;
//int MENU_SPACING = 60;
//float RES_ADJUST = 1.08f;
//float HP_BAR_ADJUST = -2;
//float HP_ADJUST = 0;
//float SP_ADJUST = 0;

// 1600x900
//int X_OFFSET = 560;
//int Y_OFFSET = 170;
//int SPACING_FACTOR = 37;
//int MENU_SPACING = 37;
//float RES_ADJUST = 1.23;
//float HP_BAR_ADJUST = -102;
//float HP_ADJUST = -14;
//float SP_ADJUST = -14;

// 1920x1080
int X_OFFSET = 0;// 560;
int Y_OFFSET = 0;// 170;
float SPACING_FACTOR = 44.0f;
int MENU_SPACING = 60;
float RES_ADJUST = 1.0f;
float HP_BAR_ADJUST = 0;
float HP_ADJUST = 0;
float SP_ADJUST = 0;

USING_NS_CC;

// Default Keyboard Controls
KeyType UP_KEY = KeyType::KEY_UP_ARROW;
KeyType DOWN_KEY = KeyType::KEY_DOWN_ARROW;
KeyType LEFT_KEY = KeyType::KEY_LEFT_ARROW;
KeyType RIGHT_KEY = KeyType::KEY_RIGHT_ARROW;
KeyType INTERACT_KEY = KeyType::KEY_E;
KeyType QUICK_KEY = KeyType::KEY_Q;
KeyType ACTIVE_KEY = KeyType::KEY_SPACE;
KeyType WEAPON_KEY = KeyType::KEY_W;
KeyType CAST_KEY = KeyType::KEY_S;
KeyType ITEM_KEY = KeyType::KEY_C;
KeyType INVENTORY_KEY = KeyType::KEY_TAB;
KeyType PAUSE_KEY = KeyType::KEY_P;

// Default Controller Controls
ButtonType UP_BUTTON = ButtonType::BUTTON_DPAD_UP;
ButtonType DOWN_BUTTON = ButtonType::BUTTON_DPAD_DOWN;
ButtonType LEFT_BUTTON = ButtonType::BUTTON_DPAD_LEFT;
ButtonType RIGHT_BUTTON = ButtonType::BUTTON_DPAD_RIGHT;
ButtonType INTERACT_BUTTON = ButtonType::BUTTON_X;
ButtonType QUICK_BUTTON = ButtonType::BUTTON_Y;
ButtonType ACTIVE_BUTTON = ButtonType::BUTTON_A;
ButtonType WEAPON_BUTTON = ButtonType::BUTTON_RIGHT_SHOULDER;
ButtonType CAST_BUTTON = ButtonType::BUTTON_B;
ButtonType ITEM_BUTTON = ButtonType::BUTTON_SELECT;
ButtonType INVENTORY_BUTTON = ButtonType::BUTTON_LEFT_SHOULDER;
ButtonType PAUSE_BUTTON = ButtonType::BUTTON_START;

ButtonType SELECT_BUTTON = ButtonType::BUTTON_A;
ButtonType BACK_BUTTON = ButtonType::BUTTON_B;

static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}
KeyType gameplayButtonToKey(ButtonType button) {
	if (button == LEFT_BUTTON) return LEFT_KEY;
	else if (button == RIGHT_BUTTON) return RIGHT_KEY;
	else if (button == UP_BUTTON) return UP_KEY;
	else if (button == DOWN_BUTTON) return DOWN_KEY;
	else if (button == QUICK_BUTTON) return QUICK_KEY;
	else if (button == CAST_BUTTON) return CAST_KEY;
	else if (button == ACTIVE_BUTTON) return ACTIVE_KEY;
	else if (button == INTERACT_BUTTON) return INTERACT_KEY;
	else if (button == INVENTORY_BUTTON) return INVENTORY_KEY;
	else if (button == WEAPON_BUTTON) return WEAPON_KEY;
	else if (button == ITEM_BUTTON) return ITEM_KEY;
	else return PAUSE_KEY;
}
KeyType menuButtonToKey(ButtonType button) {
	if (button == SELECT_BUTTON) return KeyType::KEY_ENTER;
	else if (button == BACK_BUTTON) return KeyType::KEY_ESCAPE;
	else if (button == UP_BUTTON) return KeyType::KEY_UP_ARROW;
	else if (button == DOWN_BUTTON) return KeyType::KEY_DOWN_ARROW;
	else if (button == LEFT_BUTTON) return KeyType::KEY_LEFT_ARROW;
	else if (button == RIGHT_BUTTON) return KeyType::KEY_RIGHT_ARROW;
	else return KeyType::KEY_END;
}
void registerGamepads() {
	cocos2d::Controller::startDiscoveryController();
}
cocos2d::Controller* fetchGamepad() {
	std::vector<Controller*> controllers = cocos2d::Controller::getAllController();
	if (!controllers.empty() && controllers[0] != nullptr)
		return controllers[0];

	return nullptr;
}
void restartGame(const Player &p) {
	cocos2d::experimental::AudioEngine::stopAll();
	GameTimers::removeAllGameTimers();

	// Resets passives obtained and NPCs seen
	GameTable::initializeTables();

	std::shared_ptr<Player> newPlayer = p.createNewPlayer();

	auto levelScene = LevelScene::createScene(newPlayer, FIRST_FLOOR);

	auto transition = TransitionFade::create(0.50f, levelScene);

	transition->setOnExitCallback([levelScene]() {
		auto scene = dynamic_cast<LevelScene*>(levelScene->getChildByName("Level Scene"));
		scene->scheduleTimer();
	});

	Director::getInstance()->replaceScene(transition);
}

MenuScene::MenuScene() {
	// Fill resolution vector
	resolutions.push_back(std::make_pair(1280, 720));
	resolutions.push_back(std::make_pair(1366, 768));
	resolutions.push_back(std::make_pair(1600, 900));
	resolutions.push_back(std::make_pair(1920, 1080));
}
MenuScene::~MenuScene() {
	removeAll();
}

void MenuScene::options() {
	setPrevMenuState();

	index = 0;
	removeAll();
	
	addLabel(0, 3.0f, fetchMenuText("Options Menu", "Sound"), "Sound", 52);

	addLabel(0, 1.0f, fetchMenuText("Options Menu", "Video"), "Res", 52);

	addLabel(0, -1.0f, fetchMenuText("Options Menu", "Controls"), "Keys", 52);

	addLabel(0, -3.0f, fetchMenuText("Options Menu", "Language"), "Language", 52);

	//std::function<void(KeyType, cocos2d::Event*)> cb = std::bind(&MenuScene::optionKeyPressed, this, std::placeholders::_1, std::placeholders::_2);
	auto selector = createSelectorSprite(-3.5f, 3.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::optionKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::optionButtonPressed, this), selector);
}
void MenuScene::optionKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Sound
	// 1 : Video
	// 2 : Controls
	// 3 : Language

	int maxIndex = 3;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = maxIndex;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, -3.0f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxIndex) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 3.0f * MENU_SPACING + vsh);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = true;

		switch (index) {
			// Sound
		case 0:
			soundOptions();

			break;
			// Video
		case 1:
			videoOptions();

			break;
			// Controls
		case 2:
			controlOptions();

			break;
			// Language
		case 3: 
			languageOptions();
			
			break;	
		}

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		removeAll();
		init();

		m_forward = false;
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::optionButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		optionKeyPressed(key, event);
}

void MenuScene::soundOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	// Sound FX Volume
	addLabel(0, 3.0f, fetchMenuText("Sound Menu", "Sound Effect Volume"), "Sound", 40);
	addLabel(-1.0f, 2.1f, "Lower", "Sound Lower", 30);
	addLabel(0.0f, 2.1f, std::to_string((int)(GLOBAL_SOUND_VOLUME * 100)), "Sound Volume", 30);
	addLabel(1.0f, 2.1f, "Higher", "Sound Higher", 30);

	// Music Volume
	addLabel(0, 0, fetchMenuText("Sound Menu", "Music Volume"), "Music", 40);
	addLabel(-1.0f, -0.9f, "Lower", "Music Lower", 30);
	addLabel(0.0f, -0.9f, std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100)), "Music Volume", 30);
	addLabel(1.0f, -0.9f, "Higher", "Music Higher", 30);

	// UI Volume
	addLabel(0, -3.0f, fetchMenuText("Sound Menu", "UI Volume"), "UI", 40);
	addLabel(-1.0f, -3.9f, "Lower", "UI Lower", 30);
	addLabel(0.0f, -3.9f, std::to_string((int)(GLOBAL_UI_VOLUME * 100)), "UI Volume", 30);
	addLabel(1.0f, -3.9f, "Higher", "UI Higher", 30);

	auto selector = createSelectorSprite(-3.5f, 2.1f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::soundOptionsKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::soundOptionsButtonPressed, this), selector);
}
void MenuScene::soundOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 
	// 0 : Sound Effects
	// 1 : Music
	// 2 : UI

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_SOUND_VOLUME -= 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.1f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_MUSIC_VOLUME -= 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.1f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_UI_VOLUME -= 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.1f)));
			}
		}

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_SOUND_VOLUME += 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.1f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_MUSIC_VOLUME += 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.1f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_UI_VOLUME += 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.1f)));
			}
		}

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		options();
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::soundOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		soundOptionsKeyPressed(key, event);
}

void MenuScene::videoOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addLabel(0.0f, 2.0f, fetchMenuText("Video Menu", "Screen Resolution"), "Res", 40);
	addLabel(0.0f, 1.2f, std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second), "Screen Resolution", 30);
	addLabel(0.0f, 0.4f, fetchMenuText("Video Menu", "Fullscreen"), "Fullscreen", 30);

	auto selector = createSelectorSprite(-3.5f, 2.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::videoKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::videoButtonPressed, this), selector);
}
void MenuScene::videoKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Screen Settings
	// 1 : Resolution
	// 2 : Fullscreen

	static int maxIndex = 2;

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 1) {
			if (resolutionIndex > 0) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex--;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 1) {
			if (resolutionIndex < (int)resolutions.size() - 1) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex++;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index == 1) {
			playInterfaceSound("Select 1.mp3");
			index = 2;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 0.4f * MENU_SPACING + vsh);
		}
		else if (index == 2) {
			playInterfaceSound("Select 1.mp3");
			index = 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.8f * MENU_SPACING);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 1) {
			playInterfaceSound("Select 1.mp3");
			index = 2;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.8f * MENU_SPACING);
		}
		else if (index == 2) {
			playInterfaceSound("Select 1.mp3");
			index = 1;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 1.2f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
			// Video Settings
		case 0:
			index = 1;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 1.2f * MENU_SPACING + vsh);
			break;
			// Resolution select
		case 1:
			adjustResolution();
			break;
			// Fullscreen
		case 2:
			toggleFullscreen();
			break;
		}

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		// Screen Resolution
		if (index == 1 || index == 2) {
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 2.0f * MENU_SPACING + vsh);
		}
		else {
			m_forward = false;

			options();
			restorePrevMenuState();
		}

		return;

	default: break;
	}
}
void MenuScene::videoButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		videoKeyPressed(key, event);
}
void MenuScene::adjustResolution() {
	//switch (resolutionIndex) {
	//		// 1280x720
	//case 0: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 56;
	//	MENU_SPACING = 56;
	//	RES_ADJUST = 1;
	//	HP_BAR_ADJUST = 0;
	//	HP_ADJUST = 0;
	//	SP_ADJUST = 0;
	//	break;
	//}
	//		// 1366x768
	//case 1: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 60;
	//	MENU_SPACING = 60;
	//	RES_ADJUST = 1.08f;
	//	HP_BAR_ADJUST = -2;
	//	HP_ADJUST = 0;
	//	SP_ADJUST = 0;
	//	break;
	//}
	//		// 1600x900
	//case 2: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 37;
	//	MENU_SPACING = 37;
	//	RES_ADJUST = 1.23f;
	//	HP_BAR_ADJUST = -102;
	//	HP_ADJUST = -14;
	//	SP_ADJUST = -14;
	//	break;
	//}
	//		// 1920x1080
	//case 3: {
	//	X_OFFSET = 560;// 840;
	//	Y_OFFSET = 170;// 255;
	//	SPACING_FACTOR = 44;
	//	MENU_SPACING = 45;
	//	RES_ADJUST = 1.5;
	//	HP_BAR_ADJUST = -130;
	//	HP_ADJUST = -14;
	//	SP_ADJUST = -14;
	//	break;
	//}
	//default: break;
	//}

	auto director = Director::getInstance();
	auto glview = dynamic_cast<GLViewImpl*>(director->getOpenGLView());
	std::pair<int, int> res = resolutions[resolutionIndex];

	glview->setWindowed(res.first, res.second);
	if (fullScreen)
		glview->setFullscreen();
}
void MenuScene::toggleFullscreen() {
	auto director = Director::getInstance();
	auto glview = dynamic_cast<GLViewImpl*>(director->getOpenGLView());

	if (!fullScreen)
		glview->setFullscreen();
	else
		glview->setWindowed(resolutions[resolutionIndex].first, resolutions[resolutionIndex].second);

	fullScreen = !fullScreen;
}

void MenuScene::controlOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addLabel(0, 1.0f, fetchMenuText("Controls Menu", "Keyboard"), "Keyboard", 52);

	addLabel(0, -1.0f, fetchMenuText("Controls Menu", "Gamepad"), "Gamepad", 52);

	auto selector = createSelectorSprite(-3.5f, 1.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::controlOptionsKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::controlOptionsButtonPressed, this), selector);
}
void MenuScene::controlOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Sound
	// 1 : Video
	// 2 : Controls
	// 3 : Language

	int maxIndex = 1;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = maxIndex;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, -1.0f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxIndex) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 1.0f * MENU_SPACING + vsh);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = true;

		switch (index) {
			// Keyboard
		case 0:
			keyBindings();
			break;
			// Gamepad
		case 1:
			controllerBindings();
			break;
		}
		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;
		options();

		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::controlOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		controlOptionsKeyPressed(key, event);
}

void MenuScene::keyBindings() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addSprite(0, -0.25f, 2, "Pause_Menu_Border_Red.png", "Menu Border");
	sprites.find("Menu Border")->second->setScale(0.42f);
	sprites.find("Menu Border")->second->setOpacity(170);
	
	addLabel(0, 5.0, fetchMenuText("Controls Menu", "Keyboard Controls"), "Change Keys", 48);

	// Movement
	addLabel(-5.0f, 3.6f, fetchMenuText("Controls Menu", "Up"), "Up", 28);
	addLabel(0, 3.6f, convertKeycodeToStr(UP_KEY), "Up Key", 28);

	addLabel(-5.0f, 2.9f, fetchMenuText("Controls Menu", "Down"), "Down", 28);
	addLabel(0, 2.9f, convertKeycodeToStr(DOWN_KEY), "Down Key", 28);

	addLabel(-5.0f, 2.2f, fetchMenuText("Controls Menu", "Left"), "Left", 28);
	addLabel(0, 2.2f, convertKeycodeToStr(LEFT_KEY), "Left Key", 28);

	addLabel(-5.0f, 1.5f, fetchMenuText("Controls Menu", "Right"), "Right", 28);
	addLabel(0, 1.5f, convertKeycodeToStr(RIGHT_KEY), "Right Key", 28);


	// Interact
	addLabel(-5.0f, 0.8f, fetchMenuText("Controls Menu", "Interact"), "Interact", 28);
	addLabel(0, 0.8f, convertKeycodeToStr(INTERACT_KEY), "Interact Key", 28);


	// Use item
	addLabel(-5.0f, 0.1f, fetchMenuText("Controls Menu", "Quick Item Use"), "Quick", 28);
	addLabel(0, 0.1f, convertKeycodeToStr(QUICK_KEY), "Quick Key", 28);


	// Active Item
	addLabel(-5.0f, -0.6f, fetchMenuText("Controls Menu", "Active Item"), "Active", 28);
	addLabel(0, -0.6f, convertKeycodeToStr(ACTIVE_KEY), "Active Key", 28);


	// Swap weapon
	addLabel(-5.0f, -1.3f, fetchMenuText("Controls Menu", "Switch Weapon"), "Weapon", 28);
	addLabel(0, -1.3f, convertKeycodeToStr(WEAPON_KEY), "Weapon Key", 28);


	// Cast weapon
	addLabel(-5.0f, -2.0f, fetchMenuText("Controls Menu", "Cast Weapon"), "Cast", 28);
	addLabel(0, -2.0f, convertKeycodeToStr(CAST_KEY), "Cast Key", 28);


	// Open/close item menu
	addLabel(-5.0f, -2.7f, fetchMenuText("Controls Menu", "Item Menu"), "Item", 28);
	addLabel(0, -2.7f, convertKeycodeToStr(ITEM_KEY), "Item Key", 28);


	// View inventory
	addLabel(-5.0f, -3.4f, fetchMenuText("Controls Menu", "Inventory"), "Inventory", 28);
	addLabel(0, -3.4f, convertKeycodeToStr(INVENTORY_KEY), "Inventory Key", 28);


	addLabel(-5.0f, -4.1f, fetchMenuText("Controls Menu", "Pause"), "Pause", 28);
	addLabel(0, -4.1f, convertKeycodeToStr(PAUSE_KEY), "Pause Key", 28);


	// Reset to Defaults
	addLabel(0, -5.3f, fetchMenuText("Controls Menu", "Reset To Default"), "Default", 36);

	auto selector = createSelectorSprite(-3.0f, 3.6f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::keyBindingsKeyPressed, this), selector);
}
void MenuScene::keyBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	if (settingKey) {
		setKey(keyCode, index);

		// If the key was set successfully, move the arrow back
		if (!settingKey)
			event->getCurrentTarget()->setPosition(pos.x - 1.2 * MENU_SPACING, pos.y);

		return;
	}

	int maxKeyIndex = 11;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index == 0) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, -5.3f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 0 && index <= maxKeyIndex) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = maxKeyIndex;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxKeyIndex) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 3.6f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}

		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:

		playInterfaceSound("Confirm 1.mp3");

		if (index == maxKeyIndex + 1) {
			resetKeyboardBindings();
			return;
		}

		if (!settingKey) {
			settingKey = true;
			event->getCurrentTarget()->setPosition(pos.x + 1.2 * MENU_SPACING, pos.y);
		}

		break;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		
		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		break;

	default:
		break;
	}
}
void MenuScene::setKey(KeyType keyCode, int index) {

	switch (index) {
		// Up
	case 0: {
		if (keyIsValid(keyCode, index)) {
			UP_KEY = keyCode;
			labels.find("Up Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Down
	case 1: {
		if (keyIsValid(keyCode, index)) {
			DOWN_KEY = keyCode;
			labels.find("Down Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Left
	case 2: {
		if (keyIsValid(keyCode, index)) {
			LEFT_KEY = keyCode;
			labels.find("Left Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Right
	case 3: {
		if (keyIsValid(keyCode, index)) {
			RIGHT_KEY = keyCode;
			labels.find("Right Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Interact
	case 4: {
		if (keyIsValid(keyCode, index)) {
			INTERACT_KEY = keyCode;
			labels.find("Interact Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Quick
	case 5: {
		if (keyIsValid(keyCode, index)) {
			QUICK_KEY = keyCode;
			labels.find("Quick Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Active
	case 6: {
		if (keyIsValid(keyCode, index)) {
			ACTIVE_KEY = keyCode;
			labels.find("Active Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Weapon
	case 7: {
		if (keyIsValid(keyCode, index)) {
			WEAPON_KEY = keyCode;
			labels.find("Weapon Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Cast
	case 8: {
		if (keyIsValid(keyCode, index)) {
			CAST_KEY = keyCode;
			labels.find("Cast Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Item Menu
	case 9: {
		if (keyIsValid(keyCode, index)) {
			ITEM_KEY = keyCode;
			labels.find("Item Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Inventory
	case 10: {
		if (keyIsValid(keyCode, index)) {
			INVENTORY_KEY = keyCode;
			labels.find("Inventory Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Pause
	case 11: {
		if (keyIsValid(keyCode, index)) {
			PAUSE_KEY = keyCode;
			labels.find("Pause Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
	default: break;
	}

	playInterfaceSound("Confirm 1.mp3");
	m_keyboardControls.clear();
}
bool MenuScene::keyIsValid(KeyType keyCode, int index) {
	m_keyboardControls.push_back(UP_KEY);
	m_keyboardControls.push_back(DOWN_KEY);
	m_keyboardControls.push_back(LEFT_KEY);
	m_keyboardControls.push_back(RIGHT_KEY);
	m_keyboardControls.push_back(INTERACT_KEY);
	m_keyboardControls.push_back(QUICK_KEY);
	m_keyboardControls.push_back(ACTIVE_KEY);
	m_keyboardControls.push_back(WEAPON_KEY);
	m_keyboardControls.push_back(CAST_KEY);
	m_keyboardControls.push_back(ITEM_KEY);
	m_keyboardControls.push_back(INVENTORY_KEY);
	m_keyboardControls.push_back(PAUSE_KEY);

	for (unsigned int i = 0; i < m_keyboardControls.size(); ++i) {
		if (i == index)
			continue;

		if (m_keyboardControls[i] == keyCode)
			return false;
	}

	// M, I, and Escape key are off limits
	if (keyCode == KeyType::KEY_ESCAPE ||
		keyCode == KeyType::KEY_M ||
		keyCode == KeyType::KEY_I)
		return false;

	settingKey = false;
	return true;
}
void MenuScene::resetKeyboardBindings() {
	UP_KEY = KeyType::KEY_UP_ARROW;
	DOWN_KEY = KeyType::KEY_DOWN_ARROW;
	LEFT_KEY = KeyType::KEY_LEFT_ARROW;
	RIGHT_KEY = KeyType::KEY_RIGHT_ARROW;
	INTERACT_KEY = KeyType::KEY_E;
	QUICK_KEY = KeyType::KEY_Q;
	ACTIVE_KEY = KeyType::KEY_SPACE;
	WEAPON_KEY = KeyType::KEY_W;
	CAST_KEY = KeyType::KEY_S;
	ITEM_KEY = KeyType::KEY_C;
	INVENTORY_KEY = KeyType::KEY_TAB;
	PAUSE_KEY = KeyType::KEY_P;

	labels.find("Up Key")->second->setString(convertKeycodeToStr(UP_KEY));
	labels.find("Down Key")->second->setString(convertKeycodeToStr(DOWN_KEY));
	labels.find("Left Key")->second->setString(convertKeycodeToStr(LEFT_KEY));
	labels.find("Right Key")->second->setString(convertKeycodeToStr(RIGHT_KEY));
	labels.find("Interact Key")->second->setString(convertKeycodeToStr(INTERACT_KEY));
	labels.find("Quick Key")->second->setString(convertKeycodeToStr(QUICK_KEY));
	labels.find("Active Key")->second->setString(convertKeycodeToStr(ACTIVE_KEY));
	labels.find("Weapon Key")->second->setString(convertKeycodeToStr(WEAPON_KEY));
	labels.find("Cast Key")->second->setString(convertKeycodeToStr(CAST_KEY));
	labels.find("Item Key")->second->setString(convertKeycodeToStr(ITEM_KEY));
	labels.find("Inventory Key")->second->setString(convertKeycodeToStr(INVENTORY_KEY));
	labels.find("Pause Key")->second->setString(convertKeycodeToStr(PAUSE_KEY));
}

void MenuScene::controllerBindings() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addSprite(0, -0.25f, 2, "Pause_Menu_Border_Red.png", "Menu Border");
	sprites.find("Menu Border")->second->setScale(0.42f);
	sprites.find("Menu Border")->second->setOpacity(170);

	addLabel(0, 5.0, fetchMenuText("Controls Menu", "Gamepad Controls"), "Change Keys", 48);

	// Movement
	addLabel(-5.0f, 3.6f, fetchMenuText("Controls Menu", "Up"), "Up", 28);
	addLabel(0, 3.6f, convertButtonToStr(UP_BUTTON), "Up Key", 28);

	addLabel(-5.0f, 2.9f, fetchMenuText("Controls Menu", "Down"), "Down", 28);
	addLabel(0, 2.9f, convertButtonToStr(DOWN_BUTTON), "Down Key", 28);

	addLabel(-5.0f, 2.2f, fetchMenuText("Controls Menu", "Left"), "Left", 28);
	addLabel(0, 2.2f, convertButtonToStr(LEFT_BUTTON), "Left Key", 28);

	addLabel(-5.0f, 1.5f, fetchMenuText("Controls Menu", "Right"), "Right", 28);
	addLabel(0, 1.5f, convertButtonToStr(RIGHT_BUTTON), "Right Key", 28);


	// Interact
	addLabel(-5.0f, 0.8f, fetchMenuText("Controls Menu", "Interact"), "Interact", 28);
	addLabel(0, 0.8f, convertButtonToStr(INTERACT_BUTTON), "Interact Key", 28);


	// Use item
	addLabel(-5.0f, 0.1f, fetchMenuText("Controls Menu", "Quick Item Use"), "Quick", 28);
	addLabel(0, 0.1f, convertButtonToStr(QUICK_BUTTON), "Quick Key", 28);


	// Active Item
	addLabel(-5.0f, -0.6f, fetchMenuText("Controls Menu", "Active Item"), "Active", 28);
	addLabel(0, -0.6f, convertButtonToStr(ACTIVE_BUTTON), "Active Key", 28);


	// Swap weapon
	addLabel(-5.0f, -1.3f, fetchMenuText("Controls Menu", "Switch Weapon"), "Weapon", 28);
	addLabel(0, -1.3f, convertButtonToStr(WEAPON_BUTTON), "Weapon Key", 28);


	// Cast weapon
	addLabel(-5.0f, -2.0f, fetchMenuText("Controls Menu", "Cast Weapon"), "Cast", 28);
	addLabel(0, -2.0f, convertButtonToStr(CAST_BUTTON), "Cast Key", 28);


	// Open/close item menu
	addLabel(-5.0f, -2.7f, fetchMenuText("Controls Menu", "Item Menu"), "Item", 28);
	addLabel(0, -2.7f, convertButtonToStr(ITEM_BUTTON), "Item Key", 28);


	// View inventory
	addLabel(-5.0f, -3.4f, fetchMenuText("Controls Menu", "Inventory"), "Inventory", 28);
	addLabel(0, -3.4f, convertButtonToStr(INVENTORY_BUTTON), "Inventory Key", 28);


	addLabel(-5.0f, -4.1f, fetchMenuText("Controls Menu", "Pause"), "Pause", 28);
	addLabel(0, -4.1f, convertButtonToStr(PAUSE_BUTTON), "Pause Key", 28);


	// Reset to Defaults
	addLabel(0, -5.3f, fetchMenuText("Controls Menu", "Reset To Default"), "Default", 36);

	auto selector = createSelectorSprite(-3.0f, 3.6f);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::controllerBindingsButtonPressed, this), selector);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::controllerBindingsKeyPressed, this), selector);
}
void MenuScene::controllerBindingsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	if (settingKey) {
		setButton(static_cast<ButtonType>(keyCode), index);

		// If the key was set successfully, move the arrow back
		if (!settingKey)
			event->getCurrentTarget()->setPosition(pos.x - 1.2 * MENU_SPACING, pos.y);

		return;
	}

	int maxKeyIndex = 11;

	switch (keyCode) {
	case ButtonType::BUTTON_DPAD_UP: {
		if (index == 0) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, -5.3f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		if (index > 0 && index <= maxKeyIndex) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = maxKeyIndex;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		
		break;
	}
	case ButtonType::BUTTON_DPAD_DOWN: {
		if (index < maxKeyIndex) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 3.6f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}

		break;
	}
	case ButtonType::BUTTON_A:

		playInterfaceSound("Confirm 1.mp3");

		if (index == maxKeyIndex + 1) {
			resetKeyboardBindings();
			return;
		}

		if (!settingKey) {
			settingKey = true;
			event->getCurrentTarget()->setPosition(pos.x + 1.2 * MENU_SPACING, pos.y);
		}

		break;

	case ButtonType::BUTTON_B:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		break;

	default:
		break;
	}
}
void MenuScene::controllerBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		return;
	}
}
void MenuScene::setButton(ButtonType keyCode, int index) {
	switch (index) {
		// Up
	case 0: {
		if (buttonIsValid(keyCode, index)) {
			UP_BUTTON = keyCode;
			labels.find("Up Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Down
	case 1: {
		if (buttonIsValid(keyCode, index)) {
			DOWN_BUTTON = keyCode;
			labels.find("Down Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Left
	case 2: {
		if (buttonIsValid(keyCode, index)) {
			LEFT_BUTTON = keyCode;
			labels.find("Left Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Right
	case 3: {
		if (buttonIsValid(keyCode, index)) {
			RIGHT_BUTTON = keyCode;
			labels.find("Right Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Interact
	case 4: {
		if (buttonIsValid(keyCode, index)) {
			INTERACT_BUTTON = keyCode;
			labels.find("Interact Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Quick
	case 5: {
		if (buttonIsValid(keyCode, index)) {
			QUICK_BUTTON = keyCode;
			labels.find("Quick Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Active
	case 6: {
		if (buttonIsValid(keyCode, index)) {
			ACTIVE_BUTTON = keyCode;
			labels.find("Active Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Weapon
	case 7: {
		if (buttonIsValid(keyCode, index)) {
			WEAPON_BUTTON = keyCode;
			labels.find("Weapon Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Cast
	case 8: {
		if (buttonIsValid(keyCode, index)) {
			CAST_BUTTON = keyCode;
			labels.find("Cast Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Item Menu
	case 9: {
		if (buttonIsValid(keyCode, index)) {
			ITEM_BUTTON = keyCode;
			labels.find("Item Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Inventory
	case 10: {
		if (buttonIsValid(keyCode, index)) {
			INVENTORY_BUTTON = keyCode;
			labels.find("Inventory Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Pause
	case 11: {
		if (buttonIsValid(keyCode, index)) {
			PAUSE_BUTTON = keyCode;
			labels.find("Pause Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
	default: break;
	}

	playInterfaceSound("Confirm 1.mp3");
	m_gamepadControls.clear();
}
bool MenuScene::buttonIsValid(ButtonType keyCode, int index) {
	m_gamepadControls.push_back(UP_BUTTON);
	m_gamepadControls.push_back(DOWN_BUTTON);
	m_gamepadControls.push_back(LEFT_BUTTON);
	m_gamepadControls.push_back(RIGHT_BUTTON);
	m_gamepadControls.push_back(INTERACT_BUTTON);
	m_gamepadControls.push_back(QUICK_BUTTON);
	m_gamepadControls.push_back(ACTIVE_BUTTON);
	m_gamepadControls.push_back(WEAPON_BUTTON);
	m_gamepadControls.push_back(CAST_BUTTON);
	m_gamepadControls.push_back(ITEM_BUTTON);
	m_gamepadControls.push_back(INVENTORY_BUTTON);
	m_gamepadControls.push_back(PAUSE_BUTTON);

	for (unsigned int i = 0; i < m_gamepadControls.size(); ++i) {
		if (i == index)
			continue;

		if (m_gamepadControls[i] == keyCode)
			return false;
	}

	settingKey = false;
	return true;
}
void MenuScene::resetControllerBindings() {
	UP_BUTTON = ButtonType::BUTTON_DPAD_UP;
	DOWN_BUTTON = ButtonType::BUTTON_DPAD_DOWN;
	LEFT_BUTTON = ButtonType::BUTTON_DPAD_LEFT;
	RIGHT_BUTTON = ButtonType::BUTTON_DPAD_RIGHT;
	INTERACT_BUTTON = ButtonType::BUTTON_X;
	QUICK_BUTTON = ButtonType::BUTTON_Y;
	ACTIVE_BUTTON = ButtonType::BUTTON_A;
	WEAPON_BUTTON = ButtonType::BUTTON_RIGHT_SHOULDER;
	CAST_BUTTON = ButtonType::BUTTON_B;
	ITEM_BUTTON = ButtonType::BUTTON_SELECT;
	INVENTORY_BUTTON = ButtonType::BUTTON_LEFT_SHOULDER;
	PAUSE_BUTTON = ButtonType::BUTTON_START;

	labels.find("Up Key")->second->setString(convertButtonToStr(UP_BUTTON));
	labels.find("Down Key")->second->setString(convertButtonToStr(DOWN_BUTTON));
	labels.find("Left Key")->second->setString(convertButtonToStr(LEFT_BUTTON));
	labels.find("Right Key")->second->setString(convertButtonToStr(RIGHT_BUTTON));
	labels.find("Interact Key")->second->setString(convertButtonToStr(INTERACT_BUTTON));
	labels.find("Quick Key")->second->setString(convertButtonToStr(QUICK_BUTTON));
	labels.find("Active Key")->second->setString(convertButtonToStr(ACTIVE_BUTTON));
	labels.find("Weapon Key")->second->setString(convertButtonToStr(WEAPON_BUTTON));
	labels.find("Cast Key")->second->setString(convertButtonToStr(CAST_BUTTON));
	labels.find("Item Key")->second->setString(convertButtonToStr(ITEM_BUTTON));
	labels.find("Inventory Key")->second->setString(convertButtonToStr(INVENTORY_BUTTON));
	labels.find("Pause Key")->second->setString(convertButtonToStr(PAUSE_BUTTON));
}

void MenuScene::languageOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	auto selector = createSelectorSprite(-3.5f, 3.0f);

	addLabel(0.0f, 3.0f, fetchMenuText("Language Menu", "English"), "English", 40);

	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::languageKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::languageButtonPressed, this), selector);
}
void MenuScene::languageKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		/*if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}*/

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		/*if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}*/
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
		case 0: GAME_TEXT_FILE = EN_US_FILE; break;
		default: GAME_TEXT_FILE = EN_US_FILE; break;
		}

		TextUtils::initJsonText();

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		options();
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::languageButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		languageKeyPressed(key, event);
}

void MenuScene::setPrevMenuState() {
	if (m_forward) {
		auto it = sprites.find("Selector");
		std::pair<int, cocos2d::Vec2> prevEls = std::make_pair(index, Vec2(it->second->getPositionX(), it->second->getPositionY()));
		prevMenuElements.push(prevEls);
	}
}
void MenuScene::restorePrevMenuState() {
	std::pair<int, cocos2d::Vec2> prevEls = prevMenuElements.top();
	prevMenuElements.pop();

	index = prevEls.first;
	sprites.find("Selector")->second->setPosition(prevEls.second);
}

void MenuScene::addLabel(float x, float y, std::string name, std::string id, float fontSize) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto label = Label::createWithTTF(name, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	label->setOpacity(230);
	label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	label->setAdditionalKerning(0.25f);
	this->addChild(label, 4);
	labels.insert(std::make_pair(id, label));
}
void MenuScene::updateLabel(std::string id, std::string newText) {
	labels.find(id)->second->setString(newText);
}
void MenuScene::addSprite(float x, float y, int z, const std::string &image, const std::string &id) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto sprite = Sprite::create(image);
	sprite->setPosition(x, y);
	//sprite->setScale(0.8f);
	this->addChild(sprite, z);
	sprites.insert(std::make_pair(id, sprite));
}
cocos2d::Sprite* MenuScene::createSelectorSprite(float x, float y) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(x, y);
	selector->setScale(3.0f);
	this->addChild(selector, 4);
	sprites.insert(std::make_pair("Selector", selector));

	return selector;
}

void MenuScene::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, node);
}
void MenuScene::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}

void MenuScene::removeAll() {
	removeLabels();
	removeSprites();
};
void MenuScene::removeLabels() {
	for (auto &it : labels)
		it.second->removeFromParent();
	
	labels.clear();
};
void MenuScene::removeSprites() {
	for (auto &it : sprites)
		it.second->removeFromParent();
	
	sprites.clear();
};


//		START SCENE
Scene* StartScene::createScene()
{
	TextUtils::initJsonText();
	registerGamepads();

	return StartScene::create();
}

bool StartScene::init() {
	if (!Scene::init())
		return false;
	
	// if music hasn't started already, play it
	if (id == -1)
		id = playMusic("PetterTheSturgeon - Anything_1.waw_.mp3", true);
	

	// Fill resolution vector
	/*resolutions.push_back(std::make_pair(1280, 720));
	resolutions.push_back(std::make_pair(1366, 768));
	resolutions.push_back(std::make_pair(1600, 900));
	resolutions.push_back(std::make_pair(1920, 1080));*/

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Title
	auto title1 = Label::createWithTTF("Lost Arts", TEXT_FONT, 72);
	title1->setPosition(Vec2(vsw, 2.5 * MENU_SPACING + vsh));
	title1->setTextColor(cocos2d::Color4B(224, 224, 224, 255));
	title1->enableOutline(cocos2d::Color4B(50, 55, 55, 255), 1);
	this->addChild(title1, 1);
	labels.insert(std::make_pair("title", title1));

	// background pic
	auto background = Sprite::create("super_pixel_cave_wallpaper_C.png");
	background->setPosition(Vec2(vsw, vsh));
	float scale = 1.1f * RES_ADJUST;
	background->setScale(scale);
	this->addChild(background, -5);

	// Start
	/*auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
	float x = vsw;
	float y = -0.25f * MENU_SPACING + vsh;
	start->setPosition(Vec2(x, y));
	start->setScale(0.8f);
	this->addChild(start, 3);
	sprites.insert(std::make_pair("start", start));*/

	addLabel(0, -0.25f, fetchMenuText("Start Menu", "Start"), "Start", 52);

	// Options
	addLabel(0, -1.75f, fetchMenuText("Start Menu", "Options"), "Options", 52);

	// Exit
	addLabel(0, -3.25f, fetchMenuText("Start Menu", "Exit"), "Exit", 52);
	/*auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButton1.png");
	x = visibleSize.width / 2;
	y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
	exitGame->setPosition(Vec2(x, y));
	this->addChild(exitGame, 3);
	sprites.insert(std::make_pair("exitGame", exitGame));*/

	auto selector = createSelectorSprite(-2.5f, -0.25f);
	createKeyboardEventListener(CC_CALLBACK_2(StartScene::keyPressed, this), selector);

	/*kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::keyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, selector);*/


	/*

	// MENU
	auto beginGame = MenuItemImage::create(
		"StartButton1.png",
		"StartButtonPressed1.png",
		CC_CALLBACK_1(StartScene::startGameCallback, this));
	

	if (beginGame == nullptr ||
		beginGame->getContentSize().width <= 0 ||
		beginGame->getContentSize().height <= 0)
	{
		problemLoading("'StartButton.png' and 'StartButtonPressed.png'");
	}
	else
	{
		float x = visibleSize.width / 2;
		float y = visibleSize.height / 2 - (1.2 * MENU_SPACING);
		beginGame->setPosition(Vec2(x, y));
	}


	auto exitGame = MenuItemImage::create(
		"ExitButton1.png",
		"ExitButtonPressed1.png",
		CC_CALLBACK_1(StartScene::exitGameCallback, this));

	if (exitGame == nullptr ||
		exitGame->getContentSize().width <= 0 ||
		exitGame->getContentSize().height <= 0)
	{
		problemLoading("'ExitButton1.png' and 'ExitButtonPressed1.png'");
	}
	else
	{
		float x = visibleSize.width / 2;
		float y = visibleSize.height / 2 - 3 * MENU_SPACING;
		exitGame->setPosition(Vec2(x, y));
	}

	auto startMenu = Menu::create(beginGame, exitGame, NULL);
	startMenu->setPosition(Vec2::ZERO);
	this->addChild(startMenu);
	*/

	return true;
}
void StartScene::keyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 2) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
		case 0: { // Start
			playInterfaceSound("Confirm 1.mp3");

			characterSelect();

			return;
		}
				// Options
		case 1:
			playInterfaceSound("Confirm 1.mp3");
			
			m_forward = true;
			options();

			return;
		case 2: // Exit
			playInterfaceSound("Confirm 1.mp3");

			/*auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButtonPressed1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
			exitGame->setPosition(Vec2(x, y));
			this->addChild(exitGame, 3);*/

			exitGameCallback(this);

			return;
		}
	default: break;
	}
}

void StartScene::characterSelect() {
	index = 0;
	removeAll(); // remove previous labels and sprites

	setCharacterSelectPositions(); // Initialize degrees

	float x, y;

	addLabel(0, 4, fetchMenuText("Character Select Menu", "Select Character"), "Select Character", 54);

	// Adventurer
	polarToCartesian(degree1, x, y);
	addCharacterSprite(x, y, "Player1_48x48.png", "p1");
	sprites.find("p1")->second->setScale(2.5f);

	// Spellcaster
	polarToCartesian(degree2, x, y);
	addCharacterSprite(x, y, "Spellcaster_48x48.png", "p2");

	// Acrobat
	polarToCartesian(degree3, x, y);
	addCharacterSprite(x, y, "OutsideMan2.png", "p3");

	// Spelunker
	polarToCartesian(degree4, x, y);
	addCharacterSprite(x, y, "Spelunker_48x48.png", "p4");

	// Madman
	polarToCartesian(degree5, x, y);
	addCharacterSprite(x, y, "Madman_48x48.png", "p5");

	displayCharacterInfo();

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::characterKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, sprites.find("p1")->second); // check this for player
}
void StartScene::characterKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index > 0)
			index--;
		else
			index = 4;

		rotate(false);

		playInterfaceSound("Select 1.mp3");
		
		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index < 4)
			index++;
		else
			index = 0;
	
		rotate(true);

		playInterfaceSound("Select 1.mp3");
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
		case 0:	characterID = ID_ADVENTURER; break;
		case 1: characterID = ID_SPELLCASTER; break;
		case 2: characterID = ID_ACROBAT; break;
		case 3: characterID = ID_SPELUNKER; break;
		case 4: characterID = ID_MADMAN; break;
		}

		startGameCallback(this);

		index = 0;
		return;
		
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		index = 0;

		removeAll();
		init();

		return;

	default: break;
	}
}

float StartScene::degreeToRadian(int degree) {
	static const double pi = 4 * std::atan(1);
	return degree * pi / 180.0;
}
void StartScene::polarToCartesian(int degree, float &x, float &y) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	float a, r, b, theta;
	r = 90;
	a = 2.25f;
	b = 0.85f;

	theta = degreeToRadian(degree);
	x = r * a * cos(theta) + vsw;
	y = r * b * sin(theta) + vsh;
}
void StartScene::rotate(bool clockwise) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	int scalingFactor = 1;

	if (clockwise)
		scalingFactor = -1;

	float x, y;

	cocos2d::Vector<FiniteTimeAction*> vec1, vec2, vec3, vec4, vec5;

	for (int i = 0; i < 72; i++) {
		polarToCartesian(degree1, x, y);
		vec1.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree1 += scalingFactor);

		polarToCartesian(degree2, x, y);
		vec2.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree2 += scalingFactor);

		polarToCartesian(degree3, x, y);
		vec3.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree3 += scalingFactor);

		polarToCartesian(degree4, x, y);
		vec4.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree4 += scalingFactor);

		polarToCartesian(degree5, x, y);
		vec5.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree5 += scalingFactor);
	}

	const float largeScaling = 2.5f;
	const float smallScaling = 1.0f;

	cocos2d::Sequence* seq1 = cocos2d::Sequence::create(vec1);
	cocos2d::Sequence* seq2 = cocos2d::Sequence::create(vec2);
	cocos2d::Sequence* seq3 = cocos2d::Sequence::create(vec3);
	cocos2d::Sequence* seq4 = cocos2d::Sequence::create(vec4);
	cocos2d::Sequence* seq5 = cocos2d::Sequence::create(vec5);

	sprites.find("p1")->second->runAction(seq1);
	sprites.find("p1")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 0 ? largeScaling : smallScaling));
	sprites.find("p2")->second->runAction(seq2);
	sprites.find("p2")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 1 ? largeScaling : smallScaling));

	sprites.find("p3")->second->runAction(seq3);
	sprites.find("p3")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 2 ? largeScaling : smallScaling));
	sprites.find("p4")->second->runAction(seq4);
	sprites.find("p4")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 3 ? largeScaling : smallScaling));

	sprites.find("p5")->second->runAction(seq5);
	sprites.find("p5")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 4 ? largeScaling : smallScaling));

	displayCharacterInfo();
}
void StartScene::displayCharacterInfo() {
	if (labels.find("Character Name") == labels.end()) {
		addLabel(0, -3.0f, fetchMenuText("Character Select Menu", "Adventurer"), "Character Name", 32);
		addLabel(0, -3.8f, fetchMenuText("Character Select Menu", "Adventurer Desc"), "Character Desc", 24);
		return;
	}

	switch (index) {
	case 0: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Adventurer"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Adventurer Desc"));
		break;
	}
	case 1: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Spellcaster"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Spellcaster Desc"));
		break;
	}
	case 2: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Acrobat"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Acrobat Desc"));
		break;
	}
	case 3: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Spelunker"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Spelunker Desc"));
		break;
	}
	case 4: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Madman"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Madman Desc"));
		break;
	}
	}
}
void StartScene::setAngle(int &angle) {
	while (angle < 0)
		angle += 360;

	angle = angle % 360;
}
void StartScene::setCharacterSelectPositions() {
	// If 4 characters
	/*degree1 = 270;
	degree2 = 0;
	degree3 = 90;
	degree4 = 180;*/

	// If 5 characters
	degree1 = 270;
	degree2 = 342;
	degree3 = 54;
	degree4 = 126;
	degree5 = 198;
}
void StartScene::addCharacterSprite(float x, float y, std::string image, std::string id) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto s = Sprite::createWithSpriteFrameName(image);
	s->setPosition(x, y);
	s->setScale(1.0f);
	this->addChild(s, 1);
	sprites.insert(std::make_pair(id, s));
}

void StartScene::startGameCallback(Ref* pSender) {
	playInterfaceSound("Confirm 1.mp3");
	cocos2d::experimental::AudioEngine::stop(id);

	GameTable::initializeTables();

	std::shared_ptr<Player> p = nullptr;
	switch (characterID) {
	case ID_ADVENTURER: p = std::make_shared<Adventurer>(); break;
	case ID_SPELLCASTER: p = std::make_shared<Spellcaster>(); break;
	case ID_MADMAN: p = std::make_shared<TheMadman>(); break;
	case ID_SPELUNKER: p = std::make_shared<Spelunker>(); break;
	case ID_ACROBAT: p = std::make_shared<Acrobat>(); break;
	default: p = std::make_shared<Adventurer>(); break;
	}
	
	auto levelScene = LevelScene::createScene(p, TUTORIAL);

	auto transition = TransitionFade::create(0.75f, levelScene);

	transition->setOnExitCallback([levelScene]() {
		auto scene = dynamic_cast<LevelScene*>(levelScene->getChildByName("Level Scene"));
		scene->scheduleTimer();
	});

	Director::getInstance()->replaceScene(transition);
}
void StartScene::exitGameCallback(Ref* pSender)
{
	playInterfaceSound("Cancel 1.mp3");

	//Close the cocos2d-x game scene and quit the application
	Director::getInstance()->end();

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

	//EventCustom customEndEvent("game_scene_close_event");
	//_eventDispatcher->dispatchEvent(&customEndEvent);
}


// HUD LAYER
HUDLayer::HUDLayer(std::shared_ptr<Player> p) : m_player(p) {
	
}
HUDLayer::~HUDLayer() {
	if (qKey)
		qKey->removeFromParent();

	for (auto &it : HUD)
		it.second->removeFromParent();

	for (auto &it : m_persistentLabels)
		it.second->removeFromParent();

	for (auto &it : labels)
		it.second->removeFromParent();

	for (auto &it : keyLabels)
		it.second->removeFromParent();

	for (auto &it : itemMenuSprites)
		it.second->removeFromParent();

	for (auto &it : inventoryMenuSprites)
		it.second->removeFromParent();

	for (auto &it : generalSprites)
		it.second->removeFromParent();

	for (auto &it : menuSprites)
		it.second->removeFromParent();

	if (m_player)
		m_player.reset();
}

HUDLayer* HUDLayer::create(std::shared_ptr<Player> p) {
	HUDLayer *pRet = new(std::nothrow) HUDLayer(p);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool HUDLayer::init() {
	if (!Layer::init())
		return false;

	std::string image;

	image = "Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, Z_HUD_ELEMENT, "healthbar");
	healthbar->setPosition(-680.f * RES_ADJUST + HP_BAR_ADJUST, 480.f * RES_ADJUST);
	healthbar->setScale(1.5f);
	HUD.insert(std::pair<std::string, Sprite*>("healthbar", healthbar));

	image = "Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, Z_HUD_ELEMENT, "hp");
	hp->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	hp->setPosition(-844.f * RES_ADJUST + HP_ADJUST, 480.f * RES_ADJUST);
	hp->setScale(1.5f);
	HUD.insert(std::pair<std::string, Sprite*>("hp", hp));

	createPersistentLabel(-680.f * RES_ADJUST, 482.f * RES_ADJUST, std::to_string(m_player->getHP()) + "/" + std::to_string(m_player->getMaxHP()), "HP", 22);

	createPersistentLabel(-870 * RES_ADJUST, 70 * RES_ADJUST, "Str: +" + std::to_string(m_player->getStr()), "Strength", 18);
	createPersistentLabel(-870 * RES_ADJUST, 50 * RES_ADJUST, "Dex: +" + std::to_string(m_player->getDex() + m_player->getWeapon()->getDexBonus()), "Dexterity", 18);
	createPersistentLabel(-870 * RES_ADJUST, 30 * RES_ADJUST, "Int: +" + std::to_string(m_player->getInt()), "Intellect", 18);
	createPersistentLabel(-870 * RES_ADJUST, 10 * RES_ADJUST, "Amr: +" + std::to_string(m_player->getArmor()), "Armor", 18);

	constructWeaponHUD();


	//	:::: RIGHT SIDE OF HUD ::::

	// Currency
	image = "Gold_Pile1_48x48.png";
	Sprite* goldpile = Sprite::createWithSpriteFrameName(image);
	this->addChild(goldpile, Z_HUD_SPRITE, "goldpile");
	goldpile->setPosition(855 * RES_ADJUST, 480 * RES_ADJUST);
	goldpile->setScale(1.0f * RES_ADJUST);
	goldpile->setOpacity(230);
	HUD.insert(std::pair<std::string, Sprite*>("goldpile", goldpile));

	createPersistentLabel(895 * RES_ADJUST, 480 * RES_ADJUST, "0", "Money Count", 22);

	createPersistentLabel(870.f * RES_ADJUST, -480 * RES_ADJUST, "Money Bonus : " + std::to_string(static_cast<int>(m_player->getMoneyBonus())), "Money Bonus", 20);
	m_persistentLabels.find("Money Bonus")->second->setTextColor(cocos2d::Color4B(153, 153, 255, 200));

	return true;
}

void HUDLayer::updateHUD() {
	checkPlayerStats();
	checkWeaponHUD();
	checkActiveItemHUD();
	checkRelicHUD();
	checkItemHUD();
}

void HUDLayer::showBossHP() {
	std::string image;

	//		RENDER BOSS HP
	image = "Boss_Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, Z_HUD_ELEMENT, "bosshealthbar");
	healthbar->setPosition(430, 0);
	HUD.insert(std::pair<std::string, Sprite*>("bosshealthbar", healthbar));

	image = "Boss_Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, Z_HUD_ELEMENT, "bosshp");
	hp->setAnchorPoint(Vec2(0.5, 0.0)); // set anchor point to left side
	hp->setPosition(430, -134);
	HUD.insert(std::pair<std::string, Sprite*>("bosshp", hp));
}
void HUDLayer::updateBossHUD() {
	// if there are still monsters, check for smasher
	if (m_scene->getCurrentDungeon()->monsterCount() > 0) {
		if (m_scene->getCurrentDungeon()->monsterAt(0)->getName() == "Smasher") {
			//	Check Boss HP bar
			float y_scale = m_scene->getCurrentDungeon()->monsterAt(0)->getHP() / (static_cast<float>(m_scene->getCurrentDungeon()->monsterAt(0)->getMaxHP()) * 1.0);
			cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, 1.0f, y_scale);
			HUD.find("bosshp")->second->runAction(move);
		}
		else
			deconstructBossHUD();		
	}
	// else if there aren't any monsters, deconstruct the hp bar if we haven't already
	else {
		deconstructBossHUD();
	}
}

void HUDLayer::showHUD() {
	for (auto &it : HUD)
		it.second->setVisible(true);

	for (auto &it : keyLabels)
		it.second->setVisible(true);

	for (auto &it : m_persistentLabels)
		it.second->setVisible(true);
}
void HUDLayer::hideHUD() {
	for (auto &it : HUD)
		it.second->setVisible(false);

	for (auto &it : keyLabels)
		it.second->setVisible(false);

	for (auto &it : m_persistentLabels)
		it.second->setVisible(false);
}

void HUDLayer::NPCInteraction(const NPC &npc) {
	// assign dialogue to use
	m_dialogue = npc.getDialogue();

	if (npc.getName() == MEMORIZER)
		hideHUD();

	m_line = Label::createWithTTF("", TEXT_FONT, 20);
	m_line->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	m_line->setOpacity(230);
	m_line->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_line->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_line->setAdditionalKerning(0.25f);
	this->addChild(m_line, Z_HUD_LABEL);

	// display first line
	m_line->setString(m_dialogue[m_lineIndex]);
	m_lineIndex++;

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::NPCKeyPressed, this, &npc), m_line);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::NPCButtonPressed, this, &npc), m_line);
}
void HUDLayer::NPCKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc) {
	// If there's nothing left to say, remove text and restore control
	if (m_lineIndex >= static_cast<int>(m_dialogue.size())) {
		playInterfaceSound("Confirm 1.mp3");

		if (npc->getName() == MEMORIZER)
			showHUD();

		m_lineIndex = 0;
		m_line->removeFromParent();
		m_line = nullptr;
		enableGameplayListeners();
		return;
	}

	switch (keyCode) {
	case KeyType::KEY_SPACE:
	default: {
		if (!m_dialogue.empty() && m_lineIndex < static_cast<int>(m_dialogue.size())) {
			playInterfaceSound("Confirm 1.mp3");

			// If this line indicates a prompt, then begin the prompt
			if (m_dialogue[m_lineIndex] == NPC_PROMPT) {
				m_lineIndex = 0;
				m_line->removeFromParent();
				m_line = nullptr;

				NPCPrompt(*npc);

				return;
			}

			m_line->setString(m_dialogue[m_lineIndex]);
			m_lineIndex++;
		}
	}
	}
}
void HUDLayer::NPCButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc) {
	NPCKeyPressed(KeyType::KEY_ENTER, event, npc);
}

void HUDLayer::NPCPrompt(const NPC &npc) {
	// Assign this so that we know how many choices there are
	m_dialogue = npc.getChoices();

	// menu border
	/*Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);
	menuSprites.insert(std::pair<std::string, Sprite*>("box", box));*/

	// Arrow sprite for selection
	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(-3 * MENU_SPACING, -2.5f * MENU_SPACING);
	this->addChild(selector, Z_HUD_SPRITE);
	selector->setScale(2.5f);
	generalSprites.insert(std::pair<std::string, Sprite*>("Selector", selector));

	for (int i = 0; i < static_cast<int>(m_dialogue.size()); ++i)
		addLabel(0, (-3 - (static_cast<float>(i) - 0.5)) * MENU_SPACING, m_dialogue[i], m_dialogue[i], 20);
	
	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::NPCPromptKeyPressed, this, &npc), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::NPCPromptButtonPressed, this, &npc), selector);
}
void HUDLayer::NPCPromptKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < static_cast<int>(m_dialogue.size() - 1)) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		const_cast<NPC*>(npc)->useResponse(index);

		index = 0;
		deconstructMenu(generalSprites);

		if (npc->getName() == MEMORIZER)
			hideHUD();
		
		NPCInteraction(*npc);

		return;

	default:
		break;
	}

}
void HUDLayer::NPCPromptButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc) {
	NPCPromptKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event, npc);
}

void HUDLayer::devilsWaters() {
	constructSelectionMenu();

	// Pause option
	addLabel(0, 4.8f * MENU_SPACING, fetchPromptText("Fountain", "prompt"), "pause", 40);

	// NO
	addLabel(0, 2 * MENU_SPACING, fetchPromptText("Fountain", "No"), "no", 36);

	// YES
	addLabel(0, -2 * MENU_SPACING, fetchPromptText("Fountain", "Yes"), "yes", 36);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::devilKeyPressed, this), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::devilButtonPressed, this), menuSprites.find("Selector")->second);
}
void HUDLayer::devilKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// NO
		case 0: {
			playInterfaceSound("Confirm 1.mp3");

			index = 0;
			deconstructMenu(menuSprites);

			SecondFloor* second = dynamic_cast<SecondFloor*>(m_scene->getCurrentDungeon());
			second->devilsWater(false);

			enableGameplayListeners();
			return;
		}
			// YES
		case 1: 
			playSound("Devils_Gift.mp3");

			SecondFloor* second = dynamic_cast<SecondFloor*>(m_scene->getCurrentDungeon());
			second->devilsWater(true);

			index = 0;
			deconstructMenu(menuSprites);

			enableGameplayListeners();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::devilButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	devilKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::shrineChoice(const std::string &shrine) {
	constructSelectionMenu();

	// Number of options to choose from
	int choices = 0;

	if (shrine == HEALTH_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "health prompt"), "Life", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Wealth"), "Sacrifice Wealth", 29);
		addLabel(0, 1 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Item"), "Sacrifice Item", 29);
		addLabel(0, 0 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Weapon"), "Sacrifice Weapon", 29);
		addLabel(0, -1 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Soul"), "Sacrifice Soul", 29);

		choices = 4;
	}
	else if (shrine == RELIC_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "relic prompt"), "Relic", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Obtain Relic"), "Obtain Relic", 34);
		choices = 1;

		if (m_player->hasRelic()) {
			updateLabel("Obtain Relic", fetchPromptText("Shrine", "Upgrade Relic"));
			addLabel(0, 1 * MENU_SPACING, fetchPromptText("Shrine", "Change Relic"), "Change Relic", 34);

			choices = 2;
		}
	}
	else if (shrine == CHEST_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "chest prompt"), "Chest", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Accept"), "Accept", 34);
		choices = 1;
	}

	choices--; // Subtract one for easier code in the selection part
	addLabel(0, -2 * MENU_SPACING, fetchPromptText("Shrine", "Deny"), "Walk away", 34);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::shrineKeyPressed, this, choices), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::shrineButtonPressed, this, choices), menuSprites.find("Selector")->second);
}
void HUDLayer::shrineKeyPressed(KeyType keyCode, cocos2d::Event* event, int choices) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	
	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0 && index <= choices) {
			playInterfaceSound("Select 1.mp3");

			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);
		}
		else if (index > choices) {
			playInterfaceSound("Select 1.mp3");

			index--;
			event->getCurrentTarget()->setPosition(pos.x, (2 - choices) * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < choices) {
			playInterfaceSound("Select 1.mp3");

			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);
		}
		else if (index == choices) {
			playInterfaceSound("Select 1.mp3");

			index++;
			event->getCurrentTarget()->setPosition(pos.x, -2 * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE: {
		Shrine* shrine = dynamic_cast<Shrine*>(m_scene->getCurrentDungeon());
		shrine->useChoice(index);

		index = 0;
		deconstructMenu(menuSprites);

		enableGameplayListeners();
		return;
	}
	default:
		break;
	}
}
void HUDLayer::shrineButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, int choices) {
	shrineKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event, choices);
}

void HUDLayer::inventoryMenu() {
	// INVENTORY
	auto inventory = Label::createWithTTF(fetchMenuText("Inventory Menu", "Inventory"), TEXT_FONT, 54);
	inventory->setPosition(0 * RES_ADJUST, 390 * RES_ADJUST);
	inventory->setOpacity(230);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	inventory->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	inventory->setAdditionalKerning(0.25f);
	this->addChild(inventory, Z_HUD_LABEL);
	labels.insert(std::make_pair("inventory", inventory));

	// menu border
	Sprite* longBox = Sprite::create("Inventory_Box_Length_Medium.png");
	this->addChild(longBox, Z_HUD_ELEMENT);
	longBox->setPosition(-300 * RES_ADJUST, 0);
	longBox->setScale(.6f * RES_ADJUST);
	longBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("long box", longBox));

	// menu border
	Sprite* wideBox = Sprite::create("Inventory_Box_Wide_Medium.png");
	this->addChild(wideBox, Z_HUD_ELEMENT);
	wideBox->setPosition(150 * RES_ADJUST, 75 * RES_ADJUST);
	wideBox->setScale(.6f * RES_ADJUST);
	wideBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("wide box", wideBox));

	// menu border
	Sprite* textBox = Sprite::create("Inventory_Box_Text_Medium.png");
	this->addChild(textBox, Z_HUD_ELEMENT);
	textBox->setPosition(130 * RES_ADJUST, -200 * RES_ADJUST);
	textBox->setScale(.6f * RES_ADJUST);
	textBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("text box", textBox));

	// menu border
	Sprite* selectBox = Sprite::create("Selection_Box.png");
	this->addChild(selectBox, Z_HUD_ELEMENT + 1);
	selectBox->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);
	selectBox->setScale(.1f);
	selectBox->setOpacity(200);
	selectBox->setColor(cocos2d::Color3B(255, 200, 100));
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("select box", selectBox));


	// Inventory key:
	// 
	//    0 - 1:  Weapons
	//    2 - 6:  Usable items
	//    7:      Spacebar Item
	//    8:      Relic
	//    9 - 23: Passive Items
	//
	//    0   1  |  2   3   4   5   6
	//    7   8  |  9   10  11  12  13
	//    X   X  |  14  15  16  17  18
	//    X   X  |  19  20  21  22  23

	std::string image, name, desc;

	// Display use items
	for (int i = 0; i < static_cast<int>(m_player->itemCount()); i++) {
		fetchItemInfo("Usable", m_player->itemAt(i)->getName(), name, desc);

		inventoryText[i + 2] = std::make_pair(name, desc);
		image = m_player->itemAt(i)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, Z_HUD_SPRITE);
		item->setPosition((1.9 * i * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);
		item->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));
	}

	// Display weapons
	image = m_player->getWeapon()->getImageName();
	fetchItemInfo("Weapon", m_player->getWeapon()->getName(), name, desc);
	inventoryText[0] = std::make_pair(name, desc);

	Sprite* weapon = Sprite::createWithSpriteFrameName(image);
	this->addChild(weapon, Z_HUD_SPRITE);
	weapon->setScale(1.0);
	weapon->setPosition((-350) * RES_ADJUST, 150 * RES_ADJUST);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));

	if (m_player->getStoredWeapon() != nullptr) {
		image = m_player->getStoredWeapon()->getImageName();
		fetchItemInfo("Weapon", m_player->getStoredWeapon()->getName(), name, desc);
		inventoryText[1] = std::make_pair(name, desc);

		Sprite* storedWeapon = Sprite::createWithSpriteFrameName(image);
		this->addChild(storedWeapon, Z_HUD_SPRITE);
		storedWeapon->setScale(1.0);
		storedWeapon->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Stored Weapon", storedWeapon));
	}
	/// End weapon display

	// Label for the item's name
	m_itemName = Label::createWithTTF("", TEXT_FONT, 20);
	m_itemName->setPosition(130 * RES_ADJUST, -150 * RES_ADJUST);
	m_itemName->setOpacity(230);
	m_itemName->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_itemName->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_itemName->setAdditionalKerning(0.25f);
	this->addChild(m_itemName, Z_HUD_LABEL);
	labels.insert(std::make_pair("item name", m_itemName));

	// Label for the item's description
	m_itemDescription = Label::createWithTTF("", TEXT_FONT, 20);
	m_itemDescription->setPosition(130 * RES_ADJUST, -170 * RES_ADJUST);
	m_itemDescription->setAnchorPoint(Vec2(0.5, 1.0));
	m_itemDescription->setOpacity(230);
	m_itemDescription->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_itemDescription->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_itemDescription->setAdditionalKerning(0.25f);
	this->addChild(m_itemDescription, Z_HUD_LABEL);
	labels.insert(std::make_pair("item description", m_itemDescription));

	// Displays first item info
	m_itemName->setString(inventoryText[0].first);
	std::string currentDesc = inventoryText[0].second;
	formatItemDescriptionForDisplay(currentDesc);
	m_itemDescription->setString(currentDesc);

	if (m_player->hasActiveItem()) {
		std::string shield = m_player->getActiveItem()->getName();
		fetchItemInfo("Usable", shield, name, desc);
		inventoryText[7] = std::make_pair(name, desc);

		image = m_player->getActiveItem()->getImageName();

		// shield sprite
		Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentshield, Z_HUD_SPRITE, shield);
		currentshield->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);
		currentshield->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
	}

	if (m_player->hasRelic()) {
		std::string relic = m_player->getRelic()->getName();
		fetchItemInfo("Relic", relic, name, desc);

		if (m_player->getRelic()->getLevel() > 1)
			name += " +" + std::to_string(m_player->getRelic()->getLevel() - 1);

		inventoryText[8] = std::make_pair(name, desc);

		image = m_player->getRelic()->getImageName();

		// trinket sprite
		Sprite* currentRelic = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentRelic, Z_HUD_SPRITE, relic);
		currentRelic->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);
		currentRelic->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Current Relic", currentRelic));

	}

	// Display passives
	for (int i = 0; i < static_cast<int>(m_player->passiveCount()); i++) {
		fetchItemInfo("Passive", m_player->passiveAt(i)->getName(), name, desc);
		inventoryText[i + 9] = std::make_pair(name, desc);
		image = m_player->passiveAt(i)->getImageName();

		Sprite* passive = Sprite::createWithSpriteFrameName(image);
		this->addChild(passive, Z_HUD_SPRITE);

		// X: i % 5 since 5 entries per row; Y: 80 is the number of pixels to next row, i / 5 to get the correct row
		passive->setPosition((1.9 * (i % 5) * MENU_SPACING - 90) * RES_ADJUST, (100 - (80 * (i / 5))) * RES_ADJUST);

		passive->setScale(1.0f);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Passive", passive));
	}

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::inventoryMenuKeyPressed, this), selectBox);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::inventoryMenuButtonPressed, this), selectBox);
}
void HUDLayer::inventoryMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	// Inventory key:
	// 
	//    0 - 1:  Weapons
	//    2 - 6:  Usable items
	//    7:      Relic
	//    8:      Spacebar Item
	//    9 - 23: Passive Items
	//
	//    0   1  |  2   3   4   5   6
	//    7   8  |  9   10  11  12  13
	//    X   X  |  14  15  16  17  18
	//    X   X  |  19  20  21  22  23

	if (keyCode == INVENTORY_KEY) {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 23; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		updateHUD();
		enableGameplayListeners();

		return;
	}

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 0) {
			index = 6; // end of weapons
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index--;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 3 && index <= 6) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 7) {
			index = 13; // end of items
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 9) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 9 && index <= 13) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 14 && index <= 18) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 14) {
			index = 18;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 2 && index < 6) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 6) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 7) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 9;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index < 13) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 13) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 14 && index < 18) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 18) {
			index = 14;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index == 7) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 1;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index <= 13) {
			index -= 7;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 14 && index <= 18) {
			index -= 5;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 0) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 2 && index <= 6) {
			index += 7;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index <= 13) {
			index += 5;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_I:
	case KeyType::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 23; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		updateHUD();
		enableGameplayListeners();

		return;
	}
	default: break;
	}
}
void HUDLayer::inventoryMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = gameplayButtonToKey(static_cast<ButtonType>(keyCode));
	if (key == INVENTORY_KEY)
		inventoryMenuKeyPressed(key, event);
	else
		inventoryMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::itemMenu() {
	// |----|----|----|----|----|
	// |item|item|item|item|empt|
	// |    |    |    |    |    |
	// |----|----|----|----|----|

	// [RENDER MENU DESIGN]

	for (int i = -2; i < 3; i++) {
		Sprite* box = Sprite::create("Menu_Box1.png");
		this->addChild(box, Z_HUD_ELEMENT);
		box->setPosition(i * 65, 3);
		box->setOpacity(200);
		box->setScale(1.5f);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));
	}

	std::string image = "cheese.png";
	int size = m_player->itemCount();

	for (int i = -2; i < size - 2; i++) {
		image = m_player->itemAt(i + 2)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, Z_HUD_SPRITE);
		item->setPosition(i * 65, 0.0f);
		item->setScale(1.0f);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));

		// If item is stackable, display stack amount for item
		if (m_player->itemAt(i + 2)->canStack() || m_player->hasFatStacks()) {
			auto stack = Label::createWithTTF("x" + std::to_string(m_player->itemAt(i + 2)->getCount()), TEXT_FONT, 18);
			stack->setPosition(i * 65 + 15.f, -15.f);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
			stack->setColor(Color3B(220, 220, 220));
			stack->setOpacity(255);
			this->addChild(stack, Z_HUD_LABEL);
			labels.insert(std::pair<std::string, Label*>(std::to_string(i + 2), stack));
		}
	}

	// arrow sprite for selection
	auto selector = Sprite::create("Down_Arrow.png");
	selector->setPosition(-2 * 65, 65);
	selector->setScale(3.0f);
	this->addChild(selector, Z_HUD_SPRITE);
	itemMenuSprites.insert(std::pair<std::string, Sprite*>("Selector", selector));

	// helper labels
	auto equip = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY) + ": " + fetchMenuText("Item Menu", "Quick Use Info"), TEXT_FONT, 20);
	equip->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	equip->setOpacity(200);
	equip->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	equip->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	equip->setAdditionalKerning(0.25f);
	this->addChild(equip, Z_HUD_LABEL);
	labels.insert(std::make_pair("equip", equip));

	auto use = Label::createWithTTF("Space/Enter: " + fetchMenuText("Item Menu", "Use Item"), TEXT_FONT, 20);
	use->setPosition(0 * RES_ADJUST, -300 * RES_ADJUST);
	use->setOpacity(200);
	use->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	use->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	use->setAdditionalKerning(0.25f);
	this->addChild(use, Z_HUD_LABEL);
	labels.insert(std::make_pair("use", use));

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::itemMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::itemMenuButtonPressed, this), selector);
}
void HUDLayer::itemMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	if (keyCode == ITEM_KEY) {
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();

		return;
	}
	else if (keyCode == QUICK_KEY) {
		playInterfaceSound("Confirm 1.mp3");

		// if player has items, set this to new quick item slot
		if (m_player->hasItems())
			m_scene->getCurrentDungeon()->assignQuickItem(index);
		
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();
		return;
	}

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x - 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0 && m_player->itemCount() > 1) {
			index = m_player->itemCount() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index < static_cast<int>(m_player->itemCount()) - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == m_player->itemCount() - 1 && m_player->itemCount() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ENTER: {
		playInterfaceSound("Confirm 1.mp3");

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActionsByTag(1) > 0)
			actions->update(1.0, 1);
		
		if (m_player->hasItems())
			m_scene->getCurrentDungeon()->callUse(index);

		m_scene->getCurrentDungeon()->peekDungeon('-');
	}
	case KeyType::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();

		return;
	}
	default: break;
	}
}
void HUDLayer::itemMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = gameplayButtonToKey(static_cast<ButtonType>(keyCode));
	if (key == ITEM_KEY || key == QUICK_KEY)
		itemMenuKeyPressed(key, event);
	else
		itemMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::gameOver() {
	// prevent player movement
	Director::getInstance()->getScheduler()->unscheduleAllForTarget(m_scene);
	this->_eventDispatcher->removeAllEventListeners();

	//cocos2d::experimental::AudioEngine::stopAll();
	playMusic("Fallen in Battle.mp3", false);

	deconstructMenu(menuSprites);

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();


	//       PAUSE
	//    |---------|
	// -> |  Resume |
	//    | Restart |
	//    |         |
	//    |Exit Game|
	//    |---------|


	constructSelectionMenu();

	// Game over!
	auto pause = Label::createWithTTF(fetchPromptText("Game Over", "prompt"), TEXT_FONT, 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, Z_HUD_LABEL);

	// Back to Menu option
	auto back = Label::createWithTTF(fetchPromptText("Game Over", "Main Menu"), TEXT_FONT, 36);
	back->setPosition(0, 2 * MENU_SPACING);
	this->addChild(back, Z_HUD_LABEL);

	// Restart option
	auto resume = Label::createWithTTF(fetchPromptText("Game Over", "Restart"), TEXT_FONT, 36);
	resume->setPosition(0, 1 * MENU_SPACING);
	this->addChild(resume, Z_HUD_LABEL);

	// Quit option
	auto exit = Label::createWithTTF(fetchPromptText("Game Over", "Exit Game"), TEXT_FONT, 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, Z_HUD_LABEL);


	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::gameOverKeyPressed, this), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::gameOverButtonPressed, this), menuSprites.find("Selector")->second);
}
void HUDLayer::gameOverKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index == 1) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	//case KeyType::KEY_SPACE:
		switch (index) {
			// Back to main menu
		case 0: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			playInterfaceSound("Confirm 1.mp3");

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// advance to start menu scene
			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
			// Restart
		case 1: {
			restartGame(*m_player);
			playInterfaceSound("Confirm 1.mp3");
			return;
		}
		case 2: // Exit Game
			Director::getInstance()->end();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::gameOverButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	gameOverKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::winner() {
	// prevent player movement
	this->_eventDispatcher->removeAllEventListeners();


	cocos2d::experimental::AudioEngine::stopAll();
	cocos2d::experimental::AudioEngine::play2d("Victory! All Clear.mp3", false);


	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();


	//       PAUSE
	//    |---------|
	// -> |  Resume |
	//    | Restart |
	//    |         |
	//    |Exit Game|
	//    |---------|


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, Z_HUD_ELEMENT);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);

	// You won!
	auto pause = Label::createWithTTF(fetchPromptText("Winner", "prompt"), TEXT_FONT, 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, Z_HUD_LABEL);

	// Resume option
	auto resume = Label::createWithTTF(fetchPromptText("Winner", "Restart"), TEXT_FONT, 36);
	resume->setPosition(0, 2 * MENU_SPACING);
	this->addChild(resume, Z_HUD_LABEL);

	// Quit option
	auto exit = Label::createWithTTF(fetchPromptText("Winner", "Exit Game"), TEXT_FONT, 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, Z_HUD_LABEL);

	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(selector, Z_HUD_SPRITE);
	selector->setScale(2.5f);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::winnerKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::winnerButtonPressed, this), selector);
}
void HUDLayer::winnerKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// Restart
		case 0: {
			restartGame(*m_player);
			playInterfaceSound("Confirm 1.mp3");
			return;
		}
		case 1: // Exit Game
			Director::getInstance()->end();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::winnerButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	winnerKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::constructSelectionMenu() {
	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, Z_HUD_ELEMENT);
	box->setPosition(0, 0);
	box->setScale(0.32f);
	box->setOpacity(170);
	menuSprites.insert(std::pair<std::string, Sprite*>("box", box));

	// arrow sprite for selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(arrow, Z_HUD_SPRITE);
	arrow->setScale(3.0f);
	menuSprites.insert(std::pair<std::string, Sprite*>("Selector", arrow));
}

void HUDLayer::deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites) {
	// remove menu images
	for (auto &it : sprites)
		it.second->removeFromParent();
	
	sprites.clear();

	// remove any labels
	for (auto &it : labels)
		it.second->removeFromParent();
	
	labels.clear();
}

void HUDLayer::checkPlayerStats() {
	float x_scale = m_player->getHP() / (static_cast<float>(m_player->getMaxHP()) * 1.0);
	cocos2d::Action* move = cocos2d::ScaleTo::create(.4f, x_scale * 1.5f, 1.5f);
	auto action = HUD.find("hp")->second->runAction(move);
	action->setTag(5);

	m_persistentLabels.find("Money Count")->second->setString(std::to_string(m_player->getMoney()));
	m_persistentLabels.find("Money Bonus")->second->setString("Money Bonus : " + std::to_string(static_cast<int>(m_player->getMoneyBonus())));

	m_persistentLabels.find("HP")->second->setString(std::to_string(m_player->getHP()) + "/" + std::to_string(m_player->getMaxHP()));

	m_persistentLabels.find("Strength")->second->setString("Str: +" + std::to_string(m_player->getStr()));
	m_persistentLabels.find("Dexterity")->second->setString("Dex: +" + std::to_string(m_player->getDex() + m_player->getWeapon()->getDexBonus()));
	m_persistentLabels.find("Intellect")->second->setString("Int: +" + std::to_string(m_player->getInt()));
	m_persistentLabels.find("Armor")->second->setString("Amr: +" + std::to_string(m_player->getArmor()));
}

void HUDLayer::constructActiveItemHUD() {
	std::string name = m_player->getActiveItem()->getName();
	std::string image = m_player->getActiveItem()->getImageName();

	// shield HUD box
	Sprite* activeBox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	activeBox->setPosition(-870 * RES_ADJUST, 290 * RES_ADJUST);
	activeBox->setScale(0.4f * RES_ADJUST);
	activeBox->setOpacity(160);
	activeBox->setColor(cocos2d::Color3B(255, 175, 5));
	this->addChild(activeBox, Z_HUD_ELEMENT, "Active Box");
	HUD.insert(std::pair<std::string, Sprite*>("Active Box", activeBox));

	// Active Item sprite
	Sprite* active = Sprite::createWithSpriteFrameName(image);
	this->addChild(active, Z_HUD_SPRITE, name);
	active->setPosition(-870.f * RES_ADJUST, 290.f * RES_ADJUST);
	active->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", active));

	if (m_player->activeHasMeter()) {
		// Active Item bar
		Sprite* activeBar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
		this->addChild(activeBar, Z_HUD_ELEMENT, "Active Bar");
		activeBar->setPosition(-850 * RES_ADJUST + SP_ADJUST, 250 * RES_ADJUST);
		activeBar->setScale(1.5f * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Bar", activeBar));

		// Active Item bar points
		Sprite* activePoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
		this->addChild(activePoints, Z_HUD_ELEMENT + 1, "Active Points");
		activePoints->setScale(1.5f);
		activePoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
		activePoints->setPosition(-888 * RES_ADJUST, 250.5 * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Points", activePoints));
	}

	
	auto space = Label::createWithTTF(convertKeycodeToStr(ACTIVE_KEY), TEXT_FONT, 18);
	space->setPosition(-870.f * RES_ADJUST, 265.f * RES_ADJUST);
	space->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	space->setOpacity(245);
	this->addChild(space, Z_HUD_LABEL);
	keyLabels.insert(std::pair<std::string, Label*>("Active Key", space));

	// Display stack amount for Spelunker
	if (m_player->getName() == SPELUNKER) {
		std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(m_player);
		int count = sp->getRockCount();
		sp.reset();

		// If there was no label for stackable items, add it
		if (keyLabels.find("Active Stack Amount") == keyLabels.end()) {
			auto stack = Label::createWithTTF("x" + std::to_string(count), TEXT_FONT, 16);
			stack->setPosition(-840.f * RES_ADJUST, 290.f * RES_ADJUST);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
			stack->setOpacity(250);
			this->addChild(stack, Z_HUD_LABEL);
			keyLabels.insert(std::pair<std::string, Label*>("Active Stack Amount", stack));
		}
		else {
			keyLabels.find("Active Stack Amount")->second->setString("x" + std::to_string(count));
		}
	}
}
void HUDLayer::checkActiveItemHUD() {
	if (m_player->hasActiveItem()) {
		std::string name = m_player->getActiveItem()->getName();

		if (keyLabels.find("Active Key") != keyLabels.end())
			keyLabels.find("Active Key")->second->setString(convertKeycodeToStr(ACTIVE_KEY));

		// If there wasn't an active item equipped previously, construct the menu
		if (HUD.find("Active") == HUD.end())
			constructActiveItemHUD();

		// else if current active item is different, switch the sprite
		else if (HUD.find("Active")->second->getName() != name)
			updateActiveItemHUD();

		// Display stack amount for Spelunker
		else if (m_player->getName() == SPELUNKER) {
			std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(m_player);
			int count = sp->getRockCount();
			sp.reset();

			keyLabels.find("Active Stack Amount")->second->setString("x" + std::to_string(count));
		}
	}
	else {
		// If there's no active item equipped and there previously was, deconstruct the HUD
		if (HUD.find("Active") != HUD.end())
			deconstructActiveItemHUD();
	}

	updateActiveItemBar();
}
void HUDLayer::updateActiveItemHUD() {
	// Remove old sprite
	HUD.find("Active")->second->removeFromParent();
	HUD.erase("Active");

	std::string name = m_player->getActiveItem()->getName();
	std::string image = m_player->getActiveItem()->getImageName();

	Sprite* newActive = Sprite::createWithSpriteFrameName(image);
	this->addChild(newActive, Z_HUD_SPRITE, name);
	newActive->setPosition(-870 * RES_ADJUST, 290 * RES_ADJUST);
	newActive->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", newActive));
}
void HUDLayer::updateActiveItemBar() {
	// if active has a meter, update it
	if (m_player->hasActiveItem() && m_player->activeHasMeter()) {
		float sx_scale = m_player->getCurrentActiveMeter() / (static_cast<float>(m_player->getMaxActiveMeter()) * 1.0);
		cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, sx_scale * 1.5f, 1.5f);
		HUD.find("Active Points")->second->runAction(move);
	}
}
void HUDLayer::deconstructActiveItemHUD() {
	if (HUD.find("Active Box") != HUD.end()) {
		// deconstruct the shield HUD because there's no shield equipped
		HUD.find("Active")->second->removeFromParent();
		HUD.find("Active Box")->second->removeFromParent();
		keyLabels.find("Active Key")->second->removeFromParent();

		HUD.erase(HUD.find("Active"));
		HUD.erase(HUD.find("Active Box"));
		keyLabels.erase(keyLabels.find("Active Key"));

		if (m_player->activeHasMeter()) {
			HUD.find("Active Bar")->second->removeFromParent();
			HUD.find("Active Points")->second->removeFromParent();
			HUD.erase(HUD.find("Active Bar"));
			HUD.erase(HUD.find("Active Points"));
		}

		if (m_player->getName() == SPELUNKER) {
			keyLabels.find("Active Stack Amount")->second->removeFromParent();
			keyLabels.erase(keyLabels.find("Active Stack Amount"));
		}
	}
}

void HUDLayer::constructItemHUD() {
	// Quick access item use slot
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* quickAccess = Sprite::createWithSpriteFrameName(image);
	quickAccess->setPosition(870 * RES_ADJUST, 390 * RES_ADJUST);
	quickAccess->setScale(0.4f * RES_ADJUST);
	quickAccess->setOpacity(160);
	quickAccess->setColor(cocos2d::Color3B(250, 188, 165));
	this->addChild(quickAccess, Z_HUD_ELEMENT, "quick access");
	HUD.insert(std::pair<std::string, Sprite*>("quick access", quickAccess));

	// key label
	auto qKey = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY), TEXT_FONT, 18);
	qKey->setPosition(870.f * RES_ADJUST, 362.f * RES_ADJUST);
	qKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	qKey->setOpacity(245);
	this->addChild(qKey, Z_HUD_LABEL);
	keyLabels.insert(std::pair<std::string, Label*>("qKey", qKey));

	updateItemHUD();
}
void HUDLayer::checkItemHUD() {
	if (m_player->hasItems()) {
		std::string image = m_player->itemAt(0)->getImageName();
		std::string item = m_player->itemAt(0)->getName();

		if (keyLabels.find("qKey") != keyLabels.end())
			keyLabels.find("qKey")->second->setString(convertKeycodeToStr(QUICK_KEY));

		// if there weren't any items before, construct menu again
		if (HUD.find("quick access") == HUD.end())
			constructItemHUD();
		
		// else if the item is different, switch the sprite
		else if (HUD.find("quick item") != HUD.end() && HUD.find("quick item")->second->getName() != item) {
			HUD.find("quick item")->second->removeFromParent();
			HUD.erase(HUD.find("quick item"));

			updateItemHUD();
		}

		// Display stack amount for item
		if (m_player->itemAt(0)->canStack() || m_player->hasFatStacks()) {

			// If there was no label for stackable items, add it
			if (keyLabels.find("Stack Amount") == keyLabels.end()) {
				auto stack = Label::createWithTTF("x" + std::to_string(m_player->itemAt(0)->getCount()), TEXT_FONT, 18);
				stack->setPosition(900.f * RES_ADJUST, 390.f * RES_ADJUST);
				stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
				stack->setOpacity(250);
				this->addChild(stack, Z_HUD_LABEL);
				keyLabels.insert(std::pair<std::string, Label*>("Stack Amount", stack));
			}
		}
		// Otherwise if a stack amount label exists
		if (keyLabels.find("Stack Amount") != keyLabels.end()) {

			// Remove it because the new item can't stack, or it's a different item than before
			if (!(m_player->itemAt(0)->canStack() || m_player->hasFatStacks()) || HUD.find("quick item")->second->getName() != item) {
				keyLabels.find("Stack Amount")->second->removeFromParent();
				keyLabels.erase(keyLabels.find("Stack Amount"));
			}
			// Update the count
			else if (HUD.find("quick item")->second->getName() == item) {
				keyLabels.find("Stack Amount")->second->setString("x" + std::to_string(m_player->itemAt(0)->getCount()));
			}
		}
	}
	else {
		// if there's no item quick slotted but there was previously, deconstruct the HUD
		if (HUD.find("quick access") != HUD.end())
			deconstructItemHUD();
	}
}
void HUDLayer::updateItemHUD() {
	// The item to display
	Sprite* quickitem = Sprite::createWithSpriteFrameName(m_player->itemAt(0)->getImageName());
	this->addChild(quickitem, Z_HUD_SPRITE, m_player->itemAt(0)->getName());
	quickitem->setPosition(870.f * RES_ADJUST, 390.f * RES_ADJUST);
	quickitem->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("quick item", quickitem));
}
void HUDLayer::deconstructItemHUD() {
	if (HUD.find("quick item") != HUD.end()) {
		// deconstruct the quick item HUD because player has no items
		HUD.find("quick access")->second->removeFromParent();
		HUD.find("quick item")->second->removeFromParent();
		keyLabels.find("qKey")->second->removeFromParent();

		HUD.erase(HUD.find("quick access"));
		HUD.erase(HUD.find("quick item"));
		keyLabels.erase(keyLabels.find("qKey"));
	}

	if (keyLabels.find("Stack Amount") != keyLabels.end()) {
		keyLabels.find("Stack Amount")->second->removeFromParent();
		keyLabels.erase(keyLabels.find("Stack Amount"));
	}
}

void HUDLayer::constructRelicHUD() {
	// Relic HUD box
	Sprite* trinketbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	trinketbox->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	trinketbox->setScale(.4f * RES_ADJUST);
	trinketbox->setOpacity(200);
	trinketbox->setColor(cocos2d::Color3B(200, 20, 0));
	this->addChild(trinketbox, Z_HUD_ELEMENT, "Relic Box");
	HUD.insert(std::pair<std::string, Sprite*>("Relic Box", trinketbox));

	// Relic sprite
	Sprite* currentTrinket = Sprite::createWithSpriteFrameName(m_player->getRelic()->getImageName());
	this->addChild(currentTrinket, Z_HUD_SPRITE, m_player->getRelic()->getName());
	currentTrinket->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	currentTrinket->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("Current Relic", currentTrinket));
}
void HUDLayer::checkRelicHUD() {
	if (m_player->hasRelic()) {
		std::string relic = m_player->getRelic()->getName();

		// If there is now a relic equipped, but there wasn't previously, construct the menu
		if (HUD.find("Current Relic") == HUD.end())
			constructRelicHUD();

		// else if current relic equipped is different, switch the sprite
		else if (HUD.find("Current Relic")->second->getName() != relic)
			updateRelicHUD();
	}
	else {
		// if there's no relic equipped and there was previously, deconstruct the HUD
		if (HUD.find("Current Relic") != HUD.end())
			deconstructRelicHUD();
	}
}
void HUDLayer::updateRelicHUD() {
	HUD.find("Current Relic")->second->removeFromParent();
	HUD.erase("Current Relic");

	std::string relic = m_player->getRelic()->getName();
	std::string image = m_player->getRelic()->getImageName();

	Sprite* currentRelic = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentRelic, Z_HUD_SPRITE, relic);
	currentRelic->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	currentRelic->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("Current Relic", currentRelic));
}
void HUDLayer::deconstructRelicHUD() {
	if (HUD.find("Relic Box") != HUD.end()) {
		// deconstruct the trinket HUD because there's no trinket equipped
		HUD.find("Current Relic")->second->removeFromParent();
		HUD.find("Relic Box")->second->removeFromParent();

		HUD.erase(HUD.find("Current Relic"));
		HUD.erase(HUD.find("Relic Box"));
	}
}

void HUDLayer::constructWeaponHUD() {
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* wepbox = Sprite::createWithSpriteFrameName(image);
	wepbox->setPosition(-870 * RES_ADJUST, 390 * RES_ADJUST);
	wepbox->setScale(.4f * RES_ADJUST);
	wepbox->setOpacity(160);
	this->addChild(wepbox, Z_HUD_ELEMENT, "wepbox");
	HUD.insert(std::pair<std::string, Sprite*>("wepbox", wepbox));

	// load default weapon sprite
	std::string weapon;
	weapon = m_player->getWeapon()->getName();
	image = m_player->getWeapon()->getImageName();
	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, Z_HUD_SPRITE, weapon);
	currentwep->setPosition(-870 * RES_ADJUST, 390 * RES_ADJUST);
	currentwep->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));

	// if weapon has casting ability, add label
	if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();
}
void HUDLayer::checkWeaponHUD() {
	std::string weapon = m_player->getWeapon()->getName();
	std::string image = m_player->getWeapon()->getImageName();

	if (keyLabels.find("cast key") != keyLabels.end())
		keyLabels.find("cast key")->second->setString(convertKeycodeToStr(CAST_KEY));

	// if current weapon equipped is different, switch the weapon sprite
	if (HUD.find("currentwep")->second->getName() != weapon)
		updateWeaponHUD();

	// If there was a casting label that can be removed because the player used the special, remove it
	else if (!m_player->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end())
		removeWeaponCastLabel();

	else if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();
}
void HUDLayer::updateWeaponHUD() {
	HUD.find("currentwep")->second->removeFromParent();
	HUD.erase("currentwep");

	std::string weapon = m_player->getWeapon()->getName();
	std::string image = m_player->getWeapon()->getImageName();

	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, Z_HUD_SPRITE, weapon);
	currentwep->setPosition(-870.f * RES_ADJUST, 390.f * RES_ADJUST);
	currentwep->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));

	// if weapon has casting ability, add label
	if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();

	// else remove the label if previous weapon could be cast
	else if (!m_player->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end())
		removeWeaponCastLabel();
}
void HUDLayer::addWeaponCastLabel() {
	if (keyLabels.find("cast key") == keyLabels.end()) {
		auto castKey = Label::createWithTTF(convertKeycodeToStr(CAST_KEY), TEXT_FONT, 18);
		castKey->setPosition(-870.f * RES_ADJUST, 362.0f * RES_ADJUST);
		castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
		//castKey->setOpacity(255);
		this->addChild(castKey, Z_HUD_LABEL);
		keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
	}
}
void HUDLayer::removeWeaponCastLabel() {
	auto it = keyLabels.find("cast key");
	it->second->removeFromParent();
	keyLabels.erase(it);
}

void HUDLayer::deconstructShopHUD() {
	m_itemPrice->removeFromParent();
	m_itemPrice = nullptr;
}
void HUDLayer::deconstructBossHUD() {
	if (HUD.find("bosshp") != HUD.end()) {
		// deconstruct the boss health bar because smasher is dead
		HUD.find("bosshp")->second->removeFromParent();
		HUD.find("bosshealthbar")->second->removeFromParent();

		HUD.erase(HUD.find("bosshp"));
		HUD.erase(HUD.find("bosshealthbar"));
	}
}

void HUDLayer::createPersistentLabel(float x, float y, const std::string &text, const std::string &id, float fontSize) {
	cocos2d::Label *label = Label::createWithTTF(text, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	label->setOpacity(230);
	label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	//label->setAdditionalKerning(0.25f);
	this->addChild(label, Z_HUD_LABEL);

	m_persistentLabels.insert(std::make_pair(id, label));
}
void HUDLayer::addLabel(float x, float y, const std::string &text, const std::string &id, float fontSize) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto label = Label::createWithTTF(text, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	//label->setOpacity(230);
	//label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	//label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	//label->setAdditionalKerning(0.25f);
	this->addChild(label, Z_HUD_LABEL);
	labels.insert(std::make_pair(id, label));
}
void HUDLayer::updateLabel(const std::string &id, std::string newText) {
	labels.find(id)->second->setString(newText);
}

void HUDLayer::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, node);
}
void HUDLayer::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}

void HUDLayer::enableGameplayListeners() {
	m_keyRelease = EventListenerKeyboard::create();
	m_keyRelease->onKeyReleased = CC_CALLBACK_2(HUDLayer::menuKeyReleased, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(m_keyRelease, this);

	m_buttonRelease = EventListenerController::create();
	m_buttonRelease->onKeyDown = CC_CALLBACK_3(HUDLayer::menuButtonReleased, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(m_buttonRelease, this);
}
void HUDLayer::menuKeyReleased(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ENTER:
	case KeyType::KEY_ESCAPE:
	default:
		this->_eventDispatcher->removeEventListener(m_keyRelease);
		this->_eventDispatcher->removeEventListener(m_buttonRelease);
		m_scene->enableListeners();
		//activeListener->setEnabled(true);
		//activeListener = nullptr;
	}
}
void HUDLayer::menuButtonReleased(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	menuKeyReleased(KeyType::KEY_ENTER, event);
}


//		BACKGROUND LAYER
bool BackgroundLayer::init() {
	if (!Layer::init())
		return false;


	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	std::string image;

	Sprite* background1 = Sprite::create("Level1bg1.png");
	background1->setPosition(Vec2(origin.x, origin.y));
	/// Used for setting correct position when using fancy transition 
	///background1->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
	this->addChild(background1, -5);

	Sprite* background2 = Sprite::create("Level1bg2.png");
	background2->setPosition(Vec2(origin.x, origin.y));
	//background2->setScale(1.0);
	this->addChild(background2, -6);

	Sprite* background3 = Sprite::create("Level1bg3.png");
	background3->setPosition(Vec2(origin.x, origin.y));
	//background3->setScale(1.0);
	this->addChild(background3, -7);

	return true;
}
void BackgroundLayer::updateBackground() {

}


//		SCENE FOR LEVELS
LevelScene::LevelScene(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level) : m_hud(hud), m_cameraLayer(cameraLayer), p(p), m_level(level) {
	
}
LevelScene::~LevelScene() {
	if (m_currentDungeon != nullptr)
		delete m_currentDungeon;
}

Scene* LevelScene::createScene(std::shared_ptr<Player> p, int level) {
	auto scene = Scene::create();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// create HUD layer
	HUDLayer* hud = HUDLayer::create(p);
	hud->setPosition(Vec2(vsw, vsh));
	scene->addChild(hud, 10);

	cocos2d::Node* cameraLayer = Node::create();
	scene->addChild(cameraLayer, 5);

	// create background layer
	//BackgroundLayer* bglayer = BackgroundLayer::create();
	//scene->addChild(bglayer, -10);

	// calls LevelScene init()
	LevelScene* levelScene = LevelScene::create(hud, cameraLayer, p, level);
	hud->setScene(levelScene);
	scene->addChild(levelScene, 1, "Level Scene");

	return scene;
}
LevelScene* LevelScene::create(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level)
{
	LevelScene *pRet = new(std::nothrow) LevelScene(hud, cameraLayer, p, level);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool LevelScene::init() {
	if (!Scene::init())
		return false;
	
	createLightEntity();

	// Create the dungeon
	setCurrentDungeon(m_level, p);

	m_hud->updateHUD();

	setMusic(m_currentDungeon->getLevel());

	// Reveal boss hp bar, if necessary
	if (m_currentDungeon->getLevel() == FIRST_BOSS)
		m_hud->showBossHP();

	// Hides the extra game layer!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1);

	createPlayerSpriteAndCamera();

	/// nonsense camera
	//auto visibleSize = Director::getInstance()->getVisibleSize();
	//removeChild(getDefaultCamera());
	//m_gameCamera = Camera::createPerspective(50.0f, 1920.f / 1080.f, 1.0, 2000);
	////m_gameCamera->setCameraMask((unsigned short)CameraFlag::DEFAULT, true); // mask on the node
	//m_gameCamera->lookAt(Vec3(0, 0, 0)); // camera turns to look at this point
	//m_gameCamera->setPositionZ(100);
	//this->addChild(m_gameCamera);

	updateLevelLighting();
		
	createGameplayListeners();
	
	//scheduleTimer();

	return true;
}

void LevelScene::createGameplayListeners() {
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(LevelScene::LevelKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, p->getSprite()); // check this for player

	controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = CC_CALLBACK_3(LevelScene::LevelButtonPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, p->getSprite());
}
void LevelScene::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, node);
}
void LevelScene::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}
cocos2d::Sprite* LevelScene::createSprite(std::string image, int x, int y, int z) {
	auto vSize = Director::getInstance()->getVisibleSize();
	auto vWidth = vSize.width;
	auto vHeight = vSize.height;

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	//sprite->visit();

	this->addChild(sprite, z);
	//graySprite(sprite);

	return sprite;

	//// Create new Sprite without scale, which perfoms much better
	//auto newSprite = Sprite::createWithTexture(renderTexture->getSprite()->getTexture());
	//newSprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (rows - y) - Y_OFFSET);
	//addChild(newSprite, z);
}
cocos2d::Sprite* LevelScene::createAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	this->addChild(sprite, z);

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto action = sprite->runAction(cocos2d::RepeatForever::create(cocos2d::Animate::create(animation)));
	action->setTag(10);

	return sprite;
}
/*
void LevelScene::graySprite(Sprite* sprite) {
	if (sprite)
	{
		GLProgram * p = new GLProgram();
		p->initWithFilenames("pass.vsh", "shadowMap.fsh");
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION);
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR);
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORDS);
		p->link();
		p->updateUniforms();
		sprite->setShaderProgram(p);
	}
}*/
EffectSprite* LevelScene::createEffectSprite(std::string image, int x, int y, int z) {
	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	EffectSprite* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	sprite->visit();
	sprite->setEffect(m_lighting, "SpritesheetNormals_Dummy_n.png");

	this->addChild(sprite, z);

	return sprite;
}
void LevelScene::setLightingOn(EffectSprite* sprite) {
	sprite->setEffect(m_lighting, "SpritesheetNormals_Dummy_n.png");
}
void LevelScene::movePlayerLightTo(float x, float y) {
	m_lighting->setLightPos(Vec3(x, y, 1000));
}

void LevelScene::LevelKeyPressed(KeyType keyCode, Event* event) {
	// Unschedule the inaction timer
	if (!(keyCode == INVENTORY_KEY || keyCode == ITEM_KEY ||
		keyCode == PAUSE_KEY || keyCode == KeyType::KEY_ESCAPE))
		unscheduleTimer();

	auto actions = this->getActionManager();

	// Finish any sprite actions that are allowed to finish early
	while (actions->getNumberOfRunningActionsByTag(1) > 0)
		actions->update(1.0, 1);

	int currentLevel = m_currentDungeon->getLevel();

	if (keyCode == LEFT_KEY) {
		m_currentDungeon->peekDungeon('l');

		if (m_facing == 'r') {
			p->getSprite()->setScaleX(p->getSprite()->getScaleX() * -1);
			m_facing = 'l';
		}
	}
	else if (keyCode == RIGHT_KEY) {
		m_currentDungeon->peekDungeon('r');

		if (m_facing == 'l') {
			p->getSprite()->setScaleX(p->getSprite()->getScaleX() * -1);
			m_facing = 'r';
		}
	}
	else if (keyCode == UP_KEY) {
		m_currentDungeon->peekDungeon('u');
	}
	else if (keyCode == DOWN_KEY) {
		m_currentDungeon->peekDungeon('d');
	}
	else if (keyCode == QUICK_KEY) {
		if (m_currentDungeon->getPlayer()->hasItems())
			m_currentDungeon->callUse(0);

		m_currentDungeon->peekDungeon('-');
	}
	else if (keyCode == CAST_KEY) {
		m_currentDungeon->peekDungeon(WIND_UP);
	}
	else if (keyCode == ACTIVE_KEY) {
		m_currentDungeon->peekDungeon('b');
	}
	else if (keyCode == INTERACT_KEY) {
		m_currentDungeon->peekDungeon('e');

		if (playerAdvanced(currentLevel)) {
			advanceLevel();
			return;
		}
	}
	else if (keyCode == INVENTORY_KEY) {
		disableListeners();
		m_hud->inventoryMenu();
		return;
	}
	else if (keyCode == WEAPON_KEY) {
		m_currentDungeon->peekDungeon('w');
	}
	else if (keyCode == ITEM_KEY) {
		disableListeners();
		m_hud->itemMenu();
		return;
	}
	else if (keyCode == KeyType::KEY_M) {
		if (cocos2d::experimental::AudioEngine::getVolume(m_musicID) > 0)
			cocos2d::experimental::AudioEngine::setVolume(m_musicID, 0.0);
		else
			cocos2d::experimental::AudioEngine::setVolume(m_musicID, GLOBAL_MUSIC_VOLUME);
	}
	else if (keyCode == PAUSE_KEY || keyCode == KeyType::KEY_ESCAPE) {
		pauseMenu();
		return;
	}
	else if (keyCode == KeyType::KEY_EQUAL) {
		m_currentDungeon->zoomInBy(0.02f);
	}
	else if (keyCode == KeyType::KEY_MINUS) {
		m_currentDungeon->zoomOutBy(0.02f);
	}
	else {
		m_currentDungeon->peekDungeon('-');
	}

	// Check if player return to menu from World Hub
	if (m_currentDungeon->returnedToMenu())
		return;

	m_hud->updateHUD();

	if (m_currentDungeon->getLevel() == FIRST_BOSS)
		m_hud->updateBossHUD();

	if (m_currentDungeon->getPlayer()->isDead()) {
		cocos2d::experimental::AudioEngine::stop(m_musicID);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}

	if (m_currentDungeon->getPlayer()->getWin()) {
		m_hud->winner();
		return; // prevents timer from being scheduled
	}

	scheduleTimer();
}
void LevelScene::LevelButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	LevelKeyPressed(gameplayButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void LevelScene::scheduleTimer() {
	if (m_preventTimerScheduling) {
		m_preventTimerScheduling = false;
		return;
	}

	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();
		
		// Finish any sprite actions that are allowed to finish early
		while (actions->getNumberOfRunningActionsByTag(1) > 0)
			actions->update(1.0, 1);

		m_currentDungeon->peekDungeon('-');

		m_hud->updateHUD();

		if (m_currentDungeon->getPlayer()->isDead())
			m_hud->gameOver();
		
	}, this, getTimerSpeed(), false, "Timer");
}
void LevelScene::unscheduleTimer() {
	Director::getInstance()->getScheduler()->unschedule("Timer", this);
}

void LevelScene::callFactoryTileCreation() {
	// Prevents multiple instances of this callback occurring if used by cloned Players.
	if (cocos2d::Director::getInstance()->getScheduler()->isScheduled("Factory Tile Timer", this))
		return;

	disableListeners();
	unscheduleTimer();

	m_preventTimerScheduling = true;

	int x = p->getPosX();
	int y = p->getPosY();

	static bool isPlacingTile = true;
	static char dir = 'r';

	std::string image;
	switch (dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}

	cocos2d::Sprite* sprite = m_currentDungeon->createSprite(x, y, y + Z_FLOATERS, image);
	runAction(cocos2d::Follow::create(sprite));

	Director::getInstance()->getScheduler()->schedule([this, sprite](float) {
		isPlacingTile = true;
		sprite->removeFromParent();

		cocos2d::Director::getInstance()->getScheduler()->unschedule("Factory Tile Timer", this);
		runAction(cocos2d::Follow::create(p->getSprite()));
		createGameplayListeners();
		scheduleTimer();
	}, this, 10.0f, false, "Factory Tile Timer");

	createKeyboardEventListener(CC_CALLBACK_2(LevelScene::factoryTileKeyPressed, this, sprite, isPlacingTile, dir), sprite);
	createControllerEventListener(CC_CALLBACK_3(LevelScene::factoryTileButtonPressed, this, sprite, isPlacingTile, dir), sprite);
}
void LevelScene::factoryTileKeyPressed(KeyType keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir) {
	auto actions = getActionManager();
	while (actions->getNumberOfRunningActionsByTag(1) > 0)
		actions->update(1.0, 1);

	cocos2d::Vec2 pos = event->getCurrentTarget()->getPosition();
	Coords dungeonCoords = m_currentDungeon->transformSpriteToDungeonCoordinates(pos);
	float duration = 0.08f;

	if (keyCode == LEFT_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(-1, 0));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else {
			dir = 'l';
		}
	}
	else if (keyCode == RIGHT_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(1, 0));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'r';
	}
	else if (keyCode == UP_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(0, -1));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'u';
	}
	else if (keyCode == DOWN_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(0, 1));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'd';
	}
	else if (keyCode == KeyType::KEY_SPACE) {
		if (!(m_currentDungeon->wall(dungeonCoords.x, dungeonCoords.y) || m_currentDungeon->trap(dungeonCoords.x, dungeonCoords.y))) {
			playInterfaceSound("Confirm 1.mp3");

			if (isPlacingTile) {
				isPlacingTile = false;			
			}
			else {
				isPlacingTile = true;
				m_currentDungeon->addTrap(std::make_shared<FactoryTile>(*m_currentDungeon, dungeonCoords.x, dungeonCoords.y, dir));
			}
		}
		else {
			playSound("Insufficient_Funds.mp3");
		}
	}
	else if (keyCode == KeyType::KEY_ESCAPE) {
		if (!isPlacingTile)
			isPlacingTile = true;		
	}

	if (keyCode == LEFT_KEY || keyCode == RIGHT_KEY || keyCode == UP_KEY || keyCode == DOWN_KEY) {
		if (!isPlacingTile) {	
			playInterfaceSound("Confirm 1.mp3");

			std::string image;
			switch (dir) {
			case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
			case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
			case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
			case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
			}
			target->setSpriteFrame(image);
		}
	}
}
void LevelScene::factoryTileButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir) {
	factoryTileKeyPressed(gameplayButtonToKey(static_cast<ButtonType>(keyCode)), event, target, isPlacingTile, dir);
}

void LevelScene::createLightEntity() {
	m_lighting = LightEffect::create();
	m_lighting->retain();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	Vec3 lightPos(vsw, vsh, 1000);
	m_lighting->setLightPos(lightPos);
	m_lighting->setLightCutoffRadius(500000);
	//m_lighting->setLightHalfRadius(5000);
	m_lighting->setBrightness(1.0);
	//m_lighting->setLightColor(Color3B(200, 170, 200));
	m_lighting->setAmbientLightColor(cocos2d::Color3B(0, 0, 0));
}
void LevelScene::setCurrentDungeon(int level, std::shared_ptr<Player> player) {
	switch (level) {
	case TUTORIAL: m_currentDungeon = new TutorialFloor(this, player); break;
	case WORLD_HUB: m_currentDungeon = new WorldHub(this, player); break;

	case FIRST_FLOOR: m_currentDungeon = new FirstFloor(this, player); break;
	case SECOND_FLOOR: m_currentDungeon = new SecondFloor(this, player); break;
	case THIRD_FLOOR: m_currentDungeon = new ThirdFloor(this, player); break;
	case FOURTH_FLOOR: m_currentDungeon = new FourthFloor(this, player); break;
	case FIFTH_FLOOR: m_currentDungeon = new FifthFloor(this, player); break;
	case SIXTH_FLOOR: m_currentDungeon = new SixthFloor(this, player); break;
	case SEVENTH_FLOOR: m_currentDungeon = new SeventhFloor(this, player); break;
	case EIGHTH_FLOOR: m_currentDungeon = new EighthFloor(this, player); break;
	case NINTH_FLOOR: m_currentDungeon = new NinthFloor(this, player); break;

	case FIRST_BOSS: m_currentDungeon = new FirstBoss(this, player); break;

	case FIRST_SHOP:
	//case SECOND_SHOP:
	case THIRD_SHOP:
	case FOURTH_SHOP: m_currentDungeon = new Shop(this, player, level); break;

	case FIRST_SHRINE:
	case SECOND_SHRINE:
	case THIRD_SHRINE:
	case FOURTH_SHRINE: m_currentDungeon = new Shrine(this, player, level); break;
	}

	player->setDungeon(m_currentDungeon);
}
void LevelScene::createPlayerSpriteAndCamera() {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	cocos2d::Vec2 pos = m_currentDungeon->transformDungeonToSpriteCoordinates(p->getPos());

	p->setSprite(m_currentDungeon->createSprite(p->getPos(), getHalfTileYOffset(), p->getPosY() + Z_ACTOR, p->getImageName()));
	
	m_lighting->setLightPos(Vec3(pos.x, pos.y, 5));

	switch (m_currentDungeon->getLevel()) {
	case FIRST_BOSS:
		this->setScale(0.4f);
		break;

	default:
		m_follow = cocos2d::Follow::create(p->getSprite());
		this->runAction(m_follow);
		break;
	}
}
void LevelScene::setMusic(int level) {
	//cocos2d::experimental::AudioEngine::setMaxAudioInstance(32);

	std::string music;
	switch (level) {
	case FIRST_FLOOR: music = "Exploring a cave.mp3"; break;
	case FIRST_SHRINE: music = "mistical.mp3"; break;
	case SECOND_FLOOR: music = "Tower of Lava.mp3"; break;
	case FIRST_SHOP: music = "mistical.mp3"; break;
	case THIRD_FLOOR: music = "Who turned off the lights.mp3"; break;
	case FIRST_BOSS: music = "Zero Respect.mp3"; break;
	}

	m_musicID = playMusic(music, true);
}
void LevelScene::updateLevelLighting() {
	switch (m_currentDungeon->getLevel()) {
	case FIRST_BOSS: break;
	default: m_currentDungeon->updateLighting(); break;
	}

	// hide rooms if third level
	if (m_currentDungeon->getLevel() == THIRD_FLOOR) {
		auto third = dynamic_cast<ThirdFloor*>(m_currentDungeon);
		third->hideRooms();
	}
}
float LevelScene::getTimerSpeed() {
	float speed;
	switch (m_currentDungeon->getLevel()) {
	case FIRST_FLOOR: speed = 0.70f; break;
	case SECOND_FLOOR: speed = 0.65f; break;
	case THIRD_FLOOR: speed = 0.60f; break;
	case FOURTH_FLOOR: speed = 0.55f; break;
	case FIFTH_FLOOR: speed = 0.50f; break;
	case SIXTH_FLOOR: speed = 0.45f; break;
	case SEVENTH_FLOOR: speed = 0.40f; break;
	case EIGHTH_FLOOR: speed = 0.35f; break;
	case FIRST_BOSS: speed = 0.30f; break;
	default: speed = 0.70f; break;
	}

	speed += m_currentDungeon->getPlayer()->getTimerReduction();
	return speed;
}

void LevelScene::pauseMenu() {
	Director::getInstance()->getScheduler()->pauseTarget(this);
	GameTimers::pauseAllGameTimers();

	auto pauseMenuScene = PauseMenuScene::createScene(this);

	Director::getInstance()->pushScene(pauseMenuScene);
}
void LevelScene::advanceLevel() {
	// Unschedule all timers and event listeners
	unscheduleTimer();
	GameTimers::pauseAllGameTimers();
	disableListeners();

	cocos2d::experimental::AudioEngine::stop(m_musicID);
	
	auto nextScene = LevelScene::createScene(m_currentDungeon->getPlayer(), m_currentDungeon->getLevel());

	auto transition = TransitionFade::create(0.75f, nextScene);

	transition->setOnExitCallback([nextScene]() {
		auto levelScene = dynamic_cast<LevelScene*>(nextScene->getChildByName("Level Scene"));
		levelScene->scheduleTimer();
		GameTimers::resumeAllGameTimers();
	});

	Director::getInstance()->replaceScene(transition);
}
inline bool LevelScene::playerAdvanced(int level) { 
	return m_currentDungeon->getLevel() != level;
};
void LevelScene::returnToMainMenu() {
	// Unschedule the inaction timer and event listener
	unscheduleTimer();
	GameTimers::removeAllGameTimers();
	_eventDispatcher->removeEventListener(kbListener);
	_eventDispatcher->removeEventListener(controllerListener);

	// stop music
	cocos2d::experimental::AudioEngine::stopAll();

	// advance to start menu scene
	auto startScene = StartScene::createScene();

	auto transition = TransitionFade::create(3.0f, startScene);
	Director::getInstance()->replaceScene(transition);
}
void LevelScene::showShopHUD(int x, int y) {
	// x and y are below the item's coordinates,
	// We want the price to show two above that.

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y - 2));

	// pricing symbols, prices themselves, etc.
	m_itemPrice = Label::createWithTTF("$", TEXT_FONT, 24);
	m_itemPrice->setPosition(finalPos);
	m_itemPrice->setColor(cocos2d::Color3B(255, 215, 0));
	m_itemPrice->setString("$" + std::to_string(m_currentDungeon->tileAt(Coords(x, y - 1)).price));
	this->addChild(m_itemPrice, Z_HUD_LABEL);
}
void LevelScene::deconstructShopHUD() {
	if (m_itemPrice != nullptr) {
		m_itemPrice->removeFromParent();
		m_itemPrice = nullptr;
	}
}


//		PAUSE MENU SCENE
PauseMenuScene::PauseMenuScene(LevelScene *levelScene) : m_levelScene(levelScene) {
	id = levelScene->getMusicId();
}

Scene* PauseMenuScene::createScene(LevelScene *levelScene) {
	auto scene = Scene::create();

	// calls init()
	auto layer = PauseMenuScene::create(levelScene);
	scene->addChild(layer);

	return scene;
}
PauseMenuScene* PauseMenuScene::create(LevelScene *levelScene) {
	PauseMenuScene *pRet = new(std::nothrow) PauseMenuScene(levelScene);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool PauseMenuScene::init() {
	if (!Scene::init())
		return false;
	
	cocos2d::experimental::AudioEngine::pauseAll();


	//       PAUSE
	//    |----------|
	// -> |  Resume  |
	//    | Restart  |
	//    |  Options |
	//    |   Help   |
	//    | Main Menu|
	//    | Exit Game|
	//    |----------|


	// Menu border
	addSprite(0.0f, 0.0f, 2, "Pause_Menu_Border_Red.png", "Pause Menu");
	sprites.find("Pause Menu")->second->setScale(0.3f);
	sprites.find("Pause Menu")->second->setOpacity(170);

	// Pause option
	addLabel(0.0f, 4.8f, fetchMenuText("Pause Menu", "Paused"), "Paused", 48);

	// Resume option
	addLabel(0.0f, 2.5f, fetchMenuText("Pause Menu", "Resume"), "Resume", 36);

	// Restart option
	addLabel(0.0f, 1.5f, fetchMenuText("Pause Menu", "Restart"), "Restart", 36);

	// Options
	addLabel(0.0f, 0.5f, fetchMenuText("Pause Menu", "Options"), "Options", 36);

	// "How to play" option
	addLabel(0.0f, -0.5f, fetchMenuText("Pause Menu", "Help"), "Help", 36);

	// Back to Menu option
	addLabel(0.0f, -1.5f, fetchMenuText("Pause Menu", "Main Menu"), "Main Menu", 36);

	// Quit option
	addLabel(0.0f, -2.5f, fetchMenuText("Pause Menu", "Exit Game"), "Exit Game", 36);


	auto selector = createSelectorSprite(-2.0f, 2.5f);
	createKeyboardEventListener(CC_CALLBACK_2(PauseMenuScene::pauseMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(PauseMenuScene::pauseMenuButtonPressed, this), selector);

	return true;
}

void PauseMenuScene::pauseMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0) {
			index = 5;
			event->getCurrentTarget()->setPosition(pos.x, -2.5 * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 5) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 5) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 2.5 * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// Resume
		case 0: { 
			resumeGame();
			return;
		}
			// Restart
		case 1: { 
			restartGame(*m_levelScene->getCurrentDungeon()->getPlayer());
			return;
		}
			// Options	
		case 2: {
			playInterfaceSound("Confirm 1.mp3");

			m_forward = true;
			options();

			break;
		}
			// Help menu
		case 3: { 
			playInterfaceSound("Confirm 1.mp3");

			m_forward = true;
			helpScreen();
			break;
		}
			// Main Menu
		case 4: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			GameTimers::removeAllGameTimers();
			playInterfaceSound("Confirm 1.mp3");

			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene);
			return;
		}
			// Exit Game
		case 5: {
			playInterfaceSound("Confirm 1.mp3");

			Director::getInstance()->end();
			return;
		}
		}
		break;
	//case KeyType::KEY_P:
	case KeyType::KEY_ESCAPE: {
		resumeGame();
		return;
	}
	default:
		if (keyCode == PAUSE_KEY) {
			resumeGame();
			return;
		}
	}
}
void PauseMenuScene::pauseMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	pauseMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void PauseMenuScene::resumeGame() {
	index = 0;
	cocos2d::experimental::AudioEngine::resumeAll();
	cocos2d::Director::getInstance()->popScene();
	cocos2d::Director::getInstance()->getScheduler()->resumeTarget(m_levelScene);
	GameTimers::resumeAllGameTimers();
}

void PauseMenuScene::helpScreen() {
	setPrevMenuState();

	removeAll();

	// Menu
	addSprite(0, 0, 2, "Pause_Menu_Border_Red.png", "Menu");
	sprites.find("Menu")->second->setScale(0.4f);
	sprites.find("Menu")->second->setOpacity(170);

	// Go back
	addLabel(0.0f, -5.2f, fetchMenuText("Help Menu", "OK"), "OK", 36);

	// HOW TO PLAY
	addLabel(0.0f, 5.0f, fetchMenuText("Help Menu", "How to play"), "Help", 48);


	// Movement
	addLabel(-5.0f, 2.85f, fetchMenuText("Help Menu", "Movement"), "Movement", 28);

	// Arrow keys
	addSprite(0.0f, 3.35f, 4, "KB_Arrows_U.png", "Arrow Up");
	sprites.find("Arrow Up")->second->setScale(0.8f);

	addSprite(0.0f, 2.6f, 4, "KB_Arrows_LDR.png", "Arrows");
	sprites.find("Arrows")->second->setScale(0.8f);


	// Interact
	addLabel(-5.0f, 1.5f, fetchMenuText("Help Menu", "Interact"), "Interact", 28);

	addSprite(0.0f, 1.5f, 4, "KB_Black_E.png", "Interact Key");
	sprites.find("Interact Key")->second->setScale(0.8f);


	// Quick Use key
	addLabel(-5.0f, 0.5f, fetchMenuText("Help Menu", "Use Item"), "Item", 28);

	addSprite(0.0f, 0.5f, 4, "KB_Black_Q.png", "Item Key");
	sprites.find("Item Key")->second->setScale(0.8f);


	// Active Item
	addLabel(-5.0f, -0.5f, fetchMenuText("Help Menu", "Use Active Item"), "Active", 28);

	addLabel(0.0f, -0.5f, fetchMenuText("Help Menu", "Space"), "Space", 28);
	labels.find("Space")->second->setLocalZOrder(5);

	addSprite(0.0f, -0.5f, 4, "KB_Black_Space.png", "Active Key");
	sprites.find("Active Key")->second->setScale(0.8f);


	// Switch weapon
	addLabel(-5.0f, -1.5f, fetchMenuText("Help Menu", "Switch Weapon"), "Weapon", 28);

	addSprite(0.0f, -1.5f, 4, "KB_Black_W.png", "Weapon Key");
	sprites.find("Weapon Key")->second->setScale(0.8f);


	// Open/close item menu
	addLabel(-5.0f, -2.5f, fetchMenuText("Help Menu", "Open/Close Item Menu"), "Item Menu", 28);

	addSprite(0.0f, -2.5f, 4, "KB_Black_C.png", "Item Menu Key");
	sprites.find("Item Menu Key")->second->setScale(0.8f);


	// View inventory
	addLabel(-5.0f, -3.5f, fetchMenuText("Help Menu", "Check Inventory"), "Inventory", 28);

	addSprite(0.0f, -3.5f, 4, "KB_Black_Tab.png", "Inventory Key");
	sprites.find("Inventory Key")->second->setScale(0.8f);
	

	auto selector = createSelectorSprite(-2.0f, -5.2f);
	createKeyboardEventListener(CC_CALLBACK_2(PauseMenuScene::helpMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(PauseMenuScene::helpMenuButtonPressed, this), selector);
}
void PauseMenuScene::helpMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ESCAPE: {
		playInterfaceSound("Confirm 1.mp3");

		removeAll();

		init();

		m_forward = false;
		restorePrevMenuState();
	}
	default: break;
	}
}
void PauseMenuScene::helpMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	helpMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}


// Converts Key Code to string for use in labels
std::string convertKeycodeToStr(KeyType keyCode) {
	std::string key;
	switch (keyCode) {
	case KeyType::KEY_NONE:
		key = "None";
		break;
	case KeyType::KEY_PAUSE:
		key = "Pause";
		break;
	case KeyType::KEY_SCROLL_LOCK:
		key = "Scroll Lock";
		break;
	case KeyType::KEY_PRINT:
		key = "Print";
		break;
	case KeyType::KEY_SYSREQ:
		key = "SysReq";
		break;
	case KeyType::KEY_BREAK:
		key = "Break";
		break;
	case KeyType::KEY_ESCAPE:
		key = "Escape";
		break;
	case KeyType::KEY_BACKSPACE:
		key = "Backspace";
		break;
	case KeyType::KEY_TAB:
		key = "Tab";
		break;
	case KeyType::KEY_BACK_TAB:
		key = "Back Tab";
		break;
	case KeyType::KEY_RETURN:
		key = "Return";
		break;
	case KeyType::KEY_CAPS_LOCK:
		key = "Caps Lock";
		break;
	case KeyType::KEY_LEFT_SHIFT:
		key = "Left Shift";
		break;
	case KeyType::KEY_RIGHT_SHIFT:
		key = "Right Shift";
		break;
	case KeyType::KEY_LEFT_CTRL:
		key = "Left Ctrl";
		break;
	case KeyType::KEY_RIGHT_CTRL:
		key = "Right Ctrl";
		break;
	case KeyType::KEY_LEFT_ALT:
		key = "Left Alt";
		break;
	case KeyType::KEY_RIGHT_ALT:
		key = "Right Alt";
		break;
	case KeyType::KEY_MENU:
		key = "Menu";
		break;
	case KeyType::KEY_HYPER:
		key = "Hyper";
		break;
	case KeyType::KEY_INSERT:
		key = "Insert";
		break;
	case KeyType::KEY_HOME:
		key = "Home";
		break;
	case KeyType::KEY_PG_UP:
		key = "Page Up";
		break;
	case KeyType::KEY_DELETE:
		key = "Delete";
		break;
	case KeyType::KEY_END:
		key = "End";
		break;
	case KeyType::KEY_PG_DOWN:
		key = "Page Down";
		break;
	case KeyType::KEY_LEFT_ARROW:
		key = "Left Arrow";
		break;
	case KeyType::KEY_RIGHT_ARROW:
		key = "Right Arrow";
		break;
	case KeyType::KEY_UP_ARROW:
		key = "Up Arrow";
		break;
	case KeyType::KEY_DOWN_ARROW:
		key = "Down Arrow";
		break;
	case KeyType::KEY_NUM_LOCK:
		key = "Num Lock";
		break;
	case KeyType::KEY_KP_PLUS:
		key = "Keypad Plus";
		break;
	case KeyType::KEY_KP_MINUS:
		key = "Keypad Minus";
		break;
	case KeyType::KEY_KP_MULTIPLY:
		key = "Keypad Multiply";
		break;
	case KeyType::KEY_KP_DIVIDE:
		key = "Keypad Divide";
		break;
	case KeyType::KEY_KP_ENTER:
		key = "Keypad Enter";
		break;
	case KeyType::KEY_KP_HOME:
		key = "Keypad Home";
		break;
	case KeyType::KEY_KP_UP:
		key = "Keypad Up";
		break;
	case KeyType::KEY_KP_PG_UP:
		key = "Keypad Page Up";
		break;
	case KeyType::KEY_KP_LEFT:
		key = "Keypad Left";
		break;
	case KeyType::KEY_KP_FIVE:
		key = "Keypad 5";
		break;
	case KeyType::KEY_KP_RIGHT:
		key = "Keypad Right";
		break;
	case KeyType::KEY_KP_END:
		key = "Keypad End";
		break;
	case KeyType::KEY_KP_DOWN:
		key = "Keypad Down";
		break;
	case KeyType::KEY_KP_PG_DOWN:
		key = "Keypad Page Down";
		break;
	case KeyType::KEY_KP_INSERT:
		key = "Keypad Insert";
		break;
	case KeyType::KEY_KP_DELETE:
		key = "Keypad Delete";
		break;
	case KeyType::KEY_F1:
		key = "F1";
		break;
	case KeyType::KEY_F2:
		key = "F2";
		break;
	case KeyType::KEY_F3:
		key = "F3";
		break;
	case KeyType::KEY_F4:
		key = "F4";
		break;
	case KeyType::KEY_F5:
		key = "F5";
		break;
	case KeyType::KEY_F6:
		key = "F6";
		break;
	case KeyType::KEY_F7:
		key = "F7";
		break;
	case KeyType::KEY_F8:
		key = "F8";
		break;
	case KeyType::KEY_F9:
		key = "F9";
		break;
	case KeyType::KEY_F10:
		key = "F10";
		break;
	case KeyType::KEY_F11:
		key = "F11";
		break;
	case KeyType::KEY_F12:
		key = "F12";
		break;
	case KeyType::KEY_SPACE:
		key = "Space";
		break;
	case KeyType::KEY_EXCLAM:
		key = "!";
		break;
	case KeyType::KEY_QUOTE:
		key = "\"";
		break;
	case KeyType::KEY_NUMBER:
		key = "#";
		break;
	case KeyType::KEY_DOLLAR:
		key = "$";
		break;
	case KeyType::KEY_PERCENT:
		key = "%";
		break;
	case KeyType::KEY_CIRCUMFLEX:
		key = "^";
		break;
	case KeyType::KEY_AMPERSAND:
		key = "&";
		break;
	case KeyType::KEY_APOSTROPHE:
		key = "Apostrophe";
		break;
	case KeyType::KEY_LEFT_PARENTHESIS:
		key = "(";
		break;
	case KeyType::KEY_RIGHT_PARENTHESIS:
		key = ")";
		break;
	case KeyType::KEY_ASTERISK:
		key = "*";
		break;
	case KeyType::KEY_PLUS:
		key = "+";
		break;
	case KeyType::KEY_COMMA:
		key = ",";
		break;
	case KeyType::KEY_MINUS:
		key = "-";
		break;
	case KeyType::KEY_PERIOD:
		key = ".";
		break;
	case KeyType::KEY_SLASH:
		key = "/";
		break;
	case KeyType::KEY_0:
		key = "0";
		break;
	case KeyType::KEY_1:
		key = "1";
		break;
	case KeyType::KEY_2:
		key = "2";
		break;
	case KeyType::KEY_3:
		key = "3";
		break;
	case KeyType::KEY_4:
		key = "4";
		break;
	case KeyType::KEY_5:
		key = "5";
		break;
	case KeyType::KEY_6:
		key = "6";
		break;
	case KeyType::KEY_7:
		key = "7";
		break;
	case KeyType::KEY_8:
		key = "8";
		break;
	case KeyType::KEY_9:
		key = "9";
		break;
	case KeyType::KEY_COLON:
		key = ":";
		break;
	case KeyType::KEY_SEMICOLON:
		key = ";";
		break;
	case KeyType::KEY_LESS_THAN:
		key = "<";
		break;
	case KeyType::KEY_EQUAL:
		key = "=";
		break;
	case KeyType::KEY_GREATER_THAN:
		key = ">";
		break;
	case KeyType::KEY_QUESTION:
		key = "?";
		break;
	case KeyType::KEY_AT:
		key = "@";
		break;
	case KeyType::KEY_CAPITAL_A:
		key = "A";
		break;
	case KeyType::KEY_CAPITAL_B:
		key = "B";
		break;
	case KeyType::KEY_CAPITAL_C:
		key = "C";
		break;
	case KeyType::KEY_CAPITAL_D:
		key = "D";
		break;
	case KeyType::KEY_CAPITAL_E:
		key = "E";
		break;
	case KeyType::KEY_CAPITAL_F:
		key = "F";
		break;
	case KeyType::KEY_CAPITAL_G:
		key = "G";
		break;
	case KeyType::KEY_CAPITAL_H:
		key = "H";
		break;
	case KeyType::KEY_CAPITAL_I:
		key = "I";
		break;
	case KeyType::KEY_CAPITAL_J:
		key = "J";
		break;
	case KeyType::KEY_CAPITAL_K:
		key = "K";
		break;
	case KeyType::KEY_CAPITAL_L:
		key = "L";
		break;
	case KeyType::KEY_CAPITAL_M:
		key = "M";
		break;
	case KeyType::KEY_CAPITAL_N:
		key = "N";
		break;
	case KeyType::KEY_CAPITAL_O:
		key = "O";
		break;
	case KeyType::KEY_CAPITAL_P:
		key = "P";
		break;
	case KeyType::KEY_CAPITAL_Q:
		key = "Q";
		break;
	case KeyType::KEY_CAPITAL_R:
		key = "R";
		break;
	case KeyType::KEY_CAPITAL_S:
		key = "S";
		break;
	case KeyType::KEY_CAPITAL_T:
		key = "T";
		break;
	case KeyType::KEY_CAPITAL_U:
		key = "U";
		break;
	case KeyType::KEY_CAPITAL_V:
		key = "V";
		break;
	case KeyType::KEY_CAPITAL_W:
		key = "W";
		break;
	case KeyType::KEY_CAPITAL_X:
		key = "X";
		break;
	case KeyType::KEY_CAPITAL_Y:
		key = "Y";
		break;
	case KeyType::KEY_CAPITAL_Z:
		key = "Z";
		break;
	case KeyType::KEY_LEFT_BRACKET:
		key = "[";
		break;
	case KeyType::KEY_BACK_SLASH:
		key = "\\";
		break;
	case KeyType::KEY_RIGHT_BRACKET:
		key = "]";
		break;
	case KeyType::KEY_UNDERSCORE:
		key = "_";
		break;
	case KeyType::KEY_GRAVE:
		key = "`";
		break;
	case KeyType::KEY_A:
		key = "A";
		break;
	case KeyType::KEY_B:
		key = "B";
		break;
	case KeyType::KEY_C:
		key = "C";
		break;
	case KeyType::KEY_D:
		key = "D";
		break;
	case KeyType::KEY_E:
		key = "E";
		break;
	case KeyType::KEY_F:
		key = "F";
		break;
	case KeyType::KEY_G:
		key = "G";
		break;
	case KeyType::KEY_H:
		key = "H";
		break;
	case KeyType::KEY_I:
		key = "I";
		break;
	case KeyType::KEY_J:
		key = "J";
		break;
	case KeyType::KEY_K:
		key = "K";
		break;
	case KeyType::KEY_L:
		key = "L";
		break;
	case KeyType::KEY_M:
		key = "M";
		break;
	case KeyType::KEY_N:
		key = "N";
		break;
	case KeyType::KEY_O:
		key = "O";
		break;
	case KeyType::KEY_P:
		key = "P";
		break;
	case KeyType::KEY_Q:
		key = "Q";
		break;
	case KeyType::KEY_R:
		key = "R";
		break;
	case KeyType::KEY_S:
		key = "S";
		break;
	case KeyType::KEY_T:
		key = "T";
		break;
	case KeyType::KEY_U:
		key = "U";
		break;
	case KeyType::KEY_V:
		key = "V";
		break;
	case KeyType::KEY_W:
		key = "W";
		break;
	case KeyType::KEY_X:
		key = "X";
		break;
	case KeyType::KEY_Y:
		key = "Y";
		break;
	case KeyType::KEY_Z:
		key = "Z";
		break;
	case KeyType::KEY_LEFT_BRACE:
		key = "{";
		break;
	case KeyType::KEY_BAR:
		key = "Bar";
		break;
	case KeyType::KEY_RIGHT_BRACE:
		key = "}";
		break;
	case KeyType::KEY_TILDE:
		key = "`";
		break;
	case KeyType::KEY_EURO:
		key = "Euro";
		break;
	case KeyType::KEY_POUND:
		key = "Pound";
		break;
	case KeyType::KEY_YEN:
		key = "Yen";
		break;
	case KeyType::KEY_MIDDLE_DOT:
		key = "Middle Dot";
		break;
	case KeyType::KEY_SEARCH:
		key = "Search";
		break;
	case KeyType::KEY_DPAD_LEFT:
		key = "Dpad Left";
		break;
	case KeyType::KEY_DPAD_RIGHT:
		key = "Dpad Right";
		break;
	case KeyType::KEY_DPAD_UP:
		key = "Dpad Up";
		break;
	case KeyType::KEY_DPAD_DOWN:
		key = "Dpad Down";
		break;
	case KeyType::KEY_DPAD_CENTER:
		key = "Dpad Center";
		break;
	case KeyType::KEY_ENTER:
		key = "Enter";
		break;
	case KeyType::KEY_PLAY:
		key = "Play";
		break;
	}

	return key;
}
std::string convertButtonToStr(ButtonType button) {
	switch (button) {
	case ButtonType::BUTTON_A:
		return "A";
	case ButtonType::BUTTON_B:
		return "B";
	case ButtonType::BUTTON_C:
		return "C";
	case ButtonType::BUTTON_X:
		return "X";
	case ButtonType::BUTTON_Y:
		return "Y";
	case ButtonType::BUTTON_Z:
		return "Z";
	case ButtonType::BUTTON_LEFT_SHOULDER:
		return "Left Trigger";
	case ButtonType::BUTTON_RIGHT_SHOULDER:
		return "Right Trigger";
	case ButtonType::BUTTON_DPAD_UP:
		return "Dpad Up";
	case ButtonType::BUTTON_DPAD_DOWN:
		return "Dpad Down";
	case ButtonType::BUTTON_DPAD_LEFT:
		return "Dpad Left";
	case ButtonType::BUTTON_DPAD_RIGHT:
		return "Dpad Right";
	case ButtonType::BUTTON_START:
		return "Start";
	case ButtonType::BUTTON_SELECT:
		return "Select";
	default:
		return "UNKNOWN";
	}
}

/*
bool LevelScene::isKeyPressed(KeyType code) {
	// Check if the key is currently pressed by seeing it it's in the std::map keys
	// In retrospect, keys is a terrible name for a key/value paried datatype isnt it?
	if (keys.find(code) != keys.end())
		return true;
	return false;
}

double LevelScene::keyPressedDuration(KeyType code) {
	if (!isKeyPressed(KeyType::KEY_LEFT_ARROW))
		return 0;  // Not pressed, so no duration obviously

	// Return the amount of time that has elapsed between now and when the user
	// first started holding down the key in milliseconds
	// Obviously the start time is the value we hold in our std::map keys
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::high_resolution_clock::now() - keys[code]).count();
}

void LevelScene::update(float delta) {
	// Register an update function that checks to see if the CTRL key is pressed
	// and if it is displays how long, otherwise tell the user to press it
	Node::update(delta);
	if (isKeyPressed(KeyType::KEY_LEFT_ARROW)) {
		std::stringstream ss;
		ss << "Control key has been pressed for " <<
			keyPressedDuration(KeyType::KEY_LEFT_ARROW) << " ms";
		label->setString(ss.str().c_str());
	}
	else
		label->setString("Press the CTRL Key");
}
=======
#include "cocos2d.h"
#include "GUI.h"
#include "AppDelegate.h"
#include "AudioEngine.h"
#include "ui/cocosGUI.h"
#include "global.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "FX.h"
#include "Actors.h"
#include "LightEffect.h"
#include "EffectSprite.h"
#include <cstdlib>
#include <cmath>
#include <vector>

// 1366x768
//int X_OFFSET = 560;
//int Y_OFFSET = 170;
//float SPACING_FACTOR = 51.1f;// 60;
//int MENU_SPACING = 60;
//float RES_ADJUST = 1.08f;
//float HP_BAR_ADJUST = -2;
//float HP_ADJUST = 0;
//float SP_ADJUST = 0;

// 1600x900
//int X_OFFSET = 560;
//int Y_OFFSET = 170;
//int SPACING_FACTOR = 37;
//int MENU_SPACING = 37;
//float RES_ADJUST = 1.23;
//float HP_BAR_ADJUST = -102;
//float HP_ADJUST = -14;
//float SP_ADJUST = -14;

// 1920x1080
int X_OFFSET = 0;// 560;
int Y_OFFSET = 0;// 170;
float SPACING_FACTOR = 44.0f;
int MENU_SPACING = 60;
float RES_ADJUST = 1.0f;
float HP_BAR_ADJUST = 0;
float HP_ADJUST = 0;
float SP_ADJUST = 0;

USING_NS_CC;

// Default Keyboard Controls
KeyType UP_KEY = KeyType::KEY_UP_ARROW;
KeyType DOWN_KEY = KeyType::KEY_DOWN_ARROW;
KeyType LEFT_KEY = KeyType::KEY_LEFT_ARROW;
KeyType RIGHT_KEY = KeyType::KEY_RIGHT_ARROW;
KeyType INTERACT_KEY = KeyType::KEY_E;
KeyType QUICK_KEY = KeyType::KEY_Q;
KeyType ACTIVE_KEY = KeyType::KEY_SPACE;
KeyType WEAPON_KEY = KeyType::KEY_W;
KeyType CAST_KEY = KeyType::KEY_S;
KeyType ITEM_KEY = KeyType::KEY_C;
KeyType INVENTORY_KEY = KeyType::KEY_TAB;
KeyType PAUSE_KEY = KeyType::KEY_P;

// Default Controller Controls
ButtonType UP_BUTTON = ButtonType::BUTTON_DPAD_UP;
ButtonType DOWN_BUTTON = ButtonType::BUTTON_DPAD_DOWN;
ButtonType LEFT_BUTTON = ButtonType::BUTTON_DPAD_LEFT;
ButtonType RIGHT_BUTTON = ButtonType::BUTTON_DPAD_RIGHT;
ButtonType INTERACT_BUTTON = ButtonType::BUTTON_X;
ButtonType QUICK_BUTTON = ButtonType::BUTTON_Y;
ButtonType ACTIVE_BUTTON = ButtonType::BUTTON_A;
ButtonType WEAPON_BUTTON = ButtonType::BUTTON_RIGHT_SHOULDER;
ButtonType CAST_BUTTON = ButtonType::BUTTON_B;
ButtonType ITEM_BUTTON = ButtonType::BUTTON_SELECT;
ButtonType INVENTORY_BUTTON = ButtonType::BUTTON_LEFT_SHOULDER;
ButtonType PAUSE_BUTTON = ButtonType::BUTTON_START;

ButtonType SELECT_BUTTON = ButtonType::BUTTON_A;
ButtonType BACK_BUTTON = ButtonType::BUTTON_B;

static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}
KeyType gameplayButtonToKey(ButtonType button) {
	if (button == LEFT_BUTTON) return LEFT_KEY;
	else if (button == RIGHT_BUTTON) return RIGHT_KEY;
	else if (button == UP_BUTTON) return UP_KEY;
	else if (button == DOWN_BUTTON) return DOWN_KEY;
	else if (button == QUICK_BUTTON) return QUICK_KEY;
	else if (button == CAST_BUTTON) return CAST_KEY;
	else if (button == ACTIVE_BUTTON) return ACTIVE_KEY;
	else if (button == INTERACT_BUTTON) return INTERACT_KEY;
	else if (button == INVENTORY_BUTTON) return INVENTORY_KEY;
	else if (button == WEAPON_BUTTON) return WEAPON_KEY;
	else if (button == ITEM_BUTTON) return ITEM_KEY;
	else return PAUSE_KEY;
}
KeyType menuButtonToKey(ButtonType button) {
	if (button == SELECT_BUTTON) return KeyType::KEY_ENTER;
	else if (button == BACK_BUTTON) return KeyType::KEY_ESCAPE;
	else if (button == UP_BUTTON) return KeyType::KEY_UP_ARROW;
	else if (button == DOWN_BUTTON) return KeyType::KEY_DOWN_ARROW;
	else if (button == LEFT_BUTTON) return KeyType::KEY_LEFT_ARROW;
	else if (button == RIGHT_BUTTON) return KeyType::KEY_RIGHT_ARROW;
	else return KeyType::KEY_END;
}
void registerGamepads() {
	cocos2d::Controller::startDiscoveryController();
}
cocos2d::Controller* fetchGamepad() {
	std::vector<Controller*> controllers = cocos2d::Controller::getAllController();
	if (!controllers.empty() && controllers[0] != nullptr)
		return controllers[0];

	return nullptr;
}
void restartGame(const Player &p) {
	cocos2d::experimental::AudioEngine::stopAll();
	GameTimers::removeAllGameTimers();

	// Resets passives obtained and NPCs seen
	GameTable::initializeTables();

	std::shared_ptr<Player> newPlayer = p.createNewPlayer();

	auto levelScene = LevelScene::createScene(newPlayer, FIRST_FLOOR);

	auto transition = TransitionFade::create(0.50f, levelScene);

	transition->setOnExitCallback([levelScene]() {
		auto scene = dynamic_cast<LevelScene*>(levelScene->getChildByName("Level Scene"));
		scene->scheduleTimer();
	});

	Director::getInstance()->replaceScene(transition);
}

MenuScene::MenuScene() {
	// Fill resolution vector
	resolutions.push_back(std::make_pair(1280, 720));
	resolutions.push_back(std::make_pair(1366, 768));
	resolutions.push_back(std::make_pair(1600, 900));
	resolutions.push_back(std::make_pair(1920, 1080));
}
MenuScene::~MenuScene() {
	removeAll();
}

void MenuScene::options() {
	setPrevMenuState();

	index = 0;
	removeAll();
	
	addLabel(0, 3.0f, fetchMenuText("Options Menu", "Sound"), "Sound", 52);

	addLabel(0, 1.0f, fetchMenuText("Options Menu", "Video"), "Res", 52);

	addLabel(0, -1.0f, fetchMenuText("Options Menu", "Controls"), "Keys", 52);

	addLabel(0, -3.0f, fetchMenuText("Options Menu", "Language"), "Language", 52);

	//std::function<void(KeyType, cocos2d::Event*)> cb = std::bind(&MenuScene::optionKeyPressed, this, std::placeholders::_1, std::placeholders::_2);
	auto selector = createSelectorSprite(-3.5f, 3.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::optionKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::optionButtonPressed, this), selector);
}
void MenuScene::optionKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Sound
	// 1 : Video
	// 2 : Controls
	// 3 : Language

	int maxIndex = 3;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = maxIndex;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, -3.0f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxIndex) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 3.0f * MENU_SPACING + vsh);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = true;

		switch (index) {
			// Sound
		case 0:
			soundOptions();

			break;
			// Video
		case 1:
			videoOptions();

			break;
			// Controls
		case 2:
			controlOptions();

			break;
			// Language
		case 3: 
			languageOptions();
			
			break;	
		}

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		removeAll();
		init();

		m_forward = false;
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::optionButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		optionKeyPressed(key, event);
}

void MenuScene::soundOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	// Sound FX Volume
	addLabel(0, 3.0f, fetchMenuText("Sound Menu", "Sound Effect Volume"), "Sound", 40);
	addLabel(-1.0f, 2.1f, "Lower", "Sound Lower", 30);
	addLabel(0.0f, 2.1f, std::to_string((int)(GLOBAL_SOUND_VOLUME * 100)), "Sound Volume", 30);
	addLabel(1.0f, 2.1f, "Higher", "Sound Higher", 30);

	// Music Volume
	addLabel(0, 0, fetchMenuText("Sound Menu", "Music Volume"), "Music", 40);
	addLabel(-1.0f, -0.9f, "Lower", "Music Lower", 30);
	addLabel(0.0f, -0.9f, std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100)), "Music Volume", 30);
	addLabel(1.0f, -0.9f, "Higher", "Music Higher", 30);

	// UI Volume
	addLabel(0, -3.0f, fetchMenuText("Sound Menu", "UI Volume"), "UI", 40);
	addLabel(-1.0f, -3.9f, "Lower", "UI Lower", 30);
	addLabel(0.0f, -3.9f, std::to_string((int)(GLOBAL_UI_VOLUME * 100)), "UI Volume", 30);
	addLabel(1.0f, -3.9f, "Higher", "UI Higher", 30);

	auto selector = createSelectorSprite(-3.5f, 2.1f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::soundOptionsKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::soundOptionsButtonPressed, this), selector);
}
void MenuScene::soundOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 
	// 0 : Sound Effects
	// 1 : Music
	// 2 : UI

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_SOUND_VOLUME -= 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.1f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_MUSIC_VOLUME -= 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.1f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_UI_VOLUME -= 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.1f)));
			}
		}

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_SOUND_VOLUME += 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.1f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_MUSIC_VOLUME += 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.1f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_UI_VOLUME += 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.1f)));
			}
		}

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		options();
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::soundOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		soundOptionsKeyPressed(key, event);
}

void MenuScene::videoOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addLabel(0.0f, 2.0f, fetchMenuText("Video Menu", "Screen Resolution"), "Res", 40);
	addLabel(0.0f, 1.2f, std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second), "Screen Resolution", 30);
	addLabel(0.0f, 0.4f, fetchMenuText("Video Menu", "Fullscreen"), "Fullscreen", 30);

	auto selector = createSelectorSprite(-3.5f, 2.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::videoKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::videoButtonPressed, this), selector);
}
void MenuScene::videoKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Screen Settings
	// 1 : Resolution
	// 2 : Fullscreen

	static int maxIndex = 2;

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 1) {
			if (resolutionIndex > 0) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex--;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 1) {
			if (resolutionIndex < (int)resolutions.size() - 1) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex++;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index == 1) {
			playInterfaceSound("Select 1.mp3");
			index = 2;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 0.4f * MENU_SPACING + vsh);
		}
		else if (index == 2) {
			playInterfaceSound("Select 1.mp3");
			index = 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.8f * MENU_SPACING);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 1) {
			playInterfaceSound("Select 1.mp3");
			index = 2;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.8f * MENU_SPACING);
		}
		else if (index == 2) {
			playInterfaceSound("Select 1.mp3");
			index = 1;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 1.2f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
			// Video Settings
		case 0:
			index = 1;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + vsw, 1.2f * MENU_SPACING + vsh);
			break;
			// Resolution select
		case 1:
			adjustResolution();
			break;
			// Fullscreen
		case 2:
			toggleFullscreen();
			break;
		}

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		// Screen Resolution
		if (index == 1 || index == 2) {
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 2.0f * MENU_SPACING + vsh);
		}
		else {
			m_forward = false;

			options();
			restorePrevMenuState();
		}

		return;

	default: break;
	}
}
void MenuScene::videoButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		videoKeyPressed(key, event);
}
void MenuScene::adjustResolution() {
	//switch (resolutionIndex) {
	//		// 1280x720
	//case 0: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 56;
	//	MENU_SPACING = 56;
	//	RES_ADJUST = 1;
	//	HP_BAR_ADJUST = 0;
	//	HP_ADJUST = 0;
	//	SP_ADJUST = 0;
	//	break;
	//}
	//		// 1366x768
	//case 1: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 60;
	//	MENU_SPACING = 60;
	//	RES_ADJUST = 1.08f;
	//	HP_BAR_ADJUST = -2;
	//	HP_ADJUST = 0;
	//	SP_ADJUST = 0;
	//	break;
	//}
	//		// 1600x900
	//case 2: {
	//	X_OFFSET = 560;
	//	Y_OFFSET = 170;
	//	SPACING_FACTOR = 37;
	//	MENU_SPACING = 37;
	//	RES_ADJUST = 1.23f;
	//	HP_BAR_ADJUST = -102;
	//	HP_ADJUST = -14;
	//	SP_ADJUST = -14;
	//	break;
	//}
	//		// 1920x1080
	//case 3: {
	//	X_OFFSET = 560;// 840;
	//	Y_OFFSET = 170;// 255;
	//	SPACING_FACTOR = 44;
	//	MENU_SPACING = 45;
	//	RES_ADJUST = 1.5;
	//	HP_BAR_ADJUST = -130;
	//	HP_ADJUST = -14;
	//	SP_ADJUST = -14;
	//	break;
	//}
	//default: break;
	//}

	auto director = Director::getInstance();
	auto glview = dynamic_cast<GLViewImpl*>(director->getOpenGLView());
	std::pair<int, int> res = resolutions[resolutionIndex];

	glview->setWindowed(res.first, res.second);
	if (fullScreen)
		glview->setFullscreen();
}
void MenuScene::toggleFullscreen() {
	auto director = Director::getInstance();
	auto glview = dynamic_cast<GLViewImpl*>(director->getOpenGLView());

	if (!fullScreen)
		glview->setFullscreen();
	else
		glview->setWindowed(resolutions[resolutionIndex].first, resolutions[resolutionIndex].second);

	fullScreen = !fullScreen;
}

void MenuScene::controlOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addLabel(0, 1.0f, fetchMenuText("Controls Menu", "Keyboard"), "Keyboard", 52);

	addLabel(0, -1.0f, fetchMenuText("Controls Menu", "Gamepad"), "Gamepad", 52);

	auto selector = createSelectorSprite(-3.5f, 1.0f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::controlOptionsKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::controlOptionsButtonPressed, this), selector);
}
void MenuScene::controlOptionsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Index key:
	// 
	// 0 : Sound
	// 1 : Video
	// 2 : Controls
	// 3 : Language

	int maxIndex = 1;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = maxIndex;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, -1.0f * MENU_SPACING + vsh);
		}

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxIndex) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 2.0f * MENU_SPACING);
		}
		else {
			playInterfaceSound("Select 1.mp3");
			index = 0;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + vsw, 1.0f * MENU_SPACING + vsh);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = true;

		switch (index) {
			// Keyboard
		case 0:
			keyBindings();
			break;
			// Gamepad
		case 1:
			controllerBindings();
			break;
		}
		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;
		options();

		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::controlOptionsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		controlOptionsKeyPressed(key, event);
}

void MenuScene::keyBindings() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addSprite(0, -0.25f, 2, "Pause_Menu_Border_Red.png", "Menu Border");
	sprites.find("Menu Border")->second->setScale(0.42f);
	sprites.find("Menu Border")->second->setOpacity(170);
	
	addLabel(0, 5.0, fetchMenuText("Controls Menu", "Keyboard Controls"), "Change Keys", 48);

	// Movement
	addLabel(-5.0f, 3.6f, fetchMenuText("Controls Menu", "Up"), "Up", 28);
	addLabel(0, 3.6f, convertKeycodeToStr(UP_KEY), "Up Key", 28);

	addLabel(-5.0f, 2.9f, fetchMenuText("Controls Menu", "Down"), "Down", 28);
	addLabel(0, 2.9f, convertKeycodeToStr(DOWN_KEY), "Down Key", 28);

	addLabel(-5.0f, 2.2f, fetchMenuText("Controls Menu", "Left"), "Left", 28);
	addLabel(0, 2.2f, convertKeycodeToStr(LEFT_KEY), "Left Key", 28);

	addLabel(-5.0f, 1.5f, fetchMenuText("Controls Menu", "Right"), "Right", 28);
	addLabel(0, 1.5f, convertKeycodeToStr(RIGHT_KEY), "Right Key", 28);


	// Interact
	addLabel(-5.0f, 0.8f, fetchMenuText("Controls Menu", "Interact"), "Interact", 28);
	addLabel(0, 0.8f, convertKeycodeToStr(INTERACT_KEY), "Interact Key", 28);


	// Use item
	addLabel(-5.0f, 0.1f, fetchMenuText("Controls Menu", "Quick Item Use"), "Quick", 28);
	addLabel(0, 0.1f, convertKeycodeToStr(QUICK_KEY), "Quick Key", 28);


	// Active Item
	addLabel(-5.0f, -0.6f, fetchMenuText("Controls Menu", "Active Item"), "Active", 28);
	addLabel(0, -0.6f, convertKeycodeToStr(ACTIVE_KEY), "Active Key", 28);


	// Swap weapon
	addLabel(-5.0f, -1.3f, fetchMenuText("Controls Menu", "Switch Weapon"), "Weapon", 28);
	addLabel(0, -1.3f, convertKeycodeToStr(WEAPON_KEY), "Weapon Key", 28);


	// Cast weapon
	addLabel(-5.0f, -2.0f, fetchMenuText("Controls Menu", "Cast Weapon"), "Cast", 28);
	addLabel(0, -2.0f, convertKeycodeToStr(CAST_KEY), "Cast Key", 28);


	// Open/close item menu
	addLabel(-5.0f, -2.7f, fetchMenuText("Controls Menu", "Item Menu"), "Item", 28);
	addLabel(0, -2.7f, convertKeycodeToStr(ITEM_KEY), "Item Key", 28);


	// View inventory
	addLabel(-5.0f, -3.4f, fetchMenuText("Controls Menu", "Inventory"), "Inventory", 28);
	addLabel(0, -3.4f, convertKeycodeToStr(INVENTORY_KEY), "Inventory Key", 28);


	addLabel(-5.0f, -4.1f, fetchMenuText("Controls Menu", "Pause"), "Pause", 28);
	addLabel(0, -4.1f, convertKeycodeToStr(PAUSE_KEY), "Pause Key", 28);


	// Reset to Defaults
	addLabel(0, -5.3f, fetchMenuText("Controls Menu", "Reset To Default"), "Default", 36);

	auto selector = createSelectorSprite(-3.0f, 3.6f);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::keyBindingsKeyPressed, this), selector);
}
void MenuScene::keyBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	if (settingKey) {
		setKey(keyCode, index);

		// If the key was set successfully, move the arrow back
		if (!settingKey)
			event->getCurrentTarget()->setPosition(pos.x - 1.2 * MENU_SPACING, pos.y);

		return;
	}

	int maxKeyIndex = 11;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index == 0) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, -5.3f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 0 && index <= maxKeyIndex) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = maxKeyIndex;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < maxKeyIndex) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 3.6f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}

		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:

		playInterfaceSound("Confirm 1.mp3");

		if (index == maxKeyIndex + 1) {
			resetKeyboardBindings();
			return;
		}

		if (!settingKey) {
			settingKey = true;
			event->getCurrentTarget()->setPosition(pos.x + 1.2 * MENU_SPACING, pos.y);
		}

		break;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		
		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		break;

	default:
		break;
	}
}
void MenuScene::setKey(KeyType keyCode, int index) {

	switch (index) {
		// Up
	case 0: {
		if (keyIsValid(keyCode, index)) {
			UP_KEY = keyCode;
			labels.find("Up Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Down
	case 1: {
		if (keyIsValid(keyCode, index)) {
			DOWN_KEY = keyCode;
			labels.find("Down Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Left
	case 2: {
		if (keyIsValid(keyCode, index)) {
			LEFT_KEY = keyCode;
			labels.find("Left Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Right
	case 3: {
		if (keyIsValid(keyCode, index)) {
			RIGHT_KEY = keyCode;
			labels.find("Right Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Interact
	case 4: {
		if (keyIsValid(keyCode, index)) {
			INTERACT_KEY = keyCode;
			labels.find("Interact Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Quick
	case 5: {
		if (keyIsValid(keyCode, index)) {
			QUICK_KEY = keyCode;
			labels.find("Quick Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Active
	case 6: {
		if (keyIsValid(keyCode, index)) {
			ACTIVE_KEY = keyCode;
			labels.find("Active Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Weapon
	case 7: {
		if (keyIsValid(keyCode, index)) {
			WEAPON_KEY = keyCode;
			labels.find("Weapon Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Cast
	case 8: {
		if (keyIsValid(keyCode, index)) {
			CAST_KEY = keyCode;
			labels.find("Cast Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Item Menu
	case 9: {
		if (keyIsValid(keyCode, index)) {
			ITEM_KEY = keyCode;
			labels.find("Item Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Inventory
	case 10: {
		if (keyIsValid(keyCode, index)) {
			INVENTORY_KEY = keyCode;
			labels.find("Inventory Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
		// Pause
	case 11: {
		if (keyIsValid(keyCode, index)) {
			PAUSE_KEY = keyCode;
			labels.find("Pause Key")->second->setString(convertKeycodeToStr(keyCode));
		}
		break;
	}
	default: break;
	}

	playInterfaceSound("Confirm 1.mp3");
	m_keyboardControls.clear();
}
bool MenuScene::keyIsValid(KeyType keyCode, int index) {
	m_keyboardControls.push_back(UP_KEY);
	m_keyboardControls.push_back(DOWN_KEY);
	m_keyboardControls.push_back(LEFT_KEY);
	m_keyboardControls.push_back(RIGHT_KEY);
	m_keyboardControls.push_back(INTERACT_KEY);
	m_keyboardControls.push_back(QUICK_KEY);
	m_keyboardControls.push_back(ACTIVE_KEY);
	m_keyboardControls.push_back(WEAPON_KEY);
	m_keyboardControls.push_back(CAST_KEY);
	m_keyboardControls.push_back(ITEM_KEY);
	m_keyboardControls.push_back(INVENTORY_KEY);
	m_keyboardControls.push_back(PAUSE_KEY);

	for (unsigned int i = 0; i < m_keyboardControls.size(); ++i) {
		if (i == index)
			continue;

		if (m_keyboardControls[i] == keyCode)
			return false;
	}

	// M, I, and Escape key are off limits
	if (keyCode == KeyType::KEY_ESCAPE ||
		keyCode == KeyType::KEY_M ||
		keyCode == KeyType::KEY_I)
		return false;

	settingKey = false;
	return true;
}
void MenuScene::resetKeyboardBindings() {
	UP_KEY = KeyType::KEY_UP_ARROW;
	DOWN_KEY = KeyType::KEY_DOWN_ARROW;
	LEFT_KEY = KeyType::KEY_LEFT_ARROW;
	RIGHT_KEY = KeyType::KEY_RIGHT_ARROW;
	INTERACT_KEY = KeyType::KEY_E;
	QUICK_KEY = KeyType::KEY_Q;
	ACTIVE_KEY = KeyType::KEY_SPACE;
	WEAPON_KEY = KeyType::KEY_W;
	CAST_KEY = KeyType::KEY_S;
	ITEM_KEY = KeyType::KEY_C;
	INVENTORY_KEY = KeyType::KEY_TAB;
	PAUSE_KEY = KeyType::KEY_P;

	labels.find("Up Key")->second->setString(convertKeycodeToStr(UP_KEY));
	labels.find("Down Key")->second->setString(convertKeycodeToStr(DOWN_KEY));
	labels.find("Left Key")->second->setString(convertKeycodeToStr(LEFT_KEY));
	labels.find("Right Key")->second->setString(convertKeycodeToStr(RIGHT_KEY));
	labels.find("Interact Key")->second->setString(convertKeycodeToStr(INTERACT_KEY));
	labels.find("Quick Key")->second->setString(convertKeycodeToStr(QUICK_KEY));
	labels.find("Active Key")->second->setString(convertKeycodeToStr(ACTIVE_KEY));
	labels.find("Weapon Key")->second->setString(convertKeycodeToStr(WEAPON_KEY));
	labels.find("Cast Key")->second->setString(convertKeycodeToStr(CAST_KEY));
	labels.find("Item Key")->second->setString(convertKeycodeToStr(ITEM_KEY));
	labels.find("Inventory Key")->second->setString(convertKeycodeToStr(INVENTORY_KEY));
	labels.find("Pause Key")->second->setString(convertKeycodeToStr(PAUSE_KEY));
}

void MenuScene::controllerBindings() {
	setPrevMenuState();

	index = 0;
	removeAll();

	addSprite(0, -0.25f, 2, "Pause_Menu_Border_Red.png", "Menu Border");
	sprites.find("Menu Border")->second->setScale(0.42f);
	sprites.find("Menu Border")->second->setOpacity(170);

	addLabel(0, 5.0, fetchMenuText("Controls Menu", "Gamepad Controls"), "Change Keys", 48);

	// Movement
	addLabel(-5.0f, 3.6f, fetchMenuText("Controls Menu", "Up"), "Up", 28);
	addLabel(0, 3.6f, convertButtonToStr(UP_BUTTON), "Up Key", 28);

	addLabel(-5.0f, 2.9f, fetchMenuText("Controls Menu", "Down"), "Down", 28);
	addLabel(0, 2.9f, convertButtonToStr(DOWN_BUTTON), "Down Key", 28);

	addLabel(-5.0f, 2.2f, fetchMenuText("Controls Menu", "Left"), "Left", 28);
	addLabel(0, 2.2f, convertButtonToStr(LEFT_BUTTON), "Left Key", 28);

	addLabel(-5.0f, 1.5f, fetchMenuText("Controls Menu", "Right"), "Right", 28);
	addLabel(0, 1.5f, convertButtonToStr(RIGHT_BUTTON), "Right Key", 28);


	// Interact
	addLabel(-5.0f, 0.8f, fetchMenuText("Controls Menu", "Interact"), "Interact", 28);
	addLabel(0, 0.8f, convertButtonToStr(INTERACT_BUTTON), "Interact Key", 28);


	// Use item
	addLabel(-5.0f, 0.1f, fetchMenuText("Controls Menu", "Quick Item Use"), "Quick", 28);
	addLabel(0, 0.1f, convertButtonToStr(QUICK_BUTTON), "Quick Key", 28);


	// Active Item
	addLabel(-5.0f, -0.6f, fetchMenuText("Controls Menu", "Active Item"), "Active", 28);
	addLabel(0, -0.6f, convertButtonToStr(ACTIVE_BUTTON), "Active Key", 28);


	// Swap weapon
	addLabel(-5.0f, -1.3f, fetchMenuText("Controls Menu", "Switch Weapon"), "Weapon", 28);
	addLabel(0, -1.3f, convertButtonToStr(WEAPON_BUTTON), "Weapon Key", 28);


	// Cast weapon
	addLabel(-5.0f, -2.0f, fetchMenuText("Controls Menu", "Cast Weapon"), "Cast", 28);
	addLabel(0, -2.0f, convertButtonToStr(CAST_BUTTON), "Cast Key", 28);


	// Open/close item menu
	addLabel(-5.0f, -2.7f, fetchMenuText("Controls Menu", "Item Menu"), "Item", 28);
	addLabel(0, -2.7f, convertButtonToStr(ITEM_BUTTON), "Item Key", 28);


	// View inventory
	addLabel(-5.0f, -3.4f, fetchMenuText("Controls Menu", "Inventory"), "Inventory", 28);
	addLabel(0, -3.4f, convertButtonToStr(INVENTORY_BUTTON), "Inventory Key", 28);


	addLabel(-5.0f, -4.1f, fetchMenuText("Controls Menu", "Pause"), "Pause", 28);
	addLabel(0, -4.1f, convertButtonToStr(PAUSE_BUTTON), "Pause Key", 28);


	// Reset to Defaults
	addLabel(0, -5.3f, fetchMenuText("Controls Menu", "Reset To Default"), "Default", 36);

	auto selector = createSelectorSprite(-3.0f, 3.6f);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::controllerBindingsButtonPressed, this), selector);
	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::controllerBindingsKeyPressed, this), selector);
}
void MenuScene::controllerBindingsButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	if (settingKey) {
		setButton(static_cast<ButtonType>(keyCode), index);

		// If the key was set successfully, move the arrow back
		if (!settingKey)
			event->getCurrentTarget()->setPosition(pos.x - 1.2 * MENU_SPACING, pos.y);

		return;
	}

	int maxKeyIndex = 11;

	switch (keyCode) {
	case ButtonType::BUTTON_DPAD_UP: {
		if (index == 0) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, -5.3f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		if (index > 0 && index <= maxKeyIndex) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = maxKeyIndex;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		
		break;
	}
	case ButtonType::BUTTON_DPAD_DOWN: {
		if (index < maxKeyIndex) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex) {
			index = maxKeyIndex + 1;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.2f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == maxKeyIndex + 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 3.6f * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}

		break;
	}
	case ButtonType::BUTTON_A:

		playInterfaceSound("Confirm 1.mp3");

		if (index == maxKeyIndex + 1) {
			resetKeyboardBindings();
			return;
		}

		if (!settingKey) {
			settingKey = true;
			event->getCurrentTarget()->setPosition(pos.x + 1.2 * MENU_SPACING, pos.y);
		}

		break;

	case ButtonType::BUTTON_B:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		break;

	default:
		break;
	}
}
void MenuScene::controllerBindingsKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		controlOptions();
		restorePrevMenuState();
		return;
	}
}
void MenuScene::setButton(ButtonType keyCode, int index) {
	switch (index) {
		// Up
	case 0: {
		if (buttonIsValid(keyCode, index)) {
			UP_BUTTON = keyCode;
			labels.find("Up Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Down
	case 1: {
		if (buttonIsValid(keyCode, index)) {
			DOWN_BUTTON = keyCode;
			labels.find("Down Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Left
	case 2: {
		if (buttonIsValid(keyCode, index)) {
			LEFT_BUTTON = keyCode;
			labels.find("Left Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Right
	case 3: {
		if (buttonIsValid(keyCode, index)) {
			RIGHT_BUTTON = keyCode;
			labels.find("Right Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Interact
	case 4: {
		if (buttonIsValid(keyCode, index)) {
			INTERACT_BUTTON = keyCode;
			labels.find("Interact Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Quick
	case 5: {
		if (buttonIsValid(keyCode, index)) {
			QUICK_BUTTON = keyCode;
			labels.find("Quick Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Active
	case 6: {
		if (buttonIsValid(keyCode, index)) {
			ACTIVE_BUTTON = keyCode;
			labels.find("Active Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Weapon
	case 7: {
		if (buttonIsValid(keyCode, index)) {
			WEAPON_BUTTON = keyCode;
			labels.find("Weapon Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Cast
	case 8: {
		if (buttonIsValid(keyCode, index)) {
			CAST_BUTTON = keyCode;
			labels.find("Cast Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Item Menu
	case 9: {
		if (buttonIsValid(keyCode, index)) {
			ITEM_BUTTON = keyCode;
			labels.find("Item Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Inventory
	case 10: {
		if (buttonIsValid(keyCode, index)) {
			INVENTORY_BUTTON = keyCode;
			labels.find("Inventory Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
			// Pause
	case 11: {
		if (buttonIsValid(keyCode, index)) {
			PAUSE_BUTTON = keyCode;
			labels.find("Pause Key")->second->setString(convertButtonToStr(keyCode));
		}
		break;
	}
	default: break;
	}

	playInterfaceSound("Confirm 1.mp3");
	m_gamepadControls.clear();
}
bool MenuScene::buttonIsValid(ButtonType keyCode, int index) {
	m_gamepadControls.push_back(UP_BUTTON);
	m_gamepadControls.push_back(DOWN_BUTTON);
	m_gamepadControls.push_back(LEFT_BUTTON);
	m_gamepadControls.push_back(RIGHT_BUTTON);
	m_gamepadControls.push_back(INTERACT_BUTTON);
	m_gamepadControls.push_back(QUICK_BUTTON);
	m_gamepadControls.push_back(ACTIVE_BUTTON);
	m_gamepadControls.push_back(WEAPON_BUTTON);
	m_gamepadControls.push_back(CAST_BUTTON);
	m_gamepadControls.push_back(ITEM_BUTTON);
	m_gamepadControls.push_back(INVENTORY_BUTTON);
	m_gamepadControls.push_back(PAUSE_BUTTON);

	for (unsigned int i = 0; i < m_gamepadControls.size(); ++i) {
		if (i == index)
			continue;

		if (m_gamepadControls[i] == keyCode)
			return false;
	}

	settingKey = false;
	return true;
}
void MenuScene::resetControllerBindings() {
	UP_BUTTON = ButtonType::BUTTON_DPAD_UP;
	DOWN_BUTTON = ButtonType::BUTTON_DPAD_DOWN;
	LEFT_BUTTON = ButtonType::BUTTON_DPAD_LEFT;
	RIGHT_BUTTON = ButtonType::BUTTON_DPAD_RIGHT;
	INTERACT_BUTTON = ButtonType::BUTTON_X;
	QUICK_BUTTON = ButtonType::BUTTON_Y;
	ACTIVE_BUTTON = ButtonType::BUTTON_A;
	WEAPON_BUTTON = ButtonType::BUTTON_RIGHT_SHOULDER;
	CAST_BUTTON = ButtonType::BUTTON_B;
	ITEM_BUTTON = ButtonType::BUTTON_SELECT;
	INVENTORY_BUTTON = ButtonType::BUTTON_LEFT_SHOULDER;
	PAUSE_BUTTON = ButtonType::BUTTON_START;

	labels.find("Up Key")->second->setString(convertButtonToStr(UP_BUTTON));
	labels.find("Down Key")->second->setString(convertButtonToStr(DOWN_BUTTON));
	labels.find("Left Key")->second->setString(convertButtonToStr(LEFT_BUTTON));
	labels.find("Right Key")->second->setString(convertButtonToStr(RIGHT_BUTTON));
	labels.find("Interact Key")->second->setString(convertButtonToStr(INTERACT_BUTTON));
	labels.find("Quick Key")->second->setString(convertButtonToStr(QUICK_BUTTON));
	labels.find("Active Key")->second->setString(convertButtonToStr(ACTIVE_BUTTON));
	labels.find("Weapon Key")->second->setString(convertButtonToStr(WEAPON_BUTTON));
	labels.find("Cast Key")->second->setString(convertButtonToStr(CAST_BUTTON));
	labels.find("Item Key")->second->setString(convertButtonToStr(ITEM_BUTTON));
	labels.find("Inventory Key")->second->setString(convertButtonToStr(INVENTORY_BUTTON));
	labels.find("Pause Key")->second->setString(convertButtonToStr(PAUSE_BUTTON));
}

void MenuScene::languageOptions() {
	setPrevMenuState();

	index = 0;
	removeAll();

	auto selector = createSelectorSprite(-3.5f, 3.0f);

	addLabel(0.0f, 3.0f, fetchMenuText("Language Menu", "English"), "English", 40);

	createKeyboardEventListener(CC_CALLBACK_2(MenuScene::languageKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(MenuScene::languageButtonPressed, this), selector);
}
void MenuScene::languageKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		/*if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}*/

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		/*if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}*/
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
		case 0: GAME_TEXT_FILE = EN_US_FILE; break;
		default: GAME_TEXT_FILE = EN_US_FILE; break;
		}

		TextUtils::initJsonText();

		return;

	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		m_forward = false;

		options();
		restorePrevMenuState();

		return;

	default: break;
	}
}
void MenuScene::languageButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = menuButtonToKey(static_cast<ButtonType>(keyCode));
	if (key != KeyType::KEY_END)
		languageKeyPressed(key, event);
}

void MenuScene::setPrevMenuState() {
	if (m_forward) {
		auto it = sprites.find("Selector");
		std::pair<int, cocos2d::Vec2> prevEls = std::make_pair(index, Vec2(it->second->getPositionX(), it->second->getPositionY()));
		prevMenuElements.push(prevEls);
	}
}
void MenuScene::restorePrevMenuState() {
	std::pair<int, cocos2d::Vec2> prevEls = prevMenuElements.top();
	prevMenuElements.pop();

	index = prevEls.first;
	sprites.find("Selector")->second->setPosition(prevEls.second);
}

void MenuScene::addLabel(float x, float y, std::string name, std::string id, float fontSize) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto label = Label::createWithTTF(name, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	label->setOpacity(230);
	label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	label->setAdditionalKerning(0.25f);
	this->addChild(label, 4);
	labels.insert(std::make_pair(id, label));
}
void MenuScene::updateLabel(std::string id, std::string newText) {
	labels.find(id)->second->setString(newText);
}
void MenuScene::addSprite(float x, float y, int z, const std::string &image, const std::string &id) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto sprite = Sprite::create(image);
	sprite->setPosition(x, y);
	//sprite->setScale(0.8f);
	this->addChild(sprite, z);
	sprites.insert(std::make_pair(id, sprite));
}
cocos2d::Sprite* MenuScene::createSelectorSprite(float x, float y) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	x = x * MENU_SPACING + vsw;
	y = y * MENU_SPACING + vsh;

	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(x, y);
	selector->setScale(3.0f);
	this->addChild(selector, 4);
	sprites.insert(std::make_pair("Selector", selector));

	return selector;
}

void MenuScene::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, node);
}
void MenuScene::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}

void MenuScene::removeAll() {
	removeLabels();
	removeSprites();
};
void MenuScene::removeLabels() {
	for (auto &it : labels)
		it.second->removeFromParent();
	
	labels.clear();
};
void MenuScene::removeSprites() {
	for (auto &it : sprites)
		it.second->removeFromParent();
	
	sprites.clear();
};


//		START SCENE
Scene* StartScene::createScene()
{
	TextUtils::initJsonText();
	registerGamepads();

	return StartScene::create();
}

bool StartScene::init() {
	if (!Scene::init())
		return false;
	
	// if music hasn't started already, play it
	if (id == -1)
		id = playMusic("PetterTheSturgeon - Anything_1.waw_.mp3", true);
	

	// Fill resolution vector
	/*resolutions.push_back(std::make_pair(1280, 720));
	resolutions.push_back(std::make_pair(1366, 768));
	resolutions.push_back(std::make_pair(1600, 900));
	resolutions.push_back(std::make_pair(1920, 1080));*/

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// Title
	auto title1 = Label::createWithTTF("Lost Arts", TEXT_FONT, 72);
	title1->setPosition(Vec2(vsw, 2.5 * MENU_SPACING + vsh));
	title1->setTextColor(cocos2d::Color4B(224, 224, 224, 255));
	title1->enableOutline(cocos2d::Color4B(50, 55, 55, 255), 1);
	this->addChild(title1, 1);
	labels.insert(std::make_pair("title", title1));

	// background pic
	auto background = Sprite::create("super_pixel_cave_wallpaper_C.png");
	background->setPosition(Vec2(vsw, vsh));
	float scale = 1.1f * RES_ADJUST;
	background->setScale(scale);
	this->addChild(background, -5);

	// Start
	/*auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
	float x = vsw;
	float y = -0.25f * MENU_SPACING + vsh;
	start->setPosition(Vec2(x, y));
	start->setScale(0.8f);
	this->addChild(start, 3);
	sprites.insert(std::make_pair("start", start));*/

	addLabel(0, -0.25f, fetchMenuText("Start Menu", "Start"), "Start", 52);

	// Options
	addLabel(0, -1.75f, fetchMenuText("Start Menu", "Options"), "Options", 52);

	// Exit
	addLabel(0, -3.25f, fetchMenuText("Start Menu", "Exit"), "Exit", 52);
	/*auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButton1.png");
	x = visibleSize.width / 2;
	y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
	exitGame->setPosition(Vec2(x, y));
	this->addChild(exitGame, 3);
	sprites.insert(std::make_pair("exitGame", exitGame));*/

	auto selector = createSelectorSprite(-2.5f, -0.25f);
	createKeyboardEventListener(CC_CALLBACK_2(StartScene::keyPressed, this), selector);

	/*kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::keyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, selector);*/


	/*

	// MENU
	auto beginGame = MenuItemImage::create(
		"StartButton1.png",
		"StartButtonPressed1.png",
		CC_CALLBACK_1(StartScene::startGameCallback, this));
	

	if (beginGame == nullptr ||
		beginGame->getContentSize().width <= 0 ||
		beginGame->getContentSize().height <= 0)
	{
		problemLoading("'StartButton.png' and 'StartButtonPressed.png'");
	}
	else
	{
		float x = visibleSize.width / 2;
		float y = visibleSize.height / 2 - (1.2 * MENU_SPACING);
		beginGame->setPosition(Vec2(x, y));
	}


	auto exitGame = MenuItemImage::create(
		"ExitButton1.png",
		"ExitButtonPressed1.png",
		CC_CALLBACK_1(StartScene::exitGameCallback, this));

	if (exitGame == nullptr ||
		exitGame->getContentSize().width <= 0 ||
		exitGame->getContentSize().height <= 0)
	{
		problemLoading("'ExitButton1.png' and 'ExitButtonPressed1.png'");
	}
	else
	{
		float x = visibleSize.width / 2;
		float y = visibleSize.height / 2 - 3 * MENU_SPACING;
		exitGame->setPosition(Vec2(x, y));
	}

	auto startMenu = Menu::create(beginGame, exitGame, NULL);
	startMenu->setPosition(Vec2::ZERO);
	this->addChild(startMenu);
	*/

	return true;
}
void StartScene::keyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 2) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
		case 0: { // Start
			playInterfaceSound("Confirm 1.mp3");

			characterSelect();

			return;
		}
				// Options
		case 1:
			playInterfaceSound("Confirm 1.mp3");
			
			m_forward = true;
			options();

			return;
		case 2: // Exit
			playInterfaceSound("Confirm 1.mp3");

			/*auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButtonPressed1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
			exitGame->setPosition(Vec2(x, y));
			this->addChild(exitGame, 3);*/

			exitGameCallback(this);

			return;
		}
	default: break;
	}
}

void StartScene::characterSelect() {
	index = 0;
	removeAll(); // remove previous labels and sprites

	setCharacterSelectPositions(); // Initialize degrees

	float x, y;

	addLabel(0, 4, fetchMenuText("Character Select Menu", "Select Character"), "Select Character", 54);

	// Adventurer
	polarToCartesian(degree1, x, y);
	addCharacterSprite(x, y, "Player1_48x48.png", "p1");
	sprites.find("p1")->second->setScale(2.5f);

	// Spellcaster
	polarToCartesian(degree2, x, y);
	addCharacterSprite(x, y, "Spellcaster_48x48.png", "p2");

	// Acrobat
	polarToCartesian(degree3, x, y);
	addCharacterSprite(x, y, "OutsideMan2.png", "p3");

	// Spelunker
	polarToCartesian(degree4, x, y);
	addCharacterSprite(x, y, "Spelunker_48x48.png", "p4");

	// Madman
	polarToCartesian(degree5, x, y);
	addCharacterSprite(x, y, "Madman_48x48.png", "p5");

	displayCharacterInfo();

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::characterKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, sprites.find("p1")->second); // check this for player
}
void StartScene::characterKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index > 0)
			index--;
		else
			index = 4;

		rotate(false);

		playInterfaceSound("Select 1.mp3");
		
		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index < 4)
			index++;
		else
			index = 0;
	
		rotate(true);

		playInterfaceSound("Select 1.mp3");
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
		case 0:	characterID = ID_ADVENTURER; break;
		case 1: characterID = ID_SPELLCASTER; break;
		case 2: characterID = ID_ACROBAT; break;
		case 3: characterID = ID_SPELUNKER; break;
		case 4: characterID = ID_MADMAN; break;
		}

		startGameCallback(this);

		index = 0;
		return;
		
	case KeyType::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		index = 0;

		removeAll();
		init();

		return;

	default: break;
	}
}

float StartScene::degreeToRadian(int degree) {
	static const double pi = 4 * std::atan(1);
	return degree * pi / 180.0;
}
void StartScene::polarToCartesian(int degree, float &x, float &y) {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	float a, r, b, theta;
	r = 90;
	a = 2.25f;
	b = 0.85f;

	theta = degreeToRadian(degree);
	x = r * a * cos(theta) + vsw;
	y = r * b * sin(theta) + vsh;
}
void StartScene::rotate(bool clockwise) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	int scalingFactor = 1;

	if (clockwise)
		scalingFactor = -1;

	float x, y;

	cocos2d::Vector<FiniteTimeAction*> vec1, vec2, vec3, vec4, vec5;

	for (int i = 0; i < 72; i++) {
		polarToCartesian(degree1, x, y);
		vec1.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree1 += scalingFactor);

		polarToCartesian(degree2, x, y);
		vec2.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree2 += scalingFactor);

		polarToCartesian(degree3, x, y);
		vec3.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree3 += scalingFactor);

		polarToCartesian(degree4, x, y);
		vec4.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree4 += scalingFactor);

		polarToCartesian(degree5, x, y);
		vec5.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree5 += scalingFactor);
	}

	const float largeScaling = 2.5f;
	const float smallScaling = 1.0f;

	cocos2d::Sequence* seq1 = cocos2d::Sequence::create(vec1);
	cocos2d::Sequence* seq2 = cocos2d::Sequence::create(vec2);
	cocos2d::Sequence* seq3 = cocos2d::Sequence::create(vec3);
	cocos2d::Sequence* seq4 = cocos2d::Sequence::create(vec4);
	cocos2d::Sequence* seq5 = cocos2d::Sequence::create(vec5);

	sprites.find("p1")->second->runAction(seq1);
	sprites.find("p1")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 0 ? largeScaling : smallScaling));
	sprites.find("p2")->second->runAction(seq2);
	sprites.find("p2")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 1 ? largeScaling : smallScaling));

	sprites.find("p3")->second->runAction(seq3);
	sprites.find("p3")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 2 ? largeScaling : smallScaling));
	sprites.find("p4")->second->runAction(seq4);
	sprites.find("p4")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 3 ? largeScaling : smallScaling));

	sprites.find("p5")->second->runAction(seq5);
	sprites.find("p5")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 4 ? largeScaling : smallScaling));

	displayCharacterInfo();
}
void StartScene::displayCharacterInfo() {
	if (labels.find("Character Name") == labels.end()) {
		addLabel(0, -3.0f, fetchMenuText("Character Select Menu", "Adventurer"), "Character Name", 32);
		addLabel(0, -3.8f, fetchMenuText("Character Select Menu", "Adventurer Desc"), "Character Desc", 24);
		return;
	}

	switch (index) {
	case 0: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Adventurer"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Adventurer Desc"));
		break;
	}
	case 1: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Spellcaster"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Spellcaster Desc"));
		break;
	}
	case 2: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Acrobat"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Acrobat Desc"));
		break;
	}
	case 3: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Spelunker"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Spelunker Desc"));
		break;
	}
	case 4: {
		updateLabel("Character Name", fetchMenuText("Character Select Menu", "Madman"));
		updateLabel("Character Desc", fetchMenuText("Character Select Menu", "Madman Desc"));
		break;
	}
	}
}
void StartScene::setAngle(int &angle) {
	while (angle < 0)
		angle += 360;

	angle = angle % 360;
}
void StartScene::setCharacterSelectPositions() {
	// If 4 characters
	/*degree1 = 270;
	degree2 = 0;
	degree3 = 90;
	degree4 = 180;*/

	// If 5 characters
	degree1 = 270;
	degree2 = 342;
	degree3 = 54;
	degree4 = 126;
	degree5 = 198;
}
void StartScene::addCharacterSprite(float x, float y, std::string image, std::string id) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto s = Sprite::createWithSpriteFrameName(image);
	s->setPosition(x, y);
	s->setScale(1.0f);
	this->addChild(s, 1);
	sprites.insert(std::make_pair(id, s));
}

void StartScene::startGameCallback(Ref* pSender) {
	playInterfaceSound("Confirm 1.mp3");
	cocos2d::experimental::AudioEngine::stop(id);

	GameTable::initializeTables();

	std::shared_ptr<Player> p = nullptr;
	switch (characterID) {
	case ID_ADVENTURER: p = std::make_shared<Adventurer>(); break;
	case ID_SPELLCASTER: p = std::make_shared<Spellcaster>(); break;
	case ID_MADMAN: p = std::make_shared<TheMadman>(); break;
	case ID_SPELUNKER: p = std::make_shared<Spelunker>(); break;
	case ID_ACROBAT: p = std::make_shared<Acrobat>(); break;
	default: p = std::make_shared<Adventurer>(); break;
	}
	
	auto levelScene = LevelScene::createScene(p, TUTORIAL);

	auto transition = TransitionFade::create(0.75f, levelScene);

	transition->setOnExitCallback([levelScene]() {
		auto scene = dynamic_cast<LevelScene*>(levelScene->getChildByName("Level Scene"));
		scene->scheduleTimer();
	});

	Director::getInstance()->replaceScene(transition);
}
void StartScene::exitGameCallback(Ref* pSender)
{
	playInterfaceSound("Cancel 1.mp3");

	//Close the cocos2d-x game scene and quit the application
	Director::getInstance()->end();

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

	//EventCustom customEndEvent("game_scene_close_event");
	//_eventDispatcher->dispatchEvent(&customEndEvent);
}


// HUD LAYER
HUDLayer::HUDLayer(std::shared_ptr<Player> p) : m_player(p) {
	
}
HUDLayer::~HUDLayer() {
	if (qKey)
		qKey->removeFromParent();

	for (auto &it : HUD)
		it.second->removeFromParent();

	for (auto &it : m_persistentLabels)
		it.second->removeFromParent();

	for (auto &it : labels)
		it.second->removeFromParent();

	for (auto &it : keyLabels)
		it.second->removeFromParent();

	for (auto &it : itemMenuSprites)
		it.second->removeFromParent();

	for (auto &it : inventoryMenuSprites)
		it.second->removeFromParent();

	for (auto &it : generalSprites)
		it.second->removeFromParent();

	for (auto &it : menuSprites)
		it.second->removeFromParent();

	if (m_player)
		m_player.reset();
}

HUDLayer* HUDLayer::create(std::shared_ptr<Player> p) {
	HUDLayer *pRet = new(std::nothrow) HUDLayer(p);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool HUDLayer::init() {
	if (!Layer::init())
		return false;

	std::string image;

	image = "Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, Z_HUD_ELEMENT, "healthbar");
	healthbar->setPosition(-680.f * RES_ADJUST + HP_BAR_ADJUST, 480.f * RES_ADJUST);
	healthbar->setScale(1.5f);
	HUD.insert(std::pair<std::string, Sprite*>("healthbar", healthbar));

	image = "Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, Z_HUD_ELEMENT, "hp");
	hp->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	hp->setPosition(-844.f * RES_ADJUST + HP_ADJUST, 480.f * RES_ADJUST);
	hp->setScale(1.5f);
	HUD.insert(std::pair<std::string, Sprite*>("hp", hp));

	createPersistentLabel(-680.f * RES_ADJUST, 482.f * RES_ADJUST, std::to_string(m_player->getHP()) + "/" + std::to_string(m_player->getMaxHP()), "HP", 22);

	createPersistentLabel(-870 * RES_ADJUST, 70 * RES_ADJUST, "Str: +" + std::to_string(m_player->getStr()), "Strength", 18);
	createPersistentLabel(-870 * RES_ADJUST, 50 * RES_ADJUST, "Dex: +" + std::to_string(m_player->getDex() + m_player->getWeapon()->getDexBonus()), "Dexterity", 18);
	createPersistentLabel(-870 * RES_ADJUST, 30 * RES_ADJUST, "Int: +" + std::to_string(m_player->getInt()), "Intellect", 18);
	createPersistentLabel(-870 * RES_ADJUST, 10 * RES_ADJUST, "Amr: +" + std::to_string(m_player->getArmor()), "Armor", 18);

	constructWeaponHUD();


	//	:::: RIGHT SIDE OF HUD ::::

	// Currency
	image = "Gold_Pile1_48x48.png";
	Sprite* goldpile = Sprite::createWithSpriteFrameName(image);
	this->addChild(goldpile, Z_HUD_SPRITE, "goldpile");
	goldpile->setPosition(855 * RES_ADJUST, 480 * RES_ADJUST);
	goldpile->setScale(1.0f * RES_ADJUST);
	goldpile->setOpacity(230);
	HUD.insert(std::pair<std::string, Sprite*>("goldpile", goldpile));

	createPersistentLabel(895 * RES_ADJUST, 480 * RES_ADJUST, "0", "Money Count", 22);

	createPersistentLabel(870.f * RES_ADJUST, -480 * RES_ADJUST, "Money Bonus : " + std::to_string(static_cast<int>(m_player->getMoneyBonus())), "Money Bonus", 20);
	m_persistentLabels.find("Money Bonus")->second->setTextColor(cocos2d::Color4B(153, 153, 255, 200));

	return true;
}

void HUDLayer::updateHUD() {
	checkPlayerStats();
	checkWeaponHUD();
	checkActiveItemHUD();
	checkRelicHUD();
	checkItemHUD();
}

void HUDLayer::showBossHP() {
	std::string image;

	//		RENDER BOSS HP
	image = "Boss_Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, Z_HUD_ELEMENT, "bosshealthbar");
	healthbar->setPosition(430, 0);
	HUD.insert(std::pair<std::string, Sprite*>("bosshealthbar", healthbar));

	image = "Boss_Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, Z_HUD_ELEMENT, "bosshp");
	hp->setAnchorPoint(Vec2(0.5, 0.0)); // set anchor point to left side
	hp->setPosition(430, -134);
	HUD.insert(std::pair<std::string, Sprite*>("bosshp", hp));
}
void HUDLayer::updateBossHUD() {
	// if there are still monsters, check for smasher
	if (m_scene->getCurrentDungeon()->monsterCount() > 0) {
		if (m_scene->getCurrentDungeon()->monsterAt(0)->getName() == "Smasher") {
			//	Check Boss HP bar
			float y_scale = m_scene->getCurrentDungeon()->monsterAt(0)->getHP() / (static_cast<float>(m_scene->getCurrentDungeon()->monsterAt(0)->getMaxHP()) * 1.0);
			cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, 1.0f, y_scale);
			HUD.find("bosshp")->second->runAction(move);
		}
		else
			deconstructBossHUD();		
	}
	// else if there aren't any monsters, deconstruct the hp bar if we haven't already
	else {
		deconstructBossHUD();
	}
}

void HUDLayer::showHUD() {
	for (auto &it : HUD)
		it.second->setVisible(true);

	for (auto &it : keyLabels)
		it.second->setVisible(true);

	for (auto &it : m_persistentLabels)
		it.second->setVisible(true);
}
void HUDLayer::hideHUD() {
	for (auto &it : HUD)
		it.second->setVisible(false);

	for (auto &it : keyLabels)
		it.second->setVisible(false);

	for (auto &it : m_persistentLabels)
		it.second->setVisible(false);
}

void HUDLayer::NPCInteraction(const NPC &npc) {
	// assign dialogue to use
	m_dialogue = npc.getDialogue();

	if (npc.getName() == MEMORIZER)
		hideHUD();

	m_line = Label::createWithTTF("", TEXT_FONT, 20);
	m_line->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	m_line->setOpacity(230);
	m_line->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_line->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_line->setAdditionalKerning(0.25f);
	this->addChild(m_line, Z_HUD_LABEL);

	// display first line
	m_line->setString(m_dialogue[m_lineIndex]);
	m_lineIndex++;

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::NPCKeyPressed, this, &npc), m_line);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::NPCButtonPressed, this, &npc), m_line);
}
void HUDLayer::NPCKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc) {
	// If there's nothing left to say, remove text and restore control
	if (m_lineIndex >= static_cast<int>(m_dialogue.size())) {
		playInterfaceSound("Confirm 1.mp3");

		if (npc->getName() == MEMORIZER)
			showHUD();

		m_lineIndex = 0;
		m_line->removeFromParent();
		m_line = nullptr;
		enableGameplayListeners();
		return;
	}

	switch (keyCode) {
	case KeyType::KEY_SPACE:
	default: {
		if (!m_dialogue.empty() && m_lineIndex < static_cast<int>(m_dialogue.size())) {
			playInterfaceSound("Confirm 1.mp3");

			// If this line indicates a prompt, then begin the prompt
			if (m_dialogue[m_lineIndex] == NPC_PROMPT) {
				m_lineIndex = 0;
				m_line->removeFromParent();
				m_line = nullptr;

				NPCPrompt(*npc);

				return;
			}

			m_line->setString(m_dialogue[m_lineIndex]);
			m_lineIndex++;
		}
	}
	}
}
void HUDLayer::NPCButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc) {
	NPCKeyPressed(KeyType::KEY_ENTER, event, npc);
}

void HUDLayer::NPCPrompt(const NPC &npc) {
	// Assign this so that we know how many choices there are
	m_dialogue = npc.getChoices();

	// menu border
	/*Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);
	menuSprites.insert(std::pair<std::string, Sprite*>("box", box));*/

	// Arrow sprite for selection
	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(-3 * MENU_SPACING, -2.5f * MENU_SPACING);
	this->addChild(selector, Z_HUD_SPRITE);
	selector->setScale(2.5f);
	generalSprites.insert(std::pair<std::string, Sprite*>("Selector", selector));

	for (int i = 0; i < static_cast<int>(m_dialogue.size()); ++i)
		addLabel(0, (-3 - (static_cast<float>(i) - 0.5)) * MENU_SPACING, m_dialogue[i], m_dialogue[i], 20);
	
	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::NPCPromptKeyPressed, this, &npc), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::NPCPromptButtonPressed, this, &npc), selector);
}
void HUDLayer::NPCPromptKeyPressed(KeyType keyCode, cocos2d::Event* event, const NPC *npc) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < static_cast<int>(m_dialogue.size() - 1)) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		const_cast<NPC*>(npc)->useResponse(index);

		index = 0;
		deconstructMenu(generalSprites);

		if (npc->getName() == MEMORIZER)
			hideHUD();
		
		NPCInteraction(*npc);

		return;

	default:
		break;
	}

}
void HUDLayer::NPCPromptButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, const NPC *npc) {
	NPCPromptKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event, npc);
}

void HUDLayer::devilsWaters() {
	constructSelectionMenu();

	// Pause option
	addLabel(0, 4.8f * MENU_SPACING, fetchPromptText("Fountain", "prompt"), "pause", 40);

	// NO
	addLabel(0, 2 * MENU_SPACING, fetchPromptText("Fountain", "No"), "no", 36);

	// YES
	addLabel(0, -2 * MENU_SPACING, fetchPromptText("Fountain", "Yes"), "yes", 36);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::devilKeyPressed, this), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::devilButtonPressed, this), menuSprites.find("Selector")->second);
}
void HUDLayer::devilKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// NO
		case 0: {
			playInterfaceSound("Confirm 1.mp3");

			index = 0;
			deconstructMenu(menuSprites);

			SecondFloor* second = dynamic_cast<SecondFloor*>(m_scene->getCurrentDungeon());
			second->devilsWater(false);

			enableGameplayListeners();
			return;
		}
			// YES
		case 1: 
			playSound("Devils_Gift.mp3");

			SecondFloor* second = dynamic_cast<SecondFloor*>(m_scene->getCurrentDungeon());
			second->devilsWater(true);

			index = 0;
			deconstructMenu(menuSprites);

			enableGameplayListeners();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::devilButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	devilKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::shrineChoice(const std::string &shrine) {
	constructSelectionMenu();

	// Number of options to choose from
	int choices = 0;

	if (shrine == HEALTH_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "health prompt"), "Life", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Wealth"), "Sacrifice Wealth", 29);
		addLabel(0, 1 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Item"), "Sacrifice Item", 29);
		addLabel(0, 0 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Weapon"), "Sacrifice Weapon", 29);
		addLabel(0, -1 * MENU_SPACING, fetchPromptText("Shrine", "Sacrifice Soul"), "Sacrifice Soul", 29);

		choices = 4;
	}
	else if (shrine == RELIC_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "relic prompt"), "Relic", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Obtain Relic"), "Obtain Relic", 34);
		choices = 1;

		if (m_player->hasRelic()) {
			updateLabel("Obtain Relic", fetchPromptText("Shrine", "Upgrade Relic"));
			addLabel(0, 1 * MENU_SPACING, fetchPromptText("Shrine", "Change Relic"), "Change Relic", 34);

			choices = 2;
		}
	}
	else if (shrine == CHEST_CHOICE) {
		addLabel(0, 4.5f * MENU_SPACING, fetchPromptText("Shrine", "chest prompt"), "Chest", 42);

		addLabel(0, 2 * MENU_SPACING, fetchPromptText("Shrine", "Accept"), "Accept", 34);
		choices = 1;
	}

	choices--; // Subtract one for easier code in the selection part
	addLabel(0, -2 * MENU_SPACING, fetchPromptText("Shrine", "Deny"), "Walk away", 34);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::shrineKeyPressed, this, choices), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::shrineButtonPressed, this, choices), menuSprites.find("Selector")->second);
}
void HUDLayer::shrineKeyPressed(KeyType keyCode, cocos2d::Event* event, int choices) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	
	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0 && index <= choices) {
			playInterfaceSound("Select 1.mp3");

			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);
		}
		else if (index > choices) {
			playInterfaceSound("Select 1.mp3");

			index--;
			event->getCurrentTarget()->setPosition(pos.x, (2 - choices) * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < choices) {
			playInterfaceSound("Select 1.mp3");

			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);
		}
		else if (index == choices) {
			playInterfaceSound("Select 1.mp3");

			index++;
			event->getCurrentTarget()->setPosition(pos.x, -2 * MENU_SPACING);
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE: {
		Shrine* shrine = dynamic_cast<Shrine*>(m_scene->getCurrentDungeon());
		shrine->useChoice(index);

		index = 0;
		deconstructMenu(menuSprites);

		enableGameplayListeners();
		return;
	}
	default:
		break;
	}
}
void HUDLayer::shrineButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, int choices) {
	shrineKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event, choices);
}

void HUDLayer::inventoryMenu() {
	// INVENTORY
	auto inventory = Label::createWithTTF(fetchMenuText("Inventory Menu", "Inventory"), TEXT_FONT, 54);
	inventory->setPosition(0 * RES_ADJUST, 390 * RES_ADJUST);
	inventory->setOpacity(230);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	inventory->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	inventory->setAdditionalKerning(0.25f);
	this->addChild(inventory, Z_HUD_LABEL);
	labels.insert(std::make_pair("inventory", inventory));

	// menu border
	Sprite* longBox = Sprite::create("Inventory_Box_Length_Medium.png");
	this->addChild(longBox, Z_HUD_ELEMENT);
	longBox->setPosition(-300 * RES_ADJUST, 0);
	longBox->setScale(.6f * RES_ADJUST);
	longBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("long box", longBox));

	// menu border
	Sprite* wideBox = Sprite::create("Inventory_Box_Wide_Medium.png");
	this->addChild(wideBox, Z_HUD_ELEMENT);
	wideBox->setPosition(150 * RES_ADJUST, 75 * RES_ADJUST);
	wideBox->setScale(.6f * RES_ADJUST);
	wideBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("wide box", wideBox));

	// menu border
	Sprite* textBox = Sprite::create("Inventory_Box_Text_Medium.png");
	this->addChild(textBox, Z_HUD_ELEMENT);
	textBox->setPosition(130 * RES_ADJUST, -200 * RES_ADJUST);
	textBox->setScale(.6f * RES_ADJUST);
	textBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("text box", textBox));

	// menu border
	Sprite* selectBox = Sprite::create("Selection_Box.png");
	this->addChild(selectBox, Z_HUD_ELEMENT + 1);
	selectBox->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);
	selectBox->setScale(.1f);
	selectBox->setOpacity(200);
	selectBox->setColor(cocos2d::Color3B(255, 200, 100));
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("select box", selectBox));


	// Inventory key:
	// 
	//    0 - 1:  Weapons
	//    2 - 6:  Usable items
	//    7:      Spacebar Item
	//    8:      Relic
	//    9 - 23: Passive Items
	//
	//    0   1  |  2   3   4   5   6
	//    7   8  |  9   10  11  12  13
	//    X   X  |  14  15  16  17  18
	//    X   X  |  19  20  21  22  23

	std::string image, name, desc;

	// Display use items
	for (int i = 0; i < static_cast<int>(m_player->itemCount()); i++) {
		fetchItemInfo("Usable", m_player->itemAt(i)->getName(), name, desc);

		inventoryText[i + 2] = std::make_pair(name, desc);
		image = m_player->itemAt(i)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, Z_HUD_SPRITE);
		item->setPosition((1.9 * i * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);
		item->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));
	}

	// Display weapons
	image = m_player->getWeapon()->getImageName();
	fetchItemInfo("Weapon", m_player->getWeapon()->getName(), name, desc);
	inventoryText[0] = std::make_pair(name, desc);

	Sprite* weapon = Sprite::createWithSpriteFrameName(image);
	this->addChild(weapon, Z_HUD_SPRITE);
	weapon->setScale(1.0);
	weapon->setPosition((-350) * RES_ADJUST, 150 * RES_ADJUST);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));

	if (m_player->getStoredWeapon() != nullptr) {
		image = m_player->getStoredWeapon()->getImageName();
		fetchItemInfo("Weapon", m_player->getStoredWeapon()->getName(), name, desc);
		inventoryText[1] = std::make_pair(name, desc);

		Sprite* storedWeapon = Sprite::createWithSpriteFrameName(image);
		this->addChild(storedWeapon, Z_HUD_SPRITE);
		storedWeapon->setScale(1.0);
		storedWeapon->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Stored Weapon", storedWeapon));
	}
	/// End weapon display

	// Label for the item's name
	m_itemName = Label::createWithTTF("", TEXT_FONT, 20);
	m_itemName->setPosition(130 * RES_ADJUST, -150 * RES_ADJUST);
	m_itemName->setOpacity(230);
	m_itemName->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_itemName->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_itemName->setAdditionalKerning(0.25f);
	this->addChild(m_itemName, Z_HUD_LABEL);
	labels.insert(std::make_pair("item name", m_itemName));

	// Label for the item's description
	m_itemDescription = Label::createWithTTF("", TEXT_FONT, 20);
	m_itemDescription->setPosition(130 * RES_ADJUST, -170 * RES_ADJUST);
	m_itemDescription->setAnchorPoint(Vec2(0.5, 1.0));
	m_itemDescription->setOpacity(230);
	m_itemDescription->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	m_itemDescription->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	m_itemDescription->setAdditionalKerning(0.25f);
	this->addChild(m_itemDescription, Z_HUD_LABEL);
	labels.insert(std::make_pair("item description", m_itemDescription));

	// Displays first item info
	m_itemName->setString(inventoryText[0].first);
	std::string currentDesc = inventoryText[0].second;
	formatItemDescriptionForDisplay(currentDesc);
	m_itemDescription->setString(currentDesc);

	if (m_player->hasActiveItem()) {
		std::string shield = m_player->getActiveItem()->getName();
		fetchItemInfo("Usable", shield, name, desc);
		inventoryText[7] = std::make_pair(name, desc);

		image = m_player->getActiveItem()->getImageName();

		// shield sprite
		Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentshield, Z_HUD_SPRITE, shield);
		currentshield->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);
		currentshield->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
	}

	if (m_player->hasRelic()) {
		std::string relic = m_player->getRelic()->getName();
		fetchItemInfo("Relic", relic, name, desc);

		if (m_player->getRelic()->getLevel() > 1)
			name += " +" + std::to_string(m_player->getRelic()->getLevel() - 1);

		inventoryText[8] = std::make_pair(name, desc);

		image = m_player->getRelic()->getImageName();

		// trinket sprite
		Sprite* currentRelic = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentRelic, Z_HUD_SPRITE, relic);
		currentRelic->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);
		currentRelic->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Current Relic", currentRelic));

	}

	// Display passives
	for (int i = 0; i < static_cast<int>(m_player->passiveCount()); i++) {
		fetchItemInfo("Passive", m_player->passiveAt(i)->getName(), name, desc);
		inventoryText[i + 9] = std::make_pair(name, desc);
		image = m_player->passiveAt(i)->getImageName();

		Sprite* passive = Sprite::createWithSpriteFrameName(image);
		this->addChild(passive, Z_HUD_SPRITE);

		// X: i % 5 since 5 entries per row; Y: 80 is the number of pixels to next row, i / 5 to get the correct row
		passive->setPosition((1.9 * (i % 5) * MENU_SPACING - 90) * RES_ADJUST, (100 - (80 * (i / 5))) * RES_ADJUST);

		passive->setScale(1.0f);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Passive", passive));
	}

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::inventoryMenuKeyPressed, this), selectBox);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::inventoryMenuButtonPressed, this), selectBox);
}
void HUDLayer::inventoryMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	// Inventory key:
	// 
	//    0 - 1:  Weapons
	//    2 - 6:  Usable items
	//    7:      Relic
	//    8:      Spacebar Item
	//    9 - 23: Passive Items
	//
	//    0   1  |  2   3   4   5   6
	//    7   8  |  9   10  11  12  13
	//    X   X  |  14  15  16  17  18
	//    X   X  |  19  20  21  22  23

	if (keyCode == INVENTORY_KEY) {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 23; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		updateHUD();
		enableGameplayListeners();

		return;
	}

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index == 0) {
			index = 6; // end of weapons
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index--;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 3 && index <= 6) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 7) {
			index = 13; // end of items
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 9) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 9 && index <= 13) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index > 14 && index <= 18) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 14) {
			index = 18;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 2 && index < 6) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 6) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 7) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 9;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index < 13) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 13) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 14 && index < 18) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 18) {
			index = 14;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_UP_ARROW: {
		if (index == 7) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 8) {
			index = 1;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index <= 13) {
			index -= 7;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 14 && index <= 18) {
			index -= 5;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 0) {
			index = 7;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index = 8;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 2 && index <= 6) {
			index += 7;
			event->getCurrentTarget()->setPosition(((index - 9) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index >= 9 && index <= 13) {
			index += 5;
			event->getCurrentTarget()->setPosition(((index - 14) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 20 * RES_ADJUST);

			playInterfaceSound("Select 1.mp3");
		}

		m_itemName->setString(inventoryText[index].first);
		std::string currentDesc = inventoryText[index].second;
		formatItemDescriptionForDisplay(currentDesc);
		m_itemDescription->setString(currentDesc);

		break;
	}
	case KeyType::KEY_I:
	case KeyType::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 23; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		updateHUD();
		enableGameplayListeners();

		return;
	}
	default: break;
	}
}
void HUDLayer::inventoryMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = gameplayButtonToKey(static_cast<ButtonType>(keyCode));
	if (key == INVENTORY_KEY)
		inventoryMenuKeyPressed(key, event);
	else
		inventoryMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::itemMenu() {
	// |----|----|----|----|----|
	// |item|item|item|item|empt|
	// |    |    |    |    |    |
	// |----|----|----|----|----|

	// [RENDER MENU DESIGN]

	for (int i = -2; i < 3; i++) {
		Sprite* box = Sprite::create("Menu_Box1.png");
		this->addChild(box, Z_HUD_ELEMENT);
		box->setPosition(i * 65, 3);
		box->setOpacity(200);
		box->setScale(1.5f);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));
	}

	std::string image = "cheese.png";
	int size = m_player->itemCount();

	for (int i = -2; i < size - 2; i++) {
		image = m_player->itemAt(i + 2)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, Z_HUD_SPRITE);
		item->setPosition(i * 65, 0.0f);
		item->setScale(1.0f);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));

		// If item is stackable, display stack amount for item
		if (m_player->itemAt(i + 2)->canStack() || m_player->hasFatStacks()) {
			auto stack = Label::createWithTTF("x" + std::to_string(m_player->itemAt(i + 2)->getCount()), TEXT_FONT, 18);
			stack->setPosition(i * 65 + 15.f, -15.f);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
			stack->setColor(Color3B(220, 220, 220));
			stack->setOpacity(255);
			this->addChild(stack, Z_HUD_LABEL);
			labels.insert(std::pair<std::string, Label*>(std::to_string(i + 2), stack));
		}
	}

	// arrow sprite for selection
	auto selector = Sprite::create("Down_Arrow.png");
	selector->setPosition(-2 * 65, 65);
	selector->setScale(3.0f);
	this->addChild(selector, Z_HUD_SPRITE);
	itemMenuSprites.insert(std::pair<std::string, Sprite*>("Selector", selector));

	// helper labels
	auto equip = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY) + ": " + fetchMenuText("Item Menu", "Quick Use Info"), TEXT_FONT, 20);
	equip->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	equip->setOpacity(200);
	equip->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	equip->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	equip->setAdditionalKerning(0.25f);
	this->addChild(equip, Z_HUD_LABEL);
	labels.insert(std::make_pair("equip", equip));

	auto use = Label::createWithTTF("Space/Enter: " + fetchMenuText("Item Menu", "Use Item"), TEXT_FONT, 20);
	use->setPosition(0 * RES_ADJUST, -300 * RES_ADJUST);
	use->setOpacity(200);
	use->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	use->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	use->setAdditionalKerning(0.25f);
	this->addChild(use, Z_HUD_LABEL);
	labels.insert(std::make_pair("use", use));

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::itemMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::itemMenuButtonPressed, this), selector);
}
void HUDLayer::itemMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	if (keyCode == ITEM_KEY) {
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();

		return;
	}
	else if (keyCode == QUICK_KEY) {
		playInterfaceSound("Confirm 1.mp3");

		// if player has items, set this to new quick item slot
		if (m_player->hasItems())
			m_scene->getCurrentDungeon()->assignQuickItem(index);
		
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();
		return;
	}

	switch (keyCode) {
	case KeyType::KEY_LEFT_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x - 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0 && m_player->itemCount() > 1) {
			index = m_player->itemCount() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_RIGHT_ARROW: {
		if (index < static_cast<int>(m_player->itemCount()) - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == m_player->itemCount() - 1 && m_player->itemCount() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * 65, 65);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ENTER: {
		playInterfaceSound("Confirm 1.mp3");

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActionsByTag(1) > 0)
			actions->update(1.0, 1);
		
		if (m_player->hasItems())
			m_scene->getCurrentDungeon()->callUse(index);

		m_scene->getCurrentDungeon()->peekDungeon('-');
	}
	case KeyType::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(itemMenuSprites);

		updateHUD();
		enableGameplayListeners();

		return;
	}
	default: break;
	}
}
void HUDLayer::itemMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	KeyType key = gameplayButtonToKey(static_cast<ButtonType>(keyCode));
	if (key == ITEM_KEY || key == QUICK_KEY)
		itemMenuKeyPressed(key, event);
	else
		itemMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::gameOver() {
	// prevent player movement
	Director::getInstance()->getScheduler()->unscheduleAllForTarget(m_scene);
	this->_eventDispatcher->removeAllEventListeners();

	//cocos2d::experimental::AudioEngine::stopAll();
	playMusic("Fallen in Battle.mp3", false);

	deconstructMenu(menuSprites);

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();


	//       PAUSE
	//    |---------|
	// -> |  Resume |
	//    | Restart |
	//    |         |
	//    |Exit Game|
	//    |---------|


	constructSelectionMenu();

	// Game over!
	auto pause = Label::createWithTTF(fetchPromptText("Game Over", "prompt"), TEXT_FONT, 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, Z_HUD_LABEL);

	// Back to Menu option
	auto back = Label::createWithTTF(fetchPromptText("Game Over", "Main Menu"), TEXT_FONT, 36);
	back->setPosition(0, 2 * MENU_SPACING);
	this->addChild(back, Z_HUD_LABEL);

	// Restart option
	auto resume = Label::createWithTTF(fetchPromptText("Game Over", "Restart"), TEXT_FONT, 36);
	resume->setPosition(0, 1 * MENU_SPACING);
	this->addChild(resume, Z_HUD_LABEL);

	// Quit option
	auto exit = Label::createWithTTF(fetchPromptText("Game Over", "Exit Game"), TEXT_FONT, 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, Z_HUD_LABEL);


	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::gameOverKeyPressed, this), menuSprites.find("Selector")->second);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::gameOverButtonPressed, this), menuSprites.find("Selector")->second);
}
void HUDLayer::gameOverKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index == 1) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	//case KeyType::KEY_SPACE:
		switch (index) {
			// Back to main menu
		case 0: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			playInterfaceSound("Confirm 1.mp3");

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// advance to start menu scene
			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
			// Restart
		case 1: {
			restartGame(*m_player);
			playInterfaceSound("Confirm 1.mp3");
			return;
		}
		case 2: // Exit Game
			Director::getInstance()->end();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::gameOverButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	gameOverKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::winner() {
	// prevent player movement
	this->_eventDispatcher->removeAllEventListeners();


	cocos2d::experimental::AudioEngine::stopAll();
	cocos2d::experimental::AudioEngine::play2d("Victory! All Clear.mp3", false);


	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();


	//       PAUSE
	//    |---------|
	// -> |  Resume |
	//    | Restart |
	//    |         |
	//    |Exit Game|
	//    |---------|


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, Z_HUD_ELEMENT);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);

	// You won!
	auto pause = Label::createWithTTF(fetchPromptText("Winner", "prompt"), TEXT_FONT, 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, Z_HUD_LABEL);

	// Resume option
	auto resume = Label::createWithTTF(fetchPromptText("Winner", "Restart"), TEXT_FONT, 36);
	resume->setPosition(0, 2 * MENU_SPACING);
	this->addChild(resume, Z_HUD_LABEL);

	// Quit option
	auto exit = Label::createWithTTF(fetchPromptText("Winner", "Exit Game"), TEXT_FONT, 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, Z_HUD_LABEL);

	auto selector = Sprite::create("Right_Arrow.png");
	selector->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(selector, Z_HUD_SPRITE);
	selector->setScale(2.5f);

	createKeyboardEventListener(CC_CALLBACK_2(HUDLayer::winnerKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(HUDLayer::winnerButtonPressed, this), selector);
}
void HUDLayer::winnerKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// Restart
		case 0: {
			restartGame(*m_player);
			playInterfaceSound("Confirm 1.mp3");
			return;
		}
		case 1: // Exit Game
			Director::getInstance()->end();
			return;
		}
	default:
		break;
	}
}
void HUDLayer::winnerButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	winnerKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void HUDLayer::constructSelectionMenu() {
	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, Z_HUD_ELEMENT);
	box->setPosition(0, 0);
	box->setScale(0.32f);
	box->setOpacity(170);
	menuSprites.insert(std::pair<std::string, Sprite*>("box", box));

	// arrow sprite for selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(arrow, Z_HUD_SPRITE);
	arrow->setScale(3.0f);
	menuSprites.insert(std::pair<std::string, Sprite*>("Selector", arrow));
}

void HUDLayer::deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites) {
	// remove menu images
	for (auto &it : sprites)
		it.second->removeFromParent();
	
	sprites.clear();

	// remove any labels
	for (auto &it : labels)
		it.second->removeFromParent();
	
	labels.clear();
}

void HUDLayer::checkPlayerStats() {
	float x_scale = m_player->getHP() / (static_cast<float>(m_player->getMaxHP()) * 1.0);
	cocos2d::Action* move = cocos2d::ScaleTo::create(.4f, x_scale * 1.5f, 1.5f);
	auto action = HUD.find("hp")->second->runAction(move);
	action->setTag(5);

	m_persistentLabels.find("Money Count")->second->setString(std::to_string(m_player->getMoney()));
	m_persistentLabels.find("Money Bonus")->second->setString("Money Bonus : " + std::to_string(static_cast<int>(m_player->getMoneyBonus())));

	m_persistentLabels.find("HP")->second->setString(std::to_string(m_player->getHP()) + "/" + std::to_string(m_player->getMaxHP()));

	m_persistentLabels.find("Strength")->second->setString("Str: +" + std::to_string(m_player->getStr()));
	m_persistentLabels.find("Dexterity")->second->setString("Dex: +" + std::to_string(m_player->getDex() + m_player->getWeapon()->getDexBonus()));
	m_persistentLabels.find("Intellect")->second->setString("Int: +" + std::to_string(m_player->getInt()));
	m_persistentLabels.find("Armor")->second->setString("Amr: +" + std::to_string(m_player->getArmor()));
}

void HUDLayer::constructActiveItemHUD() {
	std::string name = m_player->getActiveItem()->getName();
	std::string image = m_player->getActiveItem()->getImageName();

	// shield HUD box
	Sprite* activeBox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	activeBox->setPosition(-870 * RES_ADJUST, 290 * RES_ADJUST);
	activeBox->setScale(0.4f * RES_ADJUST);
	activeBox->setOpacity(160);
	activeBox->setColor(cocos2d::Color3B(255, 175, 5));
	this->addChild(activeBox, Z_HUD_ELEMENT, "Active Box");
	HUD.insert(std::pair<std::string, Sprite*>("Active Box", activeBox));

	// Active Item sprite
	Sprite* active = Sprite::createWithSpriteFrameName(image);
	this->addChild(active, Z_HUD_SPRITE, name);
	active->setPosition(-870.f * RES_ADJUST, 290.f * RES_ADJUST);
	active->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", active));

	if (m_player->activeHasMeter()) {
		// Active Item bar
		Sprite* activeBar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
		this->addChild(activeBar, Z_HUD_ELEMENT, "Active Bar");
		activeBar->setPosition(-850 * RES_ADJUST + SP_ADJUST, 250 * RES_ADJUST);
		activeBar->setScale(1.5f * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Bar", activeBar));

		// Active Item bar points
		Sprite* activePoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
		this->addChild(activePoints, Z_HUD_ELEMENT + 1, "Active Points");
		activePoints->setScale(1.5f);
		activePoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
		activePoints->setPosition(-888 * RES_ADJUST, 250.5 * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Points", activePoints));
	}

	
	auto space = Label::createWithTTF(convertKeycodeToStr(ACTIVE_KEY), TEXT_FONT, 18);
	space->setPosition(-870.f * RES_ADJUST, 265.f * RES_ADJUST);
	space->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	space->setOpacity(245);
	this->addChild(space, Z_HUD_LABEL);
	keyLabels.insert(std::pair<std::string, Label*>("Active Key", space));

	// Display stack amount for Spelunker
	if (m_player->getName() == SPELUNKER) {
		std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(m_player);
		int count = sp->getRockCount();
		sp.reset();

		// If there was no label for stackable items, add it
		if (keyLabels.find("Active Stack Amount") == keyLabels.end()) {
			auto stack = Label::createWithTTF("x" + std::to_string(count), TEXT_FONT, 16);
			stack->setPosition(-840.f * RES_ADJUST, 290.f * RES_ADJUST);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
			stack->setOpacity(250);
			this->addChild(stack, Z_HUD_LABEL);
			keyLabels.insert(std::pair<std::string, Label*>("Active Stack Amount", stack));
		}
		else {
			keyLabels.find("Active Stack Amount")->second->setString("x" + std::to_string(count));
		}
	}
}
void HUDLayer::checkActiveItemHUD() {
	if (m_player->hasActiveItem()) {
		std::string name = m_player->getActiveItem()->getName();

		if (keyLabels.find("Active Key") != keyLabels.end())
			keyLabels.find("Active Key")->second->setString(convertKeycodeToStr(ACTIVE_KEY));

		// If there wasn't an active item equipped previously, construct the menu
		if (HUD.find("Active") == HUD.end())
			constructActiveItemHUD();

		// else if current active item is different, switch the sprite
		else if (HUD.find("Active")->second->getName() != name)
			updateActiveItemHUD();

		// Display stack amount for Spelunker
		else if (m_player->getName() == SPELUNKER) {
			std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(m_player);
			int count = sp->getRockCount();
			sp.reset();

			keyLabels.find("Active Stack Amount")->second->setString("x" + std::to_string(count));
		}
	}
	else {
		// If there's no active item equipped and there previously was, deconstruct the HUD
		if (HUD.find("Active") != HUD.end())
			deconstructActiveItemHUD();
	}

	updateActiveItemBar();
}
void HUDLayer::updateActiveItemHUD() {
	// Remove old sprite
	HUD.find("Active")->second->removeFromParent();
	HUD.erase("Active");

	std::string name = m_player->getActiveItem()->getName();
	std::string image = m_player->getActiveItem()->getImageName();

	Sprite* newActive = Sprite::createWithSpriteFrameName(image);
	this->addChild(newActive, Z_HUD_SPRITE, name);
	newActive->setPosition(-870 * RES_ADJUST, 290 * RES_ADJUST);
	newActive->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", newActive));
}
void HUDLayer::updateActiveItemBar() {
	// if active has a meter, update it
	if (m_player->hasActiveItem() && m_player->activeHasMeter()) {
		float sx_scale = m_player->getCurrentActiveMeter() / (static_cast<float>(m_player->getMaxActiveMeter()) * 1.0);
		cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, sx_scale * 1.5f, 1.5f);
		HUD.find("Active Points")->second->runAction(move);
	}
}
void HUDLayer::deconstructActiveItemHUD() {
	if (HUD.find("Active Box") != HUD.end()) {
		// deconstruct the shield HUD because there's no shield equipped
		HUD.find("Active")->second->removeFromParent();
		HUD.find("Active Box")->second->removeFromParent();
		keyLabels.find("Active Key")->second->removeFromParent();

		HUD.erase(HUD.find("Active"));
		HUD.erase(HUD.find("Active Box"));
		keyLabels.erase(keyLabels.find("Active Key"));

		if (m_player->activeHasMeter()) {
			HUD.find("Active Bar")->second->removeFromParent();
			HUD.find("Active Points")->second->removeFromParent();
			HUD.erase(HUD.find("Active Bar"));
			HUD.erase(HUD.find("Active Points"));
		}

		if (m_player->getName() == SPELUNKER) {
			keyLabels.find("Active Stack Amount")->second->removeFromParent();
			keyLabels.erase(keyLabels.find("Active Stack Amount"));
		}
	}
}

void HUDLayer::constructItemHUD() {
	// Quick access item use slot
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* quickAccess = Sprite::createWithSpriteFrameName(image);
	quickAccess->setPosition(870 * RES_ADJUST, 390 * RES_ADJUST);
	quickAccess->setScale(0.4f * RES_ADJUST);
	quickAccess->setOpacity(160);
	quickAccess->setColor(cocos2d::Color3B(250, 188, 165));
	this->addChild(quickAccess, Z_HUD_ELEMENT, "quick access");
	HUD.insert(std::pair<std::string, Sprite*>("quick access", quickAccess));

	// key label
	auto qKey = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY), TEXT_FONT, 18);
	qKey->setPosition(870.f * RES_ADJUST, 362.f * RES_ADJUST);
	qKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	qKey->setOpacity(245);
	this->addChild(qKey, Z_HUD_LABEL);
	keyLabels.insert(std::pair<std::string, Label*>("qKey", qKey));

	updateItemHUD();
}
void HUDLayer::checkItemHUD() {
	if (m_player->hasItems()) {
		std::string image = m_player->itemAt(0)->getImageName();
		std::string item = m_player->itemAt(0)->getName();

		if (keyLabels.find("qKey") != keyLabels.end())
			keyLabels.find("qKey")->second->setString(convertKeycodeToStr(QUICK_KEY));

		// if there weren't any items before, construct menu again
		if (HUD.find("quick access") == HUD.end())
			constructItemHUD();
		
		// else if the item is different, switch the sprite
		else if (HUD.find("quick item") != HUD.end() && HUD.find("quick item")->second->getName() != item) {
			HUD.find("quick item")->second->removeFromParent();
			HUD.erase(HUD.find("quick item"));

			updateItemHUD();
		}

		// Display stack amount for item
		if (m_player->itemAt(0)->canStack() || m_player->hasFatStacks()) {

			// If there was no label for stackable items, add it
			if (keyLabels.find("Stack Amount") == keyLabels.end()) {
				auto stack = Label::createWithTTF("x" + std::to_string(m_player->itemAt(0)->getCount()), TEXT_FONT, 18);
				stack->setPosition(900.f * RES_ADJUST, 390.f * RES_ADJUST);
				stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
				stack->setOpacity(250);
				this->addChild(stack, Z_HUD_LABEL);
				keyLabels.insert(std::pair<std::string, Label*>("Stack Amount", stack));
			}
		}
		// Otherwise if a stack amount label exists
		if (keyLabels.find("Stack Amount") != keyLabels.end()) {

			// Remove it because the new item can't stack, or it's a different item than before
			if (!(m_player->itemAt(0)->canStack() || m_player->hasFatStacks()) || HUD.find("quick item")->second->getName() != item) {
				keyLabels.find("Stack Amount")->second->removeFromParent();
				keyLabels.erase(keyLabels.find("Stack Amount"));
			}
			// Update the count
			else if (HUD.find("quick item")->second->getName() == item) {
				keyLabels.find("Stack Amount")->second->setString("x" + std::to_string(m_player->itemAt(0)->getCount()));
			}
		}
	}
	else {
		// if there's no item quick slotted but there was previously, deconstruct the HUD
		if (HUD.find("quick access") != HUD.end())
			deconstructItemHUD();
	}
}
void HUDLayer::updateItemHUD() {
	// The item to display
	Sprite* quickitem = Sprite::createWithSpriteFrameName(m_player->itemAt(0)->getImageName());
	this->addChild(quickitem, Z_HUD_SPRITE, m_player->itemAt(0)->getName());
	quickitem->setPosition(870.f * RES_ADJUST, 390.f * RES_ADJUST);
	quickitem->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("quick item", quickitem));
}
void HUDLayer::deconstructItemHUD() {
	if (HUD.find("quick item") != HUD.end()) {
		// deconstruct the quick item HUD because player has no items
		HUD.find("quick access")->second->removeFromParent();
		HUD.find("quick item")->second->removeFromParent();
		keyLabels.find("qKey")->second->removeFromParent();

		HUD.erase(HUD.find("quick access"));
		HUD.erase(HUD.find("quick item"));
		keyLabels.erase(keyLabels.find("qKey"));
	}

	if (keyLabels.find("Stack Amount") != keyLabels.end()) {
		keyLabels.find("Stack Amount")->second->removeFromParent();
		keyLabels.erase(keyLabels.find("Stack Amount"));
	}
}

void HUDLayer::constructRelicHUD() {
	// Relic HUD box
	Sprite* trinketbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	trinketbox->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	trinketbox->setScale(.4f * RES_ADJUST);
	trinketbox->setOpacity(200);
	trinketbox->setColor(cocos2d::Color3B(200, 20, 0));
	this->addChild(trinketbox, Z_HUD_ELEMENT, "Relic Box");
	HUD.insert(std::pair<std::string, Sprite*>("Relic Box", trinketbox));

	// Relic sprite
	Sprite* currentTrinket = Sprite::createWithSpriteFrameName(m_player->getRelic()->getImageName());
	this->addChild(currentTrinket, Z_HUD_SPRITE, m_player->getRelic()->getName());
	currentTrinket->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	currentTrinket->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("Current Relic", currentTrinket));
}
void HUDLayer::checkRelicHUD() {
	if (m_player->hasRelic()) {
		std::string relic = m_player->getRelic()->getName();

		// If there is now a relic equipped, but there wasn't previously, construct the menu
		if (HUD.find("Current Relic") == HUD.end())
			constructRelicHUD();

		// else if current relic equipped is different, switch the sprite
		else if (HUD.find("Current Relic")->second->getName() != relic)
			updateRelicHUD();
	}
	else {
		// if there's no relic equipped and there was previously, deconstruct the HUD
		if (HUD.find("Current Relic") != HUD.end())
			deconstructRelicHUD();
	}
}
void HUDLayer::updateRelicHUD() {
	HUD.find("Current Relic")->second->removeFromParent();
	HUD.erase("Current Relic");

	std::string relic = m_player->getRelic()->getName();
	std::string image = m_player->getRelic()->getImageName();

	Sprite* currentRelic = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentRelic, Z_HUD_SPRITE, relic);
	currentRelic->setPosition(-870 * RES_ADJUST, 190 * RES_ADJUST);
	currentRelic->setScale(1.0f);
	HUD.insert(std::pair<std::string, Sprite*>("Current Relic", currentRelic));
}
void HUDLayer::deconstructRelicHUD() {
	if (HUD.find("Relic Box") != HUD.end()) {
		// deconstruct the trinket HUD because there's no trinket equipped
		HUD.find("Current Relic")->second->removeFromParent();
		HUD.find("Relic Box")->second->removeFromParent();

		HUD.erase(HUD.find("Current Relic"));
		HUD.erase(HUD.find("Relic Box"));
	}
}

void HUDLayer::constructWeaponHUD() {
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* wepbox = Sprite::createWithSpriteFrameName(image);
	wepbox->setPosition(-870 * RES_ADJUST, 390 * RES_ADJUST);
	wepbox->setScale(.4f * RES_ADJUST);
	wepbox->setOpacity(160);
	this->addChild(wepbox, Z_HUD_ELEMENT, "wepbox");
	HUD.insert(std::pair<std::string, Sprite*>("wepbox", wepbox));

	// load default weapon sprite
	std::string weapon;
	weapon = m_player->getWeapon()->getName();
	image = m_player->getWeapon()->getImageName();
	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, Z_HUD_SPRITE, weapon);
	currentwep->setPosition(-870 * RES_ADJUST, 390 * RES_ADJUST);
	currentwep->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));

	// if weapon has casting ability, add label
	if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();
}
void HUDLayer::checkWeaponHUD() {
	std::string weapon = m_player->getWeapon()->getName();
	std::string image = m_player->getWeapon()->getImageName();

	if (keyLabels.find("cast key") != keyLabels.end())
		keyLabels.find("cast key")->second->setString(convertKeycodeToStr(CAST_KEY));

	// if current weapon equipped is different, switch the weapon sprite
	if (HUD.find("currentwep")->second->getName() != weapon)
		updateWeaponHUD();

	// If there was a casting label that can be removed because the player used the special, remove it
	else if (!m_player->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end())
		removeWeaponCastLabel();

	else if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();
}
void HUDLayer::updateWeaponHUD() {
	HUD.find("currentwep")->second->removeFromParent();
	HUD.erase("currentwep");

	std::string weapon = m_player->getWeapon()->getName();
	std::string image = m_player->getWeapon()->getImageName();

	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, Z_HUD_SPRITE, weapon);
	currentwep->setPosition(-870.f * RES_ADJUST, 390.f * RES_ADJUST);
	currentwep->setScale(1.0f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));

	// if weapon has casting ability, add label
	if (m_player->getWeapon()->canBeCast())
		addWeaponCastLabel();

	// else remove the label if previous weapon could be cast
	else if (!m_player->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end())
		removeWeaponCastLabel();
}
void HUDLayer::addWeaponCastLabel() {
	if (keyLabels.find("cast key") == keyLabels.end()) {
		auto castKey = Label::createWithTTF(convertKeycodeToStr(CAST_KEY), TEXT_FONT, 18);
		castKey->setPosition(-870.f * RES_ADJUST, 362.0f * RES_ADJUST);
		castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
		//castKey->setOpacity(255);
		this->addChild(castKey, Z_HUD_LABEL);
		keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
	}
}
void HUDLayer::removeWeaponCastLabel() {
	auto it = keyLabels.find("cast key");
	it->second->removeFromParent();
	keyLabels.erase(it);
}

void HUDLayer::deconstructShopHUD() {
	m_itemPrice->removeFromParent();
	m_itemPrice = nullptr;
}
void HUDLayer::deconstructBossHUD() {
	if (HUD.find("bosshp") != HUD.end()) {
		// deconstruct the boss health bar because smasher is dead
		HUD.find("bosshp")->second->removeFromParent();
		HUD.find("bosshealthbar")->second->removeFromParent();

		HUD.erase(HUD.find("bosshp"));
		HUD.erase(HUD.find("bosshealthbar"));
	}
}

void HUDLayer::createPersistentLabel(float x, float y, const std::string &text, const std::string &id, float fontSize) {
	cocos2d::Label *label = Label::createWithTTF(text, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	label->setOpacity(230);
	label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	//label->setAdditionalKerning(0.25f);
	this->addChild(label, Z_HUD_LABEL);

	m_persistentLabels.insert(std::make_pair(id, label));
}
void HUDLayer::addLabel(float x, float y, const std::string &text, const std::string &id, float fontSize) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto label = Label::createWithTTF(text, TEXT_FONT, fontSize);
	label->setPosition(x, y);
	//label->setOpacity(230);
	//label->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	//label->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	//label->setAdditionalKerning(0.25f);
	this->addChild(label, Z_HUD_LABEL);
	labels.insert(std::make_pair(id, label));
}
void HUDLayer::updateLabel(const std::string &id, std::string newText) {
	labels.find(id)->second->setString(newText);
}

void HUDLayer::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, node);
}
void HUDLayer::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}

void HUDLayer::enableGameplayListeners() {
	m_keyRelease = EventListenerKeyboard::create();
	m_keyRelease->onKeyReleased = CC_CALLBACK_2(HUDLayer::menuKeyReleased, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(m_keyRelease, this);

	m_buttonRelease = EventListenerController::create();
	m_buttonRelease->onKeyDown = CC_CALLBACK_3(HUDLayer::menuButtonReleased, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(m_buttonRelease, this);
}
void HUDLayer::menuKeyReleased(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ENTER:
	case KeyType::KEY_ESCAPE:
	default:
		this->_eventDispatcher->removeEventListener(m_keyRelease);
		this->_eventDispatcher->removeEventListener(m_buttonRelease);
		m_scene->enableListeners();
		//activeListener->setEnabled(true);
		//activeListener = nullptr;
	}
}
void HUDLayer::menuButtonReleased(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	menuKeyReleased(KeyType::KEY_ENTER, event);
}


//		BACKGROUND LAYER
bool BackgroundLayer::init() {
	if (!Layer::init())
		return false;


	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	std::string image;

	Sprite* background1 = Sprite::create("Level1bg1.png");
	background1->setPosition(Vec2(origin.x, origin.y));
	/// Used for setting correct position when using fancy transition 
	///background1->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
	this->addChild(background1, -5);

	Sprite* background2 = Sprite::create("Level1bg2.png");
	background2->setPosition(Vec2(origin.x, origin.y));
	//background2->setScale(1.0);
	this->addChild(background2, -6);

	Sprite* background3 = Sprite::create("Level1bg3.png");
	background3->setPosition(Vec2(origin.x, origin.y));
	//background3->setScale(1.0);
	this->addChild(background3, -7);

	return true;
}
void BackgroundLayer::updateBackground() {

}


//		SCENE FOR LEVELS
LevelScene::LevelScene(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level) : m_hud(hud), m_cameraLayer(cameraLayer), p(p), m_level(level) {
	
}
LevelScene::~LevelScene() {
	if (m_currentDungeon != nullptr)
		delete m_currentDungeon;
}

Scene* LevelScene::createScene(std::shared_ptr<Player> p, int level) {
	auto scene = Scene::create();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// create HUD layer
	HUDLayer* hud = HUDLayer::create(p);
	hud->setPosition(Vec2(vsw, vsh));
	scene->addChild(hud, 10);

	cocos2d::Node* cameraLayer = Node::create();
	scene->addChild(cameraLayer, 5);

	// create background layer
	//BackgroundLayer* bglayer = BackgroundLayer::create();
	//scene->addChild(bglayer, -10);

	// calls LevelScene init()
	LevelScene* levelScene = LevelScene::create(hud, cameraLayer, p, level);
	hud->setScene(levelScene);
	scene->addChild(levelScene, 1, "Level Scene");

	return scene;
}
LevelScene* LevelScene::create(HUDLayer* hud, cocos2d::Node* cameraLayer, std::shared_ptr<Player> p, int level)
{
	LevelScene *pRet = new(std::nothrow) LevelScene(hud, cameraLayer, p, level);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool LevelScene::init() {
	if (!Scene::init())
		return false;
	
	createLightEntity();

	// Create the dungeon
	setCurrentDungeon(m_level, p);

	m_hud->updateHUD();

	setMusic(m_currentDungeon->getLevel());

	// Reveal boss hp bar, if necessary
	if (m_currentDungeon->getLevel() == FIRST_BOSS)
		m_hud->showBossHP();

	// Hides the extra game layer!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1);

	createPlayerSpriteAndCamera();

	/// nonsense camera
	//auto visibleSize = Director::getInstance()->getVisibleSize();
	//removeChild(getDefaultCamera());
	//m_gameCamera = Camera::createPerspective(50.0f, 1920.f / 1080.f, 1.0, 2000);
	////m_gameCamera->setCameraMask((unsigned short)CameraFlag::DEFAULT, true); // mask on the node
	//m_gameCamera->lookAt(Vec3(0, 0, 0)); // camera turns to look at this point
	//m_gameCamera->setPositionZ(100);
	//this->addChild(m_gameCamera);

	updateLevelLighting();
		
	createGameplayListeners();
	
	//scheduleTimer();

	return true;
}

void LevelScene::createGameplayListeners() {
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(LevelScene::LevelKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, p->getSprite()); // check this for player

	controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = CC_CALLBACK_3(LevelScene::LevelButtonPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, p->getSprite());
}
void LevelScene::createKeyboardEventListener(std::function<void(KeyType, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, node);
}
void LevelScene::createControllerEventListener(std::function<void(cocos2d::Controller*, int, cocos2d::Event*)> callback, cocos2d::Node* node) {
	auto controllerListener = EventListenerController::create();
	controllerListener->onKeyDown = callback;
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, node);
}
cocos2d::Sprite* LevelScene::createSprite(std::string image, int x, int y, int z) {
	auto vSize = Director::getInstance()->getVisibleSize();
	auto vWidth = vSize.width;
	auto vHeight = vSize.height;

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	//sprite->visit();

	this->addChild(sprite, z);
	//graySprite(sprite);

	return sprite;

	//// Create new Sprite without scale, which perfoms much better
	//auto newSprite = Sprite::createWithTexture(renderTexture->getSprite()->getTexture());
	//newSprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (rows - y) - Y_OFFSET);
	//addChild(newSprite, z);
}
cocos2d::Sprite* LevelScene::createAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	this->addChild(sprite, z);

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto action = sprite->runAction(cocos2d::RepeatForever::create(cocos2d::Animate::create(animation)));
	action->setTag(10);

	return sprite;
}
/*
void LevelScene::graySprite(Sprite* sprite) {
	if (sprite)
	{
		GLProgram * p = new GLProgram();
		p->initWithFilenames("pass.vsh", "shadowMap.fsh");
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION);
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR);
		p->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORDS);
		p->link();
		p->updateUniforms();
		sprite->setShaderProgram(p);
	}
}*/
EffectSprite* LevelScene::createEffectSprite(std::string image, int x, int y, int z) {
	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y));

	auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	EffectSprite* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	sprite->visit();
	sprite->setEffect(m_lighting, "SpritesheetNormals_Dummy_n.png");

	this->addChild(sprite, z);

	return sprite;
}
void LevelScene::setLightingOn(EffectSprite* sprite) {
	sprite->setEffect(m_lighting, "SpritesheetNormals_Dummy_n.png");
}
void LevelScene::movePlayerLightTo(float x, float y) {
	m_lighting->setLightPos(Vec3(x, y, 1000));
}

void LevelScene::LevelKeyPressed(KeyType keyCode, Event* event) {
	// Unschedule the inaction timer
	if (!(keyCode == INVENTORY_KEY || keyCode == ITEM_KEY ||
		keyCode == PAUSE_KEY || keyCode == KeyType::KEY_ESCAPE))
		unscheduleTimer();

	auto actions = this->getActionManager();

	// Finish any sprite actions that are allowed to finish early
	while (actions->getNumberOfRunningActionsByTag(1) > 0)
		actions->update(1.0, 1);

	int currentLevel = m_currentDungeon->getLevel();

	if (keyCode == LEFT_KEY) {
		m_currentDungeon->peekDungeon('l');

		if (m_facing == 'r') {
			p->getSprite()->setScaleX(p->getSprite()->getScaleX() * -1);
			m_facing = 'l';
		}
	}
	else if (keyCode == RIGHT_KEY) {
		m_currentDungeon->peekDungeon('r');

		if (m_facing == 'l') {
			p->getSprite()->setScaleX(p->getSprite()->getScaleX() * -1);
			m_facing = 'r';
		}
	}
	else if (keyCode == UP_KEY) {
		m_currentDungeon->peekDungeon('u');
	}
	else if (keyCode == DOWN_KEY) {
		m_currentDungeon->peekDungeon('d');
	}
	else if (keyCode == QUICK_KEY) {
		if (m_currentDungeon->getPlayer()->hasItems())
			m_currentDungeon->callUse(0);

		m_currentDungeon->peekDungeon('-');
	}
	else if (keyCode == CAST_KEY) {
		m_currentDungeon->peekDungeon(WIND_UP);
	}
	else if (keyCode == ACTIVE_KEY) {
		m_currentDungeon->peekDungeon('b');
	}
	else if (keyCode == INTERACT_KEY) {
		m_currentDungeon->peekDungeon('e');

		if (playerAdvanced(currentLevel)) {
			advanceLevel();
			return;
		}
	}
	else if (keyCode == INVENTORY_KEY) {
		disableListeners();
		m_hud->inventoryMenu();
		return;
	}
	else if (keyCode == WEAPON_KEY) {
		m_currentDungeon->peekDungeon('w');
	}
	else if (keyCode == ITEM_KEY) {
		disableListeners();
		m_hud->itemMenu();
		return;
	}
	else if (keyCode == KeyType::KEY_M) {
		if (cocos2d::experimental::AudioEngine::getVolume(m_musicID) > 0)
			cocos2d::experimental::AudioEngine::setVolume(m_musicID, 0.0);
		else
			cocos2d::experimental::AudioEngine::setVolume(m_musicID, GLOBAL_MUSIC_VOLUME);
	}
	else if (keyCode == PAUSE_KEY || keyCode == KeyType::KEY_ESCAPE) {
		pauseMenu();
		return;
	}
	else if (keyCode == KeyType::KEY_EQUAL) {
		m_currentDungeon->zoomInBy(0.02f);
	}
	else if (keyCode == KeyType::KEY_MINUS) {
		m_currentDungeon->zoomOutBy(0.02f);
	}
	else {
		m_currentDungeon->peekDungeon('-');
	}

	// Check if player return to menu from World Hub
	if (m_currentDungeon->returnedToMenu())
		return;

	m_hud->updateHUD();

	if (m_currentDungeon->getLevel() == FIRST_BOSS)
		m_hud->updateBossHUD();

	if (m_currentDungeon->getPlayer()->isDead()) {
		cocos2d::experimental::AudioEngine::stop(m_musicID);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}

	if (m_currentDungeon->getPlayer()->getWin()) {
		m_hud->winner();
		return; // prevents timer from being scheduled
	}

	scheduleTimer();
}
void LevelScene::LevelButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	LevelKeyPressed(gameplayButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void LevelScene::scheduleTimer() {
	if (m_preventTimerScheduling) {
		m_preventTimerScheduling = false;
		return;
	}

	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();
		
		// Finish any sprite actions that are allowed to finish early
		while (actions->getNumberOfRunningActionsByTag(1) > 0)
			actions->update(1.0, 1);

		m_currentDungeon->peekDungeon('-');

		m_hud->updateHUD();

		if (m_currentDungeon->getPlayer()->isDead())
			m_hud->gameOver();
		
	}, this, getTimerSpeed(), false, "Timer");
}
void LevelScene::unscheduleTimer() {
	Director::getInstance()->getScheduler()->unschedule("Timer", this);
}

void LevelScene::callFactoryTileCreation() {
	// Prevents multiple instances of this callback occurring if used by cloned Players.
	if (cocos2d::Director::getInstance()->getScheduler()->isScheduled("Factory Tile Timer", this))
		return;

	disableListeners();
	unscheduleTimer();

	m_preventTimerScheduling = true;

	int x = p->getPosX();
	int y = p->getPosY();

	static bool isPlacingTile = true;
	static char dir = 'r';

	std::string image;
	switch (dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}

	cocos2d::Sprite* sprite = m_currentDungeon->createSprite(x, y, y + Z_FLOATERS, image);
	runAction(cocos2d::Follow::create(sprite));

	Director::getInstance()->getScheduler()->schedule([this, sprite](float) {
		isPlacingTile = true;
		sprite->removeFromParent();

		cocos2d::Director::getInstance()->getScheduler()->unschedule("Factory Tile Timer", this);
		runAction(cocos2d::Follow::create(p->getSprite()));
		createGameplayListeners();
		scheduleTimer();
	}, this, 10.0f, false, "Factory Tile Timer");

	createKeyboardEventListener(CC_CALLBACK_2(LevelScene::factoryTileKeyPressed, this, sprite, isPlacingTile, dir), sprite);
	createControllerEventListener(CC_CALLBACK_3(LevelScene::factoryTileButtonPressed, this, sprite, isPlacingTile, dir), sprite);
}
void LevelScene::factoryTileKeyPressed(KeyType keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir) {
	auto actions = getActionManager();
	while (actions->getNumberOfRunningActionsByTag(1) > 0)
		actions->update(1.0, 1);

	cocos2d::Vec2 pos = event->getCurrentTarget()->getPosition();
	Coords dungeonCoords = m_currentDungeon->transformSpriteToDungeonCoordinates(pos);
	float duration = 0.08f;

	if (keyCode == LEFT_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(-1, 0));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else {
			dir = 'l';
		}
	}
	else if (keyCode == RIGHT_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(1, 0));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'r';
	}
	else if (keyCode == UP_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(0, -1));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'u';
	}
	else if (keyCode == DOWN_KEY) {
		if (isPlacingTile) {
			cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(dungeonCoords + Coords(0, 1));
			auto move = cocos2d::MoveTo::create(duration, finalPos);
			move->setTag(1);
			event->getCurrentTarget()->runAction(move);
		}
		else
			dir = 'd';
	}
	else if (keyCode == KeyType::KEY_SPACE) {
		if (!(m_currentDungeon->wall(dungeonCoords.x, dungeonCoords.y) || m_currentDungeon->trap(dungeonCoords.x, dungeonCoords.y))) {
			playInterfaceSound("Confirm 1.mp3");

			if (isPlacingTile) {
				isPlacingTile = false;			
			}
			else {
				isPlacingTile = true;
				m_currentDungeon->addTrap(std::make_shared<FactoryTile>(*m_currentDungeon, dungeonCoords.x, dungeonCoords.y, dir));
			}
		}
		else {
			playSound("Insufficient_Funds.mp3");
		}
	}
	else if (keyCode == KeyType::KEY_ESCAPE) {
		if (!isPlacingTile)
			isPlacingTile = true;		
	}

	if (keyCode == LEFT_KEY || keyCode == RIGHT_KEY || keyCode == UP_KEY || keyCode == DOWN_KEY) {
		if (!isPlacingTile) {	
			playInterfaceSound("Confirm 1.mp3");

			std::string image;
			switch (dir) {
			case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
			case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
			case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
			case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
			}
			target->setSpriteFrame(image);
		}
	}
}
void LevelScene::factoryTileButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event, cocos2d::Sprite* target, bool &isPlacingTile, char &dir) {
	factoryTileKeyPressed(gameplayButtonToKey(static_cast<ButtonType>(keyCode)), event, target, isPlacingTile, dir);
}

void LevelScene::createLightEntity() {
	m_lighting = LightEffect::create();
	m_lighting->retain();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	Vec3 lightPos(vsw, vsh, 1000);
	m_lighting->setLightPos(lightPos);
	m_lighting->setLightCutoffRadius(500000);
	//m_lighting->setLightHalfRadius(5000);
	m_lighting->setBrightness(1.0);
	//m_lighting->setLightColor(Color3B(200, 170, 200));
	m_lighting->setAmbientLightColor(cocos2d::Color3B(0, 0, 0));
}
void LevelScene::setCurrentDungeon(int level, std::shared_ptr<Player> player) {
	switch (level) {
	case TUTORIAL: m_currentDungeon = new TutorialFloor(this, player); break;
	case WORLD_HUB: m_currentDungeon = new WorldHub(this, player); break;

	case FIRST_FLOOR: m_currentDungeon = new FirstFloor(this, player); break;
	case SECOND_FLOOR: m_currentDungeon = new SecondFloor(this, player); break;
	case THIRD_FLOOR: m_currentDungeon = new ThirdFloor(this, player); break;
	case FOURTH_FLOOR: m_currentDungeon = new FourthFloor(this, player); break;
	case FIFTH_FLOOR: m_currentDungeon = new FifthFloor(this, player); break;
	case SIXTH_FLOOR: m_currentDungeon = new SixthFloor(this, player); break;
	case SEVENTH_FLOOR: m_currentDungeon = new SeventhFloor(this, player); break;
	case EIGHTH_FLOOR: m_currentDungeon = new EighthFloor(this, player); break;
	case NINTH_FLOOR: m_currentDungeon = new NinthFloor(this, player); break;

	case FIRST_BOSS: m_currentDungeon = new FirstBoss(this, player); break;

	case FIRST_SHOP:
	//case SECOND_SHOP:
	case THIRD_SHOP:
	case FOURTH_SHOP: m_currentDungeon = new Shop(this, player, level); break;

	case FIRST_SHRINE:
	case SECOND_SHRINE:
	case THIRD_SHRINE:
	case FOURTH_SHRINE: m_currentDungeon = new Shrine(this, player, level); break;
	}

	player->setDungeon(m_currentDungeon);
}
void LevelScene::createPlayerSpriteAndCamera() {
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	cocos2d::Vec2 pos = m_currentDungeon->transformDungeonToSpriteCoordinates(p->getPos());

	p->setSprite(m_currentDungeon->createSprite(p->getPos(), getHalfTileYOffset(), p->getPosY() + Z_ACTOR, p->getImageName()));
	
	m_lighting->setLightPos(Vec3(pos.x, pos.y, 5));

	switch (m_currentDungeon->getLevel()) {
	case FIRST_BOSS:
		this->setScale(0.4f);
		break;

	default:
		m_follow = cocos2d::Follow::create(p->getSprite());
		this->runAction(m_follow);
		break;
	}
}
void LevelScene::setMusic(int level) {
	//cocos2d::experimental::AudioEngine::setMaxAudioInstance(32);

	std::string music;
	switch (level) {
	case FIRST_FLOOR: music = "Exploring a cave.mp3"; break;
	case FIRST_SHRINE: music = "mistical.mp3"; break;
	case SECOND_FLOOR: music = "Tower of Lava.mp3"; break;
	case FIRST_SHOP: music = "mistical.mp3"; break;
	case THIRD_FLOOR: music = "Who turned off the lights.mp3"; break;
	case FIRST_BOSS: music = "Zero Respect.mp3"; break;
	}

	m_musicID = playMusic(music, true);
}
void LevelScene::updateLevelLighting() {
	switch (m_currentDungeon->getLevel()) {
	case FIRST_BOSS: break;
	default: m_currentDungeon->updateLighting(); break;
	}

	// hide rooms if third level
	if (m_currentDungeon->getLevel() == THIRD_FLOOR) {
		auto third = dynamic_cast<ThirdFloor*>(m_currentDungeon);
		third->hideRooms();
	}
}
float LevelScene::getTimerSpeed() {
	float speed;
	switch (m_currentDungeon->getLevel()) {
	case FIRST_FLOOR: speed = 0.70f; break;
	case SECOND_FLOOR: speed = 0.65f; break;
	case THIRD_FLOOR: speed = 0.60f; break;
	case FOURTH_FLOOR: speed = 0.55f; break;
	case FIFTH_FLOOR: speed = 0.50f; break;
	case SIXTH_FLOOR: speed = 0.45f; break;
	case SEVENTH_FLOOR: speed = 0.40f; break;
	case EIGHTH_FLOOR: speed = 0.35f; break;
	case FIRST_BOSS: speed = 0.30f; break;
	default: speed = 0.70f; break;
	}

	speed += m_currentDungeon->getPlayer()->getTimerReduction();
	return speed;
}

void LevelScene::pauseMenu() {
	Director::getInstance()->getScheduler()->pauseTarget(this);
	GameTimers::pauseAllGameTimers();

	auto pauseMenuScene = PauseMenuScene::createScene(this);

	Director::getInstance()->pushScene(pauseMenuScene);
}
void LevelScene::advanceLevel() {
	// Unschedule all timers and event listeners
	unscheduleTimer();
	GameTimers::pauseAllGameTimers();
	disableListeners();

	cocos2d::experimental::AudioEngine::stop(m_musicID);
	
	auto nextScene = LevelScene::createScene(m_currentDungeon->getPlayer(), m_currentDungeon->getLevel());

	auto transition = TransitionFade::create(0.75f, nextScene);

	transition->setOnExitCallback([nextScene]() {
		auto levelScene = dynamic_cast<LevelScene*>(nextScene->getChildByName("Level Scene"));
		levelScene->scheduleTimer();
		GameTimers::resumeAllGameTimers();
	});

	Director::getInstance()->replaceScene(transition);
}
inline bool LevelScene::playerAdvanced(int level) { 
	return m_currentDungeon->getLevel() != level;
};
void LevelScene::returnToMainMenu() {
	// Unschedule the inaction timer and event listener
	unscheduleTimer();
	GameTimers::removeAllGameTimers();
	_eventDispatcher->removeEventListener(kbListener);
	_eventDispatcher->removeEventListener(controllerListener);

	// stop music
	cocos2d::experimental::AudioEngine::stopAll();

	// advance to start menu scene
	auto startScene = StartScene::createScene();

	auto transition = TransitionFade::create(3.0f, startScene);
	Director::getInstance()->replaceScene(transition);
}
void LevelScene::showShopHUD(int x, int y) {
	// x and y are below the item's coordinates,
	// We want the price to show two above that.

	cocos2d::Vec2 finalPos = m_currentDungeon->transformDungeonToSpriteCoordinates(Coords(x, y - 2));

	// pricing symbols, prices themselves, etc.
	m_itemPrice = Label::createWithTTF("$", TEXT_FONT, 24);
	m_itemPrice->setPosition(finalPos);
	m_itemPrice->setColor(cocos2d::Color3B(255, 215, 0));
	m_itemPrice->setString("$" + std::to_string(m_currentDungeon->tileAt(Coords(x, y - 1)).price));
	this->addChild(m_itemPrice, Z_HUD_LABEL);
}
void LevelScene::deconstructShopHUD() {
	if (m_itemPrice != nullptr) {
		m_itemPrice->removeFromParent();
		m_itemPrice = nullptr;
	}
}


//		PAUSE MENU SCENE
PauseMenuScene::PauseMenuScene(LevelScene *levelScene) : m_levelScene(levelScene) {
	id = levelScene->getMusicId();
}

Scene* PauseMenuScene::createScene(LevelScene *levelScene) {
	auto scene = Scene::create();

	// calls init()
	auto layer = PauseMenuScene::create(levelScene);
	scene->addChild(layer);

	return scene;
}
PauseMenuScene* PauseMenuScene::create(LevelScene *levelScene) {
	PauseMenuScene *pRet = new(std::nothrow) PauseMenuScene(levelScene);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool PauseMenuScene::init() {
	if (!Scene::init())
		return false;
	
	cocos2d::experimental::AudioEngine::pauseAll();


	//       PAUSE
	//    |----------|
	// -> |  Resume  |
	//    | Restart  |
	//    |  Options |
	//    |   Help   |
	//    | Main Menu|
	//    | Exit Game|
	//    |----------|


	// Menu border
	addSprite(0.0f, 0.0f, 2, "Pause_Menu_Border_Red.png", "Pause Menu");
	sprites.find("Pause Menu")->second->setScale(0.3f);
	sprites.find("Pause Menu")->second->setOpacity(170);

	// Pause option
	addLabel(0.0f, 4.8f, fetchMenuText("Pause Menu", "Paused"), "Paused", 48);

	// Resume option
	addLabel(0.0f, 2.5f, fetchMenuText("Pause Menu", "Resume"), "Resume", 36);

	// Restart option
	addLabel(0.0f, 1.5f, fetchMenuText("Pause Menu", "Restart"), "Restart", 36);

	// Options
	addLabel(0.0f, 0.5f, fetchMenuText("Pause Menu", "Options"), "Options", 36);

	// "How to play" option
	addLabel(0.0f, -0.5f, fetchMenuText("Pause Menu", "Help"), "Help", 36);

	// Back to Menu option
	addLabel(0.0f, -1.5f, fetchMenuText("Pause Menu", "Main Menu"), "Main Menu", 36);

	// Quit option
	addLabel(0.0f, -2.5f, fetchMenuText("Pause Menu", "Exit Game"), "Exit Game", 36);


	auto selector = createSelectorSprite(-2.0f, 2.5f);
	createKeyboardEventListener(CC_CALLBACK_2(PauseMenuScene::pauseMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(PauseMenuScene::pauseMenuButtonPressed, this), selector);

	return true;
}

void PauseMenuScene::pauseMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	switch (keyCode) {
	case KeyType::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0) {
			index = 5;
			event->getCurrentTarget()->setPosition(pos.x, -2.5 * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_DOWN_ARROW: {
		if (index < 5) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 5) {
			index = 0;
			event->getCurrentTarget()->setPosition(pos.x, 2.5 * MENU_SPACING + vsh);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
		switch (index) {
			// Resume
		case 0: { 
			resumeGame();
			return;
		}
			// Restart
		case 1: { 
			restartGame(*m_levelScene->getCurrentDungeon()->getPlayer());
			return;
		}
			// Options	
		case 2: {
			playInterfaceSound("Confirm 1.mp3");

			m_forward = true;
			options();

			break;
		}
			// Help menu
		case 3: { 
			playInterfaceSound("Confirm 1.mp3");

			m_forward = true;
			helpScreen();
			break;
		}
			// Main Menu
		case 4: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			GameTimers::removeAllGameTimers();
			playInterfaceSound("Confirm 1.mp3");

			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene);
			return;
		}
			// Exit Game
		case 5: {
			playInterfaceSound("Confirm 1.mp3");

			Director::getInstance()->end();
			return;
		}
		}
		break;
	//case KeyType::KEY_P:
	case KeyType::KEY_ESCAPE: {
		resumeGame();
		return;
	}
	default:
		if (keyCode == PAUSE_KEY) {
			resumeGame();
			return;
		}
	}
}
void PauseMenuScene::pauseMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	pauseMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}

void PauseMenuScene::resumeGame() {
	index = 0;
	cocos2d::experimental::AudioEngine::resumeAll();
	cocos2d::Director::getInstance()->popScene();
	cocos2d::Director::getInstance()->getScheduler()->resumeTarget(m_levelScene);
	GameTimers::resumeAllGameTimers();
}

void PauseMenuScene::helpScreen() {
	setPrevMenuState();

	removeAll();

	// Menu
	addSprite(0, 0, 2, "Pause_Menu_Border_Red.png", "Menu");
	sprites.find("Menu")->second->setScale(0.4f);
	sprites.find("Menu")->second->setOpacity(170);

	// Go back
	addLabel(0.0f, -5.2f, fetchMenuText("Help Menu", "OK"), "OK", 36);

	// HOW TO PLAY
	addLabel(0.0f, 5.0f, fetchMenuText("Help Menu", "How to play"), "Help", 48);


	// Movement
	addLabel(-5.0f, 2.85f, fetchMenuText("Help Menu", "Movement"), "Movement", 28);

	// Arrow keys
	addSprite(0.0f, 3.35f, 4, "KB_Arrows_U.png", "Arrow Up");
	sprites.find("Arrow Up")->second->setScale(0.8f);

	addSprite(0.0f, 2.6f, 4, "KB_Arrows_LDR.png", "Arrows");
	sprites.find("Arrows")->second->setScale(0.8f);


	// Interact
	addLabel(-5.0f, 1.5f, fetchMenuText("Help Menu", "Interact"), "Interact", 28);

	addSprite(0.0f, 1.5f, 4, "KB_Black_E.png", "Interact Key");
	sprites.find("Interact Key")->second->setScale(0.8f);


	// Quick Use key
	addLabel(-5.0f, 0.5f, fetchMenuText("Help Menu", "Use Item"), "Item", 28);

	addSprite(0.0f, 0.5f, 4, "KB_Black_Q.png", "Item Key");
	sprites.find("Item Key")->second->setScale(0.8f);


	// Active Item
	addLabel(-5.0f, -0.5f, fetchMenuText("Help Menu", "Use Active Item"), "Active", 28);

	addLabel(0.0f, -0.5f, fetchMenuText("Help Menu", "Space"), "Space", 28);
	labels.find("Space")->second->setLocalZOrder(5);

	addSprite(0.0f, -0.5f, 4, "KB_Black_Space.png", "Active Key");
	sprites.find("Active Key")->second->setScale(0.8f);


	// Switch weapon
	addLabel(-5.0f, -1.5f, fetchMenuText("Help Menu", "Switch Weapon"), "Weapon", 28);

	addSprite(0.0f, -1.5f, 4, "KB_Black_W.png", "Weapon Key");
	sprites.find("Weapon Key")->second->setScale(0.8f);


	// Open/close item menu
	addLabel(-5.0f, -2.5f, fetchMenuText("Help Menu", "Open/Close Item Menu"), "Item Menu", 28);

	addSprite(0.0f, -2.5f, 4, "KB_Black_C.png", "Item Menu Key");
	sprites.find("Item Menu Key")->second->setScale(0.8f);


	// View inventory
	addLabel(-5.0f, -3.5f, fetchMenuText("Help Menu", "Check Inventory"), "Inventory", 28);

	addSprite(0.0f, -3.5f, 4, "KB_Black_Tab.png", "Inventory Key");
	sprites.find("Inventory Key")->second->setScale(0.8f);
	

	auto selector = createSelectorSprite(-2.0f, -5.2f);
	createKeyboardEventListener(CC_CALLBACK_2(PauseMenuScene::helpMenuKeyPressed, this), selector);
	createControllerEventListener(CC_CALLBACK_3(PauseMenuScene::helpMenuButtonPressed, this), selector);
}
void PauseMenuScene::helpMenuKeyPressed(KeyType keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case KeyType::KEY_ENTER:
	case KeyType::KEY_SPACE:
	case KeyType::KEY_ESCAPE: {
		playInterfaceSound("Confirm 1.mp3");

		removeAll();

		init();

		m_forward = false;
		restorePrevMenuState();
	}
	default: break;
	}
}
void PauseMenuScene::helpMenuButtonPressed(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
	helpMenuKeyPressed(menuButtonToKey(static_cast<ButtonType>(keyCode)), event);
}


// Converts Key Code to string for use in labels
std::string convertKeycodeToStr(KeyType keyCode) {
	std::string key;
	switch (keyCode) {
	case KeyType::KEY_NONE:
		key = "None";
		break;
	case KeyType::KEY_PAUSE:
		key = "Pause";
		break;
	case KeyType::KEY_SCROLL_LOCK:
		key = "Scroll Lock";
		break;
	case KeyType::KEY_PRINT:
		key = "Print";
		break;
	case KeyType::KEY_SYSREQ:
		key = "SysReq";
		break;
	case KeyType::KEY_BREAK:
		key = "Break";
		break;
	case KeyType::KEY_ESCAPE:
		key = "Escape";
		break;
	case KeyType::KEY_BACKSPACE:
		key = "Backspace";
		break;
	case KeyType::KEY_TAB:
		key = "Tab";
		break;
	case KeyType::KEY_BACK_TAB:
		key = "Back Tab";
		break;
	case KeyType::KEY_RETURN:
		key = "Return";
		break;
	case KeyType::KEY_CAPS_LOCK:
		key = "Caps Lock";
		break;
	case KeyType::KEY_LEFT_SHIFT:
		key = "Left Shift";
		break;
	case KeyType::KEY_RIGHT_SHIFT:
		key = "Right Shift";
		break;
	case KeyType::KEY_LEFT_CTRL:
		key = "Left Ctrl";
		break;
	case KeyType::KEY_RIGHT_CTRL:
		key = "Right Ctrl";
		break;
	case KeyType::KEY_LEFT_ALT:
		key = "Left Alt";
		break;
	case KeyType::KEY_RIGHT_ALT:
		key = "Right Alt";
		break;
	case KeyType::KEY_MENU:
		key = "Menu";
		break;
	case KeyType::KEY_HYPER:
		key = "Hyper";
		break;
	case KeyType::KEY_INSERT:
		key = "Insert";
		break;
	case KeyType::KEY_HOME:
		key = "Home";
		break;
	case KeyType::KEY_PG_UP:
		key = "Page Up";
		break;
	case KeyType::KEY_DELETE:
		key = "Delete";
		break;
	case KeyType::KEY_END:
		key = "End";
		break;
	case KeyType::KEY_PG_DOWN:
		key = "Page Down";
		break;
	case KeyType::KEY_LEFT_ARROW:
		key = "Left Arrow";
		break;
	case KeyType::KEY_RIGHT_ARROW:
		key = "Right Arrow";
		break;
	case KeyType::KEY_UP_ARROW:
		key = "Up Arrow";
		break;
	case KeyType::KEY_DOWN_ARROW:
		key = "Down Arrow";
		break;
	case KeyType::KEY_NUM_LOCK:
		key = "Num Lock";
		break;
	case KeyType::KEY_KP_PLUS:
		key = "Keypad Plus";
		break;
	case KeyType::KEY_KP_MINUS:
		key = "Keypad Minus";
		break;
	case KeyType::KEY_KP_MULTIPLY:
		key = "Keypad Multiply";
		break;
	case KeyType::KEY_KP_DIVIDE:
		key = "Keypad Divide";
		break;
	case KeyType::KEY_KP_ENTER:
		key = "Keypad Enter";
		break;
	case KeyType::KEY_KP_HOME:
		key = "Keypad Home";
		break;
	case KeyType::KEY_KP_UP:
		key = "Keypad Up";
		break;
	case KeyType::KEY_KP_PG_UP:
		key = "Keypad Page Up";
		break;
	case KeyType::KEY_KP_LEFT:
		key = "Keypad Left";
		break;
	case KeyType::KEY_KP_FIVE:
		key = "Keypad 5";
		break;
	case KeyType::KEY_KP_RIGHT:
		key = "Keypad Right";
		break;
	case KeyType::KEY_KP_END:
		key = "Keypad End";
		break;
	case KeyType::KEY_KP_DOWN:
		key = "Keypad Down";
		break;
	case KeyType::KEY_KP_PG_DOWN:
		key = "Keypad Page Down";
		break;
	case KeyType::KEY_KP_INSERT:
		key = "Keypad Insert";
		break;
	case KeyType::KEY_KP_DELETE:
		key = "Keypad Delete";
		break;
	case KeyType::KEY_F1:
		key = "F1";
		break;
	case KeyType::KEY_F2:
		key = "F2";
		break;
	case KeyType::KEY_F3:
		key = "F3";
		break;
	case KeyType::KEY_F4:
		key = "F4";
		break;
	case KeyType::KEY_F5:
		key = "F5";
		break;
	case KeyType::KEY_F6:
		key = "F6";
		break;
	case KeyType::KEY_F7:
		key = "F7";
		break;
	case KeyType::KEY_F8:
		key = "F8";
		break;
	case KeyType::KEY_F9:
		key = "F9";
		break;
	case KeyType::KEY_F10:
		key = "F10";
		break;
	case KeyType::KEY_F11:
		key = "F11";
		break;
	case KeyType::KEY_F12:
		key = "F12";
		break;
	case KeyType::KEY_SPACE:
		key = "Space";
		break;
	case KeyType::KEY_EXCLAM:
		key = "!";
		break;
	case KeyType::KEY_QUOTE:
		key = "\"";
		break;
	case KeyType::KEY_NUMBER:
		key = "#";
		break;
	case KeyType::KEY_DOLLAR:
		key = "$";
		break;
	case KeyType::KEY_PERCENT:
		key = "%";
		break;
	case KeyType::KEY_CIRCUMFLEX:
		key = "^";
		break;
	case KeyType::KEY_AMPERSAND:
		key = "&";
		break;
	case KeyType::KEY_APOSTROPHE:
		key = "Apostrophe";
		break;
	case KeyType::KEY_LEFT_PARENTHESIS:
		key = "(";
		break;
	case KeyType::KEY_RIGHT_PARENTHESIS:
		key = ")";
		break;
	case KeyType::KEY_ASTERISK:
		key = "*";
		break;
	case KeyType::KEY_PLUS:
		key = "+";
		break;
	case KeyType::KEY_COMMA:
		key = ",";
		break;
	case KeyType::KEY_MINUS:
		key = "-";
		break;
	case KeyType::KEY_PERIOD:
		key = ".";
		break;
	case KeyType::KEY_SLASH:
		key = "/";
		break;
	case KeyType::KEY_0:
		key = "0";
		break;
	case KeyType::KEY_1:
		key = "1";
		break;
	case KeyType::KEY_2:
		key = "2";
		break;
	case KeyType::KEY_3:
		key = "3";
		break;
	case KeyType::KEY_4:
		key = "4";
		break;
	case KeyType::KEY_5:
		key = "5";
		break;
	case KeyType::KEY_6:
		key = "6";
		break;
	case KeyType::KEY_7:
		key = "7";
		break;
	case KeyType::KEY_8:
		key = "8";
		break;
	case KeyType::KEY_9:
		key = "9";
		break;
	case KeyType::KEY_COLON:
		key = ":";
		break;
	case KeyType::KEY_SEMICOLON:
		key = ";";
		break;
	case KeyType::KEY_LESS_THAN:
		key = "<";
		break;
	case KeyType::KEY_EQUAL:
		key = "=";
		break;
	case KeyType::KEY_GREATER_THAN:
		key = ">";
		break;
	case KeyType::KEY_QUESTION:
		key = "?";
		break;
	case KeyType::KEY_AT:
		key = "@";
		break;
	case KeyType::KEY_CAPITAL_A:
		key = "A";
		break;
	case KeyType::KEY_CAPITAL_B:
		key = "B";
		break;
	case KeyType::KEY_CAPITAL_C:
		key = "C";
		break;
	case KeyType::KEY_CAPITAL_D:
		key = "D";
		break;
	case KeyType::KEY_CAPITAL_E:
		key = "E";
		break;
	case KeyType::KEY_CAPITAL_F:
		key = "F";
		break;
	case KeyType::KEY_CAPITAL_G:
		key = "G";
		break;
	case KeyType::KEY_CAPITAL_H:
		key = "H";
		break;
	case KeyType::KEY_CAPITAL_I:
		key = "I";
		break;
	case KeyType::KEY_CAPITAL_J:
		key = "J";
		break;
	case KeyType::KEY_CAPITAL_K:
		key = "K";
		break;
	case KeyType::KEY_CAPITAL_L:
		key = "L";
		break;
	case KeyType::KEY_CAPITAL_M:
		key = "M";
		break;
	case KeyType::KEY_CAPITAL_N:
		key = "N";
		break;
	case KeyType::KEY_CAPITAL_O:
		key = "O";
		break;
	case KeyType::KEY_CAPITAL_P:
		key = "P";
		break;
	case KeyType::KEY_CAPITAL_Q:
		key = "Q";
		break;
	case KeyType::KEY_CAPITAL_R:
		key = "R";
		break;
	case KeyType::KEY_CAPITAL_S:
		key = "S";
		break;
	case KeyType::KEY_CAPITAL_T:
		key = "T";
		break;
	case KeyType::KEY_CAPITAL_U:
		key = "U";
		break;
	case KeyType::KEY_CAPITAL_V:
		key = "V";
		break;
	case KeyType::KEY_CAPITAL_W:
		key = "W";
		break;
	case KeyType::KEY_CAPITAL_X:
		key = "X";
		break;
	case KeyType::KEY_CAPITAL_Y:
		key = "Y";
		break;
	case KeyType::KEY_CAPITAL_Z:
		key = "Z";
		break;
	case KeyType::KEY_LEFT_BRACKET:
		key = "[";
		break;
	case KeyType::KEY_BACK_SLASH:
		key = "\\";
		break;
	case KeyType::KEY_RIGHT_BRACKET:
		key = "]";
		break;
	case KeyType::KEY_UNDERSCORE:
		key = "_";
		break;
	case KeyType::KEY_GRAVE:
		key = "`";
		break;
	case KeyType::KEY_A:
		key = "A";
		break;
	case KeyType::KEY_B:
		key = "B";
		break;
	case KeyType::KEY_C:
		key = "C";
		break;
	case KeyType::KEY_D:
		key = "D";
		break;
	case KeyType::KEY_E:
		key = "E";
		break;
	case KeyType::KEY_F:
		key = "F";
		break;
	case KeyType::KEY_G:
		key = "G";
		break;
	case KeyType::KEY_H:
		key = "H";
		break;
	case KeyType::KEY_I:
		key = "I";
		break;
	case KeyType::KEY_J:
		key = "J";
		break;
	case KeyType::KEY_K:
		key = "K";
		break;
	case KeyType::KEY_L:
		key = "L";
		break;
	case KeyType::KEY_M:
		key = "M";
		break;
	case KeyType::KEY_N:
		key = "N";
		break;
	case KeyType::KEY_O:
		key = "O";
		break;
	case KeyType::KEY_P:
		key = "P";
		break;
	case KeyType::KEY_Q:
		key = "Q";
		break;
	case KeyType::KEY_R:
		key = "R";
		break;
	case KeyType::KEY_S:
		key = "S";
		break;
	case KeyType::KEY_T:
		key = "T";
		break;
	case KeyType::KEY_U:
		key = "U";
		break;
	case KeyType::KEY_V:
		key = "V";
		break;
	case KeyType::KEY_W:
		key = "W";
		break;
	case KeyType::KEY_X:
		key = "X";
		break;
	case KeyType::KEY_Y:
		key = "Y";
		break;
	case KeyType::KEY_Z:
		key = "Z";
		break;
	case KeyType::KEY_LEFT_BRACE:
		key = "{";
		break;
	case KeyType::KEY_BAR:
		key = "Bar";
		break;
	case KeyType::KEY_RIGHT_BRACE:
		key = "}";
		break;
	case KeyType::KEY_TILDE:
		key = "`";
		break;
	case KeyType::KEY_EURO:
		key = "Euro";
		break;
	case KeyType::KEY_POUND:
		key = "Pound";
		break;
	case KeyType::KEY_YEN:
		key = "Yen";
		break;
	case KeyType::KEY_MIDDLE_DOT:
		key = "Middle Dot";
		break;
	case KeyType::KEY_SEARCH:
		key = "Search";
		break;
	case KeyType::KEY_DPAD_LEFT:
		key = "Dpad Left";
		break;
	case KeyType::KEY_DPAD_RIGHT:
		key = "Dpad Right";
		break;
	case KeyType::KEY_DPAD_UP:
		key = "Dpad Up";
		break;
	case KeyType::KEY_DPAD_DOWN:
		key = "Dpad Down";
		break;
	case KeyType::KEY_DPAD_CENTER:
		key = "Dpad Center";
		break;
	case KeyType::KEY_ENTER:
		key = "Enter";
		break;
	case KeyType::KEY_PLAY:
		key = "Play";
		break;
	}

	return key;
}
std::string convertButtonToStr(ButtonType button) {
	switch (button) {
	case ButtonType::BUTTON_A:
		return "A";
	case ButtonType::BUTTON_B:
		return "B";
	case ButtonType::BUTTON_C:
		return "C";
	case ButtonType::BUTTON_X:
		return "X";
	case ButtonType::BUTTON_Y:
		return "Y";
	case ButtonType::BUTTON_Z:
		return "Z";
	case ButtonType::BUTTON_LEFT_SHOULDER:
		return "Left Trigger";
	case ButtonType::BUTTON_RIGHT_SHOULDER:
		return "Right Trigger";
	case ButtonType::BUTTON_DPAD_UP:
		return "Dpad Up";
	case ButtonType::BUTTON_DPAD_DOWN:
		return "Dpad Down";
	case ButtonType::BUTTON_DPAD_LEFT:
		return "Dpad Left";
	case ButtonType::BUTTON_DPAD_RIGHT:
		return "Dpad Right";
	case ButtonType::BUTTON_START:
		return "Start";
	case ButtonType::BUTTON_SELECT:
		return "Select";
	default:
		return "UNKNOWN";
	}
}

/*
bool LevelScene::isKeyPressed(KeyType code) {
	// Check if the key is currently pressed by seeing it it's in the std::map keys
	// In retrospect, keys is a terrible name for a key/value paried datatype isnt it?
	if (keys.find(code) != keys.end())
		return true;
	return false;
}

double LevelScene::keyPressedDuration(KeyType code) {
	if (!isKeyPressed(KeyType::KEY_LEFT_ARROW))
		return 0;  // Not pressed, so no duration obviously

	// Return the amount of time that has elapsed between now and when the user
	// first started holding down the key in milliseconds
	// Obviously the start time is the value we hold in our std::map keys
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::high_resolution_clock::now() - keys[code]).count();
}

void LevelScene::update(float delta) {
	// Register an update function that checks to see if the CTRL key is pressed
	// and if it is displays how long, otherwise tell the user to press it
	Node::update(delta);
	if (isKeyPressed(KeyType::KEY_LEFT_ARROW)) {
		std::stringstream ss;
		ss << "Control key has been pressed for " <<
			keyPressedDuration(KeyType::KEY_LEFT_ARROW) << " ms";
		label->setString(ss.str().c_str());
	}
	else
		label->setString("Press the CTRL Key");
}
>>>>>>> 2989379ff3214d5b91cd4c670e60b6aee17c666b
*/