#!/usr/bin/env python
'''
Launch
Start the robot, joystick, camera, logger, and prediction in their own subprocess.
Author: Tawn Kramer
'''
import os
import multiprocessing as mp
from subprocess import Popen, PIPE, STDOUT
import argparse
import time
import predict
import conf

conf.init()

class Proc():
    def __init__(self, command_plus_args):
        print 'launching', command_plus_args
        self.proc = Popen(command_plus_args, stdout=PIPE)

    def poll(self, output_arr):
        line = self.proc.stdout.readline()
        output_arr.append(line)
        while self.proc.poll() is None:
            line = self.proc.stdout.readline()
            output_arr.append(line)

    def close(self):
        self.proc.terminate()

def go(model, img_pub_address, pred_address, pred_control_address):
    try:
        #start prediction server
        ps = mp.Process( name = 'predict server', target=predict.go, args=(model, pred_address, pred_control_address))
        ps.start()
        time.sleep(4)

        #start shark
        shark_process = []
        shark_process.append(Proc(['./build/shark', ' ']))

        while True:
            output = []
            for proc in shark_process:
                proc.poll(output)
            for line in output:
                print line

    except KeyboardInterrupt:
        print 'stopping'
        for proc in shark_process:
            proc.close()
        ps.terminate()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='shark server')
    parser.add_argument('--model', type=str, default="./models/dash", help='model name')
    args = parser.parse_args()
    
    img_pub_address = ('127.0.0.1', conf.web_image_port)
    pred_address = ('127.0.0.1', conf.keras_predict_server_img_port)
    pred_control_address = ('127.0.0.1', conf.keras_predict_server_control_port)

    go(args.model, img_pub_address, pred_address, pred_control_address)
