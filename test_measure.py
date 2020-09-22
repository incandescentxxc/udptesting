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
    NIC1ip = '172.16.33.29'
    LOCip = "172.20.26.31"
    NIC1 = 'ens33'
    LOCNIC = 'ens33'

    if(mode == 0):#udp
        cmd_remote_server_start = 'ssh xxc@' + NIC1ip + ' cd ./udptesting &&./udpserver 10'
        cmd_local_addmask = 'sudo tc qdisc add dev ' + NIC1 + ' root netem loss ' + str(loss_rate)
        cmd_local_cancelmask = 'ssh xxc@' + NIC1ip + ' sudo tc qdisc del dev ' + NIC1+ ' root'
        cmd_local_cli_send = 'ssh xxc@' + NIC1ip + ' ./udpclient 11'

    #flow starts
    
    if(loss_rate != 0): # adjust the corruption rate
        os.system(cmd_local_addmask)
    thread_remote = myThread(cmd_remote_server_start)
    thread_remote.start()
    time.sleep(5)
    os.system(cmd_local_cli_send)
    if(loss_rate != 0):
        os.system(cmd_local_cancelmask)
    thread_remote.join()


if __name__ == "__main__":
    run_flow(0.1, 0, 0)





    
