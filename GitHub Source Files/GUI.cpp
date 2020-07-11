#include "GUI.h"
#include "AppDelegate.h"
#include "AudioEngine.h"
#include "ui/cocosGUI.h"
#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "FX.h"
#include "LightEffect.h"
#include "EffectSprite.h"
#include <cstdlib>
#include <cmath>
#include <vector>

// 1366x768
int X_OFFSET = 560;
int Y_OFFSET = 170;
float SPACING_FACTOR = 51.1f;// 60;
int MENU_SPACING = 60;
float RES_ADJUST = 1.08f;
float HP_BAR_ADJUST = -2;
float HP_ADJUST = 0;
float SP_ADJUST = 0;

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
//int X_OFFSET = 560;// 840;
//int Y_OFFSET = 170;// 255;
//float SPACING_FACTOR = 36.0f;
//int MENU_SPACING = 45;
//float RES_ADJUST = 1.5f;
//float HP_BAR_ADJUST = -130;
//float HP_ADJUST = -14;
//float SP_ADJUST = -14;

USING_NS_CC;

// Default Controls
cocos2d::EventKeyboard::KeyCode UP_KEY = EventKeyboard::KeyCode::KEY_UP_ARROW;
cocos2d::EventKeyboard::KeyCode DOWN_KEY = EventKeyboard::KeyCode::KEY_DOWN_ARROW;
cocos2d::EventKeyboard::KeyCode LEFT_KEY = EventKeyboard::KeyCode::KEY_LEFT_ARROW;
cocos2d::EventKeyboard::KeyCode RIGHT_KEY = EventKeyboard::KeyCode::KEY_RIGHT_ARROW;
cocos2d::EventKeyboard::KeyCode INTERACT_KEY = EventKeyboard::KeyCode::KEY_E;
cocos2d::EventKeyboard::KeyCode QUICK_KEY = EventKeyboard::KeyCode::KEY_Q;
cocos2d::EventKeyboard::KeyCode ACTIVE_KEY = EventKeyboard::KeyCode::KEY_SPACE;
cocos2d::EventKeyboard::KeyCode WEAPON_KEY = EventKeyboard::KeyCode::KEY_W;
cocos2d::EventKeyboard::KeyCode CAST_KEY = EventKeyboard::KeyCode::KEY_S;
cocos2d::EventKeyboard::KeyCode ITEM_KEY = EventKeyboard::KeyCode::KEY_C;
cocos2d::EventKeyboard::KeyCode INVENTORY_KEY = EventKeyboard::KeyCode::KEY_TAB;


MenuScene::MenuScene() {

	// Fill resolution vector
	resolutions.push_back(std::make_pair(1280, 720));
	resolutions.push_back(std::make_pair(1366, 768));
	resolutions.push_back(std::make_pair(1600, 900));
	resolutions.push_back(std::make_pair(1920, 1080));
}
Scene* MenuScene::createScene()
{
	return Scene::create();
}
MenuScene* MenuScene::create()
{
	MenuScene *pRet = new(std::nothrow) MenuScene();
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
bool MenuScene::init() {
	if (!Scene::init())
		return false;

	return true;
}

void MenuScene::options() {
	index = 0;
	removeAll(); // remove previous labels and sprites

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 3.0f * MENU_SPACING + visibleSize.height / 2);
	arrow->setScale(2.5f);
	arrow->setColor(cocos2d::Color3B(255, 150, 200));
	this->addChild(arrow, 4);
	sprites.insert(std::make_pair("arrow", arrow));

	// Sound
	addLabel(visibleSize.width / 2, 3.0f * MENU_SPACING + visibleSize.height / 2, "Volume Adjust", "Sound", 40);

	// Screen Resolution
	addLabel(visibleSize.width / 2, 0.0f * MENU_SPACING + visibleSize.height / 2, "Screen Resolution", "Res", 40);
	//addLabel(-2.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2, "Lower", "Res Lower", 30);
	addLabel(0.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2, std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second), "Screen Resolution", 30);
	addLabel(0.0f * MENU_SPACING + visibleSize.width / 2, -1.9f * MENU_SPACING + visibleSize.height / 2, "Fullscreen", "Fullscreen", 30);

	// Key Bindings
	addLabel(visibleSize.width / 2, -3.0f * MENU_SPACING + visibleSize.height / 2, "Key Bindings", "Keys", 40);

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(MenuScene::optionKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, arrow);
}
void MenuScene::optionKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 
	// 0 : Sound
	// 1 : Resolution
	// 2 : Key Bindings

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index == 3) {
			if (resolutionIndex > 0) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex--;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index == 3) {
			if (resolutionIndex < (int)resolutions.size() - 1) {
				playInterfaceSound("Select 1.mp3");
				resolutionIndex++;
				labels.find("Screen Resolution")->second->setString(std::to_string(resolutions[resolutionIndex].first) + " x " + std::to_string(resolutions[resolutionIndex].second));
			}
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}
		else if (index == 4) {
			playInterfaceSound("Select 1.mp3");
			index = 3;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.0f * MENU_SPACING);
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}
		else if (index == 3) {
			playInterfaceSound("Select 1.mp3");
			index = 4;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.0f * MENU_SPACING);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
			// Sound select
		case 0:
			soundOptions();

			break;
			// Resolution select
		case 1:
			index = 3;
			event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2);

			break;
			// Key Bindings select
		case 2:
			keyBindings();

			break;
			// Choose Resolution
		case 3: {
			adjustResolution();
			break;
		}
		case 4: {
			toggleFullscreen();
			break;
		}
		}

		return;

	case EventKeyboard::KeyCode::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		// Screen Resolution
		if (index == 3 || index == 4) {
			index = 1;
			event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 0.0f * MENU_SPACING + visibleSize.height / 2);
		}
		else {
			index = 0;

			removeAll();
			init();
		}

		return;

	default: break;
	}
}

void MenuScene::soundOptions() {
	index = 0;
	removeAll(); // remove previous labels and sprites

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 2.1f * MENU_SPACING + visibleSize.height / 2);
	arrow->setScale(2.5f);
	arrow->setColor(cocos2d::Color3B(255, 150, 200));
	this->addChild(arrow, 4);
	sprites.insert(std::make_pair("arrow", arrow));

	// Sound FX Volume
	addLabel(visibleSize.width / 2, 3.0f * MENU_SPACING + visibleSize.height / 2, "Sound Effect Volume", "Sound", 40);
	addLabel(-1.0f * MENU_SPACING + visibleSize.width / 2, 2.1f * MENU_SPACING + visibleSize.height / 2, "Lower", "Sound Lower", 30);
	addLabel(0.0f * MENU_SPACING + visibleSize.width / 2, 2.1f * MENU_SPACING + visibleSize.height / 2, std::to_string((int)(GLOBAL_SOUND_VOLUME * 100)), "Sound Volume", 30);
	addLabel(1.0f * MENU_SPACING + visibleSize.width / 2, 2.1f * MENU_SPACING + visibleSize.height / 2, "Higher", "Sound Higher", 30);

	// Music Volume
	addLabel(visibleSize.width / 2, 0.0f * MENU_SPACING + visibleSize.height / 2, "Music Volume", "Music", 40);
	addLabel(-1.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2, "Lower", "Music Lower", 30);
	addLabel(0.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2, std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100)), "Music Volume", 30);
	addLabel(1.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2, "Higher", "Music Higher", 30);

	// UI Volume
	addLabel(visibleSize.width / 2, -3.0f * MENU_SPACING + visibleSize.height / 2, "UI Volume", "UI", 40);
	addLabel(-1.0f * MENU_SPACING + visibleSize.width / 2, -3.9f * MENU_SPACING + visibleSize.height / 2, "Lower", "UI Lower", 30);
	addLabel(0.0f * MENU_SPACING + visibleSize.width / 2, -3.9f * MENU_SPACING + visibleSize.height / 2, std::to_string((int)(GLOBAL_UI_VOLUME * 100)), "UI Volume", 30);
	addLabel(1.0f * MENU_SPACING + visibleSize.width / 2, -3.9f * MENU_SPACING + visibleSize.height / 2, "Higher", "UI Higher", 30);

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(MenuScene::soundOptionsKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, arrow);
}
void MenuScene::soundOptionsKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// Index key:
	// 
	// 0, 3 : Sound
	// 1, 4 : Music
	// 2, 6 : Resolution

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_SOUND_VOLUME -= 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.0f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_MUSIC_VOLUME -= 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.0f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME > 0.0f) {
				playInterfaceSound("Select 1.mp3");
				GLOBAL_UI_VOLUME -= 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.0f)));
			}
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index == 0) {
			if (GLOBAL_SOUND_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_SOUND_VOLUME += 0.05f;
				labels.find("Sound Volume")->second->setString(std::to_string((int)(GLOBAL_SOUND_VOLUME * 100.0f)));
			}
		}
		else if (index == 1) {
			if (GLOBAL_MUSIC_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_MUSIC_VOLUME += 0.05f;
				labels.find("Music Volume")->second->setString(std::to_string((int)(GLOBAL_MUSIC_VOLUME * 100.0f)));
				cocos2d::experimental::AudioEngine::setVolume(id, GLOBAL_MUSIC_VOLUME);
			}
		}
		else if (index == 2) {
			if (GLOBAL_UI_VOLUME < 1.0f) {
				playInterfaceSound("Select 1.mp3");

				GLOBAL_UI_VOLUME += 0.05f;
				labels.find("UI Volume")->second->setString(std::to_string((int)(GLOBAL_UI_VOLUME * 100.0f)));
			}
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0 && index <= 2) {
			playInterfaceSound("Select 1.mp3");
			index--;

			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3.0f * MENU_SPACING);
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 2) {
			playInterfaceSound("Select 1.mp3");
			index++;

			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3.0f * MENU_SPACING);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		//playInterfaceSound("Confirm 1.mp3");

		//switch (index) {
		//	// Sound select
		//case 0:
		//	index = 3;
		//	event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + visibleSize.width / 2, 2.1f * MENU_SPACING + visibleSize.height / 2);

		//	break;
		//	// Music select
		//case 1:
		//	index = 4;
		//	event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + visibleSize.width / 2, -0.9f * MENU_SPACING + visibleSize.height / 2);

		//	break;
		//	// UI select
		//case 2:
		//	index = 5;
		//	event->getCurrentTarget()->setPosition(-2.0f * MENU_SPACING + visibleSize.width / 2, -3.9f * MENU_SPACING + visibleSize.height / 2);

		//	break;

		//case 3:
		//	index = 0;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 3.0f * MENU_SPACING + visibleSize.height / 2);

		//	break;
		//case 4:
		//	index = 1;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 0.0f * MENU_SPACING + visibleSize.height / 2);

		//	break;
		//case 5:
		//	index = 2;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, -3.0f * MENU_SPACING + visibleSize.height / 2);

		//	break;
		//}

		return;

	case EventKeyboard::KeyCode::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");

		//// Sound
		//if (index == 3) {
		//	index = 0;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 3.0f * MENU_SPACING + visibleSize.height / 2);
		//}
		//// Music
		//else if (index == 4) {
		//	index = 1;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, 0.0f * MENU_SPACING + visibleSize.height / 2);
		//}
		//// UI
		//else if (index == 5) {
		//	index = 2;
		//	event->getCurrentTarget()->setPosition(-3.5f * MENU_SPACING + visibleSize.width / 2, -3.0f * MENU_SPACING + visibleSize.height / 2);
		//}
		//else {
		index = 0;

		removeAll();
		options();
		//}

		return;

	default: break;
	}
}

void MenuScene::keyBindings() {
	index = 0;
	removeAll(); // remove previous labels and sprites

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();


	//    HOW TO PLAY
	//    |---------|
	//    | Controls|
	//    |   ...   |
	//    |         |
	// -> |   OK    |
	//    |---------|


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0 + visibleSize.width / 2, 0 + visibleSize.height / 2);
	box->setScale(.3f);
	box->setOpacity(170);
	sprites.insert(std::make_pair("Box", box));

	// Selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-3.0 * MENU_SPACING + visibleSize.width / 2, 3.6 * MENU_SPACING + visibleSize.height / 2);
	this->addChild(sprite, 3);
	sprite->setScale(2.5f);
	sprites.insert(std::make_pair("arrow", sprite));

	// HOW TO PLAY
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 5.0 * MENU_SPACING + visibleSize.height / 2, "Change Keys", "Change Keys", 48);

	/*auto pause = Label::createWithTTF("How to play", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 5.0 * MENU_SPACING);
	this->addChild(pause, 3);*/

	// Movement
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 3.6 * MENU_SPACING + visibleSize.height / 2, "Up:", "Up", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 3.6 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(UP_KEY), "Up Key", 28);

	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 2.9 * MENU_SPACING + visibleSize.height / 2, "Down:", "Down", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 2.9 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(DOWN_KEY), "Down Key", 28);

	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 2.2 * MENU_SPACING + visibleSize.height / 2, "Left:", "Left", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 2.2 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(LEFT_KEY), "Left Key", 28);

	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 1.5 * MENU_SPACING + visibleSize.height / 2, "Right:", "Right", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 1.5 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(RIGHT_KEY), "Right Key", 28);


	/*auto restart = Label::createWithTTF("Movement:", "fonts/Marker Felt.ttf", 28);
	restart->setPosition(-5 * MENU_SPACING, 2.85 * MENU_SPACING);
	restart->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(restart, 3);*/

	// new arrows
	/*auto arrowKeyUp = Sprite::create("KB_Arrows_U.png");
	arrowKeyUp->setPosition(0 * MENU_SPACING, 3.35*MENU_SPACING);
	arrowKeyUp->setScale(0.8f);
	this->addChild(arrowKeyUp, 4);

	auto arrowKeys = Sprite::create("KB_Arrows_LDR.png");
	arrowKeys->setPosition(0 * MENU_SPACING, 2.6*MENU_SPACING);
	arrowKeys->setScale(0.8f);
	this->addChild(arrowKeys, 4);*/


	// Interact
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 0.8 * MENU_SPACING + visibleSize.height / 2, "Interact:", "Interact", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 0.8 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(INTERACT_KEY), "Interact Key", 28);

	/*auto uselabel = Label::createWithTTF("Grab/Interact:", "fonts/Marker Felt.ttf", 28);
	uselabel->setPosition(-5 * MENU_SPACING, 1.5 * MENU_SPACING);
	uselabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(uselabel, 3);

	auto use = Sprite::create("KB_Black_E.png");
	use->setPosition(0 * MENU_SPACING, 1.5 * MENU_SPACING);
	use->setScale(0.8f);
	this->addChild(use, 4);*/


	// Use item
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, 0.1 * MENU_SPACING + visibleSize.height / 2, "Quick Item Use:", "Quick", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, 0.1 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(QUICK_KEY), "Quick Key", 28);

	/*auto item = Label::createWithTTF("Use Item:", "fonts/Marker Felt.ttf", 28);
	item->setPosition(-5 * MENU_SPACING, 0.5 * MENU_SPACING);
	item->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(item, 3);

	auto itemKey = Sprite::create("KB_Black_Q.png");
	itemKey->setPosition(0 * MENU_SPACING, 0.5 * MENU_SPACING);
	itemKey->setScale(0.8f);
	this->addChild(itemKey, 4);*/


	// Active Item
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, -0.6 * MENU_SPACING + visibleSize.height / 2, "Active Item:", "Active", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, -0.6 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(ACTIVE_KEY), "Active Key", 28);

	/*auto shield = Label::createWithTTF("Active Item:", "fonts/Marker Felt.ttf", 28);
	shield->setPosition(-5 * MENU_SPACING, -0.5 * MENU_SPACING);
	shield->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(shield, 3);

	auto spacebar = Label::createWithTTF("SPACE", "fonts/Marker Felt.ttf", 28);
	spacebar->setPosition(0 * MENU_SPACING, -0.5 * MENU_SPACING);
	spacebar->setTextColor(cocos2d::Color4B(255, 255, 255, 255));
	this->addChild(spacebar, 5);

	auto shieldKey = Sprite::create("KB_Black_Space.png");
	shieldKey->setPosition(0 * MENU_SPACING, -0.5 * MENU_SPACING);
	shieldKey->setScale(0.8f);
	this->addChild(shieldKey, 4);*/


	// Swap weapon
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, -1.3 * MENU_SPACING + visibleSize.height / 2, "Switch Weapon:", "Weapon", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, -1.3 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(WEAPON_KEY), "Weapon Key", 28);

	/*auto weplabel = Label::createWithTTF("Switch Weapon:", "fonts/Marker Felt.ttf", 28);
	weplabel->setPosition(-5 * MENU_SPACING, -1.5 * MENU_SPACING);
	weplabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(weplabel, 3);

	auto wepmenu = Sprite::create("KB_Black_W.png");
	wepmenu->setPosition(0 * MENU_SPACING, -1.5 * MENU_SPACING);
	wepmenu->setScale(0.8f);
	this->addChild(wepmenu, 4);*/


	// Cast weapon
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, -2.0 * MENU_SPACING + visibleSize.height / 2, "Cast Weapon:", "Cast", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, -2.0 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(CAST_KEY), "Cast Key", 28);


	// Open/close item menu
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, -2.7 * MENU_SPACING + visibleSize.height / 2, "Item Menu:", "Item", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, -2.7 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(ITEM_KEY), "Item Key", 28);

	/*auto itemlabel = Label::createWithTTF("Open/Close Item Menu:", "fonts/Marker Felt.ttf", 28);
	itemlabel->setPosition(-5 * MENU_SPACING, -2.5* MENU_SPACING);
	itemlabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(itemlabel, 3);

	auto itemmenu = Sprite::create("KB_Black_C.png");
	itemmenu->setPosition(0 * MENU_SPACING, -2.5 * MENU_SPACING);
	itemmenu->setScale(0.8f);
	this->addChild(itemmenu, 4);*/


	// View inventory
	addLabel(-5 * MENU_SPACING + visibleSize.width / 2, -3.4 * MENU_SPACING + visibleSize.height / 2, "Inventory:", "Inventory", 28);
	addLabel(0 * MENU_SPACING + visibleSize.width / 2, -3.4 * MENU_SPACING + visibleSize.height / 2, convertKeycodeToStr(INVENTORY_KEY), "Inventory Key", 28);

	/*auto inventory = Label::createWithTTF("Check inventory:", "fonts/Marker Felt.ttf", 28);
	inventory->setPosition(-5 * MENU_SPACING, -3.5 * MENU_SPACING);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(inventory, 3);

	auto inventoryKey = Sprite::create("KB_Black_Tab.png");
	inventoryKey->setPosition(0 * MENU_SPACING, -3.5 * MENU_SPACING);
	inventoryKey->setScale(0.8f);
	this->addChild(inventoryKey, 4);*/

	// Reset to Defaults
	addLabel(0 + visibleSize.width / 2, -5.2 * MENU_SPACING + visibleSize.height / 2, "Reset to Default", "Default", 36);
	/*auto resume = Label::createWithTTF("OK", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0 + visibleSize.width / 2, -5.2 * MENU_SPACING + visibleSize.height / 2);
	this->addChild(resume, 3);*/

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(MenuScene::keyBindingsKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, sprite);
}
void MenuScene::keyBindingsKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	if (settingKey) {
		setKey(keyCode, index);

		// If the key was set successfully, move the arrow back
		if (!settingKey)
			event->getCurrentTarget()->setPosition(pos.x - 1.2 * MENU_SPACING, pos.y);

		return;
	}

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0 && index <= 10) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 11) {
			index = 10;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.8f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 10) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 0.7f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 10) {
			index = 11;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.8f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:

		playInterfaceSound("Confirm 1.mp3");

		if (index == 11) {
			resetBindings();
			return;
		}

		if (!settingKey) {
			settingKey = true;
			event->getCurrentTarget()->setPosition(pos.x + 1.2 * MENU_SPACING, pos.y);
		}

		break;

	case EventKeyboard::KeyCode::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		index = 0;
		removeAll();

		options();
		break;

	default:
		break;
	}
}
void MenuScene::setKey(cocos2d::EventKeyboard::KeyCode keyCode, int index) {

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
	default: break;
	}

	playInterfaceSound("Confirm 1.mp3");
	controls.clear();
}
bool MenuScene::keyIsValid(cocos2d::EventKeyboard::KeyCode keyCode, int index) {

	controls.push_back(UP_KEY);
	controls.push_back(DOWN_KEY);
	controls.push_back(LEFT_KEY);
	controls.push_back(RIGHT_KEY);
	controls.push_back(INTERACT_KEY);
	controls.push_back(QUICK_KEY);
	controls.push_back(ACTIVE_KEY);
	controls.push_back(WEAPON_KEY);
	controls.push_back(CAST_KEY);
	controls.push_back(ITEM_KEY);
	controls.push_back(INVENTORY_KEY);

	for (unsigned int i = 0; i < controls.size(); i++) {

		if (i == index)
			continue;

		if (controls[i] == keyCode)
			return false;
	}

	// P, M, I, and Escape key are off limits
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE ||
		keyCode == cocos2d::EventKeyboard::KeyCode::KEY_P ||
		keyCode == cocos2d::EventKeyboard::KeyCode::KEY_M ||
		keyCode == cocos2d::EventKeyboard::KeyCode::KEY_I)
		return false;

	settingKey = false;
	return true;
}
void MenuScene::resetBindings() {
	UP_KEY = EventKeyboard::KeyCode::KEY_UP_ARROW;
	DOWN_KEY = EventKeyboard::KeyCode::KEY_DOWN_ARROW;
	LEFT_KEY = EventKeyboard::KeyCode::KEY_LEFT_ARROW;
	RIGHT_KEY = EventKeyboard::KeyCode::KEY_RIGHT_ARROW;
	INTERACT_KEY = EventKeyboard::KeyCode::KEY_E;
	QUICK_KEY = EventKeyboard::KeyCode::KEY_Q;
	ACTIVE_KEY = EventKeyboard::KeyCode::KEY_SPACE;
	WEAPON_KEY = EventKeyboard::KeyCode::KEY_W;
	CAST_KEY = EventKeyboard::KeyCode::KEY_S;
	ITEM_KEY = EventKeyboard::KeyCode::KEY_C;
	INVENTORY_KEY = EventKeyboard::KeyCode::KEY_TAB;

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

	static cocos2d::Size designResolutionSize = cocos2d::Size(1920, 1080);
	//static cocos2d::Size designResolutionSize = cocos2d::Size(1600, 900);
	//static cocos2d::Size designResolutionSize = cocos2d::Size(1366, 768);
	//static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 720);

	cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
	cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
	cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

	auto director = Director::getInstance();
	auto glview = dynamic_cast<GLViewImpl*>(director->getOpenGLView());
	std::pair<int, int> res = resolutions[resolutionIndex];

	glview->setFrameSize(res.first, res.second);
	glview->setDesignResolutionSize(res.first, res.second, ResolutionPolicy::NO_BORDER);


	//cocos2d::Size designResolutionSize = glview->getDesignResolutionSize();
	auto frameSize = glview->getFrameSize();
	// if the frame's height is larger than the height of medium size.
	if (frameSize.height > mediumResolutionSize.height)
	{
		director->setContentScaleFactor(MIN(largeResolutionSize.height / designResolutionSize.height, largeResolutionSize.width / designResolutionSize.width));
	}
	// if the frame's height is larger than the height of small size.
	else if (frameSize.height > smallResolutionSize.height)
	{
		director->setContentScaleFactor(MIN(mediumResolutionSize.height / designResolutionSize.height, mediumResolutionSize.width / designResolutionSize.width));
	}
	// if the frame's height is smaller than the height of medium size.
	else
	{
		director->setContentScaleFactor(MIN(smallResolutionSize.height / designResolutionSize.height, smallResolutionSize.width / designResolutionSize.width));
	}
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

void MenuScene::addLabel(float x, float y, std::string name, std::string id, float fontSize) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto label = Label::createWithTTF(name, "fonts/Marker Felt.ttf", fontSize);
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
void MenuScene::removeAll() {
	removeLabels();
	removeSprites();
};
void MenuScene::removeLabels() {
	for (auto &it : labels) {
		it.second->removeFromParent();
	}
	labels.clear();
};
void MenuScene::removeSprites() {
	for (auto &it : sprites) {
		it.second->removeFromParent();
	}
	sprites.clear();
};


//		START SCENE
Scene* StartScene::createScene()
{
	return StartScene::create();
}

static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

bool StartScene::init()
{
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
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// Title
	auto title1 = Label::createWithTTF("Lost Arts", "fonts/Marker Felt.ttf", 72);
	title1->setPosition(Vec2(visibleSize.width / 2, (visibleSize.height / 2) + (2.5 * MENU_SPACING)));
	title1->setTextColor(cocos2d::Color4B(224, 224, 224, 255));
	title1->enableOutline(cocos2d::Color4B(50, 55, 55, 255), 1);
	this->addChild(title1, 1);
	labels.insert(std::make_pair("title", title1));

	// background pic
	auto background = Sprite::create("super_pixel_cave_wallpaper_C.png");
	background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	float scale = 0.50f * RES_ADJUST;
	background->setScale(scale);
	this->addChild(background, -5);

	// arrow sprite for selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-2.5f * MENU_SPACING + visibleSize.width / 2, -0.25f * MENU_SPACING + visibleSize.height / 2);
	arrow->setScale(2.5);
	arrow->setColor(cocos2d::Color3B(255, 150, 200));
	this->addChild(arrow, 4);
	sprites.insert(std::make_pair("arrow", arrow));

	// Start
	auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
	float x = visibleSize.width / 2;
	float y = visibleSize.height / 2 - (0.25 * MENU_SPACING);
	start->setPosition(Vec2(x, y));
	start->setScale(0.8f);
	this->addChild(start, 3);
	sprites.insert(std::make_pair("start", start));

	// Options
	x = visibleSize.width / 2;
	y = visibleSize.height / 2 - 1.75 * MENU_SPACING;
	addLabel(x, y, "Options", "Options", 52);

	// Exit
	auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButton1.png");
	x = visibleSize.width / 2;
	y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
	exitGame->setPosition(Vec2(x, y));
	this->addChild(exitGame, 3);
	sprites.insert(std::make_pair("exitGame", exitGame));

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::keyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, arrow); // check this for player


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
void StartScene::keyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 2) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.5f * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Start
			playInterfaceSound("Confirm 1.mp3");
			index = 0;

			/*auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - (1.2 * MENU_SPACING);
			start->setPosition(Vec2(x, y));
			start->setScale(0.8f);
			this->addChild(start, 3);
			sprites.insert(std::make_pair("start", start));*/

			characterSelect();

			return;
		}
				// Options
		case 1:
			playInterfaceSound("Confirm 1.mp3");
			index = 0;

			options();

			return;
		case 2: // Exit
			playInterfaceSound("Confirm 1.mp3");
			index = 0;

			auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButtonPressed1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - 3.25 * MENU_SPACING;
			exitGame->setPosition(Vec2(x, y));
			this->addChild(exitGame, 3);

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

	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto character = Label::createWithTTF("Select Character", "fonts/Marker Felt.ttf", 48);
	character->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + (4 * MENU_SPACING)));
	character->setTextColor(cocos2d::Color4B(224, 224, 224, 255));
	character->enableOutline(cocos2d::Color4B(50, 55, 55, 255), 1);
	this->addChild(character, 1);
	labels.insert(std::make_pair("character", character));

	// Adventurer
	auto p1 = Sprite::createWithSpriteFrameName("Player1_48x48.png");
	p1->setPosition(visibleSize.width / 2, visibleSize.height / 2 - (1.4 * MENU_SPACING));
	p1->setScale(1.5f);
	//p1->setColor(cocos2d::Color3B(255, 150, 200));
	this->addChild(p1, 1);
	sprites.insert(std::make_pair("p1", p1));

	// Spellcaster
	auto p2 = Sprite::createWithSpriteFrameName("Spellcaster_48x48.png");
	p2->setPosition((2.5f * MENU_SPACING) + visibleSize.width / 2, visibleSize.height / 2);
	p2->setScale(0.8f);
	this->addChild(p2, 1);
	sprites.insert(std::make_pair("p2", p2));

	// 
	auto p3 = Sprite::createWithSpriteFrameName("Madman_48x48.png");
	p3->setPosition(visibleSize.width / 2, visibleSize.height / 2 + (1.4f * MENU_SPACING));
	p3->setScale(0.8f);
	this->addChild(p3, 1);
	sprites.insert(std::make_pair("p3", p3));

	auto p4 = Sprite::createWithSpriteFrameName("Spelunker_48x48.png");
	p4->setPosition(-2.5f * MENU_SPACING + visibleSize.width / 2, visibleSize.height / 2);
	p4->setScale(0.8f);
	this->addChild(p4, 1);
	sprites.insert(std::make_pair("p4", p4));

	displayCharacterInfo();

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(StartScene::characterKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, character); // check this for player
}
void StartScene::characterKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index > 0)
			index--;
		else
			index = 3;

		rotate(false);

		playInterfaceSound("Select 1.mp3");
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index < 3)
			index++;
		else
			index = 0;
	
		rotate(true);

		playInterfaceSound("Select 1.mp3");
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		playInterfaceSound("Confirm 1.mp3");

		switch (index) {
		case 0:
			characterID = ID_ADVENTURER;
			break;
		case 1:
			characterID = ID_SPELLCASTER;
			break;
		case 2:
			characterID = ID_MADMAN;
			break;
		case 3:
			characterID = ID_SPELUNKER;
			break;
		}

		startGameCallback(this);

		index = 0;
		return;
		
	case EventKeyboard::KeyCode::KEY_ESCAPE:
		playInterfaceSound("Confirm 1.mp3");
		index = 0;

		removeAll();
		init();

		return;

	default: break;
	}
}
void StartScene::rotate(bool clockwise) {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	int scalingFactor = 1;

	if (clockwise)
		scalingFactor = -1;

	const double pi = 4 * std::atan(1);
	double x, y, a, b, r, theta;
	r = 90;
	a = 1.55;
	b = 0.85;

	cocos2d::Vector<FiniteTimeAction*> vec1, vec2, vec3, vec4;

	for (int i = 0; i < 90; i++) {
		// Bottom (selected) character
		theta = degree1 * pi / 180.0; // Convert to radians
		x = r * a * cos(theta) + visibleSize.width / 2;
		y = r * b * sin(theta) + visibleSize.height / 2;
		vec1.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree1 += scalingFactor);

		// Right character
		theta = degree2 * pi / 180.0;
		x = r * a * cos(theta) + visibleSize.width / 2;
		y = r * b * sin(theta) + visibleSize.height / 2;
		vec2.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree2 += scalingFactor);

		// top background character
		theta = degree3 * pi / 180.0;
		x = r * a * cos(theta) + visibleSize.width / 2;
		y = r * b * sin(theta) + visibleSize.height / 2;
		vec3.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree3 += scalingFactor);

		// Left character
		theta = degree4 * pi / 180.0;
		x = r * a * cos(theta) + visibleSize.width / 2;
		y = r * b * sin(theta) + visibleSize.height / 2;
		vec4.pushBack(cocos2d::MoveTo::create(0.001f, cocos2d::Vec2(x, y)));
		setAngle(degree4 += scalingFactor);
	}

	const float largeScaling = 1.6f;
	const float smallScaling = 0.75f;

	cocos2d::Sequence* seq1 = cocos2d::Sequence::create(vec1);
	cocos2d::Sequence* seq2 = cocos2d::Sequence::create(vec2);
	cocos2d::Sequence* seq3 = cocos2d::Sequence::create(vec3);
	cocos2d::Sequence* seq4 = cocos2d::Sequence::create(vec4);

	sprites.find("p1")->second->runAction(seq1);
	sprites.find("p1")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 0 ? largeScaling : smallScaling));
	sprites.find("p2")->second->runAction(seq2);
	sprites.find("p2")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 1 ? largeScaling : smallScaling));

	sprites.find("p3")->second->runAction(seq3);
	sprites.find("p3")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 2 ? largeScaling : smallScaling));
	sprites.find("p4")->second->runAction(seq4);
	sprites.find("p4")->second->runAction(cocos2d::ScaleTo::create(0.1f, index == 3 ? largeScaling : smallScaling));

	displayCharacterInfo();
}
void StartScene::displayCharacterInfo() {
	if (labels.find("Character Name") == labels.end()) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		addLabel(visibleSize.width / 2, visibleSize.height / 2 - (2.6f * MENU_SPACING), "The Adventurer", "Character Name", 28);
		addLabel(visibleSize.width / 2, visibleSize.height / 2 - (3.4f * MENU_SPACING), "Sword and Shield", "Character Desc", 20);
		return;
	}

	switch (index) {
	case 0: {
		updateLabel("Character Name", "The Adventurer");
		updateLabel("Character Desc", "Sword and Shield");
		break;
	}
	case 1: {
		updateLabel("Character Name", "The Spellcaster");
		updateLabel("Character Desc", "Not-So-Master of the Arts");
		break;
	}
	case 2: {
		updateLabel("Character Name", "The Madman");
		updateLabel("Character Desc", "He's actually done it");
		break;
	}
	case 3: {
		updateLabel("Character Name", "The Spelunker");
		updateLabel("Character Desc", "Seasoned Explorer");
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
	degree1 = 270;
	degree2 = 0;
	degree3 = 90;
	degree4 = 180;

	// If 5 characters
}

void StartScene::startGameCallback(Ref* pSender)
{
	playInterfaceSound("Confirm 1.mp3");
	cocos2d::experimental::AudioEngine::stop(id);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// advance to next scene
	std::shared_ptr<Player> p = nullptr;

	if (characterID == ID_ADVENTURER) {
		p = std::make_shared<Adventurer>();
	}
	else if (characterID == ID_SPELLCASTER) {
		p = std::make_shared<Spellcaster>();
	}
	else if (characterID == ID_MADMAN) {
		p = std::make_shared<TheMadman>();
	}
	else if (characterID == ID_SPELUNKER) {
		p = std::make_shared<Spelunker>();
	}
	else if (characterID == ID_ACROBAT) {
		p = std::make_shared<Adventurer>();
	}
	else {
		p = std::make_shared<Adventurer>();
	}
	
	auto level1Scene = LevelScene::createScene(p, 1);
	//auto level1Scene = ShopScene::createScene(p, 4);
	level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	// switch to next scene
	Director::getInstance()->replaceScene(level1Scene);
	//Director::getInstance()->replaceScene(TransitionCrossFade::create(0.0f, level1Scene));
	//Director::getInstance()->getRunningScene()->runAction(cocos2d::MoveTo::create(0.0f, Vec2(visibleSize.width / 2, visibleSize.height / 2)));

}
void StartScene::exitGameCallback(Ref* pSender)
{
	auto audio = experimental::AudioEngine::play2d("Cancel 1.mp3", false, 1.0f);

	//Close the cocos2d-x game scene and quit the application
	Director::getInstance()->end();

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

	//EventCustom customEndEvent("game_scene_close_event");
	//_eventDispatcher->dispatchEvent(&customEndEvent);
}


// HUD LAYER
HUDLayer::HUDLayer(std::shared_ptr<Player> p) : p(p) {
	
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

	//		RENDER HUD
	image = "Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, 2, "healthbar");
	healthbar->setPosition(-400.f * RES_ADJUST + HP_BAR_ADJUST, 300.f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("healthbar", healthbar));

	image = "Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, 3, "hp");
	hp->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	hp->setPosition(-546.f * RES_ADJUST + HP_ADJUST, 300.f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("hp", hp));

	numericalHP = Label::createWithTTF(std::to_string(p->getHP()) + "/" + std::to_string(p->getMaxHP()), "fonts/Marker Felt.ttf", 18);
	numericalHP->setPosition(-400 * RES_ADJUST, 300 * RES_ADJUST); // -160, 300
	//numericalHP->enableGlow(cocos2d::Color4B(250, 128, 114, 200));
	numericalHP->setOpacity(230);
	numericalHP->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	this->addChild(numericalHP, 4);

	str = Label::createWithTTF("Str: +" + std::to_string(p->getStr()), "fonts/Marker Felt.ttf", 16);
	str->setPosition(-570 * RES_ADJUST, 70 * RES_ADJUST);
	str->setOpacity(230);
	str->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	str->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	str->setAdditionalKerning(0.25f);
	this->addChild(str, 4);

	dex = Label::createWithTTF("Dex: +" + std::to_string(p->getDex() + p->getWeapon()->getDexBonus()), "fonts/Marker Felt.ttf", 16);
	dex->setPosition(-570 * RES_ADJUST, 50 * RES_ADJUST);
	dex->setOpacity(230);
	dex->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	dex->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	dex->setAdditionalKerning(0.25f);
	this->addChild(dex, 4);

	intellect = Label::createWithTTF("Int: +" + std::to_string(p->getInt()), "fonts/Marker Felt.ttf", 16);
	intellect->setPosition(-570 * RES_ADJUST, 30 * RES_ADJUST);
	intellect->setOpacity(230);
	intellect->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	intellect->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	intellect->setAdditionalKerning(0.25f);
	this->addChild(intellect, 4);

	armor = Label::createWithTTF("Amr: +" + std::to_string(p->getArmor()), "fonts/Marker Felt.ttf", 16);
	armor->setPosition(-570 * RES_ADJUST, 10 * RES_ADJUST);
	armor->setOpacity(230);
	armor->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	armor->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	armor->setAdditionalKerning(0.25f);
	this->addChild(armor, 4);

	moneyBonus = Label::createWithTTF("Money Bonus : " + std::to_string((int)p->getMoneyBonus()), "fonts/Marker Felt.ttf", 15);
	moneyBonus->setPosition(570.f * RES_ADJUST, -330 * RES_ADJUST);
	moneyBonus->setOpacity(240);
	moneyBonus->setTextColor(cocos2d::Color4B(153, 153, 255, 200));
	//moneyBonus->enableOutline(cocos2d::Color4B(204, 255, 255, 200), 1);
	moneyBonus->setAdditionalKerning(0.25f);
	this->addChild(moneyBonus, 4);

	/*
	//// stats
	//// strength stat
	//Sprite* strBar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
	//this->addChild(strBar, 3, "strength bar");
	//strBar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 120 * RES_ADJUST);
	//strBar->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("strength bar", strBar));

	//Sprite* strbarpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
	//this->addChild(strbarpoints, 4, "strbarpoints");
	//strbarpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	//strbarpoints->setPosition(-583 * RES_ADJUST, 120 * RES_ADJUST);
	//strbarpoints->setColor(cocos2d::Color3B(0, 191, 255));
	//strbarpoints->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("strbarpoints", strbarpoints));

	//// dex stat
	//Sprite* dexbar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
	//this->addChild(dexbar, 3, "dexbar");
	//dexbar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 100 * RES_ADJUST);
	//dexbar->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("dexbar", dexbar));

	//Sprite* dexpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
	//this->addChild(dexpoints, 4, "dexpoints");
	//dexpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	//dexpoints->setPosition(-583 * RES_ADJUST, 100 * RES_ADJUST);
	//dexpoints->setColor(cocos2d::Color3B(0, 191, 255));
	//dexpoints->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("dexpoints", dexpoints));

	//// intellect stat
	//Sprite* intbar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
	//this->addChild(intbar, 3, "intbar");
	//intbar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 80 * RES_ADJUST);
	//intbar->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("intbar", intbar));

	//Sprite* intpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
	//this->addChild(intpoints, 4, "intpoints");
	//intpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	//intpoints->setPosition(-583 * RES_ADJUST, 80 * RES_ADJUST);
	//intpoints->setColor(cocos2d::Color3B(0, 191, 255));
	//intpoints->setScale(1.0 * RES_ADJUST);
	//HUD.insert(std::pair<std::string, Sprite*>("intpoints", intpoints));
	*/


	// weapon
	image = "Current_Weapon_Box_1.png";
	Sprite* wepbox = Sprite::createWithSpriteFrameName(image);
	wepbox->setPosition(-570 * RES_ADJUST, 240 * RES_ADJUST);
	wepbox->setScale(.2f * RES_ADJUST);
	wepbox->setOpacity(160);
	this->addChild(wepbox, 2, "wepbox");
	HUD.insert(std::pair<std::string, Sprite*>("wepbox", wepbox));

	// load default weapon sprite
	std::string weapon;
	weapon = p->getWeapon()->getName();
	image = p->getWeapon()->getImageName();
	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, 3, weapon);
	currentwep->setPosition(-570 * RES_ADJUST, 240 * RES_ADJUST);
	currentwep->setScale(0.6f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));

	// if weapon has casting ability, add label
	if (p->getWeapon()->canBeCast()) {

		// if new weapon previously did not have the cast label, add it
		if (keyLabels.find("cast key") == keyLabels.end()) {
			auto castKey = Label::createWithTTF(convertKeycodeToStr(CAST_KEY), "fonts/Marker Felt.ttf", 14);
			castKey->setPosition(-570.f * RES_ADJUST, 215.5f * RES_ADJUST);
			castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
			castKey->setOpacity(255);
			this->addChild(castKey, 4);
			keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
		}
	}


	//		:::: RIGHT SIDE OF HUD ::::

	// gold count
	image = "Gold_Pile1_48x48.png";
	Sprite* goldpile = Sprite::createWithSpriteFrameName(image);
	this->addChild(goldpile, 3, "goldpile");
	goldpile->setPosition(570 * RES_ADJUST, 300 * RES_ADJUST); // -190, 300
	goldpile->setScale(0.8f * RES_ADJUST);
	goldpile->setOpacity(230);
	HUD.insert(std::pair<std::string, Sprite*>("goldpile", goldpile));

	goldcount = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 18);
	goldcount->setPosition(600 * RES_ADJUST, 300 * RES_ADJUST); // -160, 300
	this->addChild(goldcount, 3);

	return true;
}
void HUDLayer::updateHUD(Dungeon &dungeon) {
	p = dungeon.getPlayer();
	int x = p->getPosX(); int y = p->getPosY();

	// :::: Gold check ::::
	goldcount->setString(std::to_string(p->getMoney()));


	// :::: Weapon check ::::
	std::string weapon = p->getWeapon()->getName();
	std::string image = p->getWeapon()->getImageName();

	// if current weapon equipped is different, switch the weapon sprite
	if (HUD.find("currentwep")->second->getName() != weapon) {
		//remove sprite
		HUD.find("currentwep")->second->removeFromParent();
		HUD.erase("currentwep");

		Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentwep, 3, weapon);
		currentwep->setPosition(-570.f * RES_ADJUST, 240.f * RES_ADJUST);
		currentwep->setScale(0.6f * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));


		// if weapon has casting ability, add label
		if (p->getWeapon()->canBeCast()) {

			// if new weapon previously did not have the cast label, add it
			if (keyLabels.find("cast key") == keyLabels.end()) {
				auto castKey = Label::createWithTTF(convertKeycodeToStr(CAST_KEY), "fonts/Marker Felt.ttf", 14);
				castKey->setPosition(-570.f * RES_ADJUST, 215.5f * RES_ADJUST);
				castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
				castKey->setOpacity(255);
				this->addChild(castKey, 4);
				keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
			}
		}
		// else remove the label if previous weapon could be cast
		else if (!p->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end()) {
			auto it = keyLabels.find("cast key");
			it->second->removeFromParent();
			keyLabels.erase(it);
		}
		
	}
	// If there was a casting label that can be removed because the player used the special, remove it
	else if (!p->getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end()) {
		auto it = keyLabels.find("cast key");
		it->second->removeFromParent();
		keyLabels.erase(it);
	}
	else if (p->getWeapon()->canBeCast()) {
		// if weapon previously did not have the cast label, add it
		if (keyLabels.find("cast key") == keyLabels.end()) {
			auto castKey = Label::createWithTTF(convertKeycodeToStr(CAST_KEY), "fonts/Marker Felt.ttf", 14);
			castKey->setPosition(-570.f * RES_ADJUST, 215.5f * RES_ADJUST);
			castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
			castKey->setOpacity(255);
			this->addChild(castKey, 4);
			keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
		}
	}


	// Update active item
	if (p->hasActiveItem()) {
		std::string name = p->getActiveItem()->getName();

		// If there wasn't an active item equipped previously, construct the menu
		if (HUD.find("Active") == HUD.end()) {
			constructActiveItemHUD();
		}
		// else if current active item is different, switch the sprite
		else if (HUD.find("Active")->second->getName() != name) {
			updateActiveItemHUD();
		}
		// Display stack amount for Spelunker
		else if (p->getName() == SPELUNKER) {
			std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(p);
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


	// :::: Trinket check ::::
	if (p->hasTrinket()) {
		std::string trinket = p->getTrinket()->getName();
		image = p->getTrinket()->getImageName();

		// if there is now a trinket equipped, but there wasn't previously, construct the menu
		if (image != "" && HUD.find("current trinket") == HUD.end()) {
			// Trinket HUD box
			Sprite* trinketbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
			trinketbox->setPosition(-570 * RES_ADJUST, 110 * RES_ADJUST);
			trinketbox->setScale(.2f * RES_ADJUST);
			trinketbox->setOpacity(200);
			trinketbox->setColor(cocos2d::Color3B(200, 20, 0));
			this->addChild(trinketbox, 2, "trinketbox");
			HUD.insert(std::pair<std::string, Sprite*>("trinketbox", trinketbox));

			// trinket sprite
			Sprite* currentTrinket = Sprite::createWithSpriteFrameName(image);
			this->addChild(currentTrinket, 3, trinket);
			currentTrinket->setPosition(-570 * RES_ADJUST, 110 * RES_ADJUST);
			currentTrinket->setScale(0.5);
			HUD.insert(std::pair<std::string, Sprite*>("current trinket", currentTrinket));
		}
		// else if current shield equipped is different, switch the sprite
		else if (image != "" && HUD.find("current trinket")->second->getName() != trinket) {
			// remove sprite
			HUD.find("current trinket")->second->removeFromParent();
			HUD.erase("current trinket");

			Sprite* currentTrinket = Sprite::createWithSpriteFrameName(image);
			this->addChild(currentTrinket, 3, trinket);
			currentTrinket->setPosition(-570 * RES_ADJUST, 110 * RES_ADJUST);
			currentTrinket->setScale(0.50);
			HUD.insert(std::pair<std::string, Sprite*>("current trinket", currentTrinket));
		}
	}
	else {
		// if there's no trinket equipped and there was previously, deconstruct the trinket HUD
		if (image == "" && HUD.find("current trinket") != HUD.end()) {
			deconstructTrinketHUD();
		}
	}
	

	// :::: HP & Stats check ::::
	double x_scale = dungeon.getPlayer()->getHP() / (static_cast<double>(dungeon.getPlayer()->getMaxHP()) * 1.0);
	cocos2d::Action* move = cocos2d::ScaleTo::create(.4f, x_scale, 1);
	auto action = HUD.find("hp")->second->runAction(move);
	action->setTag(5);

	numericalHP->setString(std::to_string(p->getHP()) + "/" + std::to_string(p->getMaxHP()));

	str->setString("Str: +" + std::to_string(p->getStr()));
	dex->setString("Dex: +" + std::to_string(p->getDex() + p->getWeapon()->getDexBonus()));
	intellect->setString("Int: +" + std::to_string(p->getInt()));
	armor->setString("Amr: +" + std::to_string(p->getArmor()));
	moneyBonus->setString("Money Bonus : " + std::to_string((int)p->getMoneyBonus()));


	// :::: Quick access check ::::
	if (p->getItems().size() > 0 && quick == 0)
		quick = 1;

	if (quick == 1 && p->getItems().size() > 0) {
		std::string image = p->getItems().at(0)->getImageName();
		std::string item = p->getItems()[0]->getName();

		// if there weren't any items before, construct menu again
		if (HUD.find("quick access") == HUD.end()) {
			constructItemHUD();
		}
		// else if the item is different, switch the sprite
		else if (HUD.find("quick item") != HUD.end() && HUD.find("quick item")->second->getName() != item) {
			HUD.find("quick item")->second->removeFromParent();
			HUD.erase(HUD.find("quick item"));

			updateItemHUD();
		}

		// Display stack amount for item
		if (p->getItems()[0]->canStack()) {

			// If there was no label for stackable items, add it
			if (keyLabels.find("Stack Amount") == keyLabels.end()) {
				std::shared_ptr<Stackable> stackable = std::dynamic_pointer_cast<Stackable>(p->getItems()[0]);

				auto stack = Label::createWithTTF("x" + std::to_string(stackable->getCount()), "fonts/Marker Felt.ttf", 16);
				stack->setPosition(600.f * RES_ADJUST, 240.f * RES_ADJUST);
				stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
				stack->setOpacity(250);
				this->addChild(stack, 4);
				keyLabels.insert(std::pair<std::string, Label*>("Stack Amount", stack));

				stackable.reset();
			}
		}
		// Otherwise if a stack amount label exists
		if (keyLabels.find("Stack Amount") != keyLabels.end()) {
			
			// Remove it because the new item can't stack, or it's a different item than before
			if (!p->getItems()[0]->canStack() || HUD.find("quick item")->second->getName() != item) {
				keyLabels.find("Stack Amount")->second->removeFromParent();
				keyLabels.erase(keyLabels.find("Stack Amount"));
			}
			// Update the count
			else if (HUD.find("quick item")->second->getName() == item) {
				std::shared_ptr<Stackable> stackable = std::dynamic_pointer_cast<Stackable>(p->getItems()[0]);
				keyLabels.find("Stack Amount")->second->setString("x" + std::to_string(stackable->getCount()));
				stackable.reset();
			}
		}
	}
	else {
		quick = 0;
		// if there's no item quick slotted but there was previously, deconstruct the HUD
		if (HUD.find("quick access") != HUD.end())
			deconstructItemHUD();	
	}
}
void HUDLayer::showShopHUD(Dungeon &dungeon, int x, int y) {
	// pricing symbols, prices themselves, etc.
	itemprice = Label::createWithTTF("$", "fonts/Marker Felt.ttf", 24);
	itemprice->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - y) - Y_OFFSET);
	this->addChild(itemprice, 3);
	itemprice->setColor(cocos2d::Color3B(255, 215, 0));
	itemprice->setString("$" + std::to_string(dungeon[(y-1)*dungeon.getCols() + x].price));
}
void HUDLayer::updateShopHUD() {

}
void HUDLayer::showBossHP() {
	std::string image;

	//		RENDER BOSS HP
	image = "Boss_Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, 2, "bosshealthbar");
	healthbar->setPosition(430, 0);
	HUD.insert(std::pair<std::string, Sprite*>("bosshealthbar", healthbar));

	image = "Boss_Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, 3, "bosshp");
	hp->setAnchorPoint(Vec2(0.5, 0.0)); // set anchor point to left side
	hp->setPosition(430, -134);
	HUD.insert(std::pair<std::string, Sprite*>("bosshp", hp));
}
void HUDLayer::updateBossHUD(Dungeon &dungeon) {
	// if there are still monsters, check for smasher
	if (!dungeon.getMonsters().empty()) {
		// if smasher is still alive, update its hp
		if (dungeon.getMonsters().at(0)->getName() == "Smasher") {
			//	Check Boss HP bar
			float y_scale = dungeon.getMonsters()[0]->getHP() / (static_cast<float>(dungeon.getMonsters()[0]->getMaxHP()) * 1.0);
			cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, 1.0f, y_scale);
			HUD.find("bosshp")->second->runAction(move);
		}
		// else if smasher is dead, deconstruct the hp bar if we haven't already
		else {
			deconstructBossHUD();
		}
	}
	// else if there aren't any monsters, deconstruct the hp bar if we haven't already
	else {
		deconstructBossHUD();
	}
}

void HUDLayer::NPCInteraction(cocos2d::EventListenerKeyboard* listener, NPC &npc, std::vector<std::string> dialogue) {
	// assign the paused listener to HUDLayer
	activeListener = listener;

	// assign dialogue to use
	m_dialogue = dialogue;

	line = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	line->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	line->setOpacity(230);
	line->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	line->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	line->setAdditionalKerning(0.25f);
	this->addChild(line, 4);

	// display first line
	line->setString(m_dialogue[lineIndex]);
	lineIndex++;

	// add event listener for selecting
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::NPCKeyPressed, this, &npc);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, line);
}
void HUDLayer::NPCKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, NPC *npc) {

	// if there's nothing left to say, remove text and restore control
	if (lineIndex >= (int)m_dialogue.size()) {
		experimental::AudioEngine::play2d("Confirm 1.mp3", false, 0.5f);

		lineIndex = 0;
		line->removeFromParent();
		line = nullptr;
		enableListener();
		return;
	}

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_SPACE:
	default: {
		if (!m_dialogue.empty() && lineIndex < (int)m_dialogue.size()) {
			experimental::AudioEngine::play2d("Confirm 1.mp3", false, 0.5f);

			// If this line indicates a prompt, then begin the prompt
			if (m_dialogue[lineIndex] == NPC_PROMPT) {
				lineIndex = 0;
				line->removeFromParent();
				line = nullptr;

				NPCPrompt(*npc, npc->getChoices());

				return;
			}

			line->setString(m_dialogue[lineIndex]);
			lineIndex++;
		}
	}
	}
}

void HUDLayer::NPCPrompt(NPC &npc, std::vector<std::string> choices) {

	// Assign this so that we know how many choices there are
	m_dialogue = choices;

	// menu border
	/*Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);
	weaponMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));*/

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-3 * MENU_SPACING, -3 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.5f);
	generalSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	for (int i = 0; i < (int)choices.size(); i++) {
		auto choice = Label::createWithTTF(choices[i], "fonts/Marker Felt.ttf", 20);
		choice->setPosition(0, (-3 - i) * MENU_SPACING); // The spacing is the only thing that changes
		this->addChild(choice, 5);
		labels.insert(std::pair<std::string, Label*>(choices[i], choice));
	}

	// add event listener for selecting
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::NPCPromptKeyPressed, this, &npc);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void HUDLayer::NPCPromptKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, NPC *npc) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < (int)(m_dialogue.size() - 1)) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:

		playInterfaceSound("Confirm 1.mp3");

		npc->useResponse(index);

		index = 0;
		deconstructMenu(generalSprites);
		
		NPCInteraction(activeListener, *npc, npc->getDialogue());

		return;

	default:
		break;
	}

}

void HUDLayer::devilsWaters(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon) {
	// assign the paused listener to HUDLayer
	activeListener = listener;

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
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);
	menuSprites.insert(std::pair<std::string, Sprite*>("box", box));

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0f);
	menuSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	// Pause option
	auto pause = Label::createWithTTF("The waters give off a strange aura. Take a drink from the fountain?", "fonts/Marker Felt.ttf", 40);
	pause->setPosition(0, 4.8f * MENU_SPACING);
	this->addChild(pause, 3);
	labels.insert(std::pair<std::string, Label*>("pause", pause));

	// NO
	auto no = Label::createWithTTF("NO", "fonts/Marker Felt.ttf", 36);
	no->setPosition(0, 2 * MENU_SPACING);
	this->addChild(no, 3);
	labels.insert(std::pair<std::string, Label*>("no", no));

	// YES
	auto yes = Label::createWithTTF("YES", "fonts/Marker Felt.ttf", 36);
	yes->setPosition(0, -2 * MENU_SPACING);
	this->addChild(yes, 3);
	labels.insert(std::pair<std::string, Label*>("yes", yes));


	// add event listener for selecting
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::devilKeyPressed, this, &dungeon);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void HUDLayer::devilKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // NO
			playInterfaceSound("Confirm 1.mp3");

			index = 0;
			deconstructMenu(menuSprites);

			SecondFloor* second = dynamic_cast<SecondFloor*>(dungeon);
			second->devilsWater(false);

			enableListener();
			return;
		}
		case 1: // YES
			playSound("Devils_Gift.mp3");

			SecondFloor* second = dynamic_cast<SecondFloor*>(dungeon);
			second->devilsWater(true);

			index = 0;
			deconstructMenu(menuSprites);

			enableListener();
			return;
		}
	default:
		break;
	}
	
}

void HUDLayer::inventoryMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon) {
	// assign the paused listener to HUDLayer
	activeListener = listener;

	p = dungeon.getPlayer();
	std::vector<std::shared_ptr<Passive>> passives = p->getPassives();
	std::vector<std::shared_ptr<Drops>> iteminv = p->getItems();

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// INVENTORY
	auto inventory = Label::createWithTTF("INVENTORY", "fonts/Marker Felt.ttf", 40);
	inventory->setPosition(0 * RES_ADJUST, 290 * RES_ADJUST);
	inventory->setOpacity(230);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	inventory->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	inventory->setAdditionalKerning(0.25f);
	this->addChild(inventory, 5);
	labels.insert(std::make_pair("inventory", inventory));

	// menu border
	Sprite* longBox = Sprite::create("Inventory_Box_Length_Medium.png");
	this->addChild(longBox, 3);
	longBox->setPosition(-300 * RES_ADJUST, 0);
	longBox->setScale(.46f * RES_ADJUST);
	longBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("long box", longBox));

	// menu border
	Sprite* wideBox = Sprite::create("Inventory_Box_Wide_Medium.png");
	this->addChild(wideBox, 3);
	wideBox->setPosition(150 * RES_ADJUST, 75 * RES_ADJUST);
	wideBox->setScale(.46f * RES_ADJUST);
	wideBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("wide box", wideBox));

	// menu border
	Sprite* textBox = Sprite::create("Inventory_Box_Text_Medium.png");
	this->addChild(textBox, 3);
	textBox->setPosition(130 * RES_ADJUST, -200 * RES_ADJUST);
	textBox->setScale(.42f * RES_ADJUST);
	textBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("text box", textBox));

	// menu border
	Sprite* selectBox = Sprite::create("Selection_Box.png");
	this->addChild(selectBox, 5);
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
	//    8:      Trinket
	//    9 - 23: Passive Items
	//
	//    0   1  |  2   3   4   5   6
	//    7   8  |  9   10  11  12  13
	//    X   X  |  14  15  16  17  18
	//    X   X  |  19  20  21  22  23

	std::string image;

	// Display use items
	for (int i = 0; i < (int)iteminv.size(); i++) {
		inventoryText[i + 2] = std::make_pair(iteminv[i]->getName(), iteminv[i]->getDescription());
		image = iteminv.at(i)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, 4);
		item->setPosition((1.9 * i * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);
		item->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));
	}

	/*if (!wepinv.empty()) {
		for (int i = 0; i < (int)wepinv.size(); i++) {
			inventoryText[i + 2] = std::make_pair(wepinv[i]->getName(), wepinv[i]->getDescription());
			image = wepinv.at(i)->getImageName();

			Sprite* weapon = Sprite::createWithSpriteFrameName(image);
			this->addChild(weapon, 4);
			weapon->setScale(1.0);
			weapon->setPosition((i * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);
			inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));
		}
	}*/

	// Display weapons
	image = p->getWeapon()->getImageName();
	inventoryText[0] = std::make_pair(p->getWeapon()->getName(), p->getWeapon()->getDescription());

	Sprite* weapon = Sprite::createWithSpriteFrameName(image);
	this->addChild(weapon, 4);
	weapon->setScale(1.0);
	weapon->setPosition((-350) * RES_ADJUST, 150 * RES_ADJUST);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));

	if (p->getStoredWeapon() != nullptr) {
		image = p->getStoredWeapon()->getImageName();
		inventoryText[1] = std::make_pair(p->getStoredWeapon()->getName(), p->getStoredWeapon()->getDescription());

		Sprite* storedWeapon = Sprite::createWithSpriteFrameName(image);
		this->addChild(storedWeapon, 4);
		storedWeapon->setScale(1.0);
		storedWeapon->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Stored Weapon", storedWeapon));
	}
	/// End weapon display

	// Label for the item's name
	itemName = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	itemName->setPosition(130 * RES_ADJUST, -150 * RES_ADJUST);
	itemName->setOpacity(230);
	itemName->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	itemName->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	itemName->setAdditionalKerning(0.25f);
	this->addChild(itemName, 5);
	labels.insert(std::make_pair("item name", itemName));

	// Label for the item's description
	itemDescription = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	itemDescription->setPosition(130 * RES_ADJUST, -170 * RES_ADJUST);
	itemDescription->setAnchorPoint(Vec2(0.5, 1.0));
	itemDescription->setOpacity(230);
	itemDescription->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	itemDescription->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	itemDescription->setAdditionalKerning(0.25f);
	this->addChild(itemDescription, 5);
	labels.insert(std::make_pair("item description", itemDescription));

	// Displays first item info
	itemName->setString(inventoryText[0].first);
	itemDescription->setString(inventoryText[0].second);

	if (p->hasActiveItem()) {
		std::string shield = p->getActiveItem()->getName();
		inventoryText[7] = std::make_pair(shield, p->getActiveItem()->getDescription());

		image = p->getActiveItem()->getImageName();

		// shield sprite
		Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentshield, 4, shield);
		currentshield->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);
		currentshield->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
	}

	if (p->hasTrinket()) {
		std::string trinket = p->getTrinket()->getName();
		inventoryText[8] = std::make_pair(trinket, p->getTrinket()->getDescription());

		image = p->getTrinket()->getImageName();

		// trinket sprite
		Sprite* currentTrinket = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentTrinket, 4, trinket);
		currentTrinket->setPosition(-250 * RES_ADJUST, 50 * RES_ADJUST);
		currentTrinket->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("current trinket", currentTrinket));

	}

	// Display passives
	for (int i = 0; i < (int)passives.size(); i++) {
		inventoryText[i + 9] = std::make_pair(passives[i]->getName(), passives[i]->getDescription());
		image = passives.at(i)->getImageName();

		Sprite* passive = Sprite::createWithSpriteFrameName(image);
		this->addChild(passive, 4);

		// X: i % 5 since 5 entries per row; Y: 80 is the number of pixels to next row, i / 5 to get the correct row
		passive->setPosition((1.9 * (i % 5) * MENU_SPACING - 90) * RES_ADJUST, (100 - (80 * (i / 5))) * RES_ADJUST);

		passive->setScale(1.0f);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("Passive", passive));
	}

	// add event listener for selecting
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::inventoryMenuKeyPressed, this, &dungeon);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, selectBox);

}
void HUDLayer::inventoryMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon) {
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	Vec2 pos = event->getCurrentTarget()->getPosition();
	p = dungeon->getPlayer();

	// Inventory key:
	// 
	//    0 - 1:  Weapons
	//    2 - 6:  Usable items
	//    7:      Trinket
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

		updateHUD(*dungeon);
		enableListener();

		return;
	}

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
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

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
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

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
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

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
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

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_I:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 23; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		updateHUD(*dungeon);
		enableListener();

		return;
	}
	default: break;
	}
}
void HUDLayer::weaponMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon) {
	// assign the paused listener to HUDLayer
	activeListener = listener;

	p = dungeon.getPlayer();
	std::vector<std::shared_ptr<Weapon>> wepinv = p->getWeapons();

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// |----|----|----|----|----|
	// |item|item|item|item|empt|
	// |    |    |    |    |    |
	// |----|----|----|----|----|

	// [RENDER MENU DESIGN]

	for (int i = -2; i < 3; i++) {
		Sprite* box = Sprite::create("Menu_Box1.png");
		this->addChild(box, 2);
		box->setPosition(i*MENU_SPACING, 3);
		box->setOpacity(200);
		weaponMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));
	}

	std::string image = "cheese.png";
	int size = wepinv.size();
	for (int i = -2; i < size - 2; i++) {
		image = wepinv.at(i + 2)->getImageName();

		Sprite* weapon = Sprite::createWithSpriteFrameName(image);
		this->addChild(weapon, 4);
		weapon->setScale(0.6f);
		weapon->setPosition(i*MENU_SPACING, origin.y);
		weaponMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));
	}

	// arrow sprite for selection
	auto sprite = Sprite::create("Down_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, MENU_SPACING);
	sprite->setScale(2.0);
	this->addChild(sprite, 4);
	weaponMenuSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	// helper label
	auto use = Label::createWithTTF("Space/Enter: Equip weapon", "fonts/Marker Felt.ttf", 20);
	use->setPosition(0 * RES_ADJUST, -300 * RES_ADJUST);
	use->setOpacity(200);
	use->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	use->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	use->setAdditionalKerning(0.25f);
	this->addChild(use, 4);
	labels.insert(std::make_pair("use", use));


	// add event listener for selecting
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::weaponMenuKeyPressed, this, &dungeon);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);

}
void HUDLayer::weaponMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon) {
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	Vec2 pos = event->getCurrentTarget()->getPosition();
	p = dungeon->getPlayer();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x - MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0 && p->getWeapons().size() > 1) {
			index = p->getWeapons().size() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index < (int)p->getWeapons().size() - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == p->getWeapons().size() - 1 && p->getWeapons().size() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ENTER: {
		playInterfaceSound("Confirm 1.mp3");
		if (p->getWeapons().size() != 0) {
			dungeon->changeWeapon(index);
		}
	}
	case EventKeyboard::KeyCode::KEY_W:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(weaponMenuSprites);

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		//while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
		//	actions->update(1.0);
		//}
		while (actions->getNumberOfRunningActionsByTag(1) > 0) {
			actions->update(1.0, 1);
		}

		switch (dungeon->getLevel()) {
		case 1: 
		case 2: 
		case 3: 
		case 4: 
		case 5:
			dungeon->peekDungeon(dungeon->getPlayer()->getPosX(), dungeon->getPlayer()->getPosY(), '-');
			break;
		}

		updateHUD(*dungeon);
		enableListener();

		return;
	}
	default: break;
	}
}
void HUDLayer::itemMenu(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon) {
	// assign the paused listener to HUDLayer
	activeListener = listener;

	p = dungeon.getPlayer();
	std::vector<std::shared_ptr<Drops>> iteminv = p->getItems();

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// |----|----|----|----|----|
	// |item|item|item|item|empt|
	// |    |    |    |    |    |
	// |----|----|----|----|----|

	// [RENDER MENU DESIGN]

	for (int i = -2; i < 3; i++) {
		Sprite* box = Sprite::create("Menu_Box1.png");
		this->addChild(box, 2);
		box->setPosition(i*MENU_SPACING, 3);
		box->setOpacity(200);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));
	}

	std::string image = "cheese.png";
	int size = iteminv.size();

	for (int i = -2; i < size - 2; i++) {
		image = iteminv.at(i + 2)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, 4);
		item->setPosition(i*MENU_SPACING, origin.y);
		item->setScale(0.8f);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));

		// If item is stackable, display stack amount for item
		if (iteminv.at(i + 2)->canStack()) {
			std::shared_ptr<Stackable> stackable = std::dynamic_pointer_cast<Stackable>(iteminv.at(i + 2));

			auto stack = Label::createWithTTF("x" + std::to_string(stackable->getCount()), "fonts/Marker Felt.ttf", 16);
			stack->setPosition(i * MENU_SPACING + 15.f, origin.y - 15.f);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
			stack->setColor(Color3B(220, 220, 220));
			stack->setOpacity(255);
			this->addChild(stack, 4);
			labels.insert(std::pair<std::string, Label*>(std::to_string(i + 2), stack));

			stackable.reset();		
		}
	}

	// arrow sprite for selection
	auto sprite = Sprite::create("Down_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, MENU_SPACING);
	sprite->setScale(2.0);
	this->addChild(sprite, 4);
	itemMenuSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	// helper labels
	auto equip = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY) + ": Assign to quick access", "fonts/Marker Felt.ttf", 20);
	equip->setPosition(0 * RES_ADJUST, -270 * RES_ADJUST);
	equip->setOpacity(200);
	equip->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	equip->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	equip->setAdditionalKerning(0.25f);
	this->addChild(equip, 4);
	labels.insert(std::make_pair("equip", equip));

	auto use = Label::createWithTTF("Space/Enter: Use item", "fonts/Marker Felt.ttf", 20);
	use->setPosition(0 * RES_ADJUST, -300 * RES_ADJUST);
	use->setOpacity(200);
	use->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	use->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	use->setAdditionalKerning(0.25f);
	this->addChild(use, 4);
	labels.insert(std::make_pair("use", use));


	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::itemMenuKeyPressed, this, &dungeon);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);

}
void HUDLayer::itemMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon) {
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	p = dungeon->getPlayer();
	int x = p->getPosX();
	int y = p->getPosY();

	if (keyCode == ITEM_KEY) {
		index = 0;
		deconstructMenu(itemMenuSprites);

		switch (dungeon->getLevel()) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			dungeon->peekDungeon(dungeon->getPlayer()->getPosX(), dungeon->getPlayer()->getPosY(), '-');
			break;
		}

		updateHUD(*dungeon);
		enableListener();

		return;
	}
	else if (keyCode == QUICK_KEY) {
		playInterfaceSound("Confirm 1.mp3");

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActionsByTag(1) > 0) {
			actions->update(1.0, 1);
		}

		// if player has items, set this to new quick item slot
		if (dungeon->getPlayer()->getItems().size() > 0) {
			quick = 1;
			dungeon->assignQuickItem(index);
		}

		index = 0;
		deconstructMenu(itemMenuSprites);

		dungeon->peekDungeon(dungeon->getPlayer()->getPosX(), dungeon->getPlayer()->getPosY(), '-');

		updateHUD(*dungeon);
		enableListener();
		return;
	}

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x - MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == 0 && p->getItems().size() > 1) {
			index = p->getItems().size() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index < (int)p->getItems().size() - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		else if (index == p->getItems().size() - 1 && p->getItems().size() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * MENU_SPACING, MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ENTER: {
		playInterfaceSound("Confirm 1.mp3");

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		//while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
		//	actions->update(1.0);
		//}
		while (actions->getNumberOfRunningActionsByTag(1) > 0) { // >1 because follow player is always running
			actions->update(1.0, 1);
		}

		if (p->getItems().size() != 0) {
			dungeon->callUse(index); // use item
		}

		// if there are no more items, then there's no more quick access item
		if (dungeon->getPlayer()->getItems().size() == 0)
			quick = 0; // reset quick if first item is used
	}
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(itemMenuSprites);

		switch (dungeon->getLevel()) {
		case 1: 
		case 2: 
		case 3:
		case 4: 
		case 5:
			dungeon->peekDungeon(dungeon->getPlayer()->getPosX(), dungeon->getPlayer()->getPosY(), '-');
			break;
		}

		updateHUD(*dungeon);
		enableListener();

		return;
	}
	default: break;
	}

}
void HUDLayer::gameOver() {
	// prevent player movement
	this->_eventDispatcher->removeAllEventListeners();
	//Director::getInstance()->getScheduler()->unscheduleAllForTarget(&scene);

	// play game over tune
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


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0f);

	// Game over!
	auto pause = Label::createWithTTF("YOU DIED", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);

	// Back to Menu option
	auto back = Label::createWithTTF("Main Menu", "fonts/Marker Felt.ttf", 36);
	back->setPosition(0, 2 * MENU_SPACING);
	this->addChild(back, 3);

	// Restart option
	auto resume = Label::createWithTTF("Restart", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, 1 * MENU_SPACING);
	this->addChild(resume, 3);

	// Quit option
	auto exit = Label::createWithTTF("Exit Game", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, 3);


	// add new event listener for game over menu
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::gameOverKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void HUDLayer::gameOver(cocos2d::Scene &scene) {
	// prevent player movement
	this->_eventDispatcher->removeAllEventListeners();
	Director::getInstance()->getScheduler()->unscheduleAllForTarget(&scene);

	// play game over tune
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


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0f);

	// Game over!
	auto pause = Label::createWithTTF("YOU DIED", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);

	// Back to Menu option
	auto back = Label::createWithTTF("Main Menu", "fonts/Marker Felt.ttf", 36);
	back->setPosition(0, 2 * MENU_SPACING);
	this->addChild(back, 3);

	// Resume option
	auto resume = Label::createWithTTF("Restart", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, 1 * MENU_SPACING);
	this->addChild(resume, 3);

	// Quit option
	auto exit = Label::createWithTTF("Exit Game", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, 3);


	// add new event listener for game over menu
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::gameOverKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void HUDLayer::gameOverKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
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
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
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
	case EventKeyboard::KeyCode::KEY_ENTER:
	//case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Back to main menu
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			playInterfaceSound("Confirm 1.mp3");

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// advance to start menu scene
			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
		case 1: { // Restart

			// generates a new dungeon and replaces the current one
			
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			playInterfaceSound("Confirm 1.mp3");

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// generate a new level 1 scene
			std::shared_ptr<Player> newPlayer(nullptr);
			if (p->getName() == ADVENTURER)
				newPlayer = std::make_shared<Adventurer>();
			else if (p->getName() == SPELLCASTER)
				newPlayer = std::make_shared<Spellcaster>();
			else if (p->getName() == MADMAN)
				newPlayer = std::make_shared<TheMadman>();
			else if (p->getName() == SPELUNKER)
				newPlayer = std::make_shared<Spelunker>();
			else if (p->getName() == ACROBAT)
				newPlayer = std::make_shared<TheMadman>();
			else
				newPlayer = std::make_shared<Adventurer>();

			auto level1Scene = LevelScene::createScene(newPlayer, 1);
			level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

			Director::getInstance()->replaceScene(level1Scene); // replace with new scene
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
void HUDLayer::winner() {
	// prevent player movement
	this->_eventDispatcher->removeAllEventListeners();


	// play winner tune
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
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2f);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0f);

	// You won!
	auto pause = Label::createWithTTF("YOU WON!", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);

	// Resume option
	auto resume = Label::createWithTTF("Restart", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, 2 * MENU_SPACING);
	this->addChild(resume, 3);

	// Quit option
	auto exit = Label::createWithTTF("Exit Game", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, 3);


	// add new event listener for win menu
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::winnerKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void HUDLayer::winnerKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 4 * MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Restart

			playInterfaceSound("Confirm 1.mp3");

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// generate a new level 1 scene
			std::shared_ptr<Player> newPlayer(nullptr);
			if (p->getName() == ADVENTURER)
				newPlayer = std::make_shared<Adventurer>();
			else if (p->getName() == SPELLCASTER)
				newPlayer = std::make_shared<Spellcaster>();
			else if (p->getName() == MADMAN)
				newPlayer = std::make_shared<TheMadman>();
			else if (p->getName() == SPELUNKER)
				newPlayer = std::make_shared<Spelunker>();
			else if (p->getName() == ACROBAT)
				newPlayer = std::make_shared<TheMadman>();
			else
				newPlayer = std::make_shared<Adventurer>();

			auto level1Scene = LevelScene::createScene(newPlayer, 1);
			level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

			Director::getInstance()->replaceScene(level1Scene); // replace with new scene
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

void HUDLayer::deconstructMenu(std::multimap<std::string, cocos2d::Sprite*> &sprites) {
	// remove menu images
	for (auto &it : sprites) {
		it.second->removeFromParent();
	}
	sprites.clear();

	// remove any labels
	for (auto &it : labels) {
		it.second->removeFromParent();
	}
	labels.clear();
}

void HUDLayer::constructActiveItemHUD() {
	std::string name = p->getActiveItem()->getName();
	std::string image = p->getActiveItem()->getImageName();

	// shield HUD box
	Sprite* activeBox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	activeBox->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
	activeBox->setScale(.2f * RES_ADJUST);
	activeBox->setOpacity(160);
	activeBox->setColor(cocos2d::Color3B(255, 175, 5));
	this->addChild(activeBox, 2, "Active Box");
	HUD.insert(std::pair<std::string, Sprite*>("Active Box", activeBox));

	// Active Item sprite
	Sprite* active = Sprite::createWithSpriteFrameName(image);
	this->addChild(active, 3, name);
	active->setPosition(-570.f * RES_ADJUST, 180.f * RES_ADJUST);
	active->setScale(0.5f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", active));

	if (p->activeHasMeter()) {
		// Active Item bar
		Sprite* activeBar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
		this->addChild(activeBar, 3, "Active Bar");
		activeBar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 150 * RES_ADJUST);
		activeBar->setScale(1.0f * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Bar", activeBar));

		// Active Item bar points
		Sprite* activePoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
		this->addChild(activePoints, 4, "Active Points");
		activePoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
		activePoints->setPosition(-583 * RES_ADJUST, 150 * RES_ADJUST);
		activePoints->setScale(0.4f * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("Active Points", activePoints));
	}

	// SPACE label
	auto space = Label::createWithTTF(convertKeycodeToStr(ACTIVE_KEY), "fonts/Marker Felt.ttf", 14);
	space->setPosition(-570.f * RES_ADJUST, 162.f * RES_ADJUST);
	space->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	space->setOpacity(245);
	this->addChild(space, 4);
	keyLabels.insert(std::pair<std::string, Label*>("space", space));

	// Display stack amount for Spelunker
	if (p->getName() == SPELUNKER) {
		std::shared_ptr<Spelunker> sp = std::dynamic_pointer_cast<Spelunker>(p);
		int count = sp->getRockCount();
		sp.reset();

		// If there was no label for stackable items, add it
		if (keyLabels.find("Active Stack Amount") == keyLabels.end()) {
			auto stack = Label::createWithTTF("x" + std::to_string(count), "fonts/Marker Felt.ttf", 16);
			stack->setPosition(-540.f * RES_ADJUST, 180.f * RES_ADJUST);
			stack->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 0);
			stack->setOpacity(250);
			this->addChild(stack, 4);
			keyLabels.insert(std::pair<std::string, Label*>("Active Stack Amount", stack));
		}
		else {
			keyLabels.find("Active Stack Amount")->second->setString("x" + std::to_string(count));
		}
	}
}
void HUDLayer::updateActiveItemHUD() {
	// Remove old sprite
	HUD.find("Active")->second->removeFromParent();
	HUD.erase("Active");

	std::string name = p->getActiveItem()->getName();
	std::string image = p->getActiveItem()->getImageName();

	Sprite* newActive = Sprite::createWithSpriteFrameName(image);
	this->addChild(newActive, 3, name);
	newActive->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
	newActive->setScale(0.50f * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("Active", newActive));
}
void HUDLayer::updateActiveItemBar() {
	// if active has a meter, update it
	if (p->hasActiveItem() && p->activeHasMeter()) {
		double sx_scale = p->getCurrentActiveMeter() / (static_cast<double>(p->getMaxActiveMeter()) * 1.0);
		cocos2d::Action* move = cocos2d::ScaleTo::create(.3f, sx_scale, 1);
		HUD.find("Active Points")->second->runAction(move);
	}
}
void HUDLayer::deconstructActiveItemHUD() {
	if (HUD.find("Active Box") != HUD.end()) {
		// deconstruct the shield HUD because there's no shield equipped
		HUD.find("Active")->second->removeFromParent();
		HUD.find("Active Box")->second->removeFromParent();
		keyLabels.find("space")->second->removeFromParent();

		HUD.erase(HUD.find("Active"));
		HUD.erase(HUD.find("Active Box"));
		keyLabels.erase(keyLabels.find("space"));

		if (p->activeHasMeter()) {
			HUD.find("Active Bar")->second->removeFromParent();
			HUD.find("Active Points")->second->removeFromParent();
			HUD.erase(HUD.find("Active Bar"));
			HUD.erase(HUD.find("Active Points"));
		}

		if (p->getName() == SPELUNKER) {
			keyLabels.find("Active Stack Amount")->second->removeFromParent();
			keyLabels.erase(keyLabels.find("Active Stack Amount"));
		}
	}
}

void HUDLayer::constructItemHUD() {
	// Quick access item use slot
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* quickAccess = Sprite::createWithSpriteFrameName(image);
	quickAccess->setPosition(570 * RES_ADJUST, 240 * RES_ADJUST);
	quickAccess->setScale(.2f * RES_ADJUST);
	quickAccess->setOpacity(160);
	quickAccess->setColor(cocos2d::Color3B(250, 188, 165));
	this->addChild(quickAccess, 2, "quick access");
	HUD.insert(std::pair<std::string, Sprite*>("quick access", quickAccess));

	// key label
	auto qKey = Label::createWithTTF(convertKeycodeToStr(QUICK_KEY), "fonts/Marker Felt.ttf", 14);
	qKey->setPosition(570.f * RES_ADJUST, 217.f * RES_ADJUST);
	qKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	qKey->setOpacity(245);
	this->addChild(qKey, 4);
	keyLabels.insert(std::pair<std::string, Label*>("qKey", qKey));

	updateItemHUD();
}
void HUDLayer::updateItemHUD() {
	// The item to display
	Sprite* quickitem = Sprite::createWithSpriteFrameName(p->getItems().at(0)->getImageName());
	this->addChild(quickitem, 3, p->getItems()[0]->getName());
	quickitem->setPosition(570.f * RES_ADJUST, 240.f * RES_ADJUST);
	quickitem->setScale(0.8f);
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
}

void HUDLayer::deconstructTrinketHUD() {
	if (HUD.find("trinketbox") != HUD.end()) {
		// deconstruct the trinket HUD because there's no trinket equipped
		HUD.find("current trinket")->second->removeFromParent();
		HUD.find("trinketbox")->second->removeFromParent();

		HUD.erase(HUD.find("current trinket"));
		HUD.erase(HUD.find("trinketbox"));
	}
}
void HUDLayer::deconstructShopHUD() {
	itemprice->removeFromParent();
	itemprice = nullptr;
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

void HUDLayer::enableListener() {
	release = EventListenerKeyboard::create();
	release->onKeyReleased = CC_CALLBACK_2(HUDLayer::menuKeyReleased, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(release, this);
}
void HUDLayer::menuKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_Q:
	case EventKeyboard::KeyCode::KEY_W:
	case EventKeyboard::KeyCode::KEY_C:
	case EventKeyboard::KeyCode::KEY_ESCAPE:
	default:
		this->_eventDispatcher->removeEventListener(release);
		activeListener->setEnabled(true);
		activeListener = nullptr;
	}

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
LevelScene::LevelScene(HUDLayer* hud, std::shared_ptr<Player> p, int level) : m_hud(hud), p(p), m_level(level) {
	
}
LevelScene::~LevelScene() {
	if (m_currentDungeon != nullptr)
		delete m_currentDungeon;
}

Scene* LevelScene::createScene(std::shared_ptr<Player> p, int level)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create(p);
	scene->addChild(hud, 10);

	// create background layer
	//BackgroundLayer* bglayer = BackgroundLayer::create();
	//scene->addChild(bglayer, -10);

	// calls LevelScene init()
	auto layer = LevelScene::create(hud, p, level);
	scene->addChild(layer);

	return scene;
}
LevelScene* LevelScene::create(HUDLayer* hud, std::shared_ptr<Player> p, int level)
{
	LevelScene *pRet = new(std::nothrow) LevelScene(hud, p, level);
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
bool LevelScene::init()
{
	if (!Scene::init()) {
		return false;
	}

	// determine the current level and create the correct dungeon
	setCurrentDungeon(m_level, p);

	// Update HUD
	m_hud->updateHUD(*m_currentDungeon);

	// music
	setMusic(getCurrentDungeon()->getLevel());

	// reveal boss hp bar, if necessary
	if (getCurrentDungeon()->getLevel() == 6)
		m_hud->showBossHP();


	auto visibleSize = Director::getInstance()->getVisibleSize();

	// board for rendering textures into
	renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height, Texture2D::PixelFormat::RGBA8888);
	renderTexture->retain();
	renderTexture->beginWithClear(0.0, 0.0, 0.0, 0.0);
	
	//gameboard = Sprite::create();
	//gameboard->setFlippedY(true);
	//gameboard->setPosition(visibleSize.width/2, visibleSize.height/2);
	//gameboard->visit();
	//this->addChild(gameboard, 0);
	

	///lighting garbage
	//// lighting?
	//m_lighting = LightEffect::create();
	//m_lighting->retain();

	//Vec3 lightPos(visibleSize.width / 2, visibleSize.height, 10);
	//m_lighting->setLightPos(lightPos);
	//m_lighting->setLightCutoffRadius(10000);
	////m_lighting->setLightHalfRadius(0.5);
	//m_lighting->setBrightness(3.0);
	//m_lighting->setLightColor(Color3B(200, 170, 200));


	// set player sprite position
	/*m_player = Sprite::create("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 2);
	auto action = this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	
	m_player->visit();*/

	createPlayerSpriteAndCamera();

	// Render all the sprites on the first floor
	renderDungeon(*m_currentDungeon);

	renderTexture->end();

	/*
	// create the light manager
	lightManager = DynamicLightManager::create();
	// use a "large" z-order to make sure this is drawn on top of everything
	this->addChild(lightManager, 20);

	// load the image used as the shadow casters, solid pixels block the lights
	auto shadowCasters = Sprite::createWithSpriteFrameName("D_Wall_Terrain1_48x48.png");
	shadowCasters->setPosition(px, py);
	shadowCasters->visit();
	this->addChild(shadowCasters, 1);

	// add some lights

	// light without using a light texture
	auto light1 = DynamicLight::create();
	light1->setShadowCasters(shadowCasters);
	light1->setSoftShadows(false);
	light1->setAdditive(true);
	light1->setLightSize(100);
	light1->setColor(Color4B(158, 255, 121, 230));
	light1->setPosition(Vec2(px - 50, py));
	lightManager->addLightSource(light1);

	// create(direction, position, color, innerAngle, outerAngle, Range)
	auto spotLight = SpotLight::create(Vec3(-1.0f, -1.0f, 10.0f), Vec3(px, py, 0.0f),
		Color3B::RED, 0.0, 0.5, 1000.0f);
	addChild(spotLight);
	*/

	//// light with light texture. Can be used to create spotLights (see light2.png inside images folder)
	//auto lightSpr = Sprite::create("PointLight4-0.png");
	//auto light2 = DynamicLight::create(lightSpr->getTexture());
	//light2->setShadowCasters(shadowCasters);
	//light2->setSoftShadows(true);
	//light2->setAdditive(true);
	//light2->setLightSize(10000);
	//light2->setColor(Color4B(158, 255, 121, 230));
	//light2->setPosition(Vec2(px, py));
	////light2->setRotation(45); // textured lights can be rotated
	//lightManager->addLightSource(light2);

	// new effect sprite
	/*
	auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Player1_48x48.png");
	m_playerLight = EffectSprite::createWithSpriteFrame(spriteFrame);
	m_playerLight->setPosition(px, py);
	m_lighting->setLightPos(Vec3(px, py, 5));
	m_playerLight->setEffect(m_lighting, "1_Spritesheet_48x48.png");
	//this->addChild(m_playerLight, 2);
	//this->runAction(Follow::createWithOffset(m_playerLight, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	*/

	// load board with the rendered sprites
	//gameboard->initWithTexture(renderTexture->getSprite()->getTexture());
	//gameboard->setPosition(visibleSize.width/2, visibleSize.height/2);

	// Hides the extra game layer!!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1); // flag on camera

	/// nonsense camera
	//gameCamera = Camera::createOrthographic(visibleSize.width, visibleSize.height, 1.0, 500);
	//gameCamera->setCameraMask((unsigned short)CameraFlag::DEFAULT, true); // mask on the node
	//gameCamera->setPosition3D(Vec3(px, py, 100)); // sets a fixed point for the camera
	//gameCamera->lookAt(Vec3(px, py, 200)); // camera turns to look at this point
	//this->addChild(gameCamera);


	setSprites(m_currentDungeon);
	updateLevelLighting();
		

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(LevelScene::LevelKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player); // check this for player

	
	scheduleTimer();

	return true;
}

void LevelScene::renderDungeon(Dungeon &dungeon) {

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	std::vector<_Tile> maze = dungeon.getDungeon();
	_Tile *tile;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Extras						 x
	//	    Gold/Money					-3
	//		Stairs/Traps				-4
	//		Floor		: BOTTOM		-5
	//

	int x, y;
	std::string image;
	
	// BEGIN DUNGEON RENDER
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand;
	int z;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			tile = &maze[i*cols + j];

			rand = randInt(8) + 1;
			switch (rand) {
			case 1: image = "FloorTile1_48x48.png"; break;
			case 2: image = "FloorTile2_48x48.png"; break;
			case 3: image = "FloorTile3_48x48.png"; break;
			case 4: image = "FloorTile4_48x48.png"; break;
			case 5: image = "FloorTile5_48x48.png"; break;
			case 6: image = "FloorTile6_48x48.png"; break;
			case 7: image = "FloorTile7_48x48.png"; break;
			case 8: image = "FloorTile8_48x48.png"; break;
			}

			Sprite* floor = createSprite(image, rows, j, i, -10);
			floor->setOpacity(252);
			if (!(/*m_level == 2 || m_level == 4 ||*/ m_level == 6))
				floor->setColor(Color3B(0, 0, 0));
		
			dungeon[i*cols + j].floor = floor;

			if (tile->upper == SMASHER && j == BOSSCOLS / 2 && i == 3) {
				// smasher position hardcoded to begin with
				image = "C_Wall_Terrain1_48x48.png";
				Sprite* smasher = createSprite(image, rows, j, i, 2);
				smasher->setScale(3.0f * GLOBAL_SPRITE_SCALE);
				smasher->setColor(Color3B(55, 30, 40));

				int pos = dungeon.findMonster(j, i);
				dungeon.getMonsters()[pos]->setSprite(smasher);
			}
			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (rows - i) - Y_OFFSET;
			}
			if (tile->npc) {
				int pos = dungeon.findNPC(j, i);

				if (pos != -1) {

					int offset = 0;
					// Move the shopkeeper's sprite up one tile since they are behind the counter
					if (dungeon.getNPCs()[pos]->getName() == SHOPKEEPER) {
						offset = -1;
					}

					image = dungeon.getNPCs()[pos]->getImageName();
					Sprite* npc = createSprite(image, rows, j, i + offset, 1);
					dungeon.getNPCs()[pos]->setSprite(npc);
				}
			}
			if (tile->shop_action != "") {

				if (tile->shop_action == "countertop") image = "Countertop_48x48.png";
				else if (tile->shop_action == "shopkeeper") image = "Shopkeeper_48x48.png";
				else if (tile->shop_action == "breakable") image = "Breakable_Crate_48x48.png";
				else if (tile->shop_action == "secret") image = "Stairs_48x48.png";
				// if tile is labeled shop_item, puts a countertop image underneath the item, as a shop does
				else if (tile->shop_action == "shop_item") image = "Countertop_48x48.png";

				if (tile->shop_action != "purchase") {
					Sprite* object = createSprite(image, rows, j, i, 1);
					dungeon.misc_sprites.push_back(object);
				}
			}
			if (tile->gold != 0) {
				switch (tile->gold) {
				case 1: image = "Gold_Coin1_48x48.png"; break;
				case 2: image = "Gold_Coin2_48x48.png"; break;
				case 3: image = "Gold_Coin3_48x48.png"; break;
				case 4: image = "Gold_Pile1_48x48.png"; break;
				case 5: image = "Gold_Pile2_48x48.png"; break;
				default: image = "Gold_Pile2_48x48.png"; break;
				}
				Sprite* gold = createSprite(image, rows, j, i, -3);
				money.push_back(gold);
			}
			if (tile->wall) {

				if (tile->wall_type == REG_WALL) {
					rand = randInt(13) + 1;
					switch (rand) {
					case 1: image = "D_Wall_Terrain1_48x48.png"; break;
					case 2: image = "D_Wall_Terrain2_48x48.png"; break;
					case 3: image = "D_Wall_Terrain3_48x48.png"; break;
					case 4: image = "D_Wall_Terrain4_48x48.png"; break;
					case 5: image = "D_Wall_Terrain5_48x48.png"; break;
					case 6: image = "D_Wall_Terrain6_48x48.png"; break;
					case 7: image = "D_Wall_Terrain7_48x48.png"; break;
					case 8: image = "D_Wall_Terrain8_48x48.png"; break;
					case 9: image = "D_Wall_Terrain9_48x48.png"; break;
					case 10: image = "D_Wall_Terrain10_48x48.png"; break;
					case 11: image = "D_Wall_Terrain11_48x48.png"; break;
					case 12: image = "D_Wall_Terrain12_48x48.png"; break;
					case 13: image = "D_Wall_Terrain13_48x48.png"; break;
					}

					Sprite* wall = createSprite(image, rows, j, i, 1);

					//wall->setColor(Color3B(210, 200, 255));
					wall->setColor(Color3B(0, 0, 0));
					walls.push_back(wall);
				}
				else if (tile->wall_type == UNB_WALL) {

					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == cols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == rows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
					else if (j == 0) rand = randInt(3) + 10;// + (randInt(2) * 3); // left border

					switch (rand) {
					case 1: image = "C_Wall_Terrain1_48x48.png"; break;
					case 2: image = "C_Wall_Terrain2_48x48.png"; break;
					case 3: image = "C_Wall_Terrain3_48x48.png"; break;
					case 4: image = "C_Wall_Terrain4_48x48.png"; break;
					case 5: image = "C_Wall_Terrain5_48x48.png"; break;
					case 6: image = "C_Wall_Terrain6_48x48.png"; break;
					case 7: image = "C_Wall_Terrain7_48x48.png"; break;
					case 8: image = "C_Wall_Terrain8_48x48.png"; break;
					case 9: image = "C_Wall_Terrain9_48x48.png"; break;
					case 10: image = "C_Wall_Terrain10_48x48.png"; break;
					case 11: image = "C_Wall_Terrain11_48x48.png"; break;
					case 12: image = "C_Wall_Terrain12_48x48.png"; break;
						//
					case 13: image = "C_Wall_Terrain13_48x48.png"; break;
					case 14: image = "C_Wall_Terrain14_48x48.png"; break;
					case 15: image = "C_Wall_Terrain15_48x48.png"; break;
					}

					Sprite* wall = createSprite(image, rows, j, i, 1);
					wall->setColor(Color3B(170, 90, 40));
					//wall->setColor(Color3B(0, 0, 0));
					walls.push_back(wall);
				}
				else if (tile->wall_type == FOUNTAIN) {
					image = "Fountain_Down_48x48.png";
					Sprite* fountain = Sprite::createWithSpriteFrameName(image);
					this->addChild(fountain, 1);
					fountain->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (rows - i) - Y_OFFSET);
					walls.push_back(fountain); // fountain is a wall
				}
				else if (tile->wall_type == DOOR_VERTICAL) {
					image = "Door_Vertical_Closed_48x48.png";
					Sprite* door = createSprite(image, rows, j, i, 1);
					doors.push_back(door);
				}
				else if (tile->wall_type == DOOR_HORIZONTAL) {
					image = "Door_Horizontal_Closed_48x48.png";
					Sprite* door = createSprite(image, rows, j, i, 1);
					doors.push_back(door);
				}

			}
			if (tile->enemy && tile->upper != SMASHER) {

				if (tile->monster_name == SPINNER) {
					// add spinner buddies to the scene
					std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monster.at(dungeon.findMonster(j, i)));

					this->addChild(spinner->getInner(), 0);
					this->addChild(spinner->getOuter(), 0);

					spinner.reset();
				}
				else if (tile->monster_name == ZAPPER) {
					// add the sparks to the scene
					std::shared_ptr<Zapper> zapper = std::dynamic_pointer_cast<Zapper>(monster.at(dungeon.findMonster(j, i)));
					for (int m = 0; m < 8; m++) {
						this->addChild(zapper->getSparks()[m], 0);
					}
					zapper.reset();
				}

				
				int pos = dungeon.findMonster(j, i);
				image = monster.at(pos)->getImageName();
				if (monster.at(pos)->hasAnimation()) {
					cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation(monster.at(pos)->getAnimationFrames(), monster.at(pos)->getAnimationFrameCount());
					Sprite* monsterSprite = createAnimation(frames, monster.at(pos)->getFrameInterval(), rows, j, i, 1);
					dungeon.getMonsters()[pos]->setSprite(monsterSprite);
				}
				else {
					Sprite* monsterSprite = createSprite(image, rows, j, i, 1);
					dungeon.getMonsters()[pos]->setSprite(monsterSprite);
				}

			}
			if (tile->item) {
				image = tile->object->getImageName();

				int z = -1;
				switch (m_level) {
				case 2:
				case 4: z = 2; break;
				}
				Sprite* object = createSprite(image, rows, j, i, z);
				items.push_back(object);
			}
			if (tile->trap) {

				image = dungeon.getTraps()[dungeon.findTrap(j, i)]->getImageName();

				if (tile->trap_name == FIREBAR) {
					z = 1;

					// add firebar buddies to the scene
					std::shared_ptr<Firebar> firebar = std::dynamic_pointer_cast<Firebar>(dungeonTraps.at(dungeon.findTrap(j, i)));

					this->addChild(firebar->getInner(), 0);
					this->addChild(firebar->getOuter(), 0);

					firebar.reset();					
				}
				else if (tile->trap_name == DOUBLE_FIREBAR) {
					z = 1;

					// add firebar buddies to the scene
					std::shared_ptr<DoubleFirebar> firebar = std::dynamic_pointer_cast<DoubleFirebar>(dungeonTraps.at(dungeon.findTrap(j, i)));

					this->addChild(firebar->getInner(), 0);
					this->addChild(firebar->getInnerMirror(), 0);
					this->addChild(firebar->getOuter(), 0);
					this->addChild(firebar->getOuterMirror(), 0);

					firebar.reset();
				}
				else if (tile->trap_name == AUTOSPIKE_DEACTIVE) {
					int n = dungeon.findTrap(j, i);
					std::shared_ptr<SpikeTrap> spiketrap = std::dynamic_pointer_cast<SpikeTrap>(dungeonTraps.at(n));

					spiketrap->getSpriteD()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteP()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteA()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					this->addChild(spiketrap->getSpriteD(), -4);
					this->addChild(spiketrap->getSpriteP(), -4);
					this->addChild(spiketrap->getSpriteA(), -4);

					spiketrap->setSpriteVisibility(false, false, false);

					continue;
				}
				else if (tile->trap_name == TRIGGERSPIKE_DEACTIVE) {
					int n = dungeon.findTrap(j, i);
					std::shared_ptr<TriggerSpike> spiketrap = std::dynamic_pointer_cast<TriggerSpike>(dungeonTraps.at(n));

					spiketrap->getSpriteD()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteP()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteA()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					this->addChild(spiketrap->getSpriteD(), -4);
					this->addChild(spiketrap->getSpriteP(), -4);
					this->addChild(spiketrap->getSpriteA(), -4);

					spiketrap->setSpriteVisibility(false, false, false);

					continue;
				}
				else if (tile->trap_name == MOVING_BLOCK) {
					z = 2;

					// change this tile to not be a trap because of invisible wall bug
					dungeon[i*cols + j].trap = false;
				}
				else if (tile->trap_name == TURRET) {
					z = 1;
				}
				else if (tile->trap_name == STAIRCASE) {
					image = "Stairs_48x48.png";
					z = -4;
				}
				else if (tile->trap_name == LOCKED_STAIRCASE) {
					image = "Locked_Stairs_48x48.png";
					z = -4;
				}
				else if (tile->trap_name == BUTTON_UNPRESSED) {
					image = "Button_Unpressed_48x48.png";
					z = -4;
				}
				else if (tile->trap_name == DEVILS_WATER) {
					image = "Water_Tile1_48x48.png";
					z = -4;
				}
				else {
					z = -4;
				}
				
				Sprite* trap = createSprite(image, rows, j, i, z);
				if (tile->trap_name == SPRING) {
					trap->setScale(0.5);
				}

				int pos = dungeon.findTrap(j, i);
				if (pos != -1)
					dungeonTraps.at(pos)->setSprite(trap);
			}

		}
	}

	m_player->setPosition(x, y);
}
cocos2d::Sprite* LevelScene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	// Screen real dimensions
	auto vSize = Director::getInstance()->getVisibleSize();
	auto vWidth = vSize.width;
	auto vHeight = vSize.height;

	// Original image
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	sprite->visit();

	this->addChild(sprite, z);
	//graySprite(sprite);

	return sprite;

	//// Create new Sprite without scale, which perfoms much better
	//auto newSprite = Sprite::createWithTexture(renderTexture->getSprite()->getTexture());
	//newSprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	//addChild(newSprite, z);
}
cocos2d::Sprite* LevelScene::createAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int rows, int x, int y, int z) {
	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	this->addChild(sprite, z);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (rows - y) - Y_OFFSET);
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	//auto remove = cocos2d::RemoveSelf::create();
	//auto sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::Animate::create(animation), remove);
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
	int rows = m_currentDungeon->getRows();

	auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	EffectSprite* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (rows - y) - Y_OFFSET);
	sprite->visit();
	this->addChild(sprite, z);

	sprite->setEffect(m_lighting, "1_Spritesheet_48x48.png");

	return sprite;
}

void LevelScene::LevelKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	// unschedule the inaction timer
	Director::getInstance()->getScheduler()->unschedule("timer", this);

	int x, y;

	auto actions = this->getActionManager();

	// resumes follow if it was paused
	if (actions->getNumberOfRunningActions() == 0 && m_currentDungeon->getLevel() != 6) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	}

	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActionsByTag(1) > 0) {
		actions->update(1.0, 1);
	}


	p = m_currentDungeon->getPlayer();
	x = p->getPosX(); y = p->getPosY();
	char facingDirection = p->facingDirection();
	int currentLevel = m_currentDungeon->getLevel();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	if (keyCode == LEFT_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'l');

		if (m_facing == 'r') {
			m_player->setScaleX(m_player->getScaleX() * -1);
			m_facing = 'l';
		}
	}
	else if (keyCode == RIGHT_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'r');

		if (m_facing == 'l') {
			m_player->setScaleX(m_player->getScaleX() * -1);
			m_facing = 'r';
		}
	}
	else if (keyCode == UP_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'u');
	}
	else if (keyCode == DOWN_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'd');
	}
	else if (keyCode == QUICK_KEY) {
		if (m_currentDungeon->getPlayer()->getItems().size() > 0)
			m_currentDungeon->callUse(0);

		m_currentDungeon->peekDungeon(x, y, '-');
	}
	else if (keyCode == CAST_KEY) {
		m_currentDungeon->peekDungeon(x, y, WIND_UP);
	}
	else if (keyCode == ACTIVE_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'b');
	}
	else if (keyCode == INTERACT_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'e');
		if (playerAdvanced(currentLevel))
			advanceLevel();
	}
	else if (keyCode == INVENTORY_KEY) {
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_currentDungeon);
	}
	else if (keyCode == WEAPON_KEY) {
		m_currentDungeon->peekDungeon(x, y, 'w');
	}
	else if (keyCode == ITEM_KEY) {
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_currentDungeon);
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_M) {
		if (cocos2d::experimental::AudioEngine::getVolume(bg_music_id) > 0)
			cocos2d::experimental::AudioEngine::setVolume(bg_music_id, 0.0);
		else
			cocos2d::experimental::AudioEngine::setVolume(bg_music_id, 1.0);
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_P || keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
		pauseMenu();
	}
	else {
		m_currentDungeon->peekDungeon(x, y, '-');
	}

	/*
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_currentDungeon->peekDungeon(x, y, 'l');
		
		if (facingDirection != 'l') {
			m_player->setScaleX(-1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_currentDungeon->peekDungeon(x, y, 'r');
		
		if (facingDirection != 'r') {
			m_player->setScaleX(1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_currentDungeon->peekDungeon(x, y, 'u');
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_currentDungeon->peekDungeon(x, y, 'd');
		
		break;
	}

	//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_currentDungeon->getPlayer()->getItems().size() > 0)
			m_currentDungeon->callUse(0);

		m_currentDungeon->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: { // for items with special actions
		m_currentDungeon->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_currentDungeon->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_currentDungeon);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_currentDungeon->peekDungeon(x, y, 'e');
		if (playerAdvanced(currentLevel)) {
			advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		//kbListener->setEnabled(false);
		//m_hud->weaponMenu(kbListener, *m_currentDungeon);
		m_currentDungeon->peekDungeon(x, y, 'w');

		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_currentDungeon);

		break;
	case EventKeyboard::KeyCode::KEY_M: { // toggle music on/off
		if (cocos2d::experimental::AudioEngine::getVolume(bg_music_id) > 0) {
			cocos2d::experimental::AudioEngine::setVolume(bg_music_id, 0.0);
		}
		else {
			cocos2d::experimental::AudioEngine::setVolume(bg_music_id, 1.0);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: // open pause menu
		pauseMenu();
		break;
	default:
		m_currentDungeon->peekDungeon(x, y, '-');
		break;
	}*/

	// Check if player return to menu from World Hub
	if (m_currentDungeon->returnedToMenu())
		return;

	// update the HUD
	m_hud->updateHUD(*m_currentDungeon);

	// update boss HUD if on boss level
	if (m_currentDungeon->getLevel() == 6) {
		m_hud->updateBossHUD(*m_currentDungeon);
	}

	// Check if player is dead, if so, run game over screen
	if (m_currentDungeon->getPlayer()->getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(bg_music_id);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}

	// Check if player found the idol
	if (m_currentDungeon->getPlayer()->getWin()) {
		m_hud->winner();
		return; // prevents timer from being scheduled
	}

	// reschedule the inaction timer
	scheduleTimer();

}
void LevelScene::scheduleTimer() {
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0 && m_currentDungeon->getLevel() != 6) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActionsByTag(1) > 0) { // >1 because follow player is always running
			actions->update(1.0, 1);
		}

		p = m_currentDungeon->getPlayer();

		m_currentDungeon->peekDungeon(m_currentDungeon->getPlayer()->getPosX(), m_currentDungeon->getPlayer()->getPosY(), '-');

		// update HUD
		m_hud->updateHUD(*m_currentDungeon);

		// Check if player is dead, if so, run game over screen
		if (m_currentDungeon->getPlayer()->getHP() <= 0) {
			m_hud->gameOver(*this);
		}
	}, this, getTimerSpeed(), false, "timer");
}

void LevelScene::createPlayerSpriteAndCamera() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	m_player = Sprite::createWithSpriteFrameName(p->getImageName());
	m_player->setPosition(0, 0);
	m_player->setScale(GLOBAL_SPRITE_SCALE);
	this->addChild(m_player, 2);
	
	switch (m_currentDungeon->getLevel()) {
	case 6:
		this->setScale(0.4f);
		this->setPosition((-visibleSize.width / 2) * .7, (-visibleSize.height / 2) * 1.2);
		break;

	default:
		this->runAction(cocos2d::Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, cocos2d::Rect::ZERO));
		break;
	}

	m_player->visit();
}
void LevelScene::setCurrentDungeon(int level, std::shared_ptr<Player> player) {
	switch (level) {
	case -1: m_currentDungeon = new TutorialFloor(player); break; // Intro/Tutorial
	case 0: m_currentDungeon = new WorldHub(player); break; // World Hub
	case 1: m_currentDungeon = new FirstFloor(player); break;
	case 2: m_currentDungeon = new Shop(player, level); break; // Shop
	case 3: m_currentDungeon = new SecondFloor(player); break;
	case 4: m_currentDungeon = new Shop(player, level); break; // Shop
	case 5: m_currentDungeon = new ThirdFloor(player); break;
	case 6: m_currentDungeon = new FirstBoss(player); break;
	}
}
Dungeon* LevelScene::getCurrentDungeon() {
	return m_currentDungeon;
}
void LevelScene::setMusic(int level) {
	cocos2d::experimental::AudioEngine::setMaxAudioInstance(250);

	std::string music;
	switch (level) {
	case 1: music = "Exploring a cave.mp3"; break;
	case 2: music = "mistical.mp3"; break;
	case 3: music = "Tower of Lava.mp3"; break;
	case 4: music = "mistical.mp3"; break;
	case 5: music = "Who turned off the lights.mp3"; break;
	case 6: music = "Zero Respect.mp3"; break;
	}
	bg_music_id = playMusic(music, true);
}
void LevelScene::setSprites(Dungeon *dungeon) {
	dungeon->getPlayerVector().at(0)->setSprite(m_player);

	dungeon->setItemSprites(items);
	dungeon->setMoneySprites(money);
	dungeon->setWallSprites(walls);
	dungeon->setDoorSprites(doors);

	dungeon->setScene(this);
}
void LevelScene::updateLevelLighting() {
	switch (m_currentDungeon->getLevel()) {
	case -1:
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:  m_currentDungeon->updateLighting(); break;
	default: break;
	}

	// hide rooms if third level
	if (m_currentDungeon->getLevel() == 5) {
		auto third = dynamic_cast<ThirdFloor*>(m_currentDungeon);
		third->hideRooms();
	}
}
float LevelScene::getTimerSpeed() {
	switch (m_currentDungeon->getLevel()) {
	case 1: return 0.70f; break;
	case 3: return 0.60f; break;
	case 5: return 0.50f; break;
	case 6: return 0.40f; break;
	default: return 0.70f; break;
	}
}

void LevelScene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene(p, bg_music_id);
	//pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->pushScene(pauseMenuScene);
	//Director::getInstance()->replaceScene(pauseMenuScene);
}
void LevelScene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// stop music
	cocos2d::experimental::AudioEngine::stop(bg_music_id);

	if (!(m_currentDungeon->getLevel() == 2 || m_currentDungeon->getLevel() == 4)) {
		auto nextScene = LevelScene::createScene(m_currentDungeon->getPlayer(), m_currentDungeon->getLevel());
		nextScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

		Director::getInstance()->replaceScene(nextScene);
	}
	else {
		auto nextScene = ShopScene::createScene(m_currentDungeon->getPlayer(), m_currentDungeon->getLevel());
		nextScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

		Director::getInstance()->replaceScene(nextScene);
	}
}
inline bool LevelScene::playerAdvanced(int level) { 
	return m_currentDungeon->getLevel() != level;
};
void LevelScene::returnToMainMenu() {

	// Unschedule the inaction timer and event listener
	Director::getInstance()->getScheduler()->unschedule("timer", this);
	kbListener->setEnabled(false);

	// stop music
	cocos2d::experimental::AudioEngine::stopAll();
	//playInterfaceSound("Confirm 1.mp3");

	// advance to start menu scene
	auto startScene = StartScene::createScene();

	//Director::getInstance()->replaceScene(startScene); // replace with new scene
	auto transition = TransitionFade::create(3.0f, startScene);
	Director::getInstance()->replaceScene(transition);
}


//		SHOP SCENE
ShopScene::ShopScene(HUDLayer* hud, std::shared_ptr<Player> p, int level) : LevelScene(hud, p, level) {
	
}
Scene* ShopScene::createScene(std::shared_ptr<Player> p, int level)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create(p);
	scene->addChild(hud, 10);

	// create background layer
	//BackgroundLayer* bglayer = BackgroundLayer::create();
	//scene->addChild(bglayer, -10);

	// calls LevelScene init()
	auto layer = ShopScene::create(hud, p, level);
	scene->addChild(layer);

	return scene;
}
ShopScene* ShopScene::create(HUDLayer* hud, std::shared_ptr<Player> p, int level)
{
	ShopScene *pRet = new(std::nothrow) ShopScene(hud, p, level);
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

void ShopScene::showShopHUD(Dungeon &dungeon, int x, int y) {
	// x and y are below the item's coordinates,
	// we want the price to show two above that.

	// pricing symbols, prices themselves, etc.
	itemprice = Label::createWithTTF("$", "fonts/Marker Felt.ttf", 24);
	itemprice->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - (y-2)) - Y_OFFSET);
	this->addChild(itemprice, 3);
	itemprice->setColor(cocos2d::Color3B(255, 215, 0));
	itemprice->setString("$" + std::to_string(dungeon[(y - 1)*dungeon.getCols() + x].price));
}
void ShopScene::deconstructShopHUD() {
	if (itemprice != nullptr) {
		itemprice->removeFromParent();
		itemprice = nullptr;
	}
}


//		PAUSE MENU SCENE
PauseMenuScene::PauseMenuScene(std::shared_ptr<Player> p, int id) : m_player(p), music_id(id) {

}
PauseMenuScene* PauseMenuScene::create(std::shared_ptr<Player> p, int id) {
	PauseMenuScene *pRet = new(std::nothrow) PauseMenuScene(p, id);
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

Scene* PauseMenuScene::createScene(std::shared_ptr<Player> p, int id) {
	auto scene = Scene::create();

	// calls init()
	auto layer = PauseMenuScene::create(p, id);
	scene->addChild(layer);

	return scene;
}
bool PauseMenuScene::init() {

	if (!Scene::init())
		return false;
	
	// pause music
	experimental::AudioEngine::pauseAll();

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;


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
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(vsw, vsh);
	box->setScale(.22f);
	box->setOpacity(170);
	sprites.insert(std::make_pair("Pause Menu", box));

	// Arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING + vsw, 2.5 * MENU_SPACING + (-index * MENU_SPACING) + vsh);
	this->addChild(sprite, 3);
	sprite->setScale(2.5f);
	sprites.insert(std::make_pair("Selector", sprite));

	// Pause option
	auto pause = Label::createWithTTF("PAUSE", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0 + vsw, 4.8 * MENU_SPACING + vsh);
	this->addChild(pause, 3);
	labels.insert(std::make_pair("PAUSE", pause));

	// Resume option
	auto resume = Label::createWithTTF("Resume", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0 + vsw, 2.5 * MENU_SPACING + vsh);
	this->addChild(resume, 3);
	labels.insert(std::make_pair("Resume", resume));

	// Restart option
	auto restart = Label::createWithTTF("Restart", "fonts/Marker Felt.ttf", 36);
	restart->setPosition(0 + vsw, 1.5 * MENU_SPACING + vsh);
	this->addChild(restart, 3);
	labels.insert(std::make_pair("Restart", restart));

	// Options
	auto options = Label::createWithTTF("Options", "fonts/Marker Felt.ttf", 36);
	options->setPosition(0 + vsw, 0.5 * MENU_SPACING + vsh);
	this->addChild(options, 3);
	labels.insert(std::make_pair("Options", options));

	// "How to play" option
	auto help = Label::createWithTTF("Help", "fonts/Marker Felt.ttf", 36);
	help->setPosition(0 + vsw, -0.5 * MENU_SPACING + vsh);
	this->addChild(help, 3);
	labels.insert(std::make_pair("Help", help));

	// Back to Menu option
	auto back = Label::createWithTTF("Main Menu", "fonts/Marker Felt.ttf", 36);
	back->setPosition(0 + vsw, -1.5 * MENU_SPACING + vsh);
	this->addChild(back, 3);
	labels.insert(std::make_pair("Main Menu", back));

	// Quit option
	auto exit = Label::createWithTTF("Exit Game", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0 + vsw, -2.5 * MENU_SPACING + vsh);
	this->addChild(exit, 3);
	labels.insert(std::make_pair("Exit", exit));


	pauseSelectListener = EventListenerKeyboard::create();
	pauseSelectListener->onKeyPressed = CC_CALLBACK_2(PauseMenuScene::pauseMenuKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(pauseSelectListener, sprite);

	return true;
}

void PauseMenuScene::pauseMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 5) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - MENU_SPACING);

			playInterfaceSound("Select 1.mp3");
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
			// Resume
		case 0: { 
			index = 0;
			experimental::AudioEngine::resumeAll();
			Director::getInstance()->popScene();

			return;
		}
			// Restart
		case 1: { 
			experimental::AudioEngine::stopAll(); // remove all sound before creating the next scene

			auto visibleSize = Director::getInstance()->getVisibleSize();
			
			// generate a new level 1 scene
			std::shared_ptr<Player> newPlayer(nullptr);
			if (m_player->getName() == ADVENTURER)
				newPlayer = std::make_shared<Adventurer>();
			else if (m_player->getName() == SPELLCASTER)
				newPlayer = std::make_shared<Spellcaster>();
			else if (m_player->getName() == MADMAN)
				newPlayer = std::make_shared<TheMadman>();
			else if (m_player->getName() == SPELUNKER)
				newPlayer = std::make_shared<Spelunker>();
			else if (m_player->getName() == ACROBAT)
				newPlayer = std::make_shared<TheMadman>();
			else
				newPlayer = std::make_shared<Adventurer>();

			auto level1Scene = LevelScene::createScene(newPlayer, 1);
			level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

			// advance to next scene
			//Director::getInstance()->popScene(); // pop original scene off stack
			Director::getInstance()->replaceScene(level1Scene); // replace with new scene
			return;
		}
			// Options	
		case 2: {
			playInterfaceSound("Confirm 1.mp3");

			options();

			break;
		}
			// Help menu
		case 3: { 
			playInterfaceSound("Confirm 1.mp3");

			//Director::getInstance()->pushScene(this);
			helpScreen();
			break;
		}
			// Main Menu
		case 4: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			playInterfaceSound("Confirm 1.mp3");

			// advance to start menu scene
			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
		case 5: // Exit Game
			playInterfaceSound("Confirm 1.mp3");

			Director::getInstance()->end();
			return;
		}
		break;
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		experimental::AudioEngine::resumeAll();
		Director::getInstance()->popScene();
		return;
	}
	default:
		break;
	}
}

void PauseMenuScene::helpScreen() {

	removeAll();

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float vsw = visibleSize.width / 2;
	float vsh = visibleSize.height / 2;

	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0 + vsw, 0 + vsh);
	box->setScale(.3f);
	box->setOpacity(170);
	sprites.insert(std::make_pair("Menu", box));

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING + vsw, -5.2 * MENU_SPACING + vsh);
	this->addChild(sprite, 3);
	sprite->setScale(2.5f);
	sprites.insert(std::make_pair("Arrow", sprite));

	// Go back
	auto resume = Label::createWithTTF("OK", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0 + vsw, -5.2 * MENU_SPACING + vsh);
	this->addChild(resume, 3);
	labels.insert(std::make_pair("OK", resume));

	// HOW TO PLAY
	auto pause = Label::createWithTTF("How to play", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0 + vsw, 5.0 * MENU_SPACING + vsh);
	this->addChild(pause, 3);
	labels.insert(std::make_pair("Help", pause));


	// Movement
	auto restart = Label::createWithTTF("Movement:", "fonts/Marker Felt.ttf", 28);
	restart->setPosition(-5 * MENU_SPACING + vsw, 2.85 * MENU_SPACING + vsh);
	restart->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(restart, 3);
	labels.insert(std::make_pair("Movement", restart));

	// Arrow keys
	auto arrowKeyUp = Sprite::create("KB_Arrows_U.png");
	arrowKeyUp->setPosition(0 * MENU_SPACING + vsw, 3.35 * MENU_SPACING + vsh);
	arrowKeyUp->setScale(0.8f);
	this->addChild(arrowKeyUp, 4);
	sprites.insert(std::make_pair("Arrow Up", arrowKeyUp));

	auto arrowKeys = Sprite::create("KB_Arrows_LDR.png");
	arrowKeys->setPosition(0 * MENU_SPACING + vsw, 2.6 * MENU_SPACING + vsh);
	arrowKeys->setScale(0.8f);
	this->addChild(arrowKeys, 4);
	sprites.insert(std::make_pair("Arrows", arrowKeys));


	// Interact
	auto uselabel = Label::createWithTTF("Grab/Interact:", "fonts/Marker Felt.ttf", 28);
	uselabel->setPosition(-5 * MENU_SPACING + vsw, 1.5 * MENU_SPACING + vsh);
	uselabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(uselabel, 3);
	labels.insert(std::make_pair("Interact", uselabel));

	auto use = Sprite::create("KB_Black_E.png");
	use->setPosition(0 * MENU_SPACING + vsw, 1.5 * MENU_SPACING + vsh);
	use->setScale(0.8f);
	this->addChild(use, 4);
	sprites.insert(std::make_pair("Interact Key", use));


	// Quick Use key
	auto item = Label::createWithTTF("Use Item:", "fonts/Marker Felt.ttf", 28);
	item->setPosition(-5 * MENU_SPACING + vsw, 0.5 * MENU_SPACING + vsh);
	item->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(item, 3);
	labels.insert(std::make_pair("Item", item));

	auto itemKey = Sprite::create("KB_Black_Q.png");
	itemKey->setPosition(0 * MENU_SPACING + vsw, 0.5 * MENU_SPACING + vsh);
	itemKey->setScale(0.8f);
	this->addChild(itemKey, 4);
	sprites.insert(std::make_pair("Item Key", itemKey));


	// Active Item
	auto shield = Label::createWithTTF("Use Active Item:", "fonts/Marker Felt.ttf", 28);
	shield->setPosition(-5 * MENU_SPACING + vsw, -0.5 * MENU_SPACING + vsh);
	shield->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(shield, 3);
	labels.insert(std::make_pair("Active", shield));

	auto spacebar = Label::createWithTTF("SPACE", "fonts/Marker Felt.ttf", 28);
	spacebar->setPosition(0 * MENU_SPACING + vsw, -0.5 * MENU_SPACING + vsh);
	spacebar->setTextColor(cocos2d::Color4B(255, 255, 255, 255));
	this->addChild(spacebar, 5);
	labels.insert(std::make_pair("Space", spacebar));

	auto shieldKey = Sprite::create("KB_Black_Space.png");
	shieldKey->setPosition(0 * MENU_SPACING + vsw, -0.5 * MENU_SPACING + vsh);
	shieldKey->setScale(0.8f);
	this->addChild(shieldKey, 4);
	sprites.insert(std::make_pair("Active Key", shieldKey));


	// Switch weapon
	auto weplabel = Label::createWithTTF("Switch Weapon:", "fonts/Marker Felt.ttf", 28);
	weplabel->setPosition(-5 * MENU_SPACING + vsw, -1.5 * MENU_SPACING + vsh);
	weplabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(weplabel, 3);
	labels.insert(std::make_pair("Weapon", weplabel));

	auto wepmenu = Sprite::create("KB_Black_W.png");
	wepmenu->setPosition(0 * MENU_SPACING + vsw, -1.5 * MENU_SPACING + vsh);
	wepmenu->setScale(0.8f);
	this->addChild(wepmenu, 4);
	sprites.insert(std::make_pair("Weapon Key", wepmenu));


	// Open/close item menu
	auto itemlabel = Label::createWithTTF("Open/Close Item Menu:", "fonts/Marker Felt.ttf", 28);
	itemlabel->setPosition(-5 * MENU_SPACING + vsw, -2.5* MENU_SPACING + vsh);
	itemlabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(itemlabel, 3);
	labels.insert(std::make_pair("Item Menu", itemlabel));

	auto itemmenu = Sprite::create("KB_Black_C.png");
	itemmenu->setPosition(0 * MENU_SPACING + vsw, -2.5 * MENU_SPACING + vsh);
	itemmenu->setScale(0.8f);
	this->addChild(itemmenu, 4);
	sprites.insert(std::make_pair("Item Menu Key", itemmenu));


	// View inventory
	auto inventory = Label::createWithTTF("Check inventory:", "fonts/Marker Felt.ttf", 28);
	inventory->setPosition(-5 * MENU_SPACING + vsw, -3.5 * MENU_SPACING + vsh);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(inventory, 3);
	labels.insert(std::make_pair("Inventory", inventory));

	auto inventoryKey = Sprite::create("KB_Black_Tab.png");
	inventoryKey->setPosition(0 * MENU_SPACING + vsw, -3.5 * MENU_SPACING + vsh);
	inventoryKey->setScale(0.8f);
	this->addChild(inventoryKey, 4);
	sprites.insert(std::make_pair("Inventory Key", inventoryKey));


	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(PauseMenuScene::helpMenuKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
}
void PauseMenuScene::helpMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		playInterfaceSound("Confirm 1.mp3");

		removeAll();

		init();
	}
	default: break;
	}
}


// Return a uniformly distributed random integer from 0 to limit-1 inclusive
int randInt(int limit) {
	return std::rand() % limit;
}

// Converts Key Code to string for use in labels
std::string convertKeycodeToStr(cocos2d::EventKeyboard::KeyCode keyCode) {

	std::string key;
	switch (keyCode) {
	case cocos2d::EventKeyboard::KeyCode::KEY_NONE:
		key = "None";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PAUSE:
		key = "Pause";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SCROLL_LOCK:
		key = "Scroll Lock";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PRINT:
		key = "Print";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SYSREQ:
		key = "SysReq";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_BREAK:
		key = "Break";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE:
		key = "Escape";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_BACKSPACE:
		key = "Backspace";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_TAB:
		key = "Tab";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_BACK_TAB:
		key = "Back Tab";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RETURN:
		key = "Return";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPS_LOCK:
		key = "Caps Lock";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_SHIFT:
		key = "Left Shift";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_SHIFT:
		key = "Right Shift";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_CTRL:
		key = "Left Ctrl";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_CTRL:
		key = "Right Ctrl";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ALT:
		key = "Left Alt";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ALT:
		key = "Right Alt";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_MENU:
		key = "Menu";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_HYPER:
		key = "Hyper";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_INSERT:
		key = "Insert";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_HOME:
		key = "Home";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PG_UP:
		key = "Page Up";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DELETE:
		key = "Delete";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_END:
		key = "End";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PG_DOWN:
		key = "Page Down";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		key = "Left Arrow";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		key = "Right Arrow";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
		key = "Up Arrow";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		key = "Down Arrow";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_NUM_LOCK:
		key = "Num Lock";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_PLUS:
		key = "Keypad Plus";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_MINUS:
		key = "Keypad Minus";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_MULTIPLY:
		key = "Keypad Multiply";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_DIVIDE:
		key = "Keypad Divide";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_ENTER:
		key = "Keypad Enter";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_HOME:
		key = "Keypad Home";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_UP:
		key = "Keypad Up";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_PG_UP:
		key = "Keypad Page Up";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_LEFT:
		key = "Keypad Left";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_FIVE:
		key = "Keypad 5";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_RIGHT:
		key = "Keypad Right";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_END:
		key = "Keypad End";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_DOWN:
		key = "Keypad Down";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_PG_DOWN:
		key = "Keypad Page Down";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_INSERT:
		key = "Keypad Insert";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_KP_DELETE:
		key = "Keypad Delete";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F1:
		key = "F1";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F2:
		key = "F2";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F3:
		key = "F3";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F4:
		key = "F4";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F5:
		key = "F5";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F6:
		key = "F6";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F7:
		key = "F7";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F8:
		key = "F8";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F9:
		key = "F9";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F10:
		key = "F10";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F11:
		key = "F11";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F12:
		key = "F12";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
		key = "Space";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_EXCLAM:
		key = "!";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_QUOTE:
		key = "\"";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_NUMBER:
		key = "#";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DOLLAR:
		key = "$";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PERCENT:
		key = "%";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CIRCUMFLEX:
		key = "^";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_AMPERSAND:
		key = "&";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_APOSTROPHE:
		key = "Apostrophe";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_PARENTHESIS:
		key = "(";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_PARENTHESIS:
		key = ")";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_ASTERISK:
		key = "*";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PLUS:
		key = "+";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_COMMA:
		key = ",";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_MINUS:
		key = "-";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PERIOD:
		key = ".";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SLASH:
		key = "/";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_0:
		key = "0";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_1:
		key = "1";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_2:
		key = "2";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_3:
		key = "3";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_4:
		key = "4";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_5:
		key = "5";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_6:
		key = "6";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_7:
		key = "7";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_8:
		key = "8";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_9:
		key = "9";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_COLON:
		key = ":";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SEMICOLON:
		key = ";";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LESS_THAN:
		key = "<";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_EQUAL:
		key = "=";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_GREATER_THAN:
		key = ">";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_QUESTION:
		key = "?";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_AT:
		key = "@";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_A:
		key = "A";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_B:
		key = "B";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_C:
		key = "C";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_D:
		key = "D";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_E:
		key = "E";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_F:
		key = "F";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_G:
		key = "G";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_H:
		key = "H";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_I:
		key = "I";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_J:
		key = "J";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_K:
		key = "K";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_L:
		key = "L";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_M:
		key = "M";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_N:
		key = "N";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_O:
		key = "O";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_P:
		key = "P";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_Q:
		key = "Q";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_R:
		key = "R";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_S:
		key = "S";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_T:
		key = "T";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_U:
		key = "U";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_V:
		key = "V";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_W:
		key = "W";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_X:
		key = "X";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_Y:
		key = "Y";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_CAPITAL_Z:
		key = "Z";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_BRACKET:
		key = "[";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_BACK_SLASH:
		key = "\\";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_BRACKET:
		key = "]";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_UNDERSCORE:
		key = "_";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_GRAVE:
		key = "`";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		key = "A";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_B:
		key = "B";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_C:
		key = "C";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		key = "D";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_E:
		key = "E";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F:
		key = "F";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_G:
		key = "G";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_H:
		key = "H";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_I:
		key = "I";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_J:
		key = "J";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_K:
		key = "K";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_L:
		key = "L";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_M:
		key = "M";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_N:
		key = "N";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_O:
		key = "O";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_P:
		key = "P";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_Q:
		key = "Q";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_R:
		key = "R";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		key = "S";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_T:
		key = "T";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_U:
		key = "U";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_V:
		key = "V";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		key = "W";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_X:
		key = "X";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_Y:
		key = "Y";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_Z:
		key = "Z";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_BRACE:
		key = "{";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_BAR:
		key = "Bar";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_BRACE:
		key = "}";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_TILDE:
		key = "`";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_EURO:
		key = "Euro";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_POUND:
		key = "Pound";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_YEN:
		key = "Yen";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_MIDDLE_DOT:
		key = "Middle Dot";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SEARCH:
		key = "Search";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT:
		key = "Dpad Left";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT:
		key = "Dpad Right";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP:
		key = "Dpad Up";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN:
		key = "Dpad Down";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_CENTER:
		key = "Dpad Center";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
		key = "Enter";
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_PLAY:
		key = "Play";
		break;
	}

	return key;
}

/*
bool LevelScene::isKeyPressed(EventKeyboard::KeyCode code) {
	// Check if the key is currently pressed by seeing it it's in the std::map keys
	// In retrospect, keys is a terrible name for a key/value paried datatype isnt it?
	if (keys.find(code) != keys.end())
		return true;
	return false;
}

double LevelScene::keyPressedDuration(EventKeyboard::KeyCode code) {
	if (!isKeyPressed(EventKeyboard::KeyCode::KEY_LEFT_ARROW))
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
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_LEFT_ARROW)) {
		std::stringstream ss;
		ss << "Control key has been pressed for " <<
			keyPressedDuration(EventKeyboard::KeyCode::KEY_LEFT_ARROW) << " ms";
		label->setString(ss.str().c_str());
	}
	else
		label->setString("Press the CTRL Key");
}
*/