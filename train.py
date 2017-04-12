'''
Train
Train your nerual network
Author: Tawn Kramer
'''
from __future__ import print_function
import os
import sys
import glob
import time
import fnmatch
import argparse
import numpy as np
from PIL import Image
import keras
import conf

conf.init()

'''
matplotlib can be a pain to setup. So handle the case where it is absent. When present,
use it to generate a plot of training results.
'''
try:
    import matplotlib
    # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')

    import matplotlib.pyplot as plt
    do_plot = True
except:
    do_plot = False

import models
from load_data import *

def go(model_name, epochs=50, inputs='./log/*.jpg', limit=None, aug_mult=1, aug_perc=0.0):

    print('working on model', model_name)

    '''
    modify config.json to select the model to train.
    '''
    if conf.model_selection == "nvidia_transposed_inputs":
        model = models.get_nvidia_model()
    elif conf.model_selection == "nvidia_standard_inputs":
        model = models.get_nvidia_model_sw()
    else:
        model = models.get_nvidia_model()

    transposeImages = (model.ch_order == 'channel_first')
    
    callbacks = [
        keras.callbacks.EarlyStopping(monitor='val_loss', patience=conf.training_patience, verbose=0),
        keras.callbacks.ModelCheckpoint(model_name + "_best", monitor='val_loss', save_best_only=True, verbose=0),
    ]
    
    batch_size = conf.training_batch_size

    #Train on session images
    X, Y = load_dataset(inputs, limit=limit, transposeImages=transposeImages, augmentMult=aug_mult, aug_perc=aug_perc)
    history = model.fit(X,Y, nb_epoch=epochs, batch_size=batch_size, validation_split=conf.training_validation_split, callbacks=callbacks)

    model.save(model_name)

    
    try:
        if do_plot:
            # summarize history for loss
            plt.plot(history.history['loss'])
            plt.plot(history.history['val_loss'])
            plt.title('model loss')
            plt.ylabel('loss')
            plt.xlabel('epoch')
            plt.legend(['train', 'test'], loc='upper left')
            plt.savefig('loss.png')
    except:
        print("problems with loss graph")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='train script')
    parser.add_argument('model', type=str, help='model name')
    parser.add_argument('--epochs', type=int, default=conf.training_default_epochs, help='number of epochs')
    parser.add_argument('--inputs', default='./log/*.jpg', help='input mask to gather images')
    parser.add_argument('--limit', type=int, default=None, help='max number of images to train with')
    parser.add_argument('--aug_mult', type=int, default=conf.training_default_aug_mult, help='how many more images to augment')
    parser.add_argument('--aug_perc', type=float, default=conf.training_default_aug_percent, help='what percentage of images to augment 0 - 1')
    args = parser.parse_args()
    
    go(args.model, epochs=args.epochs, limit=args.limit, inputs=args.inputs, aug_mult=args.aug_mult, aug_perc=args.aug_perc)

#python train.py mymodel_aug_90_x4_e200 --epochs=200 --aug_mult=4 --aug_perc=0.9
