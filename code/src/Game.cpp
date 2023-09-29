
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

void Game::resetRoom(bool debug) {
	
	BN_LOG("entered reset room with debug=",debug);
	
	if(!debug) {
		//tileManager.fullDraw();
		state = GameState::Exiting;
		
		// wait for animations to finish 
		// IS THERE A NON BUSYLOOP VER OF THIS?

		while(state == GameState::Exiting) { // wait for gamestate to no longer be exiting
			//BN_LOG("looping on ", state);
			debugText.updateText();
			bn::core::update(); 
		} 
	}

	state = GameState::Loading;
	
	BN_LOG("resetroom called");
	loadLevel();
	BN_LOG("loadlevel finished");
	fullDraw();
	BN_LOG("fulldraw finished");
	
	state = GameState::Normal;
	
	if(!debug) {
		state = GameState::Entering;
		while(state == GameState::Entering) { // wait for gamestate to no longer be entering
			//BN_LOG("looping on ", state);
			debugText.updateText();
			bn::core::update(); 
		}
	}
	
	BN_ASSERT(state == GameState::Normal, "after a entering gamestate, the next state should be normal");
	
}

void Game::loadLevel() {

	Room idek = roomManager.loadRoom();

	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			
			collisionMap[x][y] = ((u8*)idek.collision)[x + 14 * y];
			detailsMap[x][y] = ((u8*)idek.details)[x + 14 * y];
			
			// when changing the color palettes, some weird shit happened where, areas 
			// would be transparent when they shouldnt be? this hopefully fixes that
		
			if(collisionMap[x][y] == 0) {
				collisionMap[x][y] = 1;
			} 
			
			/*if(detailsMap[x][y] == 0) {
				detailsMap[x][y] = 1;
			} */
		}
	}
	
	TileType* floorPointer = (TileType*)idek.floor;
	
	
	
	tileManager.loadTiles(floorPointer);

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;
	
	entityManager.loadEntities(entitiesPointer, entitiesCount);
	
	effectsManager.reset();
	
}

void Game::fullDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap);

	tileManager.fullDraw();
	
	entityManager.fullUpdate();
}

void Game::changePalette(int offset) {
	
	// https://stackoverflow.com/questions/3417183/modulo-of-negative-numbers
	// lol
	
	const int paletteListSize = (int)(sizeof(paletteList) / sizeof(paletteList[0]));
	
	paletteIndex += offset;
	
	paletteIndex = ((paletteIndex % paletteListSize) + paletteListSize) % paletteListSize;
	
	entityManager.updatePalette(paletteList[paletteIndex]);
	
	collision.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	details.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	tileManager.floorLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	effectsManager.effectsLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	
	BackgroundMap::backgroundPalette = paletteList[paletteIndex];
	
}

Game* globalGame = NULL;

unsigned int frame = 0;

void didVBlank() {
	
	//BN_LOG("vblanked ", frame);
	
	frame = (frame + 1) % 60000;
	
	globalGame->doVBlank();
	
	//bn::core::update();
}

void Game::doVBlank() {
	static bool a, b, c = false;
	
	switch(state) {
		default:
		case GameState::Normal:
			entityManager.doVBlank();
			effectsManager.doVBlank();
			tileManager.doVBlank();
			break;
		case GameState::Exiting:
			if(!a) { a = entityManager.exitRoom(); }
			if(!b) { b = effectsManager.exitRoom(); }
			if(!c) { c = tileManager.exitRoom(); }
			if(a && b && c) {
				state = GameState::Entering;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Entering:
			if(!a) { a = entityManager.enterRoom(); }
			if(!b) { b = effectsManager.enterRoom(); }
			if(!c) { c = tileManager.enterRoom(); }
			if(a && b && c) {
				state = GameState::Normal;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Loading:
			break;
	}
	
	
}

void Game::run() {
	
	BN_LOG("hello?");
	
	globalGame = this;

	bn::core::set_vblank_callback(didVBlank);
	
	bn::timer inputTimer;

	//resetRoom();
	
	state = GameState::Loading;
	
	loadLevel();
	fullDraw();
	
	state = GameState::Normal;
	
	while(true) {
		
		miscDebug = LevStatue::rodUses;
		miscDebug2 = LevStatue::totalLev;
		
		if(bn::keypad::l_held() || bn::keypad::r_held()) {
			if(bn::keypad::l_held()) {
				roomManager.prevRoom();
			} else {
				roomManager.nextRoom();
			}
			resetRoom(true);
			
			miscTimer.restart();
			
			while(miscTimer.elapsed_ticks() < 254843 / 12) {
			
			}
		
			debugText.updateText();
			bn::core::update();

			continue;
		}
		
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > 254843 / 20) {
			
			inputTimer.restart();
			
			if(bn::keypad::start_pressed()) {
				changePalette(1);
				continue;
			} else if(bn::keypad::select_pressed()) {
				changePalette(-1);
				continue;
			}
			
			entityManager.doMoves();

			if(entityManager.hasKills()) {
				resetRoom(NULL);
				continue;
			}
			
			// inefficient, i rlly should of had a floormanager class.
			
			tileManager.fullDraw();
			
		}

		debugText.updateText();
		bn::core::update();
	}
}

