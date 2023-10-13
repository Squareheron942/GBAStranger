
import json 
import os 
import numpy as np
import re
import shutil
from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL

failures = {}

isHardMode = False


# add: id: 8 spr_voider
# eus: id: 6 spr_lover
# bee: id: 2 spr_smiler
# mon: id: 7 spr_greeder
# tan: id: 3 spr_killer
# gor: id: 5 spr_slower
# lev: id: 1 spr_watcher
# cif: id: 4 spr_atoner

class Pos:
	def __init__(self, x, y):
		self.rawX = x
		self.rawY = y
		self.x = (x - 8) // 16
		self.y = (y - 8) // 16


creationCodeData = { }

newCreationCodesData = {}

# fuck it, going to be doing this manually.
def readCreationCode(creationCode):

	#x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16
	
	if creationCode is None:
		return None
		
	
	f = open(os.path.join("../ExportData/Export_Code", creationCode + ".gml"))
	lines = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ]
	f.close()
	idek = "   ".join(lines)
	#creationCodeData[idek] = [None]
	
	if idek not in creationCodeData:
		print(RED + idek + " wasnt found in creationcode data!!! this is rlly fucking bad!!!! add it!!!!" + RESET)
		
		
		newCreationCodesData[idek] = [None]

		#return None
		exit(1)
	
	return creationCodeData[idek][isHardMode]
	
def writeFooter(f, successRoomsList):
	
	data = """

#define LOADROOMMACRO(roomName) Room((void*)&roomName::collision, (void*)&roomName::floor, (void*)&roomName::details, (void*)&roomName::entities, roomName::entityCount)	

constexpr static inline Room rooms[{:d}] = {{ {:s} }};

// inneffecient, but my gods do i not care at this point
constexpr static inline char roomNames[{:d}][{:d}] = {{ {:s} }};
"""

	idjk="""

class RoomManager {{
public:
	// what i am about to do here, is in a word, horrible 
	// but my gods im tired.
	// literally just put a array of strings and array of structs
	//,, will the structs be like
	// GODS 
	// i fucking hate how paranoid i am with memory here. 
	// im trying my best to make sure that we only load the room needed from rom but 
	// maybe im being to paranoid
	// also i havent ate.

	// getready for a bad time.
	
	// should be constexpr but im tired
	
	

	int roomIndex = 100;
	
	Room loadRoom() {{
	
		if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {{
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
		}}

		return rooms[roomIndex];
	}}
	
	const char* currentRoomName() {{ 
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {{
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}}
	
	return roomNames[roomIndex]; 
	}}

}};

"""

	successRoomsCount = len(successRoomsList)
	longestRoomStringLen = 1+len(max(successRoomsList, key=len))
	
	
	formatArray = lambda prepend, append, arr : ",".join([ "{:s}{:s}{:s}".format(prepend, elem, append) for elem in arr ])

	
	
	roomData = formatArray("LOADROOMMACRO(", ")", successRoomsList)
	roomNameData = formatArray("\"", "\"", successRoomsList)
	
	
	data = data.format(successRoomsCount, roomData, successRoomsCount, longestRoomStringLen, roomNameData)
	
	f.write(data + "\n")

	pass

def convertCollisionAndDetails(layerData):
	
	details = {}	
	
	if "Tiles_2" in layerData:
		
		details["tileset"] = layerData["Tiles_2"]["layer_data"]["background"]
		details["data"] = [ elem["id"] for line in layerData["Tiles_2"]["layer_data"]["tile_data"] for elem in line]

		if details["tileset"] == "tile_house_2":
			print("tile_house_2 aint properly converted!")
			return None
	else:
		details["data"] = [ 0 for i in range(0, 14*9) ]
	
	collision = {}	
	
	if "Tiles_1" in layerData:
		
		collision["tileset"] = layerData["Tiles_1"]["layer_data"]["background"]
		collision["data"] = [ elem["id"] for line in layerData["Tiles_1"]["layer_data"]["tile_data"] for elem in line]

		if collision["tileset"] == "tile_house_2":
			print("tile_house_2 aint properly converted!")
			return None
	else:
		collision["data"] = [ 0 for i in range(0, 14*9) ]
		
	return collision, details

def convertObjects(layerData):
	
	# due to the extremely large amount of difficulty ive had with floor objects being instances, and vice versa, this is here to fix thtat
	
	
	floorExport = [ ["Pit" for i in range(9)] for j in range(14)]
	
	entityExport = []
	
	effectExport = []
	
	# goofy 
	# oh god its so goofy
	# does python seriously not allow lambdas to be multiline/assign shit?
	# i despise that this is actually the best/most readable/easiest way to do this
	class ObjectFunctions:
		
		def obj_floor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Floor"
	
		def obj_glassfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Glass"
			
		def obj_pit(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Pit"
			
		def obj_exit(p, creationCode):
			if creationCode is not None:
				if creationCode == "b_form = 1":
					print("a exit had a bform of 1. still no idea why")
					floorExport[p.x][p.y] = "Exit"
					return
			
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Exit"
			
		def obj_floorswitch(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Switch"
			
		def obj_bombfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Bomb"
			
		def obj_deathfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Death"
			
		def obj_copyfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Copy"
			
	
	

		def obj_spawnpoint(p, creationCode):	
			entityExport.insert(0, "EntityType::Player,{:d},{:d}".format(p.x, p.y))

		def obj_chest_small(p, creationCode):
			entityExport.append("EntityType::Chest,{:d},{:d}".format(p.x, p.y))

		def obj_boulder(p, creationCode):
		
			if creationCode is None:
				entityExport.append("EntityType::Boulder,{:d},{:d}".format(p.x, p.y))
			else:				
				
				# this being here is an assumption of that only boulders will be statues. i hope that holds.
				
				statueMap = {
					"b_form = 8": lambda p : "EntityType::AddStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 7": lambda p : "EntityType::MonStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 6": lambda p : "EntityType::EusStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 5": lambda p : "EntityType::GorStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 4": lambda p : "EntityType::CifStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 3": lambda p : "EntityType::TanStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 2": lambda p : "EntityType::BeeStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 1": lambda p : "EntityType::LevStatue,{:d},{:d}".format(p.x, p.y),
				}
				
				entityExport.append(statueMap[creationCode](p))
				
		def obj_enemy_cg(p, creationCode):
			# i should be checking creationcode here, but i,,,, dont want to!
			entityExport.append("EntityType::Bull,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cl(p, creationCode):
			entityExport.append("EntityType::Leech,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cc(p, creationCode):
			entityExport.append("EntityType::Maggot,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_ch(p, creationCode):
			entityExport.append("EntityType::Eye,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cs(p, creationCode):
			entityExport.append("EntityType::Chester,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cl(p, creationCode):
			entityExport.append("EntityType::Leech,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cm(p, creationCode):
			 entityExport.append("EntityType::Mimic,{:d},{:d}".format(p.x, p.y))
			 
		def obj_enemy_co(p, creationCode):
			entityExport.append("EntityType::Diamond,{:d},{:d}".format(p.x, p.y))
	
	
	
	
		def obj_collision(p, creationCode):
			# for now, donothing, but i could maybe,,, modify this to put a black tile in collision/details that has collision?
			pass
			
		def obj_dustemiter_unknown(p, creationCode):
			pass
			
		def obj_enter_the_secret(p, creationCode):
			ObjectFunctions.obj_spawnpoint(p, creationCode)
		
		def obj_na_secret_exit(p, creationCode):
			pass
			
		def obj_npc_friend(p, creationCode):
			pass
			
		def obj_mural(p, creationCode):
			pass
			
		def obj_enemy_nm(p, creationCode):
			pass
			
		def obj_secret_exit(p, creationCode):
			pass
			
		def obj_solarsystem(p, creationCode):
			pass
		
		def obj_rest(p, creationCode):
			pass
			
		def obj_dustemit(p, creationCode):
			pass
			
		def obj_fakewall(p, creationCode):
			pass
			
		def obj_enemy_invisiblemon(p, creationCode):
			pass

		def obj_npc_mon(p, creationCode):
			pass

		def obj_npc_riddle(p, creationCode):
			pass

		def obj_npc_ykko(p, creationCode):
			pass

		def obj_demonlords_statue(p, creationCode):
			pass

		def obj_enemy_nm_b(p, creationCode):
			pass

		def obj_gray_d(p, creationCode):
			pass

		def obj_sealchest(p, creationCode):
			pass

		def obj_npc_failure_001(p, creationCode):
			pass

		def obj_npc_nun(p, creationCode):
			pass

		def obj_npc_tail(p, creationCode):
			pass

		def obj_npc_failure_002(p, creationCode):
			pass

		def obj_npc_bride(p, creationCode):
			pass

		def obj_npc_famished(p, creationCode):
			pass

		def obj_npc_failure_004(p, creationCode):
			pass

		def obj_npc_nomad(p, creationCode):
			pass

		def obj_npc_unknown(p, creationCode):
			pass

		def obj_npc_nomad2(p, creationCode):
			pass

		def obj_npc_failure_005(p, creationCode):
			pass

		def obj_npc_spawner(p, creationCode):
			pass

		def obj_npc_gor(p, creationCode):
			pass

		def obj_npc_failure_006(p, creationCode):
			pass

		def obj_npc_failure_008(p, creationCode):
			pass

		def obj_npc_oldman(p, creationCode):
			pass

		def obj_randomfloorlayout(p, creationCode):
			pass

		def obj_enter_the_danger(p, creationCode):
			pass

		def obj_spawn_intro(p, creationCode):
			ObjectFunctions.obj_spawnpoint(p, creationCode)

		def obj_intermission(p, creationCode):
			pass

		def obj_change_music(p, creationCode):
			pass

		def obj_cc_menu(p, creationCode):
			pass

		def obj_cif_epilogue2(p, creationCode):
			pass

		def obj_gateofdis(p, creationCode):
			pass

		def obj_ex_aftermath(p, creationCode):
			pass

		def obj_riddle_000(p, creationCode):
			pass

		def obj_tot_mural(p, creationCode):
			pass

		def obj_riddle_002(p, creationCode):
			pass

		def obj_riddle_003(p, creationCode):
			pass

		def obj_riddle_004(p, creationCode):
			pass

		def obj_riddle_005(p, creationCode):
			pass

		def obj_riddle_006(p, creationCode):
			pass

		def obj_riddle_001(p, creationCode):
			pass

		def obj_elevator_activate(p, creationCode):
			pass

		def obj_stainedglass_eclipse(p, creationCode):
			pass

		def obj_gray_finalmeeting(p, creationCode):
			pass

		def obj_zero_d(p, creationCode):
			pass

		def obj_lillie_d(p, creationCode):
			pass

		def obj_results(p, creationCode):
			pass

		def obj_confinement(p, creationCode):
			pass

		def obj_enemy_ct(p, creationCode):
			pass

		def obj_soundtest(p, creationCode):
			pass

		def obj_staffroll_b(p, creationCode):
			pass

		def obj_npc_dr_ab___on(p, creationCode):
			pass
		
		def obj_baal_d(p, creationCode):
			pass

		def obj_npc_tail_tail(p, creationCode):
			pass

		def obj_npc_failure_003(p, creationCode):
			pass

		def obj_npc_randobandit(p, creationCode):
			pass

		def obj_baal_m_d(p, creationCode):
			pass

		def obj_npc_failure_007(p, creationCode):
			pass

		def obj_chest(p, creationCode):
			pass

		def obj_baal_epilogue2(p, creationCode):
			pass

		def obj_gateofdis_roots(p, creationCode):
			pass

		def obj_sera_d(p, creationCode):
			pass

		def obj_intro2_bg(p, creationCode):
			pass

		def obj_lillie_finalmeeting(p, creationCode):
			pass

		def obj_freya_d(p, creationCode):
			pass

		def obj_a_corpse(p, creationCode):
			pass

		def obj_cifer_d(p, creationCode):
			pass

		def obj_tail_kill(p, creationCode):
			pass

		def obj_statue_abaddon(p, creationCode):
			pass

		def obj_gateofdis_bg(p, creationCode):
			pass

		def obj_cutscene_finalwaltz(p, creationCode):
			pass

		def obj_fm_cemetery(p, creationCode):
			pass

		def obj_npc_tail_floor(p, creationCode):
			pass

		def obj_cutscene_epilogue_part3(p, creationCode):
			pass

		def obj_cutscene_grayend(p, creationCode):
			pass

		def obj_cutscene_finalmeeting(p, creationCode):
			pass

		def obj_cutscene_lillie_end(p, creationCode):
			pass

		def obj_npc_tail_lowerfloor(p, creationCode):
			pass

		def obj_cifandbee_bg(p, creationCode):
			pass

		def obj_stinklines(p, creationCode):
			pass

		def obj_spawnpoint__dream(p, creationCode):
			pass

		def obj_npc_nolla(p, creationCode):
			pass

		def obj_floor_lev(p, creationCode):
			pass

		def obj_npc_levextra(p, creationCode):
			pass

		def obj_gor_cube(p, creationCode):
			pass

		def obj_horse(p, creationCode):
			pass

		def obj_johann_dreamIX(p, creationCode):
			pass

		def obj_gray_dreamVII(p, creationCode):
			pass

		def obj_gray_dreamVI(p, creationCode):
			pass

		def obj_prin_dreamV(p, creationCode):
			pass

		def obj_seal(p, creationCode):
			pass

		def obj_floor_hpn(p, creationCode):
			pass

		def obj_music_controller_v2(p, creationCode):
			pass

		def obj_gray_dreamIII(p, creationCode):
			pass

		def obj_bigwindow(p, creationCode):
			pass

		def obj_gray_dreamXI(p, creationCode):
			pass

		def obj_gray_dreamIX_b(p, creationCode):
			pass

		def obj_torch(p, creationCode):
			pass

		def obj_prin_c(p, creationCode):
			pass

		def obj_gray_dreamVIII(p, creationCode):
			pass

		def obj_prin_dreamVIII(p, creationCode):
			pass

		def obj_johann_dreamVIII(p, creationCode):
			pass

		def obj_ee_eclipse(p, creationCode):
			pass

		def obj_enemy_floormaster(p, creationCode):
			pass

		def obj_og_jewel(p, creationCode):
			pass

		def obj_dr_un(p, creationCode):
			pass

		def obj_player_a(p, creationCode):
			pass

		def obj_player_child(p, creationCode):
			pass

		def obj_enemy_5(p, creationCode):
			pass

		def obj_tan_intro(p, creationCode):
			pass

		def obj_mas_stuff(p, creationCode):
			pass

		def obj_baal_battleintro(p, creationCode):
			pass

		def obj_mon_battleintro(p, creationCode):
			pass

		def obj_enemy_cifhandl(p, creationCode):
			pass

		def obj_wagon(p, creationCode):
			pass

		def obj_npc_ran001(p, creationCode):
			pass

		def obj_prin_dreamVI(p, creationCode):
			pass

		def obj_gray_dreamV(p, creationCode):
			pass

		def obj_prin_dreamIII(p, creationCode):
			pass

		def obj_gray_dreamIV(p, creationCode):
			pass

		def obj_prison_bucket(p, creationCode):
			pass

		def obj_torch_fallen(p, creationCode):
			pass

		def obj_prin_dreamIX(p, creationCode):
			pass

		def obj_baal_dreamIX(p, creationCode):
			pass

		def obj_curtains(p, creationCode):
			pass

		def obj_gray_c(p, creationCode):
			pass

		def obj_enter_door(p, creationCode):
			pass

		def obj_eclipse_begin(p, creationCode):
			pass

		def obj_jguard_dreamVIII_a(p, creationCode):
			pass

		def obj_prin_dreamVII(p, creationCode):
			pass

		def obj_npc_ran002_dreamV(p, creationCode):
			pass

		def obj_npc_ran003_dreamV(p, creationCode):
			pass

		def obj_npc_ran001_dreamV(p, creationCode):
			pass

		def obj_ee_enemies(p, creationCode):
			pass

		def obj_ee_bg_future(p, creationCode):
			pass

		def obj_ex_levbg_001(p, creationCode):
			pass

		def obj_giant(p, creationCode):
			pass

		def obj_dummyfloor(p, creationCode):
			pass

		def obj_og_onion(p, creationCode):
			pass

		def obj_stairs_001(p, creationCode):
			pass

		def obj_lock(p, creationCode):
			pass

		def obj_tan_intro_body(p, creationCode):
			pass

		def obj_riddle_007(p, creationCode):
			pass

		def obj_cif_monbattleintro(p, creationCode):
			pass

		def obj_enemy_cifhandr(p, creationCode):
			pass
		
		def obj_pr_cupboard(p, creationCode):
			pass

		def obj_cifer_dreamIX(p, creationCode):
			pass

		def obj_exit_dream(p, creationCode):
			pass

		def obj_npc_ran002(p, creationCode):
			pass

		def obj_wagon_inside(p, creationCode):
			pass

		def obj_rando_dreamIII(p, creationCode):
			pass

		def obj_bandit_dreamIII(p, creationCode):
			pass

		def obj_johann_dreamIV(p, creationCode):
			pass

		def obj_jguard_dreamVIII_b(p, creationCode):
			pass

		def obj_johann_dreamV(p, creationCode):
			pass

		def obj_stairs_002(p, creationCode):
			pass

		def obj_enemy_invmon_prelude(p, creationCode):
			pass

		def obj_npc_ran003(p, creationCode):
			pass

		def obj_henchman_ded_dreamIII(p, creationCode):
			pass

		def obj_judgement_crater(p, creationCode):
			pass

		def obj_zero_dreamIX(p, creationCode):
			pass

		def obj_open_door(p, creationCode):
			pass

		def obj_jguard_dreamVIII(p, creationCode):
			pass

		def obj_jguard_dreamV(p, creationCode):
			pass

		def obj_stairs_002b(p, creationCode):
			pass

		def obj_mural_dream(p, creationCode):
			pass

		def obj_stairs_003(p, creationCode):
			pass

		def obj_floor_un(p, creationCode):
			pass
	
		def obj_console(p, creationCode):
			pass

		def obj_un_bgdecour_001(p, creationCode):
			pass

		def obj_memorywindow(p, creationCode):
			pass

		def obj_un_bgdecour_002(p, creationCode):
			pass





	
	allData = []
	
	for label in ["Instances", "Floor_INS", "Floor", "Pit"]:
		if label in layerData:
			allData += layerData[label]["layer_data"]["instances"]
	
			
	
	for inst in allData:
	
		#x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16
		
		p = Pos(inst["x"], inst["y"])

		objectDef = inst["object_definition"]
		creationCodeFilename = inst["creation_code"]
		
		
		
		if p.x < 0 or p.x >= 14 or p.y < 0 or p.y >= 9:
			continue
		
	
		
	
		if hasattr(ObjectFunctions, objectDef):
		
			creationCode = readCreationCode(creationCodeFilename)
			# avoid needing this check every time 
			if creationCode == "destroy":
				continue
		
			getattr(ObjectFunctions, objectDef)(p, creationCode)
		else:
		
			print("we dont have a definition for {:s} in convert entities".format(objectDef))
		
			if objectDef not in failures:
				failures[objectDef] = []
		
			failures[objectDef].append(layerData["roomName"])
			
			return None
	
	# MAKE SURE THE PLAYER HAS A SPAWNPOINT HERE
	if len(entityExport) == 0:
		print(RED + "there were no entities, not even a player??" + RESET)
		return None
		
	
	return [floorExport, entityExport, effectExport]
	
def convertRoom(data, outputFile):

	layerData = {}
	
	for l in data["layers"]:
		layerData[l["layer_name"]] = l
	
	layerData["roomName"] = data["name"]
	
	formatArray = lambda arr : "".join([ "{:d},".format(elem) for elem in np.array(arr).flatten() ])
	
	formatFullArray = lambda name, arr : "constexpr static inline u8 {:s}[] = {{{:s}}};".format(name, formatArray(arr))
	
	temp = convertCollisionAndDetails(layerData)
	
	if temp is None:
		print("collision/details convert failed!")
		return None
		
	collision, details = temp
		
	"""
	floorExport = convertFloor(layerData)
	
	if floorExport is None:
		return None
	
	instanceExport = convertEntities(layerData)
	
	if instanceExport is None:
		return None
	"""
	
	objectsExport = convertObjects(layerData)
	if objectsExport is None:
		return None
		
	floorExport, instanceExport, statueExport = objectsExport
	
	output = []
	
	output.append("namespace {:s} {{".format(data["name"]))
	
	if len(details["data"]) == 0:
		details["data"] = [ [0 for i in range(9)] for j in range(14)]
	
	output.append(formatFullArray("collision", collision["data"]))
	output.append(formatFullArray("details", details["data"]))

	output.append("constexpr static inline TileType floor[] = {")
	output.append("".join([ "TileType::{:s},".format(instance) for instance in np.transpose(floorExport).flatten() ]))
	output.append("};")
	
	output.append("constexpr static inline EntityHolder entities[] = {")	
	output.append("".join([ "{{{:s}}},".format(instance) for instance in instanceExport ]))
	output.append("};")
	
	output.append("constexpr static inline int entityCount = {:d};".format(len(instanceExport)))
	
	#output.append("constexpr static inline EntityHolder effects[] = {")
	#output.append("};")	
	
	output.append("constexpr static inline int effectCount = {:d};".format(len(instanceExport)))
	
	output.append("};")
	
	[ outputFile.write(line + "\n") for line in output ]

	# TODO, WE DONT EXPORT THE TILESET HERE(of what the room tiles should be)
	
	return True

def convertAllRooms(inputPath):

	global creationCodeData
	with open("creationCodeData.json") as f:
		creationCodeData = json.load(f)
	

	f = open("AllRooms.h", "w")
	
	f.write("//Did you know every time you sigh, a little bit of happiness escapes?\n")
	
	
	jsonFiles = [f for f in os.listdir(inputPath) if f.lower().endswith('.json')]
	
	
	#removeRooms = ["rm_misc_0002"]
	
	#jsonFiles = [ f for f in jsonFiles if f.rsplit(".", 1)[0] not in removeRooms ]

	#removeStrings = ["secret", "test", "misc", "trailer"]
	#removeStrings = ["stg", "house", "secret", "test", "misc", "trailer", "dream", "memories", "bee", "lev", "_ee_"]
	removeStrings = []
	
	for removeStr in removeStrings:
		# i could, and should one line this
		jsonFiles = [ f for f in jsonFiles if removeStr not in f ]
	
	
	#jsonFiles = ["rm_0005.json"]
	#jsonFiles = ["rm_0027.json"]
	#jsonFiles = ["rm_0008.json"]
	#jsonFiles = ["rm_0018.json"]
	#jsonFiles = ["rm_2intro.json"]
	
	successRooms = 0 
	totalRooms = len(jsonFiles)
	
	successRoomsList = []
	
	for file in jsonFiles:
		with open(os.path.join(inputPath, file)) as jsonFilePointer:
			data = json.load(jsonFilePointer)
			
			print("doing room {:s}".format(data["name"]))
			
			res = convertRoom(data, f)
			if res is not None:
				successRooms += 1
				successRoomsList.append(file)
			else:
				print(RED + "failure" + RESET)
			
	
	#print(successRoomsList)
	
	writeFooter(f, [ elem.rsplit(".json", 1)[0] for elem in successRoomsList ])
	
	f.close()
	
	print("")
	
	print("we converted {:6.2f}% rooms({:d}/{:d}), i hope thats acceptable.".format(100*successRooms/totalRooms, successRooms, totalRooms))
	
	print("")
	
	print("primary failure sources:")
	
	#print(json.dumps(failures, indent=4))
	
	for _, v in failures.items():
		if type(v) != list:
			print("wtf")
			exit(1)
	
	tempSorted = sorted([ [k, len(v), v] for k, v in failures.items() ], key = lambda elem : len(elem[2]), reverse=True)
	
	highlightStrings = ["mon", "_e_"]
	
	for t in tempSorted:
		temp = str(t)
		
		col = WHITE
		
		for s in highlightStrings:
			if s in temp:
				col = RED
				break
				
		print(col + temp + RESET)
		
	
	tempFile = open("temp.txt", "w")
	for t in tempSorted:
		tempFile.write("def {:s}(p, creationCode):\n".format(t[0]))
		tempFile.write("\tpass\n")
		tempFile.write("\n")
	tempFile.close()
	
	
	with open('tempCreationCodeData.json', 'w') as f:
		json.dump(newCreationCodesData, f, indent=4)
	
	
	pass

def main():

	os.chdir(os.path.dirname(__file__))

	# run ExportAllRooms.csx(in this dir), name it roomExport, put it in this dir
	# run ExportAllCode, move to this folder
	
	convertAllRooms("../ExportData/Room_Export/")
	
	shutil.copy("AllRooms.h", "../../code/src")
	
	pass
	
if __name__ == "__main__":
	main()

