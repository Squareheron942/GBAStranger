
#include "Entity.h"

#include "EntityManager.h"

#include "Game.h"

// this is the only way i could get this to work. why is this syntax like this??
Palette* Sprite::spritePalette = &defaultPalette;

// default initilization, gets overwritten in game.h
EntityManager* Entity::entityManager = NULL;
EffectsManager* Entity::effectsManager = NULL;
TileManager* Entity::tileManager = NULL;
Game* Entity::game = NULL;

int LevStatue::rodUses = 0;
int LevStatue::totalLev = 0;

// Player

EffectTypeArray questionMark[] = {EffectType(bn::sprite_tiles_items::dw_spr_question_black, 9)};

bn::pair<bool, bn::optional<Direction>> Player::doInput() {
	
	// if a direction was pressed, return that (true, dir)
	// if A was pressed, meaning we pick up a tile,,,, then (true, NULL)
	// should the move be invalid(picking up a tile with ENTITY on it, return false, null)
	
	
	if(bn::keypad::a_pressed()) {
		
		Pos tilePos(p);
		
		bool moveRes = tilePos.move(currentDir);
		
		if(!moveRes) {
			effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			return {false, bn::optional<Direction>()};
		}

		if(entityManager->hasCollision(tilePos)) {
			effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			return {false, bn::optional<Direction>()};
		}
		
		if(entityManager->hasObstacle(tilePos)) {
			// do dialogue here
			// what abt npcs tho, fuck
			// shadows not technically being enemies rlly fucks me
			
			Obstacle* temp = static_cast<Obstacle*>(*(entityManager->getMap(tilePos).begin()));
			
			temp->interact();
			
			return {false, bn::optional<Direction>()};
		}
		
		// if there is a entity in this tile, this is an invalid move.
		
		if(entityManager->hasEntity(tilePos)) {
			effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			return {false, bn::optional<Direction>()};
		}
		
		// do the tile swap.
		
		FloorTile* tile = tileManager->floorMap[tilePos.x][tilePos.y];
		
		if(tile == NULL && rod == NULL) { 
			effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			return {false, bn::optional<Direction>()};
		} else if (tile != NULL && rod != NULL) {
			effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			return {false, bn::optional<Direction>()};
		} else if(tile == NULL && rod != NULL) {
			// put tile down 
			rod->tilePos = tilePos;
			tileManager->floorMap[tilePos.x][tilePos.y] = rod;
			rod = NULL;
			entityManager->rodUse();
			tileManager->updateTile(tilePos);
			tileManager->updateRod();
		} else if(tile != NULL && rod == NULL) {
			// pick tile up
			rod = tileManager->floorMap[tilePos.x][tilePos.y];
			tileManager->floorMap[tilePos.x][tilePos.y] = NULL;
			entityManager->rodUse();
			tileManager->updateTile(tilePos);
			tileManager->updateRod();
		}

		nextMove = bn::optional<Direction>();
		
		return {true, bn::optional<Direction>()};
	}
	
	if(bn::keypad::down_pressed()) {		
		currentDir = Direction::Down;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::up_pressed()) {
		currentDir = Direction::Up;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::left_pressed()) {
		currentDir = Direction::Left;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::right_pressed()) {
		currentDir = Direction::Right;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	}
	
	
	nextMove = bn::optional<Direction>();
	return {false, bn::optional<Direction>()};
	
}

bn::optional<Direction> Player::getNextMove() {
	bn::optional<Direction> temp = nextMove;
	nextMove.reset();
	return temp; 
}

void Player::startFall() {
	
	BN_LOG("playerfal");
	// have the player do the cyote time thingy
	tileIndex = static_cast<int>(currentDir);
	fallData.insert(fallData.begin(), bn::pair<bn::sprite_tiles_item, u8>(spriteTilesArray[tileIndex], 6));
	
	// do locust bs
	if(!isVoided && locustCount > 0) {
		locustCount--;
		tileManager->updateLocust();
	}
	
	if(locustCount == 0) {
		isVoided = true;
		tileManager->updateVoidTiles();
	}
	
}

// Enemy

bn::optional<Direction> Enemy::getNextMove() {
	return bn::optional<Direction>(currentDir);
}

bn::optional<Direction> Bull::getNextMove() {
	
	if(idle) {
		// perform LOS check
		
		bn::optional<Direction> playerDir = entityManager->canSeePlayer(p);
		
		if(playerDir) {
			currentDir = playerDir.value();
			idle = false;
		} else {
			// if we are still idle, return, if not, kachow
			return bn::optional<Direction>();
		}
	}

	return bn::optional<Direction>(currentDir);
}

void Bull::moveFailed() {
	idle = true;
}

bn::optional<Direction> Chester::getNextMove() {
	// wow. it really is that simple.
	return entityManager->canSeePlayer(p);
}

bn::optional<Direction> Mimic::getNextMove() {
	
	if(!nextMove) {
		return nextMove;
	}
	
	Direction temp = nextMove.value();
	
	nextMove.reset();
	
	if(invertHorizontal) {
		if(temp == Direction::Left) {
			temp = Direction::Right;
		} else if(temp == Direction::Right) {
			temp = Direction::Left;
		}
	}
	
	if(invertVertical) {
		if(temp == Direction::Up) {
			temp = Direction::Down;
		} else if(temp == Direction::Down) {
			temp = Direction::Up;
		}
	}
	
	currentDir = temp;
	
	return temp; 
}
	
bn::optional<Direction> Diamond::getNextMove() {
	
	bn::optional<Direction> temp = nextMove;
	
	idle = !nextMove;
	
	nextMove.reset();

	return temp;
}

// Obstacle

Chest::Chest(Pos p_) : Obstacle(p_) {
	spriteTilesArray.clear();
	spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_regular);
	
	// this is horrid, but sometimes chests spawn without any floor under them. 
	// this fixes that.
	
	if(!tileManager->hasFloor(p_)) {
		tileManager->floorMap[p_.x][p_.y] = new FloorTile(p_);
	}
	
	doUpdate();
}

void Chest::interact() {
	
	// this is actually the only place im even using game in here
	// and i could(actually should? probs just pass the roomManager into here, but idk 
	// to be 100% real is that i need to go convert all that shit into namespaces, but i 
	// fucking hate namespaces, and if i have to rewrite a bunch of my h files i will freak
	
	if(game->roomManager.roomIndex > 2) {
		// this if statement is here on purpose for a very stupid easter egg, see the random boulder msgs
		// its 2 just in case, for future room ordering changes
		Pos playerPos = entityManager->player->p;
		playerPos.move(Direction::Up);
		if(playerPos != p) {
			return;
		}
	}
	
	if(animationIndex == 0) {
		animationIndex = 1;
	
		doUpdate();
	
		entityManager->player->locustCount++;
		tileManager->updateLocust();
		// this isnt counted as a successful move, but we should still update locusts
		tileManager->floorLayer.reloadCells();
	}
	
}

// why doesnt this work??
//const char* const randomBoulderMessages[] = {"jfdklsafs", "a", "123124", "VOID look heres a bunch of \rtext wow\rwe even have scrolling\nbruh1\nbruh2"};

// i swear, why cant i do this without a struct? idek if the idek var is needed, but im not going to be changing it.
struct MessageStr {
	const char* str;
	const char idek = '\0'; // sanity
};

// each string here has a limit of,, 64 non space chars in between lines(seperated with \n)
// i,, could do something to fix that though? but like ugh 
// idek 

#define MSGSTR(s) { s }
const MessageStr randomBoulderMessages[] = { 
	MSGSTR("i rlly hope this works"), 
	MSGSTR("Did you know every time you\rsigh, a little bit of happiness\rescapes?"), 
	MSGSTR("VOID look heres a bunch of \rtext wow\rwe even have scrolling\nbruh1\nbruh2"),
	MSGSTR("jesus christ i need a job"),
	MSGSTR("ugh"),
	MSGSTR("i firmly believe that cif is \rbest lord"),
	MSGSTR("please if you know anyone\rwho has a job in CS, give them\nmy contact info, i need a job"),
	MSGSTR("this program was written in\rpart by the following:\nexcessive ADHD medication,\rdepression,\na NEET lifestyle,\rand viewers like you<3"),
	MSGSTR("you might be able to still open\rthe chest when you arent facing it\ndepending on if ive fixed that yet"), // it would be funny to program in an exception for this chest only
	MSGSTR("uwu\nuwuuwuwuwuwuwu"),
};

void Boulder::interact() {
	
	static int lastIndex = -1;
	int index = randomGenerator.get_int(0, sizeof(randomBoulderMessages) / sizeof(randomBoulderMessages[0]));
	
	while(lastIndex == index) {
		index = randomGenerator.get_int(0, sizeof(randomBoulderMessages) / sizeof(randomBoulderMessages[0]));
	}
	lastIndex = index;

	const char* temp = randomBoulderMessages[index].str;
		
	effectsManager->doDialogue(temp);
}

bn::optional<Direction> Obstacle::getNextMove() {
	
	if(bumpDirections.size() == 0) {
		return bn::optional<Direction>();
	}
	
	int tempX = 0;
	int tempY = 0;
	
	for(int i=0; i<bumpDirections.size(); i++) {
		switch (bumpDirections[i]) {
			case Direction::Up:
				tempY -= 1;
				break;
			case Direction::Down:
				tempY += 1;
				break;
			case Direction::Left:
				tempX -= 1;
				break;
			case Direction::Right:
				tempX += 1;
				break;
			default:
				break;
		}
	}
		
	BN_ASSERT(!(tempX != 0 && tempY != 0), "a object was pushed in,, >=2 nonparallel directions???");
	
	bn::optional<Direction> res;
	
	if(tempX > 0) {
		res = Direction::Right;
	} else if(tempX < 0) {
		res = Direction::Left;
	} else if(tempY > 0) {
		res = Direction::Down;
	} else if(tempY < 0) {
		res = Direction::Up;
	} else {
		// push dirs canceled out, do nothing
	}
	
	bumpDirections.clear();
	return bn::optional<Direction>(res);
}

void Obstacle::startFall() {
	
	// copy over the actual sprite time zone into the falldata,
	// just so i dont have to go bs a bunch of code
	// but tbh, i rlly should.
				
	//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(spriteTilesArray[0], 9));
	
}

void EusStatue::startFall() {

	BN_ASSERT(tileManager->floorMap[p.x][p.y] == NULL, "with a eus statue, you tried pushing it onto an area that i hadnt nulled yet(a glass that just broke, or something). im to lazy rn to fix this, but if you see it msg me");

	tileManager->floorMap[p.x][p.y] = new FloorTile(p);

	// this is trash, please make tilemanager update based on tile
	tileManager->fullDraw();
	
	Obstacle::startFall();
}

bn::optional<Direction> GorStatue::getNextMove() {
	
	if(startPos == p) {
		return Obstacle::getNextMove();
	}
	
	tileIndex = 1;
	
	bumpDirections.clear();
	return bn::optional<Direction>();
}

bn::optional<Direction> MonStatue::getNextMove() {
	
	if(entityManager->canSeePlayer(p)) {
		animationIndex = 1;
		doUpdate();
		entityManager->addKill(this);
	}

	return bn::optional<Direction>();
}

void LevStatue::startFall() {
	totalLev--;
	if(isActive) {
		rodUses--;
		entityManager->rodUse();
	}
	if(rodUses != 0 && rodUses >= totalLev) {
		entityManager->addKill(entityManager->player); // ADDING THIS TO A KILL RIGHT HERE MIGHT BE A horrid idea, putting player to be safe
	}
}



