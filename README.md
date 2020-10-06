# A few pts for references

## PTP (Precision Time Protocol)

Checkout NIC support:
        sudo ethtool -T ens33 
Slave clock (Client):
        sudo ptp4l -i eno1 -m -S -s
Master clock (Server):
        sudo ptp4l -i ens33 -m -S

useful links: 
- https://blog.csdn.net/weixin_33888907/article/details/93568950
- https://blog.csdn.net/BUPTOctopus/article/details/86246335

