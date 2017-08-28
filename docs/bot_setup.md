# Setting up your bot for Shark #

### BOM ###
* 1/10th scale RC car with access to steering and esc. I used a Traxxas Slash and both were available after opening the waterproof lid of the controller.<https://www.amazon.com/gp/product/B019MS68B0>
* Raspberry Pi 3. Older versions are unlikely to have enough cpu power to work well. Other arm boards may work.<https://www.amazon.com/gp/product/B01CD5VC92>
* Fast sd card: <https://www.amazon.com/gp/product/B010Q57T02>
* Adafruit 16 channel PWM servo hat: <https://www.amazon.com/gp/product/B00SI1SPHS>
* USB Battery w two outputs ( if using usb camera ). I can recommend: <https://www.amazon.com/gp/product/B00FGN1OBU>
* some kind of camera:
[raspicam](https://www.amazon.com/gp/product/B00N1YJKFS)
or [PS3 Eye](http://a.co/08GHjk2) or [Point Grey](http://www.ebay.com/sch/i.html?_nkw=point%20grey)
* if selecting a point grey camera, look for one with usb 2.0 output
* jumper cables: <https://www.amazon.com/gp/product/B01FPMN432>
* some enclosure for the pi: <https://www.amazon.com/gp/product/B01GOSTL7Y>
* PS3 SixAxis controller: <http://a.co/b8OKh1V>
* an led. any will do. <http://a.co/inmaS3e>

Also supports raspberry pi camera:
* https://www.amazon.com/SainSmart-Fish-Eye-Camera-Raspberry-Arduino/dp/B00N1YJKFS

Or Point Grey cameras:
* https://www.ebay.com/sch/i.html?_from=R40&_trksid=p2380057.m570.l1313.TR0.TRC0.H0.Xpoint+grey+usb+camera.TRS0&_nkw=point+grey+usb+camera&_sacat=0

Optionally, This framework supports Jetson TX2
* Jetson TX2: https://www.amazon.com/NVIDIA-Jetson-TX2-Development-Kit/dp/B06XPFH939

And other 9685 based pwm boards:
* 9685 PWM Board: https://www.amazon.com/SunFounder-PCA9685-Channel-Arduino-Raspberry

Also RPLidar support:
* RpLidar A2 : https://www.amazon.com/RPLIDAR-A2-The-Thinest-LIDAR/dp/B01L1T32PI

### Setup PI ###
* install headless raspian for best performance

### Optional - Jetson TX_
* instead of a Pi3, use an Nvidia Jetson TX1 or TX2

### Setup a user ###
* You can use the default pi user, but better to create one that matches whatever pc username for easier ssh
* add user to groups:  
        *  sudo 
        *  video 
        *  input 
        *  bluetooth 
        *  i2c 
        *  gpio
* use the command ```sudo usermod -a -G groupName userName``` 


### Setup Adafruit Servo Hat ###
* setup ic2
        *  enable in ```raspi-config```
* connect servo and esc

### Setup Camera
* if raspicam, enable in sudo raspi-config
* if usb camera, create a special cable to split usb power and data

### Install Dependancies
```
sudo apt-get install libzmq-dev build-essential git cmake python3 python3-dev python3-virtualenv libv4l-dev
```

if on a raspberry pi
```
cd ~
git clone https://github.com/WiringPi/WiringPi
cd WiringPi
./build
```

build czmq
```
cd ~
git clone https://github.com/zeromq/czmq.git
cd czmq
mkdir build && cd build
cmake .. && make
sudo make install
```

if using raspicam camera
```
cd ~
git clone https://github.com/cedricve/raspicam
```

when building raspicam, I had to make this hack to get better framerate.
edit raspicam/src/private/private_impl.cpp
* change: State.framerate            = 10;
* to:     State.framerate            = 60;

```
cd raspicam
mkdir build
cd build
cmake ..
```

Some distributions do not have /usr/local/lib in the default LD_LIBRARY_PATH. To
fix this, you need to edit /etc/ld.so.conf and add in a single line:

  /usr/local/lib

then run the ldconfig command.

  sudo ldconfig


### Optional - Setup RPLIDAR
* download sdk and unzip to rplidar dir
* cd rplidar/sdk && make
* sudo cp ./output/Linux/Release/*.a /usr/local/lib/
* sudo mkdir /usr/local/include/rplidar
* sudo cp ./sdk/include/* /usr/local/include/rplidar

### Optional - Setup BreezySLAM
* git clone https://github.com/simondlevy/BreezySLAM.git
* cd BreezySlam/cpp && make
* sudo make install
* sudo mkdir /usr/local/include/BreezySLAM
* sudo cp *.hpp /usr/local/include/BreezySLAM/
 
### Clone Repo
```
git clone https://github.com/tawnkramer/shark
cd shark
```

### Setup virtualenv

```
virtualenv -p python3 env
source env/bin/activate
```

### Install python packages
```
wget https://github.com/samjabrahams/tensorflow-on-raspberry-pi/releases/download/v0.12.1/tensorflow-0.12.1-cp27-none-linux_armv7l.whl
pip install tensorflow-0.12.1-cp27-none-linux_armv7l.whl
pip install -r requirements.txt
```

### Setup Configs
```
cp config_example.json config.json  
```
edit config.json to match your setup

### Setup Status LED ###
* connect an led to any open pwm on servo hat
* modify config.json to match pwm channel choice

### Build Shark

```
mkdir build
cd build
cmake ..
make
cd ..
```

### Run Shark
```
python shark.py
```

### Run Web UI
optionally
```
cd web
python webapp.py
```
