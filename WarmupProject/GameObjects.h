#include <string>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class Objects {
public:
	Objects();
	Objects(int x, int y);
	virtual ~Objects();
	int getPosX() const;
	int getPosY() const;
	void setrandPosX();
	void setrandPosY();
private:
	int m_x;
	int m_y;
};

class Idol : public Objects {
public:
	Idol();
	~Idol();
private:
};

class Drops : public Objects {
public:
	Drops();
	void changeHP();
	void changeArmor();
	void changeDex();
	void changeStr();
private:
};

class Weapon : public Objects {
public:
	Weapon();
	Weapon(int x, int y, std::string action, int dexbonus, int dmg);
	~Weapon();
	std::string getAction() const;
	int getDexBonus() const;
	int getDmg() const;
private:
	std::string m_action;
	int m_dexbonus;
	int m_dmg;
};

class ShortSword : public Weapon {
public:
	ShortSword();
	~ShortSword();
private:
};

class RustyCutlass : public Weapon {
public:
	RustyCutlass(int x, int y);
};

class BoneAxe : public Weapon {
public:
	BoneAxe(int x, int y);
	~BoneAxe();
private:
};

#endif