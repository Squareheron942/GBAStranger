
from pydub import AudioSegment
import os, sys
import shutil
import subprocess

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "EasyPoolProcessing"))

from poolQueue import PoolQueue
from multiprocessing import Queue, Pool, cpu_count, Event
import queue
import time 
from threading import Thread

from pytube import YouTube

outputPath = "./formattedOutput/"

def convertAllMusic():

	# msc_013.ogg	
	
	inputPath = "./Exported_Sounds/audiogroup_music"
	
	file = "msc_013.ogg"
	
	inputFilePath = os.path.join(inputPath, file)
	
	song = AudioSegment.from_ogg(inputFilePath)
	
	song = song.set_channels(1)
	
	#song = song[:30*1000]
	
	#song = song.compress(2)


	outputFilePath = os.path.join(outputPath, file.rsplit(".", 1)[0] + ".wav")
	song.export(outputFilePath, format="wav", parameters=["-ar","4000"])
	
	pass

def convertAllSounds():

	inputPath = "../ExportData/Exported_Sounds/audiogroup_soundeffects/"

	wavFiles = [f for f in os.listdir(inputPath) if f.lower().endswith('.wav')]
	
	successCount = 0
	failCount = 0
	for i, wav in enumerate(wavFiles):
	
		print("converting soundfile {:50s}, {:5d} out of {:5d}".format(wav, i, len(wavFiles)))
	
		song = AudioSegment.from_wav(os.path.join(inputPath, wav))
		
		if song.duration_seconds > 5:
			print("{:50s} is to long, skipping".format(wav))
			
			failCount += 1
			continue
		
		song = song.set_channels(1)
	
		outputFilePath = os.path.join(outputPath, wav.rsplit(".", 1)[0] + ".wav")
		#song.export(outputFilePath, format="wav", parameters=["-ar","22050"])
		song.export(outputFilePath, format="wav", parameters=["-ar","44100"])

		
		successCount += 1


	
	totalCount = successCount + failCount
	print("converted {:5.2f}% ({:d}/{:d}) of sound effects, hope thats good enough".format(100 * successCount / totalCount, successCount, totalCount))
		
	pass
	
def convertMisc():
	
	outputPath = "./formattedOutput/"
	
	# hehe 
	# https://www.youtube.com/watch?v=tD-2EZCScnk&ab_channel=NudeDrummer
	
	link = "https://www.youtube.com/watch?v=tD-2EZCScnk"
	
	
	fileNames = [ f for f in os.listdir("./") if f.endswith('.mp4')]
	
	if len(fileNames) == 0:
		yt=YouTube(link)
		t=yt.streams.filter(only_audio=True)
		t[0].download("./")
		fileNames = [ f for f in os.listdir("./") if f.endswith('.mp4')]
	
	fileName = fileNames[0]
	
	subprocess.run([
    'ffmpeg',
    "-y", '-i', fileName,
    fileName.rsplit(".", 1)[0] + ".mp3"
	])
	
	song = AudioSegment.from_mp3(fileName.rsplit(".", 1)[0] + ".mp3")
	
	song = song.set_channels(1)

	outputFilePath = os.path.join(outputPath, fileName.rsplit(".", 1)[0].replace(" ", "_").lower() + ".wav")
	song.export(outputFilePath, format="wav", parameters=["-ar","44100"])

	os.remove(fileName.rsplit(".", 1)[0] + ".mp3")
	
	
	pass

def main():

	os.chdir(os.path.dirname(__file__))

	if os.path.exists("./formattedOutput"):
		shutil.rmtree("./formattedOutput")
		
	os.mkdir("./formattedOutput/")

	# makesure that audiogroup1.dat and audiogroup2.dat are next to data.win
	# run exportallsounds 
	# move that folder here

	#convertAllMusic()
	convertAllSounds()
	convertMisc()
	
	[ os.remove(os.path.join("../../code/audio/", f)) for f in os.listdir("../../code/audio/") if f.endswith(".wav") ]
	copyFunc = lambda copyFrom : [ shutil.copy(os.path.join(copyFrom, f), os.path.join("../../code/audio/", f)) for f in os.listdir(copyFrom) if f.endswith(".wav") ]
	copyFunc("./formattedOutput/")
	
	pass
	
if __name__ == "__main__":
	main()
	
