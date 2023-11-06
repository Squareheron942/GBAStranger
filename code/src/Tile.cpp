

#include "Tile.h"
#include "EffectsManager.h"
#include "EntityManager.h"
#include "Game.h"

EffectsManager* FloorTile::effectsManager = NULL;
TileManager* FloorTile::tileManager = NULL;
BackgroundMap* FloorTile::rawMap = NULL;
EntityManager* FloorTile::entityManager = NULL;
Game* FloorTile::game = NULL;

int Switch::pressedCount = 0;
int Switch::totalCount = 0;

// the gods created constexpr related errors to test mankind, and mankind failed.
// i should: be able to oneline this, be able to do it anonymously, and be able to inline it
//EffectTypeArray glassAnimation[] = {EffectType(bn::sprite_tiles_items::dw_spr_glassfloor, 8)};
//constexpr bn::span<const bn::pair<const bn::sprite_tiles_item, int>> bruh(glassAnimation);

void Glass::stepOn() {
	game->playSound(&bn::sound_items::snd_stepglassfloor);
}

void Glass::stepOff() {
	if(isAlive) {
		//effectsManager->createEffect(tilePos, EffectTypeCast(glassAnimation));
		effectsManager->glassBreak(tilePos);
	}
	isAlive = false;
	tileManager->updateTile(tilePos);
	// there just isnt a glass break sound
	/*if(game->state == GameState::Normal) {
		bn::sound_items::snd_breakglassfloor.play();
	}*/
}

void Bomb::stepOn() {
	charge++;
	if(charge == 2) {
		isAlive = false;
	
		game->playSound(&bn::sound_items::snd_vanish);
		
		// this may be bad 
		// break adjacent bomb tiles,,,, i think?
		Direction testDirections[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
		for(int i=0; i<4; i++) {
			Pos testPos = tilePos;
			if(!testPos.move(testDirections[i])) {
				continue;
			}
			
			if(tileManager->hasFloor(testPos) == TileType::Bomb) {
				Bomb* tempBomb = static_cast<Bomb*>(tileManager->floorMap[testPos.x][testPos.y]);
				if(tempBomb->charge == 1) {
					tempBomb->stepOn();
				}
			}
		}
	}
		
	tileManager->updateTile(tilePos);
}

void Death::stepOn() {
	
	// this needs to call updatemap as well. gods(maybe?)
	SaneSet<Entity*, 4>& tempMap = entityManager->getMap(tilePos);
	
	for(auto it = tempMap.begin(); it != tempMap.end(); ) {
		if((*it)->entityType() == EntityType::Player) {
			// how can i make it play a death anim?
			entityManager->addKill(*it);
			++it;
		} else {
			it = entityManager->killEntity(*it);
		}
	}
	
}

void Switch::stepOn() {
	pressedCount++;
	isSteppedOn = true;
	tileManager->updateExit();
	game->playSound(&bn::sound_items::snd_activate);
}

void Switch::stepOff() {
	pressedCount--;
	isSteppedOn = false;
	tileManager->updateExit();
	game->playSound(&bn::sound_items::snd_activate);
}

int RodTile::getTileValue() const {

	int offset = entityManager->player->hasSuperRod ? 11 : 0;

	if(!entityManager->player->hasRod && !entityManager->player->hasSuperRod) {
		return startIndex;
	}
	
	FloorTile* rodTile = NULL;
	if(entityManager->player->rod.size() != 0) {
		rodTile = entityManager->player->rod.back();
	}
	
	if(rodTile == NULL) {
		return startIndex + offset + 1;
	}
	
	TileType rodTileType = rodTile->tileType();
	
	// a switch statement with tiletype is what caused dofloorsteps to lag. will this also cause problems?
	switch(rodTileType) {
		case TileType::Death:
			return startIndex + offset + 1 + 1;
			break;
		case TileType::Floor:
			return startIndex + offset + 1 + 2;
			break;
		case TileType::Glass:
			return startIndex + offset + 1 + 3;
			break;
		case TileType::Switch:
			return startIndex + offset + 1 + 4;
			break;
		case TileType::Exit:
			return startIndex + offset + 1 + 5;
			break;
		case TileType::Copy:
			return startIndex + offset + 1 + 6;
			break;
		case TileType::Bomb:
			// program didnt like static casting to a bomb here, so we are doing this
			return startIndex + offset + 1 + 7 + (rodTile->getTileValue() == rodTile->startIndex);
			break;
		default:
			return startIndex + offset + 1 + 9;
			break;	
	}
	
}

int LocustTile::getTileValue() const {
	//return startIndex + (entityManager->player->locustCount != 0);	
	return startIndex + 1;
}

static int inline getWordTileIndex(const char c) {
	
	switch(c) {
		
		case ' ':
			return 0;
		
		case '0':
			return 2;
		case '1':
			return 3;
		case '2':
			return 4;
		case '3':
			return 5;
		case '4':
			return 6;
		case '5':
			return 7;
		case '6':
			return 8;
		case '7':
			return 9;
		case '8':
			return 10;
		case '9':
			return 11;
		
		case 'V':
			return 12;
		case 'O':
			return 13;
		case 'I':
			return 14;
		case 'D':
			return 15;
			
		case 'B':
			return 16;
		case '?':
			return 17;
			
		case 'H':
			return 18;
		case 'P':
			return 19;
		
		default: [[unlikely]]
			BN_ERROR("unknown char ", c, " passed into getWordTileIndex");
			break;
	}
	
	return 0;
}

void WordTile::draw() {
	
	u8 x = tilePos.x;
	u8 y = tilePos.y;
	
	int temp;
	
	// i hadnt ate this day, and somehow ended up with this. dont ask
	temp = getWordTileIndex(first);
	int firstTile = 228 + (((temp >> 1) << 2) | (temp & 1));
	
	temp = getWordTileIndex(second);
	int secondTile = 228 + (((temp >> 1) << 2) | (temp & 1));
	
	rawMap->setTile(x * 2 + 1, y * 2 + 1, firstTile); 
	rawMap->setTile(x * 2 + 1, y * 2 + 2, firstTile + 2); 
	
	rawMap->setTile(x * 2 + 2, y * 2 + 1, secondTile); 
	rawMap->setTile(x * 2 + 2, y * 2 + 2, secondTile + 2); 
}



