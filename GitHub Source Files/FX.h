#ifndef FX_H
#define FX_H

class cocos2d::Sprite;
class Dungeon;
class Actors;
class Player;
class Monster;

//		SPRITE EFFECTS
void runPlayerDamage(cocos2d::Sprite* sprite);
void runMonsterDamage(cocos2d::Sprite* sprite);
void runMonsterDamageBlink(cocos2d::Sprite* sprite);
void deathFade(cocos2d::Sprite* sprite);

void tintFrozen(cocos2d::Sprite* sprite);
void tintStunned(cocos2d::Sprite* sprite);
void tintPoisoned(cocos2d::Sprite* sprite);
void tintItemCast(cocos2d::Sprite* sprite);
void tintStaffCast(cocos2d::Sprite* sprite);
void untint(cocos2d::Sprite* sprite);
void fadeTransparent(cocos2d::Sprite* sprite);
void fadeIn(cocos2d::Sprite* sprite);

void gotStunned(cocos2d::Sprite* sprite);
void bloodlustTint(Actors &a);
void showShieldBlock(Dungeon& dungeon, const Player& p);

void poisonCloud(Dungeon &dungeon, int x, int y, int time, cocos2d::Color3B color);
void fadeOut(cocos2d::Sprite* sprite);
void flashFloor(Dungeon &dungeon, int x, int y, bool mega = false);

void playFootstepSound();


//		UI
void playInterfaceSound(std::string sound);


//		MUSIC
int playMusic(std::string track, bool loop);


//		SOUND EFFECTS
void playSound(std::string sound, int px, int py, int x, int y, bool loop = false);
void playSound(std::string sound, float volume = 1.0f, bool loop = false);
int playSoundWithID(std::string sound, float volume = 1.0f, bool loop = false);

void playBleed(float volume = 1.0f);
void playPoison(float volume = 1.0f);
void playBirdSound(float volume = 1.0f);
void playCrowSound(float volume = 1.0f);
void playGoldDropSound(int gold, int px, int py, int x, int y);
void playMonsterDeathByPit(const Player& p, const Monster& m);
void playArcherDaggerSwipe();

void playGotHit();
void playMiss(float volume = 1.0f);
void playEnemyHit(float volume = 1.0f);
void playHitSmasher(float volume = 1.0f);
void playShieldEquip(std::string shield);
void playShieldHit(std::string shield);
void playShieldBroken(std::string shield);

void playBoneCrunch(float volume = 1.0f);
void playCrumble(float volume = 1.0f);



#endif
