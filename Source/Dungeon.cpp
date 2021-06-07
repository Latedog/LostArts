<<<<<<< HEAD
#include "cocos2d.h"
#include "AudioEngine.h"
#include "GUI.h"
#include "global.h"
#include "EffectSprite.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "Afflictions.h"
#include "FX.h"
#include <iostream>
#include <memory>
#include <unordered_set>


Dungeon::Dungeon(LevelScene* scene, std::shared_ptr<Player> p, int level, int rows, int cols) : m_scene(scene), m_level(level), m_rows(rows), m_cols(cols) {
	player.push_back(p);
	player[0]->setDungeon(this);
	if (p->hasDelayedHealing()) {
		p->increaseStatBy(StatType::HP, p->getMaxHP() * p->getDelayedHealing());
		p->setDelayedHealing(0);
	}

	initializeTiles();
	NPCTable::initializeNPCTable();
}
_Tile& Dungeon::tileAt(const Coords &pos) {
	return m_maze[pos.y*m_cols + pos.x];
}
const _Tile& Dungeon::tileAt(const Coords &pos) const {
	return m_maze[pos.y*m_cols + pos.x];
}
Dungeon::~Dungeon() {
	player[0].reset();
	m_exit.reset();

	for (auto &it : m_playerClones)
		it.reset();

	for (auto &it : m_npcs)
		it.reset();

	for (auto &it : m_monsters)
		it.reset();

	for (auto &it : m_traps)
		it.reset();

	for (auto &it : m_items)
		it.reset();

	for (auto &it : m_money)
		it.reset();

	for (auto &it : m_doors)
		it.reset();

	for (auto &it : m_walls)
		it.reset();

	for (auto &it : m_decoys)
		it.reset();

	for (auto &it : m_miscSprites)
		if (it.sprite)
			it.sprite->removeFromParent();
}

void Dungeon::peekDungeon(char move) {
	bool usedMove = false;

	// Move player, unless they have the `Slow` attribute
	if (!player[0]->isSlow()) {
		usedMove = true;
		movePlayers(move);
	}

	// Check these if player does not have Matrix Vision
	if (!player[0]->hasMatrixVision()) {
		checkActive();
		checkDoors();
		checkMonsters();
	}
	// If they do have it, then roll to see if these get skipped
	else if (player[0]->hasMatrixVision()) {

		// Ability was finished and has finished cooling down
		if (matrixTurns == 0 && matrixCooldown == 0) {
			// Failed the roll
			if (randReal(1, 100) <= 95) {
				checkActive();
				checkDoors();
				checkMonsters();
			}
			else {
				matrixTurns = 3;
			}
		}
		// Ability is still active
		else if (matrixTurns > 0) {
			matrixTurns--;

			// Begin cooldown
			if (matrixTurns == 0)
				matrixCooldown = 18;
		}
		// Ability is not off cooldown
		else if (matrixTurns == 0 && matrixCooldown > 0) {
			matrixCooldown--;

			checkActive();
			checkDoors();
			checkMonsters();
		}
	}

	// Player moves after the monsters if they are Slow
	if (player[0]->isSlow() && !usedMove)
		movePlayers(move);

	updateLighting();

	// do any special actions unique to the floor
	specialActions();

	runSpawn();
}

void Dungeon::assignQuickItem(int index) {
	player[0]->setItemToFront(index);

	for (auto &it : m_playerClones)
		it->setItemToFront(index);
}
void Dungeon::callUse(int index) {
	player[0]->use(index);

	for (auto &it : m_playerClones)
		it->use(index);
}

void Dungeon::movePlayers(char move) {
	player[0]->move(move);

	if (!m_playerClones.empty()) {
		for (size_t i = 0; i < m_playerClones.size(); ++i)
			m_playerClones[i]->move(move);
	}
}
void Dungeon::checkActive() {
	for (int i = 0; i < static_cast<int>(m_traps.size()); ++i) {
		if (!m_traps.at(i)->isDestroyed())
			m_traps.at(i)->activeTrapAction();

		if (m_traps.at(i)->isDestroyed())
			insertTrapForRemoval(i);
	}

	trapRemoval();
}
void Dungeon::checkMonsters() {	
	if (m_monsters.empty())
		return;
	
	int mInitHP;
	Coords coords;
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		mInitHP = m_monsters.at(i)->getHP();
		coords = m_monsters.at(i)->getPos();

		// Check if monster is dead or was destroyed earlier
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed()) {
			insertActorForRemoval(i);
			continue;
		}

		m_monsters.at(i)->checkAfflictions();

		if (m_monsters.at(i)->getHP() < mInitHP)
			runMonsterDamageBlink(m_monsters.at(i)->getSprite());
		
		if (!(m_monsters.at(i)->isSpirit() || isBossLevel()) && !positionInRectangularRange(getPlayerCoordsClosestTo(coords), coords, 25, 25))
			continue;

		m_monsters.at(i)->moveCheck();

		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed()) {
			insertActorForRemoval(i);
			continue;
		}

		if (player[0]->isDead()) {
			player[0]->setCauseOfDeath(m_monsters.at(i)->getName());
			break;
		}
	}

	monsterTrapEncounter();
}

int Dungeon::findDecoy(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_decoys.size()); ++i)
		if (m_decoys.at(i)->getPos() == target)
			return i;

	return -1;
}
bool Dungeon::trickedByDecoy(const Coords &start, Coords &newTarget) {
	if (m_decoys.empty())
		return false;

	for (int i = 0; i < static_cast<int>(m_decoys.size()); ++i) {
		Coords currentPos = m_decoys[i]->getPos();

		// If monster is in range of the decoy, chase it
		if (positionInRange(start, currentPos, m_decoys[i]->getAttractRange())) {
			newTarget = currentPos;
			return true;
		}
	}

	return false;
}

void Dungeon::addDoor(std::shared_ptr<Door> door) {
	m_doors.push_back(door);
	m_walls.push_back(door);
	tileAt(door->getPos()).wallObject = door;
}
int Dungeon::findDoor(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_doors.size()); ++i)
		if (m_doors.at(i)->getPos() == target)
			return i;
		
	return -1;
}
void Dungeon::checkDoors() {
	if (!m_doors.empty()) {
		int x, y;

		for (size_t i = 0; i < m_doors.size(); ++i) {
			std::shared_ptr<Door> door = m_doors.at(i);
			x = door->getPosX();
			y = door->getPosY();

			if (door->isOpen() && !door->isHeld() && !(enemy(x, y) || hero(x, y))) {
				playSound("Door_Closed.mp3");

				door->getSprite()->setVisible(true);

				// Close the door
				door->toggleOpen();
				door->toggleHeld();
				tileAt(Coords(x, y)).wall = true;
			}
			else if (door->isOpen() && door->isHeld()) {			
				door->toggleHeld(); // Release hold on the door
			}

			door.reset();
		}
	}
}

int Dungeon::findWall(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_walls.size()); ++i)
		if (m_walls.at(i)->getPos() == target)
			return i;
		
	return -1;
}
void Dungeon::destroyWall(int x, int y) {
	Coords pos(x, y);

	if (!tileAt(pos).wallObject)
		return;

	if (tileAt(pos).wallObject->isDestructible())
		tileAt(pos).wallObject->destroy();
}
void Dungeon::removeWall(int x, int y) {
	int pos = findWall(x, y);

	if (pos == -1)
		return;

	Coords coords(x, y);

	if (m_walls.at(pos)->getName() == UNB_WALL) {
		queueRemoveSprite(m_walls.at(pos)->getSprite());
		tileAt(coords).wall = false;
		tileAt(coords).wallObject.reset();
		tileAt(coords).wallObject = nullptr;
	}

	if (m_walls.at(pos)->isDoor()) {
		int index = findDoor(x, y);
		if (index != -1)
			m_doors.erase(m_doors.begin() + index);
	}

	m_walls.at(pos).reset();
	m_walls.erase(m_walls.begin() + pos);
}

int Dungeon::findNPC(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_npcs.size()); ++i) {
		if (m_npcs.at(i)->getPos() == target)
			return i;		
	}

	return -1;
}
void Dungeon::interactWithNPC(int x, int y) {
	int pos = findNPC(x, y);

	if (pos != -1)
		m_npcs.at(pos)->talk();
}
void Dungeon::playNPCDialogue(const NPC &npc) {
	m_scene->callNPCInteraction(npc);
}
void Dungeon::provokeNPC(const Coords &pos) {
	int index = findNPC(pos.x, pos.y);

	if (index == -1)
		return;

	m_npcs[index]->provoke();
}

//		Trap handling
void Dungeon::trapRemoval() {
	if (m_trapIndexes.empty())
		return;

	// The index set is ordered from smallest to largest, so we destroy in reverse order
	int index;
	for (auto it = m_trapIndexes.crbegin(); it != m_trapIndexes.crend(); it++) {
		if (*it != -1) {
			index = *it;
			
			// If this is a decoy, remove it from the decoy vector too
			if (m_traps.at(index)->isDecoy()) {
				int pos = findDecoy(m_traps.at(index)->getPosX(), m_traps.at(index)->getPosY());
				if (pos != -1)
					m_decoys.erase(m_decoys.begin() + pos);
			}

			m_traps.erase(m_traps.begin() + index);
		}
	}

	m_trapIndexes.clear();
}
void Dungeon::insertTrapForRemoval(int index) {
	m_trapIndexes.insert(index); // queue trap for removal
}
std::vector<int> Dungeon::findTraps(int x, int y) const {
	std::vector<int> traps;
	const Coords target(x, y);

	for (int i = 0; i < static_cast<int>(m_traps.size()); ++i)
		if (m_traps.at(i)->getPos() == target)
			traps.push_back(i);

	return traps;
}
int Dungeon::countTrapNumber(int x, int y) const {
	int count = 0;
	const Coords target(x, y);
	for (size_t i = 0; i < m_traps.size(); ++i) {
		if (m_traps[i]->getPos() == target)
			count++;
	}
	
	return count;
}
void Dungeon::trapEncounter(int x, int y) {
	std::vector<int> indexes = findTraps(x, y);
	for (size_t i = 0; i < indexes.size(); ++i) {
		m_traps.at(indexes[i])->trapActionCheck(*getPlayerAt(x, y));

		if (m_traps.at(indexes[i])->isDestroyed())
			insertTrapForRemoval(indexes[i]);
	}
}
void Dungeon::monsterTrapEncounter() {
	int x, y, mInitHP;
	
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {

		// Ignore if they're dead
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed())
			continue;

		// Ignore monster segments and those that are underground
		if (m_monsters.at(i)->isMonsterSegment() || m_monsters.at(i)->isUnderground())
			continue;

		mInitHP = m_monsters.at(i)->getHP();
		x = m_monsters.at(i)->getPosX();
		y = m_monsters.at(i)->getPosY();

		if (trap(x, y)) {
			std::vector<int> indexes = findTraps(x, y);
			for (size_t idx = 0; idx < indexes.size(); ++idx) {
				m_traps.at(indexes[idx])->trapActionCheck(*m_monsters.at(i));

				if (m_traps.at(indexes[idx])->isDestroyed())
					insertTrapForRemoval(indexes[idx]);
			}
		}

		if (m_monsters.at(i)->getHP() < mInitHP)
			runMonsterDamage(m_monsters.at(i)->getSprite());
		
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed())
			insertActorForRemoval(i);	
	}

	actorRemoval();
	trapRemoval();
}
void Dungeon::singleMonsterTrapEncounter(int pos) {
	if (pos == -1)
		return;

	// Ignore monster segments
	if (m_monsters.at(pos)->isMonsterSegment())
		return;

	int x, y, mInitHP;
	
	mInitHP = m_monsters.at(pos)->getHP();
	x = m_monsters.at(pos)->getPosX();
	y = m_monsters.at(pos)->getPosY();

	if (trap(x, y)) {
		std::vector<int> indexes = findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			m_traps.at(indexes[i])->trapActionCheck(*m_monsters.at(pos));	
	}

	// Flash monster sprite if damage was taken
	if (m_monsters.at(pos)->getHP() < mInitHP)
		runMonsterDamage(m_monsters.at(pos)->getSprite());
}

void Dungeon::damagePlayer(int damage, DamageType type) {
	if (damage <= 0)
		return;

	player[0]->takeDamage(damage, type);
	damageClones(damage, type);
}
void Dungeon::damageClones(int damage, DamageType type) {
	if (!m_playerClones.empty()) {
		for (int i = m_playerClones.size() - 1; i >= 0; --i)
			m_playerClones[i]->takeDamage(damage, type);		
	}
}
void Dungeon::actorRemoval() {
	if (!m_playerClones.empty()) {
		for (int i = m_playerClones.size() - 1; i >= 0; --i) {
			if (m_playerClones[i]->isDead()) {
				m_playerClones[i]->death();
				m_playerClones[i].reset();
				m_playerClones.erase(m_playerClones.begin() + i);
			}
		}
	}

	if (m_deadActorIndexes.empty())
		return;

	int index;
	for (auto it = m_deadActorIndexes.crbegin(); it != m_deadActorIndexes.crend(); it++) {
		if (*it != -1) {
			index = *it;
			monsterDeath(index);
		}
	}

	m_deadActorIndexes.clear();
}
void Dungeon::insertActorForRemoval(int index) {
	m_deadActorIndexes.insert(index);
}
int Dungeon::findMonster(int x, int y) const {
	return findMonster(Coords(x, y));
}
int Dungeon::findMonster(const Coords &pos) const {
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		if (m_monsters.at(i)->getPos() == pos) {
			if (m_monsters.at(i)->isUnderground())
				continue;

			return i;
		}
	}

	return -1;
}
int Dungeon::findUndergroundMonster(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		if (m_monsters.at(i)->getPos() == target) {
			if (!m_monsters.at(i)->isUnderground())
				continue;

			return i;
		}
	}

	return -1;
}
int Dungeon::findSegmentedMonster(int index) const {
	int x, y;

	x = m_monsters.at(index)->getParentX();
	y = m_monsters.at(index)->getParentY();

	return findMonster(x, y);
}
void Dungeon::damageMonster(const Coords &pos, int damage, DamageType type) {
	damageMonster(findMonster(pos), damage, type);
}
void Dungeon::damageMonster(int index, int damage, DamageType type) {
	if (index == -1)
		return;

	if (!m_monsters.at(index)->canBeDamaged(type)) {
		//player[0]->botchedAttack(*m_monsters.at(index));

		return;
	}

	if (m_monsters.at(index)->isMonsterSegment()) {
		index = findSegmentedMonster(index);
		if (index == -1)
			return;
	}
	
	m_monsters.at(index)->decreaseStatBy(StatType::HP, damage);

	/// Potentially call reactToDamage here later.
	//

	if (player[0]->hasExperienceGain() && m_monsters.at(index)->isDead())
		player[0]->increaseXPBy(1);

	runMonsterDamage(m_monsters.at(index)->getSprite());
}
void Dungeon::giveAffliction(int index, std::shared_ptr<Afflictions> affliction) {
	if (m_monsters.at(index)->isMonsterSegment()) {
		index = findSegmentedMonster(index);
		if (index == -1)
			return;
	}

	m_monsters.at(index)->addAffliction(affliction);
}
void Dungeon::monsterDeath(int pos) {
	m_monsters.at(pos)->death();
	m_monsters.erase(m_monsters.begin() + pos);
}

//		Actor pushing
bool Dungeon::monsterHash(int &x, int &y, bool &switched, char move, bool strict) {
	// Let starting x and y be the monster's initial position

	// if current tile is valid, return to set monster here
	if (!(wall(x, y) || enemy(x, y) || hero(x, y)))
		return true;

	switch (move) {
		// 1. if next tile is a wall then switch direction (one time)

		// 2. else if a switch has already been made and a wall is hit, return false

		// 3. otherwise keep looking in the same path

	case 'l':
		if (!strict && wall(x - 1, y) && !switched) {
			if (wall(x, y - 1) && wall(x, y + 1))
				return false;
			
			else if (wall(x, y - 1) && !wall(x, y + 1))
				move = 'd';
			
			else if (wall(x, y + 1) && !wall(x, y - 1))
				move = 'u';
			
			else
				move = (randInt(0, 1) == 0 ? 'u' : 'd');
			
			switched = true;
		}
		else if (wall(x - 1, y) && (switched || strict))
			return false;
		
		break;
	case 'r':
		if (!strict && wall(x + 1, y) && !switched) {
			if (wall(x, y - 1) && wall(x, y + 1))
				return false;
			
			else if (wall(x, y - 1) && !wall(x, y + 1))
				move = 'd';
			
			else if (wall(x, y + 1) && !wall(x, y - 1))
				move = 'u';
			
			else
				move = (randInt(0, 1) == 0 ? 'u' : 'd');
			
			switched = true;
		}
		else if (wall(x + 1, y) && (switched || strict))
			return false;
		
		break;
	case 'u':
		if (!strict && wall(x, y - 1) && !switched) {
			if (wall(x - 1, y) && wall(x + 1, y))
				return false;
			
			else if (wall(x - 1, y) && !wall(x + 1, y))
				move = 'r';
			
			else if (wall(x + 1, y) && !wall(x - 1, y))
				move = 'l';
			
			else
				move = (randInt(0, 1) == 0 ? 'l' : 'r');
			
			switched = true;
		}
		else if (wall(x, y - 1) && (switched || strict))
			return false;
		
		break;
	case 'd':
		if (!strict && wall(x, y + 1) && !switched) {
			if (wall(x - 1, y) && wall(x + 1, y))
				return false;
			
			else if (wall(x - 1, y) && !wall(x + 1, y))
				move = 'r';
			
			else if (wall(x + 1, y) && !wall(x - 1, y))
				move = 'l';
			
			else
				move = (randInt(0, 1) == 0 ? 'l' : 'r');
			
			switched = true;
		}
		else if (wall(x, y + 1) && (switched || strict))
			return false;
		
		break;
	}

	if (move == 'l') return monsterHash(--x, y, switched, move);
	if (move == 'r') return monsterHash(++x, y, switched, move);
	if (move == 'u') return monsterHash(x, --y, switched, move);
	if (move == 'd') return monsterHash(x, ++y, switched, move);

	return false;
}
void Dungeon::pushMonster(int &mx, int &my, char move, int cx, int cy, bool strict) {
	bool b = false;
	int ox = mx;
	int oy = my;

	int pos = findMonster(mx, my);
	if (pos == -1)
		return;

	// If this is a monster segment, don't bother moving it--the parent will move it
	if (m_monsters.at(pos)->isMonsterSegment())
		return;

	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd': {
		if (monsterHash(mx, my, b, move, strict))
			m_monsters.at(pos)->moveTo(mx, my, 0.05f);
		
		// Kill the monster
		else
			m_monsters.at(pos)->setDead(true);
		
		break;
	}
			  //	CUSTOM MOVEMENT CASE in X direction
	case 'X': {
		m_monsters.at(pos)->moveTo(cx, my, 0.05f);
		break;
	}
			  //	CUSTOM MOVEMENT CASE in Y direction
	case 'Y': {
		m_monsters.at(pos)->moveTo(mx, cy, 0.05f);
		break;
	}
	}

}
void Dungeon::linearActorPush(int x, int y, int limit, int range, char move, bool pulling, bool lethal) {
	// x and y are the coordinates of the first actor to be pushed

	int ox = x, oy = y;

	std::stack<std::shared_ptr<Actors>> st;
	int pos = findMonster(x, y);
	if (pos != -1) {
		if (!m_monsters.at(pos)->isHeavy())
			st.push(m_monsters[pos]);
	}
	
	// Can be both because of Sand Centipede
	if (hero(x, y)) {
		if (!getPlayerAt(x, y)->isHeavy())
			st.push(getPlayerAt(x, y));
	}

	if (st.empty())
		return;

	int n, m;
	setDirectionalOffsets(move, n, m);

	// Find all actors we need to push
	int k = range;
	while (k > 0 + pulling) {

		switch (move) {
		case 'l': x--; break;
		case 'r': x++; break;
		case 'u': y--; break;
		case 'd': y++; break;
		case '1': x--; y--; break;
		case '2': x++; y--; break;
		case '3': x--; y++; break;
		case '4': x++; y++; break;
		}

		if (!withinBounds(x, y) || wall(x, y))
			break;

		if (enemy(x, y)) {
			int index = findMonster(x, y);

			if (index != -1) {
				if (m_monsters.at(index)->isHeavy())
					break;

				st.push(m_monsters.at(index));
			}
		}

		if (hero(x, y)) {

			// If the player is heavy, don't add them and stop
			if (getPlayer()->isHeavy())
				break;

			st.push(getPlayer());
		}

		k--;
	}

	// Now push the actors
	int ax = st.top()->getPosX(), ay = st.top()->getPosY();
	bool firstActor = true;

	// Calculates number of free spaces between first monster and last
	int lastActorPushDist = st.size() == 1 ? 0 : std::max(abs(ax - ox), abs(ay - oy)) - (st.size() - 2) - 1;
	while (!st.empty()) {
		std::shared_ptr<Actors> a = st.top();
		st.pop();

		ax = a->getPosX();
		ay = a->getPosY();
		Coords currentPos(a->getPos());
		int index = findMonster(currentPos);
		k = !firstActor ? limit : limit - lastActorPushDist;

		while (k > 0) {

			// Lethal flag indicates the actor should be killed if it was the last one against a wall
			if (wall(ax + n, ay + m) && lethal) {
				a->setHP(0);
				deathFade(a->getSprite());
				playSound("Bone_Crack1.mp3", getPlayer()->getPos(), a->getPos());

				if (a->isPlayer())
					tileAt(currentPos).hero = false;
				else {
					m_monsters[index]->setDestroyed(true);
					tileAt(currentPos).enemy = false;
				}

				break;
			}

			if (enemy(ax + n, ay + m) || wall(ax + n, ay + m) || hero(ax + n, ay + m))
				break;

			if (a->isPlayer())
				a->moveTo(ax + n, ay + m, 0.07f);
			else if (index != -1)
				m_monsters[index]->moveTo(ax + n, ay + m, 0.05f);

			if (trap(ax + n, ay + m)) {

				if (a->isPlayer()) {
					trapEncounter(ax + n, ay + m);

					if (a->isDead())
						break;
				}
				else if (index != -1) {
					singleMonsterTrapEncounter(index);

					if (m_monsters.at(index)->isDead())
						break;
				}
			}

			ax += n;
			ay += m;

			k--;
		}

		if (firstActor)
			firstActor = false;
	}

}

//		Items
void Dungeon::createItem(std::shared_ptr<Objects> item) {
	Coords pos = item->getPos();

	tileAt(pos).object = item;
	tileAt(pos).object->setSprite(createSprite(pos, pos.y + Z_ITEM, item->getImageName()));
	tileAt(pos).item = true;

	addItem(item);
}
void Dungeon::createChest(std::shared_ptr<Chests> chest) {
	Coords pos = chest->getPos();
	tileAt(pos).object = chest;
	addItem(chest);
}
int Dungeon::findItem(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
		if (m_items.at(i)->getPos() == target)
			return i;

	return -1;
}
void Dungeon::removeItem(int x, int y) {
	int pos = findItem(x, y);
	
	if (pos != -1) {
		removeSprite(m_items.at(pos)->getSprite());
		m_items.erase(m_items.begin() + pos);
	}
	
	Coords target(x, y);
	tileAt(target).item = false;
	
	if (tileAt(target).object)
		tileAt(target).object.reset();

	tileAt(target).object = nullptr;
}
void Dungeon::removeItemWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	int pos = findItem(x, y);

	if (pos != -1) {
		queueRemoveSpriteWithMultipleActions(m_items.at(pos)->getSprite(), actions);
		m_items.erase(m_items.begin() + pos);
	}

	Coords target(x, y);

	tileAt(target).item = false;
	tileAt(target).object.reset();
	tileAt(target).object = nullptr;
}
void Dungeon::itemHash(int &x, int &y) {
	// if current tile has no item and the position is valid, return to set item here
	if (!(itemObject(x, y) || wall(x, y) || trap(x, y)))
		return;

	int n = randInt(-1, 1);
	int m = randInt(-1, 1);

	// while hash draws out of bounds, find a new position
	while (!withinBounds(x + n, y + m) || wallObject(x + n, y + m)) {
		n = randInt(-1, 1);
		m = randInt(-1, 1);
	}

	// hash until we find a valid spot
	itemHash(x += n, y += m);
}
void Dungeon::collectItem(int x, int y) {
	bool itemAdded = false;

	std::shared_ptr<Objects> object = itemObject(x, y);

	if (exit(x, y)) {
		m_exit->interact();
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		playSound("Player_Movement.mp3");

		return;
	}

	// Beat the 'final' boss
	if (object->getName() == "Idol") {
		player[0]->setWin(true);
		return;
	}

	if (object->isAutoUsable()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->useItem(*this, *getPlayerAt(x, y));

		playSound(drop->getSoundName());

		drop.reset();

		removeItem(x, y);
	}
	else if (object->isUsable()) {

		// For Spellcaster
		if (object->isSpell() && player[0]->getName() == SPELLCASTER) {
			playSound(object->getSoundName());

			player[0]->equipActiveItem(object);
			removeItem(x, y);

			return;
		}

		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		player[0]->addItem(drop, itemAdded);
		drop.reset();
	}
	else if (object->isPassive()) {
		std::shared_ptr<Passive> passive = std::dynamic_pointer_cast<Passive>(object);
		player[0]->equipPassive(passive);
		passive.reset();

		itemAdded = true;
	}
	else if (object->isWeapon()) {
		std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
		player[0]->storeWeapon(weapon);
		playSound(weapon->getSoundName());

		weapon.reset();
	}
	else if (object->isShield()) {
		if (player[0]->canUseShield())
			player[0]->equipActiveItem(object);		
	}

	if (itemAdded) {
		removeItem(x, y);
	}
}
int Dungeon::findGold(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_money.size()); ++i)
		if (m_money.at(i)->getPos() == target)
			return i;

	return -1;
}
void Dungeon::goldPickup(int x, int y) {
	playSound("Gold_Pickup2.mp3");

	player[0]->setMoney(player[0]->getMoney() + gold(x, y));

	removeGold(x, y);
}
void Dungeon::addGold(int x, int y, int amount) {
	tileAt(Coords(x, y)).gold += amount;
	int gold = Dungeon::gold(x, y);

	if (gold == 0)
		return;

	std::string image;
	if (gold == 1) image = "Gold_Coin1_48x48.png";
	else if (gold == 2) image = "Gold_Coin2_48x48.png";
	else if (gold == 3) image = "Gold_Coin3_48x48.png";
	else if (gold >= 4 && gold <= 10) image = "Gold_Pile1_48x48.png";
	else if (gold > 10) image = "Gold_Pile2_48x48.png";

	// if there's gold here already, remove the sprite before we add a new one
	
	int pos = findGold(x, y);
	if (pos != -1) {
		removeSprite(m_money.at(pos)->getSprite());
		m_money.at(pos)->setSprite(createSprite(x, y, y + Z_MONEY, image));
		return;
	}
	
	m_money.push_back(std::make_shared<Gold>(*this, x, y, image));
}
void Dungeon::removeGold(int x, int y) {
	tileAt(Coords(x, y)).gold = 0;

	int pos = findGold(x, y);
	if (pos != -1) {
		removeSprite(m_money.at(pos)->getSprite());
		m_money.at(pos).reset();
		m_money.erase(m_money.begin() + pos);
	}
}
void Dungeon::removeGoldWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	tileAt(Coords(x, y)).gold = 0;

	int pos = findGold(x, y);

	if (pos != -1) {
		queueRemoveSpriteWithMultipleActions(m_money.at(pos)->getSprite(), actions);
		m_money.at(pos).reset();
		m_money.erase(m_money.begin() + pos);
	}
}

//		Helper
std::shared_ptr<Player> Dungeon::getPlayerAt(int x, int y) const {
	return getPlayerAt(Coords(x, y));
}
std::shared_ptr<Player> Dungeon::getPlayerAt(const Coords &pos) const {
	if (player[0]->getPos() == pos)
		return player[0];

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		if (m_playerClones[i]->getPos() == pos)
			return m_playerClones[i];
	}

	// If all else fails, return the real player for safety
	return player[0];
}
std::vector<Coords> Dungeon::getCoordsOfAllPlayers() const {
	std::vector<Coords> coords;
	coords.push_back(player[0]->getPos());

	for (size_t i = 0; i < m_playerClones.size(); ++i)
		coords.push_back(m_playerClones[i]->getPos());	

	return coords;
}
Coords Dungeon::getPlayerCoordsClosestTo(const Coords &start) const {
	Coords end(player[0]->getPos());

	if (m_playerClones.empty())
		return end;

	int minDistance = calculateDistanceBetween(start, end);

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		Coords other = m_playerClones[i]->getPos();

		int currentDistance = calculateDistanceBetween(start, other);

		if (currentDistance < minDistance) {
			end = other;
			minDistance = currentDistance;
		}
		else if (currentDistance == minDistance && randReal(1, 100) > 50)
			end = other; // Allows for nondeterministic behavior
	}

	return end;
}
Coords Dungeon::findNearbyFreeTile(const Coords &start) const {
	int x = start.x;
	int y = start.y;
	std::vector<Coords> coords;

	while (coords.empty()) {
		if (!withinBounds(x, y)) {
			x += randInt(-1, 1);
			y += randInt(-1, 1);
			continue;
		}

		if (!tileIsBlocked(x - 1, y))
			coords.push_back(Coords(x - 1, y));

		if (!tileIsBlocked(x + 1, y))
			coords.push_back(Coords(x + 1, y));

		if (!tileIsBlocked(x, y - 1))
			coords.push_back(Coords(x, y - 1));

		if (!tileIsBlocked(x, y + 1))
			coords.push_back(Coords(x, y + 1));

		if (!coords.empty())
			break;

		x += randInt(-1, 1);
		y += randInt(-1, 1);
	}

	int index = randInt(0, static_cast<int>(coords.size()) - 1);
	return coords[index];
}
bool Dungeon::tileIsBlocked(int x, int y) const {
	return (wall(x, y) || enemy(x, y) || hero(x, y));
}
bool Dungeon::withinBounds(int x, int y) const {
	if (x < 0 || x > m_cols - 1 || y < 0 || y > m_rows - 1)
		return false;

	return true;
}

//		Other
void Dungeon::factoryTileCreation() {
	m_scene->callFactoryTileCreation();
}
void Dungeon::clonePlayer() {
	std::shared_ptr<Player> newClone = player[0]->clone();

	// Clones have half the HP of the original player.
	// The player also has their HP reduced.
	newClone->setHP(std::max(1, player[0]->getHP() / 2));
	player[0]->setHP(std::max(1, (player[0]->getHP() * 3) / 4));

	newClone->setSprite(createSprite(player[0]->getPos(), getHalfTileYOffset(), player[0]->getPosY() + Z_ACTOR, player[0]->getImageName()));

	Coords newPos = findNearbyFreeTile(player[0]->getPos());

	newClone->setPos(newPos);
	tileAt(newPos).hero = true;
	queueMoveSprite(newClone->getSprite(), newPos, getHalfTileYOffset());
	newClone->getSprite()->setLocalZOrder(newPos.y + Z_ACTOR);

	m_playerClones.push_back(newClone);
}
void Dungeon::chainLightning(const Coords &start) {
	std::unordered_set<Coords> coordsSeen;

	// Key: Actor's coordinates, Value: Index in the monster vector
	std::stack<std::pair<Coords, int>> st;

	Coords currentPos = start;
	int index = findMonster(currentPos);
	st.push(std::make_pair(currentPos, index));

	int radius = 1;
	while (!st.empty()) {
		currentPos = st.top().first;
		index = st.top().second;
		st.pop();

		for (int i = currentPos.x - 1; i < currentPos.x + 2; ++i) {
			for (int j = currentPos.y - 1; j < currentPos.y + 2; ++j) {
				Coords pos(i, j);

				if (coordsSeen.find(pos) != coordsSeen.end())
					continue;

				coordsSeen.insert(pos);

				// Add any adjacent enemies onto the stack
				if (enemy(i, j))
					st.push(std::make_pair(pos, findMonster(pos)));
			}
		}

		// Now damage the current actor
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		runSingleAnimation(frames, 120, currentPos, currentPos.y + Z_PROJECTILE);

		damageMonster(index, getPlayer()->getInt() * 1.5, DamageType::LIGHTNING);
	}
}

//		Shop prices
void Dungeon::constructShopHUD(int x, int y) {
	return;
}
void Dungeon::deconstructShopHUD() {
	return;
}

//		*** Fill level ***
void Dungeon::initializeTiles() {
	int rows = m_rows;
	int cols = m_cols;

	_Tile *tile;
	int i, j;

	//	Initialize entire dungeon to blank space
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			tile = new _Tile;
			tile->upper = SPACE;
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = tile->spirit = false;
			tile->gold = 0;
			tile->marked = tile->noSpawn = false;
			tile->shop_action = "";
			tile->object = nullptr;
			tile->wallObject = nullptr;

			m_maze.push_back(*tile);
			delete tile;

			createFlooring(j, i);
		}
	}

	//	Initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++)
			m_walls.push_back(std::make_shared<Boundary>(this, j, i));
		
		i += (rows - 2);
	}

	//	Initialize edges of dungeon to be unbreakable walls
	for (i = 0; i < rows - 1; i++) {
		m_walls.push_back(std::make_shared<Boundary>(this, 0, i));
		m_walls.push_back(std::make_shared<Boundary>(this, cols - 1, i));
	}
}
void Dungeon::createFlooring(int x, int y) {
	Coords pos(x, y);
	std::string image;

	switch (randInt(1, 8)) {
	case 1: image = "FloorTile1_48x48.png"; break;
	case 2: image = "FloorTile2_48x48.png"; break;
	case 3: image = "FloorTile3_48x48.png"; break;
	case 4: image = "FloorTile4_48x48.png"; break;
	case 5: image = "FloorTile5_48x48.png"; break;
	case 6: image = "FloorTile6_48x48.png"; break;
	case 7: image = "FloorTile7_48x48.png"; break;
	case 8: image = "FloorTile8_48x48.png"; break;
	}

	cocos2d::Sprite* floor = createSprite(pos, Z_FLOOR, image);
	floor->setOpacity(252);

	if (m_level != FIRST_BOSS)
		floor->setColor(cocos2d::Color3B(0, 0, 0));

	tileAt(pos).floor = floor;
}

void Dungeon::generateNPCs() {
	int cols = m_cols;
	int rows = m_rows;

	int x, y;

	// BEGIN NPC GENERATION
	std::shared_ptr<NPC> npc(nullptr);
	int spawnChance = 25 + (player[0]->hasCharismaNPC() ? 25 : 0) + (player[0]->getFavor() <= -12 ? -15 : 0);

	for (int i = 0; i < 2; i++) {

		double roll = randReal(1, 100);
		if (roll < spawnChance) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || trap(x, y) || item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			npc = rollNPC(this, x, y);
			if (!npc)
				break;

			m_npcs.push_back(npc);
			npc.reset();

			spawnChance /= 2; // Halve the spawn chance every time an npc is spawned
		}
	}
	/// END NPC GENERATION
}
void Dungeon::generateMonsters() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	// BEGIN MONSTER GENERATION
	std::shared_ptr<Monster> monster(nullptr);
	std::string name;

	for (int i = 0; i < 17; i++) {

		switch (i) {
		case 0: n = 10 + randInt(0, 2); name = SEEKER; break;
		case 1: n = 8 + randInt(0, 2); name = GOO_SACK; break;
		case 2: n = 8 + randInt(0, 3); name = ROUNDABOUT; break;
		case 3: n = 5 + randInt(0, 3); name = BROUNDABOUT; break;
		case 4: n = 6 + randInt(0, 3); name = RAT; break;
		case 5: n = 3 + randInt(0, 3); name = SPIDER; break;
		case 6: n = 3 + randInt(0, 3); name = SHOOTING_SPIDER; break;
		case 7: n = 3 + randInt(0, 3); name = POUNCING_SPIDER; break;
		case 8: n = 3 + randInt(0, 3); name = GOBLIN; break;
		case 9: n = 10 + randInt(0, 3); name = WANDERER; break;
		case 10: n = 6 + randInt(0, 3); name = SLEEPING_WANDERER; break;
		case 11: n = 6 + randInt(0, 3); name = PROVOCABLE_WANDERER; break;
		case 12: n = 0 + randInt(0, 3); name = RABID_WANDERER; break;
		case 13: n = 0 + randInt(0, 3); name = TOAD; break;
		case 14: n = 3 + randInt(0, 3); name = WEAK_CRATE; break;
		case 15: n = 3 + randInt(0, 3); name = WEAK_BARREL; break;
		case 16: n = 3 + randInt(0, 3); name = WEAK_LARGE_POT; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			addMonster(createMonsterByName(name, this, x, y));

			n--;
		}
	}
	/// END MONSTER GENERATION
}
void Dungeon::generateTraps() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	//	TRAP GENERATION
	std::shared_ptr<Traps> trap(nullptr);

	for (int i = 0; i < 9; i++) { // i < number_of_different_trap_types

		// number of each trap to spawn
		switch (i) {
		case 0: n = 2 + randInt(0, 3); break; // Pit
		case 1: n = 10 + randInt(0, 0); break; // Firebars
		case 2: n = 10 + randInt(0, 0); break; // Braziers
		case 3: n = 15 + randInt(0, 5); break; // Puddles
		case 4: n = 5 + randInt(0, 10); break; // Springs
		case 5: n = 15 + randInt(0, 3); break; // Turrets
		case 6: n = 3 + randInt(0, 3); break; // 
		case 7: n = 10 + randInt(0, 3); break; // 
		case 8: n = 3 + randInt(0, 3); break; // 
		default: n = 0; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			// while position clashes with wall, player, etc., then reroll its position
			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || Dungeon::trap(x, y) || item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// Type of trap to spawn
			switch (i) {
			case 0:	trap = std::make_shared<Pit>(*this, x, y); break;
			case 1:	trap = std::make_shared<Stalactite>(*this, x, y); break;
			case 2:	trap = std::make_shared<Brazier>(*this, x, y); break;
			case 3:	trap = std::make_shared<Water>(*this, x, y); break;
			case 4: trap = std::make_shared<Spring>(*this, x, y, false); break;			
			case 5: trap = std::make_shared<Turret>(*this, x, y); break;
			case 6:	trap = std::make_shared<Puddle>(*this, x, y); break;
			case 7:	trap = std::make_shared<CrumblePit>(*this, x, y, 4); break;
			case 8:	trap = std::make_shared<PoisonWater>(*this, x, y); break;
			}

			addTrap(trap);

			n--;
			trap.reset();
		}
	}
	/// END TRAP GENERATION
}
void Dungeon::generateItems() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	//	ITEM GENERATION
	std::shared_ptr<Objects> item(nullptr);

	for (int i = 0; i < 21; i++) { // i < number_of_different_items

		// number of each item to spawn
		switch (i) {
		case 0: n = 1; break; // Life Potion
		case 1: n = 1; break; // Bomb
		case 2: n = 2; break; // Brown Chest
		case 3: n = 1; break; // Rusty Cutlass
		case 4: n = 1; break; // Skeleton Key
		case 5: n = 1; break; // Wood Shield
		case 6: n = 2; break; // Freeze Spell
		case 7: n = 1 + randInt(0, 2); break; // Earthquake Spell
		case 8: n = 2 + randInt(0, 2); break; // 
		case 9: n = 2 + randInt(0, 2); break; // 
		case 10: n = 2 + randInt(0, 2); break; // 
		case 11: n = 1 + randInt(0, 0); break; // 
		case 12: n = 1; break;
		case 13: n = 1; break;
		case 14: n = 1; break;
		case 15: n = 1; break;
		case 16: n = 1; break;
		case 17: n = 1; break;
		case 18: n = 1; break;
		case 19: n = 1; break;
		case 20: n = 1; break;
		case 21: n = 1; break;
		default: n = 0; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || hero(x, y) || trap(x, y) || Dungeon::item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// Type of item to spawn
			switch (i) {
			case 0:	item = std::make_shared<LifePotion>(x, y); break;
			case 1: item = std::make_shared<Bomb>(x, y); break;
			case 2:	item = std::make_shared<Firecrackers>(x, y); break;
			case 3:	item = std::make_shared<RockSummonSpell>(x, y); break;
			case 4:	item = std::make_shared<IceShardSpell>(x, y); break;
			case 5:	item = std::make_shared<WoodShield>(x, y); break;
			case 6:	item = std::make_shared<FreezeSpell>(x, y); break;
			case 7:	item = std::make_shared<EarthquakeSpell>(x, y); break;
			case 8:	item = std::make_shared<FireBlastSpell>(x, y); break;
			case 9:	item = std::make_shared<WhirlwindSpell>(x, y); break;
			case 10: item = std::make_shared<InvisibilitySpell>(x, y); break;
			case 11: item = std::make_shared<SteadyLifePotion>(x, y); break;
			case 12: item = std::make_shared<HailStormSpell>(x, y); break;
			case 13: item = std::make_shared<LuckUp>(x, y); break;
			case 14: item = std::make_shared<EtherealSpell>(x, y); break;
			case 15: item = std::make_shared<ShockwaveSpell>(x, y); break;
			case 16: item = std::make_shared<Matches>(x, y); break;
			case 17: item = std::make_shared<WindBlastSpell>(x, y); break;
			case 18: item = std::make_shared<BearTrap>(x, y); break;
			case 19: item = std::make_shared<FireCascadeSpell>(x, y); break;
			case 20: item = std::make_shared<WindVortexSpell>(x, y); break;
			}

			tileAt(Coords(x, y)).object = item;
			tileAt(Coords(x, y)).item = true;
			item->setSprite(createSprite(x, y, y + isShop() ? 2 : Z_ITEM, item->getImageName()));
			addItem(item);

			n--;
			item.reset();
		}
	}
	/// END ITEM GENERATION
}

void Dungeon::fillLevel(std::vector<std::string> finalvec, int start, int end) {
	int rows = getRows();
	int cols = getCols();

	int count = 0;
	std::shared_ptr<Monster> monster(nullptr);

	for (int i = start; i < rows - 1; i++) {
		for (int j = 1; j < cols + end; j++) {
			Coords currentPos(j, i);

			if (finalvec.at(count) == RANDOM_WALL) {
				// 50% chance to spawn wall
				if (randInt(0, 1) == 0) {
					std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
					m_walls.push_back(wall);
					tileAt(currentPos).wallObject = wall;
				}
			}
			else if (finalvec.at(count) == REG_WALL) {
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(Coords(j, i)).wallObject = wall;
			}
			else if (finalvec.at(count) == UNB_WALL) {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}
			else if (finalvec.at(count) == BOUNDARY) {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}
			else if (finalvec.at(count) == DOOR_HORIZONTAL) {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_HORIZONTAL));
			}
			else if (finalvec.at(count) == DOOR_VERTICAL) {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
			}
			else if (finalvec.at(count) == SPAWN_DISALLOWED) {
				tileAt(currentPos).noSpawn = true;
			}
			else if (finalvec.at(count) == STAIRCASE) {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
			}
			else if (finalvec.at(count) == LOCKED_STAIRCASE) {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i, true);
				addTrap(stairs);

				m_exit = stairs;
			}
			else if (finalvec.at(count) == DEVILS_WATER) {
				addTrap(std::make_shared<DevilsWater>(this, j, i));
			}
			else if (finalvec.at(count) == FOUNTAIN) {
				std::shared_ptr<Wall> wall = std::make_shared<Fountain>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}

			else if (finalvec.at(count) == PLAYER) {

				tileAt(currentPos).hero = true;				
				player[0]->setPos(currentPos);

				if (m_level != 1) {
					//player[0]->equipTrinket(*this, std::make_shared<>());
					//player[0]->addItem(std::make_shared<Bomb>());
					//player[0]->equipShield(*this, std::make_shared<WoodShield>());

					count++;
					continue;
				}

				//player[0]->setMoney(200);
				/*player[0]->addItem(std::make_shared<FireExplosionSpell>());
				player[0]->addItem(std::make_shared<FireExplosionSpell>());
				player[0]->addItem(std::make_shared<FireExplosionSpell>());*/
				player[0]->addItem(std::make_shared<RottenMeat>(0, 0));
				//player[0]->addItem(std::make_shared<FireBlastSpell>());
				//player[0]->addItem(std::make_shared<ShockwaveSpell>());
				//player[0]->addItem(std::make_shared<PoisonBomb>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				//player[0]->equipShield(*this, std::make_shared<ReflectShield>());
				player[0]->equipRelic(std::make_shared<SuperMagicEssence>(0, 0));
				player[0]->getRelic()->upgrade(*this, *player[0]);
				player[0]->getRelic()->upgrade(*this, *player[0]);
				player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->equipTrinket(*this, std::make_shared<CursedStrength>());
				//player[0]->equipPassive(*this, std::make_shared<ResonantSpells>());
				//player[0]->equipPassive(*this, std::make_shared<Scavenger>(0, 0));
			}
			else if (finalvec.at(count) == GOBLIN) {
				addMonster(std::make_shared<Goblin>(this, j, i, 12));
			}
			else if (finalvec.at(count) == SEEKER) {
				addMonster(std::make_shared<Seeker>(this, j, i, 10));
			}
			else if (finalvec.at(count) == DEAD_SEEKER) {
				addMonster(std::make_shared<DeadSeeker>(this, j, i));
			}
			else if (finalvec.at(count) == ITEM_THIEF) {
				addMonster(std::make_shared<ItemThief>(this, j, i));
			}
			else if (finalvec.at(count) == ARCHER) {
				addMonster(std::make_shared<Archer>(this, j, i));
			}
			else if (finalvec.at(count) == WANDERER) {
				addMonster(std::make_shared<Wanderer>(this, j, i));
			}
			else if (finalvec.at(count) == ZAPPER) {
				addMonster(std::make_shared<Zapper>(this, j, i));
			}
			else if (finalvec.at(count) == SPINNER) {
				addMonster(std::make_shared<Spinner>(this, j, i));
			}
			else if (finalvec.at(count) == GOO_SACK) {
				addMonster(std::make_shared<GooSack>(this, j, i));
			}
			else if (finalvec.at(count) == BOMBEE) {
				addMonster(std::make_shared<Bombee>(this, j, i));
			}
			else if (finalvec.at(count) == ROUNDABOUT) {
				addMonster(std::make_shared<Roundabout>(this, j, i));
			}
			else if (finalvec.at(count) == FIRE_ROUNDABOUT) {
				addMonster(std::make_shared<FireRoundabout>(this, j, i));
			}
			else if (finalvec.at(count) == BROUNDABOUT) {
				addMonster(std::make_shared<Broundabout>(this, j, i));
			}
			else if (finalvec.at(count) == PIKEMAN) {
				addMonster(std::make_shared<Pikeman>(this, j, i));
			}
			else if (finalvec.at(count) == RANDOM_MONSTER) {
				monster = rollMonster(m_level, this, j, i);
				addMonster(monster);
			}

			else if (finalvec.at(count) == PIT) {
				addTrap(std::make_shared<Pit>(*this, j, i));
			}
			else if (finalvec.at(count) == FIREBAR) {
				addTrap(std::make_shared<Firebar>(*this, j, i));
			}
			else if (finalvec.at(count) == DOUBLE_FIREBAR) {
				addTrap(std::make_shared<DoubleFirebar>(*this, j, i));
			}
			else if (finalvec.at(count) == SPIKES) {
				addTrap(std::make_shared<Spikes>(*this, j, i));
			}
			else if (finalvec.at(count) == AUTOSPIKE_DEACTIVE) {
				addTrap(std::make_shared<SpikeTrap>(*this, j, i, 4));
			}
			else if (finalvec.at(count) == TRIGGERSPIKE_DEACTIVE) {
				addTrap(std::make_shared<TriggerSpike>(*this, j, i));
			}
			else if (finalvec.at(count) == PUDDLE) {
				addTrap(std::make_shared<Puddle>(*this, j, i));
			}
			else if (finalvec.at(count) == SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false));
			}
			else if (finalvec.at(count) == LEFT_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'l'));
			}
			else if (finalvec.at(count) == RIGHT_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'r'));
			}
			else if (finalvec.at(count) == UP_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'u'));
			}
			else if (finalvec.at(count) == DOWN_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'd'));
			}
			else if (finalvec.at(count) == TL_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '2'));
			}
			else if (finalvec.at(count) == TR_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '1'));
			}
			else if (finalvec.at(count) == BL_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '3'));
			}
			else if (finalvec.at(count) == BR_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '4'));
			}
			else if (finalvec.at(count) == LEFT_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'l'));
			}
			else if (finalvec.at(count) == RIGHT_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'r'));
			}
			else if (finalvec.at(count) == UP_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'u'));
			}
			else if (finalvec.at(count) == DOWN_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'd'));
			}
			else if (finalvec.at(count) == TURRET) {
				char move;
				switch (randInt(1, 4)) {
				case 1: move = 'l'; break;	// L
				case 2: move = 'r'; break;	// R
				case 3: move = 'u'; break;	// U
				case 4: move = 'd'; break;	// D
				}

				addTrap(std::make_shared<Turret>(*this, j, i, move));
			}
			else if (finalvec.at(count) == MOVING_BLOCK) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, '-', 5));
			}
			else if (finalvec.at(count) == MOVING_BLOCK_H) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, 'l', 4));
			}
			else if (finalvec.at(count) == MOVING_BLOCK_V) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, 'u', 4));
			}
			else if (finalvec.at(count) == CRUMBLE_PIT) {
				addTrap(std::make_shared<CrumblePit>(*this, j, i));
			}
			else if (finalvec.at(count) == CRUMBLE_LAVA) {
				addTrap(std::make_shared<CrumbleLava>(*this, j, i));
			}
			else if (finalvec.at(count) == LAVA) {
				addTrap(std::make_shared<Lava>(*this, j, i));
			}

			else if (finalvec.at(count) == GOLD) {
				addGold(j, i, 4 + randInt(1, 10));
			}
			else if (finalvec.at(count) == HEART_POD) {
				createItem(std::make_shared<HeartPod>(j, i));
			}
			else if (finalvec.at(count) == LIFEPOT) {
				createItem(std::make_shared<LifePotion>(j, i));
			}
			else if (finalvec.at(count) == BIG_LIFEPOT) {
				createItem(std::make_shared<BigLifePotion>(j, i));
			}
			else if (finalvec.at(count) == ARMOR) {
				createItem(std::make_shared<ArmorDrop>(j, i));
			}
			else if (finalvec.at(count) == STATPOT) {
				createItem(std::make_shared<StatPotion>(j, i));
			}
			else if (finalvec.at(count) == BOMB) {
				createItem(std::make_shared<Bomb>(j, i));			
			}
			else if (finalvec.at(count) == BROWN_CHEST) {
				createChest(std::make_shared<BrownChest>(this, j, i));
			}
			else if (finalvec.at(count) == SILVER_CHEST) {
				createChest(std::make_shared<SilverChest>(this, j, i));
			}
			else if (finalvec.at(count) == GOLDEN_CHEST) {
				createChest(std::make_shared<GoldenChest>(this, j, i));
			}
			else if (finalvec.at(count) == RANDOM_CHEST) {
				createChest(rollChest(this, j, i, Rarity::ALL, true));
			}
			else if (finalvec.at(count) == RANDOM_ITEM) {
				tileAt(currentPos).object = rollItem(this, j, i, Rarity::COMMON, false, true);
			}
			else if (finalvec.at(count) == RANDOM_WEAPON) {
				createItem(rollWeapon(this, j, i, Rarity::RARE, true));
			}
			else if (finalvec.at(count) == RANDOM_SHIELD) {
				createItem(rollShield(this, j, i, Rarity::COMMON, false));
			}

			count++;
		}
	}

}



/** ******* ****** ****** **********
*
*		SPRITE SETTING/MOVING
*
*/

// Helper
Coords Dungeon::transformSpriteToDungeonCoordinates(const cocos2d::Vec2 &pos) const {
	int x, y;
	x = std::ceil((pos.x + X_OFFSET) / SPACING_FACTOR);
	y = std::ceil(static_cast<float>(m_rows) - ((pos.y + Y_OFFSET) / SPACING_FACTOR));

	return Coords(x, y);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const Coords &inCoords) const {
	return transformDungeonToSpriteCoordinates(inCoords, cocos2d::Vec2::ZERO);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const Coords &inCoords, const cocos2d::Vec2 &offset) const {
	return transformDungeonToSpriteCoordinates(static_cast<cocos2d::Vec2>(inCoords), offset);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const cocos2d::Vec2 &inCoords) const {
	return transformDungeonToSpriteCoordinates(inCoords, cocos2d::Vec2::ZERO);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const cocos2d::Vec2 &inCoords, const cocos2d::Vec2 &offset) const {
	float x, y;
	x = inCoords.x * SPACING_FACTOR - X_OFFSET;
	y = (m_rows - inCoords.y)*SPACING_FACTOR - Y_OFFSET;

	x += offset.x;
	y += offset.y;

	return cocos2d::Vec2(x, y);
}

cocos2d::Vec2 Dungeon::dungeonToLightCoordinates(float x, float y) const {
	auto vs = cocos2d::Director::getInstance()->getVisibleSize();
	auto vsw = vs.width / 2;
	auto vsh = vs.height / 2;

	float fx = x + vsw;
	float fy = y + vsh;

	return cocos2d::Vec2(fx, fy);
}

// Create new sprites
SpriteType* Dungeon::createEffectSprite(int x, int y, int z, const std::string &image) {
	auto spriteFrame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	SpriteType* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	m_scene->addChild(sprite, z);

	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(Coords(x, y));

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	m_scene->setLightingOn(sprite);

	return sprite;
}

cocos2d::Sprite* Dungeon::createSprite(int x, int y, int z, const std::string &image) {
	return createSprite(Coords(x, y), cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const Coords &pos, int z, const std::string &image) {
	return createSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const Coords &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	return createSprite(static_cast<cocos2d::Vec2>(pos), offset, z, image);
}

cocos2d::Sprite* Dungeon::createSprite(float x, float y, int z, const std::string &image) {
	return createSprite(cocos2d::Vec2(x, y), cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const cocos2d::Vec2 &pos, int z, const std::string &image) {
	return createSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);

	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	return sprite;
}

void Dungeon::addSprite(int x, int y, int z, std::string image) {
	(void)createMiscSprite(Coords(x, y), z, image);
}
cocos2d::Sprite* Dungeon::createMiscSprite(const Coords &pos, int z, const std::string &image) {
	return createMiscSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createMiscSprite(const Coords &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	cocos2d::Sprite* sprite = createSprite(pos, offset, z, image);

	MiscSprite misc(pos, sprite);
	m_miscSprites.push_back(misc);

	return sprite;
}

cocos2d::Sprite* Dungeon::createWallSprite(int x, int y, int z, std::string type) {
	std::string image;
	cocos2d::Sprite* wall = nullptr;

	if (type == REG_WALL) {
		switch (randInt(1, 13)) {
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

		wall = createSprite(x, y, z, image);
	}
	else if (type == UNB_WALL) {
		switch (randInt(1, 15)) {
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

		wall = createSprite(x, y, z, image);
	}
	else if (type == FOUNTAIN) {
		wall = createSprite(x, y, z, "Fountain_Down_48x48.png");
	}

	return wall;
}

// Set sprites to position
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, int x, int y) {
	teleportSprite(sprite, Coords(x, y));
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const Coords &pos) {
	teleportSprite(sprite, pos, cocos2d::Vec2::ZERO);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const Coords &pos, const cocos2d::Vec2 &offset) {
	teleportSprite(sprite, static_cast<cocos2d::Vec2>(pos), offset);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, float x, float y) {
	teleportSprite(sprite, cocos2d::Vec2(x, y));
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &pos) {
	teleportSprite(sprite, pos, cocos2d::Vec2::ZERO);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);
	sprite->setPosition(finalPos);
}

// Move sprites
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time, bool front) {
	queueMoveSprite(sprite, Coords(cx, cy), time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const Coords &destination, float time) {
	queueMoveSprite(sprite, destination, cocos2d::Vec2::ZERO, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const Coords &destination, const cocos2d::Vec2 &offset, float time) {
	queueMoveSprite(sprite, static_cast<cocos2d::Vec2>(destination), offset, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, float cx, float cy, float time, bool front) {
	queueMoveSprite(sprite, cocos2d::Vec2(cx, cy), time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &destination, float time) {
	queueMoveSprite(sprite, destination, cocos2d::Vec2::ZERO, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &destination, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 pos = transformDungeonToSpriteCoordinates(destination, offset);

	cocos2d::MoveTo *move = cocos2d::MoveTo::create(time, pos);
	insertActionIntoSpawn(sprite, move);
}

cocos2d::MoveTo* Dungeon::getMoveAction(int cx, int cy, float time) {
	return getMoveAction(Coords(cx, cy), time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const Coords &pos, float time) {
	return getMoveAction(pos, cocos2d::Vec2::ZERO, time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const Coords &pos, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	return cocos2d::MoveTo::create(time, finalPos);
}
cocos2d::MoveTo* Dungeon::getMoveAction(float cx, float cy, float time) {
	return getMoveAction(cocos2d::Vec2(cx, cy), time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const cocos2d::Vec2 &pos, float time) {
	return getMoveAction(pos, cocos2d::Vec2::ZERO, time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	return cocos2d::MoveTo::create(time, finalPos);
}

// Remove sprites
void Dungeon::queueRemoveSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	insertActionIntoSpawn(sprite, remove);
}
void Dungeon::queueRemoveSpriteWithMultipleActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	for (auto &it : actions)
		insertActionIntoSpawn(sprite, it);

	insertActionIntoSpawn(sprite, cocos2d::RemoveSelf::create());
}
void Dungeon::removeSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	sprite->runAction(remove);
}

// Custom actions
void Dungeon::queueCustomAction(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action) {
	insertActionIntoSpawn(sprite, action);
}
void Dungeon::queueCustomActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	for (auto &it : actions)
		insertActionIntoSpawn(sprite, it);
}
void Dungeon::queueCustomSpawnActions(cocos2d::Sprite* sprite, const cocos2d::Vector<cocos2d::FiniteTimeAction*> &actions) {
	auto spawn = cocos2d::Spawn::create(actions);
	insertActionIntoSpawn(sprite, spawn);
}

// Update sprite coloring (lighting)
void Dungeon::updateLighting() {

	if (m_level == FIRST_BOSS)
		return;

	//cocos2d::Vec2 vec = dungeonToLightCoordinates(player[0]->getPosX(), player[0]->getPosY());
	//m_scene->movePlayerLightTo(vec.x, vec.y);

	int cols = m_cols;

	int px = player[0]->getPosX();
	int py = player[0]->getPosY();
	int p_cutoff = player[0]->getVision();

	int sx, sy, dist, color;

	// Extra sprites for decoration
	for (size_t i = 0; i < m_miscSprites.size(); ++i) {
		if (!m_miscSprites[i].sprite)
			continue;

		sx = m_miscSprites[i].pos.x;
		sy = m_miscSprites[i].pos.y;

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);
		color = 0;

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
	}

	// npc sprites
	for (unsigned int i = 0; i < m_npcs.size(); i++) {
		sx = m_npcs.at(i)->getPosX();
		sy = m_npcs.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = 0;

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));		
	}

	// monster sprites
	int mx, my;
	for (size_t i = 0; i < m_monsters.size(); ++i) {
		mx = m_monsters.at(i)->getPosX();
		my = m_monsters.at(i)->getPosY();
		dist = abs(mx - px) + abs(my - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = 0;

		// If monster is a source of light
		if (m_monsters.at(i)->emitsLight()) {
			m_monsters.at(i)->addLightEmitters(lightEmitters);
			continue;
		}

		if (player[0]->hasMonsterIllumination()) {
			lightEmitters.push_back(Coords(mx, my));
			continue;
		}

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

		// If monster uses extra sprites, update their lighting too
		if (m_monsters.at(i)->hasExtraSprites())
			m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
	}

	// trap sprites
	for (unsigned int i = 0; i < m_traps.size(); i++) {

		if (m_traps.at(i)->getSprite() == nullptr)
			continue;

		sx = m_traps.at(i)->getPosX();
		sy = m_traps.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		// if player has trap illumination
		if (player[0]->hasTrapIllumination() && m_traps.at(i)->canBeIlluminated())
			lightEmitters.push_back(Coords(sx, sy));

		// if trap is lava or other light-emitting source, emit light from their position
		if (m_traps.at(i)->emitsLight()) {
			lightEmitters.push_back(Coords(m_traps.at(i)->getPosX(), m_traps.at(i)->getPosY()));
			continue;
		}

		if (m_traps.at(i)->getName() == LAVA) {
			int cutoff = p_cutoff + 6;
			formula = (cutoff - dist) / (float)(cutoff);
			color = (255 * formula);

			if (dist >= cutoff)
				color = 19; // color value calculated by taking (1/cutoff) * 255

			m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
		else {
			color = 0;
			if (dist <= p_cutoff)
				color = (255 * formula);

			if (m_traps.at(i)->getSprite() != nullptr)
				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

			// if trap uses extra sprites, update their lighting too
			if (m_traps.at(i)->hasExtraSprites())
				m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));			
		}
	}

	// floor sprites
	for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
		for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
			if (withinBounds(i, j)) {
				sx = i;
				sy = j;

				dist = abs(sx - px) + abs(sy - py);
				float formula = (p_cutoff - dist) / (float)p_cutoff;
				color = 0;

				if (dist <= p_cutoff)
					color = (255 * formula);

				tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	// Walls
	for (unsigned int i = 0; i < m_walls.size(); i++) {
		sx = m_walls.at(i)->getPosX();
		sy = m_walls.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));	
	}

	// item sprites
	for (unsigned int i = 0; i < m_items.size(); i++) {
		sx = m_items.at(i)->getPosX();
		sy = m_items.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (player[0]->hasItemIllumination())
			lightEmitters.push_back(Coords(sx, sy));

		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
	}

	// money sprites
	for (unsigned int i = 0; i < m_money.size(); i++) {
		sx = m_money.at(i)->getPosX();
		sy = m_money.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (player[0]->hasGoldIllumination())
			lightEmitters.push_back(Coords(sx, sy));

		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
	}


	updateSecondaryLighting();
}
void Dungeon::updateSecondaryLighting() {
	int cols = m_cols;

	int px, py;
	int p_cutoff = 2;

	int sx, sy, dist, color;
	cocos2d::Color3B c3b;

	for (unsigned int n = 0; n < lightEmitters.size(); ++n) {

		px = lightEmitters.at(n).x;
		py = lightEmitters.at(n).y;

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				
				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites()) {
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));
				}
			}
			
		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) {
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// item sprites
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	// Check other light sources
	for (unsigned int pos = 0; pos < m_lightSources.size(); ++pos) {
		px = m_lightSources.at(pos).pos.x;
		py = m_lightSources.at(pos).pos.y;
		p_cutoff = static_cast<int>(m_lightSources.at(pos).radius);

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));

				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites())
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));			
			}

		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) { // boundary check
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// Items
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	for (unsigned int pos = 0; pos < m_playerClones.size(); ++pos) {

		px = m_playerClones[pos]->getPosX();
		py = m_playerClones[pos]->getPosY();
		p_cutoff = m_playerClones[pos]->getVision();

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));

				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites())
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));				
			}

		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) {					
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// item sprites
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	lightEmitters.clear();
}

int Dungeon::findLightSource(int x, int y, std::string name) {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_lightSources.size()); ++i) {
		if (m_lightSources.at(i).pos == target && m_lightSources.at(i).name == name) {
			return i;
		}
	}

	return -1;
}
void Dungeon::addLightSource(int x, int y, double strength, std::string name) {
	Coords coords(x, y);
	LightSource lightSource(coords, strength, name);

	m_lightSources.push_back(lightSource);
}
void Dungeon::removeLightSource(int x, int y, std::string name) {
	int pos = findLightSource(x, y, name);

	if (pos == -1)
		return;

	m_lightSources.erase(m_lightSources.begin() + pos);
}

void Dungeon::zoomInBy(float factor) {
	// Since the ratio of sprite scale to spacing factor is 1/44, we increment by this same ratio
	// NOTE: This ratio is subject to change when art is finalized.

	GLOBAL_SPRITE_SCALE += factor;
	SPACING_FACTOR += factor * 44.0f;

	updateSpriteScaling();
}
void Dungeon::zoomOutBy(float factor) {
	if (GLOBAL_SPRITE_SCALE - factor <= 0.0f)
		return;
	
	GLOBAL_SPRITE_SCALE -= factor;
	SPACING_FACTOR -= factor * 44.0f;

	updateSpriteScaling();
}
void Dungeon::updateSpriteScaling() {
	int cols = m_cols;
	int rows = m_rows;

	cocos2d::ScaleTo* scale = cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;

	actions.pushBack(scale);
	actions.pushBack(getMoveAction(player[0]->getPos(), getHalfTileYOffset()));
	queueCustomSpawnActions(player[0]->getSprite(), actions);

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_playerClones[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_playerClones[i]->getSprite(), actions);
	}

	// Extra sprites for decoration
	for (size_t i = 0; i < m_miscSprites.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_miscSprites[i].pos));
		queueCustomSpawnActions(m_miscSprites[i].sprite, actions);

		//m_miscSprites[i].sprite->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_miscSprites[i].sprite, cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_miscSprites[i].sprite, m_miscSprites[i].pos.x, m_miscSprites[i].pos.y);
	}

	// npc sprites
	for (size_t i = 0; i < m_npcs.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_npcs[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_npcs[i]->getSprite(), actions);

		//m_npcs[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_npcs[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_npcs[i]->getSprite(), m_npcs[i]->getPosX(), m_npcs[i]->getPosY());
	}

	// monster sprites
	for (size_t i = 0; i < m_monsters.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_monsters[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_monsters[i]->getSprite(), actions);

		//m_monsters[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_monsters[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_monsters[i]->getSprite(), m_monsters[i]->getPosX(), m_monsters[i]->getPosY());
	}

	// trap sprites
	for (size_t i = 0; i < m_traps.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_traps[i]->getPos()));
		queueCustomSpawnActions(m_traps[i]->getSprite(), actions);

		//m_traps[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_traps[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_traps[i]->getSprite(), m_traps[i]->getPosX(), m_traps[i]->getPosY());
	}

	// floor sprites
	for (int i = 0; i < cols; ++i) {
		for (int j = 0; j < rows; ++j) {
			actions.clear();
			actions.pushBack(scale->clone());
			actions.pushBack(getMoveAction(i, j));
			queueCustomSpawnActions(tileAt(Coords(i, j)).floor, actions);
		}
	}

	// Walls
	for (size_t i = 0; i < m_walls.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_walls[i]->getPos()));
		queueCustomSpawnActions(m_walls[i]->getSprite(), actions);

		//m_walls[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_walls[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_walls[i]->getSprite(), m_walls[i]->getPosX(), m_walls[i]->getPosY());
	}

	// item sprites
	for (size_t i = 0; i < m_items.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_items[i]->getPos()));
		queueCustomSpawnActions(m_items[i]->getSprite(), actions);

		//m_items[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_items[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_items[i]->getSprite(), m_items[i]->getPosX(), m_items[i]->getPosY());
	}

	// money sprites
	for (size_t i = 0; i < m_money.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_money[i]->getPos()));
		queueCustomSpawnActions(m_money[i]->getSprite(), actions);

		//m_money[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_money[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_money[i]->getSprite(), m_money[i]->getPosX(), m_money[i]->getPosY());
	}

	runSpawn();
}

// Run sprite actions
int Dungeon::spriteFound(cocos2d::Sprite* sprite) {
	for (int i = 0; i < static_cast<int>(m_seq.size()); ++i)
		if (m_seq[i].first == sprite)
			return i;
	
	return -1;
}
void Dungeon::insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front) {
	int i = spriteFound(sprite);

	if (i == -1) {
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(action);
		if (!front)
			m_seq.push_back(std::make_pair(sprite, v));	
		else
			m_seq.insert(m_seq.begin(), std::make_pair(sprite, v));
		
	}
	else {
		m_seq.at(i).second.pushBack(action);
	}
}
void Dungeon::runSpawn() {
	if (!m_seq.empty()) {
		for (auto &it : m_seq) {
			if (it.first) {
				cocos2d::Sequence* seq = cocos2d::Sequence::create(it.second);

				auto singleAction = it.first->runAction(seq);
				singleAction->setTag(1);
			}
		}
		m_seq.clear();
	}
}

cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	return runAnimationForever(frames, frameInterval, Coords(x, y), cocos2d::Vec2::ZERO, z);
}
cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, int z) {
	return runAnimationForever(frames, frameInterval, pos, cocos2d::Vec2::ZERO, z);
}
cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, const cocos2d::Vec2 &offset, int z) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, pos, offset);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	sprite->runAction(cocos2d::RepeatForever::create(cocos2d::Animate::create(animation)));
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	return sprite;
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	runSingleAnimation(frames, frameInterval, Coords(x, y), cocos2d::Vec2::ZERO, z);
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, int z) {
	runSingleAnimation(frames, frameInterval, pos, cocos2d::Vec2::ZERO, z);
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, const cocos2d::Vec2 &offset, int z) {
	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, pos, offset);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto remove = cocos2d::RemoveSelf::create();
	auto sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::Animate::create(animation), remove);
	auto action = sprite->runAction(sequence);
	action->setTag(5);
}
void Dungeon::runAnimationWithCallback(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z,
	std::function<void(Dungeon&, int, int, int, std::string)> cb, std::string image) {

	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, x, y);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto remove = cocos2d::RemoveSelf::create();
	auto sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::Animate::create(animation), remove);
	auto action = sprite->runAction(sequence);
	action->setTag(5);
	
	// Adds the last sprite so that its lighting can be updated
	cb(*this, x, y, 0, image);
}



// ===========================================
//		   :::: TUTORIAL LEVEL ::::
// ===========================================
TutorialFloor::TutorialFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, TUTORIAL, 26, 41) {
	int rows = getRows();
	int cols = getCols();

	// Get the level layout
	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPosX(j);
				player[0]->setPosY(i);
				player[0]->setMoney(200);
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				player[0]->setVision(8);

				break;
			}
			case '#': {				
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
					// Water tile
			case 'w': {
				addTrap(std::make_shared<Water>(*this, j, i));				

				break;
			}
					  // Dirt tile
			case 'D': {
				if (randReal(1, 100) > 10)
					addTrap(std::make_shared<Sand>(*this, j, i));
				else
					addTrap(std::make_shared<Quicksand>(*this, j, i));
				
				//createItem(std::make_shared<RPGInABottle>(j, i));
				break;
			}
					// Breakable object
			case 'B': { 
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					// Arrow Sign
			case 'A': { 
				//std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				//m_monsters.push_back(object);

				//m_npcs.push_back(std::make_shared<Enchanter>(this, j, i));

				addMonster(Wriggler::create(this, Coords(j, i)));

				//createChest(std::make_shared<HauntedChest>(this, j, i));

				//addMonster(std::make_shared<ShieldMaster>(this, j, i));

				//addTrap(std::make_shared<LightAbsorber>(*this, j, i));
				//addTrap(std::make_shared<SquareMovingTile>(*this, j, i, 'u', 4, false));

				break;
			}
					// Regular Sign
			case 'R': { 
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					// Outside Man 1
			case 'M': {			
				/*std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC1>(j, i);
				m_npcs.push_back(npc);*/

				//m_npcs.push_back(std::make_shared<InjuredExplorer>(this, j, i));

				//addMonster(std::make_shared<IncendiaryInfuser>(this, j, i));

				//addTrap(std::make_shared<SquareMovingBlock>(*this, j, i, 'l', 6, true));
				addTrap(std::make_shared<DNASplitter>(*this, j, i));

				break;
			}
					  // Outside Man 2
			case 'Q': {
				std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC2>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
					  // Outside Woman 1
			case 'F': {
				std::shared_ptr<NPC> npc = std::make_shared<Trader>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
			}
			
		}
	}

	/*int x, y;
	for (int i = 0; i < 30; i++) {
		x = randInt(1, cols - 2);
		y = randInt(1, rows - 2);

		while (wall(x, y) || enemy(x, y) || hero(x, y)) {
			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);
		}

		addMonster(rollMonster(SEVENTH_FLOOR, this, x, y));
	}*/
}

std::vector<char> TutorialFloor::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char r = 'r'; // Rock
	char k = 'k'; // 
	char d = 'd'; //
	// 'w' : Water
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'A' : Arrow Sign
	// 'R' : Regular Sign
	// 'M' : Outside Man 1
	// 'Q' : Outside Man 2
	// 'F' : Outside Woman 1
	

	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*3*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*4*/	'#', '#', '#', '#', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#', '#', '#',
		/*5*/	'#', '#', '#', '#', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', 'Q', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*6*/	'#', '#', '#', 'w', 'w', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#',
		/*7*/	'#', '#', '#', 'w', 'w', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*8*/	'#', '#', '#', 'D', 'w', 'w', 'w', 'D', 'D', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', '#', 'B', 'A', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*9*/	'#', '#', '#', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', 'M', ' ', ' ', ' ', ' ', ' ', '#', '#', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  s , '#', '#', '#', '#', '#',
		/*10*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'M', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*11*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'R', '#', 'B', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*12*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'F', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*13*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*14*/	'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*15*/	'#', '#', '#', '#', ' ', ' ', 'M', ' ', 'A', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', ' ', ' ', '#', 'B', 'A', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#',
		/*16*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', 'D', ' ', '#', ' ', 'A', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#',
		/*17*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', 'B', 'B', '#', '#', '#', '#',
		/*18*/	'#', '#', '#', '#', 'M', '@', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'B', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#',
		/*19*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
		/*20*/	'#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*21*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*22*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*23*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*24*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
	};

	return level;
}


// ===========================================
//			  :::: WORLD HUB ::::
// ===========================================
WorldHub::WorldHub(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, WORLD_HUB, 52, 41) {
	int rows = getRows();
	int cols = getCols();

	// Get the level layout
	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;

				player[0]->setPosX(j);
				player[0]->setPosY(i);
				//player[0]->setMoney(1000);
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));

				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
						 // Water tile
			case 'w': {
				std::shared_ptr<Traps> water = std::make_shared<Water>(*this, j, i);
				addTrap(water);
				
				break;
			}
					  // Dirt tile
			case 'D': {
				break;
			}
					  // Breakable object
			case 'B': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					  // Arrow Sign
			case 'A': {
				std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Regular Sign
			case 'R': {
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Exit to Main Menu
			case 'E': {
				tileAt(currentPos).menu = true;

				break;
			}
			}

		}
	}
}

void WorldHub::specialActions() {
	Coords pos = getPlayer()->getPos();

	// If player stepped on the exit to menu tile, then exit to main menu
	if (tileAt(pos).menu) {
		m_scene->returnToMainMenu();
		m_return = true;
	}
}

std::vector<char> WorldHub::generate() {
	char dv = DOOR_V;
	char S = STAIRS;
	// 'w' : Water
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'A' : Arrow Sign
	// 'R' : Regular Sign
	// 'E' : Exit to Main Menu


	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*3*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*4*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*5*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*6*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*7*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*8*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*9*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', '#',  S , '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*10*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*11*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*12*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*13*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*14*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*15*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*16*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*17*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*18*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*19*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*20*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*21*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*22*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*23*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*24*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*26*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*27*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'B', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*28*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*29*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*30*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'R', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*31*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*32*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*33*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*34*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*35*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*36*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*37*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*38*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*39*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*40*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*41*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*42*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '@', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*43*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*44*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*45*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*46*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'E', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*47*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*48*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*49*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*50*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*51*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
	};

	return level;
}



// ===========================================
//				:::: SHOP ::::
// ===========================================
Shop::Shop(LevelScene* scene, std::shared_ptr<Player> p, int level) : Dungeon(scene, p, level, MAXROWS_SHOP1, MAXCOLS_SHOP1) {

	int rows = getRows();
	int cols = getCols();

	// Add light source in the middle of the shop
	addLightSource(13, 5, 9, "Shop Light");

	// Price adjustment multiplier
	float baseMultiplier = 1.0f;
	switch (level) {
	case FIRST_SHOP: baseMultiplier = 1.0f; break;
	case SECOND_SHOP: baseMultiplier = 2.2f; break;
	case THIRD_SHOP: baseMultiplier = 3.6f; break;
	case FOURTH_SHOP: baseMultiplier = 5.0f; break;
	default: baseMultiplier = 1.0f; break;
	}
	
	m_priceMultiplier = baseMultiplier * (p->getFavor() <= -8 ? 1.15f : 1.0f) * (p->hasCheapShops() ? 0.5f : 1.0f);

	// get the shop layout
	std::vector<char> v = generate();

	// begin mapping shop
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPosX(j);
				player[0]->setPosY(i);
				player[0]->setMoney(1000);
				//player[0]->storeWeapon(std::make_shared<BoneAxe>(0, 0));
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
				// purchase spot
			case 'p': {
				tileAt(currentPos).shop_action = PURCHASE;
				break;
			}
				// item price
			case '$': {

				break;
			}
				// shop counter
			case 'c': {
				addSprite(j, i, i + Z_ITEM - 1, "Countertop_48x48.png");

				tileAt(currentPos).wall = true;
				tileAt(currentPos).boundary = true;
				break;
			}
				// shopkeeper
			case 'k': {
				std::shared_ptr<NPC> shopkeeper = std::make_shared<Shopkeeper>(this, j, i + 1);
				m_npcs.push_back(shopkeeper);

				break;
			}
				// breakable object
			case 'd': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}
				
				m_monsters.push_back(object);

				break;
			}
				// secret thing
			case 'b': {
				addSprite(j, i, i + Z_EXIT, "Stairs_48x48.png");
				//cocos2d::Sprite* object = createSprite(j, i, 1, "Stairs_48x48.png");
				//misc_sprites.push_back(object);

				break;
			}
					  // 1, 2, 3 : hp, shield repair, utility
			case '1': {
				switch (randInt(0, 4)) {
				case 0: tileAt(currentPos).object = std::make_shared<LifePotion>(j, i); tileAt(currentPos).price = 30 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<BigLifePotion>(j, i); tileAt(currentPos).price = 45 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<SoulPotion>(j, i); tileAt(currentPos).price = 37 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<HalfLifePotion>(j, i); tileAt(currentPos).price = 42 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<SteadyLifePotion>(j, i); tileAt(currentPos).price = 36 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;

				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
			case '2': {
				switch (randInt(0, 5)) {
				case 0: tileAt(currentPos).object = std::make_shared<SmokeBomb>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<Bomb>(j, i);  tileAt(currentPos).price = 25 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<StatPotion>(j, i); tileAt(currentPos).price = 23 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Matches>(j, i); tileAt(currentPos).price = 16 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<BearTrap>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<Firecrackers>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
			case '3': {
				switch (randInt(0, 6)) {
				case 0: tileAt(currentPos).object = std::make_shared<LifePotion>(j, i); tileAt(currentPos).price = 30 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<BinaryLifePotion>(j, i); tileAt(currentPos).price = 45 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<RottenApple>(j, i); tileAt(currentPos).price = 25 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Bomb>(j, i); tileAt(currentPos).price = 21 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<PoisonBomb>(j, i); tileAt(currentPos).price = 24 * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<RottenMeat>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<ArmorDrop>(j, i); tileAt(currentPos).price = 55 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
					  // 4, 5 : random items
			case '4': {
				switch (randInt(0, 8)) {
				case 0: tileAt(currentPos).object = std::make_shared<BoStaff>(j, i); tileAt(currentPos).price = (80 + randInt(0, 5)) * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<Pike>(j, i); tileAt(currentPos).price = (80 + randInt(0, 5)) * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<Boomerang>(j, i); tileAt(currentPos).price = (55 + randInt(0, 5)) * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Katana>(j, i); tileAt(currentPos).price = (85 + randInt(0, 5)) * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<Estoc>(j, i); tileAt(currentPos).price = (120 + randInt(0, 10)) * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<Hammer>(j, i); tileAt(currentPos).price = 58 * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<Nunchuks>(j, i); tileAt(currentPos).price = 65 * m_priceMultiplier; break;
				case 7: tileAt(currentPos).object = std::make_shared<Zweihander>(j, i); tileAt(currentPos).price = (70 + randInt(0, 8)) * m_priceMultiplier; break;
				case 8: tileAt(currentPos).object = std::make_shared<Jian>(j, i); tileAt(currentPos).price = (70 + randInt(0, 8)) * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(j, i, i + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
			case '5': {
				switch (randInt(0, 10)) {
				case 0: tileAt(currentPos).object = std::make_shared<BigLifePotion>(j, i); tileAt(currentPos).price = (30) * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<FireBlastSpell>(j, i); tileAt(currentPos).price = (39 + randInt(0, 3)) * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<FireExplosionSpell>(j, i); tileAt(currentPos).price = (40 + randInt(0, 3)) * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<FireCascadeSpell>(j, i); tileAt(currentPos).price = (45 + randInt(0, 9)) * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<Claw>(j, i); tileAt(currentPos).price = (85 + randInt(0, 20)) * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<FreezeSpell>(j, i); tileAt(currentPos).price = (38 + randInt(0, 5)) * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<HailStormSpell>(j, i); tileAt(currentPos).price = (48 + randInt(0, 5)) * m_priceMultiplier; break;
				case 7: tileAt(currentPos).object = std::make_shared<EtherealSpell>(j, i); tileAt(currentPos).price = (50 + randInt(0, 6)) * m_priceMultiplier; break;
				case 8: tileAt(currentPos).object = std::make_shared<RockSummonSpell>(j, i); tileAt(currentPos).price = (43 + randInt(0, 8)) * m_priceMultiplier; break;
				case 9: tileAt(currentPos).object = std::make_shared<ShockwaveSpell>(j, i); tileAt(currentPos).price = (51 + randInt(0, 7)) * m_priceMultiplier; break;
				case 10: tileAt(currentPos).object = std::make_shared<WindBlastSpell>(j, i); tileAt(currentPos).price = (32 + randInt(0, 8)) * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
					  // 6 : random chest
			case '6': {
				double n = randReal(1, 100) + p->getLuck();
				
				// 50% chance
				if (n <= 50) { tileAt(currentPos).object = std::make_shared<BrownChest>(this, j, i); tileAt(currentPos).price = 25 * m_priceMultiplier; }
				// 10% chance
				else if (n > 50 && n <= 60) { tileAt(currentPos).object = std::make_shared<LifeChest>(this, j, i); tileAt(currentPos).price = 50 * m_priceMultiplier; }
				// 25% chance
				else if (n > 60 && n <= 95) { tileAt(currentPos).object = std::make_shared<SilverChest>(this, j, i); tileAt(currentPos).price = 55 * m_priceMultiplier; }
				// 5% chance
				else if (n > 95) { tileAt(currentPos).object = std::make_shared<GoldenChest>(this, j, i); tileAt(currentPos).price = 120 * m_priceMultiplier; }

				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
			}
		}
	}
}

void Shop::constructShopHUD(int x, int y) {
	// if there's a price already floating, remove before showing the new one
	if (m_scene->getPriceLabel() != nullptr)
		m_scene->deconstructShopHUD();

	m_scene->showShopHUD(x, y);
}
void Shop::deconstructShopHUD() {
	m_scene->deconstructShopHUD();
}

void Shop::purchaseItem(int x, int y) {
	std::shared_ptr<Objects> object = itemObject(x, y - 1);
	int playerGold = player[0]->getMoney();
	int price = tileAt(Coords(x, y - 1)).price;

	int items = player[0]->itemCount();
	int maxItems = player[0]->maxItemCount();

	// if player has enough money, let them buy it
	if (playerGold >= price && (!object->isUsable() || (object->isUsable() && items < maxItems))) {
		playSound("Purchase_Item1.mp3");

		player[0]->setMoney(playerGold - price);
		tileAt(Coords(x, y)).shop_action = ""; // removes purchase ability
		collectItem(x, y - 1);

		// remove HUD after purchasing
		m_scene->deconstructShopHUD();
	}
	else {
		playSound("Insufficient_Funds.mp3");
	}
}
void Shop::collectItem(int x, int y) {
	// NOTE: If coming from purchase item, y is equal to (py - 1) where py is the player's y position.
	//       If collectItem is called directly, then there is no y shift.

	bool itemAdded = false;
	
	std::string shop_action = tileAt(Coords(x, y)).shop_action;
	std::shared_ptr<Objects> object = (shop_action == CHEST_PURCHASED ? itemObject(x, y - 1) : itemObject(x, y));

	// if it's the stairs, then advance level and return
	if (exit(x, y)) {
		m_exit->interact();
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		playSound("Player_Movement.mp3");

		return;
	}

	if (object->isAutoUsable()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->useItem(*this, *getPlayerAt(x, y));
		playSound(drop->getSoundName());
		drop.reset();

		if (shop_action == CHEST_PURCHASED)
			removeItem(x, y - 1);
		else
			removeItem(x, y);
	}
	else if (object->isUsable()) {

		// For Spellcaster
		if (object->isSpell() && player[0]->getName() == SPELLCASTER) {
			player[0]->equipActiveItem(object);

			if (shop_action == CHEST_PURCHASED)
				removeItem(x, y - 1);			
			else
				removeItem(x, y);
			
			return;
		}

		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		player[0]->addItem(drop, itemAdded);
		drop.reset();
		
		if (!itemAdded) {
			playSound("Player_Movement.mp3");
			return;
		}
	}
	else if (object->isPassive()) {
		std::shared_ptr<Passive> passive = std::dynamic_pointer_cast<Passive>(object);
		player[0]->equipPassive(passive);
		passive.reset();

		itemAdded = true;
	}
	else if (object->isWeapon()) {
		std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
		player[0]->storeWeapon(weapon);
		playSound(weapon->getSoundName());
		
		weapon.reset();
	}
	else if (object->isChest()) {
		std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(object);
		chest->attemptOpen(*this);
		tileAt(Coords(x, y + 1)).shop_action = CHEST_PURCHASED;

		chest.reset();	
	}
	else if (object->isShield()) {
		if (player[0]->canUseShield())
			player[0]->equipActiveItem(object);		
	}

	if (itemAdded) {
		if (shop_action == CHEST_PURCHASED)
			removeItem(x, y - 1);		
		else
			removeItem(x, y);
	}
}

std::vector<char> Shop::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char c = 'c'; // shop countertop, where items will sit
	char k = 'k'; // shopkeeper
	char d = 'd'; // random breakable object
	// 1, 2, 3 : hp, shield repair, bombs
	// 4, 5 : random items
	// 6 : random chest
	// 'p' : stand here to purchase
	// '$' : insert price here

	std::vector<char> shop = {
	//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26
/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#',
/*3*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*4*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ', 'b', ' ', '#', '#',
/*5*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*6*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
/*7*/	'#', '#', '#', '#', '#', '#', '#', ' ', '$', '$', '$', ' ',  k , '$', ' ', '$', ' ', '$', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
/*8*/	'#', '#', '#', '#', '#', '#', '#',  c , '1', '2', '3',  c ,  c , '4',  c , '5',  c , '6',  c ,  c , '#', '#', '#', '#', '#', '#', '#',
/*9*/	'#',  d ,  d , ' ', ' ',  d , '#', ' ', 'p', 'p', 'p', ' ', ' ', 'p', ' ', 'p', ' ', 'p', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*10*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#',
/*11*/	'#', '@', ' ', ' ', ' ', ' ',  dv, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ',   s, ' ', ' ', '#',
/*12*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*13*/	'#',  d , ' ', ' ',  d ,  d , '#',  d ,  d , ' ', ' ',  d , ' ', ' ',  d , ' ', ' ', ' ',  d ,  d , '#', ' ', ' ', ' ',  d ,  d , '#',
/*14*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
		};

	return shop;
}


// ===========================================
//				:::: SHRINE ::::
// ===========================================
Shrine::Shrine(LevelScene* scene, std::shared_ptr<Player> p, int level) : Dungeon(scene, p, level, MAXROWS_SHRINE1, MAXCOLS_SHRINE1) {
	int rows = getRows();
	int cols = getCols();

	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPos(currentPos);
				//player[0]->equipPassive(*this, std::make_shared<IronCleats>(0, 0));
				//player[0]->equipRelic(*this, std::make_shared<BrightStar>());
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				//player[0]->setVision(15);

				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
					  // Regular Wall
			case 'W': {
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				m_exit = std::make_shared<Stairs>(*this, j, i);
				addTrap(m_exit);

				break;
			}
						 // Water tile
			case 'w': {
				addTrap(std::make_shared<Water>(*this, j, i));
				
				break;
			}
					  // Dirt tile
			case 'D': {
				break;
			}
					  // Breakable object
			case 'B': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					  // Arrow Sign
			case 'A': {
				std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Regular Sign
			case 'R': {
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Outside Man 1
			case 'M': {
				std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC1>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
					  // Lit Brazier
			case 't': {
				std::shared_ptr<Brazier> trap = std::make_shared<Brazier>(*this, j, i);
				litBraziers.push_back(trap);
				m_traps.emplace_back(trap);
				trap.reset();
				tileAt(currentPos).trap = true;
				tileAt(currentPos).wall = true;
				
				break;
			}
					  // Unlit Brazier
			case 'u': {
				std::shared_ptr<Brazier> trap = std::make_shared<Brazier>(*this, j, i);
				m_traps.emplace_back(trap);
				trap.reset();
				tileAt(currentPos).trap = true;
				tileAt(currentPos).wall = true;
				break;
			}
					  // Relic choice
			case 'T': {
				tileAt(currentPos).shrine_action = RELIC_CHOICE;
				break;
			}
					  // Recover Health
			case 'H': {
				tileAt(currentPos).shrine_action = HEALTH_CHOICE;
				break;
			}
					  // Chest choice
			case 'C': {
				tileAt(currentPos).shrine_action = CHEST_CHOICE;
				break;
			}
			default: break;
			}
		}
	}
}
Shrine::~Shrine() {
	for (auto &it : litBraziers)
		it.reset();
}

void Shrine::specialActions() {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();
	Coords playerCoords(player[0]->getPos());

	// Ignite the two Braziers if player has walked close enough
	if (!braziersLit && y <= 13) {
		playSound("Fireblast_Spell2.mp3");

		for (size_t i = 0; i < litBraziers.size(); ++i)
			litBraziers.at(i)->ignite();
		
		braziersLit = true;
	}

	// Check if player chose chests and if they opened any yet
	if (choseChests) {
		bool chestOpened = false;
		for (size_t i = 0; i < chestCoordinates.size(); ++i) {
			int n = chestCoordinates.at(i).x;
			int m = chestCoordinates.at(i).y;

			if (itemObject(n, m) && !itemObject(n, m)->isChest() || !itemObject(n, m)) {
				playSound("Guardians_Defeated.mp3");

				chestOpened = true;
				break;
			}
		}

		// Remove the other chests
		if (chestOpened) {
			for (unsigned int i = 0; i < chestCoordinates.size(); i++) {
				int n = chestCoordinates.at(i).x;
				int m = chestCoordinates.at(i).y;

				// Remove the chest
				if (itemObject(n, m) && itemObject(n, m)->isChest()) {
					removeItem(n, m);

					tileAt(Coords(n, m)).wall = false;
				}
			}
			choseChests = false;
		}
	}

	// Check if player walked off the spot where they can access the shrine
	if (tileAt(playerCoords).shrine_action == "") {
		if (makingChoice)
			makingChoice = false;
	}

	if (tileAt(playerCoords).shrine_action != "" && !makingChoice) {
		m_scene->callShrineChoice(tileAt(playerCoords).shrine_action);
		makingChoice = true;
	}
}

void Shrine::useChoice(int index) {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();
	Coords playerCoords(player[0]->getPos());

	std::string shrine = tileAt(playerCoords).shrine_action;

	if (shrine == HEALTH_CHOICE) {
		switch (index) {
			// Wealth
		case 0:
			if (player[0]->getMoney() >= 200) {
				playSound("Devils_Gift.mp3");
				player[0]->setMoney(player[0]->getMoney() - 200);

				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Item (Passive or all items)
		case 1:
			if (player[0]->hasItems()) {
				playSound("Devils_Gift.mp3");

				if (player[0]->hasPassives())
					player[0]->removePassive(randInt(0, (int)player[0]->passiveCount() - 1));
				else
					player[0]->removeItems();

				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Weapon
		case 2:
			if (sacrificeWeapon()) {
				playSound("Devils_Gift.mp3");
				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Soul (Max HP)
		case 3: {
			// Lose 25% of max hp capacity
			playSound("Devils_Gift.mp3");
			playGotHit();

			int newMaxHP = player[0]->getMaxHP() * 0.75;
			player[0]->setMaxHP(newMaxHP);

			if (player[0]->getHP() > player[0]->getMaxHP())
				player[0]->setMaxHP(player[0]->getMaxHP());

			player[0]->decreaseFavorBy(2);
			break;
		}
		default: index = -1;
		}

		// Restore health to max if they chose to
		if (index != -1)
			player[0]->setHP(player[0]->getMaxHP());
		
	}
	else if (shrine == RELIC_CHOICE) {

		// Choices when player already has a relic
		if (player[0]->hasRelic()) {
			switch (index) {
				// Upgrade
			case 0: {
				playSound("Devils_Gift.mp3");
				upgradeRelic();
				break;
			}
				// Switch
			case 1: {
				playSound("Devils_Gift.mp3");
				swapRelic();
				break;
			}
			default: index = -1;
			}
		}
		// One choice if player has no trinket
		else {
			switch (index) {
				// Acquire
			case 0: {
				playSound("Devils_Gift.mp3");
				giveRelic();
				break;
			}
			default: index = -1;
			}
		}
	}
	else if (shrine == CHEST_CHOICE) {
		switch (index) {
		case 0: {
			playSound("Devils_Gift.mp3");

			spawnChests();
			choseChests = true;
			break;
		}
		default: index = -1;
		}
	}

	// If player accepted an offer
	if (index != -1) {

		// Light the brazier to indicate its use
		if (trap(x, y - 1)) {
			std::vector<int> indexes = findTraps(x, y - 1);
			for (size_t i = 0; i < indexes.size(); ++i) {
				if (m_traps.at(indexes[i])->canBeIgnited())
					m_traps.at(indexes[i])->ignite();
			}
		}

		tileAt(playerCoords).shrine_action = ""; // Remove prompt ability
	}
}
bool Shrine::sacrificeWeapon() {
	if (player[0]->getWeapon()->getName() == HANDS && player[0]->getStoredWeapon() == nullptr)
		return false;

	// Only possesses 1 weapon
	if (player[0]->getWeapon()->getName() == HANDS && player[0]->getStoredWeapon() != nullptr ||
		player[0]->getStoredWeapon() == nullptr) {

		if (player[0]->getStoredWeapon() != nullptr)
			player[0]->switchWeapon(); // Swap usable weapon into their hand

		player[0]->equipWeapon(std::make_shared<ShortSword>());
	}
	// Possesses 2 weapons
	else {
		// Choose one at random and remove it
		if (randReal(1, 100) > 50)
			player[0]->switchWeapon();

		player[0]->removeStoredWeapon();	
	}

	return true;
}
void Shrine::giveRelic() {
	player[0]->decreaseFavorBy(4);
	player[0]->equipRelic(rollRelic(this, Rarity::COMMON));
}
void Shrine::upgradeRelic() {
	playSound("Potion_Pickup.mp3");
	player[0]->getRelic()->upgrade(*this, *player[0]);

	player[0]->decreaseFavorBy(2);
}
void Shrine::swapRelic() {
	int prevRelicLevel = player[0]->getRelic()->getLevel();

	std::shared_ptr<Relic> relic = rollRelic(this, Rarity::COMMON);

	player[0]->removeRelic();
	relic->setLevel(std::max(1, prevRelicLevel - 1));
	player[0]->equipRelic(relic);

	player[0]->decreaseFavorBy(3);
}
void Shrine::spawnChests() {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();

	int n = x - 1; int m = y + 1;
	for (int i = x - 1; i < x + 2; i++) {

		n = i;
		m = y + 1;

		if (itemObject(n, m))
			itemHash(n, m);

		/*std::shared_ptr<Objects> objects(nullptr);
		switch (randInt(0, 5)) {
		case 0: objects = std::make_shared<BrownChest>(this, n, m); break;
		case 1: objects = std::make_shared<SilverChest>(this, n, m); break;
		case 2: objects = std::make_shared<GoldenChest>(this, n, m); break;
		case 3: objects = std::make_shared<LifeChest>(this, n, m); break;
		case 4: objects = std::make_shared<ExplodingChest>(this, n, m); break;
		case 5: objects = std::make_shared<HauntedChest>(this, n, m); break;
		}*/

		tileAt(Coords(n, m)).object = rollChest(this, n, m, Rarity::ALL, true);
		m_items.push_back(tileAt(Coords(n, m)).object);

		chestCoordinates.push_back(Coords(n, m));
	}

	player[0]->decreaseFavorBy(2);
}

std::vector<char> Shrine::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char r = 'r'; // Rock
	char k = 'k'; // 
	char d = 'd'; //
	// 'W' : Regular Wall
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'T' : Trinkets
	// 'H' : Health
	// 'C' : Chest choice
	// 't' : Lit Brazier
	// 'u' : Unlit Brazier

	/**
	* RegEx: '#',(( '[A-Za-z#@ ]{1}',){24}) '#',
	*/

	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*2*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*3*/	'#', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*4*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*5*/	'#', 'W', 'W', 'W', 'W', 'W', 'B', 'H', ' ', 'W', 'W', ' ', 'T', ' ', 'W', 'W', ' ', 'C', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*6*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', ' ', ' ', ' ', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*7*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', 'W', 'W', ' ', 'W', 'W', ' ', ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*8*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 't', 'W', ' ', 'W', 't', ' ', ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*9*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*10*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*11*/	'#', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', ' ', ' ',  s , ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*12*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ', 'B', 'B', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', '#',
		/*13*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'B', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*14*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', '#',
		/*15*/	'#', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*16*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*17*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*18*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*19*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', 'W', 'W', 'W', '@', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*20*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*21*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*22*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*23*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*24*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
	};

	return level;
}


// ==============================================
//				:::: 1ST FLOOR ::::
// ==============================================
FirstFloor::FirstFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIRST_FLOOR, MAXROWS, MAXCOLS) {

	// BEGIN LEVEL GENERATION
	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec);
	/// END LEVEL GENERATION

	generateNPCs();
	generateMonsters();
	generateTraps();
	generateItems();
}

// Level generation
std::vector<std::string> FirstFloor::topChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> one =
	{ {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  i_bc,  sp, wll, wll},
	  {  sp, m_se,  t_f,  t_p, wll,   sp},
	  {  sp, m_se,  m_z,   sp,   _dv,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> two =
	{ {wll, t_tr,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll, wll,   sp,  PIT,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> three =
	{ {wll,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp, t_df,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> four =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {wll,   sp,  m_k,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> five =
	{ {wll, i_ri, wll, wll, t_td,   sp},
	  {t_tr,   sp,   sp,   sp,   sp,   sp},
	  {wll,  PIT, wll,   sp,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> six =
	{ {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s} };

	std::vector<std::vector<std::string>> seven =
	{ {wll, wll, PIT, wll,  i_gd, wll},
	  {t_df, wll,   sp, wll,   sp, wll},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp, wll,  PIT, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> eight =
	{ {t_td,   sp,   sp,   sp,   sp, t_td},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, t_tl, wll, wll, t_tr,   sp},
	  {  sp, wll, wll, wll, wll,   sp} };

	std::vector<std::vector<std::string>> nine =
	{ {  sp,   sp,  t_f, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll, t_mb},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,  t_f,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ten =
	{ {wll, wll,   sp, wll, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {  sp, wll, i_gd, wll,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> eleven =
	{ {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {wll,   sp, i_gd, r_wl,   sp, r_wl},
	  {  sp,   sp, wll,   sp,  t_s,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp, r_wl},
	  {wll,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,  wll,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  { wll,  t_p,   sp,   sp,   sp,  wll},
	  { wll,   sp,   sp,   sp,   sp,  wll},
	  {  sp,   sp,   sp, t_ts,  wll,   sp},
	  {  sp,   sp,  wll,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ { t_sk, t_sk, i_rc, i_gd, t_sk, t_sk},
	  { t_sk, t_sk, t_cf, t_cf, t_sk, t_sk},
	  {   sp, t_sk, t_sk, t_sk, t_sk,   sp},
	  {   sp,   sp, t_sk, t_sk,   sp,   sp},
	  {   sp,   sp,   sp,   sp,   sp,   sp},
	  {t_str,   sp,   sp,   sp,   sp,t_stl} };

	std::vector<std::vector<std::string>> xiv =
	{ {  wll,   sp,   sp,   sp,t_sbr,  wll},
	  {t_str,  wll,   sp,   sp,  wll,   sp},
	  {   sp,   sp,  wll,  wll,   sp,   sp},
	  {   sp,  t_s,  wll,  wll,   sp,  t_s},
	  {   sp,  wll,   sp,t_str,  wll,   sp},
	  {  wll,   sp,   sp,   sp,   sp,  wll} };

	std::vector<std::vector<std::string>> xv =
	{ { wll, i_gd,   sp,  wll, t_td,  wll},
	  { wll,  wll,   sp,  wll,   sp,  wll},
	  { wll,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,t_mbh,   sp,  t_p,   sp},
	  {  sp,   sp, t_tu,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xvi =
	{ {   sp,   sp, t_sk,   sp,   sp,  wll},
	  {   sp,   sp,t_mbv,   sp,   sp,t_mbv},
	  {   sp,   sp,   sp, i_ri,t_mbh,   sp},
	  {   sp,   sp, t_cf, m_gs,   sp,   sp},
	  {t_mbh, t_pt,   sp,   sp, t_ts,   sp},
	  {   sp,t_mbv,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ { wll,   sp,  m_rm, i_rw, m_gs,  wll},
	  { _dv, m_rm,  m_rm, m_rm,  sp,  wll},
	  { wll, t_ts,   sp,   sp, i_gd,  wll},
	  { wll,   sp, m_br,   sp,  t_s,  wll},
	  { wll,  wll,  wll,  _dh,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xviii =
	{ {  sp,  wll,  wll, i_ri,  wll,  wll},
	  { wll,   sp,  wll,  wll, r_wl,  wll},
	  {  sp,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,  wll},
	  {  sp,   sp,  wll,   sp, r_wl,  wll},
	  {  sp,  wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xix =
	{ { unb,  _x_,  unb, i_gc,  t_f,  unb},
	  { _dv, t_as,  unb,  _x_,   sp,  unb},
	  { unb,  _dh,  unb,  _x_,   sp,  unb},
	  { unb,t_mbh,  _x_,  _x_,  wll,  unb},
	  { unb,  _x_, t_su,  _x_,  unb,  unb},
	  { unb,  unb,  unb,  unb,  wll,  wll} };

	std::vector<std::vector<std::string>> xx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  { wll, t_tr,   sp,   sp, t_cf,  wll},
	  { wll, t_tr,   sp,   sp, t_cf,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	/*c.push_back(xx);
	c.push_back(xxi);
	c.push_back(xxii);
	c.push_back(xxiii);
	c.push_back(xxiv);
	c.push_back(xxv);
	c.push_back(xxvi);*/

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> FirstFloor::middleChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {r_wl,   sp, r_wl,   sp, r_wl,   sp},
	  {  sp,   sp, t_ts, t_ts,   sp,   sp},
	  {  sp, r_wl, t_ts, t_ts,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp, r_wl},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll, wll},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {t_td,   sp, r_wl,   sp, wll,   sp},
	  {  sp, wll,   sp, wll, t_df,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp, r_wl,   sp,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, r_wl,   sp},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, i_gd, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {wll, wll,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,  t_p,   sp,   sp, wll,   sp},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp, t_tl},
	  { t_f, wll, wll, wll,   sp, wll},
	  {wll,   sp,   sp, r_wl,   sp, wll},
	  {  sp,   sp, r_wl, r_wl,   sp, wll},
	  {t_tr,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp, r_wl, t_f,   sp, r_wl},
	  {r_wl, r_wl,   sp, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl,   sp, r_wl,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl, r_wl},
	  {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl, t_f, r_wl, r_wl} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,  PIT,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,  PIT,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  t_s,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp, wll,   sp,   sp, r_wl, r_wl},
	  {  sp,   sp, wll,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp, wll, i_ri, wll, wll,   sp},
	  {wll, wll, t_ts, wll, wll,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll, i_bc, wll,   sp},
	  {  sp, wll, m_rm,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {  sp, wll,   sp, t_tu,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp,   sp, r_wl, r_wl, t_mb,   sp},
	  {  sp,  t_s, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {wll, r_wl,   sp,   sp,   sp, t_td},
	  {  sp,   sp, r_wl,   sp,   sp,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, t_df, wll, wll, wll, t_tu} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, t_tl},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp, t_ts, t_mb,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,  t_s, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {wll, r_wl,   sp, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, i_gd, wll, r_wl, wll},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> xviii =
	{ {wll, r_wl, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, r_wl, wll, i_gd, wll, wll},
	  {wll, wll, r_wl, wll, wll, wll},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll, wll,   sp, wll} };

	std::vector<std::vector<std::string>> xix =
	{ {wll,   sp, wll, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp, wll},
	  {wll, wll,   sp,   sp, wll, wll},
	  {  sp,   sp,   sp,   sp, r_wl, wll},
	  {wll, wll, wll,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> xx =
	{ {wll,   sp,   sp, r_wl,   sp,   sp},
	  {wll,   sp,   sp, r_wl, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp, wll,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> xxii =
	{ {  sp,   sp,  wll,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  { wll,  _x_,  _x_, m_gs,  wll,   sp},
	  { wll,  _x_,  wll, i_gd,  wll,   sp},
	  { wll,  _x_,  _x_,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,t_sbr,  _x_,t_sbl,   sp},
	  {  sp,   sp,  _x_, t_pt,  _x_,   sp},
	  {  sp,   sp,t_str,  _x_,t_stl,   sp},
	  {r_wl,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiv =
	{ {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,  wll, t_sk,  wll,  wll,  wll},
	  {  sp,  wll, t_ts, t_ts, i_ri,  wll},
	  {  sp,  _x_, t_ts,  wll,  wll,   sp},
	  {  sp,  wll,  _x_,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,  wll,   sp} };

	std::vector<std::vector<std::string>> xxv =
	{ {  sp,   sp,   sp, t_td,   sp,   sp},
	  {  sp,  t_s,   sp,  t_p,   sp,   sp},
	  {  sp,   sp,   sp, t_ts,   sp,   sp},
	  {  sp,  wll, t_sk, i_ri, t_sk,  wll},
	  {  sp,   sp,   sp, t_sk,   sp,   sp},
	  {  sp,  t_s,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvi =
	{ {r_wl,  wll,   sp,   sp,  wll,   sp},
	  { wll,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  t_p,   sp,   sp,   sp},
	  {  sp,   sp,t_mbv,   sp,  t_s,   sp},
	  {  sp,   sp, t_pt,   sp,  wll,   sp},
	  { wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvii =
	{ {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,  wll,   sp,   sp,   sp,   sp},
	  { wll,   sp,   sp,  wll,   sp,   sp},
	  {  sp,   sp, t_tl,  wll,   sp,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxviii =
	{ {t_tr,   sp,   sp,   sp,   sp, t_td},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {t_tu,   sp,   sp,   sp,   sp, t_tl} };

	std::vector<std::vector<std::string>> xxix =
	{ {t_td,  t_p,   sp,   sp,  t_p, t_tl},
	  {  sp,   sp,   sp, t_sk,   sp,   sp},
	  {  sp,   sp, t_pt,   sp,   sp,  t_s},
	  { t_p,   sp,   sp, t_sk,   sp,   sp},
	  {  sp, t_sk,   sp,   sp, t_ts,  t_p},
	  {t_tr,   sp,  t_p,   sp,   sp, t_tu} };

	std::vector<std::vector<std::string>> xxx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,  wll,  t_s,   sp},
	  { wll, t_ts,  wll,  wll,  wll,   sp},
	  { wll,  wll, i_bc,  wll,  wll,   sp},
	  { wll, t_ts,  wll, t_ts, t_ts,   sp},
	  {  sp,  wll,  wll, t_ts,   sp, t_tu} };

	/*std::vector<std::vector<std::string>> xxxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,  t_s,  t_s,  t_s,   sp},
	  {  sp,  t_s,  t_s,  t_s,   sp,   sp},
	  {  sp,  t_s,  t_s,   sp,  t_s,   sp},
	  {  sp,   sp,  t_s,  t_s,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };*/

	std::vector<std::vector<std::string>> xxxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };


	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	c.push_back(xxii);
	c.push_back(xxiii);
	c.push_back(xxiv);
	c.push_back(xxv);
	c.push_back(xxvi);
	c.push_back(xxvii);
	c.push_back(xxviii);
	c.push_back(xxix);
	c.push_back(xxx);
	c.push_back(xxxi);

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> FirstFloor::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, t_s,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {t_tr,   sp,   sp,   sp,   sp, wll},
	  {wll, i_bc, wll, wll,   sp, r_wl} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp, wll, wll, wll,   sp, wll},
	  {  sp, wll,   sp,  m_g, wll, r_wl},
	  {  sp, wll,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll,   sp},
	  {wll,  PIT,   sp, wll, wll,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, t_s, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, t_s, wll, wll},
	  {wll, wll,   sp,   sp, r_wl, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll, i_gd, i_ri, i_gd,   sp, wll} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp, i_ri, wll, wll},
	  { m_z,   sp, t_tu, t_tu, wll, wll} };

	std::vector<std::vector<std::string>> v =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,  t_f, wll,   sp, r_wl, r_wl},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp,   sp,   sp},
	  {wll, wll,   sp, wll,   sp,   sp},
	  {wll, r_wl, wll, wll,   sp,   sp},
	  {wll, i_ri, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp, wll, wll},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {wll, i_gd,   sp, r_wl,   sp, wll},
	  {  sp, wll,   sp, t_mb,   sp, r_wl},
	  {  sp,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp, wll,   _dh, wll,   sp,   sp},
	  {wll, wll,   sp, wll, wll,   sp},
	  {wll, wll,   sp, wll, wll, r_wl},
	  {wll,   sp,   sp,   sp, wll, wll},
	  {wll, wll,   sp, m_rm, m_rm, wll},
	  {wll, wll, m_rm, m_rm, m_rm, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp, wll, wll, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll, wll,   sp},
	  {wll, wll, t_ts, wll, t_ts,   sp},
	  {wll,   sp, t_ts, t_ts, t_ts, wll},
	  {wll, t_ts, t_ts,   sp, t_ts, wll} };

	std::vector<std::vector<std::string>> x =
	{ {r_wl,   sp,   sp,   sp, wll,   sp},
	  {  sp,   sp, r_wl,   sp, t_mb, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,  t_s, wll, wll,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll, wll, t_mb,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl, wll, wll, wll, wll, r_wl} };

	std::vector<std::vector<std::string>> xii =
	{ {wll,   sp,   sp,   sp,   sp, r_wl},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, i_gd, wll, t_f,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_mb,   sp,   sp},
	  {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, m_g,  m_g, wll, wll},
	  {wll, wll, i_rc, i_sc, wll, wll} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp, wll, wll,   _dh, wll},
	  {  sp, wll, m_rm, wll,   sp, wll},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   _dv,   sp, wll, r_wl, wll},
	  {  sp, wll,   sp,   _dv,   sp,   sp},
	  { t_s, wll, wll, wll, wll,  t_f} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xviii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };


	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	/*c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);*/

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::vector<std::vector<std::string>>> FirstFloor::mixChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	int cols = getCols();

	std::string x = SPAWN_DISALLOWED;
	std::string play = PLAYER;
	std::string exit = STAIRCASE;


	std::vector<std::vector<std::vector<std::string>>> v;

	std::vector<std::vector<std::string>> pChunk =
	{ {   x,    x, wll, wll, r_wl,    x},
	  {wll,    x,    x,    x,    x,    x},
	  {   x,    x, play,    x,    x, wll},
	  {wll,    x,    x,    x,    x, wll},
	  {r_wl,    x,    x,    x,    x, wll},
	  {   x, wll, wll,    x, wll,    x} };

	std::vector<std::vector<std::string>> eChunk =
	{ {   x,    x, r_wl, r_wl, r_wl,    x},
	  {r_wl, m_rm, m_rm, m_rm,    x,    x},
	  {   x, m_rm, exit, m_rm,    x, r_wl},
	  { wll, m_rm, m_rm, m_rm, r_wl,  wll},
	  {r_wl,    x,    x,    x,    x,  wll},
	  {   x,    x, r_wl,    x,   wll,   x} };

	std::vector<std::vector<std::string>> specialRoom1 =
	{ {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  m_g,   sp, wll, wll},
	  {wll, wll, i_sc, i_sc, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, wll, wll, wll,   sp}, };

	std::vector<std::vector<std::string>> specialRoom2 =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };



	for (int i = 0; i < (cols - 2) / 6 /*number_of_columns - 2 / length_of_each_chunk*/; i++) {
		if ((layer - 1) * 6 + i == playerChunk)
			v.push_back(pChunk);
		else if ((layer - 1) * 6 + i == exitChunk)
			v.push_back(eChunk);
		else if ((layer - 1) * 6 + i == specialChunk1)
			v.push_back(specialRoom1);
		else if ((layer - 1) * 6 + i == specialChunk2)
			v.push_back(specialRoom2);
		else
			v.push_back(c[randInt(0, (int)c.size() - 1)]);
	}

	/*if (layer == specialChunkLayer1) {
		s = randInt(0, (int)v.size() - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom1);
	}
	if (layer == specialChunkLayer2) {
		s = randInt(0, (int)v.size() - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom2);
	}*/

	//// put in player chunk
	//v.erase(v.begin() + playerChunk);
	//v.emplace(v.begin() + playerChunk, pChunk);

	return v;
}
std::vector<std::string> FirstFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (unsigned int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 6; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}

std::vector<std::string> FirstFloor::generateLevel() {
	int rows = getRows();
	int cols = getCols();

	std::vector<std::vector<std::vector<std::string>>> b;
	std::vector<std::string> v, finalvec;


	// player spawns in middle with special chunk
	int n = randInt(1, 4);
	switch (n) {
	case 1: playerChunk = 14; break;
	case 2: playerChunk = 15; break;
	case 3: playerChunk = 20; break;
	case 4: playerChunk = 21; break;
	}

	// exit spawns in one of four corners
	n = randInt(1, 4);
	switch (n) {
	case 1: exitChunk = 0; break;
	case 2: exitChunk = 5; break;
	case 3: exitChunk = 30; break;
	case 4: exitChunk = 35; break;
	}

	// chooses layer for the special chunks to be on
	specialChunk1 = randInt(0, 35);
	while (specialChunk1 == playerChunk || specialChunk1 == exitChunk) {
		specialChunk1 = randInt(0, 35);
	}

	specialChunk2 = randInt(0, 35);
	while (specialChunk2 == playerChunk || specialChunk2 == exitChunk) {
		specialChunk2 = randInt(0, 35);
	}

	specialChunkLayer1 = randInt(1, 6);
	specialChunkLayer2 = randInt(1, 6);
	layer = 1;

	int count = 0;
	while (count < (rows - 2) / 6 /* divided by length of chunk*/) {
		if (count == 0) {
			v = topChunks(b);
			layer++;
		}
		else if (count >= 1 && count <= 4) {
			v = middleChunks(b);
			layer++;
		}
		else if (count == 5) {
			v = bottomChunks(b);
			layer++;
		}

		for (unsigned int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		b.clear();
		count++;
	}

	return finalvec;
}


// ==============================================
//				:::: 2ND FLOOR ::::
// ==============================================
SecondFloor::SecondFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SECOND_FLOOR, MAXROWS2, MAXCOLS2), m_watersCleared(true), m_guardiansLeft(-1) {
	int rows = getRows();
	int cols = getCols();

	// :::: GENERATE LEVEL ::::
	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec);
	// ---- END LEVEL GENERATION ----


	//	BUTTON for unlocking exit
	std::shared_ptr<Traps> button = std::make_shared<Button>(*this, cols / 2 - 1, rows / 2 - 6);
	m_traps.push_back(button);


	// GENERATE EXTRA MONSTERS
	int n, x, y;
	std::shared_ptr<Monster> monster(nullptr);
	for (int i = 0; i < 18; i++) {

		//	selects how many of a type of monster to place
		switch (i) {
		case 0: n = 8 + randInt(0, 4); break; // 
		case 1: n = 5 + randInt(0, 5); break; //
		case 2: n = 6 + randInt(0, 8); break; // 
		case 3: n = 15 + randInt(0, 10); break; // 
		case 4: n = 1 + randInt(0, 2); break; // 
		case 5: n = 2 + randInt(0, 2); break; // 
		case 6: n = 5 + randInt(0, 3); break; // 
		case 7: n = 14 + randInt(0, 3); break; // 
		case 8: n = 5 + randInt(0, 4); break; //
		case 9: n = 8 + randInt(0, 4); break; //
		case 10: n = 12 + randInt(0, 4); break; //
		case 11: n = 8 + randInt(0, 4); break; //
		case 12: n = 4 + randInt(0, 4); break; //
		case 13: n = 8 + randInt(0, 4); break; //
		case 14: n = 4 + randInt(0, 4); break; //
		case 15: n = 1 + randInt(0, 1); break; //
		case 16: n = 10 + randInt(0, 4); break; //
		case 17: n = 4 + randInt(0, 4); break; //
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || trap(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// the type of enemy
			switch (i) {
			case 0:
				monster = std::make_shared<Wanderer>(this, x, y); break;
			case 1:
				monster = std::make_shared<RabidWanderer>(this, x, y); break;
			case 2:		
				monster = std::make_shared<ProvocableWanderer>(this, x, y); break;
			case 3:
				monster = std::make_shared<FlameWanderer>(this, x, y); break;
			case 4:
				monster = std::make_shared<Bombee>(this, x, y); break;
			case 5:
				monster = std::make_shared<CharredBombee>(this, x, y); break;
			case 6:
				monster = std::make_shared<Roundabout>(this, x, y); break;
			case 7:
				monster = std::make_shared<FireRoundabout>(this, x, y); break;
			case 8:
				monster = std::make_shared<Zapper>(this, x, y); break;
			case 9:
				monster = std::make_shared<Spinner>(this, x, y); break;
			case 10:
				monster = std::make_shared<Puff>(this, x, y); break;
			case 11:
				monster = std::make_shared<GustyPuff>(this, x, y); break;
			case 12:
				monster = std::make_shared<StrongGustyPuff>(this, x, y); break;
			case 13:
				monster = std::make_shared<InvertedPuff>(this, x, y); break;
			case 14:
				monster = std::make_shared<SpikedInvertedPuff>(this, x, y); break;
			case 15:
				monster = std::make_shared<ItemThief>(this, x, y); break;
			case 16:
				monster = std::make_shared<Serpent>(this, x, y); break;
			case 17:
				monster = std::make_shared<Charger>(this, x, y); break;
			}

			addMonster(monster);

			n--;
			monster.reset();
		}
	}
	
}

void SecondFloor::specialActions() {
	// check if player entered devil's waters and killed all guardians
	for (int i = m_guardians.size() - 1; i >= 0; i--) {
		if (m_guardians[i]->isDead()) {
			m_guardians[i].reset();
			m_guardians.erase(m_guardians.begin() + i);
			m_guardiansLeft--;
			if (m_guardiansLeft == 0)
				m_watersCleared = true;
		}
	}

	if (m_guardiansLeft == 0 && m_watersCleared) {
		guardiansDefeated();

		m_guardiansLeft = -1; // prevents if statement from being entered again
	}
}
bool SecondFloor::specialTrapCheck(int x, int y) {
	std::vector<int> indexes = findTraps(x, y);
	for (int i = 0; i < static_cast<int>(indexes.size()); ++i) {
		if (m_traps.at(indexes[i])->getName() == BUTTON_UNPRESSED) {
			if (m_exit->isLocked()) {
				playSound("Button_Pressed.mp3");
				playSound("Exit_Opening.mp3");

				m_exit->toggleLock();

				// Replace button with pressed button
				m_traps.at(indexes[i])->getSprite()->setSpriteFrame("Button_Pressed_48x48.png");

				// Replace staircase sprite
				m_exit->getSprite()->setSpriteFrame("Stairs_48x48.png");
			}
		}
		else
			trapEncounter(x, y);
	}

	return true;
}

void SecondFloor::devilsWater(bool bathed) {
	if (bathed) {
		m_waterPrompt = false;
		devilsGift();
	}
	m_waterPrompt = false;
}
void SecondFloor::devilsGift() {
	m_watersUsed = true;

	player[0]->increaseStatBy(StatType::MAXHP, 30);
	player[0]->addAffliction(std::make_shared<HealOverTime>(30));

	std::shared_ptr<Monster> g1, g2, g3, g4;
	g1 = std::make_shared<ArmoredSerpent>(this, 31, 27);
	g2 = std::make_shared<ArmoredSerpent>(this, 31, 28);
	g3 = std::make_shared<ArmoredSerpent>(this, 42, 27);
	g4 = std::make_shared<ArmoredSerpent>(this, 42, 28);
	
	g1->setName("Guardian");
	g2->setName("Guardian");
	g3->setName("Guardian");
	g4->setName("Guardian");

	m_monsters.push_back(g1); m_guardians.push_back(g1);
	m_monsters.push_back(g2); m_guardians.push_back(g2);
	m_monsters.push_back(g3); m_guardians.push_back(g3);
	m_monsters.push_back(g4); m_guardians.push_back(g4);

	// Close off the exits with walls after accepting the devils gift
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			m_walls.push_back(std::make_shared<UnbreakableWall>(this, i, j));
		}
	}

	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			m_walls.push_back(std::make_shared<UnbreakableWall>(this, i, j));
		}
	}

	m_watersCleared = false;
	m_guardiansLeft = 4;
}
void SecondFloor::guardiansDefeated() {
	playSound("Guardians_Defeated.mp3");

	// remove the blocked walls
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			removeWall(i, j);
		}
	}
	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			removeWall(i, j);
		}
	}
}
void SecondFloor::devilsWaterPrompt() {
	m_waterPrompt = true;

	m_scene->callDevilsWaters();
}



std::vector<std::vector<std::string>> SecondFloor::outermostChunks() {

	std::vector<std::vector<std::string>> i = 
	{ { wll,  wll,  wll, i_gd,  wll,   sp,  wll,  wll},
	  { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   xx, t_cl,   xx},
	  {  sp,   xx,  t_f,   xx,   sp,   sp,  wll,   sp},
	  { wll,   xx,   xx,   xx,  wll, t_lv,  wll,   sp},
	  {  sp,  wll,   sp,  wll, i_gd,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ii = 
	{ {r_wl,   sp, t_td,  wll,  wll,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp, t_lv, t_lv,  t_s, t_mb,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> iii = 
	{ { wll,  wll,   sp,   sp, t_cl,   sp,  wll,   sp},
	  {  sp,  wll,   sp,  wll,   sp,   sp,   sp,  wll},
	  {  sp,   sp,   sp,  t_f,  wll,   sp,   sp, r_wl},
	  {  sp,   sp,  wll, i_gd,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp,   sp,   sp},
	  { wll,   sp,   sp, t_cl,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll,   sp, i_gd,  wll,  wll,   sp,   sp,  wll},
	  { wll, i_gd,   sp,  wll,  wll, i_ri,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll, t_tr,   sp, t_cl, t_cl,   sp,   sp,  wll},
	  { wll,   sp,   sp, t_cl, t_cl,   sp, t_tl,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi = 
	{ {  sp,  t_s,   sp,  wll,  wll,  m_z,  m_z,  wll},
	  { wll,  wll,   sp,  wll,  wll,   xx,   xx,   sp},
	  {m_sp, t_cl,   sp,  wll, t_cl,   xx,  wll,   sp},
	  { wll,  wll,   xx, t_cl,   xx,   xx,  wll,   sp},
	  {  sp,  t_s,   sp,   xx,  t_f,   sp, t_sk, r_wl},
	  {  sp,   sp,   sp,   sp,  wll,   sp, t_sk,   sp} };

	std::vector<std::vector<std::string>> vii = 
	{ { wll,   sp,   sp,   sp,  t_s,   sp,  m_z,   sp},
	  {  sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,   xx,   sp,   sp},
	  {  sp,  t_s,   sp,   sp, r_wl, t_cl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   xx,  wll,   sp},
	  {  sp,  m_z,  t_s, t_mb, t_cl, t_cl, t_cl,  wll} };

	std::vector<std::vector<std::string>> viii = 
	{ {t_td,   sp, t_td,   sp, t_td,   sp, t_td,   sp},
	  {  sp,   sp,   sp,  t_s,   sp,  t_s,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp},
	  {  sp, t_tu,   sp, t_tu,   sp, t_tu,   sp, t_tu} };

	std::vector<std::vector<std::string>> ix = 
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {  sp,   sp,   sp,   sp,   sp, m_sp,   sp,   sp},
	  {  sp,   sp, m_sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp, t_lv, t_lv, t_cl, t_lv,   sp,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_cl, t_cl, t_lv, t_cl, t_cl,   sp},
	  {  sp, t_cl, t_cl, t_lv, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_lv, t_lv,   sp},
	  {  sp,   sp, t_lv, t_cl, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> xi = 
	{ {  sp,   sp, t_lv, i_bc,   sp, t_lv,   sp,   sp},
	  {  sp,   sp, t_lv, t_lv,   sp, t_lv,   sp,   sp},
	  {  sp, m_sp,   sp, t_lv, t_sk, t_lv, m_sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_tl, t_tr,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii = 
	{ {  sp, t_cl,   sp,t_mbv, t_cl, t_lv, t_cl,   sp},
	  {  sp,t_mbv, t_cl, t_cl,   sp, t_cl, t_cl,   sp},
	  {  sp,   sp, t_cl,   sp,t_mbv, t_cl,   sp,   sp},
	  {  sp,   sp,t_mbv,   sp, t_cl, t_cl,   sp,   sp},
	  {  sp, t_cl,   sp,   sp, t_cl, t_cl,t_mbv,   sp},
	  {  sp, t_cl,   sp,t_mbv,   sp,   sp, t_cl,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/


	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> SecondFloor::edgeChunks() {
	std::vector<std::vector<std::string>> i =
	{ { wll,   sp,  wll,  wll,  wll,   xx,   xx,   xx},
	  { wll,  wll,   sp,   sp,   sp,  wll,   xx,  wll},
	  {  sp,   sp,   xx,   sp,   sp,   sp,   xx,   sp},
	  { wll,   sp,   xx,  wll,  wll,   sp,   sp, r_wl},
	  { wll,  wll,   xx,   sp,   sp,  wll,  wll,   sp},
	  {r_wl,   sp,  wll,   sp,   sp,   sp,   sp,  wll} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_rc},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_gd},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  {i_gd,   sp, m_sp, t_cl, t_cl,   sp,  m_k,  wll},
	  {i_rc,   sp, m_sp, t_cl, t_cl,   sp,  m_a,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,  wll, t_cl,   sp,  wll,   sp},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  wll, m_sp,  wll,   sp,  wll, m_sp},
	  { wll, t_lv,   sp,   sp,   sp, t_cl,   sp,   sp},
	  {  sp,  wll,   sp,   sp, t_cl,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,  wll,   sp,  t_s},
	  {  sp,  wll,   sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,  m_z,   sp,   sp, m_sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,  wll,   xx,   xx,   sp,  wll,   sp},
	  {  xx,  wll,   xx,   xx,  wll,   sp,   sp,  wll},
	  {  xx,  wll, i_rt, t_lv,   sp,  wll,   sp,  wll},
	  {  xx, t_lv,  wll,  wll,   sp,  wll,   sp,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ {  xx,   xx,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,  t_f, m_rm,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll},
	  {t_lv, t_lv, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll, i_rs, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll,  wll, t_lv,   sp,   sp,   sp,  t_f,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ { wll,   xx,   sp,  wll,  wll,  wll,   sp,  wll},
	  { wll,   xx, t_df,   xx,   xx,   xx,  wll,  wll},
	  { wll,   xx,   xx,   xx,  wll,   xx,   sp,  wll},
	  { t_f,  wll,  wll,  wll,   xx,   xx,   sp,  wll},
	  {  xx,   xx,   xx,   xx,   xx, t_df,   sp, i_gd},
	  {  xx,   xx,  wll, t_ts,  wll,   sp,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ { wll,   sp,   sp,   sp,   sp,   sp,  wll, i_gd},
	  {  sp, t_sk,   sp, t_sk,   sp, t_sk,  wll,  wll},
	  {  sp,   sp, t_as,   sp, t_as,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, m_rm,   sp, m_sp,   sp, m_rm,   sp, m_rm},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  t_f, i_gd,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll, m_sp, i_gd,   sp,   sp},
	  {  sp,   sp, i_ri, m_sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, i_gd,  t_f,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ { wll,  wll, r_wl,  m_z, m_sp,  wll,  wll,  wll},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,  wll,   sp, m_rm,   sp,   sp,   sp, r_wl},
	  { wll, r_wl,   sp,   sp, m_rm,   sp,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,   sp,  wll,  wll},
	  { wll,  wll,  wll, m_sp,  m_z,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp, t_lv,  wll},
	  { wll, t_lv, t_lv,   sp, t_cl, t_lv, i_rc, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_cl,   sp, t_lv,   sp},
	  {  sp,   sp, t_lv, t_lv, t_cl,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixEdgeChunks(c);

	return c.at(randInt(0, (int)c.size() - 1));
}

/*
std::vector<std::string> SecondFloor::innerChunksCT(std::vector<std::vector<std::vector<std::string>>> &c) {
	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);

	c = mixInnerChunksCT(c);

	return combineChunks(c);
}*/

std::vector<std::vector<std::string>> cornerChunks(int corner, bool innerRing) {

	/**  Corner legend:
	*    TL: 1 (A)
	*    TR: 2 (B)
	*    BL: 3 (C)
	*    BR: 4 (D)
	*/

	// Top-left: 1
	std::vector<std::vector<std::string>> A_i =
	{ {i_ri, t_cl, m_gs,   sp,   sp,   sp,   sp,   sp},
	  {t_cl, t_cl,   sp,   sp, r_wl,   sp,   sp,   sp},
	  {m_gs,   sp, m_gs,   sp, m_rm,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, r_wl,   sp,   sp,   sp},
	  {  sp, r_wl, m_rm, r_wl, r_wl,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Top-right: 2
	std::vector<std::vector<std::string>> B_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Bottom-left: 3
	std::vector<std::vector<std::string>> C_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Bottom-right: 4
	std::vector<std::vector<std::string>> D_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(A_i);
	c.push_back(A_ii);
	c.push_back(A_iii);
	c.push_back(A_iv);
	c.push_back(B_i);
	c.push_back(B_ii);
	c.push_back(B_iii);
	c.push_back(B_iv);
	c.push_back(C_i);
	c.push_back(C_ii);
	c.push_back(C_iii);
	c.push_back(C_iv);
	c.push_back(D_i);
	c.push_back(D_ii);
	c.push_back(D_iii);
	c.push_back(D_iv);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	*/

	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> junctionChunks() {
	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	*/


	return c.at(randInt(0, (int)c.size() - 1));
}

std::vector<std::vector<std::string>> SecondFloor::lavaChunks() {
	//	 LAVA CHUNKS
	std::vector<std::vector<std::string>> i =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a, t_tt, t_tt,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a,  m_a, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv,  m_a,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iv =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  t_f, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> v =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv},
	  {t_lv, t_lv, t_lv, t_df, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_df, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vi =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv,  t_f, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> viii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ix =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	//c.push_back(x);
	//c.push_back(xi);
	//c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixInnerChunks1(c);

	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> SecondFloor::fixedChunks(std::string chunk) {
	// the player's chunk where they spawn
	std::vector<std::vector<std::string>> pChunk =
	{ {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,   xx},
	  { wll,   xx,   xx,   xx,   xx,   xx, r_wl,   xx},
	  {  xx,   xx,   xx, _ply,   xx,  wll,   xx,   xx},
	  {  xx,   xx,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,  wll,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,  wll}, };


	// the chunk where the staircase will spawn
	std::vector<std::vector<std::string>> eChunk =
	{ { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
	  { wll,   sp,   sp, m_rm,   sp, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, lkex, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, m_rm, m_rm,   sp,  wll},
	  { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll} };


	// the chunk where player's can enter the center ring
	std::vector<std::vector<std::string>> outerBridge =
	{ {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv} };

	// empty chunk
	std::vector<std::vector<std::string>> empty =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	// The 9 special chunks in the center

	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll, r_wl,  wll,  wll,  wll,  wll,  wll},
	  { wll, i_gd,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, i_gd,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, i_gd,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll, r_wl,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	// center chunk with fountain
	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll, i_ri,  wll,  wll,  wll,   sp,   sp},
	  { wll, i_rw,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, r_wl,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll, r_wl,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, r_wl,  wll,  wll},
	  { wll,  wll,  wll,  wll, i_gd,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	// get the correct chunk
	if (chunk == "pChunk") return pChunk;
	else if (chunk == "eChunk") return eChunk;
	else if (chunk == "outerBridge") return outerBridge;
	else if (chunk == "empty") return empty;
	else if (chunk == "i") return i;
	else if (chunk == "ii") return ii;
	else if (chunk == "iii") return iii;
	else if (chunk == "iv") return iv;
	else if (chunk == "v") return v;
	else if (chunk == "vi") return vi;
	else if (chunk == "vii") return vii;
	else if (chunk == "viii") return viii;
	else if (chunk == "ix") return ix;
	else return empty;
}

std::vector<std::string> SecondFloor::generateLevel() {
	std::vector<std::string> finalvec, b;
	std::vector<std::vector<std::vector<std::string>>> v;

	for (layer; layer <= 9; layer++) {
		for (int i = 0; i < (getCols() - 2) / 8 /* divided by length of chunk */; i++) {

			if (!(layer == 1 || layer == 9 || layer == 5) && (i == 0 || i == 8)) {
				v.push_back(edgeChunks());
			}
			else if (layer == 1 || layer == 9) {
				if (i == 0 || i == 4 || i == 8)
					v.push_back(fixedChunks("empty"));
			}
			else if (layer == 2 || layer == 8) {

				// lava chunks
				if (i >= 1 && i <= 3 || i >= 5 && i <= 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("outerBridge"));
				}
			}
			else if (layer == 3 || layer == 7) {

				// lava chunks
				if (i == 1 || i == 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("empty"));
				}
			}
			// 4 - 6 are the special chunks in the center
			else if (layer == 4) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("i"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("ii"));
				else if (i == 5) v.push_back(fixedChunks("iii"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}
			else if (layer == 5) {

				// special chunks
				if (i == 0) v.push_back(fixedChunks("pChunk"));
				else if (i == 2 || i == 6) v.push_back(fixedChunks("empty"));
				else if (i == 3) v.push_back(fixedChunks("iv"));
				else if (i == 4) v.push_back(fixedChunks("v"));
				else if (i == 5) v.push_back(fixedChunks("vi"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());
				else if (i == 8) v.push_back(fixedChunks("eChunk"));

			}
			else if (layer == 6) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("vii"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("viii"));
				else if (i == 5) v.push_back(fixedChunks("ix"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}


			// if nothing was added in the intermediary, then add an "outer" chunk
			if (i == v.size())
				v.push_back(outermostChunks());
		}

		// transform 3d vector into 1d vector
		b = combineChunks(v);

		// add to the final dungeon map
		for (unsigned int i = 0; i < b.size(); i++)
			finalvec.push_back(b.at(i));

		b.clear();
		v.clear();
	}

	return finalvec;
}
std::vector<std::string> SecondFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (unsigned int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 8; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}



// ==============================================
//				:::: 3RD FLOOR ::::
// ==============================================
ThirdFloor::ThirdFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, THIRD_FLOOR, MAXROWS3, MAXCOLS3), m_locked(false) {
	int rows = getRows();
	int cols = getCols();

	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec, 0, 0);

	// END LEVEL GENERATION
}

void ThirdFloor::checkDoors() {
	int cols = getCols();

	if (!m_doors.empty()) {
		int x, y;

		for (unsigned int i = 0; i < m_doors.size(); i++) {

			std::shared_ptr<Door> door = std::dynamic_pointer_cast<Door>(m_doors.at(i));
			x = door->getPosX();
			y = door->getPosY();

			// if door is locked, check if room is clear to unlock
			if (door->isLocked()) {
				if (roomCleared())
					toggleDoorLocks(x, y);
			}
			// if door is unlocked, open, isn't being held, and nothing is in the way
			else if (door->isOpen() && !door->isHeld() && !(enemy(x, y) || hero(x, y))) {
				playSound("Door_Closed.mp3");

				// show the room the player is in
				revealRoom();

				door->getSprite()->setVisible(true);

				door->toggleOpen();
				door->toggleHeld();
				tileAt(Coords(x, y)).wall = true;

				// if the room the player is now in isn't cleared, lock all doors
				if (!roomCleared())
					toggleDoorLocks(x, y);

			}
			else if (door->isOpen() && door->isHeld()) {
				// release hold on the door
				door->toggleHeld();
			}

			door.reset();
		}
	}
}
bool ThirdFloor::roomCleared() {
	int px = player[0]->getPosX();
	int py = player[0]->getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int mx, my, mxchunk, mychunk;
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		mx = m_monsters.at(i)->getPosX(); mxchunk = mx / 14;
		my = m_monsters.at(i)->getPosY(); mychunk = my / 10;

		if (mxchunk == xchunk && mychunk == ychunk)
			return false;	
	}

	return true;
}
void ThirdFloor::toggleDoorLocks(int dx, int dy) {
	Coords pos(dx, dy);

	Coords playerCoords = getPlayerCoordsClosestTo(pos);

	playSound("Door_Locked.mp3");

	int x, y;

	if (tileAt(pos).wallObject->getName() == DOOR_VERTICAL) {
		if (playerCoords.isLeftOf(pos)) {		
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx - 14 && y == dy) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy - 5) || (x == dx - 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}

		}
		// else if player is to the right of the given door
		else {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx + 14 && y == dy) || door->getPos() == pos)
					door->toggleLock();

				else if ((x == dx + 7 && y == dy - 5) || (x == dx + 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}
		}
	}
	// door is oriented horizontally
	else {
		if (playerCoords.isAbove(pos)) {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy - 10) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy - 5) || (x == dx + 7 && y == dy - 5))
					door->toggleLock();
				
				door.reset();
			}

		}
		// else if player is below the given door
		else {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy + 10) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy + 5) || (x == dx + 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}
		}
	}
}

void ThirdFloor::hideRooms() {
	// Hide all elements
	for (unsigned int i = 0; i < m_walls.size(); i++) {
		Coords currentPos(m_walls[i]->getPos());

		if (tileAt(currentPos).boundary || tileAt(currentPos).wallObject && tileAt(currentPos).wallObject->isDoor())
			continue;

		m_walls.at(i)->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		m_monsters[i]->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_items.size(); i++) {
		m_items.at(i)->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_traps.size(); i++) {
		m_traps[i]->getSprite()->setVisible(false);
	}
	
	revealRoom();
}
void ThirdFloor::revealRoom() {
	int px = player[0]->getPosX();
	int py = player[0]->getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int sx, sy, sxchunk, sychunk;

	for (unsigned int i = 0; i < m_walls.size(); i++) {
		sx = m_walls.at(i)->getPosX();
		sy = m_walls.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_walls.at(i)->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		sx = m_monsters.at(i)->getPosX();
		sy = m_monsters.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_monsters[i]->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_items.size(); i++) {
		sx = m_items.at(i)->getPosX();
		sy = m_items.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_items.at(i)->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_traps.size(); i++) {
		sx = m_traps.at(i)->getPosX();
		sy = m_traps.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_traps[i]->getSprite()->setVisible(true);
		
	}

}

void ThirdFloor::TLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{ { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp, m_ds,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
	  { wll,   sp,   sp, m_ds,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{	{ unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as,  i_b, t_as,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, i_lp,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  wll,  m_w,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, i_bc,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll, i_bc,  i_b, i_bc,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,   sp, m_ds,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	// contains a single corner chunk
	pickSingleChunk(current, pool);
}
void ThirdFloor::topChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_tr,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  t_f,   sp,  m_z,  wll,  m_z,   sp,   sp,  t_f,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_tl,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  t_f,  wll,  unb},
		{ wll, t_lv,  wll,   sp,  m_z,   sp,  wll,   sp,  m_z,   sp,  wll, t_lv,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  m_z,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_sp,  wll,   sp,   sp,   sp,  wll, m_sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_k,  wll,  m_g,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll, t_as, t_as,  wll, m_ds,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll, t_as, t_as,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  unb},
		{  sp,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp,   sp, m_ds, t_as,  wll, t_as, m_ds,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds,   sp, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{  sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  _dv},
		{ wll,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb,  wll,  unb},
		{ wll,  wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb,  wll,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,  wll,   sp,  wll,  wll,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  unb,  wll, t_ts,  wll,  unb,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, t_lv,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::TRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp, m_ds, m_ds, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds, m_ds, m_ds,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  m_k,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_a,  wll,  unb},
		{ wll,   sp,  m_w,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll, i_ar, t_lv,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, t_lv,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  unb,  unb,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb, m_sp,  unb,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  unb,  unb,  unb,  unb,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  unb,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  i_b, t_lv, t_tl,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::leftEdgeChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll, i_lp, t_ts,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_gs,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,t_mbv,  wll, i_ri,  wll,t_mbv,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp, t_lv,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w,  wll,   sp,   sp,   sp,   sp, m_br,  t_f,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll, i_hp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vi =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  t_s, t_cf,  t_s,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> viii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ix =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> x =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(vi);
	//pool.push_back(vii);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

}
void ThirdFloor::middleChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_br,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_fr,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb},
		{ m_z,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  m_r,  i_b,  m_r, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ m_z,   sp,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_as,  wll,   sp,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{  sp, t_ts, t_ts, t_as,  wll,  wll,  m_w,  wll,  wll, t_as, t_ts, t_ts,   sp,  _dv},
		{ wll, t_ts, t_ts, t_as,  wll,  wll,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, m_ds,   sp,  t_f,   sp, m_ds,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll, i_ri,  wll,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  wll, t_df,  wll,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_a,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll, m_ds,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp, t_as,  wll,  wll,  wll, t_as,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_sp,   sp,  wll,  wll,  m_b,  wll,  wll,   sp, m_sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, m_gs,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll,  wll,  wll,  m_a,  wll,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  m_b,  wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  m_w,   sp,   sp,  wll,  i_b,  wll,   sp,   sp,  m_w,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,  wll,  wll,  wll,  m_b,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::rightEdgeChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp, t_as, t_as,  wll,  unb},
		{ wll,   sp,  wll,  wll,   sp,  m_g, m_ds,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  wll, m_br, m_ds,  wll,   sp,  m_r,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  m_w,  wll, m_ds,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{  sp,   sp, t_lv,   sp,   sp,  m_k,  m_w,  m_k,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{  sp,   sp, m_sp,  wll,   sp, t_as, m_sp, t_as,   sp,   sp,   sp, m_ds,  wll,  unb},
		{ wll,  wll, t_ts,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  t_s,   sp,   sp, m_ds,  wll,  m_k,   sp,   sp,  t_s,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, t_ts, t_ts, t_ts, t_ts,  wll,  t_s, t_ts, t_ts, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::BLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   {  unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,   sp,  wll,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,   sp,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll, t_tr, t_lv,   sp,   sp,   sp,  m_z,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,  wll,  m_z,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_k, t_df,  m_k,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp, t_as,   sp,  wll,  wll,  wll,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  wll,  _dv},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_ar,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  i_b,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_z,  wll,  m_z,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);
}
void ThirdFloor::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_sp,  wll, m_sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  m_w,  wll,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_ri,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = { { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 {  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  m_r,  unb},
		{ wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  m_r,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  m_r,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_r,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{  sp,   sp,   sp,  t_f, t_lv, t_as, t_lv, t_as, t_lv,  t_f,   sp,   sp,   sp,  _dv},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,  m_b, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{  sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv, t_tl,  wll,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  unb, i_lp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  unb,  wll,  wll,  wll, t_df,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,  wll,  wll,  wll,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  wll, i_ar,  wll, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_w,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_w,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);

	//return combineChunks(c);
}
std::vector<std::string> ThirdFloor::BRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp, i_gd,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll, i_ri,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	//pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

std::vector<std::vector<std::string>> ThirdFloor::playerChunks() {

	std::vector<std::vector<std::string>> TLplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ply,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ply,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLplayerchunk =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp, _ply,   sp,   sp,   sp, t_as,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, t_as, _ply, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_playerchunk) {
	case 1: return TLplayerchunk;
	case 2: return TRplayerchunk;
	case 3: return BLplayerchunk;
	case 4: return BRplayerchunk;
	default: return TLplayerchunk;
	}
}
std::vector<std::vector<std::string>> ThirdFloor::exitChunks() {

	std::vector<std::vector<std::string>> TLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ext,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_rc,   sp,  wll,  wll,   sp, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_as, t_as, _ext, t_as, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_exitchunk) {
	case 1: return TLexitchunk;
	case 2: return TRexitchunk;
	case 3: return BLexitchunk;
	case 4: return BRexitchunk;
	default: return TLexitchunk;
	}
}

void ThirdFloor::mixChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	for (int i = 1; i < (getCols() - 2) / 13 - 1; i++) {
		current.push_back(pool[randInt(0, (int)pool.size() - 1)]);
	}
}
void ThirdFloor::pickSingleChunk(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	// selects one random corner chunk and pushes it
	if (m_layer == m_playerchunk && !m_playerplaced) {
		current.push_back(playerChunks());
		m_playerplaced = true;
	}
	else if (m_layer == m_exitchunk && !m_exitplaced) {
		current.push_back(exitChunks());
		m_exitplaced = true;
	}
	else
		current.push_back(pool[randInt(0, (int)pool.size() - 1)]);
}

std::vector<std::string> ThirdFloor::generateLevel() {
	// selects layer for the player and exit chunks to be on
	m_layer = 1;
	m_playerchunk = randInt(1, 4);
	m_playerplaced = m_exitplaced = false;

	switch (m_playerchunk) {
	case 1: m_exitchunk = 4; break;
	case 2: m_exitchunk = 3; break;
	case 3: m_exitchunk = 2; break;
	case 4: m_exitchunk = 1; break;
	default: break;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<std::string>>> c, pool;
	std::vector<std::string> v, finalvec;

	int count = 0;
	while (count < (getRows() - 2) / 9) {
		if (count == 0) {
			TLChunks(c, pool); pool.clear(); m_layer++;
			topChunks(c, pool); pool.clear();
			v = TRChunks(c, pool); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool); pool.clear();
			middleChunks(c, pool); pool.clear();
			v = rightEdgeChunks(c, pool); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool); pool.clear(); m_layer++;
			bottomChunks(c, pool); pool.clear();
			v = BRChunks(c, pool); pool.clear();
		}

		for (unsigned int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		c.clear();
		count++;
	}

	return finalvec;
}
std::vector<std::string> ThirdFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 10; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < (int)c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 14; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}



FourthFloor::FourthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FOURTH_FLOOR, 0, 0) {

}

FifthFloor::FifthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIFTH_FLOOR, 0, 0) {

}

SixthFloor::SixthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SIXTH_FLOOR, 0, 0) {

}

SeventhFloor::SeventhFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SEVENTH_FLOOR, 0, 0) {

}

EighthFloor::EighthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, EIGHTH_FLOOR, 0, 0) {

}

NinthFloor::NinthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, NINTH_FLOOR, 0, 0) {

}



// ================================================
//				:::: BOSS FLOOR ::::
// ================================================
FirstBoss::FirstBoss(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIRST_BOSS, BOSSROWS, BOSSCOLS) {
	int rows = getRows();
	int cols = getCols();

	//	spawn player
	player[0]->setPosX(cols / 2);
	player[0]->setPosY(rows - 3);
	tileAt(p->getPos()).hero = true;


	//	spawn boss
	addMonster(std::make_shared<Smasher>(this));


	//	spawn spike traps
	int x, y, speed;
	int m = 12 + randInt(0, 5);

	while (m > 0) {
		x = randInt(1, cols - 2);
		y = rows / 2 + randInt(0, rows / 2 - 1);
		speed = 3 + randInt(0, 2);

		while (trap(x, y)) {
			x = randInt(1, cols - 2);
			y = rows / 2 + randInt(0, rows / 2 - 1);
			speed = 3 + randInt(0, 2);
		}

		addTrap(std::make_shared<SpikeTrap>(*this, x, y, speed));
		m--;
	}
}

=======
#include "cocos2d.h"
#include "AudioEngine.h"
#include "GUI.h"
#include "global.h"
#include "EffectSprite.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "Afflictions.h"
#include "FX.h"
#include <iostream>
#include <memory>
#include <unordered_set>


Dungeon::Dungeon(LevelScene* scene, std::shared_ptr<Player> p, int level, int rows, int cols) : m_scene(scene), m_level(level), m_rows(rows), m_cols(cols) {
	player.push_back(p);
	player[0]->setDungeon(this);
	if (p->hasDelayedHealing()) {
		p->increaseStatBy(StatType::HP, p->getMaxHP() * p->getDelayedHealing());
		p->setDelayedHealing(0);
	}

	initializeTiles();
	NPCTable::initializeNPCTable();
}
_Tile& Dungeon::tileAt(const Coords &pos) {
	return m_maze[pos.y*m_cols + pos.x];
}
const _Tile& Dungeon::tileAt(const Coords &pos) const {
	return m_maze[pos.y*m_cols + pos.x];
}
Dungeon::~Dungeon() {
	player[0].reset();
	m_exit.reset();

	for (auto &it : m_playerClones)
		it.reset();

	for (auto &it : m_npcs)
		it.reset();

	for (auto &it : m_monsters)
		it.reset();

	for (auto &it : m_traps)
		it.reset();

	for (auto &it : m_items)
		it.reset();

	for (auto &it : m_money)
		it.reset();

	for (auto &it : m_doors)
		it.reset();

	for (auto &it : m_walls)
		it.reset();

	for (auto &it : m_decoys)
		it.reset();

	for (auto &it : m_miscSprites)
		if (it.sprite)
			it.sprite->removeFromParent();
}

void Dungeon::peekDungeon(char move) {
	bool usedMove = false;

	// Move player, unless they have the `Slow` attribute
	if (!player[0]->isSlow()) {
		usedMove = true;
		movePlayers(move);
	}

	// Check these if player does not have Matrix Vision
	if (!player[0]->hasMatrixVision()) {
		checkActive();
		checkDoors();
		checkMonsters();
	}
	// If they do have it, then roll to see if these get skipped
	else if (player[0]->hasMatrixVision()) {

		// Ability was finished and has finished cooling down
		if (matrixTurns == 0 && matrixCooldown == 0) {
			// Failed the roll
			if (randReal(1, 100) <= 95) {
				checkActive();
				checkDoors();
				checkMonsters();
			}
			else {
				matrixTurns = 3;
			}
		}
		// Ability is still active
		else if (matrixTurns > 0) {
			matrixTurns--;

			// Begin cooldown
			if (matrixTurns == 0)
				matrixCooldown = 18;
		}
		// Ability is not off cooldown
		else if (matrixTurns == 0 && matrixCooldown > 0) {
			matrixCooldown--;

			checkActive();
			checkDoors();
			checkMonsters();
		}
	}

	// Player moves after the monsters if they are Slow
	if (player[0]->isSlow() && !usedMove)
		movePlayers(move);

	updateLighting();

	// do any special actions unique to the floor
	specialActions();

	runSpawn();
}

void Dungeon::assignQuickItem(int index) {
	player[0]->setItemToFront(index);

	for (auto &it : m_playerClones)
		it->setItemToFront(index);
}
void Dungeon::callUse(int index) {
	player[0]->use(index);

	for (auto &it : m_playerClones)
		it->use(index);
}

void Dungeon::movePlayers(char move) {
	player[0]->move(move);

	if (!m_playerClones.empty()) {
		for (size_t i = 0; i < m_playerClones.size(); ++i)
			m_playerClones[i]->move(move);
	}
}
void Dungeon::checkActive() {
	for (int i = 0; i < static_cast<int>(m_traps.size()); ++i) {
		if (!m_traps.at(i)->isDestroyed())
			m_traps.at(i)->activeTrapAction();

		if (m_traps.at(i)->isDestroyed())
			insertTrapForRemoval(i);
	}

	trapRemoval();
}
void Dungeon::checkMonsters() {	
	if (m_monsters.empty())
		return;
	
	int mInitHP;
	Coords coords;
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		mInitHP = m_monsters.at(i)->getHP();
		coords = m_monsters.at(i)->getPos();

		// Check if monster is dead or was destroyed earlier
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed()) {
			insertActorForRemoval(i);
			continue;
		}

		m_monsters.at(i)->checkAfflictions();

		if (m_monsters.at(i)->getHP() < mInitHP)
			runMonsterDamageBlink(m_monsters.at(i)->getSprite());
		
		if (!(m_monsters.at(i)->isSpirit() || isBossLevel()) && !positionInRectangularRange(getPlayerCoordsClosestTo(coords), coords, 25, 25))
			continue;

		m_monsters.at(i)->moveCheck();

		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed()) {
			insertActorForRemoval(i);
			continue;
		}

		if (player[0]->isDead()) {
			player[0]->setCauseOfDeath(m_monsters.at(i)->getName());
			break;
		}
	}

	monsterTrapEncounter();
}

int Dungeon::findDecoy(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_decoys.size()); ++i)
		if (m_decoys.at(i)->getPos() == target)
			return i;

	return -1;
}
bool Dungeon::trickedByDecoy(const Coords &start, Coords &newTarget) {
	if (m_decoys.empty())
		return false;

	for (int i = 0; i < static_cast<int>(m_decoys.size()); ++i) {
		Coords currentPos = m_decoys[i]->getPos();

		// If monster is in range of the decoy, chase it
		if (positionInRange(start, currentPos, m_decoys[i]->getAttractRange())) {
			newTarget = currentPos;
			return true;
		}
	}

	return false;
}

void Dungeon::addDoor(std::shared_ptr<Door> door) {
	m_doors.push_back(door);
	m_walls.push_back(door);
	tileAt(door->getPos()).wallObject = door;
}
int Dungeon::findDoor(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_doors.size()); ++i)
		if (m_doors.at(i)->getPos() == target)
			return i;
		
	return -1;
}
void Dungeon::checkDoors() {
	if (!m_doors.empty()) {
		int x, y;

		for (size_t i = 0; i < m_doors.size(); ++i) {
			std::shared_ptr<Door> door = m_doors.at(i);
			x = door->getPosX();
			y = door->getPosY();

			if (door->isOpen() && !door->isHeld() && !(enemy(x, y) || hero(x, y))) {
				playSound("Door_Closed.mp3");

				door->getSprite()->setVisible(true);

				// Close the door
				door->toggleOpen();
				door->toggleHeld();
				tileAt(Coords(x, y)).wall = true;
			}
			else if (door->isOpen() && door->isHeld()) {			
				door->toggleHeld(); // Release hold on the door
			}

			door.reset();
		}
	}
}

int Dungeon::findWall(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_walls.size()); ++i)
		if (m_walls.at(i)->getPos() == target)
			return i;
		
	return -1;
}
void Dungeon::destroyWall(int x, int y) {
	Coords pos(x, y);

	if (!tileAt(pos).wallObject)
		return;

	if (tileAt(pos).wallObject->isDestructible())
		tileAt(pos).wallObject->destroy();
}
void Dungeon::removeWall(int x, int y) {
	int pos = findWall(x, y);

	if (pos == -1)
		return;

	Coords coords(x, y);

	if (m_walls.at(pos)->getName() == UNB_WALL) {
		queueRemoveSprite(m_walls.at(pos)->getSprite());
		tileAt(coords).wall = false;
		tileAt(coords).wallObject.reset();
		tileAt(coords).wallObject = nullptr;
	}

	if (m_walls.at(pos)->isDoor()) {
		int index = findDoor(x, y);
		if (index != -1)
			m_doors.erase(m_doors.begin() + index);
	}

	m_walls.at(pos).reset();
	m_walls.erase(m_walls.begin() + pos);
}

int Dungeon::findNPC(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_npcs.size()); ++i) {
		if (m_npcs.at(i)->getPos() == target)
			return i;		
	}

	return -1;
}
void Dungeon::interactWithNPC(int x, int y) {
	int pos = findNPC(x, y);

	if (pos != -1)
		m_npcs.at(pos)->talk();
}
void Dungeon::playNPCDialogue(const NPC &npc) {
	m_scene->callNPCInteraction(npc);
}
void Dungeon::provokeNPC(const Coords &pos) {
	int index = findNPC(pos.x, pos.y);

	if (index == -1)
		return;

	m_npcs[index]->provoke();
}

//		Trap handling
void Dungeon::trapRemoval() {
	if (m_trapIndexes.empty())
		return;

	// The index set is ordered from smallest to largest, so we destroy in reverse order
	int index;
	for (auto it = m_trapIndexes.crbegin(); it != m_trapIndexes.crend(); it++) {
		if (*it != -1) {
			index = *it;
			
			// If this is a decoy, remove it from the decoy vector too
			if (m_traps.at(index)->isDecoy()) {
				int pos = findDecoy(m_traps.at(index)->getPosX(), m_traps.at(index)->getPosY());
				if (pos != -1)
					m_decoys.erase(m_decoys.begin() + pos);
			}

			m_traps.erase(m_traps.begin() + index);
		}
	}

	m_trapIndexes.clear();
}
void Dungeon::insertTrapForRemoval(int index) {
	m_trapIndexes.insert(index); // queue trap for removal
}
std::vector<int> Dungeon::findTraps(int x, int y) const {
	std::vector<int> traps;
	const Coords target(x, y);

	for (int i = 0; i < static_cast<int>(m_traps.size()); ++i)
		if (m_traps.at(i)->getPos() == target)
			traps.push_back(i);

	return traps;
}
int Dungeon::countTrapNumber(int x, int y) const {
	int count = 0;
	const Coords target(x, y);
	for (size_t i = 0; i < m_traps.size(); ++i) {
		if (m_traps[i]->getPos() == target)
			count++;
	}
	
	return count;
}
void Dungeon::trapEncounter(int x, int y) {
	std::vector<int> indexes = findTraps(x, y);
	for (size_t i = 0; i < indexes.size(); ++i) {
		m_traps.at(indexes[i])->trapActionCheck(*getPlayerAt(x, y));

		if (m_traps.at(indexes[i])->isDestroyed())
			insertTrapForRemoval(indexes[i]);
	}
}
void Dungeon::monsterTrapEncounter() {
	int x, y, mInitHP;
	
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {

		// Ignore if they're dead
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed())
			continue;

		// Ignore monster segments and those that are underground
		if (m_monsters.at(i)->isMonsterSegment() || m_monsters.at(i)->isUnderground())
			continue;

		mInitHP = m_monsters.at(i)->getHP();
		x = m_monsters.at(i)->getPosX();
		y = m_monsters.at(i)->getPosY();

		if (trap(x, y)) {
			std::vector<int> indexes = findTraps(x, y);
			for (size_t idx = 0; idx < indexes.size(); ++idx) {
				m_traps.at(indexes[idx])->trapActionCheck(*m_monsters.at(i));

				if (m_traps.at(indexes[idx])->isDestroyed())
					insertTrapForRemoval(indexes[idx]);
			}
		}

		if (m_monsters.at(i)->getHP() < mInitHP)
			runMonsterDamage(m_monsters.at(i)->getSprite());
		
		if (m_monsters.at(i)->isDead() || m_monsters.at(i)->wasDestroyed())
			insertActorForRemoval(i);	
	}

	actorRemoval();
	trapRemoval();
}
void Dungeon::singleMonsterTrapEncounter(int pos) {
	if (pos == -1)
		return;

	// Ignore monster segments
	if (m_monsters.at(pos)->isMonsterSegment())
		return;

	int x, y, mInitHP;
	
	mInitHP = m_monsters.at(pos)->getHP();
	x = m_monsters.at(pos)->getPosX();
	y = m_monsters.at(pos)->getPosY();

	if (trap(x, y)) {
		std::vector<int> indexes = findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			m_traps.at(indexes[i])->trapActionCheck(*m_monsters.at(pos));	
	}

	// Flash monster sprite if damage was taken
	if (m_monsters.at(pos)->getHP() < mInitHP)
		runMonsterDamage(m_monsters.at(pos)->getSprite());
}

void Dungeon::damagePlayer(int damage, DamageType type) {
	if (damage <= 0)
		return;

	player[0]->takeDamage(damage, type);
	damageClones(damage, type);
}
void Dungeon::damageClones(int damage, DamageType type) {
	if (!m_playerClones.empty()) {
		for (int i = m_playerClones.size() - 1; i >= 0; --i)
			m_playerClones[i]->takeDamage(damage, type);		
	}
}
void Dungeon::actorRemoval() {
	if (!m_playerClones.empty()) {
		for (int i = m_playerClones.size() - 1; i >= 0; --i) {
			if (m_playerClones[i]->isDead()) {
				m_playerClones[i]->death();
				m_playerClones[i].reset();
				m_playerClones.erase(m_playerClones.begin() + i);
			}
		}
	}

	if (m_deadActorIndexes.empty())
		return;

	int index;
	for (auto it = m_deadActorIndexes.crbegin(); it != m_deadActorIndexes.crend(); it++) {
		if (*it != -1) {
			index = *it;
			monsterDeath(index);
		}
	}

	m_deadActorIndexes.clear();
}
void Dungeon::insertActorForRemoval(int index) {
	m_deadActorIndexes.insert(index);
}
int Dungeon::findMonster(int x, int y) const {
	return findMonster(Coords(x, y));
}
int Dungeon::findMonster(const Coords &pos) const {
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		if (m_monsters.at(i)->getPos() == pos) {
			if (m_monsters.at(i)->isUnderground())
				continue;

			return i;
		}
	}

	return -1;
}
int Dungeon::findUndergroundMonster(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_monsters.size()); ++i) {
		if (m_monsters.at(i)->getPos() == target) {
			if (!m_monsters.at(i)->isUnderground())
				continue;

			return i;
		}
	}

	return -1;
}
int Dungeon::findSegmentedMonster(int index) const {
	int x, y;

	x = m_monsters.at(index)->getParentX();
	y = m_monsters.at(index)->getParentY();

	return findMonster(x, y);
}
void Dungeon::damageMonster(const Coords &pos, int damage, DamageType type) {
	damageMonster(findMonster(pos), damage, type);
}
void Dungeon::damageMonster(int index, int damage, DamageType type) {
	if (index == -1)
		return;

	if (!m_monsters.at(index)->canBeDamaged(type)) {
		//player[0]->botchedAttack(*m_monsters.at(index));

		return;
	}

	if (m_monsters.at(index)->isMonsterSegment()) {
		index = findSegmentedMonster(index);
		if (index == -1)
			return;
	}
	
	m_monsters.at(index)->decreaseStatBy(StatType::HP, damage);

	/// Potentially call reactToDamage here later.
	//

	if (player[0]->hasExperienceGain() && m_monsters.at(index)->isDead())
		player[0]->increaseXPBy(1);

	runMonsterDamage(m_monsters.at(index)->getSprite());
}
void Dungeon::giveAffliction(int index, std::shared_ptr<Afflictions> affliction) {
	if (m_monsters.at(index)->isMonsterSegment()) {
		index = findSegmentedMonster(index);
		if (index == -1)
			return;
	}

	m_monsters.at(index)->addAffliction(affliction);
}
void Dungeon::monsterDeath(int pos) {
	m_monsters.at(pos)->death();
	m_monsters.erase(m_monsters.begin() + pos);
}

//		Actor pushing
bool Dungeon::monsterHash(int &x, int &y, bool &switched, char move, bool strict) {
	// Let starting x and y be the monster's initial position

	// if current tile is valid, return to set monster here
	if (!(wall(x, y) || enemy(x, y) || hero(x, y)))
		return true;

	switch (move) {
		// 1. if next tile is a wall then switch direction (one time)

		// 2. else if a switch has already been made and a wall is hit, return false

		// 3. otherwise keep looking in the same path

	case 'l':
		if (!strict && wall(x - 1, y) && !switched) {
			if (wall(x, y - 1) && wall(x, y + 1))
				return false;
			
			else if (wall(x, y - 1) && !wall(x, y + 1))
				move = 'd';
			
			else if (wall(x, y + 1) && !wall(x, y - 1))
				move = 'u';
			
			else
				move = (randInt(0, 1) == 0 ? 'u' : 'd');
			
			switched = true;
		}
		else if (wall(x - 1, y) && (switched || strict))
			return false;
		
		break;
	case 'r':
		if (!strict && wall(x + 1, y) && !switched) {
			if (wall(x, y - 1) && wall(x, y + 1))
				return false;
			
			else if (wall(x, y - 1) && !wall(x, y + 1))
				move = 'd';
			
			else if (wall(x, y + 1) && !wall(x, y - 1))
				move = 'u';
			
			else
				move = (randInt(0, 1) == 0 ? 'u' : 'd');
			
			switched = true;
		}
		else if (wall(x + 1, y) && (switched || strict))
			return false;
		
		break;
	case 'u':
		if (!strict && wall(x, y - 1) && !switched) {
			if (wall(x - 1, y) && wall(x + 1, y))
				return false;
			
			else if (wall(x - 1, y) && !wall(x + 1, y))
				move = 'r';
			
			else if (wall(x + 1, y) && !wall(x - 1, y))
				move = 'l';
			
			else
				move = (randInt(0, 1) == 0 ? 'l' : 'r');
			
			switched = true;
		}
		else if (wall(x, y - 1) && (switched || strict))
			return false;
		
		break;
	case 'd':
		if (!strict && wall(x, y + 1) && !switched) {
			if (wall(x - 1, y) && wall(x + 1, y))
				return false;
			
			else if (wall(x - 1, y) && !wall(x + 1, y))
				move = 'r';
			
			else if (wall(x + 1, y) && !wall(x - 1, y))
				move = 'l';
			
			else
				move = (randInt(0, 1) == 0 ? 'l' : 'r');
			
			switched = true;
		}
		else if (wall(x, y + 1) && (switched || strict))
			return false;
		
		break;
	}

	if (move == 'l') return monsterHash(--x, y, switched, move);
	if (move == 'r') return monsterHash(++x, y, switched, move);
	if (move == 'u') return monsterHash(x, --y, switched, move);
	if (move == 'd') return monsterHash(x, ++y, switched, move);

	return false;
}
void Dungeon::pushMonster(int &mx, int &my, char move, int cx, int cy, bool strict) {
	bool b = false;
	int ox = mx;
	int oy = my;

	int pos = findMonster(mx, my);
	if (pos == -1)
		return;

	// If this is a monster segment, don't bother moving it--the parent will move it
	if (m_monsters.at(pos)->isMonsterSegment())
		return;

	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd': {
		if (monsterHash(mx, my, b, move, strict))
			m_monsters.at(pos)->moveTo(mx, my, 0.05f);
		
		// Kill the monster
		else
			m_monsters.at(pos)->setDead(true);
		
		break;
	}
			  //	CUSTOM MOVEMENT CASE in X direction
	case 'X': {
		m_monsters.at(pos)->moveTo(cx, my, 0.05f);
		break;
	}
			  //	CUSTOM MOVEMENT CASE in Y direction
	case 'Y': {
		m_monsters.at(pos)->moveTo(mx, cy, 0.05f);
		break;
	}
	}

}
void Dungeon::linearActorPush(int x, int y, int limit, int range, char move, bool pulling, bool lethal) {
	// x and y are the coordinates of the first actor to be pushed

	int ox = x, oy = y;

	std::stack<std::shared_ptr<Actors>> st;
	int pos = findMonster(x, y);
	if (pos != -1) {
		if (!m_monsters.at(pos)->isHeavy())
			st.push(m_monsters[pos]);
	}
	
	// Can be both because of Sand Centipede
	if (hero(x, y)) {
		if (!getPlayerAt(x, y)->isHeavy())
			st.push(getPlayerAt(x, y));
	}

	if (st.empty())
		return;

	int n, m;
	setDirectionalOffsets(move, n, m);

	// Find all actors we need to push
	int k = range;
	while (k > 0 + pulling) {

		switch (move) {
		case 'l': x--; break;
		case 'r': x++; break;
		case 'u': y--; break;
		case 'd': y++; break;
		case '1': x--; y--; break;
		case '2': x++; y--; break;
		case '3': x--; y++; break;
		case '4': x++; y++; break;
		}

		if (!withinBounds(x, y) || wall(x, y))
			break;

		if (enemy(x, y)) {
			int index = findMonster(x, y);

			if (index != -1) {
				if (m_monsters.at(index)->isHeavy())
					break;

				st.push(m_monsters.at(index));
			}
		}

		if (hero(x, y)) {

			// If the player is heavy, don't add them and stop
			if (getPlayer()->isHeavy())
				break;

			st.push(getPlayer());
		}

		k--;
	}

	// Now push the actors
	int ax = st.top()->getPosX(), ay = st.top()->getPosY();
	bool firstActor = true;

	// Calculates number of free spaces between first monster and last
	int lastActorPushDist = st.size() == 1 ? 0 : std::max(abs(ax - ox), abs(ay - oy)) - (st.size() - 2) - 1;
	while (!st.empty()) {
		std::shared_ptr<Actors> a = st.top();
		st.pop();

		ax = a->getPosX();
		ay = a->getPosY();
		Coords currentPos(a->getPos());
		int index = findMonster(currentPos);
		k = !firstActor ? limit : limit - lastActorPushDist;

		while (k > 0) {

			// Lethal flag indicates the actor should be killed if it was the last one against a wall
			if (wall(ax + n, ay + m) && lethal) {
				a->setHP(0);
				deathFade(a->getSprite());
				playSound("Bone_Crack1.mp3", getPlayer()->getPos(), a->getPos());

				if (a->isPlayer())
					tileAt(currentPos).hero = false;
				else {
					m_monsters[index]->setDestroyed(true);
					tileAt(currentPos).enemy = false;
				}

				break;
			}

			if (enemy(ax + n, ay + m) || wall(ax + n, ay + m) || hero(ax + n, ay + m))
				break;

			if (a->isPlayer())
				a->moveTo(ax + n, ay + m, 0.07f);
			else if (index != -1)
				m_monsters[index]->moveTo(ax + n, ay + m, 0.05f);

			if (trap(ax + n, ay + m)) {

				if (a->isPlayer()) {
					trapEncounter(ax + n, ay + m);

					if (a->isDead())
						break;
				}
				else if (index != -1) {
					singleMonsterTrapEncounter(index);

					if (m_monsters.at(index)->isDead())
						break;
				}
			}

			ax += n;
			ay += m;

			k--;
		}

		if (firstActor)
			firstActor = false;
	}

}

//		Items
void Dungeon::createItem(std::shared_ptr<Objects> item) {
	Coords pos = item->getPos();

	tileAt(pos).object = item;
	tileAt(pos).object->setSprite(createSprite(pos, pos.y + Z_ITEM, item->getImageName()));
	tileAt(pos).item = true;

	addItem(item);
}
void Dungeon::createChest(std::shared_ptr<Chests> chest) {
	Coords pos = chest->getPos();
	tileAt(pos).object = chest;
	addItem(chest);
}
int Dungeon::findItem(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
		if (m_items.at(i)->getPos() == target)
			return i;

	return -1;
}
void Dungeon::removeItem(int x, int y) {
	int pos = findItem(x, y);
	
	if (pos != -1) {
		removeSprite(m_items.at(pos)->getSprite());
		m_items.erase(m_items.begin() + pos);
	}
	
	Coords target(x, y);
	tileAt(target).item = false;
	
	if (tileAt(target).object)
		tileAt(target).object.reset();

	tileAt(target).object = nullptr;
}
void Dungeon::removeItemWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	int pos = findItem(x, y);

	if (pos != -1) {
		queueRemoveSpriteWithMultipleActions(m_items.at(pos)->getSprite(), actions);
		m_items.erase(m_items.begin() + pos);
	}

	Coords target(x, y);

	tileAt(target).item = false;
	tileAt(target).object.reset();
	tileAt(target).object = nullptr;
}
void Dungeon::itemHash(int &x, int &y) {
	// if current tile has no item and the position is valid, return to set item here
	if (!(itemObject(x, y) || wall(x, y) || trap(x, y)))
		return;

	int n = randInt(-1, 1);
	int m = randInt(-1, 1);

	// while hash draws out of bounds, find a new position
	while (!withinBounds(x + n, y + m) || wallObject(x + n, y + m)) {
		n = randInt(-1, 1);
		m = randInt(-1, 1);
	}

	// hash until we find a valid spot
	itemHash(x += n, y += m);
}
void Dungeon::collectItem(int x, int y) {
	bool itemAdded = false;

	std::shared_ptr<Objects> object = itemObject(x, y);

	if (exit(x, y)) {
		m_exit->interact();
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		playSound("Player_Movement.mp3");

		return;
	}

	// Beat the 'final' boss
	if (object->getName() == "Idol") {
		player[0]->setWin(true);
		return;
	}

	if (object->isAutoUsable()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->useItem(*this, *getPlayerAt(x, y));

		playSound(drop->getSoundName());

		drop.reset();

		removeItem(x, y);
	}
	else if (object->isUsable()) {

		// For Spellcaster
		if (object->isSpell() && player[0]->getName() == SPELLCASTER) {
			playSound(object->getSoundName());

			player[0]->equipActiveItem(object);
			removeItem(x, y);

			return;
		}

		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		player[0]->addItem(drop, itemAdded);
		drop.reset();
	}
	else if (object->isPassive()) {
		std::shared_ptr<Passive> passive = std::dynamic_pointer_cast<Passive>(object);
		player[0]->equipPassive(passive);
		passive.reset();

		itemAdded = true;
	}
	else if (object->isWeapon()) {
		std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
		player[0]->storeWeapon(weapon);
		playSound(weapon->getSoundName());

		weapon.reset();
	}
	else if (object->isShield()) {
		if (player[0]->canUseShield())
			player[0]->equipActiveItem(object);		
	}

	if (itemAdded) {
		removeItem(x, y);
	}
}
int Dungeon::findGold(int x, int y) const {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_money.size()); ++i)
		if (m_money.at(i)->getPos() == target)
			return i;

	return -1;
}
void Dungeon::goldPickup(int x, int y) {
	playSound("Gold_Pickup2.mp3");

	player[0]->setMoney(player[0]->getMoney() + gold(x, y));

	removeGold(x, y);
}
void Dungeon::addGold(int x, int y, int amount) {
	tileAt(Coords(x, y)).gold += amount;
	int gold = Dungeon::gold(x, y);

	if (gold == 0)
		return;

	std::string image;
	if (gold == 1) image = "Gold_Coin1_48x48.png";
	else if (gold == 2) image = "Gold_Coin2_48x48.png";
	else if (gold == 3) image = "Gold_Coin3_48x48.png";
	else if (gold >= 4 && gold <= 10) image = "Gold_Pile1_48x48.png";
	else if (gold > 10) image = "Gold_Pile2_48x48.png";

	// if there's gold here already, remove the sprite before we add a new one
	
	int pos = findGold(x, y);
	if (pos != -1) {
		removeSprite(m_money.at(pos)->getSprite());
		m_money.at(pos)->setSprite(createSprite(x, y, y + Z_MONEY, image));
		return;
	}
	
	m_money.push_back(std::make_shared<Gold>(*this, x, y, image));
}
void Dungeon::removeGold(int x, int y) {
	tileAt(Coords(x, y)).gold = 0;

	int pos = findGold(x, y);
	if (pos != -1) {
		removeSprite(m_money.at(pos)->getSprite());
		m_money.at(pos).reset();
		m_money.erase(m_money.begin() + pos);
	}
}
void Dungeon::removeGoldWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	tileAt(Coords(x, y)).gold = 0;

	int pos = findGold(x, y);

	if (pos != -1) {
		queueRemoveSpriteWithMultipleActions(m_money.at(pos)->getSprite(), actions);
		m_money.at(pos).reset();
		m_money.erase(m_money.begin() + pos);
	}
}

//		Helper
std::shared_ptr<Player> Dungeon::getPlayerAt(int x, int y) const {
	return getPlayerAt(Coords(x, y));
}
std::shared_ptr<Player> Dungeon::getPlayerAt(const Coords &pos) const {
	if (player[0]->getPos() == pos)
		return player[0];

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		if (m_playerClones[i]->getPos() == pos)
			return m_playerClones[i];
	}

	// If all else fails, return the real player for safety
	return player[0];
}
std::vector<Coords> Dungeon::getCoordsOfAllPlayers() const {
	std::vector<Coords> coords;
	coords.push_back(player[0]->getPos());

	for (size_t i = 0; i < m_playerClones.size(); ++i)
		coords.push_back(m_playerClones[i]->getPos());	

	return coords;
}
Coords Dungeon::getPlayerCoordsClosestTo(const Coords &start) const {
	Coords end(player[0]->getPos());

	if (m_playerClones.empty())
		return end;

	int minDistance = calculateDistanceBetween(start, end);

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		Coords other = m_playerClones[i]->getPos();

		int currentDistance = calculateDistanceBetween(start, other);

		if (currentDistance < minDistance) {
			end = other;
			minDistance = currentDistance;
		}
		else if (currentDistance == minDistance && randReal(1, 100) > 50)
			end = other; // Allows for nondeterministic behavior
	}

	return end;
}
Coords Dungeon::findNearbyFreeTile(const Coords &start) const {
	int x = start.x;
	int y = start.y;
	std::vector<Coords> coords;

	while (coords.empty()) {
		if (!withinBounds(x, y)) {
			x += randInt(-1, 1);
			y += randInt(-1, 1);
			continue;
		}

		if (!tileIsBlocked(x - 1, y))
			coords.push_back(Coords(x - 1, y));

		if (!tileIsBlocked(x + 1, y))
			coords.push_back(Coords(x + 1, y));

		if (!tileIsBlocked(x, y - 1))
			coords.push_back(Coords(x, y - 1));

		if (!tileIsBlocked(x, y + 1))
			coords.push_back(Coords(x, y + 1));

		if (!coords.empty())
			break;

		x += randInt(-1, 1);
		y += randInt(-1, 1);
	}

	int index = randInt(0, static_cast<int>(coords.size()) - 1);
	return coords[index];
}
bool Dungeon::tileIsBlocked(int x, int y) const {
	return (wall(x, y) || enemy(x, y) || hero(x, y));
}
bool Dungeon::withinBounds(int x, int y) const {
	if (x < 0 || x > m_cols - 1 || y < 0 || y > m_rows - 1)
		return false;

	return true;
}

//		Other
void Dungeon::factoryTileCreation() {
	m_scene->callFactoryTileCreation();
}
void Dungeon::clonePlayer() {
	std::shared_ptr<Player> newClone = player[0]->clone();

	// Clones have half the HP of the original player.
	// The player also has their HP reduced.
	newClone->setHP(std::max(1, player[0]->getHP() / 2));
	player[0]->setHP(std::max(1, (player[0]->getHP() * 3) / 4));

	newClone->setSprite(createSprite(player[0]->getPos(), getHalfTileYOffset(), player[0]->getPosY() + Z_ACTOR, player[0]->getImageName()));

	Coords newPos = findNearbyFreeTile(player[0]->getPos());

	newClone->setPos(newPos);
	tileAt(newPos).hero = true;
	queueMoveSprite(newClone->getSprite(), newPos, getHalfTileYOffset());
	newClone->getSprite()->setLocalZOrder(newPos.y + Z_ACTOR);

	m_playerClones.push_back(newClone);
}
void Dungeon::chainLightning(const Coords &start) {
	std::unordered_set<Coords> coordsSeen;

	// Key: Actor's coordinates, Value: Index in the monster vector
	std::stack<std::pair<Coords, int>> st;

	Coords currentPos = start;
	int index = findMonster(currentPos);
	st.push(std::make_pair(currentPos, index));

	int radius = 1;
	while (!st.empty()) {
		currentPos = st.top().first;
		index = st.top().second;
		st.pop();

		for (int i = currentPos.x - 1; i < currentPos.x + 2; ++i) {
			for (int j = currentPos.y - 1; j < currentPos.y + 2; ++j) {
				Coords pos(i, j);

				if (coordsSeen.find(pos) != coordsSeen.end())
					continue;

				coordsSeen.insert(pos);

				// Add any adjacent enemies onto the stack
				if (enemy(i, j))
					st.push(std::make_pair(pos, findMonster(pos)));
			}
		}

		// Now damage the current actor
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		runSingleAnimation(frames, 120, currentPos, currentPos.y + Z_PROJECTILE);

		damageMonster(index, getPlayer()->getInt() * 1.5, DamageType::LIGHTNING);
	}
}

//		Shop prices
void Dungeon::constructShopHUD(int x, int y) {
	return;
}
void Dungeon::deconstructShopHUD() {
	return;
}

//		*** Fill level ***
void Dungeon::initializeTiles() {
	int rows = m_rows;
	int cols = m_cols;

	_Tile *tile;
	int i, j;

	//	Initialize entire dungeon to blank space
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			tile = new _Tile;
			tile->upper = SPACE;
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = tile->spirit = false;
			tile->gold = 0;
			tile->marked = tile->noSpawn = false;
			tile->shop_action = "";
			tile->object = nullptr;
			tile->wallObject = nullptr;

			m_maze.push_back(*tile);
			delete tile;

			createFlooring(j, i);
		}
	}

	//	Initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++)
			m_walls.push_back(std::make_shared<Boundary>(this, j, i));
		
		i += (rows - 2);
	}

	//	Initialize edges of dungeon to be unbreakable walls
	for (i = 0; i < rows - 1; i++) {
		m_walls.push_back(std::make_shared<Boundary>(this, 0, i));
		m_walls.push_back(std::make_shared<Boundary>(this, cols - 1, i));
	}
}
void Dungeon::createFlooring(int x, int y) {
	Coords pos(x, y);
	std::string image;

	switch (randInt(1, 8)) {
	case 1: image = "FloorTile1_48x48.png"; break;
	case 2: image = "FloorTile2_48x48.png"; break;
	case 3: image = "FloorTile3_48x48.png"; break;
	case 4: image = "FloorTile4_48x48.png"; break;
	case 5: image = "FloorTile5_48x48.png"; break;
	case 6: image = "FloorTile6_48x48.png"; break;
	case 7: image = "FloorTile7_48x48.png"; break;
	case 8: image = "FloorTile8_48x48.png"; break;
	}

	cocos2d::Sprite* floor = createSprite(pos, Z_FLOOR, image);
	floor->setOpacity(252);

	if (m_level != FIRST_BOSS)
		floor->setColor(cocos2d::Color3B(0, 0, 0));

	tileAt(pos).floor = floor;
}

void Dungeon::generateNPCs() {
	int cols = m_cols;
	int rows = m_rows;

	int x, y;

	// BEGIN NPC GENERATION
	std::shared_ptr<NPC> npc(nullptr);
	int spawnChance = 25 + (player[0]->hasCharismaNPC() ? 25 : 0) + (player[0]->getFavor() <= -12 ? -15 : 0);

	for (int i = 0; i < 2; i++) {

		double roll = randReal(1, 100);
		if (roll < spawnChance) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || trap(x, y) || item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			npc = rollNPC(this, x, y);
			if (!npc)
				break;

			m_npcs.push_back(npc);
			npc.reset();

			spawnChance /= 2; // Halve the spawn chance every time an npc is spawned
		}
	}
	/// END NPC GENERATION
}
void Dungeon::generateMonsters() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	// BEGIN MONSTER GENERATION
	std::shared_ptr<Monster> monster(nullptr);
	std::string name;

	for (int i = 0; i < 17; i++) {

		switch (i) {
		case 0: n = 10 + randInt(0, 2); name = SEEKER; break;
		case 1: n = 8 + randInt(0, 2); name = GOO_SACK; break;
		case 2: n = 8 + randInt(0, 3); name = ROUNDABOUT; break;
		case 3: n = 5 + randInt(0, 3); name = BROUNDABOUT; break;
		case 4: n = 6 + randInt(0, 3); name = RAT; break;
		case 5: n = 3 + randInt(0, 3); name = SPIDER; break;
		case 6: n = 3 + randInt(0, 3); name = SHOOTING_SPIDER; break;
		case 7: n = 3 + randInt(0, 3); name = POUNCING_SPIDER; break;
		case 8: n = 3 + randInt(0, 3); name = GOBLIN; break;
		case 9: n = 10 + randInt(0, 3); name = WANDERER; break;
		case 10: n = 6 + randInt(0, 3); name = SLEEPING_WANDERER; break;
		case 11: n = 6 + randInt(0, 3); name = PROVOCABLE_WANDERER; break;
		case 12: n = 0 + randInt(0, 3); name = RABID_WANDERER; break;
		case 13: n = 0 + randInt(0, 3); name = TOAD; break;
		case 14: n = 3 + randInt(0, 3); name = WEAK_CRATE; break;
		case 15: n = 3 + randInt(0, 3); name = WEAK_BARREL; break;
		case 16: n = 3 + randInt(0, 3); name = WEAK_LARGE_POT; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			addMonster(createMonsterByName(name, this, x, y));

			n--;
		}
	}
	/// END MONSTER GENERATION
}
void Dungeon::generateTraps() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	//	TRAP GENERATION
	std::shared_ptr<Traps> trap(nullptr);

	for (int i = 0; i < 9; i++) { // i < number_of_different_trap_types

		// number of each trap to spawn
		switch (i) {
		case 0: n = 2 + randInt(0, 3); break; // Pit
		case 1: n = 10 + randInt(0, 0); break; // Firebars
		case 2: n = 10 + randInt(0, 0); break; // Braziers
		case 3: n = 15 + randInt(0, 5); break; // Puddles
		case 4: n = 5 + randInt(0, 10); break; // Springs
		case 5: n = 15 + randInt(0, 3); break; // Turrets
		case 6: n = 3 + randInt(0, 3); break; // 
		case 7: n = 10 + randInt(0, 3); break; // 
		case 8: n = 3 + randInt(0, 3); break; // 
		default: n = 0; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			// while position clashes with wall, player, etc., then reroll its position
			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || Dungeon::trap(x, y) || item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// Type of trap to spawn
			switch (i) {
			case 0:	trap = std::make_shared<Pit>(*this, x, y); break;
			case 1:	trap = std::make_shared<Stalactite>(*this, x, y); break;
			case 2:	trap = std::make_shared<Brazier>(*this, x, y); break;
			case 3:	trap = std::make_shared<Water>(*this, x, y); break;
			case 4: trap = std::make_shared<Spring>(*this, x, y, false); break;			
			case 5: trap = std::make_shared<Turret>(*this, x, y); break;
			case 6:	trap = std::make_shared<Puddle>(*this, x, y); break;
			case 7:	trap = std::make_shared<CrumblePit>(*this, x, y, 4); break;
			case 8:	trap = std::make_shared<PoisonWater>(*this, x, y); break;
			}

			addTrap(trap);

			n--;
			trap.reset();
		}
	}
	/// END TRAP GENERATION
}
void Dungeon::generateItems() {
	int cols = m_cols;
	int rows = m_rows;

	int n, x, y;

	//	ITEM GENERATION
	std::shared_ptr<Objects> item(nullptr);

	for (int i = 0; i < 21; i++) { // i < number_of_different_items

		// number of each item to spawn
		switch (i) {
		case 0: n = 1; break; // Life Potion
		case 1: n = 1; break; // Bomb
		case 2: n = 2; break; // Brown Chest
		case 3: n = 1; break; // Rusty Cutlass
		case 4: n = 1; break; // Skeleton Key
		case 5: n = 1; break; // Wood Shield
		case 6: n = 2; break; // Freeze Spell
		case 7: n = 1 + randInt(0, 2); break; // Earthquake Spell
		case 8: n = 2 + randInt(0, 2); break; // 
		case 9: n = 2 + randInt(0, 2); break; // 
		case 10: n = 2 + randInt(0, 2); break; // 
		case 11: n = 1 + randInt(0, 0); break; // 
		case 12: n = 1; break;
		case 13: n = 1; break;
		case 14: n = 1; break;
		case 15: n = 1; break;
		case 16: n = 1; break;
		case 17: n = 1; break;
		case 18: n = 1; break;
		case 19: n = 1; break;
		case 20: n = 1; break;
		case 21: n = 1; break;
		default: n = 0; break;
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || hero(x, y) || trap(x, y) || Dungeon::item(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// Type of item to spawn
			switch (i) {
			case 0:	item = std::make_shared<LifePotion>(x, y); break;
			case 1: item = std::make_shared<Bomb>(x, y); break;
			case 2:	item = std::make_shared<Firecrackers>(x, y); break;
			case 3:	item = std::make_shared<RockSummonSpell>(x, y); break;
			case 4:	item = std::make_shared<IceShardSpell>(x, y); break;
			case 5:	item = std::make_shared<WoodShield>(x, y); break;
			case 6:	item = std::make_shared<FreezeSpell>(x, y); break;
			case 7:	item = std::make_shared<EarthquakeSpell>(x, y); break;
			case 8:	item = std::make_shared<FireBlastSpell>(x, y); break;
			case 9:	item = std::make_shared<WhirlwindSpell>(x, y); break;
			case 10: item = std::make_shared<InvisibilitySpell>(x, y); break;
			case 11: item = std::make_shared<SteadyLifePotion>(x, y); break;
			case 12: item = std::make_shared<HailStormSpell>(x, y); break;
			case 13: item = std::make_shared<LuckUp>(x, y); break;
			case 14: item = std::make_shared<EtherealSpell>(x, y); break;
			case 15: item = std::make_shared<ShockwaveSpell>(x, y); break;
			case 16: item = std::make_shared<Matches>(x, y); break;
			case 17: item = std::make_shared<WindBlastSpell>(x, y); break;
			case 18: item = std::make_shared<BearTrap>(x, y); break;
			case 19: item = std::make_shared<FireCascadeSpell>(x, y); break;
			case 20: item = std::make_shared<WindVortexSpell>(x, y); break;
			}

			tileAt(Coords(x, y)).object = item;
			tileAt(Coords(x, y)).item = true;
			item->setSprite(createSprite(x, y, y + isShop() ? 2 : Z_ITEM, item->getImageName()));
			addItem(item);

			n--;
			item.reset();
		}
	}
	/// END ITEM GENERATION
}

void Dungeon::fillLevel(std::vector<std::string> finalvec, int start, int end) {
	int rows = getRows();
	int cols = getCols();

	int count = 0;
	std::shared_ptr<Monster> monster(nullptr);

	for (int i = start; i < rows - 1; i++) {
		for (int j = 1; j < cols + end; j++) {
			Coords currentPos(j, i);

			if (finalvec.at(count) == RANDOM_WALL) {
				// 50% chance to spawn wall
				if (randInt(0, 1) == 0) {
					std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
					m_walls.push_back(wall);
					tileAt(currentPos).wallObject = wall;
				}
			}
			else if (finalvec.at(count) == REG_WALL) {
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(Coords(j, i)).wallObject = wall;
			}
			else if (finalvec.at(count) == UNB_WALL) {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}
			else if (finalvec.at(count) == BOUNDARY) {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}
			else if (finalvec.at(count) == DOOR_HORIZONTAL) {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_HORIZONTAL));
			}
			else if (finalvec.at(count) == DOOR_VERTICAL) {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
			}
			else if (finalvec.at(count) == SPAWN_DISALLOWED) {
				tileAt(currentPos).noSpawn = true;
			}
			else if (finalvec.at(count) == STAIRCASE) {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
			}
			else if (finalvec.at(count) == LOCKED_STAIRCASE) {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i, true);
				addTrap(stairs);

				m_exit = stairs;
			}
			else if (finalvec.at(count) == DEVILS_WATER) {
				addTrap(std::make_shared<DevilsWater>(this, j, i));
			}
			else if (finalvec.at(count) == FOUNTAIN) {
				std::shared_ptr<Wall> wall = std::make_shared<Fountain>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
			}

			else if (finalvec.at(count) == PLAYER) {

				tileAt(currentPos).hero = true;				
				player[0]->setPos(currentPos);

				if (m_level != 1) {
					//player[0]->equipTrinket(*this, std::make_shared<>());
					//player[0]->addItem(std::make_shared<Bomb>());
					//player[0]->equipShield(*this, std::make_shared<WoodShield>());

					count++;
					continue;
				}

				//player[0]->setMoney(200);
				/*player[0]->addItem(std::make_shared<FireExplosionSpell>());
				player[0]->addItem(std::make_shared<FireExplosionSpell>());
				player[0]->addItem(std::make_shared<FireExplosionSpell>());*/
				player[0]->addItem(std::make_shared<RottenMeat>(0, 0));
				//player[0]->addItem(std::make_shared<FireBlastSpell>());
				//player[0]->addItem(std::make_shared<ShockwaveSpell>());
				//player[0]->addItem(std::make_shared<PoisonBomb>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				//player[0]->equipShield(*this, std::make_shared<ReflectShield>());
				player[0]->equipRelic(std::make_shared<SuperMagicEssence>(0, 0));
				player[0]->getRelic()->upgrade(*this, *player[0]);
				player[0]->getRelic()->upgrade(*this, *player[0]);
				player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->equipTrinket(*this, std::make_shared<CursedStrength>());
				//player[0]->equipPassive(*this, std::make_shared<ResonantSpells>());
				//player[0]->equipPassive(*this, std::make_shared<Scavenger>(0, 0));
			}
			else if (finalvec.at(count) == GOBLIN) {
				addMonster(std::make_shared<Goblin>(this, j, i, 12));
			}
			else if (finalvec.at(count) == SEEKER) {
				addMonster(std::make_shared<Seeker>(this, j, i, 10));
			}
			else if (finalvec.at(count) == DEAD_SEEKER) {
				addMonster(std::make_shared<DeadSeeker>(this, j, i));
			}
			else if (finalvec.at(count) == ITEM_THIEF) {
				addMonster(std::make_shared<ItemThief>(this, j, i));
			}
			else if (finalvec.at(count) == ARCHER) {
				addMonster(std::make_shared<Archer>(this, j, i));
			}
			else if (finalvec.at(count) == WANDERER) {
				addMonster(std::make_shared<Wanderer>(this, j, i));
			}
			else if (finalvec.at(count) == ZAPPER) {
				addMonster(std::make_shared<Zapper>(this, j, i));
			}
			else if (finalvec.at(count) == SPINNER) {
				addMonster(std::make_shared<Spinner>(this, j, i));
			}
			else if (finalvec.at(count) == GOO_SACK) {
				addMonster(std::make_shared<GooSack>(this, j, i));
			}
			else if (finalvec.at(count) == BOMBEE) {
				addMonster(std::make_shared<Bombee>(this, j, i));
			}
			else if (finalvec.at(count) == ROUNDABOUT) {
				addMonster(std::make_shared<Roundabout>(this, j, i));
			}
			else if (finalvec.at(count) == FIRE_ROUNDABOUT) {
				addMonster(std::make_shared<FireRoundabout>(this, j, i));
			}
			else if (finalvec.at(count) == BROUNDABOUT) {
				addMonster(std::make_shared<Broundabout>(this, j, i));
			}
			else if (finalvec.at(count) == PIKEMAN) {
				addMonster(std::make_shared<Pikeman>(this, j, i));
			}
			else if (finalvec.at(count) == RANDOM_MONSTER) {
				monster = rollMonster(m_level, this, j, i);
				addMonster(monster);
			}

			else if (finalvec.at(count) == PIT) {
				addTrap(std::make_shared<Pit>(*this, j, i));
			}
			else if (finalvec.at(count) == FIREBAR) {
				addTrap(std::make_shared<Firebar>(*this, j, i));
			}
			else if (finalvec.at(count) == DOUBLE_FIREBAR) {
				addTrap(std::make_shared<DoubleFirebar>(*this, j, i));
			}
			else if (finalvec.at(count) == SPIKES) {
				addTrap(std::make_shared<Spikes>(*this, j, i));
			}
			else if (finalvec.at(count) == AUTOSPIKE_DEACTIVE) {
				addTrap(std::make_shared<SpikeTrap>(*this, j, i, 4));
			}
			else if (finalvec.at(count) == TRIGGERSPIKE_DEACTIVE) {
				addTrap(std::make_shared<TriggerSpike>(*this, j, i));
			}
			else if (finalvec.at(count) == PUDDLE) {
				addTrap(std::make_shared<Puddle>(*this, j, i));
			}
			else if (finalvec.at(count) == SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false));
			}
			else if (finalvec.at(count) == LEFT_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'l'));
			}
			else if (finalvec.at(count) == RIGHT_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'r'));
			}
			else if (finalvec.at(count) == UP_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'u'));
			}
			else if (finalvec.at(count) == DOWN_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, 'd'));
			}
			else if (finalvec.at(count) == TL_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '2'));
			}
			else if (finalvec.at(count) == TR_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '1'));
			}
			else if (finalvec.at(count) == BL_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '3'));
			}
			else if (finalvec.at(count) == BR_SPRING) {
				addTrap(std::make_shared<Spring>(*this, j, i, false, '4'));
			}
			else if (finalvec.at(count) == LEFT_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'l'));
			}
			else if (finalvec.at(count) == RIGHT_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'r'));
			}
			else if (finalvec.at(count) == UP_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'u'));
			}
			else if (finalvec.at(count) == DOWN_TURRET) {
				addTrap(std::make_shared<Turret>(*this, j, i, 'd'));
			}
			else if (finalvec.at(count) == TURRET) {
				char move;
				switch (randInt(1, 4)) {
				case 1: move = 'l'; break;	// L
				case 2: move = 'r'; break;	// R
				case 3: move = 'u'; break;	// U
				case 4: move = 'd'; break;	// D
				}

				addTrap(std::make_shared<Turret>(*this, j, i, move));
			}
			else if (finalvec.at(count) == MOVING_BLOCK) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, '-', 5));
			}
			else if (finalvec.at(count) == MOVING_BLOCK_H) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, 'l', 4));
			}
			else if (finalvec.at(count) == MOVING_BLOCK_V) {
				addTrap(std::make_shared<LinearMovingBlock>(*this, j, i, 'u', 4));
			}
			else if (finalvec.at(count) == CRUMBLE_PIT) {
				addTrap(std::make_shared<CrumblePit>(*this, j, i));
			}
			else if (finalvec.at(count) == CRUMBLE_LAVA) {
				addTrap(std::make_shared<CrumbleLava>(*this, j, i));
			}
			else if (finalvec.at(count) == LAVA) {
				addTrap(std::make_shared<Lava>(*this, j, i));
			}

			else if (finalvec.at(count) == GOLD) {
				addGold(j, i, 4 + randInt(1, 10));
			}
			else if (finalvec.at(count) == HEART_POD) {
				createItem(std::make_shared<HeartPod>(j, i));
			}
			else if (finalvec.at(count) == LIFEPOT) {
				createItem(std::make_shared<LifePotion>(j, i));
			}
			else if (finalvec.at(count) == BIG_LIFEPOT) {
				createItem(std::make_shared<BigLifePotion>(j, i));
			}
			else if (finalvec.at(count) == ARMOR) {
				createItem(std::make_shared<ArmorDrop>(j, i));
			}
			else if (finalvec.at(count) == STATPOT) {
				createItem(std::make_shared<StatPotion>(j, i));
			}
			else if (finalvec.at(count) == BOMB) {
				createItem(std::make_shared<Bomb>(j, i));			
			}
			else if (finalvec.at(count) == BROWN_CHEST) {
				createChest(std::make_shared<BrownChest>(this, j, i));
			}
			else if (finalvec.at(count) == SILVER_CHEST) {
				createChest(std::make_shared<SilverChest>(this, j, i));
			}
			else if (finalvec.at(count) == GOLDEN_CHEST) {
				createChest(std::make_shared<GoldenChest>(this, j, i));
			}
			else if (finalvec.at(count) == RANDOM_CHEST) {
				createChest(rollChest(this, j, i, Rarity::ALL, true));
			}
			else if (finalvec.at(count) == RANDOM_ITEM) {
				tileAt(currentPos).object = rollItem(this, j, i, Rarity::COMMON, false, true);
			}
			else if (finalvec.at(count) == RANDOM_WEAPON) {
				createItem(rollWeapon(this, j, i, Rarity::RARE, true));
			}
			else if (finalvec.at(count) == RANDOM_SHIELD) {
				createItem(rollShield(this, j, i, Rarity::COMMON, false));
			}

			count++;
		}
	}

}



/** ******* ****** ****** **********
*
*		SPRITE SETTING/MOVING
*
*/

// Helper
Coords Dungeon::transformSpriteToDungeonCoordinates(const cocos2d::Vec2 &pos) const {
	int x, y;
	x = std::ceil((pos.x + X_OFFSET) / SPACING_FACTOR);
	y = std::ceil(static_cast<float>(m_rows) - ((pos.y + Y_OFFSET) / SPACING_FACTOR));

	return Coords(x, y);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const Coords &inCoords) const {
	return transformDungeonToSpriteCoordinates(inCoords, cocos2d::Vec2::ZERO);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const Coords &inCoords, const cocos2d::Vec2 &offset) const {
	return transformDungeonToSpriteCoordinates(static_cast<cocos2d::Vec2>(inCoords), offset);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const cocos2d::Vec2 &inCoords) const {
	return transformDungeonToSpriteCoordinates(inCoords, cocos2d::Vec2::ZERO);
}
cocos2d::Vec2 Dungeon::transformDungeonToSpriteCoordinates(const cocos2d::Vec2 &inCoords, const cocos2d::Vec2 &offset) const {
	float x, y;
	x = inCoords.x * SPACING_FACTOR - X_OFFSET;
	y = (m_rows - inCoords.y)*SPACING_FACTOR - Y_OFFSET;

	x += offset.x;
	y += offset.y;

	return cocos2d::Vec2(x, y);
}

cocos2d::Vec2 Dungeon::dungeonToLightCoordinates(float x, float y) const {
	auto vs = cocos2d::Director::getInstance()->getVisibleSize();
	auto vsw = vs.width / 2;
	auto vsh = vs.height / 2;

	float fx = x + vsw;
	float fy = y + vsh;

	return cocos2d::Vec2(fx, fy);
}

// Create new sprites
SpriteType* Dungeon::createEffectSprite(int x, int y, int z, const std::string &image) {
	auto spriteFrame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(image);
	SpriteType* sprite = EffectSprite::createWithSpriteFrame(spriteFrame);
	m_scene->addChild(sprite, z);

	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(Coords(x, y));

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	m_scene->setLightingOn(sprite);

	return sprite;
}

cocos2d::Sprite* Dungeon::createSprite(int x, int y, int z, const std::string &image) {
	return createSprite(Coords(x, y), cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const Coords &pos, int z, const std::string &image) {
	return createSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const Coords &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	return createSprite(static_cast<cocos2d::Vec2>(pos), offset, z, image);
}

cocos2d::Sprite* Dungeon::createSprite(float x, float y, int z, const std::string &image) {
	return createSprite(cocos2d::Vec2(x, y), cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const cocos2d::Vec2 &pos, int z, const std::string &image) {
	return createSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createSprite(const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);

	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	sprite->setPosition(finalPos);
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	return sprite;
}

void Dungeon::addSprite(int x, int y, int z, std::string image) {
	(void)createMiscSprite(Coords(x, y), z, image);
}
cocos2d::Sprite* Dungeon::createMiscSprite(const Coords &pos, int z, const std::string &image) {
	return createMiscSprite(pos, cocos2d::Vec2::ZERO, z, image);
}
cocos2d::Sprite* Dungeon::createMiscSprite(const Coords &pos, const cocos2d::Vec2 &offset, int z, const std::string &image) {
	cocos2d::Sprite* sprite = createSprite(pos, offset, z, image);

	MiscSprite misc(pos, sprite);
	m_miscSprites.push_back(misc);

	return sprite;
}

cocos2d::Sprite* Dungeon::createWallSprite(int x, int y, int z, std::string type) {
	std::string image;
	cocos2d::Sprite* wall = nullptr;

	if (type == REG_WALL) {
		switch (randInt(1, 13)) {
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

		wall = createSprite(x, y, z, image);
	}
	else if (type == UNB_WALL) {
		switch (randInt(1, 15)) {
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

		wall = createSprite(x, y, z, image);
	}
	else if (type == FOUNTAIN) {
		wall = createSprite(x, y, z, "Fountain_Down_48x48.png");
	}

	return wall;
}

// Set sprites to position
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, int x, int y) {
	teleportSprite(sprite, Coords(x, y));
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const Coords &pos) {
	teleportSprite(sprite, pos, cocos2d::Vec2::ZERO);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const Coords &pos, const cocos2d::Vec2 &offset) {
	teleportSprite(sprite, static_cast<cocos2d::Vec2>(pos), offset);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, float x, float y) {
	teleportSprite(sprite, cocos2d::Vec2(x, y));
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &pos) {
	teleportSprite(sprite, pos, cocos2d::Vec2::ZERO);
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);
	sprite->setPosition(finalPos);
}

// Move sprites
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time, bool front) {
	queueMoveSprite(sprite, Coords(cx, cy), time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const Coords &destination, float time) {
	queueMoveSprite(sprite, destination, cocos2d::Vec2::ZERO, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const Coords &destination, const cocos2d::Vec2 &offset, float time) {
	queueMoveSprite(sprite, static_cast<cocos2d::Vec2>(destination), offset, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, float cx, float cy, float time, bool front) {
	queueMoveSprite(sprite, cocos2d::Vec2(cx, cy), time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &destination, float time) {
	queueMoveSprite(sprite, destination, cocos2d::Vec2::ZERO, time);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, const cocos2d::Vec2 &destination, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 pos = transformDungeonToSpriteCoordinates(destination, offset);

	cocos2d::MoveTo *move = cocos2d::MoveTo::create(time, pos);
	insertActionIntoSpawn(sprite, move);
}

cocos2d::MoveTo* Dungeon::getMoveAction(int cx, int cy, float time) {
	return getMoveAction(Coords(cx, cy), time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const Coords &pos, float time) {
	return getMoveAction(pos, cocos2d::Vec2::ZERO, time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const Coords &pos, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	return cocos2d::MoveTo::create(time, finalPos);
}
cocos2d::MoveTo* Dungeon::getMoveAction(float cx, float cy, float time) {
	return getMoveAction(cocos2d::Vec2(cx, cy), time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const cocos2d::Vec2 &pos, float time) {
	return getMoveAction(pos, cocos2d::Vec2::ZERO, time);
}
cocos2d::MoveTo* Dungeon::getMoveAction(const cocos2d::Vec2 &pos, const cocos2d::Vec2 &offset, float time) {
	cocos2d::Vec2 finalPos = transformDungeonToSpriteCoordinates(pos, offset);

	return cocos2d::MoveTo::create(time, finalPos);
}

// Remove sprites
void Dungeon::queueRemoveSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	insertActionIntoSpawn(sprite, remove);
}
void Dungeon::queueRemoveSpriteWithMultipleActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	for (auto &it : actions)
		insertActionIntoSpawn(sprite, it);

	insertActionIntoSpawn(sprite, cocos2d::RemoveSelf::create());
}
void Dungeon::removeSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	sprite->runAction(remove);
}

// Custom actions
void Dungeon::queueCustomAction(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action) {
	insertActionIntoSpawn(sprite, action);
}
void Dungeon::queueCustomActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions) {
	for (auto &it : actions)
		insertActionIntoSpawn(sprite, it);
}
void Dungeon::queueCustomSpawnActions(cocos2d::Sprite* sprite, const cocos2d::Vector<cocos2d::FiniteTimeAction*> &actions) {
	auto spawn = cocos2d::Spawn::create(actions);
	insertActionIntoSpawn(sprite, spawn);
}

// Update sprite coloring (lighting)
void Dungeon::updateLighting() {

	if (m_level == FIRST_BOSS)
		return;

	//cocos2d::Vec2 vec = dungeonToLightCoordinates(player[0]->getPosX(), player[0]->getPosY());
	//m_scene->movePlayerLightTo(vec.x, vec.y);

	int cols = m_cols;

	int px = player[0]->getPosX();
	int py = player[0]->getPosY();
	int p_cutoff = player[0]->getVision();

	int sx, sy, dist, color;

	// Extra sprites for decoration
	for (size_t i = 0; i < m_miscSprites.size(); ++i) {
		if (!m_miscSprites[i].sprite)
			continue;

		sx = m_miscSprites[i].pos.x;
		sy = m_miscSprites[i].pos.y;

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);
		color = 0;

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
	}

	// npc sprites
	for (unsigned int i = 0; i < m_npcs.size(); i++) {
		sx = m_npcs.at(i)->getPosX();
		sy = m_npcs.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = 0;

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));		
	}

	// monster sprites
	int mx, my;
	for (size_t i = 0; i < m_monsters.size(); ++i) {
		mx = m_monsters.at(i)->getPosX();
		my = m_monsters.at(i)->getPosY();
		dist = abs(mx - px) + abs(my - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = 0;

		// If monster is a source of light
		if (m_monsters.at(i)->emitsLight()) {
			m_monsters.at(i)->addLightEmitters(lightEmitters);
			continue;
		}

		if (player[0]->hasMonsterIllumination()) {
			lightEmitters.push_back(Coords(mx, my));
			continue;
		}

		if (dist <= p_cutoff)
			color = (255 * formula);

		m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

		// If monster uses extra sprites, update their lighting too
		if (m_monsters.at(i)->hasExtraSprites())
			m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
	}

	// trap sprites
	for (unsigned int i = 0; i < m_traps.size(); i++) {

		if (m_traps.at(i)->getSprite() == nullptr)
			continue;

		sx = m_traps.at(i)->getPosX();
		sy = m_traps.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		// if player has trap illumination
		if (player[0]->hasTrapIllumination() && m_traps.at(i)->canBeIlluminated())
			lightEmitters.push_back(Coords(sx, sy));

		// if trap is lava or other light-emitting source, emit light from their position
		if (m_traps.at(i)->emitsLight()) {
			lightEmitters.push_back(Coords(m_traps.at(i)->getPosX(), m_traps.at(i)->getPosY()));
			continue;
		}

		if (m_traps.at(i)->getName() == LAVA) {
			int cutoff = p_cutoff + 6;
			formula = (cutoff - dist) / (float)(cutoff);
			color = (255 * formula);

			if (dist >= cutoff)
				color = 19; // color value calculated by taking (1/cutoff) * 255

			m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
		else {
			color = 0;
			if (dist <= p_cutoff)
				color = (255 * formula);

			if (m_traps.at(i)->getSprite() != nullptr)
				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

			// if trap uses extra sprites, update their lighting too
			if (m_traps.at(i)->hasExtraSprites())
				m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));			
		}
	}

	// floor sprites
	for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
		for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
			if (withinBounds(i, j)) {
				sx = i;
				sy = j;

				dist = abs(sx - px) + abs(sy - py);
				float formula = (p_cutoff - dist) / (float)p_cutoff;
				color = 0;

				if (dist <= p_cutoff)
					color = (255 * formula);

				tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	// Walls
	for (unsigned int i = 0; i < m_walls.size(); i++) {
		sx = m_walls.at(i)->getPosX();
		sy = m_walls.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;
		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));	
	}

	// item sprites
	for (unsigned int i = 0; i < m_items.size(); i++) {
		sx = m_items.at(i)->getPosX();
		sy = m_items.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (player[0]->hasItemIllumination())
			lightEmitters.push_back(Coords(sx, sy));

		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
	}

	// money sprites
	for (unsigned int i = 0; i < m_money.size(); i++) {
		sx = m_money.at(i)->getPosX();
		sy = m_money.at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (player[0]->hasGoldIllumination())
			lightEmitters.push_back(Coords(sx, sy));

		color = (255 * formula);

		if (dist > p_cutoff)
			color = 0;

		m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
	}


	updateSecondaryLighting();
}
void Dungeon::updateSecondaryLighting() {
	int cols = m_cols;

	int px, py;
	int p_cutoff = 2;

	int sx, sy, dist, color;
	cocos2d::Color3B c3b;

	for (unsigned int n = 0; n < lightEmitters.size(); ++n) {

		px = lightEmitters.at(n).x;
		py = lightEmitters.at(n).y;

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				
				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites()) {
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));
				}
			}
			
		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) {
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// item sprites
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	// Check other light sources
	for (unsigned int pos = 0; pos < m_lightSources.size(); ++pos) {
		px = m_lightSources.at(pos).pos.x;
		py = m_lightSources.at(pos).pos.y;
		p_cutoff = static_cast<int>(m_lightSources.at(pos).radius);

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));

				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites())
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));			
			}

		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) { // boundary check
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// Items
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	for (unsigned int pos = 0; pos < m_playerClones.size(); ++pos) {

		px = m_playerClones[pos]->getPosX();
		py = m_playerClones[pos]->getPosY();
		p_cutoff = m_playerClones[pos]->getVision();

		// Extra sprites for decoration
		for (unsigned int i = 0; i < m_miscSprites.size(); i++) {
			if (!m_miscSprites[i].sprite)
				continue;

			sx = m_miscSprites[i].pos.x;
			sy = m_miscSprites[i].pos.y;

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / static_cast<float>(p_cutoff);

			if (dist <= p_cutoff) {
				c3b = m_miscSprites[i].sprite->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_miscSprites[i].sprite->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// npc sprites
		for (unsigned int i = 0; i < m_npcs.size(); i++) {
			sx = m_npcs.at(i)->getPosX();
			sy = m_npcs.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_npcs.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_npcs.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// monster sprites
		int mx, my;
		for (unsigned int i = 0; i < m_monsters.size(); i++) {
			mx = m_monsters.at(i)->getPosX();
			my = m_monsters.at(i)->getPosY();
			dist = abs(mx - px) + abs(my - py);

			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_monsters.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if monster uses extra sprites, update their lighting too
				if (m_monsters.at(i)->hasExtraSprites()) {
					m_monsters.at(i)->updateSpriteLighting(cocos2d::Color3B(color, color, color));
				}
			}

		}

		// trap sprites
		for (unsigned int i = 0; i < m_traps.size(); i++) {

			if (m_traps.at(i)->getSprite() == nullptr)
				continue;

			sx = m_traps.at(i)->getPosX();
			sy = m_traps.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_traps.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));

				m_traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (m_traps.at(i)->hasExtraSprites())
					m_traps.at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));				
			}

		}

		// floor sprites
		for (int i = px - std::max(15, p_cutoff); i < px + std::max(15, p_cutoff) + 1; i++) {
			for (int j = py - std::max(15, p_cutoff); j < py + std::max(15, p_cutoff) + 1; j++) {
				if (withinBounds(i, j)) {					
					sx = i;
					sy = j;

					dist = abs(sx - px) + abs(sy - py);
					float formula = (p_cutoff - dist) / (float)p_cutoff;

					if (dist <= p_cutoff) {
						c3b = tileAt(Coords(i, j)).floor->getDisplayedColor();
						color = std::max((int)c3b.r, (int)(255 * formula));
						tileAt(Coords(i, j)).floor->setColor(cocos2d::Color3B(color, color, color));
					}

				}
			}
		}

		// wall sprites
		for (unsigned int i = 0; i < m_walls.size(); i++) {
			sx = m_walls.at(i)->getPosX();
			sy = m_walls.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_walls.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_walls.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// item sprites
		for (unsigned int i = 0; i < m_items.size(); i++) {
			sx = m_items.at(i)->getPosX();
			sy = m_items.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_items.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_items.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}

		// money sprites
		for (unsigned int i = 0; i < m_money.size(); i++) {
			sx = m_money.at(i)->getPosX();
			sy = m_money.at(i)->getPosY();

			dist = abs(sx - px) + abs(sy - py);
			float formula = (p_cutoff - dist) / (float)p_cutoff;

			if (dist <= p_cutoff) {
				c3b = m_money.at(i)->getSprite()->getDisplayedColor();
				color = std::max((int)c3b.r, (int)(255 * formula));
				m_money.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	lightEmitters.clear();
}

int Dungeon::findLightSource(int x, int y, std::string name) {
	const Coords target(x, y);
	for (int i = 0; i < static_cast<int>(m_lightSources.size()); ++i) {
		if (m_lightSources.at(i).pos == target && m_lightSources.at(i).name == name) {
			return i;
		}
	}

	return -1;
}
void Dungeon::addLightSource(int x, int y, double strength, std::string name) {
	Coords coords(x, y);
	LightSource lightSource(coords, strength, name);

	m_lightSources.push_back(lightSource);
}
void Dungeon::removeLightSource(int x, int y, std::string name) {
	int pos = findLightSource(x, y, name);

	if (pos == -1)
		return;

	m_lightSources.erase(m_lightSources.begin() + pos);
}

void Dungeon::zoomInBy(float factor) {
	// Since the ratio of sprite scale to spacing factor is 1/44, we increment by this same ratio
	// NOTE: This ratio is subject to change when art is finalized.

	GLOBAL_SPRITE_SCALE += factor;
	SPACING_FACTOR += factor * 44.0f;

	updateSpriteScaling();
}
void Dungeon::zoomOutBy(float factor) {
	if (GLOBAL_SPRITE_SCALE - factor <= 0.0f)
		return;
	
	GLOBAL_SPRITE_SCALE -= factor;
	SPACING_FACTOR -= factor * 44.0f;

	updateSpriteScaling();
}
void Dungeon::updateSpriteScaling() {
	int cols = m_cols;
	int rows = m_rows;

	cocos2d::ScaleTo* scale = cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;

	actions.pushBack(scale);
	actions.pushBack(getMoveAction(player[0]->getPos(), getHalfTileYOffset()));
	queueCustomSpawnActions(player[0]->getSprite(), actions);

	for (size_t i = 0; i < m_playerClones.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_playerClones[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_playerClones[i]->getSprite(), actions);
	}

	// Extra sprites for decoration
	for (size_t i = 0; i < m_miscSprites.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_miscSprites[i].pos));
		queueCustomSpawnActions(m_miscSprites[i].sprite, actions);

		//m_miscSprites[i].sprite->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_miscSprites[i].sprite, cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_miscSprites[i].sprite, m_miscSprites[i].pos.x, m_miscSprites[i].pos.y);
	}

	// npc sprites
	for (size_t i = 0; i < m_npcs.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_npcs[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_npcs[i]->getSprite(), actions);

		//m_npcs[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_npcs[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_npcs[i]->getSprite(), m_npcs[i]->getPosX(), m_npcs[i]->getPosY());
	}

	// monster sprites
	for (size_t i = 0; i < m_monsters.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_monsters[i]->getPos(), getHalfTileYOffset()));
		queueCustomSpawnActions(m_monsters[i]->getSprite(), actions);

		//m_monsters[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_monsters[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_monsters[i]->getSprite(), m_monsters[i]->getPosX(), m_monsters[i]->getPosY());
	}

	// trap sprites
	for (size_t i = 0; i < m_traps.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_traps[i]->getPos()));
		queueCustomSpawnActions(m_traps[i]->getSprite(), actions);

		//m_traps[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_traps[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_traps[i]->getSprite(), m_traps[i]->getPosX(), m_traps[i]->getPosY());
	}

	// floor sprites
	for (int i = 0; i < cols; ++i) {
		for (int j = 0; j < rows; ++j) {
			actions.clear();
			actions.pushBack(scale->clone());
			actions.pushBack(getMoveAction(i, j));
			queueCustomSpawnActions(tileAt(Coords(i, j)).floor, actions);
		}
	}

	// Walls
	for (size_t i = 0; i < m_walls.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_walls[i]->getPos()));
		queueCustomSpawnActions(m_walls[i]->getSprite(), actions);

		//m_walls[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_walls[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_walls[i]->getSprite(), m_walls[i]->getPosX(), m_walls[i]->getPosY());
	}

	// item sprites
	for (size_t i = 0; i < m_items.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_items[i]->getPos()));
		queueCustomSpawnActions(m_items[i]->getSprite(), actions);

		//m_items[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_items[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_items[i]->getSprite(), m_items[i]->getPosX(), m_items[i]->getPosY());
	}

	// money sprites
	for (size_t i = 0; i < m_money.size(); ++i) {
		actions.clear();
		actions.pushBack(scale->clone());
		actions.pushBack(getMoveAction(m_money[i]->getPos()));
		queueCustomSpawnActions(m_money[i]->getSprite(), actions);

		//m_money[i]->getSprite()->setScale(GLOBAL_SPRITE_SCALE);
		//queueCustomAction(m_money[i]->getSprite(), cocos2d::ScaleTo::create(0.1f, GLOBAL_SPRITE_SCALE));
		//queueMoveSprite(m_money[i]->getSprite(), m_money[i]->getPosX(), m_money[i]->getPosY());
	}

	runSpawn();
}

// Run sprite actions
int Dungeon::spriteFound(cocos2d::Sprite* sprite) {
	for (int i = 0; i < static_cast<int>(m_seq.size()); ++i)
		if (m_seq[i].first == sprite)
			return i;
	
	return -1;
}
void Dungeon::insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front) {
	int i = spriteFound(sprite);

	if (i == -1) {
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(action);
		if (!front)
			m_seq.push_back(std::make_pair(sprite, v));	
		else
			m_seq.insert(m_seq.begin(), std::make_pair(sprite, v));
		
	}
	else {
		m_seq.at(i).second.pushBack(action);
	}
}
void Dungeon::runSpawn() {
	if (!m_seq.empty()) {
		for (auto &it : m_seq) {
			if (it.first) {
				cocos2d::Sequence* seq = cocos2d::Sequence::create(it.second);

				auto singleAction = it.first->runAction(seq);
				singleAction->setTag(1);
			}
		}
		m_seq.clear();
	}
}

cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	return runAnimationForever(frames, frameInterval, Coords(x, y), cocos2d::Vec2::ZERO, z);
}
cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, int z) {
	return runAnimationForever(frames, frameInterval, pos, cocos2d::Vec2::ZERO, z);
}
cocos2d::Sprite* Dungeon::runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, const cocos2d::Vec2 &offset, int z) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, pos, offset);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	sprite->runAction(cocos2d::RepeatForever::create(cocos2d::Animate::create(animation)));
	sprite->setScale(GLOBAL_SPRITE_SCALE);

	return sprite;
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z) {
	runSingleAnimation(frames, frameInterval, Coords(x, y), cocos2d::Vec2::ZERO, z);
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, int z) {
	runSingleAnimation(frames, frameInterval, pos, cocos2d::Vec2::ZERO, z);
}
void Dungeon::runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, const Coords &pos, const cocos2d::Vec2 &offset, int z) {
	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	sprite->setScale(GLOBAL_SPRITE_SCALE);
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, pos, offset);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto remove = cocos2d::RemoveSelf::create();
	auto sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::Animate::create(animation), remove);
	auto action = sprite->runAction(sequence);
	action->setTag(5);
}
void Dungeon::runAnimationWithCallback(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z,
	std::function<void(Dungeon&, int, int, int, std::string)> cb, std::string image) {

	auto sprite = cocos2d::Sprite::createWithSpriteFrame(frames.front());
	m_scene->addChild(sprite, z);
	teleportSprite(sprite, x, y);

	auto animation = cocos2d::Animation::createWithSpriteFrames(frames, 1.0f / frameInterval);
	auto remove = cocos2d::RemoveSelf::create();
	auto sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::Animate::create(animation), remove);
	auto action = sprite->runAction(sequence);
	action->setTag(5);
	
	// Adds the last sprite so that its lighting can be updated
	cb(*this, x, y, 0, image);
}



// ===========================================
//		   :::: TUTORIAL LEVEL ::::
// ===========================================
TutorialFloor::TutorialFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, TUTORIAL, 26, 41) {
	int rows = getRows();
	int cols = getCols();

	// Get the level layout
	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPosX(j);
				player[0]->setPosY(i);
				player[0]->setMoney(200);
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				player[0]->setVision(8);

				break;
			}
			case '#': {				
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
					// Water tile
			case 'w': {
				addTrap(std::make_shared<Water>(*this, j, i));				

				break;
			}
					  // Dirt tile
			case 'D': {
				if (randReal(1, 100) > 10)
					addTrap(std::make_shared<Sand>(*this, j, i));
				else
					addTrap(std::make_shared<Quicksand>(*this, j, i));
				
				//createItem(std::make_shared<RPGInABottle>(j, i));
				break;
			}
					// Breakable object
			case 'B': { 
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					// Arrow Sign
			case 'A': { 
				//std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				//m_monsters.push_back(object);

				//m_npcs.push_back(std::make_shared<Enchanter>(this, j, i));

				addMonster(Wriggler::create(this, Coords(j, i)));

				//createChest(std::make_shared<HauntedChest>(this, j, i));

				//addMonster(std::make_shared<ShieldMaster>(this, j, i));

				//addTrap(std::make_shared<LightAbsorber>(*this, j, i));
				//addTrap(std::make_shared<SquareMovingTile>(*this, j, i, 'u', 4, false));

				break;
			}
					// Regular Sign
			case 'R': { 
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					// Outside Man 1
			case 'M': {			
				/*std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC1>(j, i);
				m_npcs.push_back(npc);*/

				//m_npcs.push_back(std::make_shared<InjuredExplorer>(this, j, i));

				//addMonster(std::make_shared<IncendiaryInfuser>(this, j, i));

				//addTrap(std::make_shared<SquareMovingBlock>(*this, j, i, 'l', 6, true));
				addTrap(std::make_shared<DNASplitter>(*this, j, i));

				break;
			}
					  // Outside Man 2
			case 'Q': {
				std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC2>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
					  // Outside Woman 1
			case 'F': {
				std::shared_ptr<NPC> npc = std::make_shared<Trader>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
			}
			
		}
	}

	/*int x, y;
	for (int i = 0; i < 30; i++) {
		x = randInt(1, cols - 2);
		y = randInt(1, rows - 2);

		while (wall(x, y) || enemy(x, y) || hero(x, y)) {
			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);
		}

		addMonster(rollMonster(SEVENTH_FLOOR, this, x, y));
	}*/
}

std::vector<char> TutorialFloor::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char r = 'r'; // Rock
	char k = 'k'; // 
	char d = 'd'; //
	// 'w' : Water
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'A' : Arrow Sign
	// 'R' : Regular Sign
	// 'M' : Outside Man 1
	// 'Q' : Outside Man 2
	// 'F' : Outside Woman 1
	

	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*3*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*4*/	'#', '#', '#', '#', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#', '#', '#',
		/*5*/	'#', '#', '#', '#', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', 'Q', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*6*/	'#', '#', '#', 'w', 'w', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#',
		/*7*/	'#', '#', '#', 'w', 'w', 'w', 'w', 'w', 'w', 'D', ' ', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*8*/	'#', '#', '#', 'D', 'w', 'w', 'w', 'D', 'D', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', '#', 'B', 'A', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*9*/	'#', '#', '#', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', 'M', ' ', ' ', ' ', ' ', ' ', '#', '#', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  s , '#', '#', '#', '#', '#',
		/*10*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'M', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*11*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'R', '#', 'B', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#', '#',
		/*12*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'F', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*13*/	'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*14*/	'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', ' ', ' ', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#',
		/*15*/	'#', '#', '#', '#', ' ', ' ', 'M', ' ', 'A', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', ' ', ' ', '#', 'B', 'A', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#',
		/*16*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', 'D', ' ', '#', ' ', 'A', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#',
		/*17*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', 'B', 'B', '#', '#', '#', '#',
		/*18*/	'#', '#', '#', '#', 'M', '@', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'B', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B', '#', '#', '#', '#', '#',
		/*19*/	'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', 'D', 'D', 'D', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
		/*20*/	'#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*21*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*22*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*23*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*24*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
	};

	return level;
}


// ===========================================
//			  :::: WORLD HUB ::::
// ===========================================
WorldHub::WorldHub(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, WORLD_HUB, 52, 41) {
	int rows = getRows();
	int cols = getCols();

	// Get the level layout
	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;

				player[0]->setPosX(j);
				player[0]->setPosY(i);
				//player[0]->setMoney(1000);
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));

				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
						 // Water tile
			case 'w': {
				std::shared_ptr<Traps> water = std::make_shared<Water>(*this, j, i);
				addTrap(water);
				
				break;
			}
					  // Dirt tile
			case 'D': {
				break;
			}
					  // Breakable object
			case 'B': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					  // Arrow Sign
			case 'A': {
				std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Regular Sign
			case 'R': {
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Exit to Main Menu
			case 'E': {
				tileAt(currentPos).menu = true;

				break;
			}
			}

		}
	}
}

void WorldHub::specialActions() {
	Coords pos = getPlayer()->getPos();

	// If player stepped on the exit to menu tile, then exit to main menu
	if (tileAt(pos).menu) {
		m_scene->returnToMainMenu();
		m_return = true;
	}
}

std::vector<char> WorldHub::generate() {
	char dv = DOOR_V;
	char S = STAIRS;
	// 'w' : Water
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'A' : Arrow Sign
	// 'R' : Regular Sign
	// 'E' : Exit to Main Menu


	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*3*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*4*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*5*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*6*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*7*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*8*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*9*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', '#',  S , '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*10*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*11*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*12*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*13*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*14*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*15*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*16*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*17*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*18*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*19*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*20*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*21*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*22*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*23*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*24*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*26*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*27*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'B', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*28*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*29*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*30*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'R', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*31*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*32*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*33*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*34*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*35*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*36*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*37*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*38*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*39*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*40*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*41*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*42*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '@', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*43*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*44*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*45*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*46*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'E', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*47*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*48*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*49*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*50*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*51*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
	};

	return level;
}



// ===========================================
//				:::: SHOP ::::
// ===========================================
Shop::Shop(LevelScene* scene, std::shared_ptr<Player> p, int level) : Dungeon(scene, p, level, MAXROWS_SHOP1, MAXCOLS_SHOP1) {

	int rows = getRows();
	int cols = getCols();

	// Add light source in the middle of the shop
	addLightSource(13, 5, 9, "Shop Light");

	// Price adjustment multiplier
	float baseMultiplier = 1.0f;
	switch (level) {
	case FIRST_SHOP: baseMultiplier = 1.0f; break;
	case SECOND_SHOP: baseMultiplier = 2.2f; break;
	case THIRD_SHOP: baseMultiplier = 3.6f; break;
	case FOURTH_SHOP: baseMultiplier = 5.0f; break;
	default: baseMultiplier = 1.0f; break;
	}
	
	m_priceMultiplier = baseMultiplier * (p->getFavor() <= -8 ? 1.15f : 1.0f) * (p->hasCheapShops() ? 0.5f : 1.0f);

	// get the shop layout
	std::vector<char> v = generate();

	// begin mapping shop
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPosX(j);
				player[0]->setPosY(i);
				player[0]->setMoney(1000);
				//player[0]->storeWeapon(std::make_shared<BoneAxe>(0, 0));
				//player[0]->equipTrinket(*this, std::make_shared<>());
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				std::shared_ptr<Stairs> stairs = std::make_shared<Stairs>(*this, j, i);
				addTrap(stairs);

				m_exit = stairs;
				break;
			}
				// purchase spot
			case 'p': {
				tileAt(currentPos).shop_action = PURCHASE;
				break;
			}
				// item price
			case '$': {

				break;
			}
				// shop counter
			case 'c': {
				addSprite(j, i, i + Z_ITEM - 1, "Countertop_48x48.png");

				tileAt(currentPos).wall = true;
				tileAt(currentPos).boundary = true;
				break;
			}
				// shopkeeper
			case 'k': {
				std::shared_ptr<NPC> shopkeeper = std::make_shared<Shopkeeper>(this, j, i + 1);
				m_npcs.push_back(shopkeeper);

				break;
			}
				// breakable object
			case 'd': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}
				
				m_monsters.push_back(object);

				break;
			}
				// secret thing
			case 'b': {
				addSprite(j, i, i + Z_EXIT, "Stairs_48x48.png");
				//cocos2d::Sprite* object = createSprite(j, i, 1, "Stairs_48x48.png");
				//misc_sprites.push_back(object);

				break;
			}
					  // 1, 2, 3 : hp, shield repair, utility
			case '1': {
				switch (randInt(0, 4)) {
				case 0: tileAt(currentPos).object = std::make_shared<LifePotion>(j, i); tileAt(currentPos).price = 30 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<BigLifePotion>(j, i); tileAt(currentPos).price = 45 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<SoulPotion>(j, i); tileAt(currentPos).price = 37 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<HalfLifePotion>(j, i); tileAt(currentPos).price = 42 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<SteadyLifePotion>(j, i); tileAt(currentPos).price = 36 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;

				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
			case '2': {
				switch (randInt(0, 5)) {
				case 0: tileAt(currentPos).object = std::make_shared<SmokeBomb>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<Bomb>(j, i);  tileAt(currentPos).price = 25 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<StatPotion>(j, i); tileAt(currentPos).price = 23 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Matches>(j, i); tileAt(currentPos).price = 16 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<BearTrap>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<Firecrackers>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
			case '3': {
				switch (randInt(0, 6)) {
				case 0: tileAt(currentPos).object = std::make_shared<LifePotion>(j, i); tileAt(currentPos).price = 30 * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<BinaryLifePotion>(j, i); tileAt(currentPos).price = 45 * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<RottenApple>(j, i); tileAt(currentPos).price = 25 * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Bomb>(j, i); tileAt(currentPos).price = 21 * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<PoisonBomb>(j, i); tileAt(currentPos).price = 24 * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<RottenMeat>(j, i); tileAt(currentPos).price = 20 * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<ArmorDrop>(j, i); tileAt(currentPos).price = 55 * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");

				break;
			}
					  // 4, 5 : random items
			case '4': {
				switch (randInt(0, 8)) {
				case 0: tileAt(currentPos).object = std::make_shared<BoStaff>(j, i); tileAt(currentPos).price = (80 + randInt(0, 5)) * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<Pike>(j, i); tileAt(currentPos).price = (80 + randInt(0, 5)) * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<Boomerang>(j, i); tileAt(currentPos).price = (55 + randInt(0, 5)) * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<Katana>(j, i); tileAt(currentPos).price = (85 + randInt(0, 5)) * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<Estoc>(j, i); tileAt(currentPos).price = (120 + randInt(0, 10)) * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<Hammer>(j, i); tileAt(currentPos).price = 58 * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<Nunchuks>(j, i); tileAt(currentPos).price = 65 * m_priceMultiplier; break;
				case 7: tileAt(currentPos).object = std::make_shared<Zweihander>(j, i); tileAt(currentPos).price = (70 + randInt(0, 8)) * m_priceMultiplier; break;
				case 8: tileAt(currentPos).object = std::make_shared<Jian>(j, i); tileAt(currentPos).price = (70 + randInt(0, 8)) * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(j, i, i + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
			case '5': {
				switch (randInt(0, 10)) {
				case 0: tileAt(currentPos).object = std::make_shared<BigLifePotion>(j, i); tileAt(currentPos).price = (30) * m_priceMultiplier; break;
				case 1: tileAt(currentPos).object = std::make_shared<FireBlastSpell>(j, i); tileAt(currentPos).price = (39 + randInt(0, 3)) * m_priceMultiplier; break;
				case 2: tileAt(currentPos).object = std::make_shared<FireExplosionSpell>(j, i); tileAt(currentPos).price = (40 + randInt(0, 3)) * m_priceMultiplier; break;
				case 3: tileAt(currentPos).object = std::make_shared<FireCascadeSpell>(j, i); tileAt(currentPos).price = (45 + randInt(0, 9)) * m_priceMultiplier; break;
				case 4: tileAt(currentPos).object = std::make_shared<Claw>(j, i); tileAt(currentPos).price = (85 + randInt(0, 20)) * m_priceMultiplier; break;
				case 5: tileAt(currentPos).object = std::make_shared<FreezeSpell>(j, i); tileAt(currentPos).price = (38 + randInt(0, 5)) * m_priceMultiplier; break;
				case 6: tileAt(currentPos).object = std::make_shared<HailStormSpell>(j, i); tileAt(currentPos).price = (48 + randInt(0, 5)) * m_priceMultiplier; break;
				case 7: tileAt(currentPos).object = std::make_shared<EtherealSpell>(j, i); tileAt(currentPos).price = (50 + randInt(0, 6)) * m_priceMultiplier; break;
				case 8: tileAt(currentPos).object = std::make_shared<RockSummonSpell>(j, i); tileAt(currentPos).price = (43 + randInt(0, 8)) * m_priceMultiplier; break;
				case 9: tileAt(currentPos).object = std::make_shared<ShockwaveSpell>(j, i); tileAt(currentPos).price = (51 + randInt(0, 7)) * m_priceMultiplier; break;
				case 10: tileAt(currentPos).object = std::make_shared<WindBlastSpell>(j, i); tileAt(currentPos).price = (32 + randInt(0, 8)) * m_priceMultiplier; break;
				}

				tileAt(currentPos).object->setSprite(createSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM, tileAt(currentPos).object->getImageName()));
				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				tileAt(currentPos).wall = true;
				tileAt(currentPos).item = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
					  // 6 : random chest
			case '6': {
				double n = randReal(1, 100) + p->getLuck();
				
				// 50% chance
				if (n <= 50) { tileAt(currentPos).object = std::make_shared<BrownChest>(this, j, i); tileAt(currentPos).price = 25 * m_priceMultiplier; }
				// 10% chance
				else if (n > 50 && n <= 60) { tileAt(currentPos).object = std::make_shared<LifeChest>(this, j, i); tileAt(currentPos).price = 50 * m_priceMultiplier; }
				// 25% chance
				else if (n > 60 && n <= 95) { tileAt(currentPos).object = std::make_shared<SilverChest>(this, j, i); tileAt(currentPos).price = 55 * m_priceMultiplier; }
				// 5% chance
				else if (n > 95) { tileAt(currentPos).object = std::make_shared<GoldenChest>(this, j, i); tileAt(currentPos).price = 120 * m_priceMultiplier; }

				m_items.push_back(tileAt(currentPos).object);
				tileAt(currentPos).boundary = true;
				
				addSprite(currentPos.x, currentPos.y, currentPos.y + Z_ITEM - 1, "Countertop_48x48.png");
				
				break;
			}
			}
		}
	}
}

void Shop::constructShopHUD(int x, int y) {
	// if there's a price already floating, remove before showing the new one
	if (m_scene->getPriceLabel() != nullptr)
		m_scene->deconstructShopHUD();

	m_scene->showShopHUD(x, y);
}
void Shop::deconstructShopHUD() {
	m_scene->deconstructShopHUD();
}

void Shop::purchaseItem(int x, int y) {
	std::shared_ptr<Objects> object = itemObject(x, y - 1);
	int playerGold = player[0]->getMoney();
	int price = tileAt(Coords(x, y - 1)).price;

	int items = player[0]->itemCount();
	int maxItems = player[0]->maxItemCount();

	// if player has enough money, let them buy it
	if (playerGold >= price && (!object->isUsable() || (object->isUsable() && items < maxItems))) {
		playSound("Purchase_Item1.mp3");

		player[0]->setMoney(playerGold - price);
		tileAt(Coords(x, y)).shop_action = ""; // removes purchase ability
		collectItem(x, y - 1);

		// remove HUD after purchasing
		m_scene->deconstructShopHUD();
	}
	else {
		playSound("Insufficient_Funds.mp3");
	}
}
void Shop::collectItem(int x, int y) {
	// NOTE: If coming from purchase item, y is equal to (py - 1) where py is the player's y position.
	//       If collectItem is called directly, then there is no y shift.

	bool itemAdded = false;
	
	std::string shop_action = tileAt(Coords(x, y)).shop_action;
	std::shared_ptr<Objects> object = (shop_action == CHEST_PURCHASED ? itemObject(x, y - 1) : itemObject(x, y));

	// if it's the stairs, then advance level and return
	if (exit(x, y)) {
		m_exit->interact();
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		playSound("Player_Movement.mp3");

		return;
	}

	if (object->isAutoUsable()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->useItem(*this, *getPlayerAt(x, y));
		playSound(drop->getSoundName());
		drop.reset();

		if (shop_action == CHEST_PURCHASED)
			removeItem(x, y - 1);
		else
			removeItem(x, y);
	}
	else if (object->isUsable()) {

		// For Spellcaster
		if (object->isSpell() && player[0]->getName() == SPELLCASTER) {
			player[0]->equipActiveItem(object);

			if (shop_action == CHEST_PURCHASED)
				removeItem(x, y - 1);			
			else
				removeItem(x, y);
			
			return;
		}

		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		player[0]->addItem(drop, itemAdded);
		drop.reset();
		
		if (!itemAdded) {
			playSound("Player_Movement.mp3");
			return;
		}
	}
	else if (object->isPassive()) {
		std::shared_ptr<Passive> passive = std::dynamic_pointer_cast<Passive>(object);
		player[0]->equipPassive(passive);
		passive.reset();

		itemAdded = true;
	}
	else if (object->isWeapon()) {
		std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
		player[0]->storeWeapon(weapon);
		playSound(weapon->getSoundName());
		
		weapon.reset();
	}
	else if (object->isChest()) {
		std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(object);
		chest->attemptOpen(*this);
		tileAt(Coords(x, y + 1)).shop_action = CHEST_PURCHASED;

		chest.reset();	
	}
	else if (object->isShield()) {
		if (player[0]->canUseShield())
			player[0]->equipActiveItem(object);		
	}

	if (itemAdded) {
		if (shop_action == CHEST_PURCHASED)
			removeItem(x, y - 1);		
		else
			removeItem(x, y);
	}
}

std::vector<char> Shop::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char c = 'c'; // shop countertop, where items will sit
	char k = 'k'; // shopkeeper
	char d = 'd'; // random breakable object
	// 1, 2, 3 : hp, shield repair, bombs
	// 4, 5 : random items
	// 6 : random chest
	// 'p' : stand here to purchase
	// '$' : insert price here

	std::vector<char> shop = {
	//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26
/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#',
/*3*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*4*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ', 'b', ' ', '#', '#',
/*5*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*6*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
/*7*/	'#', '#', '#', '#', '#', '#', '#', ' ', '$', '$', '$', ' ',  k , '$', ' ', '$', ' ', '$', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
/*8*/	'#', '#', '#', '#', '#', '#', '#',  c , '1', '2', '3',  c ,  c , '4',  c , '5',  c , '6',  c ,  c , '#', '#', '#', '#', '#', '#', '#',
/*9*/	'#',  d ,  d , ' ', ' ',  d , '#', ' ', 'p', 'p', 'p', ' ', ' ', 'p', ' ', 'p', ' ', 'p', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*10*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#',
/*11*/	'#', '@', ' ', ' ', ' ', ' ',  dv, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ',   s, ' ', ' ', '#',
/*12*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*13*/	'#',  d , ' ', ' ',  d ,  d , '#',  d ,  d , ' ', ' ',  d , ' ', ' ',  d , ' ', ' ', ' ',  d ,  d , '#', ' ', ' ', ' ',  d ,  d , '#',
/*14*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
		};

	return shop;
}


// ===========================================
//				:::: SHRINE ::::
// ===========================================
Shrine::Shrine(LevelScene* scene, std::shared_ptr<Player> p, int level) : Dungeon(scene, p, level, MAXROWS_SHRINE1, MAXCOLS_SHRINE1) {
	int rows = getRows();
	int cols = getCols();

	std::vector<char> v = generate();

	// Begin mapping
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Coords currentPos(j, i);

			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				tileAt(currentPos).hero = true;
				
				player[0]->setPos(currentPos);
				//player[0]->equipPassive(*this, std::make_shared<IronCleats>(0, 0));
				//player[0]->equipRelic(*this, std::make_shared<BrightStar>());
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->getRelic()->upgrade(*this, *player[0]);
				//player[0]->equipShield(*this, std::make_shared<WoodShield>());
				//player[0]->setVision(15);

				break;
			}
			case '#': {
				std::shared_ptr<Wall> wall = std::make_shared<Boundary>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
					  // Regular Wall
			case 'W': {
				std::shared_ptr<Wall> wall = std::make_shared<RegularWall>(this, j, i);
				m_walls.push_back(wall);
				tileAt(currentPos).wallObject = wall;
				break;
			}
			case DOOR_V: {
				addDoor(std::make_shared<RegularDoor>(this, j, i, DOOR_VERTICAL));
				break;
			}
			case STAIRS: {
				m_exit = std::make_shared<Stairs>(*this, j, i);
				addTrap(m_exit);

				break;
			}
						 // Water tile
			case 'w': {
				addTrap(std::make_shared<Water>(*this, j, i));
				
				break;
			}
					  // Dirt tile
			case 'D': {
				break;
			}
					  // Breakable object
			case 'B': {
				std::shared_ptr<Monster> object(nullptr);
				switch (randInt(0, 2)) {
				case 0:
					object = std::make_shared<WeakCrate>(this, j, i);
					break;
				case 1:
					object = std::make_shared<WeakBarrel>(this, j, i);
					break;
				case 2:
					object = std::make_shared<LargePot>(this, j, i);
					break;
				}

				m_monsters.push_back(object);

				break;
			}
					  // Arrow Sign
			case 'A': {
				std::shared_ptr<Monster> object = std::make_shared<ArrowSign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Regular Sign
			case 'R': {
				std::shared_ptr<Monster> object = std::make_shared<Sign>(this, j, i);
				m_monsters.push_back(object);

				break;
			}
					  // Outside Man 1
			case 'M': {
				std::shared_ptr<NPC> npc = std::make_shared<TutorialNPC1>(this, j, i);
				m_npcs.push_back(npc);

				break;
			}
					  // Lit Brazier
			case 't': {
				std::shared_ptr<Brazier> trap = std::make_shared<Brazier>(*this, j, i);
				litBraziers.push_back(trap);
				m_traps.emplace_back(trap);
				trap.reset();
				tileAt(currentPos).trap = true;
				tileAt(currentPos).wall = true;
				
				break;
			}
					  // Unlit Brazier
			case 'u': {
				std::shared_ptr<Brazier> trap = std::make_shared<Brazier>(*this, j, i);
				m_traps.emplace_back(trap);
				trap.reset();
				tileAt(currentPos).trap = true;
				tileAt(currentPos).wall = true;
				break;
			}
					  // Relic choice
			case 'T': {
				tileAt(currentPos).shrine_action = RELIC_CHOICE;
				break;
			}
					  // Recover Health
			case 'H': {
				tileAt(currentPos).shrine_action = HEALTH_CHOICE;
				break;
			}
					  // Chest choice
			case 'C': {
				tileAt(currentPos).shrine_action = CHEST_CHOICE;
				break;
			}
			default: break;
			}
		}
	}
}
Shrine::~Shrine() {
	for (auto &it : litBraziers)
		it.reset();
}

void Shrine::specialActions() {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();
	Coords playerCoords(player[0]->getPos());

	// Ignite the two Braziers if player has walked close enough
	if (!braziersLit && y <= 13) {
		playSound("Fireblast_Spell2.mp3");

		for (size_t i = 0; i < litBraziers.size(); ++i)
			litBraziers.at(i)->ignite();
		
		braziersLit = true;
	}

	// Check if player chose chests and if they opened any yet
	if (choseChests) {
		bool chestOpened = false;
		for (size_t i = 0; i < chestCoordinates.size(); ++i) {
			int n = chestCoordinates.at(i).x;
			int m = chestCoordinates.at(i).y;

			if (itemObject(n, m) && !itemObject(n, m)->isChest() || !itemObject(n, m)) {
				playSound("Guardians_Defeated.mp3");

				chestOpened = true;
				break;
			}
		}

		// Remove the other chests
		if (chestOpened) {
			for (unsigned int i = 0; i < chestCoordinates.size(); i++) {
				int n = chestCoordinates.at(i).x;
				int m = chestCoordinates.at(i).y;

				// Remove the chest
				if (itemObject(n, m) && itemObject(n, m)->isChest()) {
					removeItem(n, m);

					tileAt(Coords(n, m)).wall = false;
				}
			}
			choseChests = false;
		}
	}

	// Check if player walked off the spot where they can access the shrine
	if (tileAt(playerCoords).shrine_action == "") {
		if (makingChoice)
			makingChoice = false;
	}

	if (tileAt(playerCoords).shrine_action != "" && !makingChoice) {
		m_scene->callShrineChoice(tileAt(playerCoords).shrine_action);
		makingChoice = true;
	}
}

void Shrine::useChoice(int index) {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();
	Coords playerCoords(player[0]->getPos());

	std::string shrine = tileAt(playerCoords).shrine_action;

	if (shrine == HEALTH_CHOICE) {
		switch (index) {
			// Wealth
		case 0:
			if (player[0]->getMoney() >= 200) {
				playSound("Devils_Gift.mp3");
				player[0]->setMoney(player[0]->getMoney() - 200);

				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Item (Passive or all items)
		case 1:
			if (player[0]->hasItems()) {
				playSound("Devils_Gift.mp3");

				if (player[0]->hasPassives())
					player[0]->removePassive(randInt(0, (int)player[0]->passiveCount() - 1));
				else
					player[0]->removeItems();

				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Weapon
		case 2:
			if (sacrificeWeapon()) {
				playSound("Devils_Gift.mp3");
				player[0]->decreaseFavorBy(2);
			}
			else
				index = -1;

			break;
			// Soul (Max HP)
		case 3: {
			// Lose 25% of max hp capacity
			playSound("Devils_Gift.mp3");
			playGotHit();

			int newMaxHP = player[0]->getMaxHP() * 0.75;
			player[0]->setMaxHP(newMaxHP);

			if (player[0]->getHP() > player[0]->getMaxHP())
				player[0]->setMaxHP(player[0]->getMaxHP());

			player[0]->decreaseFavorBy(2);
			break;
		}
		default: index = -1;
		}

		// Restore health to max if they chose to
		if (index != -1)
			player[0]->setHP(player[0]->getMaxHP());
		
	}
	else if (shrine == RELIC_CHOICE) {

		// Choices when player already has a relic
		if (player[0]->hasRelic()) {
			switch (index) {
				// Upgrade
			case 0: {
				playSound("Devils_Gift.mp3");
				upgradeRelic();
				break;
			}
				// Switch
			case 1: {
				playSound("Devils_Gift.mp3");
				swapRelic();
				break;
			}
			default: index = -1;
			}
		}
		// One choice if player has no trinket
		else {
			switch (index) {
				// Acquire
			case 0: {
				playSound("Devils_Gift.mp3");
				giveRelic();
				break;
			}
			default: index = -1;
			}
		}
	}
	else if (shrine == CHEST_CHOICE) {
		switch (index) {
		case 0: {
			playSound("Devils_Gift.mp3");

			spawnChests();
			choseChests = true;
			break;
		}
		default: index = -1;
		}
	}

	// If player accepted an offer
	if (index != -1) {

		// Light the brazier to indicate its use
		if (trap(x, y - 1)) {
			std::vector<int> indexes = findTraps(x, y - 1);
			for (size_t i = 0; i < indexes.size(); ++i) {
				if (m_traps.at(indexes[i])->canBeIgnited())
					m_traps.at(indexes[i])->ignite();
			}
		}

		tileAt(playerCoords).shrine_action = ""; // Remove prompt ability
	}
}
bool Shrine::sacrificeWeapon() {
	if (player[0]->getWeapon()->getName() == HANDS && player[0]->getStoredWeapon() == nullptr)
		return false;

	// Only possesses 1 weapon
	if (player[0]->getWeapon()->getName() == HANDS && player[0]->getStoredWeapon() != nullptr ||
		player[0]->getStoredWeapon() == nullptr) {

		if (player[0]->getStoredWeapon() != nullptr)
			player[0]->switchWeapon(); // Swap usable weapon into their hand

		player[0]->equipWeapon(std::make_shared<ShortSword>());
	}
	// Possesses 2 weapons
	else {
		// Choose one at random and remove it
		if (randReal(1, 100) > 50)
			player[0]->switchWeapon();

		player[0]->removeStoredWeapon();	
	}

	return true;
}
void Shrine::giveRelic() {
	player[0]->decreaseFavorBy(4);
	player[0]->equipRelic(rollRelic(this, Rarity::COMMON));
}
void Shrine::upgradeRelic() {
	playSound("Potion_Pickup.mp3");
	player[0]->getRelic()->upgrade(*this, *player[0]);

	player[0]->decreaseFavorBy(2);
}
void Shrine::swapRelic() {
	int prevRelicLevel = player[0]->getRelic()->getLevel();

	std::shared_ptr<Relic> relic = rollRelic(this, Rarity::COMMON);

	player[0]->removeRelic();
	relic->setLevel(std::max(1, prevRelicLevel - 1));
	player[0]->equipRelic(relic);

	player[0]->decreaseFavorBy(3);
}
void Shrine::spawnChests() {
	int x = player[0]->getPosX();
	int y = player[0]->getPosY();

	int n = x - 1; int m = y + 1;
	for (int i = x - 1; i < x + 2; i++) {

		n = i;
		m = y + 1;

		if (itemObject(n, m))
			itemHash(n, m);

		/*std::shared_ptr<Objects> objects(nullptr);
		switch (randInt(0, 5)) {
		case 0: objects = std::make_shared<BrownChest>(this, n, m); break;
		case 1: objects = std::make_shared<SilverChest>(this, n, m); break;
		case 2: objects = std::make_shared<GoldenChest>(this, n, m); break;
		case 3: objects = std::make_shared<LifeChest>(this, n, m); break;
		case 4: objects = std::make_shared<ExplodingChest>(this, n, m); break;
		case 5: objects = std::make_shared<HauntedChest>(this, n, m); break;
		}*/

		tileAt(Coords(n, m)).object = rollChest(this, n, m, Rarity::ALL, true);
		m_items.push_back(tileAt(Coords(n, m)).object);

		chestCoordinates.push_back(Coords(n, m));
	}

	player[0]->decreaseFavorBy(2);
}

std::vector<char> Shrine::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char r = 'r'; // Rock
	char k = 'k'; // 
	char d = 'd'; //
	// 'W' : Regular Wall
	// 'B' : Breakable object
	// 'D' : Dirt
	// 'T' : Trinkets
	// 'H' : Health
	// 'C' : Chest choice
	// 't' : Lit Brazier
	// 'u' : Unlit Brazier

	/**
	* RegEx: '#',(( '[A-Za-z#@ ]{1}',){24}) '#',
	*/

	std::vector<char> level = {
		//       0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
		/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		/*1*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*2*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*3*/	'#', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*4*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'u', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*5*/	'#', 'W', 'W', 'W', 'W', 'W', 'B', 'H', ' ', 'W', 'W', ' ', 'T', ' ', 'W', 'W', ' ', 'C', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*6*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', ' ', ' ', ' ', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*7*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', 'W', 'W', ' ', 'W', 'W', ' ', ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*8*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 't', 'W', ' ', 'W', 't', ' ', ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*9*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*10*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'B', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*11*/	'#', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', ' ', ' ',  s , ' ', ' ', 'B', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*12*/	'#', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ', 'B', 'B', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', '#',
		/*13*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'B', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*14*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', '#',
		/*15*/	'#', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*16*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*17*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*18*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*19*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', 'W', 'W', 'W', '@', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*20*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*21*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*22*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*23*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*24*/	'#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#',
		/*25*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
	};

	return level;
}


// ==============================================
//				:::: 1ST FLOOR ::::
// ==============================================
FirstFloor::FirstFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIRST_FLOOR, MAXROWS, MAXCOLS) {

	// BEGIN LEVEL GENERATION
	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec);
	/// END LEVEL GENERATION

	generateNPCs();
	generateMonsters();
	generateTraps();
	generateItems();
}

// Level generation
std::vector<std::string> FirstFloor::topChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> one =
	{ {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  i_bc,  sp, wll, wll},
	  {  sp, m_se,  t_f,  t_p, wll,   sp},
	  {  sp, m_se,  m_z,   sp,   _dv,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> two =
	{ {wll, t_tr,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll, wll,   sp,  PIT,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> three =
	{ {wll,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp, t_df,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> four =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {wll,   sp,  m_k,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> five =
	{ {wll, i_ri, wll, wll, t_td,   sp},
	  {t_tr,   sp,   sp,   sp,   sp,   sp},
	  {wll,  PIT, wll,   sp,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> six =
	{ {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s} };

	std::vector<std::vector<std::string>> seven =
	{ {wll, wll, PIT, wll,  i_gd, wll},
	  {t_df, wll,   sp, wll,   sp, wll},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp, wll,  PIT, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> eight =
	{ {t_td,   sp,   sp,   sp,   sp, t_td},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, t_tl, wll, wll, t_tr,   sp},
	  {  sp, wll, wll, wll, wll,   sp} };

	std::vector<std::vector<std::string>> nine =
	{ {  sp,   sp,  t_f, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll, t_mb},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,  t_f,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ten =
	{ {wll, wll,   sp, wll, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {  sp, wll, i_gd, wll,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> eleven =
	{ {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {wll,   sp, i_gd, r_wl,   sp, r_wl},
	  {  sp,   sp, wll,   sp,  t_s,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp, r_wl},
	  {wll,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,  wll,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  { wll,  t_p,   sp,   sp,   sp,  wll},
	  { wll,   sp,   sp,   sp,   sp,  wll},
	  {  sp,   sp,   sp, t_ts,  wll,   sp},
	  {  sp,   sp,  wll,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ { t_sk, t_sk, i_rc, i_gd, t_sk, t_sk},
	  { t_sk, t_sk, t_cf, t_cf, t_sk, t_sk},
	  {   sp, t_sk, t_sk, t_sk, t_sk,   sp},
	  {   sp,   sp, t_sk, t_sk,   sp,   sp},
	  {   sp,   sp,   sp,   sp,   sp,   sp},
	  {t_str,   sp,   sp,   sp,   sp,t_stl} };

	std::vector<std::vector<std::string>> xiv =
	{ {  wll,   sp,   sp,   sp,t_sbr,  wll},
	  {t_str,  wll,   sp,   sp,  wll,   sp},
	  {   sp,   sp,  wll,  wll,   sp,   sp},
	  {   sp,  t_s,  wll,  wll,   sp,  t_s},
	  {   sp,  wll,   sp,t_str,  wll,   sp},
	  {  wll,   sp,   sp,   sp,   sp,  wll} };

	std::vector<std::vector<std::string>> xv =
	{ { wll, i_gd,   sp,  wll, t_td,  wll},
	  { wll,  wll,   sp,  wll,   sp,  wll},
	  { wll,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,t_mbh,   sp,  t_p,   sp},
	  {  sp,   sp, t_tu,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xvi =
	{ {   sp,   sp, t_sk,   sp,   sp,  wll},
	  {   sp,   sp,t_mbv,   sp,   sp,t_mbv},
	  {   sp,   sp,   sp, i_ri,t_mbh,   sp},
	  {   sp,   sp, t_cf, m_gs,   sp,   sp},
	  {t_mbh, t_pt,   sp,   sp, t_ts,   sp},
	  {   sp,t_mbv,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ { wll,   sp,  m_rm, i_rw, m_gs,  wll},
	  { _dv, m_rm,  m_rm, m_rm,  sp,  wll},
	  { wll, t_ts,   sp,   sp, i_gd,  wll},
	  { wll,   sp, m_br,   sp,  t_s,  wll},
	  { wll,  wll,  wll,  _dh,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xviii =
	{ {  sp,  wll,  wll, i_ri,  wll,  wll},
	  { wll,   sp,  wll,  wll, r_wl,  wll},
	  {  sp,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,  wll},
	  {  sp,   sp,  wll,   sp, r_wl,  wll},
	  {  sp,  wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xix =
	{ { unb,  _x_,  unb, i_gc,  t_f,  unb},
	  { _dv, t_as,  unb,  _x_,   sp,  unb},
	  { unb,  _dh,  unb,  _x_,   sp,  unb},
	  { unb,t_mbh,  _x_,  _x_,  wll,  unb},
	  { unb,  _x_, t_su,  _x_,  unb,  unb},
	  { unb,  unb,  unb,  unb,  wll,  wll} };

	std::vector<std::vector<std::string>> xx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  { wll, t_tr,   sp,   sp, t_cf,  wll},
	  { wll, t_tr,   sp,   sp, t_cf,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	/*c.push_back(xx);
	c.push_back(xxi);
	c.push_back(xxii);
	c.push_back(xxiii);
	c.push_back(xxiv);
	c.push_back(xxv);
	c.push_back(xxvi);*/

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> FirstFloor::middleChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {r_wl,   sp, r_wl,   sp, r_wl,   sp},
	  {  sp,   sp, t_ts, t_ts,   sp,   sp},
	  {  sp, r_wl, t_ts, t_ts,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp, r_wl},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll, wll},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {t_td,   sp, r_wl,   sp, wll,   sp},
	  {  sp, wll,   sp, wll, t_df,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp, r_wl,   sp,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, r_wl,   sp},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, i_gd, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {wll, wll,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,  t_p,   sp,   sp, wll,   sp},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp, t_tl},
	  { t_f, wll, wll, wll,   sp, wll},
	  {wll,   sp,   sp, r_wl,   sp, wll},
	  {  sp,   sp, r_wl, r_wl,   sp, wll},
	  {t_tr,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp, r_wl, t_f,   sp, r_wl},
	  {r_wl, r_wl,   sp, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl,   sp, r_wl,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl, r_wl},
	  {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl, t_f, r_wl, r_wl} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,  PIT,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,  PIT,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  t_s,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp, wll,   sp,   sp, r_wl, r_wl},
	  {  sp,   sp, wll,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp, wll, i_ri, wll, wll,   sp},
	  {wll, wll, t_ts, wll, wll,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll, i_bc, wll,   sp},
	  {  sp, wll, m_rm,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {  sp, wll,   sp, t_tu,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp,   sp, r_wl, r_wl, t_mb,   sp},
	  {  sp,  t_s, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {wll, r_wl,   sp,   sp,   sp, t_td},
	  {  sp,   sp, r_wl,   sp,   sp,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, t_df, wll, wll, wll, t_tu} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, t_tl},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp, t_ts, t_mb,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,  t_s, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {wll, r_wl,   sp, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, i_gd, wll, r_wl, wll},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> xviii =
	{ {wll, r_wl, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, r_wl, wll, i_gd, wll, wll},
	  {wll, wll, r_wl, wll, wll, wll},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll, wll,   sp, wll} };

	std::vector<std::vector<std::string>> xix =
	{ {wll,   sp, wll, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp, wll},
	  {wll, wll,   sp,   sp, wll, wll},
	  {  sp,   sp,   sp,   sp, r_wl, wll},
	  {wll, wll, wll,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> xx =
	{ {wll,   sp,   sp, r_wl,   sp,   sp},
	  {wll,   sp,   sp, r_wl, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp, wll,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> xxii =
	{ {  sp,   sp,  wll,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  { wll,  _x_,  _x_, m_gs,  wll,   sp},
	  { wll,  _x_,  wll, i_gd,  wll,   sp},
	  { wll,  _x_,  _x_,  wll,   sp,   sp},
	  {  sp,  wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,t_sbr,  _x_,t_sbl,   sp},
	  {  sp,   sp,  _x_, t_pt,  _x_,   sp},
	  {  sp,   sp,t_str,  _x_,t_stl,   sp},
	  {r_wl,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxiv =
	{ {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,  wll, t_sk,  wll,  wll,  wll},
	  {  sp,  wll, t_ts, t_ts, i_ri,  wll},
	  {  sp,  _x_, t_ts,  wll,  wll,   sp},
	  {  sp,  wll,  _x_,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,  wll,   sp} };

	std::vector<std::vector<std::string>> xxv =
	{ {  sp,   sp,   sp, t_td,   sp,   sp},
	  {  sp,  t_s,   sp,  t_p,   sp,   sp},
	  {  sp,   sp,   sp, t_ts,   sp,   sp},
	  {  sp,  wll, t_sk, i_ri, t_sk,  wll},
	  {  sp,   sp,   sp, t_sk,   sp,   sp},
	  {  sp,  t_s,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvi =
	{ {r_wl,  wll,   sp,   sp,  wll,   sp},
	  { wll,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  t_p,   sp,   sp,   sp},
	  {  sp,   sp,t_mbv,   sp,  t_s,   sp},
	  {  sp,   sp, t_pt,   sp,  wll,   sp},
	  { wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxvii =
	{ {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,  wll,   sp,   sp,   sp,   sp},
	  { wll,   sp,   sp,  wll,   sp,   sp},
	  {  sp,   sp, t_tl,  wll,   sp,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxviii =
	{ {t_tr,   sp,   sp,   sp,   sp, t_td},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {t_tu,   sp,   sp,   sp,   sp, t_tl} };

	std::vector<std::vector<std::string>> xxix =
	{ {t_td,  t_p,   sp,   sp,  t_p, t_tl},
	  {  sp,   sp,   sp, t_sk,   sp,   sp},
	  {  sp,   sp, t_pt,   sp,   sp,  t_s},
	  { t_p,   sp,   sp, t_sk,   sp,   sp},
	  {  sp, t_sk,   sp,   sp, t_ts,  t_p},
	  {t_tr,   sp,  t_p,   sp,   sp, t_tu} };

	std::vector<std::vector<std::string>> xxx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,  wll,  t_s,   sp},
	  { wll, t_ts,  wll,  wll,  wll,   sp},
	  { wll,  wll, i_bc,  wll,  wll,   sp},
	  { wll, t_ts,  wll, t_ts, t_ts,   sp},
	  {  sp,  wll,  wll, t_ts,   sp, t_tu} };

	/*std::vector<std::vector<std::string>> xxxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,  t_s,  t_s,  t_s,   sp},
	  {  sp,  t_s,  t_s,  t_s,   sp,   sp},
	  {  sp,  t_s,  t_s,   sp,  t_s,   sp},
	  {  sp,   sp,  t_s,  t_s,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };*/

	std::vector<std::vector<std::string>> xxxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };


	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	c.push_back(xxii);
	c.push_back(xxiii);
	c.push_back(xxiv);
	c.push_back(xxv);
	c.push_back(xxvi);
	c.push_back(xxvii);
	c.push_back(xxviii);
	c.push_back(xxix);
	c.push_back(xxx);
	c.push_back(xxxi);

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> FirstFloor::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, t_s,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {t_tr,   sp,   sp,   sp,   sp, wll},
	  {wll, i_bc, wll, wll,   sp, r_wl} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp, wll, wll, wll,   sp, wll},
	  {  sp, wll,   sp,  m_g, wll, r_wl},
	  {  sp, wll,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll,   sp},
	  {wll,  PIT,   sp, wll, wll,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, t_s, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, t_s, wll, wll},
	  {wll, wll,   sp,   sp, r_wl, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll, i_gd, i_ri, i_gd,   sp, wll} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp, i_ri, wll, wll},
	  { m_z,   sp, t_tu, t_tu, wll, wll} };

	std::vector<std::vector<std::string>> v =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,  t_f, wll,   sp, r_wl, r_wl},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp,   sp,   sp},
	  {wll, wll,   sp, wll,   sp,   sp},
	  {wll, r_wl, wll, wll,   sp,   sp},
	  {wll, i_ri, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp, wll, wll},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {wll, i_gd,   sp, r_wl,   sp, wll},
	  {  sp, wll,   sp, t_mb,   sp, r_wl},
	  {  sp,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp, wll,   _dh, wll,   sp,   sp},
	  {wll, wll,   sp, wll, wll,   sp},
	  {wll, wll,   sp, wll, wll, r_wl},
	  {wll,   sp,   sp,   sp, wll, wll},
	  {wll, wll,   sp, m_rm, m_rm, wll},
	  {wll, wll, m_rm, m_rm, m_rm, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp, wll, wll, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll, wll,   sp},
	  {wll, wll, t_ts, wll, t_ts,   sp},
	  {wll,   sp, t_ts, t_ts, t_ts, wll},
	  {wll, t_ts, t_ts,   sp, t_ts, wll} };

	std::vector<std::vector<std::string>> x =
	{ {r_wl,   sp,   sp,   sp, wll,   sp},
	  {  sp,   sp, r_wl,   sp, t_mb, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,  t_s, wll, wll,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll, wll, t_mb,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl, wll, wll, wll, wll, r_wl} };

	std::vector<std::vector<std::string>> xii =
	{ {wll,   sp,   sp,   sp,   sp, r_wl},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, i_gd, wll, t_f,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_mb,   sp,   sp},
	  {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, m_g,  m_g, wll, wll},
	  {wll, wll, i_rc, i_sc, wll, wll} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp, wll, wll,   _dh, wll},
	  {  sp, wll, m_rm, wll,   sp, wll},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   _dv,   sp, wll, r_wl, wll},
	  {  sp, wll,   sp,   _dv,   sp,   sp},
	  { t_s, wll, wll, wll, wll,  t_f} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xviii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };


	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	/*c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);*/

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::vector<std::vector<std::string>>> FirstFloor::mixChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	int cols = getCols();

	std::string x = SPAWN_DISALLOWED;
	std::string play = PLAYER;
	std::string exit = STAIRCASE;


	std::vector<std::vector<std::vector<std::string>>> v;

	std::vector<std::vector<std::string>> pChunk =
	{ {   x,    x, wll, wll, r_wl,    x},
	  {wll,    x,    x,    x,    x,    x},
	  {   x,    x, play,    x,    x, wll},
	  {wll,    x,    x,    x,    x, wll},
	  {r_wl,    x,    x,    x,    x, wll},
	  {   x, wll, wll,    x, wll,    x} };

	std::vector<std::vector<std::string>> eChunk =
	{ {   x,    x, r_wl, r_wl, r_wl,    x},
	  {r_wl, m_rm, m_rm, m_rm,    x,    x},
	  {   x, m_rm, exit, m_rm,    x, r_wl},
	  { wll, m_rm, m_rm, m_rm, r_wl,  wll},
	  {r_wl,    x,    x,    x,    x,  wll},
	  {   x,    x, r_wl,    x,   wll,   x} };

	std::vector<std::vector<std::string>> specialRoom1 =
	{ {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  m_g,   sp, wll, wll},
	  {wll, wll, i_sc, i_sc, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, wll, wll, wll,   sp}, };

	std::vector<std::vector<std::string>> specialRoom2 =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };



	for (int i = 0; i < (cols - 2) / 6 /*number_of_columns - 2 / length_of_each_chunk*/; i++) {
		if ((layer - 1) * 6 + i == playerChunk)
			v.push_back(pChunk);
		else if ((layer - 1) * 6 + i == exitChunk)
			v.push_back(eChunk);
		else if ((layer - 1) * 6 + i == specialChunk1)
			v.push_back(specialRoom1);
		else if ((layer - 1) * 6 + i == specialChunk2)
			v.push_back(specialRoom2);
		else
			v.push_back(c[randInt(0, (int)c.size() - 1)]);
	}

	/*if (layer == specialChunkLayer1) {
		s = randInt(0, (int)v.size() - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom1);
	}
	if (layer == specialChunkLayer2) {
		s = randInt(0, (int)v.size() - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom2);
	}*/

	//// put in player chunk
	//v.erase(v.begin() + playerChunk);
	//v.emplace(v.begin() + playerChunk, pChunk);

	return v;
}
std::vector<std::string> FirstFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (unsigned int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 6; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}

std::vector<std::string> FirstFloor::generateLevel() {
	int rows = getRows();
	int cols = getCols();

	std::vector<std::vector<std::vector<std::string>>> b;
	std::vector<std::string> v, finalvec;


	// player spawns in middle with special chunk
	int n = randInt(1, 4);
	switch (n) {
	case 1: playerChunk = 14; break;
	case 2: playerChunk = 15; break;
	case 3: playerChunk = 20; break;
	case 4: playerChunk = 21; break;
	}

	// exit spawns in one of four corners
	n = randInt(1, 4);
	switch (n) {
	case 1: exitChunk = 0; break;
	case 2: exitChunk = 5; break;
	case 3: exitChunk = 30; break;
	case 4: exitChunk = 35; break;
	}

	// chooses layer for the special chunks to be on
	specialChunk1 = randInt(0, 35);
	while (specialChunk1 == playerChunk || specialChunk1 == exitChunk) {
		specialChunk1 = randInt(0, 35);
	}

	specialChunk2 = randInt(0, 35);
	while (specialChunk2 == playerChunk || specialChunk2 == exitChunk) {
		specialChunk2 = randInt(0, 35);
	}

	specialChunkLayer1 = randInt(1, 6);
	specialChunkLayer2 = randInt(1, 6);
	layer = 1;

	int count = 0;
	while (count < (rows - 2) / 6 /* divided by length of chunk*/) {
		if (count == 0) {
			v = topChunks(b);
			layer++;
		}
		else if (count >= 1 && count <= 4) {
			v = middleChunks(b);
			layer++;
		}
		else if (count == 5) {
			v = bottomChunks(b);
			layer++;
		}

		for (unsigned int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		b.clear();
		count++;
	}

	return finalvec;
}


// ==============================================
//				:::: 2ND FLOOR ::::
// ==============================================
SecondFloor::SecondFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SECOND_FLOOR, MAXROWS2, MAXCOLS2), m_watersCleared(true), m_guardiansLeft(-1) {
	int rows = getRows();
	int cols = getCols();

	// :::: GENERATE LEVEL ::::
	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec);
	// ---- END LEVEL GENERATION ----


	//	BUTTON for unlocking exit
	std::shared_ptr<Traps> button = std::make_shared<Button>(*this, cols / 2 - 1, rows / 2 - 6);
	m_traps.push_back(button);


	// GENERATE EXTRA MONSTERS
	int n, x, y;
	std::shared_ptr<Monster> monster(nullptr);
	for (int i = 0; i < 18; i++) {

		//	selects how many of a type of monster to place
		switch (i) {
		case 0: n = 8 + randInt(0, 4); break; // 
		case 1: n = 5 + randInt(0, 5); break; //
		case 2: n = 6 + randInt(0, 8); break; // 
		case 3: n = 15 + randInt(0, 10); break; // 
		case 4: n = 1 + randInt(0, 2); break; // 
		case 5: n = 2 + randInt(0, 2); break; // 
		case 6: n = 5 + randInt(0, 3); break; // 
		case 7: n = 14 + randInt(0, 3); break; // 
		case 8: n = 5 + randInt(0, 4); break; //
		case 9: n = 8 + randInt(0, 4); break; //
		case 10: n = 12 + randInt(0, 4); break; //
		case 11: n = 8 + randInt(0, 4); break; //
		case 12: n = 4 + randInt(0, 4); break; //
		case 13: n = 8 + randInt(0, 4); break; //
		case 14: n = 4 + randInt(0, 4); break; //
		case 15: n = 1 + randInt(0, 1); break; //
		case 16: n = 10 + randInt(0, 4); break; //
		case 17: n = 4 + randInt(0, 4); break; //
		}

		while (n > 0) {

			x = randInt(1, cols - 2);
			y = randInt(1, rows - 2);

			while (noSpawn(x, y) || wall(x, y) || enemy(x, y) || hero(x, y) || trap(x, y)) {
				x = randInt(1, cols - 2);
				y = randInt(1, rows - 2);
			}

			// the type of enemy
			switch (i) {
			case 0:
				monster = std::make_shared<Wanderer>(this, x, y); break;
			case 1:
				monster = std::make_shared<RabidWanderer>(this, x, y); break;
			case 2:		
				monster = std::make_shared<ProvocableWanderer>(this, x, y); break;
			case 3:
				monster = std::make_shared<FlameWanderer>(this, x, y); break;
			case 4:
				monster = std::make_shared<Bombee>(this, x, y); break;
			case 5:
				monster = std::make_shared<CharredBombee>(this, x, y); break;
			case 6:
				monster = std::make_shared<Roundabout>(this, x, y); break;
			case 7:
				monster = std::make_shared<FireRoundabout>(this, x, y); break;
			case 8:
				monster = std::make_shared<Zapper>(this, x, y); break;
			case 9:
				monster = std::make_shared<Spinner>(this, x, y); break;
			case 10:
				monster = std::make_shared<Puff>(this, x, y); break;
			case 11:
				monster = std::make_shared<GustyPuff>(this, x, y); break;
			case 12:
				monster = std::make_shared<StrongGustyPuff>(this, x, y); break;
			case 13:
				monster = std::make_shared<InvertedPuff>(this, x, y); break;
			case 14:
				monster = std::make_shared<SpikedInvertedPuff>(this, x, y); break;
			case 15:
				monster = std::make_shared<ItemThief>(this, x, y); break;
			case 16:
				monster = std::make_shared<Serpent>(this, x, y); break;
			case 17:
				monster = std::make_shared<Charger>(this, x, y); break;
			}

			addMonster(monster);

			n--;
			monster.reset();
		}
	}
	
}

void SecondFloor::specialActions() {
	// check if player entered devil's waters and killed all guardians
	for (int i = m_guardians.size() - 1; i >= 0; i--) {
		if (m_guardians[i]->isDead()) {
			m_guardians[i].reset();
			m_guardians.erase(m_guardians.begin() + i);
			m_guardiansLeft--;
			if (m_guardiansLeft == 0)
				m_watersCleared = true;
		}
	}

	if (m_guardiansLeft == 0 && m_watersCleared) {
		guardiansDefeated();

		m_guardiansLeft = -1; // prevents if statement from being entered again
	}
}
bool SecondFloor::specialTrapCheck(int x, int y) {
	std::vector<int> indexes = findTraps(x, y);
	for (int i = 0; i < static_cast<int>(indexes.size()); ++i) {
		if (m_traps.at(indexes[i])->getName() == BUTTON_UNPRESSED) {
			if (m_exit->isLocked()) {
				playSound("Button_Pressed.mp3");
				playSound("Exit_Opening.mp3");

				m_exit->toggleLock();

				// Replace button with pressed button
				m_traps.at(indexes[i])->getSprite()->setSpriteFrame("Button_Pressed_48x48.png");

				// Replace staircase sprite
				m_exit->getSprite()->setSpriteFrame("Stairs_48x48.png");
			}
		}
		else
			trapEncounter(x, y);
	}

	return true;
}

void SecondFloor::devilsWater(bool bathed) {
	if (bathed) {
		m_waterPrompt = false;
		devilsGift();
	}
	m_waterPrompt = false;
}
void SecondFloor::devilsGift() {
	m_watersUsed = true;

	player[0]->increaseStatBy(StatType::MAXHP, 30);
	player[0]->addAffliction(std::make_shared<HealOverTime>(30));

	std::shared_ptr<Monster> g1, g2, g3, g4;
	g1 = std::make_shared<ArmoredSerpent>(this, 31, 27);
	g2 = std::make_shared<ArmoredSerpent>(this, 31, 28);
	g3 = std::make_shared<ArmoredSerpent>(this, 42, 27);
	g4 = std::make_shared<ArmoredSerpent>(this, 42, 28);
	
	g1->setName("Guardian");
	g2->setName("Guardian");
	g3->setName("Guardian");
	g4->setName("Guardian");

	m_monsters.push_back(g1); m_guardians.push_back(g1);
	m_monsters.push_back(g2); m_guardians.push_back(g2);
	m_monsters.push_back(g3); m_guardians.push_back(g3);
	m_monsters.push_back(g4); m_guardians.push_back(g4);

	// Close off the exits with walls after accepting the devils gift
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			m_walls.push_back(std::make_shared<UnbreakableWall>(this, i, j));
		}
	}

	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			m_walls.push_back(std::make_shared<UnbreakableWall>(this, i, j));
		}
	}

	m_watersCleared = false;
	m_guardiansLeft = 4;
}
void SecondFloor::guardiansDefeated() {
	playSound("Guardians_Defeated.mp3");

	// remove the blocked walls
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			removeWall(i, j);
		}
	}
	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			removeWall(i, j);
		}
	}
}
void SecondFloor::devilsWaterPrompt() {
	m_waterPrompt = true;

	m_scene->callDevilsWaters();
}



std::vector<std::vector<std::string>> SecondFloor::outermostChunks() {

	std::vector<std::vector<std::string>> i = 
	{ { wll,  wll,  wll, i_gd,  wll,   sp,  wll,  wll},
	  { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   xx, t_cl,   xx},
	  {  sp,   xx,  t_f,   xx,   sp,   sp,  wll,   sp},
	  { wll,   xx,   xx,   xx,  wll, t_lv,  wll,   sp},
	  {  sp,  wll,   sp,  wll, i_gd,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ii = 
	{ {r_wl,   sp, t_td,  wll,  wll,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp, t_lv, t_lv,  t_s, t_mb,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> iii = 
	{ { wll,  wll,   sp,   sp, t_cl,   sp,  wll,   sp},
	  {  sp,  wll,   sp,  wll,   sp,   sp,   sp,  wll},
	  {  sp,   sp,   sp,  t_f,  wll,   sp,   sp, r_wl},
	  {  sp,   sp,  wll, i_gd,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp,   sp,   sp},
	  { wll,   sp,   sp, t_cl,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll,   sp, i_gd,  wll,  wll,   sp,   sp,  wll},
	  { wll, i_gd,   sp,  wll,  wll, i_ri,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll, t_tr,   sp, t_cl, t_cl,   sp,   sp,  wll},
	  { wll,   sp,   sp, t_cl, t_cl,   sp, t_tl,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi = 
	{ {  sp,  t_s,   sp,  wll,  wll,  m_z,  m_z,  wll},
	  { wll,  wll,   sp,  wll,  wll,   xx,   xx,   sp},
	  {m_sp, t_cl,   sp,  wll, t_cl,   xx,  wll,   sp},
	  { wll,  wll,   xx, t_cl,   xx,   xx,  wll,   sp},
	  {  sp,  t_s,   sp,   xx,  t_f,   sp, t_sk, r_wl},
	  {  sp,   sp,   sp,   sp,  wll,   sp, t_sk,   sp} };

	std::vector<std::vector<std::string>> vii = 
	{ { wll,   sp,   sp,   sp,  t_s,   sp,  m_z,   sp},
	  {  sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,   xx,   sp,   sp},
	  {  sp,  t_s,   sp,   sp, r_wl, t_cl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   xx,  wll,   sp},
	  {  sp,  m_z,  t_s, t_mb, t_cl, t_cl, t_cl,  wll} };

	std::vector<std::vector<std::string>> viii = 
	{ {t_td,   sp, t_td,   sp, t_td,   sp, t_td,   sp},
	  {  sp,   sp,   sp,  t_s,   sp,  t_s,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp},
	  {  sp, t_tu,   sp, t_tu,   sp, t_tu,   sp, t_tu} };

	std::vector<std::vector<std::string>> ix = 
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {  sp,   sp,   sp,   sp,   sp, m_sp,   sp,   sp},
	  {  sp,   sp, m_sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp, t_lv, t_lv, t_cl, t_lv,   sp,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_cl, t_cl, t_lv, t_cl, t_cl,   sp},
	  {  sp, t_cl, t_cl, t_lv, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_lv, t_lv,   sp},
	  {  sp,   sp, t_lv, t_cl, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> xi = 
	{ {  sp,   sp, t_lv, i_bc,   sp, t_lv,   sp,   sp},
	  {  sp,   sp, t_lv, t_lv,   sp, t_lv,   sp,   sp},
	  {  sp, m_sp,   sp, t_lv, t_sk, t_lv, m_sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_tl, t_tr,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii = 
	{ {  sp, t_cl,   sp,t_mbv, t_cl, t_lv, t_cl,   sp},
	  {  sp,t_mbv, t_cl, t_cl,   sp, t_cl, t_cl,   sp},
	  {  sp,   sp, t_cl,   sp,t_mbv, t_cl,   sp,   sp},
	  {  sp,   sp,t_mbv,   sp, t_cl, t_cl,   sp,   sp},
	  {  sp, t_cl,   sp,   sp, t_cl, t_cl,t_mbv,   sp},
	  {  sp, t_cl,   sp,t_mbv,   sp,   sp, t_cl,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/


	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> SecondFloor::edgeChunks() {
	std::vector<std::vector<std::string>> i =
	{ { wll,   sp,  wll,  wll,  wll,   xx,   xx,   xx},
	  { wll,  wll,   sp,   sp,   sp,  wll,   xx,  wll},
	  {  sp,   sp,   xx,   sp,   sp,   sp,   xx,   sp},
	  { wll,   sp,   xx,  wll,  wll,   sp,   sp, r_wl},
	  { wll,  wll,   xx,   sp,   sp,  wll,  wll,   sp},
	  {r_wl,   sp,  wll,   sp,   sp,   sp,   sp,  wll} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_rc},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_gd},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  {i_gd,   sp, m_sp, t_cl, t_cl,   sp,  m_k,  wll},
	  {i_rc,   sp, m_sp, t_cl, t_cl,   sp,  m_a,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,  wll, t_cl,   sp,  wll,   sp},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  wll, m_sp,  wll,   sp,  wll, m_sp},
	  { wll, t_lv,   sp,   sp,   sp, t_cl,   sp,   sp},
	  {  sp,  wll,   sp,   sp, t_cl,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,  wll,   sp,  t_s},
	  {  sp,  wll,   sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,  m_z,   sp,   sp, m_sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,  wll,   xx,   xx,   sp,  wll,   sp},
	  {  xx,  wll,   xx,   xx,  wll,   sp,   sp,  wll},
	  {  xx,  wll, i_rt, t_lv,   sp,  wll,   sp,  wll},
	  {  xx, t_lv,  wll,  wll,   sp,  wll,   sp,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ {  xx,   xx,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,  t_f, m_rm,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll},
	  {t_lv, t_lv, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll, i_rs, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll,  wll, t_lv,   sp,   sp,   sp,  t_f,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ { wll,   xx,   sp,  wll,  wll,  wll,   sp,  wll},
	  { wll,   xx, t_df,   xx,   xx,   xx,  wll,  wll},
	  { wll,   xx,   xx,   xx,  wll,   xx,   sp,  wll},
	  { t_f,  wll,  wll,  wll,   xx,   xx,   sp,  wll},
	  {  xx,   xx,   xx,   xx,   xx, t_df,   sp, i_gd},
	  {  xx,   xx,  wll, t_ts,  wll,   sp,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ { wll,   sp,   sp,   sp,   sp,   sp,  wll, i_gd},
	  {  sp, t_sk,   sp, t_sk,   sp, t_sk,  wll,  wll},
	  {  sp,   sp, t_as,   sp, t_as,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, m_rm,   sp, m_sp,   sp, m_rm,   sp, m_rm},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  t_f, i_gd,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll, m_sp, i_gd,   sp,   sp},
	  {  sp,   sp, i_ri, m_sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, i_gd,  t_f,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ { wll,  wll, r_wl,  m_z, m_sp,  wll,  wll,  wll},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,  wll,   sp, m_rm,   sp,   sp,   sp, r_wl},
	  { wll, r_wl,   sp,   sp, m_rm,   sp,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,   sp,  wll,  wll},
	  { wll,  wll,  wll, m_sp,  m_z,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp, t_lv,  wll},
	  { wll, t_lv, t_lv,   sp, t_cl, t_lv, i_rc, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_cl,   sp, t_lv,   sp},
	  {  sp,   sp, t_lv, t_lv, t_cl,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixEdgeChunks(c);

	return c.at(randInt(0, (int)c.size() - 1));
}

/*
std::vector<std::string> SecondFloor::innerChunksCT(std::vector<std::vector<std::vector<std::string>>> &c) {
	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);

	c = mixInnerChunksCT(c);

	return combineChunks(c);
}*/

std::vector<std::vector<std::string>> cornerChunks(int corner, bool innerRing) {

	/**  Corner legend:
	*    TL: 1 (A)
	*    TR: 2 (B)
	*    BL: 3 (C)
	*    BR: 4 (D)
	*/

	// Top-left: 1
	std::vector<std::vector<std::string>> A_i =
	{ {i_ri, t_cl, m_gs,   sp,   sp,   sp,   sp,   sp},
	  {t_cl, t_cl,   sp,   sp, r_wl,   sp,   sp,   sp},
	  {m_gs,   sp, m_gs,   sp, m_rm,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, r_wl,   sp,   sp,   sp},
	  {  sp, r_wl, m_rm, r_wl, r_wl,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> A_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Top-right: 2
	std::vector<std::vector<std::string>> B_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> B_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Bottom-left: 3
	std::vector<std::vector<std::string>> C_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> C_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	// Bottom-right: 4
	std::vector<std::vector<std::string>> D_i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> D_iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(A_i);
	c.push_back(A_ii);
	c.push_back(A_iii);
	c.push_back(A_iv);
	c.push_back(B_i);
	c.push_back(B_ii);
	c.push_back(B_iii);
	c.push_back(B_iv);
	c.push_back(C_i);
	c.push_back(C_ii);
	c.push_back(C_iii);
	c.push_back(C_iv);
	c.push_back(D_i);
	c.push_back(D_ii);
	c.push_back(D_iii);
	c.push_back(D_iv);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	*/

	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> junctionChunks() {
	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	*/


	return c.at(randInt(0, (int)c.size() - 1));
}

std::vector<std::vector<std::string>> SecondFloor::lavaChunks() {
	//	 LAVA CHUNKS
	std::vector<std::vector<std::string>> i =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a, t_tt, t_tt,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a,  m_a, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv,  m_a,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iv =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  t_f, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> v =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv},
	  {t_lv, t_lv, t_lv, t_df, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_df, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vi =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv,  t_f, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> viii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ix =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	//c.push_back(x);
	//c.push_back(xi);
	//c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixInnerChunks1(c);

	return c.at(randInt(0, (int)c.size() - 1));
}
std::vector<std::vector<std::string>> SecondFloor::fixedChunks(std::string chunk) {
	// the player's chunk where they spawn
	std::vector<std::vector<std::string>> pChunk =
	{ {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,   xx},
	  { wll,   xx,   xx,   xx,   xx,   xx, r_wl,   xx},
	  {  xx,   xx,   xx, _ply,   xx,  wll,   xx,   xx},
	  {  xx,   xx,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,  wll,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,  wll}, };


	// the chunk where the staircase will spawn
	std::vector<std::vector<std::string>> eChunk =
	{ { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
	  { wll,   sp,   sp, m_rm,   sp, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, lkex, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, m_rm, m_rm,   sp,  wll},
	  { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll} };


	// the chunk where player's can enter the center ring
	std::vector<std::vector<std::string>> outerBridge =
	{ {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv} };

	// empty chunk
	std::vector<std::vector<std::string>> empty =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	// The 9 special chunks in the center

	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll, r_wl,  wll,  wll,  wll,  wll,  wll},
	  { wll, i_gd,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, i_gd,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, i_gd,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll, r_wl,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	// center chunk with fountain
	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll, i_ri,  wll,  wll,  wll,   sp,   sp},
	  { wll, i_rw,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, r_wl,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll, r_wl,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, r_wl,  wll,  wll},
	  { wll,  wll,  wll,  wll, i_gd,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	// get the correct chunk
	if (chunk == "pChunk") return pChunk;
	else if (chunk == "eChunk") return eChunk;
	else if (chunk == "outerBridge") return outerBridge;
	else if (chunk == "empty") return empty;
	else if (chunk == "i") return i;
	else if (chunk == "ii") return ii;
	else if (chunk == "iii") return iii;
	else if (chunk == "iv") return iv;
	else if (chunk == "v") return v;
	else if (chunk == "vi") return vi;
	else if (chunk == "vii") return vii;
	else if (chunk == "viii") return viii;
	else if (chunk == "ix") return ix;
	else return empty;
}

std::vector<std::string> SecondFloor::generateLevel() {
	std::vector<std::string> finalvec, b;
	std::vector<std::vector<std::vector<std::string>>> v;

	for (layer; layer <= 9; layer++) {
		for (int i = 0; i < (getCols() - 2) / 8 /* divided by length of chunk */; i++) {

			if (!(layer == 1 || layer == 9 || layer == 5) && (i == 0 || i == 8)) {
				v.push_back(edgeChunks());
			}
			else if (layer == 1 || layer == 9) {
				if (i == 0 || i == 4 || i == 8)
					v.push_back(fixedChunks("empty"));
			}
			else if (layer == 2 || layer == 8) {

				// lava chunks
				if (i >= 1 && i <= 3 || i >= 5 && i <= 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("outerBridge"));
				}
			}
			else if (layer == 3 || layer == 7) {

				// lava chunks
				if (i == 1 || i == 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("empty"));
				}
			}
			// 4 - 6 are the special chunks in the center
			else if (layer == 4) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("i"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("ii"));
				else if (i == 5) v.push_back(fixedChunks("iii"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}
			else if (layer == 5) {

				// special chunks
				if (i == 0) v.push_back(fixedChunks("pChunk"));
				else if (i == 2 || i == 6) v.push_back(fixedChunks("empty"));
				else if (i == 3) v.push_back(fixedChunks("iv"));
				else if (i == 4) v.push_back(fixedChunks("v"));
				else if (i == 5) v.push_back(fixedChunks("vi"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());
				else if (i == 8) v.push_back(fixedChunks("eChunk"));

			}
			else if (layer == 6) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("vii"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("viii"));
				else if (i == 5) v.push_back(fixedChunks("ix"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}


			// if nothing was added in the intermediary, then add an "outer" chunk
			if (i == v.size())
				v.push_back(outermostChunks());
		}

		// transform 3d vector into 1d vector
		b = combineChunks(v);

		// add to the final dungeon map
		for (unsigned int i = 0; i < b.size(); i++)
			finalvec.push_back(b.at(i));

		b.clear();
		v.clear();
	}

	return finalvec;
}
std::vector<std::string> SecondFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (unsigned int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 8; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}



// ==============================================
//				:::: 3RD FLOOR ::::
// ==============================================
ThirdFloor::ThirdFloor(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, THIRD_FLOOR, MAXROWS3, MAXCOLS3), m_locked(false) {
	int rows = getRows();
	int cols = getCols();

	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec, 0, 0);

	// END LEVEL GENERATION
}

void ThirdFloor::checkDoors() {
	int cols = getCols();

	if (!m_doors.empty()) {
		int x, y;

		for (unsigned int i = 0; i < m_doors.size(); i++) {

			std::shared_ptr<Door> door = std::dynamic_pointer_cast<Door>(m_doors.at(i));
			x = door->getPosX();
			y = door->getPosY();

			// if door is locked, check if room is clear to unlock
			if (door->isLocked()) {
				if (roomCleared())
					toggleDoorLocks(x, y);
			}
			// if door is unlocked, open, isn't being held, and nothing is in the way
			else if (door->isOpen() && !door->isHeld() && !(enemy(x, y) || hero(x, y))) {
				playSound("Door_Closed.mp3");

				// show the room the player is in
				revealRoom();

				door->getSprite()->setVisible(true);

				door->toggleOpen();
				door->toggleHeld();
				tileAt(Coords(x, y)).wall = true;

				// if the room the player is now in isn't cleared, lock all doors
				if (!roomCleared())
					toggleDoorLocks(x, y);

			}
			else if (door->isOpen() && door->isHeld()) {
				// release hold on the door
				door->toggleHeld();
			}

			door.reset();
		}
	}
}
bool ThirdFloor::roomCleared() {
	int px = player[0]->getPosX();
	int py = player[0]->getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int mx, my, mxchunk, mychunk;
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		mx = m_monsters.at(i)->getPosX(); mxchunk = mx / 14;
		my = m_monsters.at(i)->getPosY(); mychunk = my / 10;

		if (mxchunk == xchunk && mychunk == ychunk)
			return false;	
	}

	return true;
}
void ThirdFloor::toggleDoorLocks(int dx, int dy) {
	Coords pos(dx, dy);

	Coords playerCoords = getPlayerCoordsClosestTo(pos);

	playSound("Door_Locked.mp3");

	int x, y;

	if (tileAt(pos).wallObject->getName() == DOOR_VERTICAL) {
		if (playerCoords.isLeftOf(pos)) {		
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx - 14 && y == dy) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy - 5) || (x == dx - 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}

		}
		// else if player is to the right of the given door
		else {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx + 14 && y == dy) || door->getPos() == pos)
					door->toggleLock();

				else if ((x == dx + 7 && y == dy - 5) || (x == dx + 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}
		}
	}
	// door is oriented horizontally
	else {
		if (playerCoords.isAbove(pos)) {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy - 10) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy - 5) || (x == dx + 7 && y == dy - 5))
					door->toggleLock();
				
				door.reset();
			}

		}
		// else if player is below the given door
		else {
			for (size_t i = 0; i < m_doors.size(); ++i) {
				std::shared_ptr<Door> door = m_doors.at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy + 10) || door->getPos() == pos)
					door->toggleLock();
				
				else if ((x == dx - 7 && y == dy + 5) || (x == dx + 7 && y == dy + 5))
					door->toggleLock();
				
				door.reset();
			}
		}
	}
}

void ThirdFloor::hideRooms() {
	// Hide all elements
	for (unsigned int i = 0; i < m_walls.size(); i++) {
		Coords currentPos(m_walls[i]->getPos());

		if (tileAt(currentPos).boundary || tileAt(currentPos).wallObject && tileAt(currentPos).wallObject->isDoor())
			continue;

		m_walls.at(i)->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		m_monsters[i]->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_items.size(); i++) {
		m_items.at(i)->getSprite()->setVisible(false);
	}
	for (unsigned int i = 0; i < m_traps.size(); i++) {
		m_traps[i]->getSprite()->setVisible(false);
	}
	
	revealRoom();
}
void ThirdFloor::revealRoom() {
	int px = player[0]->getPosX();
	int py = player[0]->getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int sx, sy, sxchunk, sychunk;

	for (unsigned int i = 0; i < m_walls.size(); i++) {
		sx = m_walls.at(i)->getPosX();
		sy = m_walls.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_walls.at(i)->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_monsters.size(); i++) {
		sx = m_monsters.at(i)->getPosX();
		sy = m_monsters.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_monsters[i]->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_items.size(); i++) {
		sx = m_items.at(i)->getPosX();
		sy = m_items.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_items.at(i)->getSprite()->setVisible(true);
		
	}
	for (unsigned int i = 0; i < m_traps.size(); i++) {
		sx = m_traps.at(i)->getPosX();
		sy = m_traps.at(i)->getPosY();
		sxchunk = sx / 14;
		sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk)
			m_traps[i]->getSprite()->setVisible(true);
		
	}

}

void ThirdFloor::TLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{ { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp, m_ds,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
	  { wll,   sp,   sp, m_ds,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{	{ unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as,  i_b, t_as,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, i_lp,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  wll,  m_w,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, i_bc,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll, i_bc,  i_b, i_bc,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,   sp, m_ds,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	// contains a single corner chunk
	pickSingleChunk(current, pool);
}
void ThirdFloor::topChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_tr,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  t_f,   sp,  m_z,  wll,  m_z,   sp,   sp,  t_f,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_tl,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  t_f,  wll,  unb},
		{ wll, t_lv,  wll,   sp,  m_z,   sp,  wll,   sp,  m_z,   sp,  wll, t_lv,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  m_z,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_sp,  wll,   sp,   sp,   sp,  wll, m_sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_k,  wll,  m_g,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll, t_as, t_as,  wll, m_ds,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll, t_as, t_as,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  unb},
		{  sp,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp,   sp, m_ds, t_as,  wll, t_as, m_ds,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds,   sp, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{  sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  _dv},
		{ wll,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb,  wll,  unb},
		{ wll,  wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb,  wll,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,  wll,   sp,  wll,  wll,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  unb,  wll, t_ts,  wll,  unb,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, t_lv,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::TRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp, m_ds, m_ds, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds, m_ds, m_ds,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  m_k,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_a,  wll,  unb},
		{ wll,   sp,  m_w,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll, i_ar, t_lv,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, t_lv,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  unb,  unb,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb, m_sp,  unb,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  unb,  unb,  unb,  unb,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  unb,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  i_b, t_lv, t_tl,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::leftEdgeChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll, i_lp, t_ts,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_gs,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,t_mbv,  wll, i_ri,  wll,t_mbv,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp, t_lv,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w,  wll,   sp,   sp,   sp,   sp, m_br,  t_f,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll, i_hp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vi =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  t_s, t_cf,  t_s,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> viii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ix =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> x =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(vi);
	//pool.push_back(vii);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

}
void ThirdFloor::middleChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_br,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_fr,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb},
		{ m_z,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  m_r,  i_b,  m_r, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ m_z,   sp,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_as,  wll,   sp,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{  sp, t_ts, t_ts, t_as,  wll,  wll,  m_w,  wll,  wll, t_as, t_ts, t_ts,   sp,  _dv},
		{ wll, t_ts, t_ts, t_as,  wll,  wll,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, m_ds,   sp,  t_f,   sp, m_ds,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll, i_ri,  wll,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  wll, t_df,  wll,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_a,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll, m_ds,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp, t_as,  wll,  wll,  wll, t_as,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_sp,   sp,  wll,  wll,  m_b,  wll,  wll,   sp, m_sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, m_gs,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll,  wll,  wll,  m_a,  wll,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  m_b,  wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  m_w,   sp,   sp,  wll,  i_b,  wll,   sp,   sp,  m_w,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,  wll,  wll,  wll,  m_b,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::rightEdgeChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp, t_as, t_as,  wll,  unb},
		{ wll,   sp,  wll,  wll,   sp,  m_g, m_ds,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  wll, m_br, m_ds,  wll,   sp,  m_r,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  m_w,  wll, m_ds,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{  sp,   sp, t_lv,   sp,   sp,  m_k,  m_w,  m_k,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{  sp,   sp, m_sp,  wll,   sp, t_as, m_sp, t_as,   sp,   sp,   sp, m_ds,  wll,  unb},
		{ wll,  wll, t_ts,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  t_s,   sp,   sp, m_ds,  wll,  m_k,   sp,   sp,  t_s,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, t_ts, t_ts, t_ts, t_ts,  wll,  t_s, t_ts, t_ts, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::BLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   {  unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,   sp,  wll,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,   sp,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll, t_tr, t_lv,   sp,   sp,   sp,  m_z,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,  wll,  m_z,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_k, t_df,  m_k,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp, t_as,   sp,  wll,  wll,  wll,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  wll,  _dv},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_ar,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  i_b,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_z,  wll,  m_z,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);
}
void ThirdFloor::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_sp,  wll, m_sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  m_w,  wll,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_ri,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = { { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 {  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  m_r,  unb},
		{ wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  m_r,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  m_r,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_r,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{  sp,   sp,   sp,  t_f, t_lv, t_as, t_lv, t_as, t_lv,  t_f,   sp,   sp,   sp,  _dv},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,  m_b, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{  sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv, t_tl,  wll,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  unb, i_lp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  unb,  wll,  wll,  wll, t_df,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,  wll,  wll,  wll,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  wll, i_ar,  wll, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_w,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_w,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool);

	//return combineChunks(c);
}
std::vector<std::string> ThirdFloor::BRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp, i_gd,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll, i_ri,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	//pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

std::vector<std::vector<std::string>> ThirdFloor::playerChunks() {

	std::vector<std::vector<std::string>> TLplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ply,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ply,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLplayerchunk =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp, _ply,   sp,   sp,   sp, t_as,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, t_as, _ply, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_playerchunk) {
	case 1: return TLplayerchunk;
	case 2: return TRplayerchunk;
	case 3: return BLplayerchunk;
	case 4: return BRplayerchunk;
	default: return TLplayerchunk;
	}
}
std::vector<std::vector<std::string>> ThirdFloor::exitChunks() {

	std::vector<std::vector<std::string>> TLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ext,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_rc,   sp,  wll,  wll,   sp, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_as, t_as, _ext, t_as, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_exitchunk) {
	case 1: return TLexitchunk;
	case 2: return TRexitchunk;
	case 3: return BLexitchunk;
	case 4: return BRexitchunk;
	default: return TLexitchunk;
	}
}

void ThirdFloor::mixChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	for (int i = 1; i < (getCols() - 2) / 13 - 1; i++) {
		current.push_back(pool[randInt(0, (int)pool.size() - 1)]);
	}
}
void ThirdFloor::pickSingleChunk(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	// selects one random corner chunk and pushes it
	if (m_layer == m_playerchunk && !m_playerplaced) {
		current.push_back(playerChunks());
		m_playerplaced = true;
	}
	else if (m_layer == m_exitchunk && !m_exitplaced) {
		current.push_back(exitChunks());
		m_exitplaced = true;
	}
	else
		current.push_back(pool[randInt(0, (int)pool.size() - 1)]);
}

std::vector<std::string> ThirdFloor::generateLevel() {
	// selects layer for the player and exit chunks to be on
	m_layer = 1;
	m_playerchunk = randInt(1, 4);
	m_playerplaced = m_exitplaced = false;

	switch (m_playerchunk) {
	case 1: m_exitchunk = 4; break;
	case 2: m_exitchunk = 3; break;
	case 3: m_exitchunk = 2; break;
	case 4: m_exitchunk = 1; break;
	default: break;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<std::string>>> c, pool;
	std::vector<std::string> v, finalvec;

	int count = 0;
	while (count < (getRows() - 2) / 9) {
		if (count == 0) {
			TLChunks(c, pool); pool.clear(); m_layer++;
			topChunks(c, pool); pool.clear();
			v = TRChunks(c, pool); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool); pool.clear();
			middleChunks(c, pool); pool.clear();
			v = rightEdgeChunks(c, pool); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool); pool.clear(); m_layer++;
			bottomChunks(c, pool); pool.clear();
			v = BRChunks(c, pool); pool.clear();
		}

		for (unsigned int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		c.clear();
		count++;
	}

	return finalvec;
}
std::vector<std::string> ThirdFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 10; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < (int)c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 14; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}



FourthFloor::FourthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FOURTH_FLOOR, 0, 0) {

}

FifthFloor::FifthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIFTH_FLOOR, 0, 0) {

}

SixthFloor::SixthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SIXTH_FLOOR, 0, 0) {

}

SeventhFloor::SeventhFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, SEVENTH_FLOOR, 0, 0) {

}

EighthFloor::EighthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, EIGHTH_FLOOR, 0, 0) {

}

NinthFloor::NinthFloor(LevelScene *scene, std::shared_ptr<Player> p) : Dungeon(scene, p, NINTH_FLOOR, 0, 0) {

}



// ================================================
//				:::: BOSS FLOOR ::::
// ================================================
FirstBoss::FirstBoss(LevelScene* scene, std::shared_ptr<Player> p) : Dungeon(scene, p, FIRST_BOSS, BOSSROWS, BOSSCOLS) {
	int rows = getRows();
	int cols = getCols();

	//	spawn player
	player[0]->setPosX(cols / 2);
	player[0]->setPosY(rows - 3);
	tileAt(p->getPos()).hero = true;


	//	spawn boss
	addMonster(std::make_shared<Smasher>(this));


	//	spawn spike traps
	int x, y, speed;
	int m = 12 + randInt(0, 5);

	while (m > 0) {
		x = randInt(1, cols - 2);
		y = rows / 2 + randInt(0, rows / 2 - 1);
		speed = 3 + randInt(0, 2);

		while (trap(x, y)) {
			x = randInt(1, cols - 2);
			y = rows / 2 + randInt(0, rows / 2 - 1);
			speed = 3 + randInt(0, 2);
		}

		addTrap(std::make_shared<SpikeTrap>(*this, x, y, speed));
		m--;
	}
}

>>>>>>> 2989379ff3214d5b91cd4c670e60b6aee17c666b
