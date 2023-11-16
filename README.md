# RainPotsRunner

## Setting up
### Clone Repo
cd Documents

### 1. Configure Serial Port
sudo raspi-config : Serialport

### 2. Install Depencencies: 
sudo apt-get install libjsoncpp-dev  
sudo apt-get install curl  
sudo apt-get install  libcurl4-openssl-dev  

###


# Run at Startup

## Opition 1 - .bashrc

if [[ ! $(pgrep rainpots) ]]; then
    rainpots 2> /home/pi/Documents/rainpots/log/rainpots-error.log 1> /home/pi/Documents/rainpots/log/rainpots.log
fi

## Option 2 - via Service




