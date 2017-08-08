![shark.jpg](https://github.com/tawnkramer/shark/blob/master/web/img/shark.jpg)

# Shark #

A framework for controlling, recording, training, and running a self driving robotic car. Focused now on an implementation that runs on 1/10th scale rc cars with pwm servo steering and ESC throttle controller driven by an 9865 Servo board with a RaspberryPi 3 or Jetson TX2.

---
[![IMAGE ALT TEXT](https://img.youtube.com/vi/EWDvgN8rauk/0.jpg)](https://www.youtube.com/watch?v=EWDvgN8rauk "shark video")

### Goals ###

* Use ZeroMQ and C/Python to create a mesh of components that are flexible and fast 
* Run all code on the robot ( pi 3 / jetson tx2 )
* Manage things with a mobile device via web page and joystick controller
* Train in the cloud

### Build your bot ###
* check [docs/bot_setup.md](https://github.com/tawnkramer/shark/blob/master/docs/bot_setup.md)

Once you are ready, you can install them as services. 
Check shark.service and sharkweb.service to run on startup.

---
## Workflow ##

### Check camera output: ###
* Navigate to [web page](http://raspberrypi.local:8080)
* Select 'robot'. You should see a live image from camera

### Logging: ###
* press X on PS3 Sixaxis controls to enable recording
* left analog to steer, right analog forward to throttle
* only recording when throttle is non zero
* press X to disable recording

### Edit logs: ###
* Navigate to [web page](http://raspberrypi.local:8080)
* Select 'log'
* Select 'view/edit logs'
* observe recorded logs
* if unwanted frames, use slider, 'set trim start', and 'set trim end' to set range
* use 'trim log' button to remove unwanted frames

### Manual Training: ###
* copy logs to your PC: scp me@pi.local:~/projects/shark/log/*.jpg ~/projects/shark/log
* train model on your PC: python train.py mymodel
* cp mymodel to pi: scp mymodel me@pi.local:~/projects/shark/model/
* on the pi: python shark.py --model mymodel

### Web EC2 Based Training: ###
* check [docs/aws_setup.md](https://github.com/tawnkramer/shark/blob/master/docs/aws_setup.md)
* Navigate to [web page](http://raspberrypi.local:8080)
* Select 'ec2' button
* Select 'start ec2'
* Wait for 1 to 2 minutes and select 'check ec2' until the machine is ready.
* Select 'prepare host' to copy code to remote host
* From the home menu select 'log'
* Select 'upload logs'
* From the home menu select 'train'
* Select 'model' and name the new model
* Optionally select 'epochs' and set upper limit of epochs
* Select 'train' to start training. Feedback varies depending on browser. Better luck from a desktop browser. Tested mostly on Firefox.
* When complete, select 'push model' to tell robot predict loop to load that trained result.
* Ready to test self driving
* When you are done with server, select 'release ec2' to shutdown remote machine.

### Self Driving: ###
* hit triangle toggle start self driving.
* use dpad up and down to modify throttle scale

