'''
Led Status
Handle the led status light
Author: Tawn Kramer
'''
import sys
import time
import threading
try:
    import RPi.GPIO as GPIO
except:
    print 'no RPi.GPIO support'
    import gpio_stub as GPIO
    
import conf

def setup(pin=23):
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(pin,GPIO.OUT)

def set_led(pin=23, on_off=True):
    '''
    takes a pin and a bool whether to turn led on or off
    '''
    if on_off:
        GPIO.output(pin, GPIO.HIGH)
    else:
        GPIO.output(pin, GPIO.LOW)

def blink(pin=23, n_times=3, delay=1.0):
    while n_times > 0:
        set_led(pin, True)
        time.sleep(delay)
        set_led(pin, False)
        time.sleep(delay)
        n_times -= 1

def test():
    pin = int(sys.argv[1])
    print "using pin", pin
    setup(pin)
    blink(pin)


'''
This show_status is a timer that must be kept
positive to keep the status light blinking.
We fire the status light when we get images to record.
'''
show_status = 0.0

def blink_status_thread():
    global show_status
    setup(conf.status_pin)
    while show_status > 0.0:
        blink(conf.status_pin, n_times=1, delay=1.0)
        show_status -= 2.0

def start_status_thread():
    th = threading.Thread(target=blink_status_thread)
    th.daemon = True
    global show_status
    show_status = 3.0
    th.start()

def keep_status_alive():
    global show_status
    if show_status <= 0.0:
        start_status_thread()
    show_status = 3.0

def shutdown():
    global show_status
    if show_status > 0.0:
        show_status = 0.0
        time.sleep(2)
    #make sure we leave the led off
    set_led(conf.status_pin, False)
    
  
if __name__ == "__main__":
    test()

