import tkinter as tk
import os
window = tk.Tk()
window.title('UDP Jammer')

window.geometry("500x300")

NIC1 = ' ens33 '

l1 = tk.Label(window, bg='green', fg='white', width=20, text='Delay=0ms')
l1.pack()

def print_selection(v):
    l1.config(text='Delay=' + v + "ms")
    cmd0 = "sudo tc qdisc del root dev" + NIC1
    cmd1 = "sudo tc qdisc add dev" + NIC1 + "root netem delay " + v + "ms"
    cmd2 = "sudo tc qdisc change dev" + NIC1 + "root netem delay " + v + "ms"
    os.system(cmd0)
    os.system(cmd1)
    os.system(cmd2)

s1 = tk.Scale(window, label='Physical Delay', from_=0,to=500, orient=tk.HORIZONTAL,length=1000,
             showvalue=0,tickinterval=50,resolution=0.1,command=print_selection)
s1.pack()


l2 = tk.Label(window, bg='green', fg='white', width=20, text='Loss=0%')
l2.pack()

def print_selection2(v):
    l2.config(text='Loss=' + v + "%")
    cmd0 = "sudo tc qdisc del root dev" + NIC1
    cmd1 = "sudo tc qdisc add dev" + NIC1 + "root netem loss " + v + "%"
    cmd2 = "sudo tc qdisc change dev" + NIC1 + "root netem loss " + v + "%"
    os.system(cmd0)
    os.system(cmd1)
    os.system(cmd2)

s2 = tk.Scale(window, label='Physical Loss', from_=0.001,to=2, orient=tk.HORIZONTAL,length=1000,
             showvalue=0,tickinterval=0.1,resolution=0.1,command=print_selection2)
s2.pack()


window.mainloop()
