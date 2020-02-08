import os
import sys
import subprocess
import time
import csv
def run_flow(rate, mode):
	
	NIC1ip = '192.168.1.44' # server IP
	LOClip = '192.168.1.199'
	NIC1 = 'ens33' # sending Interface 
	target_NIC = 'ens33' # modifying interface 

	# cmd_start_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' cat /home/incandescentxxc/udpVideo/haha.txt > haha' # locate to the file storing videos in remote server
	if(mode == 0):# udp
		outputVideoName = 'udpvideo' + str(rate) + '.mp4'
		outputStatsName = 'udppsnr' + str(rate) + '.log'
		cmd_exec_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -f h264 -i udp://192.168.1.44:8000?timeout=15000000 '+outputVideoName
		cmd_cal_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -i Trial1_55M.mp4 -i '+ outputVideoName + ' -lavfi psnr="stats_file=psnr.log" -f null -'
		cmd_tran_remote_server = 'scp incandescentxxc@' + NIC1ip + ':psnr.log '+outputStatsName
	else: # udplite
		outputVideoName = 'udplitevideo' + str(rate) + '.mp4'
		outputStatsName = 'udplitepsnr' + str(rate) + '.log'
		cmd_exec_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -f h264 -i udplite://192.168.1.44:8000?timeout=15000000 '+outputVideoName
		cmd_cal_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -i Trial1_55M.mp4 -i '+ outputVideoName + ' -lavfi psnr="stats_file=psnr.log" -f null -'
		cmd_tran_remote_server = 'scp incandescentxxc@' + NIC1ip + ':psnr.log '+outputStatsName
	
	# flow starts here
	if(rate != 0):
		os.system("sudo tc qdisc add dev " + target_NIC + " root netem corrupt " + str(rate))
	# os.system("cd /home/xuxiaochuan/CodesHQ/UDPVideo")
	# remote_server = subprocess.Popen(cmd_start_remote_server, shell=True, stdout=subprocess.PIPE)
	# remote_server.wait()
	# time.sleep(5)
	remote_server_exec = subprocess.Popen(cmd_exec_remote_server, shell=True, stdout=subprocess.PIPE)
	time.sleep(6) # wait for the server to start listening
	if(mode == 0): # udp
		os.system("ffmpeg -re -i Trial1_55M.mp4 -f h264 udp://192.168.1.44:8000")
	else: #udplite
		os.system("ffmpeg -re -i Trial1_55M.mp4 -f h264 udplite://192.168.1.44:8000")
	# remote_server_exec.wait()
	remote_server_cal = subprocess.Popen(cmd_cal_remote_server, shell=True, stdout=subprocess.PIPE)
	remote_server_cal.wait()
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
	with open(outputStatsName, 'r') as infile:
		counter = 0
		psnr_ys = 0
		for line in infile:
			lineContents = line.split(" ")
			psnr = lineContents[6].split(":")
			psnr_y = float(psnr[1])
			psnr_ys += psnr_y
			counter += 1
	
	psnr_avg = psnr_ys / counter
	print("Counter is: " + str(counter) +"\n")
	print("psnr_y_avg:" + str(psnr_avg) +"\n")
	return psnr_avg

	


	

	

if __name__ == "__main__":
	udpDict = {}
	udpLiteDict = {}
	for bit_error in [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]:
		udpDict[str(bit_error)] = run_exp(bit_error, 0)
		udpLiteDict[str(bit_error)] = run_exp(bit_error, 1)
	print(udpDict)
	print("\n")
	print("\n")
	print(udpLiteDict)
	with open('udp.csv','w',newline="") as csv_file:
		writer = csv.writer(csv_file)
		for key, value in udpDict.items():
			writer.writerow([key, value])
	with open('udplite.csv','w',newline="") as csv_file:
		writer = csv.writer(csv_file)
		for key, value in udpLiteDict.items():
			writer.writerow([key, value])



