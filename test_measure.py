import os
import sys
import threading
import time
import csv
class myThread (threading.Thread):
    def __init__(self,cmd):
        threading.Thread.__init__(self)
        self.cmd = cmd
    def run(self):
        print("start\n")
        os.system(self.cmd)

def run_flow(loss_rate, cor_rate, mode):
    NIC1ip = "192.168.1.133"
    LOCip = "192.168.1.115"
    NIC1 = 'ens38'
    LOCNIC = 'ens35'

    if(mode == 1): #udp
        cmd_remote_server_start = 'ssh xxc@' + NIC1ip + ' \"cd ./udptesting && ./udpserver 1 10\"'
        cmd_local_cli_send = './udpclient 1 10 10000'
    elif(mode == 2): #udpclient
        cmd_remote_server_start = 'ssh xxc@' + NIC1ip + ' \"cd ./udptesting && ./udpserver 2 10\"'
        cmd_local_cli_send = './udpclient 2 10 10000'

    cmd_local_addmask = 'sudo tc qdisc add dev ' + LOCNIC + ' root netem corrupt ' + str(loss_rate)
    cmd_local_cancelmask = 'sudo tc qdisc del dev ' + LOCNIC+ ' root'
    #flow starts
    
    if(loss_rate != 0): # adjust the corruption rate
        os.system(cmd_local_addmask)
    thread_remote = myThread(cmd_remote_server_start)
    thread_remote.start()
    time.sleep(1)
    os.system(cmd_local_cli_send)
    if(loss_rate != 0):
        os.system(cmd_local_cancelmask)
    thread_remote.join()

if __name__ == "__main__":
    for i in range(0, 11, 1):
        loss_rate = i/2
        time.sleep(0.5)
        run_flow(loss_rate, 0, 1)
        time.sleep(0.5)
        # run_flow(loss_rate, 0, 2)





    
