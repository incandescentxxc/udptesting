import os
import sys
import subprocess
import time
import csv
def run_flow(rate, mode):
	
	NIC1ip = '192.168.1.45' # server IP
	LOClip = '192.168.1.199'
	NIC1 = 'ens33' # sending Interface 
	target_NIC = 'ens33' # modifying interface 

	# cmd_start_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' cat /home/incandescentxxc/udpVideo/haha.txt > haha' # locate to the file storing videos in remote server
	if(mode == 0):# udp
		outputVideoName = 'udpvideo' + str(rate) + '.mp4'
		outputStatsName = 'udppsnr' + str(rate) + '.log'
		cmd_exec_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -f h264 -i udp://192.168.1.45:8000?timeout=15000000 '+outputVideoName

	else: # udplite
		outputVideoName = 'udplitevideo' + str(rate) + '.mp4'
		outputStatsName = 'udplitepsnr' + str(rate) + '.log'
		cmd_exec_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -f h264 -i udplite://192.168.1.45:8000?timeout=15000000 '+outputVideoName

	cmd_cal_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -i video_480.mp4 -i '+ outputVideoName + ' -lavfi psnr="stats_file=psnr.log" -f null -'
	cmd_getFPS_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 ' + outputVideoName + ' > numberFrame.txt'
	cmd_tran_remote_server = 'scp incandescentxxc@' + NIC1ip + ':psnr.log '+outputStatsName
	
	# flow starts here
	if(rate != 0): # adjust the corruption rate
		os.system("sudo tc qdisc add dev " + target_NIC + " root netem corrupt " + str(rate))
	remote_server_exec = subprocess.Popen(cmd_exec_remote_server, shell=True, stdout=subprocess.PIPE)
	time.sleep(6) # wait for the server to start listening
	if(mode == 0): # udp
		os.system("ffmpeg -re -i video_480.mp4 -f h264 udp://192.168.1.45:8000")
	else: #udplite
		os.system("ffmpeg -re -i video_480.mp4 -f h264 udplite://192.168.1.45:8000")
	remote_server_exec.wait()
	remote_server_cal = subprocess.Popen(cmd_cal_remote_server, shell=True, stdout=subprocess.PIPE) # calculate psnr
	remote_server_cal.wait()
	remote_server_fps = subprocess.Popen(cmd_getFPS_remote_server, shell=True, stdout=subprocess.PIPE) # get the number of frames
	remote_server_fps.wait()
	remote_server_tra = subprocess.Popen(cmd_tran_remote_server, shell=True, stdout=subprocess.PIPE)
	remote_server_tra.wait()
	if(rate != 0):
		os.system("sudo tc qdisc del dev ens33 root") # restore origin config
	

def run_exp(rate, mode):
	run_flow(rate, mode)
	# TODO write to the file
	if(mode ==0):
		outputStatsName = 'udppsnr' + str(rate) + '.log'
	else:
		outputStatsName = 'udplitepsnr' + str(rate) + '.log'

	with open("numberFrame.txt", "r") as paraFile:
		num = int(paraFile.readline())
	with open(outputStatsName, 'r') as infile:
		psnr_ys = 0
		counter = 0
		for i in range(0, num):
			line = infile.readline()
			if(line == ''): # may be occurred when the frame number got exceeds the actual frame number
				print("Empty line!")
				return psnr_ys/counter
			lineContents = line.split(" ")
			psnr = lineContents[6].split(":")
			psnr_y = float(psnr[1])
			psnr_ys += psnr_y
			counter += 1
	
	psnr_avg = psnr_ys / num
	print("Counter is: " + str(counter) +"\n")
	print("psnr_y_avg:" + str(psnr_avg) +"\n")
	return psnr_avg

	


	

	

if __name__ == "__main__":
	udpDict = {}
	udpLiteDict = {}
	for bit_error in [1]:
		udpDict[str(bit_error)] = run_exp(bit_error, 0)
		udpLiteDict[str(bit_error)] = run_exp(bit_error, 1)
	
	print("UDP: \n")
	print(udpDict)
	print("UDPLite: \n")
	print(udpLiteDict)
	with open('udp.csv','w',newline="") as csv_file:
		writer = csv.writer(csv_file)
		for key, value in udpDict.items():
			writer.writerow([key, value])
	with open('udplite.csv','w',newline="") as csv_file:
		writer = csv.writer(csv_file)
		for key, value in udpLiteDict.items():
			writer.writerow([key, value])



