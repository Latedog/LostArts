#include "GUI.h"
#include "AppDelegate.h"
#include "AudioEngine.h"
#include "ui/cocosGUI.h"
#include "utilities.h"
#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include <vector>

USING_NS_CC;

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
	//////////////////////////////
	// 1. super init first
	if (!Scene::init()) {
		return false;
	}


	//id = experimental::AudioEngine::play2d("Title Theme.mp3", true, 0.7f);
	id = experimental::AudioEngine::play2d("PetterTheSturgeon - Anything_1.waw_.mp3", true, 1.0f);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	auto title1 = Label::createWithTTF("Super Mini Rogue", "fonts/Marker Felt.ttf", 48);
	title1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + (1*MENU_SPACING)));
	title1->setTextColor(cocos2d::Color4B(224, 224, 224, 255));
	title1->enableOutline(cocos2d::Color4B(50, 55, 55, 255), 1);
	this->addChild(title1, 1);

	// player sprite
	/*playerSprite = Sprite::createWithSpriteFrameName("Player1_48x48.png");
	playerSprite->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 2*MENU_SPACING));
	this->addChild(playerSprite, 1);*/

	// background pic
	auto background = Sprite::create("super_pixel_cave_wallpaper_C.png");
	background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	background->setScale(0.55);
	this->addChild(background, -5);

	// arrow sprite for selection
	auto arrow = Sprite::create("Right_Arrow.png");
	arrow->setPosition(-2.5f * MENU_SPACING + visibleSize.width / 2, -1.2f * MENU_SPACING + visibleSize.height / 2);
	arrow->setScale(2.5);
	arrow->setColor(cocos2d::Color3B(255, 150, 200));
	this->addChild(arrow, 4);

	// start button
	auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
	float x = visibleSize.width / 2;
	float y = visibleSize.height / 2 - (1.2 * MENU_SPACING);
	start->setPosition(Vec2(x, y));
	start->setScale(0.8);
	this->addChild(start, 3);

	// exit button
	auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButton1.png");
	x = visibleSize.width / 2;
	y = visibleSize.height / 2 - 3 * MENU_SPACING;
	exitGame->setPosition(Vec2(x, y));
	this->addChild(exitGame, 3);

	// add keyboard event listener for actions
	auto kbListener = EventListenerKeyboard::create();
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

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	/*
	auto closeItem = MenuItemImage::create(
		"CloseNormal.png",
		"CloseSelected.png",
		CC_CALLBACK_1(StartScene::menuCloseCallback, this));

	if (closeItem == nullptr ||
		closeItem->getContentSize().width <= 0 ||
		closeItem->getContentSize().height <= 0)
	{
		problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
	}
	else
	{
		float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
		float y = origin.y + closeItem->getContentSize().height / 2;
		closeItem->setPosition(Vec2(x, y));
	}

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);
	*/
	/////////////////////////////
	// 3. add your codes below...

	// add a label shows "Hello World"
	// create and initialize a label
	/*
	auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
	if (label == nullptr)
	{
		problemLoading("'fonts/Marker Felt.ttf'");
	}
	else
	{
		// position the label on the center of the screen
		label->setPosition(Vec2(origin.x + visibleSize.width / 2,
			origin.y + visibleSize.height - label->getContentSize().height));

		// add the label as a child to this layer
		this->addChild(label, 1);
	}

	// add "HelloWorld" splash screen"
	auto sprite = Sprite::create("HelloWorld.png");
	if (sprite == nullptr)
	{
		problemLoading("'HelloWorld.png'");
	}
	else
	{
		// position the sprite on the center of the screen
		sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

		// add the sprite as a child to this layer
		this->addChild(sprite, 0);
	}
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
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 1.8f * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1.8f * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Start
			experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);
			index = 0;

			auto start = cocos2d::Sprite::createWithSpriteFrameName("StartButton1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - (1.2 * MENU_SPACING);
			start->setPosition(Vec2(x, y));
			start->setScale(0.8);
			this->addChild(start, 3);

			startGameCallback(this);

			return;
		}
		case 1: // Exit
			experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);
			index = 0;

			auto exitGame = cocos2d::Sprite::createWithSpriteFrameName("ExitButtonPressed1.png");
			float x = visibleSize.width / 2;
			float y = visibleSize.height / 2 - 3 * MENU_SPACING;
			exitGame->setPosition(Vec2(x, y));
			this->addChild(exitGame, 3);

			exitGameCallback(this);

			return;
		}
	default: break;
	}
}
void StartScene::startGameCallback(Ref* pSender)
{
	experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::stop(id);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// advance to next scene
	Player p;
	auto level1Scene = Level1Scene::createScene();
	level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	// switch to next scene
	Director::getInstance()->replaceScene(level1Scene);
	//Director::getInstance()->replaceScene(TransitionCrossFade::create(0.2, level1Scene));
	//Director::getInstance()->getRunningScene()->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

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
/*
HUDLayer::HUDLayer() {
	auto defaultCamera = Director::getInstance()->getRunningScene()->getDefaultCamera();
	//Director::getInstance()->getRunningScene()->setCameraMask((unsigned short)CameraFlag::USER2, true); // mask on the node
	defaultCamera->setCameraFlag(CameraFlag::USER2);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	std::string image;

	//		RENDER HUD
	image = "Health_Bar_Empty_Long.png";
	Sprite* healthbar = Sprite::createWithSpriteFrameName(image);
	this->addChild(healthbar, 2, "healthbar");
	healthbar->setPosition(-380, 170);
	//healthbar->setCameraMask((unsigned short)CameraFlag::USER2, true);
	HUD.insert(std::pair<std::string, Sprite*>("healthbar", healthbar));

	image = "Health_Points_Long.png";
	Sprite* hp = Sprite::createWithSpriteFrameName(image);
	this->addChild(hp, 3, "hp");
	hp->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	hp->setPosition(-526, 170);
	HUD.insert(std::pair<std::string, Sprite*>("hp", hp));

	image = "Current_Weapon_Box_1.png";
	Sprite* wepbox = Sprite::create(image);
	this->addChild(wepbox, 2, "wepbox");
	wepbox->setPosition(530, -195);
	wepbox->setScale(.2);
	wepbox->setOpacity(160);
	HUD.insert(std::pair<std::string, Sprite*>("wepbox", wepbox));

	//load default weapon sprite
	std::string weapon;
	weapon = DUNGEON.getPlayer().getWeapon().getAction();
	if (weapon == "Short Sword")
		image = "Short_Sword.png";
	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, 3, weapon);
	currentwep->setPosition(530, -195);
	currentwep->setScale(1.2);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));
}
*/

bool HUDLayer::init() {
	if (!Layer::init())
		return false;

	std::string image;

	// just used to initialize weapon
	Player p;

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

	numericalHP = Label::createWithTTF(std::to_string(p.getHP()) + "/" + std::to_string(p.getMaxHP()), "fonts/Marker Felt.ttf", 18);
	numericalHP->setPosition(-400 * RES_ADJUST, 300 * RES_ADJUST); // -160, 300
	//numericalHP->enableGlow(cocos2d::Color4B(250, 128, 114, 200));
	numericalHP->setOpacity(230);
	numericalHP->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	this->addChild(numericalHP, 4);

	str = Label::createWithTTF("Str: +" + std::to_string(p.getStr()), "fonts/Marker Felt.ttf", 16);
	str->setPosition(-570 * RES_ADJUST, 70 * RES_ADJUST);
	str->setOpacity(230);
	str->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	str->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	str->setAdditionalKerning(0.25f);
	this->addChild(str, 4);

	dex = Label::createWithTTF("Dex: +" + std::to_string(p.getDex()), "fonts/Marker Felt.ttf", 16);
	dex->setPosition(-570 * RES_ADJUST, 50 * RES_ADJUST);
	dex->setOpacity(230);
	dex->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	dex->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	dex->setAdditionalKerning(0.25f);
	this->addChild(dex, 4);

	intellect = Label::createWithTTF("Int: +" + std::to_string(p.getInt()), "fonts/Marker Felt.ttf", 16);
	intellect->setPosition(-570 * RES_ADJUST, 30 * RES_ADJUST);
	intellect->setOpacity(230);
	intellect->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	intellect->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	intellect->setAdditionalKerning(0.25f);
	this->addChild(intellect, 4);

	armor = Label::createWithTTF("Amr: +" + std::to_string(p.getArmor()), "fonts/Marker Felt.ttf", 16);
	armor->setPosition(-570 * RES_ADJUST, 10 * RES_ADJUST);
	armor->setOpacity(230);
	armor->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	armor->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	armor->setAdditionalKerning(0.25f);
	this->addChild(armor, 4);

	moneyBonus = Label::createWithTTF("Money Bonus : " + std::to_string((int)p.getMoneyBonus()), "fonts/Marker Felt.ttf", 15);
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
	wepbox->setScale(.2 * RES_ADJUST);
	wepbox->setOpacity(160);
	this->addChild(wepbox, 2, "wepbox");
	HUD.insert(std::pair<std::string, Sprite*>("wepbox", wepbox));

	// load default weapon sprite
	std::string weapon;
	weapon = p.getWeapon()->getAction();
	if (weapon == "Short Sword")
		image = "Short_Sword.png";
	Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentwep, 3, weapon);
	currentwep->setPosition(-570 * RES_ADJUST, 240 * RES_ADJUST);
	currentwep->setScale(1.2 * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));


	//		:::: RIGHT SIDE OF HUD ::::

	// gold count
	image = "Gold_Pile1_48x48.png";
	Sprite* goldpile = Sprite::createWithSpriteFrameName(image);
	this->addChild(goldpile, 3, "goldpile");
	goldpile->setPosition(570 * RES_ADJUST, 300 * RES_ADJUST); // -190, 300
	goldpile->setScale(0.8 * RES_ADJUST);
	goldpile->setOpacity(230);
	HUD.insert(std::pair<std::string, Sprite*>("goldpile", goldpile));

	goldcount = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 18);
	goldcount->setPosition(600 * RES_ADJUST, 300 * RES_ADJUST); // -160, 300
	this->addChild(goldcount, 3);

	return true;
}
void HUDLayer::updateHUD(Dungeon &dungeon) {
	p = dungeon.getPlayer();
	int x = p.getPosX(); int y = p.getPosY();

	// :::: Gold check ::::
	goldcount->setString(std::to_string(p.getMoney()));


	// :::: Weapon check ::::
	std::string weapon = p.getWeapon()->getAction();
	std::string image = p.getWeapon()->getImageName();

	// if current weapon equipped is different, switch the weapon sprite
	if (HUD.find("currentwep")->second->getName() != weapon) {
		//remove sprite
		HUD.find("currentwep")->second->removeFromParent();
		HUD.erase("currentwep");

		Sprite* currentwep = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentwep, 3, weapon);
		currentwep->setPosition(-570.f * RES_ADJUST, 240.f * RES_ADJUST);
		currentwep->setScale(0.6 * RES_ADJUST);
		HUD.insert(std::pair<std::string, Sprite*>("currentwep", currentwep));


		// if weapon has casting ability, add label
		if (p.getWeapon()->canBeCast()) {

			// if new weapon previously did not have the cast label, add it
			if (keyLabels.find("cast key") == keyLabels.end()) {
				auto castKey = Label::createWithTTF("S", "fonts/Marker Felt.ttf", 14);
				castKey->setPosition(-570.f * RES_ADJUST, 215.5f * RES_ADJUST);
				castKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
				castKey->setOpacity(255);
				this->addChild(castKey, 4);
				keyLabels.insert(std::pair<std::string, Label*>("cast key", castKey));
			}
		}
		// else remove the label if previous weapon could be cast
		else if (!p.getWeapon()->canBeCast() && keyLabels.find("cast key") != keyLabels.end()) {
			auto it = keyLabels.find("cast key");
			it->second->removeFromParent();
			keyLabels.erase(it);
		}
		
	}


	image = "";
	// :::: Shield check ::::
	if (p.hasShield()) {
		std::string shield = p.getShield()->getItem();
		image = p.getShield()->getImageName();

		// else if there is now a shield equipped, but there wasn't previously, construct the menu
		if (image != "" && HUD.find("currentshield") == HUD.end()) {
			constructShieldHUD();

			/*
			// shield HUD box
			Sprite* shieldbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
			shieldbox->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
			shieldbox->setScale(.2 * RES_ADJUST);
			shieldbox->setOpacity(160);
			shieldbox->setColor(cocos2d::Color3B(255, 175, 5));
			this->addChild(shieldbox, 2, "shieldbox");
			HUD.insert(std::pair<std::string, Sprite*>("shieldbox", shieldbox));

			// shield sprite
			Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
			this->addChild(currentshield, 3, shield);
			currentshield->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
			currentshield->setScale(0.5 * RES_ADJUST);
			HUD.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));

			// shield durability bar
			Sprite* shieldbar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
			this->addChild(shieldbar, 3, "shieldbar");
			shieldbar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 150 * RES_ADJUST);
			shieldbar->setScale(1.0 * RES_ADJUST);
			HUD.insert(std::pair<std::string, Sprite*>("shieldbar", shieldbar));

			// shield bar points
			Sprite* shieldpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
			this->addChild(shieldpoints, 4, "shieldpoints");
			shieldpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
			shieldpoints->setPosition(-583 * RES_ADJUST, 150 * RES_ADJUST);
			shieldpoints->setScale(0.4 * RES_ADJUST);
			HUD.insert(std::pair<std::string, Sprite*>("shieldpoints", shieldpoints));*/
		}
		// else if current shield equipped is different, switch the sprite
		else if (image != "" && HUD.find("currentshield")->second->getName() != shield) {
			//remove sprite
			HUD.find("currentshield")->second->removeFromParent();
			HUD.erase("currentshield");

			Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
			this->addChild(currentshield, 3, shield);
			currentshield->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
			currentshield->setScale(0.50f * RES_ADJUST);
			HUD.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
		}
	}
	else {
		// if there's no shield equipped and there was previously, deconstruct the shield HUD
		if (HUD.find("currentshield") != HUD.end()) {
			deconstructShieldHUD();
		}
	}

	/*
	//// else if there is now a shield equipped, but there wasn't previously, construct the menu
	//else if (image != "" && HUD.find("currentshield") == HUD.end()) {
	//	// shield HUD box
	//	Sprite* shieldbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	//	shieldbox->setPosition(-570, 180);
	//	shieldbox->setScale(.2);
	//	shieldbox->setOpacity(160);
	//	shieldbox->setColor(cocos2d::Color3B(255, 175, 5));
	//	this->addChild(shieldbox, 2, "shieldbox");
	//	HUD.insert(std::pair<std::string, Sprite*>("shieldbox", shieldbox));

	//	// shield sprite
	//	Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
	//	this->addChild(currentshield, 3, shield);
	//	currentshield->setPosition(-570, 180);
	//	currentshield->setScale(0.5);
	//	HUD.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));

	//	// shield durability bar
	//	Sprite* shieldbar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
	//	this->addChild(shieldbar, 3, "shieldbar");
	//	shieldbar->setPosition(-550, 150);
	//	shieldbar->setScale(1.0);
	//	HUD.insert(std::pair<std::string, Sprite*>("shieldbar", shieldbar));

	//	// shield bar points
	//	Sprite* shieldpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
	//	this->addChild(shieldpoints, 4, "shieldpoints");
	//	shieldpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	//	shieldpoints->setPosition(-583, 150);
	//	shieldpoints->setScale(0.4);
	//	HUD.insert(std::pair<std::string, Sprite*>("shieldpoints", shieldpoints));
	//}
	//// else if current shield equipped is different, switch the sprite
	//else if (image != "" && HUD.find("currentshield")->second->getName() != shield) {
	//	//remove sprite
	//	HUD.find("currentshield")->second->removeFromParent();
	//	HUD.erase("currentshield");

	//	Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
	//	this->addChild(currentshield, 3, shield);
	//	currentshield->setPosition(-570, 180);
	//	currentshield->setScale(0.50);
	//	HUD.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
	//}
	*/

	// if player has a shield, update its shield bar
	if (HUD.find("currentshield") != HUD.end()) {
		double sx_scale = dungeon.getPlayer().getShield()->getDurability() / (static_cast<double>(dungeon.getPlayer().getShield()->getMaxDurability()) * 1.0);
		cocos2d::Action* move = cocos2d::ScaleTo::create(.3, sx_scale, 1);
		HUD.find("shieldpoints")->second->runAction(move);
	}


	// :::: Trinket check ::::
	if (p.hasTrinket()) {
		std::string trinket = p.getTrinket()->getItem();
		image = p.getTrinket()->getImageName();

		// else if there is now a shield equipped, but there wasn't previously, construct the menu
		if (image != "" && HUD.find("current trinket") == HUD.end()) {
			// Trinket HUD box
			Sprite* trinketbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
			trinketbox->setPosition(-570 * RES_ADJUST, 110 * RES_ADJUST);
			trinketbox->setScale(.2 * RES_ADJUST);
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
	double x_scale = dungeon.getPlayer().getHP() / (static_cast<double>(dungeon.getPlayer().getMaxHP()) * 1.0);
	cocos2d::Action* move = cocos2d::ScaleTo::create(.3, x_scale, 1);
	HUD.find("hp")->second->runAction(move);

	numericalHP->setString(std::to_string(p.getHP()) + "/" + std::to_string(p.getMaxHP()));

	str->setString("Str: +" + std::to_string(p.getStr()));
	dex->setString("Dex: +" + std::to_string(p.getDex()));
	intellect->setString("Int: +" + std::to_string(p.getInt()));
	armor->setString("Amr: +" + std::to_string(p.getArmor()));
	moneyBonus->setString("Money Bonus : " + std::to_string((int)p.getMoneyBonus()));


	// :::: Quick access check ::::
	if (p.getItems().size() > 0 && quick == 0)
		quick = 1;

	if (quick == 1 && p.getItems().size() > 0) {
		std::string image = p.getItems().at(0)->getImageName();
		std::string item = p.getItems()[0]->getItem();

		// if there wern't any items before, construct menu again
		if (HUD.find("quick access") == HUD.end()) {
			constructItemHUD();

			Sprite* quickitem = Sprite::createWithSpriteFrameName(image);
			this->addChild(quickitem, 3, item);
			quickitem->setPosition(570.f * RES_ADJUST, 240.f * RES_ADJUST);
			quickitem->setScale(0.8);
			HUD.insert(std::pair<std::string, Sprite*>("quick item", quickitem));
		}
		// else if the item is different, switch the sprite
		else if (HUD.find("quick item") != HUD.end() && HUD.find("quick item")->second->getName() != item) {
			HUD.find("quick item")->second->removeFromParent();
			HUD.erase(HUD.find("quick item"));

			Sprite* quickitem = Sprite::createWithSpriteFrameName(image);
			this->addChild(quickitem, 3, item);
			quickitem->setPosition(570.f * RES_ADJUST, 240.f * RES_ADJUST);
			quickitem->setScale(0.8);
			HUD.insert(std::pair<std::string, Sprite*>("quick item", quickitem));
		}
		/*
		// remove old sprite, if it existed
		if (HUD.find("quick item") != HUD.end() && HUD.find("quick item")->second->getName() != item) {
			HUD.find("quick item")->second->removeFromParent();
			HUD.erase(HUD.find("quick item"));
		}

		// add box again if not there
		if (HUD.find("quick access") == HUD.end()) {
			constructItemHUD();
		}

		std::vector<std::shared_ptr<Drops>> iteminv = p.getItems();
		std::string image;
		int size = iteminv.size();

		image = iteminv.at(0)->getImageName();

		Sprite* quickitem = Sprite::createWithSpriteFrameName(image);
		this->addChild(quickitem, 3, item);
		quickitem->setPosition(570.f * RES_ADJUST, 240.f * RES_ADJUST);
		quickitem->setScale(0.8);
		HUD.insert(std::pair<std::string, Sprite*>("quick item", quickitem));*/
	}
	else {
		quick = 0;
		// if there's no item quick slotted but there was previously, deconstruct the HUD
		if (HUD.find("quick access") != HUD.end()) {
			deconstructItemHUD();
		}
	}
}
void HUDLayer::showShopHUD(Dungeon &dungeon, int x, int y) {
	// pricing symbols, prices themselves, etc.
	itemprice = Label::createWithTTF("$", "fonts/Marker Felt.ttf", 24);
	itemprice->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - y) - Y_OFFSET);
	this->addChild(itemprice, 3);
	itemprice->setColor(cocos2d::Color3B(255, 215, 0));
	itemprice->setString("$" + std::to_string(dungeon.getDungeon()[(y-1)*dungeon.getCols() + x].price));
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
			double y_scale = dungeon.getMonsters()[0]->getHP() / (static_cast<double>(dungeon.getMonsters()[0]->getMaxHP()) * 1.0);
			cocos2d::Action* move = cocos2d::ScaleTo::create(.3, 1, y_scale);
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

void HUDLayer::NPCInteraction(cocos2d::EventListenerKeyboard* listener, Dungeon &dungeon, std::vector<std::string> dialogue) {
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
	eventListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::NPCKeyPressed, this, &dungeon);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, line);
}
void HUDLayer::NPCKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event, Dungeon *dungeon) {

	// if there's nothing left to say, remove text and restore control
	if (lineIndex >= m_dialogue.size()) {
		experimental::AudioEngine::play2d("Confirm 1.mp3", false, 0.5f);

		lineIndex = 0;
		line->removeFromParent();
		line = nullptr;
		enableListener();
		return;
	}

	switch (keyCode) {
	default: {
		if (!m_dialogue.empty() && lineIndex < m_dialogue.size()) {
			experimental::AudioEngine::play2d("Confirm 1.mp3", false, 0.5f);
			line->setString(m_dialogue[lineIndex]);
			lineIndex++;
		}
	}
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
	box->setScale(.2);
	box->setOpacity(170);
	weaponMenuSprites.insert(std::pair<std::string, Sprite*>("box", box));

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0);
	weaponMenuSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	// Pause option
	auto pause = Label::createWithTTF("The waters give off a strange aura. Take a drink from the fountain?", "fonts/Marker Felt.ttf", 40);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);
	labels.insert(std::pair<std::string, Label*>("pause", pause));

	// NO
	auto no = Label::createWithTTF("NO", "fonts/Marker Felt.ttf", 36);
	no->setPosition(0, 2 * MENU_SPACING);
	this->addChild(no, 3);
	labels.insert(std::pair<std::string, Label*>("no", no));

	// YES
	auto yes = Label::createWithTTF("YES", "fonts/Marker Felt.ttf", 36);
	yes->setPosition(0, -1 * MENU_SPACING);
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
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // NO
			experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			index = 0;
			deconstructMenu(weaponMenuSprites);

			//DUNGEON2.devilsWater(false);
			SecondFloor* second = dynamic_cast<SecondFloor*>(dungeon);
			second->devilsWater(false);

			enableListener();
			return;
		}
		case 1: // YES
			experimental::AudioEngine::play2d("Devils_Gift.mp3", false, 1.0f);

			// drink the water
			//DUNGEON2.devilsWater(true);
			SecondFloor* second = dynamic_cast<SecondFloor*>(dungeon);
			second->devilsWater(true);

			index = 0;
			deconstructMenu(weaponMenuSprites);

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
	std::vector<std::shared_ptr<Weapon>> wepinv = p.getWeapons();
	std::vector<std::shared_ptr<Drops>> iteminv = p.getItems();

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
	longBox->setScale(.5);
	longBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("long box", longBox));

	// menu border
	Sprite* wideBox = Sprite::create("Inventory_Box_Wide_Medium.png");
	this->addChild(wideBox, 3);
	wideBox->setPosition(150 * RES_ADJUST, 75 * RES_ADJUST);
	wideBox->setScale(.5);
	wideBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("wide box", wideBox));

	// menu border
	Sprite* textBox = Sprite::create("Inventory_Box_Text_Medium.png");
	this->addChild(textBox, 3);
	textBox->setPosition(130 * RES_ADJUST, -200 * RES_ADJUST);
	textBox->setScale(.45);
	textBox->setOpacity(170);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("text box", textBox));

	// menu border
	Sprite* selectBox = Sprite::create("Selection_Box.png");
	this->addChild(selectBox, 5);
	selectBox->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);
	selectBox->setScale(.1);
	selectBox->setOpacity(200);
	selectBox->setColor(cocos2d::Color3B(255, 200, 100));
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("select box", selectBox));

	// items to display
	std::string image;
	if (!wepinv.empty()) {
		for (int i = 0; i < wepinv.size(); i++) {
			inventoryText[i + 2] = std::make_pair(wepinv[i]->getItem(), wepinv[i]->getDescription());
			image = wepinv.at(i)->getImageName();

			Sprite* weapon = Sprite::createWithSpriteFrameName(image);
			this->addChild(weapon, 4);
			weapon->setScale(1.0);
			weapon->setPosition((i * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);
			inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));
		}
	}

	image = p.getWeapon()->getImageName();
	inventoryText[0] = std::make_pair(p.getWeapon()->getItem(), p.getWeapon()->getDescription());

	Sprite* weapon = Sprite::createWithSpriteFrameName(image);
	this->addChild(weapon, 4);
	weapon->setScale(1.0);
	weapon->setPosition((-350) * RES_ADJUST, 150 * RES_ADJUST);
	inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("weapon", weapon));

	itemName = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	itemName->setPosition(130 * RES_ADJUST, -150 * RES_ADJUST);
	itemName->setOpacity(230);
	itemName->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	itemName->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	itemName->setAdditionalKerning(0.25f);
	this->addChild(itemName, 5);
	labels.insert(std::make_pair("item name", itemName));

	itemDescription = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	itemDescription->setPosition(130 * RES_ADJUST, -170 * RES_ADJUST);
	itemDescription->setAnchorPoint(Vec2(0.5, 1.0));
	itemDescription->setOpacity(230);
	itemDescription->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	itemDescription->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	itemDescription->setAdditionalKerning(0.25f);
	this->addChild(itemDescription, 5);
	labels.insert(std::make_pair("item description", itemDescription));

	// display first line
	itemName->setString(inventoryText[0].first);
	itemDescription->setString(inventoryText[0].second);

	if (p.hasShield()) {
		std::string shield = p.getShield()->getItem();
		inventoryText[1] = std::make_pair(shield, p.getShield()->getDescription());

		image = p.getShield()->getImageName();

		// shield sprite
		Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentshield, 4, shield);
		currentshield->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);
		currentshield->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));
	}

	if (p.hasTrinket()) {
		std::string trinket = p.getTrinket()->getItem();
		inventoryText[12] = std::make_pair(trinket, p.getTrinket()->getDescription());

		image = p.getTrinket()->getImageName();

		// trinket sprite
		Sprite* currentTrinket = Sprite::createWithSpriteFrameName(image);
		this->addChild(currentTrinket, 4, trinket);
		currentTrinket->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);
		currentTrinket->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("current trinket", currentTrinket));

	}

	for (int i = 0; i < iteminv.size(); i++) {
		inventoryText[i + 7] = std::make_pair(iteminv[i]->getItem(), iteminv[i]->getDescription());
		image = iteminv.at(i)->getImageName();

		Sprite* item = Sprite::createWithSpriteFrameName(image);
		this->addChild(item, 4);
		item->setPosition((1.9 * i * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);
		item->setScale(1.0);
		inventoryMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));
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

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index == 0) {
			index = 6; // end of weapons
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 1) {
			index--;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(-250 * RES_ADJUST, 150 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index >= 3 && index <= 6) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 7) {
			index = 12;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index > 7 && index <= 11) {
			index--;
			event->getCurrentTarget()->setPosition(((index - 7) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 12) {
			index = 11; // end of items
			event->getCurrentTarget()->setPosition(((index - 7) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition((pos.x + 127) * RES_ADJUST, pos.y);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index >= 2 && index < 6) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 6) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index >= 7 && index < 11) {
			index++;
			event->getCurrentTarget()->setPosition(((index - 7) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 11) {
			index = 12;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 12) {
			index = 7;
			event->getCurrentTarget()->setPosition(((index - 7) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		if (index == 12 || index == 13) {
			index = 0;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 150 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index >= 7 && index <= 11) {
			index -= 5;
			event->getCurrentTarget()->setPosition(((index - 2) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 180 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index == 0 || index == 1) {
			index = 12;
			event->getCurrentTarget()->setPosition(-350 * RES_ADJUST, 50 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index >= 2 && index <= 6) {
			index += 5;
			event->getCurrentTarget()->setPosition(((index - 7) * 1.9 * MENU_SPACING - 90) * RES_ADJUST, 100 * RES_ADJUST);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}

		itemName->setString(inventoryText[index].first);
		itemDescription->setString(inventoryText[index].second);

		break;
	}
	case EventKeyboard::KeyCode::KEY_I:
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(inventoryMenuSprites);

		// clear inventory text
		for (int i = 0; i < 14; i++) {
			inventoryText[i].first = "";
			inventoryText[i].second = "";
		}

		// if there are any lingering actions, finish them instantly
		//auto actions = this->getActionManager();
		//while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
		//	actions->update(1.0);
		//}

		//dungeon->peekDungeon(dungeon->getPlayer().getPosX(), dungeon->getPlayer().getPosY(), '-');

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
	std::vector<std::shared_ptr<Weapon>> wepinv = p.getWeapons();

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
		weapon->setScale(0.6);
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

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 0 && p.getWeapons().size() > 1) {
			index = p.getWeapons().size() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index < (int)p.getWeapons().size() - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == p.getWeapons().size() - 1 && p.getWeapons().size() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ENTER: {
		cocos2d::experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);
		if (p.getWeapons().size() != 0) {
			dungeon->player.at(0).wield(index);
		}
	}
	case EventKeyboard::KeyCode::KEY_W:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(weaponMenuSprites);

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		switch (dungeon->getLevel()) {
		case 1: 
		case 2: 
		case 3: 
		case 4: 
		case 5: {
			
			dungeon->peekDungeon(dungeon->getPlayer().getPosX(), dungeon->getPlayer().getPosY(), '-');

			int px = dungeon->getPlayer().getPosX() - p.getPosX();
			int py = dungeon->getPlayer().getPosY() - p.getPosY();

			// play step sound effect if player moved
			if (px != 0 || py != 0) {
				playFootstepSound();
			}
			
			break;
		}
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
	std::vector<std::shared_ptr<Drops>> iteminv = p.getItems();

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
		item->setScale(0.8);
		itemMenuSprites.insert(std::pair<std::string, Sprite*>("item", item));
	}

	// arrow sprite for selection
	auto sprite = Sprite::create("Down_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, MENU_SPACING);
	sprite->setScale(2.0);
	this->addChild(sprite, 4);
	itemMenuSprites.insert(std::pair<std::string, Sprite*>("sprite", sprite));

	// helper labels
	auto equip = Label::createWithTTF("Q: Assign to quick access", "fonts/Marker Felt.ttf", 20);
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
	int x = p.getPosX();
	int y = p.getPosY();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		if (index > 0) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x - MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 0 && p.getItems().size() > 1) {
			index = p.getItems().size() - 1;
			event->getCurrentTarget()->setPosition(pos.x + index * MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		if (index < (int)p.getItems().size() - 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x + MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == p.getItems().size() - 1 && p.getItems().size() > 1) {
			index = 0;
			event->getCurrentTarget()->setPosition(-2 * MENU_SPACING, MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_Q: {
		cocos2d::experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		// if player has items, set this to new quick item slot
		if (dungeon->getPlayer().getItems().size() > 0) {
			quick = 1;
			dungeon->getPlayerVector()[0].setItemToFront(index);
		}

		index = 0;
		deconstructMenu(itemMenuSprites);

		dungeon->peekDungeon(dungeon->getPlayer().getPosX(), dungeon->getPlayer().getPosY(), '-');

		// play step sound effect if player moved
		int px = dungeon->getPlayer().getPosX() - p.getPosX();
		int py = dungeon->getPlayer().getPosY() - p.getPosY();

		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		updateHUD(*dungeon);
		enableListener();
		return;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ENTER: {
		cocos2d::experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

		// if there are any lingering actions, finish them instantly
		auto actions = this->getActionManager();
		while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		if (p.getItems().size() != 0) {
			dungeon->callUse(dungeon->getDungeon(), x, y, index); // use item
		}

		// if there are no more items, then there's no more quick access item
		if (dungeon->getPlayer().getItems().size() == 0)
			quick = 0; // reset quick if first item is used
	}
	case EventKeyboard::KeyCode::KEY_C:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		index = 0;
		deconstructMenu(itemMenuSprites);

		//// if there are any lingering actions, finish them instantly
		//auto actions = this->getActionManager();
		//while (actions->getNumberOfRunningActions() > 1) { // >1 because follow player is always running
		//	actions->update(1.0);
		//}

		switch (dungeon->getLevel()) {
		case 1: 
		case 2: 
		case 3:
		case 4: 
		case 5: {
			
			dungeon->peekDungeon(dungeon->getPlayer().getPosX(), dungeon->getPlayer().getPosY(), '-');
			
			int px = dungeon->getPlayer().getPosX() - p.getPosX();
			int py = dungeon->getPlayer().getPosY() - p.getPosY();

			// play step sound effect if player moved
			if (px != 0 || py != 0) {
				playFootstepSound();
			}

			break;
		}
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
	cocos2d::experimental::AudioEngine::play2d("Fallen in Battle.mp3", false);


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
	box->setScale(.2);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0);

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
	cocos2d::experimental::AudioEngine::play2d("Fallen in Battle.mp3", false);


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
	box->setScale(.2);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0);

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

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 2) {
			index--;
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index == 0) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 1 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		else if (index == 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	//case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Back to main menu
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			auto visibleSize = Director::getInstance()->getVisibleSize();

			// advance to start menu scene
			auto startScene = StartScene::createScene();
			//startScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
		case 1: { // Restart

			// generates a new dungeon and replaces the current one
			
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			// generate a new level 1 scene
			auto visibleSize = Director::getInstance()->getVisibleSize();
			Vec2 origin = Director::getInstance()->getVisibleOrigin();

			// advance to next scene
			auto level1Scene = Level1Scene::createScene();
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
	box->setScale(.2);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0);

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
	exit->setPosition(0, -1 * MENU_SPACING);
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
			event->getCurrentTarget()->setPosition(pos.x, pos.y + 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 1) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - 3 * MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Restart
			// generates a new dungeon and replaces the current one
			Dungeon d;
			//DUNGEON = d;

			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			// generate a new level 1 scene
			auto visibleSize = Director::getInstance()->getVisibleSize();
			Vec2 origin = Director::getInstance()->getVisibleOrigin();

			// advance to next scene
			auto level1Scene = Level1Scene::createScene();
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
void HUDLayer::constructShieldHUD() {
	std::string shield = p.getShield()->getItem();
	std::string image = p.getShield()->getImageName();

	// shield HUD box
	Sprite* shieldbox = Sprite::createWithSpriteFrameName("Current_Weapon_Box_1.png");
	shieldbox->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
	shieldbox->setScale(.2 * RES_ADJUST);
	shieldbox->setOpacity(160);
	shieldbox->setColor(cocos2d::Color3B(255, 175, 5));
	this->addChild(shieldbox, 2, "shieldbox");
	HUD.insert(std::pair<std::string, Sprite*>("shieldbox", shieldbox));

	// shield sprite
	Sprite* currentshield = Sprite::createWithSpriteFrameName(image);
	this->addChild(currentshield, 3, shield);
	currentshield->setPosition(-570 * RES_ADJUST, 180 * RES_ADJUST);
	currentshield->setScale(0.5 * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("currentshield", currentshield));

	// shield durability bar
	Sprite* shieldbar = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Empty2.png");
	this->addChild(shieldbar, 3, "shieldbar");
	shieldbar->setPosition(-550 * RES_ADJUST + SP_ADJUST, 150 * RES_ADJUST);
	shieldbar->setScale(1.0 * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("shieldbar", shieldbar));

	// shield bar points
	Sprite* shieldpoints = Sprite::createWithSpriteFrameName("Shield_Durability_Bar_Points2.png");
	this->addChild(shieldpoints, 4, "shieldpoints");
	shieldpoints->setAnchorPoint(Vec2(0, 0.5)); // set anchor point to left side
	shieldpoints->setPosition(-583 * RES_ADJUST, 150 * RES_ADJUST);
	shieldpoints->setScale(0.4 * RES_ADJUST);
	HUD.insert(std::pair<std::string, Sprite*>("shieldpoints", shieldpoints));

	// SPACE label
	auto space = Label::createWithTTF("Space", "fonts/Marker Felt.ttf", 14);
	space->setPosition(-570.f * RES_ADJUST, 162.f * RES_ADJUST);
	space->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	space->setOpacity(245);
	this->addChild(space, 4);
	keyLabels.insert(std::pair<std::string, Label*>("space", space));
}
void HUDLayer::deconstructShieldHUD() {
	if (HUD.find("shieldbox") != HUD.end()) {
		// deconstruct the shield HUD because there's no shield equipped
		HUD.find("currentshield")->second->removeFromParent();
		HUD.find("shieldbox")->second->removeFromParent();
		HUD.find("shieldbar")->second->removeFromParent();
		HUD.find("shieldpoints")->second->removeFromParent();
		keyLabels.find("space")->second->removeFromParent();

		HUD.erase(HUD.find("currentshield"));
		HUD.erase(HUD.find("shieldbox"));
		HUD.erase(HUD.find("shieldbar"));
		HUD.erase(HUD.find("shieldpoints"));
		keyLabels.erase(keyLabels.find("space"));
	}
}
void HUDLayer::constructItemHUD() {
	// Quick access item use slot
	std::string image = "Current_Weapon_Box_1.png";
	Sprite* quickAccess = Sprite::createWithSpriteFrameName(image);
	quickAccess->setPosition(570 * RES_ADJUST, 240 * RES_ADJUST);
	quickAccess->setScale(.2 * RES_ADJUST);
	quickAccess->setOpacity(160);
	quickAccess->setColor(cocos2d::Color3B(250, 188, 165));
	this->addChild(quickAccess, 2, "quick access");
	HUD.insert(std::pair<std::string, Sprite*>("quick access", quickAccess));

	// key label
	auto qKey = Label::createWithTTF("Q", "fonts/Marker Felt.ttf", 14);
	qKey->setPosition(570.f * RES_ADJUST, 217.f * RES_ADJUST);
	qKey->enableOutline(cocos2d::Color4B(0, 0, 0, 255), 1);
	qKey->setOpacity(245);
	this->addChild(qKey, 4);
	keyLabels.insert(std::pair<std::string, Label*>("qKey", qKey));
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
		// deconstruct the trinket HUD because there's no shield equipped
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


//		LEVEL 1 SCENE
Level1Scene::Level1Scene(HUDLayer* hud) : m_hud(hud) {
	cocos2d::experimental::AudioEngine::setMaxAudioInstance(200);
}
Scene* Level1Scene::createScene()
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create();
	scene->addChild(hud, 10);

	// create background layer
	BackgroundLayer* bglayer = BackgroundLayer::create();
	scene->addChild(bglayer, -10);

	// calls Level1Scene init()
	auto layer = Level1Scene::create(hud);
	scene->addChild(layer);

	return scene;
}
Level1Scene* Level1Scene::create(HUDLayer* hud)
{
	Level1Scene *pRet = new(std::nothrow) Level1Scene(hud);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool Level1Scene::init()
{
	if (!Scene::init()) {
		return false;
	}

	Dungeons dungeons;
	m_dungeons = dungeons;

	m_dungeons.DUNGEON1 = &DUNGEON;


	// music
	//id = experimental::AudioEngine::play2d("Abandoned Hopes.mp3", true);
	bg_music_id = experimental::AudioEngine::play2d("Exploring a cave.mp3", true, 0.8f);
	cocos2d::experimental::AudioEngine::setMaxAudioInstance(200);

	Vec2 origin = Director::getInstance()->getVisibleOrigin();
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
	m_player = Sprite::create("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 2);
	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));

	m_player->visit();

	// Render all the sprites on the first floor
	int px = 0, py = 0; // player's position; to be set by renderDungeon
	//renderDungeon(DUNGEON, MAXROWS, MAXCOLS, px, py);
	renderDungeon(*(m_dungeons.DUNGEON1), m_dungeons.DUNGEON1->getRows(), m_dungeons.DUNGEON1->getCols(), px, py);
	m_player->setPosition(px, py);

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


	renderTexture->end();

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

	// gives the sprite vectors to the dungeon
	/*DUNGEON.setPlayerSprite(m_player);
	DUNGEON.getPlayerVector().at(0).setSprite(m_player);

	DUNGEON.setItemSprites(items);
	DUNGEON.setMoneySprites(money);
	DUNGEON.setWallSprites(walls);
	DUNGEON.setScene(this);*/

	//
	m_dungeons.DUNGEON1->setPlayerSprite(m_player);
	m_dungeons.DUNGEON1->getPlayerVector().at(0).setSprite(m_player);

	m_dungeons.DUNGEON1->setItemSprites(items);
	m_dungeons.DUNGEON1->setMoneySprites(money);
	m_dungeons.DUNGEON1->setWallSprites(walls);
	m_dungeons.DUNGEON1->setDoorSprites(doors);
	m_dungeons.DUNGEON1->setScene(this);

	m_dungeons.DUNGEON1->updateLighting();

	// add keyboard event listener for actions
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(Level1Scene::Level1KeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player); // check this for player

	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON1->getLevel() == 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON1->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON1->peekDungeon(m_dungeons.DUNGEON1->getPlayer().getPosX(), m_dungeons.DUNGEON1->getPlayer().getPosY(), '-');

		// Player can be pushed around (by springs), so need to update to true location by
		// taking (new player pos - old player pos) * SpacingFactor
		int px = m_dungeons.DUNGEON1->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON1->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}
		//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px * SPACING_FACTOR, -py * SPACING_FACTOR));
		//m_player->runAction(move);

		// update HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON1);

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON1->getPlayer().getHP() <= 0) {
			m_hud->gameOver(*this);
		}
	}, this, 0.75, false, "level 1 timer");

	return true;
}
void Level1Scene::renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y) {
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


	std::string image;
	
	// BEGIN DUNGEON RENDER
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand;
	int z;
	for (int i = 0; i < maxrows; i++) {
		for (int j = 0; j < maxcols; j++) {
			tile = &maze[i*maxcols + j];

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

			Sprite* floor = createSprite(image, maxrows, j, i, -10);
			floor->setOpacity(252);
			floor->setColor(Color3B(0, 0, 0));
			
			dungeon[i*maxcols + j].floor = floor;

			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (maxrows - i) - Y_OFFSET;
			}
			if (tile->npc) {
				int pos = dungeon.findNPC(j, i);
				if (pos != -1) {
					image = dungeon.getNPCs()[pos]->getImageName();
					Sprite* npc = createSprite(image, maxrows, j, i, 1);
					dungeon.getNPCs()[pos]->setSprite(npc);
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
				Sprite* gold = createSprite(image, maxrows, j, i, -3);
				money.push_back(gold);
			}
			if (tile->wall) {
				switch (tile->top) {
				case WALL: {
					rand = randInt(13) + 1;
					switch (rand) {
						/*case 1: image = "Wall_Face1.png"; break;
						case 2: image = "Wall_Face2.png"; break;
						case 3: image = "Wall_Face3.png"; break;
						case 4: image = "Wall_Face4.png"; break;*/
						/*case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;*/
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
					break;
				}
				case UNBREAKABLE_WALL: {
					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == maxcols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == maxrows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
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

						/*switch (rand) {
						case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain19_A.png"; break;
						case 11: image = "Wall_Terrain20_A.png"; break;
						case 12: image = "Wall_Terrain21_A.png"; break;
						case 13: image = "Wall_Terrain22_A.png"; break;*/
						/*case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;
						case 13: image = "Wall_Terrain13_A.png"; break;
						case 14: image = "Wall_Terrain14_A.png"; break;
						case 15: image = "Wall_Terrain15_A.png"; break;
						case 16: image = "Wall_Terrain16_A.png"; break;
						case 17: image = "Wall_Terrain17_A.png"; break;
						case 18: image = "Wall_Terrain18_A.png"; break;*/
						//case 19: image = "Wall_Terrain19_A.png"; break;
						//case 20: image = "Wall_Terrain20_A.png"; break;
						//case 21: image = "Wall_Terrain21_A.png"; break;
						//case 22: image = "Wall_Terrain22_A.png"; break;
					}
					/*
					switch (rand) {
					case 1: image = "Wall_Terrain1.png"; break;
					case 2: image = "Wall_Terrain2.png"; break;
					case 3: image = "Wall_Terrain3.png"; break;
					case 4: image = "Wall_Terrain4.png"; break;
					case 5: image = "Wall_Terrain5.png"; break;
					case 6: image = "Wall_Terrain6.png"; break;
					case 7: image = "Wall_Terrain7.png"; break;
					case 8: image = "Wall_Terrain8.png"; break;
					case 9: image = "Wall_Terrain9.png"; break;
					case 10: image = "Wall_Terrain10.png"; break;
					case 11: image = "Wall_Terrain11.png"; break;
					case 12: image = "Wall_Terrain12.png"; break;
					case 13: image = "Wall_Terrain13.png"; break;
					}*/
					break;
				}
				case DOOR_V: image = "Door_Vertical_Closed_48x48.png"; break;
				case DOOR_H: image = "Door_Horizontal_Closed_48x48.png"; break;
				}
				if (tile->top == WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);

					//wall->setColor(Color3B(210, 200, 255));
					wall->setColor(Color3B(0, 0, 0));
					walls.push_back(wall);
				}
				else if (tile->top == UNBREAKABLE_WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);
					wall->setColor(Color3B(170, 90, 40));
					//wall->setColor(Color3B(0, 0, 0));
				}
				else if (tile->top == DOOR_H || tile->top == DOOR_V) {
					Sprite* door = createSprite(image, maxrows, j, i, 1);
					doors.push_back(door);
				}
			}
			if (tile->enemy) {

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
				Sprite* monsterSprite = createSprite(image, maxrows, j, i, 1);

				dungeon.getMonsters()[pos]->setSprite(monsterSprite);
			}
			if (tile->item) {
				image = tile->object->getImageName();

				Sprite* object = createSprite(image, maxrows, j, i, -1);
				items.push_back(object);
			}
			if (tile->trap) {

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
					dungeon.getDungeon()[i*maxcols + j].trap = false;
				}
				else if (tile->trap_name == TURRET) {
					z = 1;
				}
				else {
					z = -4;
				}
				
				image = dungeon.getTraps()[dungeon.findTrap(j, i)]->getImageName();
				Sprite* trap = createSprite(image, maxrows, j, i, z);
				if (tile->trap_name == SPRING) {
					trap->setScale(0.5);
				}

				int pos = dungeon.findTrap(j, i);
				if (pos != -1)
					dungeonTraps.at(pos)->setSprite(trap);
			}

		}
	}

	//dungeon.updateLighting();
}
cocos2d::Sprite* Level1Scene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	// Screen real dimensions
	auto vSize = Director::getInstance()->getVisibleSize();
	auto vWidth = vSize.width;
	auto vHeight = vSize.height;

	// Original image
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);

	sprite->visit();

	this->addChild(sprite, z);
	//graySprite(sprite);

	return sprite;

	//// Create new Sprite without scale, which perfoms much better
	//auto newSprite = Sprite::createWithTexture(renderTexture->getSprite()->getTexture());
	//newSprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	//addChild(newSprite, z);
}
/*
void Level1Scene::graySprite(Sprite* sprite) {
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
}
EffectSprite* Level1Scene::createEffectSprite(std::string image, int maxrows, int x, int y, int z) {
	auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	EffectSprite* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	//sprite->visit();
	this->addChild(sprite, z);

	sprite->setEffect(m_lighting, "1_Spritesheet_48x48.png");

	return sprite;
}
*/

void Level1Scene::Level1KeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	// unschedule the inaction timer
	Director::getInstance()->getScheduler()->unschedule("level 1 timer", this);

	int x, y;
	char c;

	auto actions = this->getActionManager();

	// resumes follow if it was paused
	if (actions->getNumberOfRunningActions() == 0) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	}
	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON1->getLevel() == 1) { // >1 because follow player is always running
		actions->update(1.0);
	}


	p = m_dungeons.DUNGEON1->getPlayer();
	x = p.getPosX(); y = p.getPosY();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'l');
		
		if (p.facingDirection() != 'l') {
			m_player->setScaleX(-1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'r');
		
		if (p.facingDirection() != 'r') {
			m_player->setScaleX(1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'u');
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'd');
		
		break;
	}

	//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_dungeons.DUNGEON1->getPlayer().getItems().size() > 0)
			m_dungeons.DUNGEON1->callUse(m_dungeons.DUNGEON1->getDungeon(), x, y, 0);

		m_dungeons.DUNGEON1->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: { // for items with special actions
		m_dungeons.DUNGEON1->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_dungeons.DUNGEON1);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_dungeons.DUNGEON1->peekDungeon(x, y, 'e');
		if (m_dungeons.DUNGEON1->getLevel() == 2) {
			advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		kbListener->setEnabled(false);
		m_hud->weaponMenu(kbListener, *m_dungeons.DUNGEON1);

		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_dungeons.DUNGEON1);

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
		Director::getInstance()->pushScene(this);
		pauseMenu();
		break;
	default:
		m_dungeons.DUNGEON1->peekDungeon(x, y, '-');
		break;
	}

	// Player can be pushed around (by springs), so need to update to true location
	int px = m_dungeons.DUNGEON1->getPlayer().getPosX() - x;
	int py = m_dungeons.DUNGEON1->getPlayer().getPosY() - y;

	// play step sound effect if player moved
	if (px != 0 || py != 0) {
		playFootstepSound();
	}


	// update the HUD
	m_hud->updateHUD(*m_dungeons.DUNGEON1);


	// Check if player is dead, if so, run game over screen
	if (m_dungeons.DUNGEON1->getPlayer().getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(bg_music_id);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}

	// reschedule the inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON1->getLevel() == 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON1->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON1->peekDungeon(m_dungeons.DUNGEON1->getPlayer().getPosX(), m_dungeons.DUNGEON1->getPlayer().getPosY(), '-');

		// Player can be pushed around (by springs), so need to update to true location
		int px = m_dungeons.DUNGEON1->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON1->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		// update HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON1);

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON1->getPlayer().getHP() <= 0) {
			m_hud->gameOver(*this);
			//Director::getInstance()->getScheduler()->unschedule("level 1 timer", this);
		}
	}, this, 0.75, false, "level 1 timer");
}
void Level1Scene::scheduleTimer() {
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON1->getLevel() == 1) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON1->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON1->peekDungeon(m_dungeons.DUNGEON1->getPlayer().getPosX(), m_dungeons.DUNGEON1->getPlayer().getPosY(), '-');

		// Player can be pushed around (by springs), so need to update to true location
		int px = m_dungeons.DUNGEON1->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON1->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		// update HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON1);

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON1->getPlayer().getHP() <= 0) {
			m_hud->gameOver(*this);
		}
	}, this, 0.75, false, "level 1 timer");
}

void Level1Scene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene();
	pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(pauseMenuScene);
}
void Level1Scene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto nextScene = Shop1Scene::createScene(m_dungeons.DUNGEON1->getPlayer(), m_dungeons.DUNGEON1->getLevel());
	//auto nextScene = Level3Scene::createScene(m_dungeons.DUNGEON1->getPlayer());
	nextScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	// stop music
	cocos2d::experimental::AudioEngine::stop(bg_music_id);

	Director::getInstance()->replaceScene(nextScene);
}


//		SHOP 1 SCENE
Shop1Scene::Shop1Scene(HUDLayer* hud, Player p, int level) : m_hud(hud), p(p), m_level(level) {

}
Scene* Shop1Scene::createScene(Player p, int level)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create();
	scene->addChild(hud, 10);

	// create background layer
	BackgroundLayer* bglayer = BackgroundLayer::create();
	scene->addChild(bglayer, -10);

	// calls Level1Scene init()
	auto layer = Shop1Scene::create(hud, p, level);
	scene->addChild(layer);

	return scene;
}
Shop1Scene* Shop1Scene::create(HUDLayer* hud, Player p, int level)
{
	Shop1Scene *pRet = new(std::nothrow) Shop1Scene(hud, p, level);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool Shop1Scene::init()
{
	if (!Scene::init()) {
		return false;
	}


	Dungeons dungeons;
	m_dungeons = dungeons;

	Shop* SHOP1 = new Shop(p, m_level);
	m_dungeons.SHOP1 = SHOP1;

	// music
	id = experimental::AudioEngine::play2d("mistical.mp3", true);

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// board for rendering textures into
	renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height, Texture2D::PixelFormat::RGBA8888);
	renderTexture->retain();
	renderTexture->beginWithClear(0.0, 0.0, 0.0, 0.0);

	//gameboard = Sprite::create();
	//gameboard->setFlippedY(true);
	//gameboard->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//this->addChild(gameboard, 0);

	// create player sprite and follow
	m_player = Sprite::createWithSpriteFrameName("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 1);
	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	m_player->visit();

	// Render all the sprites on the first floor
	int px = 0, py = 0;
	renderDungeon(*m_dungeons.SHOP1, m_dungeons.SHOP1->getRows(), m_dungeons.SHOP1->getCols(), px, py);
	m_player->setPosition(px, py);

	renderTexture->end();
	// load board with the rendered sprites
	//gameboard->initWithTexture(renderTexture->getSprite()->getTexture());


	// Hides the extra game layer!!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1); // flag on camera


	// gives the sprite vectors to the dungeon
	m_dungeons.SHOP1->setPlayerSprite(m_player);
	m_dungeons.SHOP1->getPlayerVector().at(0).setSprite(m_player);

	m_dungeons.SHOP1->setItemSprites(items);
	m_dungeons.SHOP1->setWallSprites(walls);
	m_dungeons.SHOP1->setDoorSprites(doors);
	m_dungeons.SHOP1->setScene(this);


	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(Shop1Scene::Shop1KeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player);

	return true;
}
void Shop1Scene::renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y) {
	std::vector<_Tile> maze = dungeon.getDungeon();
	_Tile *tile;

	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Stairs/Traps				-2
	//		Extras						 x
	//		Floor		: BOTTOM		-4
	//

	std::string image;

	// Begin dungeon render
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand, z;
	for (int i = 0; i < maxrows; i++) {
		for (int j = 0; j < maxcols; j++) {
			tile = &maze[i*maxcols + j];

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

			Sprite* floor = createSprite(image, maxrows, j, i, -10);
			floor->setOpacity(240);
			dungeon[i*maxcols + j].floor = floor;


			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (maxrows - i) - Y_OFFSET;
			}
			if (tile->shop_action != "") {
				if (tile->shop_action == "countertop") image = "Countertop_48x48.png";
				else if (tile->shop_action == "shopkeeper") image = "Shopkeeper_48x48.png";
				else if (tile->shop_action == "breakable") image = "Breakable_Crate_48x48.png";
				else if (tile->shop_action == "secret") image = "Stairs_48x48.png";
				// if tile is labeled shop_item, puts a countertop image underneath the item, as a shop does
				else if (tile->shop_action == "shop_item") image = "Countertop_48x48.png";
				
				if (tile->shop_action != "purchase")
					Sprite* item = createSprite(image, maxrows, j, i, 1);
			}
			if (tile->wall) {
				switch (tile->top) {
				case WALL: {
					rand = randInt(13) + 1;
					switch (rand) {
						/*case 1: image = "Wall_Face1.png"; break;
						case 2: image = "Wall_Face2.png"; break;
						case 3: image = "Wall_Face3.png"; break;
						case 4: image = "Wall_Face4.png"; break;*/
						/*case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;*/
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
					break;
				}
				case UNBREAKABLE_WALL: {
					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == maxcols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == maxrows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
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

						/*switch (rand) {
						case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain19_A.png"; break;
						case 11: image = "Wall_Terrain20_A.png"; break;
						case 12: image = "Wall_Terrain21_A.png"; break;
						case 13: image = "Wall_Terrain22_A.png"; break;*/
						/*case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;
						case 13: image = "Wall_Terrain13_A.png"; break;
						case 14: image = "Wall_Terrain14_A.png"; break;
						case 15: image = "Wall_Terrain15_A.png"; break;
						case 16: image = "Wall_Terrain16_A.png"; break;
						case 17: image = "Wall_Terrain17_A.png"; break;
						case 18: image = "Wall_Terrain18_A.png"; break;*/
						//case 19: image = "Wall_Terrain19_A.png"; break;
						//case 20: image = "Wall_Terrain20_A.png"; break;
						//case 21: image = "Wall_Terrain21_A.png"; break;
						//case 22: image = "Wall_Terrain22_A.png"; break;
					}
					/*
					switch (rand) {
					case 1: image = "Wall_Terrain1.png"; break;
					case 2: image = "Wall_Terrain2.png"; break;
					case 3: image = "Wall_Terrain3.png"; break;
					case 4: image = "Wall_Terrain4.png"; break;
					case 5: image = "Wall_Terrain5.png"; break;
					case 6: image = "Wall_Terrain6.png"; break;
					case 7: image = "Wall_Terrain7.png"; break;
					case 8: image = "Wall_Terrain8.png"; break;
					case 9: image = "Wall_Terrain9.png"; break;
					case 10: image = "Wall_Terrain10.png"; break;
					case 11: image = "Wall_Terrain11.png"; break;
					case 12: image = "Wall_Terrain12.png"; break;
					case 13: image = "Wall_Terrain13.png"; break;
					}*/
					break;
				}
				case DOOR_V: image = "Door_Vertical_Closed_48x48.png"; break;
				default: image = "cheese.png"; break;
				}
				if (tile->top == WALL) {
					Sprite* wall = Sprite::createWithSpriteFrameName(image);
					this->addChild(wall, 1);
					wall->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - i) - Y_OFFSET);
					wall->setColor(Color3B(210, 200, 255));
					walls.push_back(wall);
				}
				else if (tile->top == UNBREAKABLE_WALL) {
					Sprite* wall = Sprite::createWithSpriteFrameName(image);
					this->addChild(wall, 2);
					wall->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - i) - Y_OFFSET);
					wall->setColor(Color3B(170, 90, 40));
				}
				else if (tile->top == DOOR_H || tile->top == DOOR_V) {
					Sprite* door = createSprite(image, maxrows, j, i, 1);
					doors.push_back(door);
				}
				/*else {
					Sprite* monster = createSprite(image, maxrows, j, i, 1);
					monsters.push_back(monster);
				}*/
			}
			if (tile->item) {
				if (tile->object != nullptr) {
					image = tile->object->getImageName();

					Sprite* object = createSprite(image, maxrows, j, i, 2);
					items.push_back(object);
				}
			}
			if (tile->trap) {

				z = -4;
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
					dungeon.getDungeon()[i*maxcols + j].trap = false;
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
				else if (tile->traptile == BUTTON) {
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

				Sprite* trap = createSprite(image, maxrows, j, i, z);
				if (tile->trap_name == SPRING) {
					trap->setScale(0.5);
				}

				int pos = dungeon.findTrap(j, i);
				if (pos != -1)
					dungeonTraps.at(pos)->setSprite(trap);

			}

		}
	}
}
cocos2d::Sprite* Shop1Scene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	sprite->visit();
	this->addChild(sprite, z);

	return sprite;
}

void Shop1Scene::Shop1KeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	int x, y;

	auto actions = this->getActionManager();

	// resumes follow if it was paused
	if (actions->getNumberOfRunningActions() == 0) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	}
	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActions() > 1 && m_dungeons.SHOP1->getLevel() == 2) {
		actions->update(1.0);
	}

	p = m_dungeons.SHOP1->getPlayer();
	x = p.getPosX(); y = p.getPosY();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_dungeons.SHOP1->peekDungeon(x, y, 'l');
		
		if (p.facingDirection() != 'l') {
			m_player->setScaleX(-1);
		}
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_dungeons.SHOP1->peekDungeon(x, y, 'r');
		
		if (p.facingDirection() != 'r') {
			m_player->setScaleX(1);
		}

		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_dungeons.SHOP1->peekDungeon(x, y, 'u');
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_dungeons.SHOP1->peekDungeon(x, y, 'd');
		
		break;
	}

				//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_dungeons.SHOP1->getPlayer().getItems().size() > 0)
			m_dungeons.SHOP1->callUse(m_dungeons.SHOP1->getDungeon(), x, y, 0);

		m_dungeons.SHOP1->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: {
		m_dungeons.SHOP1->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_dungeons.SHOP1->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_dungeons.SHOP1);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_dungeons.SHOP1->peekDungeon(x, y, 'e');
		if (m_dungeons.SHOP1->getLevel() > m_level) {
			advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		kbListener->setEnabled(false);
		m_hud->weaponMenu(kbListener, *m_dungeons.SHOP1);

		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_dungeons.SHOP1);

		break;
	case EventKeyboard::KeyCode::KEY_M: { // toggle music on/off
		if (cocos2d::experimental::AudioEngine::getVolume(id) > 0) {
			cocos2d::experimental::AudioEngine::setVolume(id, 0.0);
		}
		else {
			cocos2d::experimental::AudioEngine::setVolume(id, 1.0);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: // open pause menu
		Director::getInstance()->pushScene(this);
		pauseMenu();
		//this->pauseSchedulerAndActions();
		break;
	default:
		m_dungeons.SHOP1->peekDungeon(x, y, '-');
		break;
	}

	// Update player sprite position
	int px = m_dungeons.SHOP1->getPlayer().getPosX() - x;
	int py = m_dungeons.SHOP1->getPlayer().getPosY() - y;

	// play step sound effect if player moved
	if (px != 0 || py != 0) {
		playFootstepSound();
	}
	cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px * SPACING_FACTOR, -py * SPACING_FACTOR));
	event->getCurrentTarget()->runAction(move);

	// update the HUD
	m_hud->updateHUD(*m_dungeons.SHOP1);

	// Check if player is dead, if so, run game over screen
	if (m_dungeons.SHOP1->getPlayer().getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(id);
		m_hud->gameOver(*this);
	}
}

void Shop1Scene::showShopHUD(Dungeon &dungeon, int x, int y) {
	// x and y are below the item's coordinates,
	// we want to the price to show two above that

	// pricing symbols, prices themselves, etc.
	itemprice = Label::createWithTTF("$", "fonts/Marker Felt.ttf", 24);
	itemprice->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - (y-2)) - Y_OFFSET);
	this->addChild(itemprice, 3);
	itemprice->setColor(cocos2d::Color3B(255, 215, 0));
	itemprice->setString("$" + std::to_string(dungeon.getDungeon()[(y - 1)*dungeon.getCols() + x].price));
}
void Shop1Scene::deconstructShopHUD() {
	itemprice->removeFromParent();
	itemprice = nullptr;
}
void Shop1Scene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene();
	pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(pauseMenuScene);
}
void Shop1Scene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// stop music
	cocos2d::experimental::AudioEngine::stop(id);

	if (m_dungeons.SHOP1->getLevel() == 3) {
		auto level2Scene = Level2Scene::createScene(m_dungeons.SHOP1->getPlayer());
		level2Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

		Director::getInstance()->replaceScene(level2Scene);
	}
	else if (m_dungeons.SHOP1->getLevel() == 5) {
		auto nextScene = Level3Scene::createScene(m_dungeons.SHOP1->getPlayer());
		nextScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

		Director::getInstance()->replaceScene(nextScene);
	}
}


//		LEVEL 2 SCENE
Level2Scene::Level2Scene(HUDLayer* hud, Player p) : m_hud(hud), p(p) {

}
Scene* Level2Scene::createScene(Player p)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create();
	scene->addChild(hud, 10);

	// create background layer
	BackgroundLayer* bglayer = BackgroundLayer::create();
	scene->addChild(bglayer, -10);

	// calls Level1Scene init()
	auto layer = Level2Scene::create(hud, p);
	scene->addChild(layer);

	return scene;
}
Level2Scene* Level2Scene::create(HUDLayer* hud, Player p)
{
	Level2Scene *pRet = new(std::nothrow) Level2Scene(hud, p);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool Level2Scene::init()
{
	if (!Scene::init()) {
		return false;
	}

	Dungeons dungeons;
	m_dungeons = dungeons;

	SecondFloor* DUNGEON2 = new SecondFloor(p);
	m_dungeons.DUNGEON2 = DUNGEON2;

	// music
	//id = experimental::AudioEngine::play2d("Sunstrider.mp3", true);
	id = experimental::AudioEngine::play2d("Tower of Lava.mp3", true);
	cocos2d::experimental::AudioEngine::setMaxAudioInstance(200);

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// board for rendering textures into
	renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height, Texture2D::PixelFormat::RGBA8888);
	renderTexture->retain();
	renderTexture->beginWithClear(0.0, 0.0, 0.0, 0.0);

	//gameboard = Sprite::create();
	//gameboard->setFlippedY(true);
	//gameboard->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//this->addChild(gameboard, 0);

	// create player sprite and follow
	m_player = Sprite::createWithSpriteFrameName("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 1);
	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	m_player->visit();

	// Render all the sprites on the first floor
	int px = 0, py = 0;
	//renderDungeon(DUNGEON2, MAXROWS2, MAXCOLS2, px, py);
	renderDungeon(*m_dungeons.DUNGEON2, m_dungeons.DUNGEON2->getRows(), m_dungeons.DUNGEON2->getCols(), px, py);
	m_player->setPosition(px, py);

	renderTexture->end();
	// load board with the rendered sprites
	//gameboard->initWithTexture(renderTexture->getSprite()->getTexture());


	// Hides the extra game layer!!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1); // flag on camera


	// gives the sprite vectors to the dungeon
	m_dungeons.DUNGEON2->setPlayerSprite(m_player);
	m_dungeons.DUNGEON2->getPlayerVector().at(0).setSprite(m_player);

	m_dungeons.DUNGEON2->setMoneySprites(money);
	m_dungeons.DUNGEON2->setItemSprites(items);
	m_dungeons.DUNGEON2->setWallSprites(walls);
	m_dungeons.DUNGEON2->setDoorSprites(doors);
	m_dungeons.DUNGEON2->setScene(this);

	m_dungeons.DUNGEON2->updateLighting();
	
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(Level2Scene::Level2KeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player);


	// schedule inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON2->getLevel() == 3) { // >1 because follow player is always running
			actions->update(1.0);
		}
		
		p = m_dungeons.DUNGEON2->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON2->peekDungeon(m_dungeons.DUNGEON2->getPlayer().getPosX(), m_dungeons.DUNGEON2->getPlayer().getPosY(), '-');

		// Update player sprite position
		int px = m_dungeons.DUNGEON2->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON2->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}
		//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px * SPACING_FACTOR, -py * SPACING_FACTOR));
		//m_player->runAction(move);

		// update the HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON2);

		// check if player interacted with the fountain
		if (m_dungeons.DUNGEON2->getWaterPrompt()) {
			kbListener->setEnabled(false);
		}

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON2->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}
	}, this, 0.65, false, "level 2 timer");

	return true;
}
void Level2Scene::renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y) {
	std::vector<_Tile> maze = dungeon.getDungeon();
	_Tile *tile;

	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Stairs/Traps				-2
	//		Extras						 x
	//		Floor		: BOTTOM		-4
	//

	std::string image;

	// Begin dungeon render
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand, z;
	for (int i = 0; i < maxrows; i++) {
		for (int j = 0; j < maxcols; j++) {
			tile = &maze[i*maxcols + j];

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

			Sprite* floor = createSprite(image, maxrows, j, i, -10);
			floor->setOpacity(250);
			floor->setColor(Color3B(0, 0, 0));
			
			dungeon[i*maxcols + j].floor = floor;


			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (maxrows - i) - Y_OFFSET;
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
				Sprite* gold = createSprite(image, maxrows, j, i, -3);
				money.push_back(gold);
			}
			if (tile->wall) {
				switch (tile->top) {
				case WALL: {
					rand = randInt(13) + 1;
					switch (rand) {
						/*case 1: image = "Wall_Face1.png"; break;
						case 2: image = "Wall_Face2.png"; break;
						case 3: image = "Wall_Face3.png"; break;
						case 4: image = "Wall_Face4.png"; break;*/
						/*case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;*/
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
					break;
				}
				case UNBREAKABLE_WALL: {
					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == maxcols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == maxrows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
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

						/*switch (rand) {
						case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain19_A.png"; break;
						case 11: image = "Wall_Terrain20_A.png"; break;
						case 12: image = "Wall_Terrain21_A.png"; break;
						case 13: image = "Wall_Terrain22_A.png"; break;*/
						/*case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;
						case 13: image = "Wall_Terrain13_A.png"; break;
						case 14: image = "Wall_Terrain14_A.png"; break;
						case 15: image = "Wall_Terrain15_A.png"; break;
						case 16: image = "Wall_Terrain16_A.png"; break;
						case 17: image = "Wall_Terrain17_A.png"; break;
						case 18: image = "Wall_Terrain18_A.png"; break;*/
						//case 19: image = "Wall_Terrain19_A.png"; break;
						//case 20: image = "Wall_Terrain20_A.png"; break;
						//case 21: image = "Wall_Terrain21_A.png"; break;
						//case 22: image = "Wall_Terrain22_A.png"; break;
					}
					/*
					switch (rand) {
					case 1: image = "Wall_Terrain1.png"; break;
					case 2: image = "Wall_Terrain2.png"; break;
					case 3: image = "Wall_Terrain3.png"; break;
					case 4: image = "Wall_Terrain4.png"; break;
					case 5: image = "Wall_Terrain5.png"; break;
					case 6: image = "Wall_Terrain6.png"; break;
					case 7: image = "Wall_Terrain7.png"; break;
					case 8: image = "Wall_Terrain8.png"; break;
					case 9: image = "Wall_Terrain9.png"; break;
					case 10: image = "Wall_Terrain10.png"; break;
					case 11: image = "Wall_Terrain11.png"; break;
					case 12: image = "Wall_Terrain12.png"; break;
					case 13: image = "Wall_Terrain13.png"; break;
					}*/
					break;
				}
				case c_FOUNTAIN: image = "Fountain_Down_48x48.png"; break;
				case DOOR_V: image = "Door_Vertical_Closed_48x48.png"; break;
				case DOOR_H: image = "Door_Horizontal_Closed_48x48.png"; break;
				}
				if (tile->top == WALL) {
					Sprite* wall = Sprite::createWithSpriteFrameName(image);
					this->addChild(wall, 1);
					wall->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - i) - Y_OFFSET);
					wall->setColor(Color3B(210, 200, 255));
					walls.push_back(wall);
				}
				else if (tile->top == UNBREAKABLE_WALL) {
					Sprite* wall = Sprite::createWithSpriteFrameName(image);
					this->addChild(wall, 1);
					wall->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - i) - Y_OFFSET);
					wall->setColor(Color3B(170, 90, 40));
				}
				else if (tile->top == c_FOUNTAIN) {
					Sprite* fountain = Sprite::createWithSpriteFrameName(image);
					this->addChild(fountain, 1);
					fountain->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - i) - Y_OFFSET);
					walls.push_back(fountain); // fountain is a wall
				}
				else if (tile->top == DOOR_H || tile->top == DOOR_V) {
					Sprite* door = createSprite(image, maxrows, j, i, 1);
					doors.push_back(door);
				}
			}
			if (tile->enemy) {
				
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
				Sprite* monsterSprite = createSprite(image, maxrows, j, i, 1);

				dungeon.getMonsters()[pos]->setSprite(monsterSprite);
			}
			if (tile->item) {
				if (tile->object != nullptr) {
					image = tile->object->getImageName();

					Sprite* object = createSprite(image, maxrows, j, i, -1);
					items.push_back(object);
				}
			}
			if (tile->trap) {
				
				z = -4;
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
					dungeon.getDungeon()[i*maxcols + j].trap = false;
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
				else if (tile->traptile == BUTTON) {
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
				
				//image = dungeon.getTraps()[dungeon.findTrap(j, i)]->getImageName();
				Sprite* trap = createSprite(image, maxrows, j, i, z);
				if (tile->trap_name == SPRING) {
					trap->setScale(0.5);
				}

				int pos = dungeon.findTrap(j, i);
				if (pos != -1)
					dungeonTraps.at(pos)->setSprite(trap);
				
			}

		}
	}
}
cocos2d::Sprite* Level2Scene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	sprite->visit();
	this->addChild(sprite, z);

	return sprite;
}

void Level2Scene::Level2KeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	// unschedule the inaction timer
	Director::getInstance()->getScheduler()->unschedule("level 2 timer", this);

	int x, y;

	auto actions = this->getActionManager();

	// resumes follow if it was paused
	if (actions->getNumberOfRunningActions() == 0) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	}
	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON2->getLevel() == 3) {
		actions->update(1.0);
	}

	p = m_dungeons.DUNGEON2->getPlayer();
	x = p.getPosX(); y = p.getPosY();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'l');
		
		if (p.facingDirection() != 'l') {
			m_player->setScaleX(-1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'r');
		
		if (p.facingDirection() != 'r') {
			m_player->setScaleX(1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'u');
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'd');
		break;
	}

		//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_dungeons.DUNGEON2->getPlayer().getItems().size() > 0)
			m_dungeons.DUNGEON2->callUse(m_dungeons.DUNGEON2->getDungeon(), x, y, 0);

		m_dungeons.DUNGEON2->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: {
		m_dungeons.DUNGEON2->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_dungeons.DUNGEON2);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_dungeons.DUNGEON2->peekDungeon(x, y, 'e');
		if (m_dungeons.DUNGEON2->getLevel() == 4) {
			advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		kbListener->setEnabled(false);
		m_hud->weaponMenu(kbListener, *m_dungeons.DUNGEON2);
		
		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_dungeons.DUNGEON2);

		break;
	case EventKeyboard::KeyCode::KEY_M: { // toggle music on/off
		if (cocos2d::experimental::AudioEngine::getVolume(id) > 0) {
			cocos2d::experimental::AudioEngine::setVolume(id, 0.0);
		}
		else {
			cocos2d::experimental::AudioEngine::setVolume(id, 1.0);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: // open pause menu
		Director::getInstance()->pushScene(this);
		pauseMenu();
		this->pauseSchedulerAndActions();
		break;
	default:
		m_dungeons.DUNGEON2->peekDungeon(x, y, '-');
		break;
	}

	// Update player sprite position
	int px = m_dungeons.DUNGEON2->getPlayer().getPosX() - x;
	int py = m_dungeons.DUNGEON2->getPlayer().getPosY() - y;

	// play step sound effect if player moved
	if (px != 0 || py != 0) {
		playFootstepSound();
	}
	//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px * SPACING_FACTOR, -py * SPACING_FACTOR));
	//event->getCurrentTarget()->runAction(move);

	// update the HUD
	m_hud->updateHUD(*m_dungeons.DUNGEON2);

	// check if player interacted with the fountain
	if (m_dungeons.DUNGEON2->getWaterPrompt()) {
		kbListener->setEnabled(false);
	}

	// Check if player is dead, if so, run game over screen
	if (m_dungeons.DUNGEON2->getPlayer().getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(id);
		m_hud->gameOver();
		return;
	}


	// reschedule inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON2->getLevel() == 3) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON2->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON2->peekDungeon(m_dungeons.DUNGEON2->getPlayer().getPosX(), m_dungeons.DUNGEON2->getPlayer().getPosY(), '-');

		// Update player sprite position
		int px = m_dungeons.DUNGEON2->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON2->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}
		//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px * SPACING_FACTOR, -py * SPACING_FACTOR));
		//m_player->runAction(move);

		// update the HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON2);

		// check if player interacted with the fountain
		if (m_dungeons.DUNGEON2->getWaterPrompt()) {
			kbListener->setEnabled(false);
		}

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON2->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}
	}, this, 0.65, false, "level 2 timer");
}

void Level2Scene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene();
	pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(pauseMenuScene);
}
void Level2Scene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	// stop music
	cocos2d::experimental::AudioEngine::stop(id);

	auto nextScene = Shop1Scene::createScene(m_dungeons.DUNGEON2->getPlayer(), m_dungeons.DUNGEON2->getLevel());
	nextScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	Director::getInstance()->replaceScene(nextScene);

	/*auto level3Scene = Level3Scene::createScene(m_dungeons.DUNGEON2->getPlayer());
	level3Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));*/

	//Director::getInstance()->replaceScene(level3Scene);
}


//		LEVEL 3 SCENE
Level3Scene::Level3Scene(HUDLayer* hud, Player p) : m_hud(hud), p(p) {

}
Scene* Level3Scene::createScene(Player p)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create();
	scene->addChild(hud, 10);

	// create background layer
	BackgroundLayer* bglayer = BackgroundLayer::create();
	scene->addChild(bglayer, -10);

	// calls Level1Scene init()
	auto layer = Level3Scene::create(hud, p);
	scene->addChild(layer);

	return scene;
}
Level3Scene* Level3Scene::create(HUDLayer* hud, Player p)
{
	Level3Scene *pRet = new(std::nothrow) Level3Scene(hud, p);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool Level3Scene::init()
{
	if (!Scene::init()) {
		return false;
	}

	Dungeons dungeons;
	m_dungeons = dungeons;

	ThirdFloor* DUNGEON3 = new ThirdFloor(p);
	m_dungeons.DUNGEON3 = DUNGEON3;

	// music
	//id = experimental::AudioEngine::play2d("Zero Respect.mp3", true, 1.0f);
	id = experimental::AudioEngine::play2d("Who turned off the lights.mp3", true, 1.0f);
	cocos2d::experimental::AudioEngine::setMaxAudioInstance(200);

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// board for rendering textures into
	renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height, Texture2D::PixelFormat::RGBA8888);
	renderTexture->retain();
	renderTexture->beginWithClear(0.0, 0.0, 0.0, 0.0);

	//gameboard = Sprite::create();
	//gameboard->setFlippedY(true);
	//gameboard->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//this->addChild(gameboard, 0);

	// create player sprite and follow
	m_player = Sprite::create("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 1);
	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	m_player->visit();

	// Render all the sprites on the first floor
	int px = 0, py = 0;
	renderDungeon(*m_dungeons.DUNGEON3, m_dungeons.DUNGEON3->getRows(), m_dungeons.DUNGEON3->getCols(), px, py);
	m_player->setPosition(px, py);

	renderTexture->end();
	// load board with the rendered sprites
	//gameboard->initWithTexture(renderTexture->getSprite()->getTexture());

	// Hides the extra game layer!!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1); // flag on camera


	// gives the sprite vectors to the dungeon
	m_dungeons.DUNGEON3->setPlayerSprite(m_player);
	m_dungeons.DUNGEON3->getPlayerVector().at(0).setSprite(m_player);

	m_dungeons.DUNGEON3->setMoneySprites(money);
	m_dungeons.DUNGEON3->setItemSprites(items);
	m_dungeons.DUNGEON3->setWallSprites(walls);
	m_dungeons.DUNGEON3->setDoorSprites(doors);
	m_dungeons.DUNGEON3->setScene(this);

	m_dungeons.DUNGEON3->updateLighting();

	// hide all except the starting room
	m_dungeons.DUNGEON3->hideRooms();


	// add keyboard listener
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(Level3Scene::Level3KeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player);

	
	// schedule inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON3->getLevel() == 4) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON3->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON3->peekDungeon(m_dungeons.DUNGEON3->getPlayer().getPosX(), m_dungeons.DUNGEON3->getPlayer().getPosY(), '-');

		// Update player sprite position
		int px = m_dungeons.DUNGEON3->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON3->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		// update the HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON3);

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON3->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}

	}, this, 0.60, false, "level 3 timer");


	return true;
}
void Level3Scene::renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y) {
	std::vector<_Tile> maze = dungeon.getDungeon();
	_Tile *tile;

	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Stairs/Traps				-2
	//		Extras						 x
	//		Floor		: BOTTOM		-4
	//

	std::string image;

	// Begin dungeon render
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand;
	int z;
	for (int i = 0; i < maxrows; i++) {
		for (int j = 0; j < maxcols; j++) {
			tile = &maze[i*maxcols + j];

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

			Sprite* floor = createSprite(image, maxrows, j, i, -10);
			floor->setOpacity(252);
			floor->setColor(Color3B(0, 0, 0));
			
			dungeon[i*maxcols + j].floor = floor;

			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (maxrows - i) - Y_OFFSET;
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
				Sprite* gold = createSprite(image, maxrows, j, i, -3);
				money.push_back(gold);
			}
			if (tile->wall) {
				switch (tile->top) {
				case WALL: {
					rand = randInt(13) + 1;
					switch (rand) {
						/*case 1: image = "Wall_Face1.png"; break;
						case 2: image = "Wall_Face2.png"; break;
						case 3: image = "Wall_Face3.png"; break;
						case 4: image = "Wall_Face4.png"; break;*/
						/*case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;*/
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
					break;
				}
				case UNBREAKABLE_WALL: {
					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == maxcols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == maxrows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
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

						/*switch (rand) {
						case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain19_A.png"; break;
						case 11: image = "Wall_Terrain20_A.png"; break;
						case 12: image = "Wall_Terrain21_A.png"; break;
						case 13: image = "Wall_Terrain22_A.png"; break;*/
						/*case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;
						case 13: image = "Wall_Terrain13_A.png"; break;
						case 14: image = "Wall_Terrain14_A.png"; break;
						case 15: image = "Wall_Terrain15_A.png"; break;
						case 16: image = "Wall_Terrain16_A.png"; break;
						case 17: image = "Wall_Terrain17_A.png"; break;
						case 18: image = "Wall_Terrain18_A.png"; break;*/
						//case 19: image = "Wall_Terrain19_A.png"; break;
						//case 20: image = "Wall_Terrain20_A.png"; break;
						//case 21: image = "Wall_Terrain21_A.png"; break;
						//case 22: image = "Wall_Terrain22_A.png"; break;
					}
					/*
					switch (rand) {
					case 1: image = "Wall_Terrain1.png"; break;
					case 2: image = "Wall_Terrain2.png"; break;
					case 3: image = "Wall_Terrain3.png"; break;
					case 4: image = "Wall_Terrain4.png"; break;
					case 5: image = "Wall_Terrain5.png"; break;
					case 6: image = "Wall_Terrain6.png"; break;
					case 7: image = "Wall_Terrain7.png"; break;
					case 8: image = "Wall_Terrain8.png"; break;
					case 9: image = "Wall_Terrain9.png"; break;
					case 10: image = "Wall_Terrain10.png"; break;
					case 11: image = "Wall_Terrain11.png"; break;
					case 12: image = "Wall_Terrain12.png"; break;
					case 13: image = "Wall_Terrain13.png"; break;
					}*/
					break;
				}
				case DOOR_H: image = "Door_Horizontal_Closed_48x48.png"; break;
				case DOOR_V: image = "Door_Vertical_Closed_48x48.png"; break;
				}

				if (tile->top == WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);
					wall->setColor(Color3B(210, 200, 255));
					walls.push_back(wall);
				}
				else if (tile->top == UNBREAKABLE_WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);
					wall->setColor(Color3B(170, 90, 40));
				}
				else if (tile->top == DOOR_H || tile->top == DOOR_V) {
					Sprite* door = createSprite(image, maxrows, j, i, 1);
					doors.push_back(door);
				}
			}
			if (tile->enemy) {

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
				Sprite* monsterSprite = createSprite(image, maxrows, j, i, 1);

				dungeon.getMonsters()[pos]->setSprite(monsterSprite);
			}
			if (tile->item) {
				image = tile->object->getImageName();
				Sprite* object = createSprite(image, maxrows, j, i, -1);
				items.push_back(object);
			}
			if (tile->trap) {

				z = -4;
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
					dungeon.getDungeon()[i*maxcols + j].trap = false;
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
				else if (tile->traptile == BUTTON) {
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

				Sprite* trap = createSprite(image, maxrows, j, i, z);
				if (tile->trap_name == SPRING) {
					trap->setScale(0.5);
				}

				int pos = dungeon.findTrap(j, i);
				if (pos != -1)
					dungeonTraps.at(pos)->setSprite(trap);
			}
		}
	}

}
cocos2d::Sprite* Level3Scene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	sprite->visit();
	this->addChild(sprite, z);

	return sprite;
}

void Level3Scene::Level3KeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	// unschedule inaction timer
	Director::getInstance()->getScheduler()->unschedule("level 3 timer", this);

	int x, y;
	char c;

	auto actions = this->getActionManager();

	// resumes follow if it was paused
	if (actions->getNumberOfRunningActions() == 0) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	}
	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON3->getLevel() == 4) {
		actions->update(1.0);
	}

	p = m_dungeons.DUNGEON3->getPlayer();
	x = p.getPosX(); y = p.getPosY();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'l');

		if (p.facingDirection() != 'l') {
			m_player->setScaleX(-1);
		}
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'r');

		if (p.facingDirection() != 'r') {
			m_player->setScaleX(1);
		}
		
		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'u');
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'd');
		break;
	}

		//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_dungeons.DUNGEON3->getPlayer().getItems().size() > 0)
			m_dungeons.DUNGEON3->callUse(m_dungeons.DUNGEON3->getDungeon(), x, y, 0);

		m_dungeons.DUNGEON3->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: {
		m_dungeons.DUNGEON3->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_dungeons.DUNGEON3);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_dungeons.DUNGEON3->peekDungeon(x, y, 'e');
		if (m_dungeons.DUNGEON3->getLevel() == 5) {
			advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		kbListener->setEnabled(false);
		m_hud->weaponMenu(kbListener, *m_dungeons.DUNGEON3);

		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_dungeons.DUNGEON3);

		break;
	case EventKeyboard::KeyCode::KEY_M: { // toggle music on/off
		// USE OTHER AUDIO ENGINE; SIMPLE AUDIO ENGINE IS INCOMPLETE
		if (cocos2d::experimental::AudioEngine::getVolume(id) > 0) {
			cocos2d::experimental::AudioEngine::setVolume(id, 0.0);
		}
		else {
			cocos2d::experimental::AudioEngine::setVolume(id, 1.0);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: // open pause menu
		Director::getInstance()->pushScene(this);
		pauseMenu();
		break;
	default:
		m_dungeons.DUNGEON3->peekDungeon(x, y, '-');
		break;
	}

	// Update player sprite position
	int px = m_dungeons.DUNGEON3->getPlayer().getPosX() - x;
	int py = m_dungeons.DUNGEON3->getPlayer().getPosY() - y;

	// play step sound effect if player moved
	if (px != 0 || py != 0) {
		playFootstepSound();
	}

	// update the HUD
	m_hud->updateHUD(*m_dungeons.DUNGEON3);

	// Check if player is dead, if so, run game over screen
	if (m_dungeons.DUNGEON3->getPlayer().getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(id);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}


	// reschedule inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		// resumes follow if it was paused
		if (actions->getNumberOfRunningActions() == 0) {
			auto visibleSize = Director::getInstance()->getVisibleSize();
			this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 1 && m_dungeons.DUNGEON3->getLevel() == 4) { // >1 because follow player is always running
			actions->update(1.0);
		}

		p = m_dungeons.DUNGEON3->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.DUNGEON3->peekDungeon(m_dungeons.DUNGEON3->getPlayer().getPosX(), m_dungeons.DUNGEON3->getPlayer().getPosY(), '-');

		// Update player sprite position
		int px = m_dungeons.DUNGEON3->getPlayer().getPosX() - x;
		int py = m_dungeons.DUNGEON3->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		// update the HUD
		m_hud->updateHUD(*m_dungeons.DUNGEON3);

		// Check if player is dead, if so, run game over screen
		if (m_dungeons.DUNGEON3->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}

	}, this, 0.60, false, "level 3 timer");
}

void Level3Scene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene();
	pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(pauseMenuScene);
}
void Level3Scene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto boss1Scene = Boss1Scene::createScene(m_dungeons.DUNGEON3->getPlayer());
	boss1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	// stop music
	cocos2d::experimental::AudioEngine::stop(id);

	Director::getInstance()->replaceScene(boss1Scene);
}


//		BOSS LEVEL
Boss1Scene::Boss1Scene(HUDLayer* hud, Player p) : m_hud(hud), p(p) {

}
Scene* Boss1Scene::createScene(Player p)
{
	auto scene = Scene::create();

	// create HUD layer
	HUDLayer* hud = HUDLayer::create();
	scene->addChild(hud, 10);

	// create background layer
	BackgroundLayer* bglayer = BackgroundLayer::create();
	scene->addChild(bglayer, -10);

	// calls Level1Scene init()
	auto layer = Boss1Scene::create(hud, p);
	scene->addChild(layer);

	return scene;
}
Boss1Scene* Boss1Scene::create(HUDLayer* hud, Player p)
{
	Boss1Scene *pRet = new(std::nothrow) Boss1Scene(hud, p);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool Boss1Scene::init()
{
	if (!Scene::init()) {
		return false;
	}

	Dungeons dungeons;
	m_dungeons = dungeons;

	FirstBoss* BOSS1 = new FirstBoss(p);
	m_dungeons.BOSS1 = BOSS1;

	// reveal boss hp bar
	m_hud->showBossHP();

	// music
	id = experimental::AudioEngine::play2d("Zero Respect.mp3", true, 1.0f);

	auto visibleSize = Director::getInstance()->getVisibleSize();

	// board for rendering textures into
	renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height, Texture2D::PixelFormat::RGBA8888);
	renderTexture->retain();
	renderTexture->beginWithClear(0.0, 0.0, 0.0, 0.0);

	//gameboard = Sprite::create();
	//gameboard->setFlippedY(true);
	//gameboard->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//this->addChild(gameboard, 0);

	// create player sprite and follow
	m_player = Sprite::create("Player1_48x48.png");
	m_player->setPosition(0, 0);
	this->addChild(m_player, 1);
	//this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
	this->setScale(0.4);
	this->setPosition((-visibleSize.width / 2) * .7, (-visibleSize.height / 2) * 1.2);
	//this->runAction(Follow::createWithOffset(m_player, 0, -visibleSize.height / 2, Rect(visibleSize.width / 1.5, visibleSize.height/1.9, 400, 900)));
	m_player->visit();

	// Render all the sprites on the first floor
	int px = 0, py = 0;
	renderDungeon(*m_dungeons.BOSS1, m_dungeons.BOSS1->getRows(), m_dungeons.BOSS1->getCols(), px, py);
	m_player->setPosition(px, py);

	renderTexture->end();
	// load board with the rendered sprites
	//gameboard->initWithTexture(renderTexture->getSprite()->getTexture());

	// Hides the extra game layer!!
	auto defaultCamera = this->getDefaultCamera();
	defaultCamera->setCameraFlag(CameraFlag::USER1); // flag on camera


	// gives the sprite vectors to the dungeon
	/*BOSS1.setPlayerSprite(m_player);
	BOSS1.getPlayerVector().at(0).setSprite(m_player);

	BOSS1.setMonsterSprites(monsters);
	BOSS1.setItemSprites(items);
	BOSS1.setTrapSprites(traps);
	BOSS1.setSpikeProjectileSprites(spike_projectiles);
	BOSS1.setWallSprites(walls);
	BOSS1.setDoorSprites(doors);
	BOSS1.setScene(this);*/

	m_dungeons.BOSS1->setPlayerSprite(m_player);
	m_dungeons.BOSS1->getPlayerVector().at(0).setSprite(m_player);

	m_dungeons.BOSS1->setItemSprites(items);
	m_dungeons.BOSS1->setSpikeProjectileSprites(spike_projectiles);
	m_dungeons.BOSS1->setWallSprites(walls);
	m_dungeons.BOSS1->setDoorSprites(doors);
	m_dungeons.BOSS1->setScene(this);


	// add keyboard listener
	kbListener = EventListenerKeyboard::create();
	kbListener->onKeyPressed = CC_CALLBACK_2(Boss1Scene::Boss1KeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, m_player);


	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		//// resumes follow if it was paused
		//if (actions->getNumberOfRunningActions() == 0) {
		//	auto visibleSize = Director::getInstance()->getVisibleSize();
		//	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		//}
		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 0) { // >1 because follow player is always running
			actions->update(1.0);
		}
		Player p = m_dungeons.BOSS1->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.BOSS1->peekDungeon(m_dungeons.BOSS1->getPlayer().getPosX(), m_dungeons.BOSS1->getPlayer().getPosY(), '-');
		
		// Smasher moves player around, so need to update to true location by
		// taking (new player pos - old player pos) * SpacingFactor
		int px = m_dungeons.BOSS1->getPlayer().getPosX() - x;
		int py = m_dungeons.BOSS1->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px*SPACING_FACTOR, -py * SPACING_FACTOR));
		//m_player->runAction(move);

		// update the HUD
		m_hud->updateHUD(*m_dungeons.BOSS1);
		m_hud->updateBossHUD(*m_dungeons.BOSS1);


		// Check if player is dead, if so, run game over screen
		if (m_dungeons.BOSS1->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}

		// Check if player found the idol
		if (m_dungeons.BOSS1->getPlayer().getWin()) {
			m_hud->winner();
		}
	}, this, 0.45, false, "boss 1 timer");


	return true;
}
void Boss1Scene::renderDungeon(Dungeon &dungeon, int maxrows, int maxcols, int &x, int &y) {
	std::vector<_Tile> maze = dungeon.getDungeon();
	_Tile *tile;

	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Stairs/Traps				-2
	//		Extras						 x
	//		Floor		: BOTTOM		-4
	//

	std::string image;

	// Begin dungeon render
	std::vector<std::shared_ptr<Monster>> monster = dungeon.getMonsters();
	std::vector<std::shared_ptr<Traps>> dungeonTraps = dungeon.getTraps();
	int rand;
	for (int i = 0; i < maxrows; i++) {
		for (int j = 0; j < maxcols; j++) {
			tile = &maze[i*maxcols + j];

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

			Sprite* floor = createSprite(image, maxrows, j, i, -5);
			floor->setOpacity(245);
			dungeon[i*maxcols + j].floor = floor;


			if (tile->upper == SMASHER && j == BOSSCOLS / 2 && i == 3) {
				// smasher position hardcoded to begin with
				image = "C_Wall_Terrain1_48x48.png";
				Sprite* smasher = createSprite(image, maxrows, j, i, 2);
				smasher->setScale(3.0);
				smasher->setColor(Color3B(55, 30, 40));
				monsters.push_back(smasher);

				int pos = dungeon.findMonster(j, i);
				dungeon.getMonsters()[pos]->setSprite(smasher);
			}
			if (tile->hero) {
				x = j * SPACING_FACTOR - X_OFFSET;
				y = SPACING_FACTOR * (maxrows - i) - Y_OFFSET;
			}
			if (tile->wall) {
				switch (tile->top) {
				case WALL: {
					rand = randInt(13) + 1;
					switch (rand) {
						/*case 1: image = "Wall_Face1.png"; break;
						case 2: image = "Wall_Face2.png"; break;
						case 3: image = "Wall_Face3.png"; break;
						case 4: image = "Wall_Face4.png"; break;*/
						/*case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;*/
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
					break;
				}
				case UNBREAKABLE_WALL: {
					if (i == 0) rand = randInt(3) + 1 + (randInt(2) * 12); // upper border
					else if (j == maxcols - 1) rand = randInt(3) + 4;// + (randInt(2) * 9); // right border
					else if (i == maxrows - 1) rand = randInt(3) + 7 + (randInt(2) * 6); // lower border
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

						/*switch (rand) {
						case 1: image = "Wall_Terrain1_A.png"; break;
						case 2: image = "Wall_Terrain2_A.png"; break;
						case 3: image = "Wall_Terrain3_A.png"; break;
						case 4: image = "Wall_Terrain4_A.png"; break;
						case 5: image = "Wall_Terrain5_A.png"; break;
						case 6: image = "Wall_Terrain6_A.png"; break;
						case 7: image = "Wall_Terrain7_A.png"; break;
						case 8: image = "Wall_Terrain8_A.png"; break;
						case 9: image = "Wall_Terrain9_A.png"; break;
						case 10: image = "Wall_Terrain19_A.png"; break;
						case 11: image = "Wall_Terrain20_A.png"; break;
						case 12: image = "Wall_Terrain21_A.png"; break;
						case 13: image = "Wall_Terrain22_A.png"; break;*/
						/*case 10: image = "Wall_Terrain10_A.png"; break;
						case 11: image = "Wall_Terrain11_A.png"; break;
						case 12: image = "Wall_Terrain12_A.png"; break;
						case 13: image = "Wall_Terrain13_A.png"; break;
						case 14: image = "Wall_Terrain14_A.png"; break;
						case 15: image = "Wall_Terrain15_A.png"; break;
						case 16: image = "Wall_Terrain16_A.png"; break;
						case 17: image = "Wall_Terrain17_A.png"; break;
						case 18: image = "Wall_Terrain18_A.png"; break;*/
						//case 19: image = "Wall_Terrain19_A.png"; break;
						//case 20: image = "Wall_Terrain20_A.png"; break;
						//case 21: image = "Wall_Terrain21_A.png"; break;
						//case 22: image = "Wall_Terrain22_A.png"; break;
					}
					/*
					switch (rand) {
					case 1: image = "Wall_Terrain1.png"; break;
					case 2: image = "Wall_Terrain2.png"; break;
					case 3: image = "Wall_Terrain3.png"; break;
					case 4: image = "Wall_Terrain4.png"; break;
					case 5: image = "Wall_Terrain5.png"; break;
					case 6: image = "Wall_Terrain6.png"; break;
					case 7: image = "Wall_Terrain7.png"; break;
					case 8: image = "Wall_Terrain8.png"; break;
					case 9: image = "Wall_Terrain9.png"; break;
					case 10: image = "Wall_Terrain10.png"; break;
					case 11: image = "Wall_Terrain11.png"; break;
					case 12: image = "Wall_Terrain12.png"; break;
					case 13: image = "Wall_Terrain13.png"; break;
					}*/
					break;
				}
				case DOOR_H: image = "Door_Horizontal_Closed_48x48.png"; break;
				case DOOR_V: image = "Door_Vertical_Closed_48x48.png"; break;
				}
				if (tile->top == WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);
					wall->setColor(Color3B(210, 200, 255));
					walls.push_back(wall);
				}
				else if (tile->top == UNBREAKABLE_WALL) {
					Sprite* wall = createSprite(image, maxrows, j, i, 1);
					wall->setColor(Color3B(170, 90, 40));
				}
				else if (tile->top == DOOR_H || tile->top == DOOR_V) {
					Sprite* door = createSprite(image, maxrows, j, i, 1);
					doors.push_back(door);
				}
			}
			if (tile->enemy && tile->upper != SMASHER) {
				switch (tile->top) {
				case c_SPINNER: {
					// add spinner buddies to the scene
					std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monster.at(dungeon.findMonster(j, i)));

					this->addChild(spinner->getInner(), 0);
					this->addChild(spinner->getOuter(), 0);

					spinner.reset();
					break;
				}
				case c_ZAPPER: {
					// add the sparks to the scene
					std::shared_ptr<Zapper> zapper = std::dynamic_pointer_cast<Zapper>(monster.at(dungeon.findMonster(j, i)));
					for (int m = 0; m < 8; m++) {
						this->addChild(zapper->getSparks()[m], 0);
					}
					zapper.reset();
					break;
				}
				default: image = "cheese.png"; break;
				}

				int pos = dungeon.findMonster(j, i);
				image = monster.at(pos)->getImageName();
				Sprite* monsterSprite = createSprite(image, maxrows, j, i, 1);

				dungeon.getMonsters()[pos]->setSprite(monsterSprite);
			}
			if (tile->item) {
				image = tile->object->getImageName();

				Sprite* object = createSprite(image, maxrows, j, i, -1);
				items.push_back(object);
			}
			if (tile->trap) {
				switch (tile->traptile) {
				case STAIRS: image = "Stairs_48x48.png"; break;
				case SPIKETRAP_DEACTIVE: image = "Spiketrap_Deactive_48x48.png"; break;
				case SPIKETRAP_PRIMED: image = "Spiketrap_Primed_48x48.png"; break;
				case SPIKETRAP_ACTIVE: image = "Spiketrap_Active_48x48.png"; break;
				default: image = "cheese.png"; break;
				}
				if (tile->traptile == SPIKETRAP_DEACTIVE) {
					int n = dungeon.findTrap(j, i);
					std::shared_ptr<SpikeTrap> spiketrap = std::dynamic_pointer_cast<SpikeTrap>(dungeonTraps.at(n));

					spiketrap->getSpriteD()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteP()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteA()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					this->addChild(spiketrap->getSpriteD(), 0);
					this->addChild(spiketrap->getSpriteP(), 0);
					this->addChild(spiketrap->getSpriteA(), 0);
				}
				else if (tile->traptile == TRIGGER_SPIKE_DEACTIVE) {
					int n = dungeon.findTrap(j, i);
					std::shared_ptr<TriggerSpike> spiketrap = std::dynamic_pointer_cast<TriggerSpike>(dungeonTraps.at(n));

					spiketrap->getSpriteD()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteP()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					spiketrap->getSpriteA()->setPosition(j * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (dungeon.getRows() - i) - Y_OFFSET);
					this->addChild(spiketrap->getSpriteD(), 0);
					this->addChild(spiketrap->getSpriteP(), 0);
					this->addChild(spiketrap->getSpriteA(), 0);
				}
				else if (tile->traptile != DOOR_H && tile->traptile != DOOR_V) {
					Sprite* trap = createSprite(image, maxrows, j, i, -2);
					traps.push_back(trap);
				}
			}

		}
	}

}
cocos2d::Sprite* Boss1Scene::createSprite(std::string image, int maxrows, int x, int y, int z) {
	Sprite* sprite = Sprite::createWithSpriteFrameName(image);
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, SPACING_FACTOR * (maxrows - y) - Y_OFFSET);
	sprite->visit();
	this->addChild(sprite, z);

	return sprite;
}

void Boss1Scene::Boss1KeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
	// unschedule the inaction timer
	Director::getInstance()->getScheduler()->unschedule("boss 1 timer", this);

	int x, y;
	char c;

	auto actions = this->getActionManager();

	//// resumes follow if it was paused
	//if (actions->getNumberOfRunningActions() == 0) {
	//	auto visibleSize = Director::getInstance()->getVisibleSize();
	//	this->runAction(Follow::createWithOffset(m_player, 0, -visibleSize.height / 2, Rect(visibleSize.width / 1.5, visibleSize.height / 1.9, 400, 1100)));
	//}

	// if there are any lingering actions, finish them instantly
	while (actions->getNumberOfRunningActions() > 0) {
		actions->update(1.0);
	}

	p = m_dungeons.BOSS1->getPlayer();
	x = p.getPosX(); y = p.getPosY();
	Vec2 pos = event->getCurrentTarget()->getPosition();

	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
		m_dungeons.BOSS1->peekDungeon(x, y, 'l');

		if (p.facingDirection() != 'l') {
			m_player->setScaleX(-1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: {
		m_dungeons.BOSS1->peekDungeon(x, y, 'r');

		if (p.facingDirection() != 'r') {
			m_player->setScaleX(1);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_UP_ARROW: {
		m_dungeons.BOSS1->peekDungeon(x, y, 'u');
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		m_dungeons.BOSS1->peekDungeon(x, y, 'd');
		break;
	}

		//		Non-movement actions
	case EventKeyboard::KeyCode::KEY_Q: { // for using quick access items
		if (m_dungeons.BOSS1->getPlayer().getItems().size() > 0)
			m_dungeons.BOSS1->callUse(m_dungeons.BOSS1->getDungeon(), x, y, 0);

		m_dungeons.BOSS1->peekDungeon(x, y, '-');
		break;
	}
	case EventKeyboard::KeyCode::KEY_S: {
		m_dungeons.BOSS1->peekDungeon(x, y, WIND_UP);
		break;
	}
	case EventKeyboard::KeyCode::KEY_SPACE:
		m_dungeons.BOSS1->peekDungeon(x, y, 'b');
		break;
	case EventKeyboard::KeyCode::KEY_TAB:
	case EventKeyboard::KeyCode::KEY_I: // open inventory/stats screen for viewing
		kbListener->setEnabled(false);
		m_hud->inventoryMenu(kbListener, *m_dungeons.BOSS1);
		break;
	case EventKeyboard::KeyCode::KEY_E: // pick up item/interact
		m_dungeons.BOSS1->peekDungeon(x, y, 'e');
		if (m_dungeons.BOSS1->getLevel() == 4) {
			;//advanceLevel();
		}
		break;
	case EventKeyboard::KeyCode::KEY_W: // open weapon menu
		kbListener->setEnabled(false);
		m_hud->weaponMenu(kbListener, *m_dungeons.BOSS1);

		break;
	case EventKeyboard::KeyCode::KEY_C: // open item menu
		kbListener->setEnabled(false);
		m_hud->itemMenu(kbListener, *m_dungeons.BOSS1);

		break;
	case EventKeyboard::KeyCode::KEY_M: { // toggle music on/off
		if (cocos2d::experimental::AudioEngine::getVolume(id) > 0) {
			cocos2d::experimental::AudioEngine::setVolume(id, 0.0);
		}
		else {
			cocos2d::experimental::AudioEngine::setVolume(id, 1.0);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_P:
	case EventKeyboard::KeyCode::KEY_ESCAPE: // open pause menu
		Director::getInstance()->pushScene(this);
		pauseMenu();
		
		break;
	default:
		m_dungeons.BOSS1->peekDungeon(x, y, '-');
		break;
	}

	// Smasher moves player around, so need to update to true location by
	// taking (new player pos - old player pos) * SpacingFactor
	int px = m_dungeons.BOSS1->getPlayer().getPosX() - x;
	int py = m_dungeons.BOSS1->getPlayer().getPosY() - y;

	// play step sound effect if player moved
	if (px != 0 || py != 0) {
		playFootstepSound();
	}

	//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px*SPACING_FACTOR, -py * SPACING_FACTOR));
	//event->getCurrentTarget()->runAction(move);

	// update the HUD
	m_hud->updateHUD(*m_dungeons.BOSS1);
	m_hud->updateBossHUD(*m_dungeons.BOSS1);


	// Check if player is dead, if so, run game over screen
	if (m_dungeons.BOSS1->getPlayer().getHP() <= 0) {
		cocos2d::experimental::AudioEngine::stop(id);
		m_hud->gameOver();
		return; // prevents timer from being scheduled
	}

	// Check if player found the idol
	if (m_dungeons.BOSS1->getPlayer().getWin()) {
		m_hud->winner();
		return; // prevents timer from being scheduled
	}

	// reschedule the inaction timer
	Director::getInstance()->getScheduler()->schedule([this](float) {
		auto actions = this->getActionManager();

		//// resumes follow if it was paused
		//if (actions->getNumberOfRunningActions() == 0) {
		//	auto visibleSize = Director::getInstance()->getVisibleSize();
		//	this->runAction(Follow::createWithOffset(m_player, -visibleSize.width / 2, -visibleSize.height / 2, Rect::ZERO));
		//}

		// if there are any lingering actions, finish them instantly
		while (actions->getNumberOfRunningActions() > 0) { // >1 because follow player is always running
			actions->update(1.0);
		}
		Player p = m_dungeons.BOSS1->getPlayer();
		int x = p.getPosX(); int y = p.getPosY();

		m_dungeons.BOSS1->peekDungeon(m_dungeons.BOSS1->getPlayer().getPosX(), m_dungeons.BOSS1->getPlayer().getPosY(), '-');

		// Smasher moves player around, so need to update to true location by
		// taking (new player pos - old player pos) * SpacingFactor
		int px = m_dungeons.BOSS1->getPlayer().getPosX() - x;
		int py = m_dungeons.BOSS1->getPlayer().getPosY() - y;

		// play step sound effect if player moved
		if (px != 0 || py != 0) {
			playFootstepSound();
		}

		//cocos2d::Action* move = cocos2d::MoveBy::create(.10, cocos2d::Vec2(px*SPACING_FACTOR, -py * SPACING_FACTOR));
		//m_player->runAction(move);

		// update the HUD
		m_hud->updateHUD(*m_dungeons.BOSS1);
		m_hud->updateBossHUD(*m_dungeons.BOSS1);


		// Check if player is dead, if so, run game over screen
		if (m_dungeons.BOSS1->getPlayer().getHP() <= 0) {
			cocos2d::experimental::AudioEngine::stop(id);
			m_hud->gameOver(*this);
		}

		// Check if player found the idol
		if (m_dungeons.BOSS1->getPlayer().getWin()) {
			m_hud->winner();
		}
	}, this, 0.50, false, "boss 1 timer");
}

void Boss1Scene::pauseMenu() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto pauseMenuScene = PauseMenuScene::createScene();
	pauseMenuScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(pauseMenuScene);
}
void Boss1Scene::advanceLevel() {
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto level3Scene = Level3Scene::createScene(m_dungeons.BOSS1->getPlayer());
	level3Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	// stop music
	cocos2d::experimental::AudioEngine::stop(id);

	Director::getInstance()->replaceScene(level3Scene);
}


//		PAUSE MENU SCENE

/*
PauseMenuScene::PauseMenuScene(cocos2d::Scene* levelcene) : m_scene(scene) {

}
PauseMenuScene* PauseMenuScene::create(cocos2d::Scene* scene) {
	PauseMenuScene *pRet = new(std::nothrow) PauseMenuScene(scene);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
*/
Scene* PauseMenuScene::createScene() {
	auto scene = Scene::create();

	// calls init()
	auto layer = PauseMenuScene::create();
	scene->addChild(layer);

	return scene;
}
bool PauseMenuScene::init() {
	if (!Scene::init()) {
		return false;
	}

	// pause music
	experimental::AudioEngine::pauseAll();


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
	box->setScale(.2);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.5);

	// Pause option
	auto pause = Label::createWithTTF("PAUSE", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);

	// Resume option
	auto resume = Label::createWithTTF("Resume", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, 2 * MENU_SPACING);
	this->addChild(resume, 3);

	// Restart option
	auto restart = Label::createWithTTF("Restart", "fonts/Marker Felt.ttf", 36);
	restart->setPosition(0, 1 * MENU_SPACING);
	this->addChild(restart, 3);

	// "How to play" option
	auto help = Label::createWithTTF("Help", "fonts/Marker Felt.ttf", 36);
	help->setPosition(0, 0 * MENU_SPACING);
	this->addChild(help, 3);

	// Back to Menu option
	auto back = Label::createWithTTF("Main Menu", "fonts/Marker Felt.ttf", 36);
	back->setPosition(0, -1 * MENU_SPACING);
	this->addChild(back, 3);

	// Quit option
	auto exit = Label::createWithTTF("Exit Game", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0, -2 * MENU_SPACING);
	this->addChild(exit, 3);


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

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
		if (index < 4) {
			index++;
			event->getCurrentTarget()->setPosition(pos.x, pos.y - MENU_SPACING);

			cocos2d::experimental::AudioEngine::play2d("Select 1.mp3", false, 1.0f);
		}
		break;
	}
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
		switch (index) {
		case 0: { // Resume
			index = 0;
			experimental::AudioEngine::resumeAll();
			Director::getInstance()->popScene();

			return;
		}
		case 1: { // Restart
			
			experimental::AudioEngine::stopAll(); // remove all sound before creating the next scene

			// generate a new level 1 scene
			auto visibleSize = Director::getInstance()->getVisibleSize();
			Vec2 origin = Director::getInstance()->getVisibleOrigin();

			auto level1Scene = Level1Scene::createScene();
			level1Scene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

			// advance to next scene
			Director::getInstance()->popScene(); // pop original scene off stack
			Director::getInstance()->replaceScene(level1Scene); // replace with new scene
			return;
		}
		case 2: { // Help menu
			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			Director::getInstance()->pushScene(this);
			helpScreen();
			break;
		}
		case 3: {
			// stop music
			cocos2d::experimental::AudioEngine::stopAll();
			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

			// advance to start menu scene
			auto startScene = StartScene::createScene();

			Director::getInstance()->replaceScene(startScene); // replace with new scene
			return;
		}
		case 4: // Exit Game
			auto audio = experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

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
	// prevent pause menu selection movement
	//this->_eventDispatcher->removeEventListener(pauseSelectListener);
	//this->pauseSelectListener->setEnabled(false);


	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();

	auto helpScene = HelpScene::createScene();
	helpScene->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

	Director::getInstance()->replaceScene(helpScene);


	/*
	//    HOW TO PLAY
	//    |---------|
	// -> |  Resume |
	//    |         |
	//    |   ...   |
	//    |         |
	//    |---------|


	// menu border
	Sprite* box = Sprite::create("Pause_Menu_Border_Red.png");
	this->addChild(box, 2);
	box->setPosition(0, 0);
	box->setScale(.2);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.0);

	// HOW TO PLAY
	auto pause = Label::createWithTTF("How to play", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 4.8 * MENU_SPACING);
	this->addChild(pause, 3);

	// Go back
	auto resume = Label::createWithTTF("OK", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, 2 * MENU_SPACING);
	this->addChild(resume, 3);
	//auto sprite = Sprite::create("Right_Arrow.png");
	//sprite->setPosition(-2 * MENU_SPACING, MENU_SPACING);
	//this->addChild(sprite, 4);

	// How to move
	auto restart = Label::createWithTTF("Move", "fonts/Marker Felt.ttf", 36);
	restart->setPosition(0, 1 * MENU_SPACING);
	this->addChild(restart, 3);

	// Pick up/interact
	auto exit = Label::createWithTTF("Use/Interact: e", "fonts/Marker Felt.ttf", 36);
	exit->setPosition(0, 0);
	this->addChild(exit, 3);

	// Switch weapons


	// Use items





	// add help screen event listener
	helpSelectListener = EventListenerKeyboard::create();
	helpSelectListener->onKeyPressed = CC_CALLBACK_2(PauseMenuScene::helpMenuKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(helpSelectListener, sprite);
	*/
}

/// unused
void PauseMenuScene::helpMenuKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		cocos2d::experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

		// remove help menu selection movement
		//this->helpSelectListener->setEnabled(false);
		//this->_eventDispatcher->removeEventListener(helpSelectListener);
		helpSelectListener = nullptr;

		auto sprite = Sprite::create("Right_Arrow.png");
		sprite->setPosition(-2 * MENU_SPACING, 2 * MENU_SPACING);
		this->addChild(sprite, 3);
		sprite->setScale(2.0);

		// add back pause menu event listener
		pauseSelectListener = EventListenerKeyboard::create();
		pauseSelectListener->onKeyPressed = CC_CALLBACK_2(PauseMenuScene::pauseMenuKeyPressed, this);
		this->_eventDispatcher->addEventListenerWithSceneGraphPriority(pauseSelectListener, sprite);
		this->pauseSelectListener->setEnabled(true);
	}
	default: break;
	}
}
/// end


//		HELP SCREEN SCENE
Scene* HelpScene::createScene() {
	auto scene = Scene::create();

	// calls init()
	auto layer = HelpScene::create();
	scene->addChild(layer);

	return scene;
}
bool HelpScene::init() {
	if (!Scene::init()) {
		return false;
	}

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
	box->setPosition(0, 0);
	box->setScale(.3);
	box->setOpacity(170);

	// arrow sprite for selection
	auto sprite = Sprite::create("Right_Arrow.png");
	sprite->setPosition(-2 * MENU_SPACING, -5.2 * MENU_SPACING);
	this->addChild(sprite, 3);
	sprite->setScale(2.5);

	// Go back
	auto resume = Label::createWithTTF("OK", "fonts/Marker Felt.ttf", 36);
	resume->setPosition(0, -5.2 * MENU_SPACING);
	this->addChild(resume, 3);

	// HOW TO PLAY
	auto pause = Label::createWithTTF("How to play", "fonts/Marker Felt.ttf", 48);
	pause->setPosition(0, 5.0 * MENU_SPACING);
	this->addChild(pause, 3);

	// Movement
	auto restart = Label::createWithTTF("Movement:", "fonts/Marker Felt.ttf", 28);
	restart->setPosition(-5 * MENU_SPACING, 2.85 * MENU_SPACING);
	restart->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(restart, 3);

	/*auto up = Sprite::create("KB_Up_Arrow.png");
	up->setPosition(0 * MENU_SPACING, 2.2*MENU_SPACING);
	up->setScale(1.2);
	this->addChild(up, 4);

	auto left = Sprite::create("KB_Left_Arrow.png");
	left->setPosition(-0.4 * MENU_SPACING, 1.8*MENU_SPACING);
	left->setScale(1.2);
	this->addChild(left, 4);

	auto down = Sprite::create("KB_Down_Arrow.png");
	down->setPosition(0 * MENU_SPACING, 1.8*MENU_SPACING);
	down->setScale(1.2);
	this->addChild(down, 4);

	auto right = Sprite::create("KB_Right_Arrow.png");
	right->setPosition(0.4 * MENU_SPACING, 1.8*MENU_SPACING);
	right->setScale(1.2);
	this->addChild(right, 4);*/

	// new arrows
	auto arrowKeyUp = Sprite::create("KB_Arrows_U.png");
	arrowKeyUp->setPosition(0 * MENU_SPACING, 3.35*MENU_SPACING);
	arrowKeyUp->setScale(0.8);
	this->addChild(arrowKeyUp, 4);

	auto arrowKeys = Sprite::create("KB_Arrows_LDR.png");
	arrowKeys->setPosition(0 * MENU_SPACING, 2.6*MENU_SPACING);
	arrowKeys->setScale(0.8);
	this->addChild(arrowKeys, 4);


	// Interact
	auto uselabel = Label::createWithTTF("Grab/Interact:", "fonts/Marker Felt.ttf", 28);
	uselabel->setPosition(-5 * MENU_SPACING, 1.5 * MENU_SPACING);
	uselabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(uselabel, 3);

	auto use = Sprite::create("KB_Black_E.png");
	use->setPosition(0 * MENU_SPACING, 1.5 * MENU_SPACING);
	use->setScale(0.8);
	this->addChild(use, 4);


	// Use item
	auto item = Label::createWithTTF("Use Item:", "fonts/Marker Felt.ttf", 28);
	item->setPosition(-5 * MENU_SPACING, 0.5 * MENU_SPACING);
	item->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(item, 3);

	auto itemKey = Sprite::create("KB_Black_Q.png");
	itemKey->setPosition(0 * MENU_SPACING, 0.5 * MENU_SPACING);
	itemKey->setScale(0.8);
	this->addChild(itemKey, 4);


	// Shield
	auto shield = Label::createWithTTF("Use Shield:", "fonts/Marker Felt.ttf", 28);
	shield->setPosition(-5 * MENU_SPACING, -0.5 * MENU_SPACING);
	shield->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(shield, 3);

	auto spacebar = Label::createWithTTF("SPACE", "fonts/Marker Felt.ttf", 28);
	spacebar->setPosition(0 * MENU_SPACING, -0.5 * MENU_SPACING);
	spacebar->setTextColor(cocos2d::Color4B(255, 255, 255, 255));
	this->addChild(spacebar, 5);

	auto shieldKey = Sprite::create("KB_Black_Space.png");
	shieldKey->setPosition(0 * MENU_SPACING, -0.5 * MENU_SPACING);
	shieldKey->setScale(0.8);
	this->addChild(shieldKey, 4);


	// Open/close weapon menu
	auto weplabel = Label::createWithTTF("Open/Close Weapon Menu:", "fonts/Marker Felt.ttf", 28);
	weplabel->setPosition(-5 * MENU_SPACING, -1.5 * MENU_SPACING);
	weplabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(weplabel, 3);

	auto wepmenu = Sprite::create("KB_Black_W.png");
	wepmenu->setPosition(0 * MENU_SPACING, -1.5 * MENU_SPACING);
	wepmenu->setScale(0.8);
	this->addChild(wepmenu, 4);


	// Open/close item menu
	auto itemlabel = Label::createWithTTF("Open/Close Item Menu:", "fonts/Marker Felt.ttf", 28);
	itemlabel->setPosition(-5 * MENU_SPACING, -2.5* MENU_SPACING);
	itemlabel->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(itemlabel, 3);

	auto itemmenu = Sprite::create("KB_Black_C.png");
	itemmenu->setPosition(0 * MENU_SPACING, -2.5 * MENU_SPACING);
	itemmenu->setScale(0.8);
	this->addChild(itemmenu, 4);


	// View inventory
	auto inventory = Label::createWithTTF("Check inventory:", "fonts/Marker Felt.ttf", 28);
	inventory->setPosition(-5 * MENU_SPACING, -3.5 * MENU_SPACING);
	inventory->setTextColor(cocos2d::Color4B(230, 230, 250, 255));
	this->addChild(inventory, 3);

	auto inventoryKey = Sprite::create("KB_Black_Tab.png");
	inventoryKey->setPosition(0 * MENU_SPACING, -3.5 * MENU_SPACING);
	inventoryKey->setScale(0.8);
	this->addChild(inventoryKey, 4);


	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(HelpScene::helpSceneKeyPressed, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);

	return true;
}

void HelpScene::helpSceneKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_ENTER:
	case EventKeyboard::KeyCode::KEY_SPACE:
	case EventKeyboard::KeyCode::KEY_ESCAPE: {
		experimental::AudioEngine::play2d("Confirm 1.mp3", false, 1.0f);

		Director::getInstance()->popScene();
	}
	default: break;
	}
}


//	SOUND EFFECTS
void playFootstepSound() {
	std::string sound;
	switch (randInt(1)) {
	case 0: sound = "FootStepGeneric1.mp3"; break;
	case 1: sound = "FootStepGeneric2.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, 0.5f);
}


/*
bool Level1Scene::isKeyPressed(EventKeyboard::KeyCode code) {
	// Check if the key is currently pressed by seeing it it's in the std::map keys
	// In retrospect, keys is a terrible name for a key/value paried datatype isnt it?
	if (keys.find(code) != keys.end())
		return true;
	return false;
}

double Level1Scene::keyPressedDuration(EventKeyboard::KeyCode code) {
	if (!isKeyPressed(EventKeyboard::KeyCode::KEY_LEFT_ARROW))
		return 0;  // Not pressed, so no duration obviously

	// Return the amount of time that has elapsed between now and when the user
	// first started holding down the key in milliseconds
	// Obviously the start time is the value we hold in our std::map keys
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::high_resolution_clock::now() - keys[code]).count();
}

void Level1Scene::update(float delta) {
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