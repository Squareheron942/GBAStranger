

#include "TileManager.h"
#include "EffectsManager.h"
#include "EntityManager.h"

#include "Game.h"

// TODO: should tiles have entitymanagaer access?
// copy tiles could call shadow spawns from there
// death tiles could kill from there 
// also, i would be passing the entity in as an optional param, maybe?
// assuming that doesnt cause slowdown, that would be a good idea

void TileManager::loadTiles(u8* floorPointer, SecretHolder* secrets, int secretsCount, const char* exitDest) {
	
	u8 uncompressedFloor[126];
	game->uncompressData(uncompressedFloor, floorPointer);
	
	floorSteps.clear();
	
	if(entityManager->player != NULL) {
		Player* player = entityManager->player;
		
		for(int i=0; i<player->rod.size(); i++) {
			delete player->rod[i];
			player->rod[i] = NULL;
		}
		player->rod.clear();
	
	}
	
	exitTile = NULL;
	rodTile = NULL;
	locustTile = NULL;
	locustCounterTile = NULL;
	memoryTile = NULL;
	wingsTile  = NULL;
	swordTile  = NULL;
	floorTile1 = NULL;
	floorTile2 = NULL;
	
	int switchTracker = 0;

	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}
			
			floorMap[x][y] = NULL;
			
			Pos tempPos(x, y);

			switch(static_cast<TileType>(uncompressedFloor[x + 14 * y])) {
				case TileType::Pit:
					break;
				case TileType::Floor:
					floorMap[x][y] = new FloorTile(tempPos);
					break;
				case TileType::Glass:
					floorMap[x][y] = new Glass(tempPos);
					break;
				case TileType::Bomb:
					floorMap[x][y] = new Bomb(tempPos);
					break;
				case TileType::Death:
					floorMap[x][y] = new Death(tempPos);
					break;
				case TileType::Copy:
					floorMap[x][y] = new Copy(tempPos);
					break;
				case TileType::Exit:
					floorMap[x][y] = new Exit(tempPos);
					//BN_ASSERT(exitTile == NULL, "tried loading in two exits on one level?");
					// i could just,,, loop over the floor every time a switch is pressed, but i dont rlly want to do that
					exitTile = static_cast<Exit*>(floorMap[x][y]);
					break;
				case TileType::Switch:
					floorMap[x][y] = new Switch(tempPos);
					switchTracker++;
					break;
				default:
					BN_ERROR("unknown tile tried to get loaded in, wtf");
					break;
			}
		}
	}
	
	//BN_ASSERT(exitTile != NULL, "no exittile was loaded in this level?");

	Switch::pressedCount = 0; // is this,,, ok? TODO, SEE IF THIS CAUSES ISSUES
	
	BN_ASSERT(Switch::pressedCount == 0, "after loading in new tiles, the number of pressed tiles wasnt 0? Switch::pressedCount = ", Switch::pressedCount);
	
	BN_ASSERT(Switch::totalCount == switchTracker, "after loading in new tiles, the number of switches and number inside the class wasnt equal???");
	
	// draw out the UI.
	
	// SHOULD THE UI BE EFFECT BASED?
	// i could add funcs for pickup and putdown, and have the tiles possess effectswhich draw them out?
	// ugh, tbh it looks fine except for the D in void. maybe thats a manual fix.
	
	floorMap[0][8] = new WordTile(Pos(0, 8));
	
	floorMap[1][8] = new WordTile(Pos(1, 8), 'V', 'O');
	floorMap[2][8] = new WordTile(Pos(2, 8), 'I', 'D');
	// we now have a special tile for the 'ID'
	//floorMap[2][8] = new WordTile(Pos(2, 8), '~' + 1, '~' + 2);
	
	voidTile1 = static_cast<WordTile*>(floorMap[1][8]);
	voidTile2 = static_cast<WordTile*>(floorMap[2][8]);
	
	floorMap[3][8] = new WordTile(Pos(3, 8));
	//floorMap[4][8] = new WordTile(Pos(4, 8), 'L', 'C');
	floorMap[4][8] = new LocustTile(Pos(4, 8));
	locustTile = static_cast<LocustTile*>(floorMap[4][8]);
	
	//floorMap[5][8] = new WordTile(Pos(5, 8), '4', '2');
	floorMap[5][8] = new WordTile(Pos(5, 8), ' ', ' ');
	locustCounterTile = static_cast<WordTile*>(floorMap[5][8]);
	
	//floorMap[6][8] = new WordTile(Pos(6, 8), 'R', 'D');
	floorMap[6][8] = new RodTile(Pos(6, 8));
	rodTile = static_cast<RodTile*>(floorMap[6][8]);
	
	
	
	floorMap[7][8] = new WordTile(Pos(7, 8));
	
	// rodtile, and everything else should maybe(in the future) be replaced with spritetiles with custom lambdas?
	memoryTile = new SpriteTile(Pos(8, 8), []() -> int {
		Player* player = globalGame->entityManager.player;
		
		BN_ASSERT(player != NULL, "in a spriteTileFunc, player was null");
		
		if(player->hasMemory) {
			return 51 + ( globalGame->mode == 2 ? 3 : 0) + 0;
		}
		
		return 57;
	});
	floorMap[8][8] = memoryTile;
	
	wingsTile = new SpriteTile(Pos(9, 8), []() -> int {
		Player* player = globalGame->entityManager.player;
		
		BN_ASSERT(player != NULL, "in a spriteTileFunc, player was null");
		
		if(player->hasWings) {
			return 51 + ( globalGame->mode == 2 ? 3 : 0) + 1;
		}
		
		return 57;
	});
	floorMap[9][8] = wingsTile;
	
	swordTile = new SpriteTile(Pos(10, 8), []() -> int {
		Player* player = globalGame->entityManager.player;
		
		BN_ASSERT(player != NULL, "in a spriteTileFunc, player was null");
		
		if(player->hasSword) {
			Pos tempPos = player->p;
			if(tempPos.move(player->currentDir) && globalGame->entityManager.hasEnemy(tempPos)) {
				return 67 + (globalGame->mode == 2 ? 4 : 0) + ((frame % 16) / 4);
			}
			return 51 + ( globalGame->mode == 2 ? 3 : 0) + 2;
		}
		
		return 57;
	});
	floorMap[10][8] = swordTile;
	
	floorMap[11][8] = new WordTile(Pos(11, 8));
	
	// this should be changed. roommanager should just have a array with a 3 length char array for what floor number should be displayed(or ???)
	int roomIndex = game->roomManager.roomIndex;
	
	BN_ASSERT(roomIndex <= 999, "why in tarnation is the roommanager's roomindex greater than 999???");
	
	if(roomIndex <= 256) {
		floorMap[12][8] = new WordTile(Pos(12, 8), 'B', '0' + roomIndex / 100);
		floorMap[13][8] = new WordTile(Pos(13, 8), '0' + (roomIndex / 10) % 10, '0' + (roomIndex % 10));
	} else {
		floorMap[12][8] = new WordTile(Pos(12, 8), 'B', '?');
		floorMap[13][8] = new WordTile(Pos(13, 8), '?', '?');
	}
	
	floorTile1 = static_cast<WordTile*>(floorMap[12][8]);
	floorTile2 = static_cast<WordTile*>(floorMap[13][8]);
	
	
	exitDestination = exitDest;
	secretDestinations.clear();
	
	secretsCount--;
	secrets++;
	
	for(int i=0; i<secretsCount; i++) {
		
		Pos tempSecretPos(secrets->x, secrets->y);
		
		bn::pair<const char*, Pos> tempPair(secrets->dest, tempSecretPos);
		secretDestinations.push_back(tempPair);
		
		secrets++;
	}
	
}

int TileManager::checkBrandIndex(const unsigned (&testBrand)[6]) {
	
	int matchIndex = -1;
	
	unsigned matches[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	// spr_lordborders
	
	// doing this equality check in the above for loop would save computation
	for(unsigned i=0; i<sizeof(allBrands) / sizeof(allBrands[0]); i++) {
		for(int j=0; j<6; j++) {
			if(testBrand[j] == allBrands[i][j]) {
				matches[i]++;
			}
		}
	}
	
	for(unsigned i=0; i<sizeof(matches) / sizeof(matches[0]); i++) {
		if(matches[i] == 6) {
			matchIndex = i;
			break;
		}
	}
	
	//BN_LOG("checkbrand returned ", matchIndex);

	return matchIndex;
}

const char* TileManager::checkBrand() {

	static int prevMatchIndex = -1;

	switch(game->roomManager.roomIndex) {
		case 23: 
		case 53: 
		case 67:
		case 89:
		case 137:
		case 157:
		case 179:
		case 223:
		case 227:
		
			break;
		default:
			prevMatchIndex = -1;
			return NULL;
			break;
	}
	
	if(!entityManager->player->inRod(exitTile)) {
		// if the player doesnt have the exit tile, return
		cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
		prevMatchIndex = -1;
		return NULL;
	}
	
	
	// should this variable be kept update whenever updatetile is called as to not be expensive?
	// what datatype is the least expensive for this?
	unsigned roomState[6] = {0, 0, 0, 0, 0, 0};
	
	
	for(int y=1; y<=6; y++) {
		unsigned temp = 0;
		for(int x=4; x<=9; x++) {
			temp = ((temp << 1) | !!hasFloor(x, y));
		}
		roomState[y-1] = temp;
	}
	

	

	//BN_LOG(matches[0], matches[1], matches[2], matches[3], matches[4], matches[5], matches[6], matches[7], matches[8], matches[9]);
	
	int matchIndex = checkBrandIndex(roomState);
	
	
	
	const bn::regular_bg_item* lordBackgrounds[8] = {
	&bn::regular_bg_items::dw_spr_lordborders_index0,
	&bn::regular_bg_items::dw_spr_lordborders_index1,
	&bn::regular_bg_items::dw_spr_lordborders_index2,
	&bn::regular_bg_items::dw_spr_lordborders_index3,
	&bn::regular_bg_items::dw_spr_lordborders_index4,
	&bn::regular_bg_items::dw_spr_lordborders_index5,
	&bn::regular_bg_items::dw_spr_lordborders_index6,
	&bn::regular_bg_items::dw_spr_lordborders_index7
	};
	
	//BN_LOG(matchIndex, " ", prevMatchIndex);
	
	if(matchIndex != -1) {
	
	
		if(matchIndex != prevMatchIndex) {
			if(matchIndex < 8) {
				cutsceneManager->cutsceneLayer.rawMap.create(*lordBackgrounds[matchIndex], 1);
			} else {
				cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
			}
			
			//cutsceneManager->cutsceneLayer.rawMap.bgPointer.set_palette(game->pal->getBGPaletteFade(2, false));
			cutsceneManager->cutsceneLayer.rawMap.bgPointer.set_palette(game->pal->getBlackBGPalette());
			
			
			//BN_LOG(matchIndex, " ", prevMatchIndex);
			effectsManager->fadeBrand();
			prevMatchIndex = matchIndex;
		}
	
		return destinations[matchIndex];
	}
	cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
	
	prevMatchIndex = matchIndex;
	return NULL;
}

TileManager::~TileManager() {

	if(entityManager->player != NULL) {
		Player* player = entityManager->player;
		
		for(int i=0; i<player->rod.size(); i++) {
			delete player->rod[i];
			player->rod[i] = NULL;
		}
		player->rod.clear();
	
	}
	
	// will leftover pointers leave problems?
	exitTile = NULL;
	rodTile = NULL;
	locustTile = NULL;
	locustCounterTile = NULL;
	memoryTile = NULL;
	wingsTile  = NULL;
	swordTile  = NULL;
	
	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}
			floorMap[x][y] = NULL;
		}
	}
	
	
}

// ----- 

void TileManager::doFloorSteps() { 
	
	// rlly should of made a tilemanager
	
	// STEP UPDATES OCCUR AFTER DOING EXIT CHECKING, 
	// SINCE IF EVERY BUTTON IS PRESSED, YOU HAVE ONE TICK TO EXIT
	// but,, what about shadows,, this is weird
	// THIS DELAYED PRESS THING ONLY WORKS IF IT WAS A NONPLAYER ENTITY PRESSING IT!!
	
	// CURRENTLY SINCE I WANTED TO STORE THE ENTITYTYPE, POSES ARE
	// NO LONGER UNIQUE
	
	bn::optional<Entity*> res;
	
	// horrid memory usage
	static SaneSet<Pos, MAXENTITYSPRITES> stepOns;
	static SaneSet<Pos, MAXENTITYSPRITES> stepOffs;
	stepOns.clear();
	stepOffs.clear();
	
	for(auto it = floorSteps.cbegin(); it != floorSteps.cend(); ++it) {
		
		BN_ASSERT((*it).second.first != (*it).second.second, "in doFloorSteps, calculating steps. why is a move here with the same start and end??");
		
		stepOffs.insert((*it).second.first);
		stepOns.insert((*it).second.second);
	}
	
	// stepoffs occur before stepons bc of shadows, and switches
	for(auto it = stepOffs.cbegin(); it != stepOffs.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) [[likely]] {
			stepOff(tempPos);
		}
	}
	
	for(auto it = stepOns.cbegin(); it != stepOns.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) [[likely]] {
			stepOn(tempPos);
		}
	}
	
	// ok now we are in that class. im still going to leave parts ofthis hardcoded tho
	for(auto it = floorSteps.cbegin(); it != floorSteps.cend(); ++it) {
		if((*it).first == EntityType::Player) {
			
			Pos start = (*it).second.first;
			//Pos end = (*it).second.second;
			
			// todo,  could i impliment deathtiles in here?
			// but since floorsteps only has the entity type and not the entity, i cant?
			// gods 
			// actually i just now realized that this is in a if player ifstatement
			
			if(hasFloor(start) == TileType::Copy) { 
				entityManager->shadowQueue.push_back(start);
			}
		}
	}
	

	stepOns.clear();
	stepOffs.clear();
	floorSteps.clear();
	
	if(hasFloor(entityManager->player->p) == TileType::Exit && Switch::pressedCount == Switch::totalCount) {
		entityManager->addKill(NULL);
	}
	
	
	// this does not need to be called every time
	// if it causes slowdown, fix it
	// this rlly should just call,,,, updatetile,, i think?
	floorLayer.reloadCells();
	
	// IS THIS NEEDED 
	game->collision.reloadCells();
	
	// calling this here may be excessive!
	checkBrand();
}

void TileManager::updateTile(const Pos& p) { 
	
	const u8 x = p.x;
	const u8 y = p.y;

	if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
		delete floorMap[x][y];
		floorMap[x][y] = NULL;
	}
	
	if(floorMap[x][y] == NULL) {
		// the collision check isnt needed, but im keeping it here just in case
		// i also could(and maybe should?) use the hascollison func. 
		// idk abt speed but it would be good to standardize it, as im currently doing with the hasfloor func
		
		if(y > 0 && !hasCollision(Pos(x, y-1)) && hasFloor(x, y-1) && floorMap[x][y-1]->drawDropOff()) {
			FloorTile::drawDropOff(x, y);
		} else {
			FloorTile::drawPit(x, y);
		}
		
		// some issues could be caused here if this tile isnt freed yet. 
		// rewrite all map calls to hasfloor
		if(y < 8 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawPit(x, y+1);
		}
		
	} else {
		floorMap[x][y]->draw();
		
		if(floorMap[x][y]->drawDropOff() && y < 7 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawDropOff(x, y+1);
		}
	}

	
}

void TileManager::updateExit() {
	if(exitTile == NULL) {
		return;
	}
	if(entityManager->player->inRod(exitTile)) {
		return;
	}
	updateTile(exitTile->tilePos);
}

void TileManager::updateRod() {
	// i dont like the way these funcs are being coded tbh
	if(rodTile == NULL) {
		return;
	}
	if(entityManager->player->inRod(rodTile)) {
		return;
	}
	
	updateTile(rodTile->tilePos);
}

void TileManager::updateLocust() {

	if(!entityManager->player->inRod(locustTile)) {
		updateTile(locustTile->tilePos);
	}
	
	if(!entityManager->player->inRod(locustCounterTile)) {
		
		//if(entityManager->player->locustCount != 0) {
		locustCounterTile->first = '0' + ((entityManager->player->locustCount / 10) % 10);
		locustCounterTile->second = '0' + (entityManager->player->locustCount % 10);
		//

		updateTile(locustCounterTile->tilePos);
	}
	
}

void TileManager::updateVoidTiles() {
	
	bool isVoided = entityManager->player->isVoided;
	//BN_LOG("brrhuasdiofhsjkfsl, ", isVoided);
	if(!entityManager->player->inRod(voidTile1)) {
		
		voidTile1->first = isVoided ? 'V' : 'H';
		voidTile1->second = isVoided ? 'O' : 'P';
		
		updateTile(voidTile1->tilePos);
	}
	
	if(!entityManager->player->inRod(voidTile2)) {
		
		voidTile2->first = isVoided ? 'I' : '0';
		voidTile2->second = isVoided ? 'D' : '7';
		
		updateTile(voidTile2->tilePos);
	}
	
	
}

void TileManager::updateBurdenTiles() {
	
	if(!entityManager->player->inRod(memoryTile)) {
		updateTile(memoryTile->tilePos);
	}
	
	if(!entityManager->player->inRod(wingsTile)) {
		updateTile(wingsTile->tilePos);
	}
	
	if(!entityManager->player->inRod(swordTile)) {
		updateTile(swordTile->tilePos);
	}
	
}

int TileManager::getLocustCount() {
	
	if(entityManager->player->inRod(locustCounterTile) || 
		entityManager->player->inRod(locustTile)) {
		return 0;
	}

	
	Pos counterPos = locustCounterTile->tilePos;
	Pos locustPos = locustTile->tilePos;
	locustPos.move(Direction::Right);
	
	if(locustPos != counterPos) {
		return 0;
	}
	
	return locustCounterTile->getNumber();
}

int TileManager::getRoomIndex() {
	
	if(entityManager->player->inRod(floorTile2)) {
		return -1;
	}
	
	Pos floor2Pos = floorTile2->tilePos;
	Pos floor1Pos = floorTile1->tilePos;
	floor1Pos.move(Direction::Right);
	
	if(floor1Pos != floor2Pos) {
		return -1;
	}
	
	char temp = floorTile1->second;
	if(temp == '?') {
		return -1;
	}
	
	return ((floorTile1->second - '0') * 100) + floorTile2->getNumber();
}

bool TileManager::hasCollision(const Pos& p) {
	return entityManager->hasCollision(p);
}

void TileManager::fullDraw() { 
	
	floorLayer.draw(game->collisionMap, floorMap);
	
	// i do not like this!
	updateExit();
	updateRod();
	updateLocust();
	updateVoidTiles();
	updateBurdenTiles();
}

bool TileManager::exitRoom() {
	return true;
}

bool TileManager::enterRoom() {
	return true;
}

void TileManager::doVBlank() { profileFunction();
	
	// things like glass breaking(and maybe others) should occur in here!
	
	
	// i PRAY that this doesnt kill performance. something sorta similar to this may have (curse)ed performance back when i was trying to get death tiles working?
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] == NULL) {
				continue;
			}
			
			BN_ASSERT(floorMap[x][y]->isAlive, "a floor tile wasnt alive during vblank?? how??");
			
			if(floorMap[x][y]->isSteppedOn) {
				floorMap[x][y]->isSteppedOnAnimation();
			} 
			if(floorMap[x][y]->tileType() == TileType::Exit) { // not ideal code.
				static_cast<Exit*>(floorMap[x][y])->isFirstCall = false;
			}
		}
	}
	
	// i wonder/hope this is ok 
	// sword tile needs to be updated every,, frame for flashing?
	if(swordTile != NULL && !entityManager->player->inRod(swordTile)) {
		//updateTile(swordTile->tilePos);
		swordTile->draw();
		floorLayer.reloadCells(); // might cause horrid lag
	}
	
	
	
	
	
	return;
}

// -----

bn::optional<TileType> TileManager::hasFloor(const u8& x, const u8& y) { 
	const FloorTile* temp = floorMap[x][y];
	
	if(temp == NULL || !temp->isAlive) {
		return bn::optional<TileType>();
	}
	
	return bn::optional<TileType>(temp->tileType());
}

void TileManager::stepOff(Pos p) { 
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepoff on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOff();
}

void TileManager::stepOn(Pos p) { 
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepon on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOn();
}


