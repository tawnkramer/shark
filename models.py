'''
Models
Define the different NN models we will use
Author: Tawn Kramer
'''
from __future__ import print_function
from keras.models import Sequential
from keras.layers import Convolution2D, MaxPooling2D
from keras.layers import Dense, Lambda, ELU
from keras.layers import Activation, Dropout, Flatten, Dense
from keras.layers import Cropping2D

import conf

conf.init()

def show_model_summary(model):
    model.summary()
    for layer in model.layers:
        print(layer.output_shape)

def get_nvidia_model(model_output_dim):
    '''
    this model is inspired by the NVIDIA paper
    https://images.nvidia.com/content/tegra/automotive/images/2016/solutions/pdf/end-to-end-dl-using-px.pdf
    Activation is ELU
    Nvidia uses YUV plane inputs
    Final dense layers are adjusted for the lower resolutions in use
    channel last order is used because it results in fewer final weights and performs better
    on limited cpu resources, but does not match the recommended order for Tensorflow.
    Check get_nvidia_model_sw for a model using Tensorflow recommended ordering of channels
    '''
    row, col, ch = conf.row, conf.col, conf.ch
    
    model = Sequential()
    model.ch_order = 'channel_first'
    model.add(Lambda(lambda x: x/127.5 - 1.,
            input_shape=(ch, col, row),
            output_shape=(ch, col, row)))
    model.add(Convolution2D(24, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(ELU())
    model.add(Convolution2D(36, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(ELU())
    model.add(Convolution2D(48, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(ELU())
    model.add(Convolution2D(64, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Flatten())
    model.add(Dropout(.2))
    model.add(ELU())
    model.add(Dense(512))
    model.add(Dropout(.5))
    model.add(ELU())
    model.add(Dense(256))
    model.add(ELU())
    model.add(Dense(128))
    model.add(ELU())
    model.add(Dense(model_output_dim))

    model.compile(optimizer="adam", loss="mse")
    return model

def get_nvidia_model2(model_output_dim):
    '''
    this model is inspired by the NVIDIA paper
    https://images.nvidia.com/content/tegra/automotive/images/2016/solutions/pdf/end-to-end-dl-using-px.pdf
    Activation is ELU
    Nvidia uses YUV plane inputs
    Final dense layers are adjusted for the lower resolutions in use
    channel last order is used because it results in fewer final weights and performs better
    on limited cpu resources, but does not match the recommended order for Tensorflow.
    Check get_nvidia_model_sw for a model using Tensorflow recommended ordering of channels
    '''
    row, col, ch = conf.row, conf.col, conf.ch
    input_shape=(row, col, ch)
    model = Sequential()
    model.ch_order = 'channel_last'

    model.add(Lambda(lambda x: x/127.5 - 1.,
            input_shape=(row, col, ch),
            output_shape=(row, col, ch)))

    #model.add(Cropping2D(cropping=((20,20), (0,0))))

    model.add(Convolution2D(64, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(36, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(48, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(64, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(64, 3, 3, subsample=(1, 1), border_mode="same"))
    model.add(Flatten())
    model.add(Dropout(.2))
    model.add(Activation('relu'))
    model.add(Dense(1000))
    model.add(Dropout(.5))
    model.add(Activation('relu'))
    model.add(Dense(100))
    model.add(Activation('relu'))
    model.add(Dense(10))
    model.add(Activation('tanh'))

    #two floats for steering and throttle commands
    model.add(Dense(model_output_dim))

    #choose a loss function and optimizer
    model.compile(loss='mse', optimizer='adam')

    show_model_summary(model)

    return model


def get_simple_model(model_output_dim):
    '''
    trying for a more simple model
    '''
    row, col, ch = conf.row, conf.col, conf.ch
    input_shape=(row, col, ch)
    model = Sequential()
    model.ch_order = 'channel_last'

    model.add(Lambda(lambda x: x/127.5 - 1.,
            input_shape=(row, col, ch),
            output_shape=(row, col, ch)))

    
    model.add(MaxPooling2D((2, 2)))    
    model.add(Convolution2D(9, 5, 5, subsample=(4, 4), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Dropout(.5))
    model.add(MaxPooling2D((2, 2)))
    model.add(Convolution2D(9, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Dropout(.5))
    model.add(MaxPooling2D((2, 2)))
    model.add(Flatten())
    model.add(Dropout(.2))
    model.add(Activation('relu'))
    model.add(Dense(1000))
    model.add(Dropout(.5))
    model.add(Dense(10))
    model.add(Activation('tanh'))

    #two floats for steering and throttle commands
    model.add(Dense(model_output_dim))

    #choose a loss function and optimizer
    model.compile(loss='mse', optimizer='adam')

    show_model_summary(model)

    return model


def get_nvidia_model_sw(model_output_dim):
    '''
    this model is based on the NVIDIA paper
    https://images.nvidia.com/content/tegra/automotive/images/2016/solutions/pdf/end-to-end-dl-using-px.pdf
    This follows a similar approach to model above, but sets the channel order
    to the recommended for Tensorflow. This results in nearly 5x more trainiable weights
    and did not result in better overal performance in my tests.
    '''
    row, col, ch = conf.row, conf.col, conf.ch
    
    model = Sequential()
    model.ch_order = 'channel_last'
    model.add(Lambda(lambda x: x/127.5 - 1.,
            input_shape=(row, col, ch),
            output_shape=(row, col, ch)))
    model.add(Convolution2D(24, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(36, 5, 5, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(48, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Activation('relu'))
    model.add(Convolution2D(64, 3, 3, subsample=(2, 2), border_mode="same"))
    model.add(Flatten())
    model.add(Dropout(.2))
    model.add(Activation('relu'))
    model.add(Dense(512))
    model.add(Dropout(.5))
    model.add(Activation('relu'))
    model.add(Dense(256))
    model.add(Activation('relu'))
    model.add(Dense(128))
    model.add(Activation('tanh'))
    model.add(Dense(model_output_dim))

    model.compile(optimizer="adam", loss="mse")
    return model
