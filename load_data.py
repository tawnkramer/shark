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

#some images are getting cut off
min_image_size = 1 * 1024

def augment_image(img, shadow_images=None):
    #change the coloration, sharpness, and composite a shadow
    factor = random.uniform(0.5, 2.0)
    img = ImageEnhance.Brightness(img).enhance(factor)
    factor = random.uniform(0.5, 1.0)
    img = ImageEnhance.Contrast(img).enhance(factor)
    factor = random.uniform(0.5, 1.5)
    img = ImageEnhance.Sharpness(img).enhance(factor)
    factor = random.uniform(0.0, 1.0)
    img = ImageEnhance.Color(img).enhance(factor)
    try:
        if shadow_images is not None:
            iShad = random.randrange(0, len(shadow_images))
            shadow = Image.open(shadow_images[iShad]).rotate(random.randrange(-15, 15))
            shadow.thumbnail((256, 256))
            r, g, b, a = shadow.split()
            top = Image.merge("RGB", (r, g, b))
            mask = Image.merge("L", (a,))
            mask = ImageEnhance.Brightness(mask).enhance(random.uniform(0.5, 1.0))
            offset = (random.randrange(-64, 64), random.randrange(-64, 64))
            img.paste(top, offset, mask)
    except:
        #print('failed shadow composite')
        #why does this sometimes fail, but mostly work?
        pass
    return img


def get_files(filemask):
    path, mask = os.path.split(filemask)
    #print(path, mask)
    matches = []
    for root, dirnames, filenames in os.walk(path):
        #print(root, dirnames, filenames)
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
            img = augment_image(img, shadow_images=shadow_images)
        
        img_arr = np.array(img)

        if transposeImages:
            img_arr = img_arr.transpose()

    data = parse_img_filepath(file_path)
    return img_arr, data

def load_dataset(filemask, limit=None, transposeImages=False, augmentMult=3, aug_perc=0.0):
    clean_zero_len_files(filemask)
    img_paths = get_files(filemask)
    img_count = len(img_paths)
    gen = load_generator(filemask, limit, transposeImages, augmentMult, aug_perc)
    print( "found", img_count, "images.")
    img_count *= augmentMult
    print("with augmentation:", img_count)

    if limit is not None and img_count > limit:
        print('limiting to', limit)
        img_count = limit

    X = [] #images
    Y = [] #velocity (angle, speed)
    for _ in range(img_count):
        x, y = next(gen)
        X.append(x)
        Y.append(y)
        
    X = np.array(X) #image array [[image1],[image2]...]
    Y = np.array(Y) #array [[angle1, speed1],[angle2, speed2] ...]

    return X, Y


def load_generator(filemask, limit, transposeImages, augmentMult=3, aug_perc=0.0):
    ''' 
    Return a generator that will loops through image arrays and data labels.
    ''' 
    img_paths = get_files(filemask)

    #when augmenting, which images to use for shadows
    shadow_images = ['./shadows/male1.png', './shadows/male2.png', './shadows/female1.png']

    #if limit is not None and len(img_paths) > limit:
    #    img_paths = img_paths[:limit]
    #    print('limiting images to', limit)

    while True:
        for f in img_paths:
            #when augmenting, allow that same image to be augmented multiple times.
            for variation in range(0, augmentMult):
                img_arr, data = get_data(f, transposeImages, aug_perc, shadow_images=shadow_images)
                
                #only steering for now
                data_arr = np.array([data['steering'], data['throttle']])

                yield img_arr, data_arr



def batch_generator(filemask, batch_size, transposeImages, augmentMult=3, aug_perc=0):
    clean_zero_len_files(filemask)
    img_paths = get_files(filemask)
    img_count = len(img_paths)
    limit = None
    augmentMult=augmentMult
    gen = load_generator(filemask, limit=limit, transposeImages=transposeImages, augmentMult=augmentMult, aug_perc=aug_perc)
    print("found", img_count, "images.")
    img_count *= augmentMult
    print("with augmentation:", img_count)

    num_batches = img_count / batch_size
    print(num_batches, "batches")

    for b in range(num_batches):
        X = [] #images
        Y = [] #velocity (angle, speed)
        for _ in range(batch_size):
            x, y = next(gen)
            X.append(x)
            Y.append(y)
            
        X = np.array(X) #image array [[image1],[image2]...]
        Y = np.array(Y) #array [[angle1, speed1],[angle2, speed2] ...]

        yield X, Y
