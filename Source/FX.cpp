#include "cocos2d.h"
#include "AudioEngine.h"
#include "global.h"
#include "FX.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include <string>

std::map<std::string, SoundManager::SoundInfo> SoundManager::m_activeSounds;

float GLOBAL_MUSIC_VOLUME = 0.5f;
float GLOBAL_SOUND_VOLUME = 0.5f;
float GLOBAL_UI_VOLUME = 0.5f;

//		SPRITE EFFECTS
void runPlayerDamage(cocos2d::Sprite* sprite) {
	auto tintRed = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 70, 70));
	auto tintNormal = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 255, 255));
	auto action2 = sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));

	auto blink = cocos2d::Blink::create(0.2f, 4);
	auto reveal = cocos2d::Show::create();
	auto action1 = sprite->runAction(cocos2d::Sequence::createWithTwoActions(blink, reveal));

	action1->setTag(1);
	action2->setTag(1);
}
void runMonsterDamage(cocos2d::Sprite* sprite) {
	auto tintRed = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 60, 60));
	auto tintNormal = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 255, 255));
	auto action2 = sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));

	auto blink = cocos2d::Blink::create(0.2f, 5);
	auto reveal = cocos2d::Show::create();
	auto action1 = sprite->runAction(cocos2d::Sequence::createWithTwoActions(blink, reveal));

	action1->setTag(5);
	action2->setTag(5);
}
void runMonsterDamageBlink(cocos2d::Sprite* sprite) {
	auto blink = cocos2d::Blink::create(0.08f, 1);
	auto reveal = cocos2d::Show::create();
	auto action = sprite->runAction(cocos2d::Sequence::createWithTwoActions(blink, reveal));
	
	action->setTag(5);
}
void deathFade(cocos2d::Sprite* sprite) {
	float duration = 1.0f;
	auto scale = cocos2d::ScaleTo::create(duration, 0.0f);
	auto fade = cocos2d::FadeOut::create(duration);
	auto move = cocos2d::MoveBy::create(0.5f, cocos2d::Vec2(0, -SPACING_FACTOR / 4));

	cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
	actions.pushBack(scale);
	actions.pushBack(fade);
	actions.pushBack(move);
	auto spawn = cocos2d::Spawn::create(actions);

	auto action = sprite->runAction(spawn);
	action->setTag(5);
}

void tintFrozen(cocos2d::Sprite* sprite) {
	auto tintBlue = cocos2d::TintTo::create(0.0f, cocos2d::Color3B(68, 85, 140));
	sprite->runAction(tintBlue);
	//auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
	//sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintBlue, tintNormal));
}
void tintStunned(cocos2d::Sprite* sprite) {
	auto tintGray = cocos2d::TintTo::create(0.0f, cocos2d::Color3B(100, 100, 100));
	sprite->runAction(tintGray);
	//sprite->runAction(cocos2d::Blink::create(0.2, 3));
}
void tintPoisoned(cocos2d::Sprite* sprite) {
	auto tintGreen = cocos2d::TintTo::create(0.0f, cocos2d::Color3B(173, 255, 47));
	auto reveal = cocos2d::Show::create();
	auto action = sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintGreen, reveal));
	action->setTag(1);
}
void tintItemCast(cocos2d::Sprite* sprite) {
	auto tintGold = cocos2d::TintTo::create(0.0f, cocos2d::Color3B(255, 215, 0));
	auto tint = cocos2d::TintTo::create(0.3f, cocos2d::Color3B(255, 255, 255));
	auto tinting = cocos2d::Sequence::createWithTwoActions(tintGold, tint);
	sprite->runAction(tinting);
}
void tintStaffCast(cocos2d::Sprite* sprite) {
	auto tintGold = cocos2d::TintTo::create(0.f, cocos2d::Color3B(148, 10, 200));
	auto tint = cocos2d::TintTo::create(0.3f, cocos2d::Color3B(255, 255, 255));
	auto tinting = cocos2d::Sequence::createWithTwoActions(tintGold, tint);
	sprite->runAction(tinting);
}
void untint(cocos2d::Sprite* sprite) {
	auto tint = cocos2d::TintTo::create(0.0f, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(tint);
}
void fadeTransparent(cocos2d::Sprite* sprite) {
	auto fadeTransparent = cocos2d::FadeTo::create(.3f, 40);
	//auto fadeTransparentReverse = cocos2d::FadeTo::reverse(fadeTransparent);
	sprite->runAction(fadeTransparent);
	//sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintBlue, tintNormal));
}
void fadeIn(cocos2d::Sprite* sprite) {
	auto fadeIn = cocos2d::FadeTo::create(.3f, 255);
	sprite->runAction(fadeIn);
}

void gotStunned(cocos2d::Sprite* sprite) {
	auto tintGray = cocos2d::TintTo::create(0, cocos2d::Color3B(100, 100, 100));
	sprite->runAction(tintGray);

	auto blink = cocos2d::Blink::create(0.2f, 3);
	auto reveal = cocos2d::Show::create();
	auto action = cocos2d::Sequence::createWithTwoActions(blink, reveal);
	
	action->setTag(5);
	sprite->runAction(action);
}
void bloodlustTint(Actors &a) {
	float percent = a.getHP() / (float)a.getMaxHP();

	int redness = 255; // sprites will have their b and g values reduced to make them appear redder

	if (percent > 0.25f && percent <= 0.4f) redness = 190;
	else if (percent > 0.15f && percent <= 0.25f) redness = 140;
	else if (percent > 0.10f && percent <= 0.15f) redness = 100;
	else if (percent > 0.05f && percent <= 0.10f) redness = 75;
	else if (percent > 0.00f && percent <= 0.05f) redness = 50;

	a.getSprite()->setColor(cocos2d::Color3B(255, redness, redness));
}

void showShieldBlock(Dungeon& dungeon, const Player& p) {
	int x = p.getPosX();
	int y = p.getPosY();

	float n = 0, m = 0;
	switch (p.facingDirection()) {
	case 'l': n = -0.5, m = 0; break;
	case 'r': n = 0.5, m = 0; break;
	case 'u': n = 0, m = -0.5; break;
	case 'd': n = 0; m = 0.5; break;
	}

	cocos2d::Vec2 pos(x + n, y + m);

	cocos2d::Sprite* shield = dungeon.createSprite(pos, getHalfTileYOffset(), y + Z_ACTOR + (m < 0 ? -1 : 0), p.getActiveItem()->getImageName());
	shield->setScale(0.7f * GLOBAL_SPRITE_SCALE);

	auto delay = cocos2d::DelayTime::create(0.3f);
	auto fadeOut = cocos2d::FadeOut::create(0.0f);
	auto remove = cocos2d::RemoveSelf::create();
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(delay);
	v.pushBack(fadeOut);
	v.pushBack(remove);
	auto sequence = cocos2d::Sequence::create(v);
	auto action = shield->runAction(sequence);
	action->setTag(1);
}

void poisonCloud(Dungeon &dungeon, int x, int y, int time, cocos2d::Color3B color) {
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {
			if (dungeon.withinBounds(j, i) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) { // boundary check
				auto tintGreen = cocos2d::TintTo::create(time, color);
				dungeon.tileAt(Coords(j, i)).floor->runAction(tintGreen);
			}
		}
	}
}
void fadeOut(cocos2d::Sprite* sprite) {
	auto fade = cocos2d::FadeOut::create(0.5f);
	sprite->runAction(fade);
}
void flashFloor(Dungeon &dungeon, int x, int y, bool mega) {
	for (int i = y - (1 + mega); i < y + (2 + mega); i++) {
		for (int j = x - (1 + mega); j < x + (2 + mega); j++) {
			if (dungeon.withinBounds(j, i)) {
				auto tintWhite = cocos2d::TintTo::create(0.08f, cocos2d::Color3B(183, 255, 255));
				auto untint = cocos2d::TintTo::create(0.08f, cocos2d::Color3B(200, 200, 200));
				auto tinting = cocos2d::Sequence::createWithTwoActions(tintWhite, untint);
				dungeon.tileAt(Coords(j, i)).floor->runAction(tinting);
				dungeon.tileAt(Coords(j, i)).floor->runAction(cocos2d::Blink::create(0.08f, 1));
			}
		}
	}
}


bool SoundManager::soundExists(const std::string &soundName) {
	if (m_activeSounds.find(soundName) != m_activeSounds.end())
		return true;

	return false;
}
bool SoundManager::currentSoundIsQuieterThan(const std::string &soundName, float volume) {
	if (m_activeSounds.find(soundName)->second.volume < volume)
		return true;

	return false;
}
void SoundManager::addSound(const std::string &soundName, int id, float volume) {
	SoundInfo info;
	info.audioID = id;
	info.volume = volume;

	m_activeSounds.insert(std::make_pair(soundName, info));
}
void SoundManager::adjustVolumeTo(const std::string &soundName, float volume) {
	auto it = m_activeSounds.find(soundName);

	it->second.volume = volume;
	cocos2d::experimental::AudioEngine::setVolume(it->second.audioID, volume);
}
void SoundManager::stopSound(const std::string &soundName) {
	cocos2d::experimental::AudioEngine::stop(m_activeSounds.find(soundName)->second.audioID);
}
void SoundManager::removeSound(const std::string &soundName) {
	m_activeSounds.erase(soundName);
}

//		UI
void playInterfaceSound(std::string sound) {
	cocos2d::experimental::AudioEngine::play2d(sound, false, GLOBAL_UI_VOLUME);
}


//		MUSIC
int playMusic(std::string track, bool loop) {
	return cocos2d::experimental::AudioEngine::play2d(track, loop, GLOBAL_MUSIC_VOLUME);
}


//		SOUND EFFECTS
void playSound(const std::string &sound, float volume, bool loop) {
	// Reserve one sound for game over music
	if (cocos2d::experimental::AudioEngine::getPlayingAudioCount() == cocos2d::experimental::AudioEngine::getMaxAudioInstance() - 1)
		return;

	if (SoundManager::soundExists(sound) && SoundManager::currentSoundIsQuieterThan(sound, volume)) {
		SoundManager::stopSound(sound);
		SoundManager::removeSound(sound);

		//SoundManager::adjustVolumeTo(sound, volume);
		//return;
	}

	int id = cocos2d::experimental::AudioEngine::play2d(sound, loop, volume * GLOBAL_SOUND_VOLUME);
	SoundManager::addSound(sound, id, volume * GLOBAL_SOUND_VOLUME);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[sound](int, const std::string &) {
		SoundManager::removeSound(sound);
	});
}
void playSound(const std::string &sound, const Coords &start, const Coords &end, bool loop) {
	float range = abs(start.x - end.x) + abs(start.y - end.y);

	if (range > 12)
		return;

	float volume = exp(-(range / 3));
	playSound(sound, volume, loop);
}
int playSoundWithID(std::string sound, float volume, bool loop) {
	// Reserve one sound for game over music
	if (cocos2d::experimental::AudioEngine::getPlayingAudioCount() == cocos2d::experimental::AudioEngine::getMaxAudioInstance() - 1)
		return -1;

	return cocos2d::experimental::AudioEngine::play2d(sound, loop, volume * GLOBAL_SOUND_VOLUME);
}

void playBleed(float volume) {
	std::string sound;
	switch (randInt(1, 4)) {
	case 1: sound = "Water_Drip1.mp3"; break;
	case 2: sound = "Water_Drip2.mp3"; break;
	case 3: sound = "Water_Drip3.mp3"; break;
	case 4: sound = "Water_Drip4.mp3"; break;
	}

	playSound(sound);
}
void playPoison(const Player& p, int x, int y) {
	std::string sound;
	switch (randInt(1, 2)) {
	case 1: sound = "Poison_Damage1.mp3"; break;
	case 2: sound = "Poison_Damage2.mp3"; break;
	}

	playSound(sound, p.getPos(), Coords(x, y));
}
void playBirdSound(float volume) {
	std::string sound;
	switch (randInt(1, 3)) {
	case 1: sound = "Bird1.mp3"; break;
	case 2: sound = "Bird2.mp3"; break;
	case 3: sound = "Bird3.mp3"; break;
	case 4: sound = "Crow1.mp3"; break;
	case 5: sound = "Crow2.mp3"; break;
	case 6: sound = "Crow3.mp3"; break;
	case 7: sound = "Crow4.mp3"; break;
	}

	playSound(sound);
}
void playCrowSound(float volume) {
	std::string sound;
	switch (randInt(1, 4)) {
	case 1: sound = "Crow1.mp3"; break;
	case 2: sound = "Crow2.mp3"; break;
	case 3: sound = "Crow3.mp3"; break;
	case 4: sound = "Crow4.mp3"; break;
	}
	playSound(sound);
}
void playGoldDropSound(int gold, int px, int py, int x, int y) {
	std::string sound;

	if (gold == 1) sound = "Coin_Drop1.mp3";
	else if (gold == 2) sound = "Coin_Drop2.mp3";
	else if (gold == 3) sound = "Coin_Drop3.mp3";
	else if (gold > 3) sound = "Coin_Drop5.mp3";
	
	playSound(sound, Coords(px, py), Coords(x, y));
}

void playMonsterDeathByPit(const Player& p, const Monster& m) {
	std::string monster = m.getName();
	if (monster == GOBLIN) {
		playSound("Goblin_Falling.mp3", p.getPos(), m.getPos());
	}
	else if (monster == PIKEMAN) {
		playSound("Knight_Falling.mp3", p.getPos(), m.getPos());
	}

	playSound("Falling_In_A_Hole.mp3", p.getPos(), m.getPos());
}
void playArcherDaggerSwipe() {
	std::string sound;
	switch (randInt(1, 3)) {
	case 1: sound = "Dagger_Swipe1.mp3"; break;
	case 2: sound = "Dagger_Swipe2.mp3"; break;
	case 3: sound = "Dagger_Swipe3.mp3"; break;
	}

	playSound(sound);
}

void playGotHit() {
	std::string sound;
	
	switch (randInt(1, 5)) {
	case 1: sound = "Hit_Female1.mp3"; break;
	case 2: sound = "Hit_Female2.mp3"; break;
	case 3: sound = "Hit_Female3.mp3"; break;
	case 4: sound = "Hit_Female4.mp3"; break;
	case 5: sound = "Hit_Female5.mp3"; break;
	}

	playSound(sound);
	playSound("Player_Hit.mp3");
}
void playMiss(float volume) {
	std::string sound;
	switch (randInt(1, 5)) {
	case 1: sound = "Slash1.mp3"; break;
	case 2: sound = "Slash2.mp3"; break;
	case 3: sound = "Slash3.mp3"; break;
	case 4: sound = "Slash4.mp3"; break;
	case 5: sound = "Slash5.mp3"; break;
	}

	playSound(sound);
}
void playEnemyHit(float volume) {
	playSound("Enemy_Hit.mp3");
}
void playHitSmasher(float volume) {
	std::string sound;

	switch (randInt(1, 5)) {
	case 1: sound = "Metal_Hit1.mp3"; break;
	case 2: sound = "Metal_Hit2.mp3"; break;
	case 3: sound = "Metal_Hit3.mp3"; break;
	case 4: sound = "Metal_Hit4.mp3"; break;
	case 5: sound = "Metal_Hit5.mp3"; break;
	}

	playSound(sound);
}
void playShieldEquip(std::string shield) {
	if (shield == "Wood Shield") {
		playSound("Shield_Pickup.mp3");
	}
	else if (shield == "Iron Shield") {
		playSound("Iron_Shield_Pickup.mp3");
	}
}
void playShieldHit(std::string shield) {
	if (shield == "Wood Shield") {
		playSound("Wood_Shield_Impact.mp3");
	}
	else if (shield == "Iron Shield") {
		playSound("Metal_Hit1.mp3");
	}
	else {
		playSound("Wood_Shield_Impact.mp3");
	}
}
void playShieldBroken(std::string shield) {
	if (shield == "Wood Shield") {
		playSound("Wood_Shield_Broken.mp3");
	}
	else if (shield == "Iron Shield") {
		playSound("Iron_Shield_Broken.mp3");
	}
}

void playBoneCrunch(float volume) {
	std::string sound;
	switch (randInt(0, 0)) {
	case 0:	sound = "Bone_Crack1.mp3"; break;
	case 1: sound = "Bone_Crack2.mp3"; break;
	}
	playSound(sound);
}
void playCrumble(float volume) {
	std::string sound;
	switch (randInt(0, 3)) {
	case 0:	sound = "RockFootStep1.mp3"; break;
	case 1: sound = "RockFootStep2.mp3"; break;
	case 2: sound = "RockFootStep3.mp3"; break;
	case 3: sound = "RockFootStep4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}

void playFootstepSound() {
	std::string sound;
	switch (randInt(0, 0)) {
	case 0: sound = "FootStepGeneric1.mp3"; break;
	case 1: sound = "FootStepGeneric2.mp3"; break;
	}
	playSound(sound, 0.6f);
}