import os
import sys
import subprocess
import time

def run_flow(rate):
	NIC1ip = '192.168.1.44' # server IP
	
	NIC1 = 'ens33' # sending Interface 
	target_NIC = 'ens33' # modifying interface 
	
	outputVideoName = 'video' + str(rate) + '.mp4'
	outputStatsName = 'psnr' + str(rate) + '.txt'
	# cmd_start_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' cat /home/incandescentxxc/udpVideo/haha.txt > haha' # locate to the file storing videos in remote server
	cmd_exec_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -f h264 -i udp://192.168.1.44:8000?timeout=15000000 '+outputVideoName
	cmd_cal_remote_server = 'ssh incandescentxxc@' + NIC1ip + ' ffmpeg -i Trial1_55M.mp4 -i '+ outputVideoName + ' -lavfi psnr="stats_file=psnr.log" -f null - > ' + outputStatsName
	# flow starts here
	if(rate != 0):
		os.system("sudo tc qdisc add dev " + target_NIC + " root netem corrupt " + rate)
	# os.system("cd /home/xuxiaochuan/CodesHQ/UDPVideo")
	# remote_server = subprocess.Popen(cmd_start_remote_server, shell=True, stdout=subprocess.PIPE)
	# remote_server.wait()
	# time.sleep(5)
	remote_server_exec = subprocess.Popen(cmd_exec_remote_server, shell=True, stdout=subprocess.PIPE)
	time.sleep(5) # wait for the server to start listening
	os.system("ffmpeg -re -i Trial1_55M.mp4 -f h264 udp://192.168.1.44:8000")
	# remote_server_exec.wait()
	remote_server_cal = subprocess.Popen(cmd_cal_remote_server, shell=True, stdout=subprocess.PIPE)
	remote_server_cal.wait()
	if(rate != 0):
		os.system("sudo tc qdisc del dev ens33 root") # restore origin config
	

def run_exp(rate):
	run_flow(rate)
	# TODO write to the file
	

	

if __name__ == "__main__":
	# for bit_error in [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]:
	# 	run_exp(bit_error)
	run_exp(0)

