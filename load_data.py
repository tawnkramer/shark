'''
LoadData
Handle loading image data and augmentation
Author: Tawn Kramer
'''
from __future__ import print_function
import os
import numpy as np
import fnmatch
import random
from PIL import Image
from PIL import ImageEnhance
import conf
import augment

#some images are getting cut off
min_image_size = 1 * 1024


def get_files(filemask):
    path, mask = os.path.split(filemask)
    matches = []
    for root, dirnames, filenames in os.walk(path):
        for filename in fnmatch.filter(filenames, mask):
            matches.append(os.path.join(root, filename))
    return matches

def clean_zero_len_files(filemask):
    img_paths = get_files(filemask)
    for f in img_paths:
        if os.path.getsize(f) < min_image_size:
            os.unlink(f)

def parse_img_filepath(filepath):
    f = filepath.split('/')[-1]
    f = f.split('.')[0]
    f = f.split('_')

    '''
    The neural network seems to train well on values that are not too large or small.
    We recorded the raw axis values. So we normalize them and then apply a STEERING_NN_SCALE
    that puts them roughly in units of degrees +- 30 or so.
    '''
    steering = float(f[3]) / float(conf.js_axis_scale) * conf.STEERING_NN_SCALE
    throttle = float(f[5]) / float(conf.js_axis_scale) * conf.STEERING_NN_SCALE
    
    data = {'steering':steering, 'throttle':throttle }
    return data

def get_data(file_path, transposeImages=False, aug_perc=0.0, shadow_images=None):
    with Image.open(file_path) as img:
        '''
        augment most images, but leave some as is
        '''
        if random.uniform(0.0, 1.0) < aug_perc:
            img = augment.augment_image(img, shadow_images=shadow_images)
        
        img_arr = np.array(img)

        if transposeImages:
            img_arr = img_arr.transpose()

    data = parse_img_filepath(file_path)
    return img_arr, data

