# RainPotsRunner

For RNBO 1.2.3

## Setting up

### 1. Configure Serial Port
sudo raspi-config : Serialport

### 2. Install Depencencies: 
`sudo apt-get install libjsoncpp-dev`   
`sudo apt-get install curl`  
`sudo apt-get install  libcurl4-openssl-dev`    

### 3. Clone Repo and Build
`cd ~/Documents`  
`git clone git@github.com:janmech/RainPotsRunner.git`
`cd RainPotsRunner`
`make`
`make install`


# Run at Startup

## Opition 1 - via .bashrc
`cd ~`  
`namo .bashrc`  

add at the end of the file:


```
if [[ ! $(pgrep rainpots) ]]; then
    rainpots 2> /home/pi/Documents/rainpots/log/rainpots-error.log 1> /home/pi/Documents/rainpots/log/rainpots.log
fi
```
Save and close

## Option 2 - via Service

`cd /lib/systemd/system`  
`sudo nano rainpots.service` :    

```
[Unit]
  Description=RainPots Service
  After=multi-user.target
  StartLimitIntervalSec=500
  StartLimitBurst=5
  StartLimitInterval=0
  Wants=rnbooscquery.service
  After=rnbooscquery.service
  PartOf=rnbooscquery.service

[Service]
  Type=idle
  ExecStart=/usr/bin/rainpots
  KillSignal=SIGINT
  User=pi
  Group=audio
  Restart=on-failure
  RestartSec=5s
  ExecStartPre=/bin/sleep 10
  StandardOutput=append:/home/pi/Documents/rainpots/log/rainpots.log
  StandardError=append:/home/pi/Documents/rainpots/log/rainpots-error.log

[Install]
  WantedBy=multi-user.target
  Alias=rainpots
```
Save and exit  

`sudo systemctl daemon-reload`  
`sudo systemctl enable rainpots`  

`sudo reboot` 



NOTE: 
`ExecStartPre=/bin/sleep 10` is a workaround so that the rainpots service doesn't interfere with the RNBORunner. If the system/RNBO doesn't boot up properly try to increase the wait time (in seconds)

If it doesn't work disable service: 
`sudo servcice rainpots stop`
`sudo systemctl enable rainpots` 

and try:  

 **Opition 1 - via .bashrc**

