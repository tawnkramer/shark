#!/usr/bin/env python
'''
Predict
Create a server to accept image inputs and run them against a trained neural network.
Author: Tawn Kramer
'''
from __future__ import print_function
import os
import argparse
import time
import json
import traceback
import numpy as np
import keras
import zmq
import conf
import models
from PIL import Image
from PIL import ImageEnhance
import random
import load_data
import conf

conf.init()

def go(model_path, pred_address, pred_control_address):
    
    '''
    Start the server
    '''
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    connect_str = "tcp://%s:%s" % ('*', pred_address[1])
    print('pred listening on', connect_str)
    rc = socket.bind(connect_str)

    cont_socket = context.socket(zmq.REP)
    connect_str = "tcp://%s:%s" % ('*', pred_control_address[1])
    print('pred contl listening on', connect_str)
    rc = cont_socket.bind(connect_str)
    
    print('starting prediction server')

    poller = zmq.Poller()
    poller.register(socket, zmq.POLLIN)
    poller.register(cont_socket, zmq.POLLIN)
    
    '''
    load model and weights,
    post predictions to socket
    '''
    try:
        print('loading model', model_path)
        model = keras.models.load_model(model_path)
    except:
        model = None
        print('failed to open model:', model_path)
        #print(traceback.format_exc())
    
    _row, _col, _ch = conf.row, conf.col, conf.ch
    view_image = False
    do_predict = False
    num_pred = 0
    start = 0
    throttle = 0.0
    on_course = 1.0
    off_course = 0.0

    while True:
        socks = dict(poller.poll())

        if socket in socks and socks[socket] == zmq.POLLIN:
            '''
            we have an image
            '''
            img_str = socket.recv()
            #print('got an image')
            lin_arr = np.fromstring(img_str, dtype=np.uint8)
            img = lin_arr.reshape(_row, _col, _ch)
            
            if model is not None:
                count, h, w, ch = model.inputs[0].get_shape()
                ih, iw, ich = img.shape
                
                if h == ich and ch == ih:
                    img = img.transpose()
                    
                try:
                    outputs = model.predict(img[None, :, :, :])

                    if len(outputs[0]) == 1:
                        steering = outputs
                    elif len(outputs[0]) == 2:
                        steering, throttle = outputs[0]
                    elif len(outputs[0]) == 3:
                        steering, on_course, off_course = outputs[0]

                    '''
                    This steering is going to be in degrees. We scale that to our bot.
                    '''
                    steering = steering / conf.STEERING_NN_SCALE * conf.js_axis_scale
                    throttle = throttle / conf.STEERING_NN_SCALE * conf.js_axis_scale
                except:
                    print('problems w predict. probably wrong image dimension for model')
                    print(traceback.format_exc())
                    steering = 0.0
            else:
                steering = 0.0

            prediction = '{ "steering" : %f,  "throttle" : %f}' %\
                (steering, throttle)

            socket.send(prediction)
            #print ('prediction', prediction)
            if num_pred == 0:
                num_pred += 1
                start = time.time()
            else:
                num_pred += 1
            
            if num_pred == 100:
                duration = time.time() - start
                print('sending pred at', (float)(num_pred) / duration, 'fps')
                num_pred = 0

        if cont_socket in socks and socks[cont_socket] == zmq.POLLIN:
            '''
            we have new model
            '''
            message = cont_socket.recv()
            try:
                print('got:', message)
                jsonObj = json.loads(message)
                if jsonObj['command'] == 'set_throttle':
                    throttle = jsonObj['throttle']
                
                if jsonObj['command'] == 'load_model':
                    model_name = jsonObj['model_path']
                    model_path = os.path.join("./models/", model_name)
                    print('loading model', model_path)
                    try:
                        model = keras.models.load_model(model_path)
                        cont_socket.send("model loaded")
                    except:
                        cont_socket.send("model load failed : %s", model_path)
                if jsonObj['command'] == 'ping':
                    cont_socket.send("predict is alive.")
            except:
                cont_socket.send("failure on message")

def pred_image(model_path, image_path):
    if model_path.find('.json') != -1:
        with open(model_path, 'r') as jfile:
            model = keras.models.model_from_json(json.load(jfile))

        model.compile("sgd", "mse")
        weights_file = model_path.replace('json', 'keras')
        model.load_weights(weights_file)
    else:
        model = keras.models.load_model(model_path)

    img = Image.open(image_path)
    enh = True
    if enh:
        img = load_data.augment_image(img)
        img.show()

    img = np.array(img)       
    count, h, w, ch = model.inputs[0].get_shape()
    ih, iw, ich = img.shape
    if h == ich and ch == ih:
        img = img.transpose()
    print('input shape, %d, %d , %d' % (w, h, ch))
    print('img shape', img.shape)
    #img = img.transpose()
    outputs = model.predict(img[None, :, :, :])
    print('outputs', outputs / conf.STEERING_NN_SCALE * conf.js_axis_scale)

    
def profile_speed(model_path, model=None):
    '''
    load a keras model and feed it a random array to test the speed.
    accept a .json/.keras(h5) pair or a .h5 model file.
    '''
    if model is None:
        if model_path.find('.json') != -1:
            with open(model_path, 'r') as jfile:
                model = keras.models.model_from_json(json.load(jfile))

            model.compile("sgd", "mse")
            weights_file = model_path.replace('json', 'keras')
            model.load_weights(weights_file)
        else:
            model = keras.models.load_model(model_path)

    #query the input to see what it likes
    count, w, h, ch = model.inputs[0].get_shape()

    #generate random array in the right shape
    img = np.random.rand(int(w), int(h), int(ch))

    num_pred = 0

    try:
        while True:

            '''
            run forward pass on model
            '''
            model.predict(img[None, :, :, :])

            '''
            keep track of iterations and give feed back on iter/sec
            '''
            if num_pred == 0:
                num_pred += 1
                start = time.time()
            else:
                num_pred += 1
            if num_pred == 300:
                duration = time.time() - start
                print('pred at', (float)(num_pred) / duration, 'fps')
                num_pred = 0

    except KeyboardInterrupt:
        pass


# ***** main *****
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='prediction server')
    parser.add_argument('model', type=str, help='model name')
    parser.add_argument('--test_speed', dest='test_speed', action="store_true", help='option to trigger profiling from local images')
    parser.add_argument('--test_image', help='option do pass on single image')
    parser.set_defaults(test_speed=False, test_image=None)
    args = parser.parse_args()

    if args.test_image is not None:
        pred_image(args.model, args.test_image)
    elif(args.test_speed):
        #model = models.get_nvidia_model_sw()
        #model = models.get_nvidia_model()
        model = None
        print('running profile on prediction loop')
        profile_speed(args.model, model)
    else:
        pred_address =  ('localhost', conf.keras_predict_server_img_port)
        pred_control_address =  ('localhost', conf.keras_predict_server_control_port)
        go(args.model, pred_address, pred_control_address)

