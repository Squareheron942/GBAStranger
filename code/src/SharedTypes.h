#pragma once

#include "dataWinIncludes.h"

#include "bn_sound.h"
#include "bn_unordered_map.h"
#include "bn_regular_bg_tiles_ptr.h"



#include "Profiler.h"

//#include <cstring>

#include "bn_bg_tiles.h"
#include "bn_bg_maps.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_tiles.h"
#include "bn_sprite_palettes.h"
//#include "bn_best_fit_allocator.h"

#include "bn_version.h"

#include "bn_music_items.h"
//#include "bn_music_actions.h"

#include "bn_music.h"
#include "bn_sound.h"
#include "bn_sound_items.h"
#include "bn_sound_actions.h"
#include "common_variable_8x8_sprite_font.h"

#include "bn_blending.h"

// it is so (curse)ing dumb that i didnt realize this was a thing until now 
#include "bn_window.h"
#include "bn_rect_window.h"

#include "bn_bg_palette_ptr.h"

#include "bn_istring_base.h"

#include "bn_random.h"
#include "bn_sram.h"

#include "bn_sprites_mosaic.h"

#include "bn_link.h"
#include "bn_link_state.h"
#include "bn_link_player.h"

// danger zone, again 
#include "bn_bg_blocks_manager.h"

#include "bn_green_swap.h"

#ifdef ENABLELOGGING

#include "bn_log.h"

/*
#undef BN_LOG

#define BN_LOG_MOD(...) \
        do \
        { \
            char _bn_string[BN_CFG_LOG_MAX_SIZE]; \
            bn::istring_base _bn_istring(_bn_string); \
            bn::ostringstream _bn_string_stream(_bn_istring); \
            _bn_string_stream.append_args(__VA_ARGS__); \
            bn::log(_bn_istring); \
        } while(false)
			
#define BN_LOG(...) do { BN_LOG_MOD(__FILE__, " ", __LINE__);  BN_LOG_MOD(__VA_ARGS__); } while(false)
*/

#else 
#define BN_LOG(...) do {} while (false)
#endif



/*
#include "bn_log.h"
#include "bn_config_log.h"
#include "bn_log_backend.h"
*/

#include "Palette.h"

// getting this include to work was such a painful process for reasons i still dont get. a make clean fixed them(i think)?
//#include "bn_profiler.h"
//#include "bn_config_profiler.h"

#include <bn_deque.h>

#include "fontData.h"

#include "bn_time.h"
#include "bn_date.h"


#ifdef DISABLEASSERTS

#warning asserts are disabled! you might have a bad time (sans noises)

#undef BN_ASSERT
#define BN_ASSERT(...) do {} while (false)

#endif

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//#define ABS(v) ( v < 0 ? -v : v )
// gods this is dumb, and one of the most ANNOYING bugs ive had to find recently
#define ABS(v) ( (v) < 0 ? -(v) : (v) )

#define CLAMP(value, min_val, max_val) MAX(MIN((value), (max_val)), (min_val))

typedef unsigned char u8;

// unsure of accuracy, but will make things slightly easier, hopefully
#define FRAMETICKS 4372

// TODO, GO OVER ALL FUNCS, AND DEFINE WHAT CAN BE AS CONST REF
// idrk if c++ optimization does that for me? but regardless its a good idea

#define MAXSPRITES 128

//#define MAXTEXTSPRITES 56
//#define MAXEFFECTSPRITES 8

#define MAXTEXTSPRITES 128
#define MAXEFFECTSPRITES 112

//#define MAXENTITYSPRITES 12
#define MAXENTITYSPRITES 64

//#define MAXENTITYSPRITES MAXSPRITES - MAXTEXTSPRITES - MAXEFFECTSPRITES

//static_assert(MAXENTITYSPRITES > 0);

extern unsigned int frame;
extern int playerIdleFrame;
extern bool isVblank;
extern unsigned boobaCount;
extern unsigned playerMoveCount;

extern bn::random randomGenerator;

extern void delay(int delayFrameCount);

extern unsigned short* col0;
extern unsigned short* col1;
extern unsigned short* col2;
extern unsigned short* col3;
extern unsigned short* col4;

extern unsigned* stareTiles;
extern unsigned short* stareMap;
extern int* stareTilesCount;
extern int* stareMapCount;

extern unsigned* glitchTiles;
extern int* glitchTilesCount;

#define MONTH (\
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
: __DATE__ [2] == 'b' ? 2 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
: __DATE__ [2] == 'y' ? 5 \
: __DATE__ [2] == 'l' ? 7 \
: __DATE__ [2] == 'g' ? 8 \
: __DATE__ [2] == 'p' ? 9 \
: __DATE__ [2] == 't' ? 10 \
: __DATE__ [2] == 'v' ? 11 \
: 12)

__attribute__((section(".iwram"))) unsigned short bruhRand();

__attribute__((noinline, optimize("O0"), target("arm"), section(".iwram"))) unsigned getMiscData();

void doNothing();

enum class GameState {
	Normal, // normal gameplay
	Exiting, // we either just completed a level, or just died
	Entering, // we just exited the exiting state and are now reloading stuffs. 
	Loading, // loading in new data, do nothing for now.
	Paused, // either actually paused(which i havent even thought about making) or in dialogue.
	Dialogue,
	Cutscene,
	Sleep,
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const GameState& e) {
static const char *GameStateToString[] ={ 
	"Normal",
	"Exiting", 
	"Entering",
	"Loading",
};

	stream << GameStateToString[static_cast<int>(e)];
	return stream;

}

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

// all these ostreams should of been added days ago omfg.
// nothing i googled mentioned i could overload ostream for enums
inline bn::ostringstream& operator<<(bn::ostringstream& stream, const Direction& e) {
static const char *DirectionToString[] ={ 
  	"Up",
	"Down",
	"Left",
	"Right",
};

	stream << DirectionToString[static_cast<int>(e)];
	return stream;

}

enum class EntityType {
	Entity,
	
	Player,
	
	Leech,
	Maggot,
	Eye,
	Bull,
	Chester,
	
	Mimic,
	WhiteMimic,
	GrayMimic,
	BlackMimic,
	
	Diamond,
	Shadow,
	
	Boulder,
	Chest,
	
	AddStatue,
	EusStatue,
	BeeStatue,
	MonStatue,
	TanStatue,
	GorStatue,
	LevStatue,
	CifStatue,
	
	Interactable,
	
	// i am not sure if this is the best way to do this, but I am going with it 
	EmptyChest
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const EntityType& e) {
static const char *EntityTypeToString[] ={ 
  	"Entity",
	"Player",
	"Leech",
	"Maggot",
	"Eye",
	"Bull",
	"Chester",
	"Mimic",
	"WhiteMimic",
	"GrayMimic",
	"BlackMimic",
	"Diamond",
	"Shadow",
	"Boulder",
	"Chest",
	"AddStatue",
	"EusStatue",
	"BeeStatue",
	"MonStatue",
	"TanStatue",
	"GorStatue",
	"LevStatue",
	"CifStatue",
	"Interactable",
	"EmptyChest"
};

	stream << EntityTypeToString[static_cast<int>(e)];
	return stream;

}

enum class TileType {
	Pit,
	Floor,
	Glass,
	Bomb,
	Death,
	Copy,
	Exit,
	Switch,
	WordTile,
	RodTile,
	LocustTile,
	SpriteTile,
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const TileType& e) {
static const char *TileTypeToString[] ={ 
	"Pit",
	"Floor",
	"Glass",
	"Bomb",
	"Death",
	"Copy",
	"Exit",
	"Switch",
	"WordTile",
	"RodTile",
	"LocustTile",
	"SpriteTile",
};

	stream << TileTypeToString[static_cast<int>(e)];
	return stream;

}

class BackgroundMap {
public:

	bn::regular_bg_map_cell cells[32 * 32];
	bn::regular_bg_map_item mapItem;
	//bn::regular_bg_item bg_item;
	bn::regular_bg_ptr bgPointer;
	bn::regular_bg_map_ptr bgMap;

	static Palette* backgroundPalette;
	
	// bn::bg_palette_item palette
	// how in tarnation can butano look you in the face, and say "yea do this for a background map"
	BackgroundMap(bn::regular_bg_tiles_item& tileset, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),
		
		// it seems that,, bg_item holds on to whatever previous tileset it was?
		
		bgPointer(bn::regular_bg_item(tileset, 
		
		backgroundPalette->getBGPalette()
		
		,mapItem).create_bg(8, 48)),
		bgMap(bgPointer.map())
		{
			
			// for all tilesets from the game, tile 0 
			// is just a checkerboard. if possible, i rlly, and i mean really, 
			// should overwrite that with transparent here.
			// actually, (curse) it, ill just do it in preprocessing.
			
			init(zIndex);
			
		}
	
	// alternate constructor for when (trying) to use an allocated tileset instead of a normal one(for vram modifications)
	BackgroundMap(bn::regular_bg_tiles_ptr& tilesPointer, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),

		bgPointer(bn::regular_bg_item(
			tilesPointer.vram().value(), 
			backgroundPalette->getColors(),
			backgroundPalette->getBPP(),
			*cells,
			bn::size(32, 32)
		).create_bg(8, 48)),
		
		bgMap(bgPointer.map())
	{
		init(zIndex);	
	}
	
	BackgroundMap(const bn::regular_bg_item& bgItem, int zIndex = 0) :
	mapItem(cells[0], bn::size(32, 32)),
	bgPointer(bgItem.create_bg(8, 48)),
	bgMap(bgPointer.map())
	{
		//init(zIndex);
		bgPointer.set_priority(zIndex);
	}
	
	
	void create(const bn::regular_bg_item& bgItem, int zIndex = 0) {
		bgPointer.set_priority(zIndex);
		
		//auto temp = bgItem.create_bg(8, 48);
		//bgPointer = bgItem.create_bg(8, 48, 0);
		
		//bgMap = bgItem.map_item();
		
		//bgMap.reload_cells_ref();
		
		bgPointer.set_tiles(bgItem.tiles_item());
		bgPointer.set_map(bgItem.map_item());
		
		bgPointer.set_x(8 + 8);
		bgPointer.set_y(8 + 48);
		
		// does this do a vblank? if so i mean ill just cover it with another bg ig
		reloadCells();
		
	}
		
	void init(int zIndex) {
		//bgPointer.set_z_order(zIndex);
			bgPointer.set_priority(zIndex);
			
			//setTile(0,21,4);
			//setTile(31,31,4);

			setTile(1,21,4);
			setTile(1,22,4);
			setTile(1,23,4);
			
			setTile(4,21,4);
			setTile(4,22,4);
			setTile(4,23,4);
			
			setTile(2,24,4);
			setTile(3,24,4);
			
			// -----
			
			setTile(7,21,4);
			setTile(7,22,4);
			setTile(7,23,4);
			
			setTile(9,21,4);
			setTile(9,22,4);
			setTile(9,23,4);
			
			setTile(11,21,4);
			setTile(11,22,4);
			setTile(11,23,4);
			
			setTile(8,24,4);
			setTile(10,24,4);
			
			// -----
			
			setTile(14,21,4);
			setTile(14,22,4);
			setTile(14,23,4);
			
			setTile(17,21,4);
			setTile(17,22,4);
			setTile(17,23,4);
			
			setTile(15,24,4);
			setTile(16,24,4);
	}
	
	void setTile(int x, int y, int tileIndex) {
	
		bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell = current_cell_info.cell(); 
	
		// if lag happens to occur, we could not update this until a frame is done
		// yep, lag occured.
		// sorta fixed it, but to be safe, im ballin here
		//bgMap.reload_cells_ref();
	}
	
	void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) {
		
		bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell_info.set_horizontal_flip(flipX);
		current_cell_info.set_vertical_flip(flipY);
		
		current_cell = current_cell_info.cell(); 
	
	}
	
	void reloadCells() {
		bgMap.reload_cells_ref();
	}
	
};

class Layer {
public:

	// rawmap is now a pointer to avoid all the bs in regards to statically allocating the bs in it
	// WILL THIS BEING IN HEAP SLOW (curse) DOWN??
	// bc if so,, we are (curse)ed
	BackgroundMap rawMap;
	// dont ask
	Layer(bn::regular_bg_tiles_item tileset, int zIndex, int fillIndex = 0) :
	rawMap(tileset, zIndex)
	{				
			init(fillIndex);
		}
		
	Layer(bn::regular_bg_tiles_ptr tilesPointer, int zIndex, int fillIndex = 0) :
		rawMap(tilesPointer, zIndex)
		{
			init(fillIndex);
		}
		
	Layer(bn::regular_bg_item bgItem, int zIndex = 0) :
	rawMap(bgItem, zIndex)
	{
		rawMap.reloadCells();
	}
		
	void init(int fillIndex) {
		
		//setup black border., just black the whole screen
		for(int i=0; i<30; i++) {
			for(int j=0; j<20; j++) {
				rawMap.setTile(i, j, fillIndex);
			}
		}
		rawMap.reloadCells();
	}
		
	virtual ~Layer() = default;
	
	virtual void draw(u8 (&gameMap)[14][9]) {
		
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				
				u8 tile = gameMap[x][y];
			
				rawMap.setTile(x * 2 + 1, y * 2 + 1, 4 * tile); 
				rawMap.setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1); 
				rawMap.setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2); 
				rawMap.setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3); 
			}
		}
		
		rawMap.reloadCells();
	}
	
	void setTile(int x, int y, int tileIndex) { 
		rawMap.setTile(x, y, tileIndex); 
	}
	
	void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) { 
		rawMap.setTile(x, y, tileIndex, flipX, flipY);
	}
	
	u8 tempTileIndicies[4];
	
	void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
		// this func actually being able to flip (curse) properly is UNCONFIRMED bc I AM SLEEPY
		
		tempTileIndicies[0] = 4 * tile + ((flipY << 1) | flipX);
		tempTileIndicies[1] = 4 * tile + ((flipY << 1) | !flipX);
		tempTileIndicies[2] = 4 * tile + ((!flipY << 1) | flipX);
		tempTileIndicies[3] = 4 * tile + ((!flipY << 1) | !flipX);
		
		rawMap.setTile(x * 2 + 1, y * 2 + 1, tempTileIndicies[0], flipX, flipY); 
		rawMap.setTile(x * 2 + 2, y * 2 + 1, tempTileIndicies[1], flipX, flipY); 
		rawMap.setTile(x * 2 + 1, y * 2 + 2, tempTileIndicies[2], flipX, flipY); 
		rawMap.setTile(x * 2 + 2, y * 2 + 2, tempTileIndicies[3], flipX, flipY); 	
	}
	
	// goofy
	void update() {
		rawMap.reloadCells();
	}
	
	void reloadCells() {
		rawMap.reloadCells();
	}
	
};

class Pos {
public:

	int x;
	int y;

	Pos(int x_, int y_) : x(x_), y(y_) { 

		/*
		if(!(x >= 0 && y >= 0 && x < 14 && y < 9)) {
			// this is a remnant of the one time i had to use vscode for debugging.
			BN_LOG("bruh");
		}
		*/

		BN_ASSERT(x >= 0 && y >= 0 && x < 14 && y < 9, "invalid pos created at ", x, " ", y);
	}
	
	Pos(const Pos& other) : x(other.x), y(other.y) {}
	
	Pos& operator=(const Pos& other) {
        if (this != &other) {
            x = other.x;
			y = other.y;
        }
        return *this;
    }
	
	bool operator<(const Pos& other) const {
		return (x + 14 * y) < (other.x + 14 * y);
	}
	
	Pos operator+(const Pos& other) {

		int tempX = x + other.x;
		int tempY = y + other.y;
		
		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }
		
		return Pos(tempX, tempY);
	}
	
	Pos operator-(const Pos& other) {

		int tempX = x - other.x;
		int tempY = y - other.y;
		
		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }
		
		return Pos(tempX, tempY);
	}
	
	bool move(Direction moveDir) {
		
		/*
		int newX = x;
		int newY = y;
		
		switch (moveDir) {
			case Direction::Up:
				newY -= 1;
				break;
			case Direction::Down:
				newY += 1;
				break;
			case Direction::Left:
				newX -= 1;
				break;
			case Direction::Right:
				newX += 1;
				break;
			default:
				break;
		}
		
		if(newX < 0 || newY < 0 || newX >= 14 || newY >= 9) {
			return false;
		}
		
		x = newX;
		y = newY;
		
		return true;
		*/
		
		
	
		
		switch (moveDir) {
			case Direction::Up:
				if(y == 0) {
					return false;
				}
				y -= 1;
				break;
			case Direction::Down:
				if(y == 8) {
					return false;
				}
				y += 1;
				break;
			case Direction::Left:
				if(x == 0) {
					return false;
				}
				x -= 1;
				break;
			case Direction::Right:
				if(x == 13) {
					return false;
				}
				x += 1;
				break;
			default:
				break;
		}
		
		
		/*
		if(newX < 0 || newY < 0 || newX >= 14 || newY >= 9) {
			return false;
		}
		*/
		
		return true;
		
	}
	
	bool moveInvert(Direction moveDir, bool invertHorizontal, bool invertVertical) {
	
		if(invertHorizontal) {
			if(moveDir == Direction::Left) {
				moveDir = Direction::Right;
			} else if(moveDir == Direction::Right) {
				moveDir = Direction::Left;
			}
		}
		
		if(invertVertical) {
			if(moveDir == Direction::Up) {
				moveDir = Direction::Down;
			} else if(moveDir == Direction::Down) {
				moveDir = Direction::Up;
			}
		}
		
		return move(moveDir);
	}
	
	bool operator==(Pos const& rhs) const { return x == rhs.x && y == rhs.y; }

	bool sanity() const {
		// checks if im going insane.
		return x >= 0 && y >= 0 && x < 14 && y < 9;
	}
	
};

inline Pos safePos(signed char x, signed char y) {

	if(x < 0) { x = 0; }
	if(y < 0) { y = 0; }
	if(x >= 14) { x = 13; }
	if(y >= 9) { y = 8; }
	
	return Pos(x, y);
}

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const Pos& p) {
	//stream << "(" << p.x << ", " << p.y << ")";
	stream.append("(");
	stream.append(p.x);
	stream.append(", ");
	stream.append(p.y);
	stream.append(")");

	return stream;
}

struct EntityHolder {
	const EntityType t;
	//const u8 x;
	//const u8 y;
	const unsigned short x;
	const unsigned short y;
};

struct EffectHolder {
	const bn::sprite_tiles_item* tiles;
	const int x;
	const int y;
	const int width;
	const int height;
	const bool collide = true;
	const int priority = 2;
	const bool autoAnimate = false;
};

struct SecretHolder {
	
	// holds the,,, yea secrets
	// if this secret is on top of an exit,, how do i track that?
	// ill just set a flag on load.
	// as for the 0 array issue, first thing will have a pos of -1, -1
	// if dest is null, just send to the next thing in the list? i suppose
	// ordering will have to be done manually, but tbh that is fine
	
	const int x;
	const int y;
	
	const char* dest;
	const char sanity = '\0'; // i dont trust this program.
};

class Room {
public:
	const void* collision;
	const void* floor;
	const void* details;

	const void* entities;
	const int entityCount;
	
	const void* effects;
	const int effectsCount;
	
	const void* secrets;
	const int secretsCount;
	
	const void* exitDest;
	
	const void* collisionTiles;
	const void* detailsTiles;

	constexpr Room(
	const void* collision_, const void* floor_, const void* details_, 
	const void* entities_, const int entityCount_, 
	const void* effects_, const int effectsCount_,
	const void* secrets_, const int secretsCount_,
	const void* exitDest_,
	const void* collisionTiles_, const void* detailsTiles_
	) :
	collision(collision_), floor(floor_), details(details_), 
	entities(entities_), entityCount(entityCount_), 
	effects(effects_), effectsCount(effectsCount_),
	secrets(secrets_), secretsCount(secretsCount_),
	exitDest(exitDest_),
	collisionTiles(collisionTiles_), detailsTiles(detailsTiles_)
	{ }
	
};

//#include "Palette.h"

//class Entity;

//template <size_t Size>
//using EntitySet = bn::unordered_set<Entity*, Size, bn::hash<Entity*>, bn::equal_to<Entity*>>;

//template <size_t Size>
//using EntitySetIterator = EntitySet<Size>::iterator;
//using EntitySetIterator = bn::unordered_set::iterator;

// set without insanity. slower? def, but im so (curse)ing done
// ill haeve the backend be a vector.
// resizing on every insert,,, gods
// i however, cannot give a (curse) anymore :) 
// spent way to long on this only to stop being a (curse) and understand what pointer stability is
// i could maybe use,, unique pointer? but tbh i just dont want to 
// actually, we only have unique pointers! no shared, so im doing this
// insertion will be slow, but at least lookup will be fast.
template <typename T, int maxVecSize>
class SaneSet {
private:
	
	bn::vector<T, maxVecSize> data;

	int binarySearch(const T& elem) const {
		int left = 0;
        int right = data.size() - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (data[mid] == elem) {
                return mid;
            } else if (data[mid] < elem) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
		return -1;
	}
	
	int getInsertIndex(const T& elem) const {
        int left = 0;
        int right = data.size();

        while (left < right) {
            int mid = left + (right - left) / 2;

            if (data[mid] < elem) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        return left;
    }
	
public:

	SaneSet() {}	

	SaneSet(const SaneSet& other) : data(other.data) {}
	
	SaneSet& operator=(const SaneSet& other) { 
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
	
	void insert(const T& elem) {
		int index = binarySearch(elem);
        if (index == -1) {
            // Element not found, insert it at the appropriate position
            int insertIndex = getInsertIndex(elem);
            data.insert(data.begin() + insertIndex, elem);
        }
	}
	
	bool contains(const T& elem) const {
		if(data.size() == 0) {
			return false;
		}
		return binarySearch(elem) != -1;
	}
	
	bn::vector<T, maxVecSize>::iterator erase(const T& elem) {
        int index = binarySearch(elem);
        if (index != -1) {
            return data.erase(data.begin() + index);
        }
		return data.end();
    }
	
	bn::vector<T, maxVecSize>::iterator erase(const bn::vector<T, maxVecSize>::iterator it) {
		return data.erase(it);
	}
	
	bn::vector<T, maxVecSize>::iterator insert(const bn::vector<T, maxVecSize>::iterator it) {
		int index = binarySearch(*it);
        if (index == -1) {
            // Element not found, insert it at the appropriate position
            int insertIndex = getInsertIndex(*it);
            return data.insert(data.begin() + insertIndex, *it);
        }
		return data.begin() + index;
	}
	
	int size() const {
		return data.size();
	}
	
	// is returning these iterators,,, ok??
	
	auto begin() {
		return data.begin();
	}
	
	auto end() {
		return data.end();
	}
	
	auto cbegin() const {
		return data.cbegin();
	}
	
	auto cend() const {
		return data.cend();
	}
	
	void clear() {
		data.clear();
	}
	
	int maxSize() const {
		return maxVecSize;
	}
	
	
	
};

template <typename T, int maxVecSize>
class SaneVector : public bn::vector<T, maxVecSize> { // war crime
public:
	
	SaneVector(std::initializer_list<T> l) {		
		for(auto it = l.begin(); it != l.end(); ++it) {
			this->push_back(*it);
		}
	}

};

inline char* strcpy(char* dest, const char* src) {
	char *original_dest = dest;

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = '\0';

	return original_dest;
}

inline char* strncpy_custom(char* dest, const char* src, size_t n) {
    char *dest_start = dest;

    while (*src != '\0' && n > 0) {
        *dest++ = *src++;
        n--;
    }

    // If n is greater than the length of src, fill the remaining with null characters.
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }

    return dest_start;
}

inline void* memset(void* ptr, int value, size_t num) {
	unsigned char *byte_ptr = (unsigned char *)ptr;
	unsigned char byte_value = (unsigned char)value;

	for (size_t i = 0; i < num; i++) {
		byte_ptr[i] = byte_value;
	}

	return ptr;
}

inline int strcmp(const char *str1, const char *str2) {
	
	if(WTF(str1) != WTF(str2)) {
		return -1;
	}
	
	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 != *str2) {
			return (*str1 - *str2);
		}
		str1++;
		str2++;
	}

	return 0;
}

struct MessageStr {
	const char* str;
	const char idek = '\0'; // sanity
};

constexpr unsigned hashString(const char* str) {
    unsigned hash = 0;

    while(*str) {
        hash = (hash * 31) + (*str);
        str++;
    }

    return hash;
}

inline char* strstrCustom(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;  // Empty needle is always found
    }

    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;

        // Check for substring match
        while (*n != '\0' && *h == *n) {
            h++;
            n++;
        }

        // If the entire substring is found, return the starting address
        if (*n == '\0') {
            return (char*)haystack;
        }

        haystack++;  // Move to the next character in the haystack
    }

    return NULL;  // Substring not found
}




