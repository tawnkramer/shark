#!/usr/bin/env python
'''
Webapp
This creates a web service using cherrypy to help manage the robot
This is designed to run on the robot
Author: Tawn Kramer
'''
from __future__ import print_function
import os
import sys
import time
import signal
from subprocess import Popen, PIPE, STDOUT
import multiprocessing as mp
from io import BytesIO
from threading import Thread
import glob
import numpy as np
import traceback
import cherrypy
import json
import zmq
from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw
sys.path.append('..')
#import led_status
import conf

cherrypy.server.socket_host = '0.0.0.0'


def img_to_binary(img):
    '''
    accepts: PIL image
    returns: binary stream
    '''
    f = BytesIO()
    img.save(f, format='jpeg')
    return f.getvalue()

class Proc():
    def __init__(self, command_plus_args, shell=False):
        print('launching', command_plus_args)
        self.proc = Popen(command_plus_args, shell=shell, stdout=PIPE)

    def get_output(self, output_arr):
        line = self.proc.stdout.readline()
        output_arr.append(line)
        while self.proc.poll() is None:
            line = self.proc.stdout.readline()
            output_arr.append(line)

    def poll(self):
        '''
        returns a tuple of string line output as long as process is running
        and bool, True when complete
        '''
        line = self.proc.stdout.readline()
        #i'd like to read stderr, but it kills the feedback from stdout during training, ARGGG!!!
        #line = line + self.proc.stderr.readline()
        complete = self.proc.poll() is not None
        return line, complete

    def close(self):
        self.proc.terminate()

class WebSite(object):
    #we can init any variables we want to persist. These
    #will be available for any of our methods
    def __init__(self):
        #led_status.setup(conf.status_pin)
        #led_status.blink(conf.status_pin, n_times=3, delay=0.5)
        self.pred_control_address = ('127.0.0.1', conf.keras_predict_server_control_port)
        self.last_frame = Image.open('./img/shark.jpg')
        self.model_file = conf.web_rel_default_model
        self.model = None
        self.iImage = 0
        self.train_epochs = conf.training_default_epochs
        self.set_ec2_train_defaults()
        self.sync_remote_proc = None
        self.sync_local_proc = None
        self.sync_proc_log = []
        
        if conf.debug_test_web:
            print("verbose debug message enabled.")

        #target local by default
        self.target_local()
        #you can also target a non aws server.
        #self.set_detalt_alter_server()
        
    def set_default_alter_server(self):
        self.aws_host_username = conf.alt_train_user
        self.aws_host_ip = conf.alt_train_host
        self.pem_file = None
        self.source_command = "source ~/tensorflow/bin/activate" 

    def set_ec2_train_defaults(self):
        self.aws_host_username = "ubuntu"
        self.aws_host_ip = None
        self.aws_instance_id = None
        self.aws_req_id = None
        self.pem_file = conf.pem_filename
        self.source_command = 'source rem_env'

    def target_alt(self):
        #you can also target a non aws server.
        self.aws_host_username = conf.alt_train_user
        self.aws_host_ip = conf.alt_train_host
        self.pem_file = None
        self.source_command = "source ~/tensorflow/bin/activate"
        raise cherrypy.HTTPRedirect("/manage_ec2")
    target_alt.exposed = True

    def target_local(self):
        #you can also target the local device
        self.aws_host_username = conf.local_user
        self.aws_host_ip = "localhost"
        self.pem_file = None
        self.source_command = "ls"
        #raise cherrypy.HTTPRedirect("/manage_ec2")
    target_local.exposed = True

    def get_css(self):
        return '''
        <style>
            a {
                display: block;
                width: 300px;
                height: 100px;
                background: #4E9CAF;
                padding: 30px;
                text-align: center;
                border-radius: 5px;
                color: white;
                font-weight: bold;
                font-size: 40px;
                text-decoration: none;
            }

        .img_stream {
                width: 300px;
                padding: 30px;
        }

        .frame_slider {
                width: 300px;
                margin: 30px;
        }

        .new_model_form {
                width: 300px;
                background: salmon;
                padding: 30px;
                text-align: center;
                border-radius: 5px;
                color: white;
                font-weight: bold;
                font-size: 30px;
                margin:auto;
        }

        .padded {
            padding: 10px;            
            margin: 5pf;
        }        

        .active_model {
                display: block;
                width: 300px;
                height: 50px;
                background: salmon;
                padding: 30px;
                text-align: center;
                border-radius: 5px;
                color: white;
                font-weight: bold;
                font-size: 30px;
        }

        .trim {
                display: block;
                width: 75px;
                height: 40px;
                background: salmon;
                padding: 5px;
                text-align: center;
                border-radius: 5px;
                color: white;
                font-weight: bold;
                font-size: 20px;
                float: left;
                margin: 10px;
        }

        .sel_model {
                display: block;
                width: 600px;
                height: 100px;
                background: #4E9CAF;
                padding: 30px;
                text-align: center;
                border-radius: 5px;
                color: white;
                font-weight: bold;
                font-size: 30px;
                text-decoration: none;
            }

        .clear {
            clear: both;
        }

        </style>
          '''

    def get_js(self):
        return '''
        <script src="static/js/main.js"></script>
        '''

    def easy_page(self, content, title="Shark Robot Control"):
        page = '''
        <html>
        <head>        
        <title>%s</title>
        %s\n
        %s
        </head>
        <body bgcolor="#ffffff">        
        %s
        </body>
        </html>
        ''' % ( title, self.get_css(), self.get_js(), content)

        return page

    def stream_page(self, title="Shark Robot Control"):
        page = '''
        <html>
        <head>        
        <title>%s</title>
        %s\n
        %s
        </head>
        <body bgcolor="#ffffff">        
        ''' % ( title, self.get_css(), self.get_js())
        return page
        
    #this method gets called when the user goes to the root of our site
    #note that the .exposed needs to be added to every function that will be a url on our site.
    def index(self):
        return self.easy_page('''
        <img src="/img/shark.jpg"></img><br>
        <a href="/manage_robot">robot</a><br>
        <a href="/manage_ec2">ec2</a><br>
        <a href="/manage_log">log</a><br>        
        <a href="/manage_train">train</a><br>        
        ''')
    index.exposed = True

    def manage_robot(self):
        return self.easy_page('''
        <br>
        <img  class="img_stream" src="/img_live"></img><br>        
        <a href="/">home</a><br>
        <a href="/lidar">lidar</a><br>
        ''')
    manage_robot.exposed = True

    def check_robot(self):
        res = []
        command = '{ "command" : "ping" }'
        context = zmq.Context()
        socket = context.socket(zmq.REQ)
        connect_str = "tcp://%s:%s" % (self.pred_control_address[0], self.pred_control_address[1])
        socket.connect(connect_str)
        socket.send_string(command)
        reply = socket.recv()
        res.append(reply)
        res.append("<br>")
        res.append('<img src="/img_live"></img><br>')
        res.append(self.home_link())        
        res.append('<a href="/manage_robot">robot</a><br>')
        res.append('<a href="/lidar">lidar</a><br>')
        return self.easy_page("".join(res))
    check_robot.exposed = True

    def img_post(self, image):
        #take an image and set it to self.last_frame
        #so it will be exposed in img_live
        self.last_frame = Image.open(BytesIO(image))
    img_post.exposed = True

    def img_live(self):
        cherrypy.response.headers["Content-Type"] = "multipart/x-mixed-replace;boundary=--boundarydonotcross"
        boundary = b"--boundarydonotcross"
        def content():
            iImage = 0
            delay  = 1.0 / 30.0
            context = zmq.Context()
            socket = context.socket(zmq.REQ)
            connect_str = "tcp://127.0.0.1:%d" % conf.web_image_port
            print("connecting to live image at:", connect_str)
            socket.connect(connect_str)
            command = "hi"
            row, col, ch = conf.row, conf.col, conf.ch
            font = ImageFont.truetype("./static/fonts/BADABB__.TTF", 16)
            while True:
                socket.send_string(command)
                img_str = socket.recv()
                lin_arr = np.fromstring(img_str, dtype=np.uint8)
                img_arr = lin_arr.reshape(row, col, ch)
                img = Image.fromarray(img_arr, "RGB")
                '''
                if self.model is not None:
                    try:
                        pred = self.model.predict(img_arr[None, :, :, :])[0][0]
                        draw = ImageDraw.Draw(img)
                        pred_str = '%0.2f' % pred
                        draw.text((0, 0),pred_str,(128,128,255),font=font)
                    except:
                        pass
                '''
                with BytesIO() as output:
                    img.save(output, "JPEG")
                    jpg_img = output.getvalue()
                cont_len = "Content-length: %s\r\n\r\n" % len(jpg_img)
                yield(boundary)
                yield(b"Content-type: image/jpeg\r\n")
                yield( str.encode(cont_len) )
                yield(jpg_img)
                #time.sleep(delay)
                
        return content()
    img_live.exposed = True
    img_live._cp_config = {'response.stream': True}

    def lidar(self):
        res = []
        res.append(self.home_link())        
        res.append("<br>")
        res.append('<a href="/manage_robot">robot</a><br>')
        res.append("<br>")      
        res.append('<img src="/lidar_live"></img><br>')
        return self.easy_page("".join(res))
    lidar.exposed = True

    def lidar_live(self):
        cherrypy.response.headers["Content-Type"] = "multipart/x-mixed-replace;boundary=--boundarydonotcross"
        boundary = b"--boundarydonotcross"
        def content():
            iImage = 0
            context = zmq.Context()
            socket = context.socket(zmq.REQ)
            connect_str = "tcp://127.0.0.1:%d" % conf.web_lidar_port
            print("connecting to live image at:", connect_str)
            socket.connect(connect_str)
            command = "hi"
            row, col, ch = 512, 512, 3
            font = ImageFont.truetype("./static/fonts/BADABB__.TTF", 16)
            while True:
                socket.send_string(command)
                img_str = socket.recv()
                lin_arr = np.fromstring(img_str, dtype=np.uint8)
                img_arr = lin_arr.reshape(row, col, ch)
                img = Image.fromarray(img_arr, "RGB")
                with BytesIO() as output:
                    img.save(output, "JPEG")
                    jpg_img = output.getvalue()
                yield(boundary)
                yield(b"Content-type: image/jpeg\r\n")
                yield(("Content-length: %s\r\n\r\n" % len(jpg_img)).encode() )
                yield(jpg_img)
                
        return content()
    lidar_live.exposed = True
    lidar_live._cp_config = {'response.stream': True}

    def manage_log(self):
        res = []
        res.append(self.home_link())        
        res.append(
        '''
            <a href="/edit_logs">view/edit logs</a><br>
            <a href="/upload_logs">upload logs</a><br>
            <a href="/rsync_logs">rsync logs</a><br>
            <a href="/new_logs_dir">fresh logs</a><br>
            <a href="/select_logs">select logs</a><br>
        ''')
        return self.easy_page("".join(res))
    manage_log.exposed = True

    def get_log_dir(self):
        return os.path.join('../', conf.log_dir)

    def save_current_log_dir(self):
        '''
        If there are any files in the current log, then move them to a new dir named with the current time stamp
        '''
        files = glob.glob(os.path.join(self.get_log_dir(), '*'))
        if len(files) > 0:
            ts = time.strftime("%Y_%m_%d__%H_%M_%S")
            newLogDir = "../log_%s" % ts
            self.exec_command_via_shell_script('mv ../log %s' % newLogDir)        

    def new_logs_dir(self):
        self.save_current_log_dir()
        self.exec_command_via_shell_script('cd .. && mkdir %s' % conf.log_dir)
        raise cherrypy.HTTPRedirect('/manage_log')
    new_logs_dir.exposed = True

    def select_logs(self):
        paths = glob.glob(self.get_log_dir() + "*")
        res = []
        for p in paths:
            label = " ".join(p.split('__'))
            res.append('<a href="/set_logdir?dir=%s">%s</a><br>' % ( p, label))
        return self.easy_page(''.join(res))
    select_logs.exposed = True

    def set_logdir(self, dir):
        if dir != self.get_log_dir():
            self.save_current_log_dir()
            self.exec_command_via_shell_script('rm -rf %s' % self.get_log_dir())          
            self.exec_command_via_shell_script('mv %s %s' % ( self.get_log_dir(), dir))
        raise cherrypy.HTTPRedirect('/manage_log')
    set_logdir.exposed = True

    def edit_logs(self):
        res = []
        self.gather_log_images()        
        res.append('<img class="img_stream" src="/img_log"></img><br>')

        res.append('<input class="frame_slider" type="range" min="0" max="%d" value="0" id="frame_slider" oninput="on_frame_slider(value)">' % len(self.log_dir))
        res.append('<div>')
        res.append('<div class="trim" id="num_images">%d images </div>' %  len(self.log_dir))
        res.append('<div class="trim" id="trim_start">trim start</div><div class="trim" id="trim_end">trim end</div>')
        res.append('<div class="clear"></div>')
        res.append('</div>')
        res.append('''<a onclick='ajax_load("/play_pause");'>Play/Pause</a><br>''')
        res.append('''<a onclick='ajax_load("/set_trim_start", "trim_start");'>Set Trim Start</a><br>''')
        res.append('''<a onclick='ajax_load("/set_trim_end", "trim_end");'>Set Trim End</a><br>''')
        res.append('''<a href="/trim_log">Trim Log</a><br>''')
        res.append(self.home_link())
        self.playLog = True
        return self.easy_page("".join(res))
    edit_logs.exposed = True

    def set_log_frame(self, frame):
        self.iImage = int(frame)
        if self.iImage >= len(self.log_dir):
            self.iImage = len(self.log_dir) - 1
        self.playLog = False
    set_log_frame.exposed = True

    def play_pause(self):
        self.playLog = not self.playLog
    play_pause.exposed = True

    def gather_log_images(self):
        self.log_dir = glob.glob(os.path.join(self.get_log_dir(), "*.jpg"))
        self.log_dir.sort(key=lambda x: os.stat(x).st_mtime)
        self.iImage = 0
        self.trim_start = 0
        self.trim_end = 0

    def set_trim_start(self):
        self.trim_start = self.iImage
        return str(self.trim_start)
    set_trim_start.exposed = True

    def set_trim_end(self):
        self.trim_end = self.iImage
        return str(self.trim_end)
    set_trim_end.exposed = True

    def trim_log(self):
        iStart = self.trim_start
        iEnd = self.trim_end
        for i in range(iStart, iEnd):
            filename = self.log_dir[i]
            os.unlink(filename)
        self.gather_log_images()
        self.iImage = iStart - 100
        if self.iImage < 0:
            self.iImage = 0
        raise cherrypy.HTTPRedirect('/edit_logs')
    trim_log.exposed = True

    def img_log(self):
        cherrypy.response.headers["Content-Type"] = "multipart/x-mixed-replace;boundary=--boundarydonotcross"
        boundary = b"--boundarydonotcross"
        
        def content():
            interval = 1.0 / 60.0
            self.last_time = time.time()
            font = ImageFont.truetype("./static/fonts/BADABB__.TTF", 16)
            img = Image.open('./img/shark.jpg')
            img = img_to_binary(img)
            while True:
                since_last = time.time() - self.last_time
                if True:
                    if self.iImage < len(self.log_dir):
                        try:
                            img = Image.open(self.log_dir[self.iImage])
                            draw = ImageDraw.Draw(img)
                            draw.text((0, 0),str(self.iImage),(255,255,255),font=font)
                            ''' This can be used to preview pred.
                            if self.model is not None:
                                try:
                                    img_arr = np.array(img)
                                    pred = self.model.predict(img_arr[None, :, :, :])[0][0]
                                    draw = ImageDraw.Draw(img)
                                    pred_str = '%0.2f' % pred
                                    draw.text((0, 100),pred_str,(128,128,255),font=font)
                                except:
                                    pass
                            '''        
                            if self.playLog:
                                self.iImage += 1
                            
                            img = img_to_binary(img)
                        except:
                            img = Image.open('./img/shark.jpg')
                            img = img_to_binary(img)
                    if img is not None:
                        yield(boundary)
                        yield(b"Content-type: image/jpeg\r\n")
                        yield(b"Content-length: %s\r\n\r\n" % len(img))
                        yield(img)
                    time.sleep(interval)
        return content()
    img_log.exposed = True
    img_log._cp_config = {'response.stream': True}


    def manage_ec2(self):
        return self.easy_page('''
        <div class="active_model" >%s</div><br>
        <a href="/">home</a><br>
        <a href="/start_ec2">start ec2</a><br>
        <a href="/check_ec2">check ec2</a><br>
        <a href="/prepare_host">prepare host</a><br>
        <a href="/sync_code">sync code</a><br>
        <a href="/release_ec2">release ec2</a><br><br>
        <a href="/target_alt">target alt server</a><br>
        <a href="/target_local">target local</a><br>
        ''' % (self.aws_host_ip))
    manage_ec2.exposed = True
    
    #here's a utility function that's not callable from a website url.
    def home_link(self):
        return '<a href="/">home</a><br>'

    def start_ec2(self):
        #request spot instances
        p = Proc(['aws', 'ec2', 'request-spot-instances',  '--spot-price',  conf.aws_spot_price,
            '--launch-specification', 'file://%s' % conf.aws_config])
        outp = []
        res = ['<pre>']
        p.get_output(outp)
        res.append("".join(outp))
        res.append('</pre>')
        res.append('<br> <a href="/check_ec2">check ec2</a><br>')
        res.append(self.home_link())
        return self.easy_page("".join(res))
    start_ec2.exposed = True

    def check_ec2(self):
        #reset these
        self.set_ec2_train_defaults()
        
        p = Proc(['aws', 'ec2', 'describe-instances'])
        outp = []
        res = []
        res = ['<pre>']
        p.get_output(outp)
        try:
            jsonobj = json.loads(''.join(outp))
            for reservation in jsonobj["Reservations"]:
                state = str(reservation["Instances"][0]["State"]["Name"])
                if state == "running":
                    res.append("State: %s\n" % state)
                    self.aws_host_ip = str(reservation["Instances"][0]["PublicIpAddress"])
                    res.append("IP: %s\n" % self.aws_host_ip)
                    self.aws_instance_id = str(reservation["Instances"][0]["InstanceId"])
                    res.append("Instance Id: %s\n" % self.aws_instance_id)
                    self.aws_req_id = str(reservation["Instances"][0]["SpotInstanceRequestId"])
                    res.append("Request Id: %s\n" % self.aws_req_id)
                    res.append("\n")
                else:         
                    res.append("State: %s\n" % state)
                    res.append("Instance Id: %s\n" % reservation["Instances"][0]["InstanceId"])
                    res.append("Request Id: %s\n" % reservation["Instances"][0]["SpotInstanceRequestId"])
                    res.append("\n")
            res.append('</pre>')
        except:
            res.append("".join(outp))
            res.append('</pre>')
        res.append(self.home_link())
        res.append('<a href="/manage_ec2">ec2</a>')
        return self.easy_page("".join(res))
    check_ec2.exposed = True

    def release_ec2(self):
        if self.aws_req_id is None or self.aws_instance_id is None:
            raise cherrypy.HTTPRedirect('/check_ec2')
        #release spot instances and cancel request
        #aws ec2 cancel-spot-instance-requests --spot-instance-request-ids sir-e9ci6pgg
        #aws ec2 terminate-instances --instance-ids i-08bac57c2f10f5dbb
        p = Proc(['aws', 'ec2', 'cancel-spot-instance-requests',  '--spot-instance-request-ids', self.aws_req_id ])
        outp = []
        res = ['<pre>']
        p.get_output(outp)
        res.append("".join(outp))
        p = Proc(['aws', 'ec2', 'terminate-instances',  '--instance-ids', self.aws_instance_id ])
        p.get_output(outp)
        res.append("".join(outp))
        res.append('</pre>')
        res.append(self.home_link())
        res.append('<br> <a href="/manage_ec2">ec2</a>')
        return self.easy_page("".join(res))
    release_ec2.exposed = True

    def manage_train(self):
        p, fnm = os.path.split(self.model_file)        
        return self.easy_page('''
        <p class=active_model>model: %s  host: %s  epochs: %d </p><br>
        <a href="/">home</a><br>
        <a href="/select_model">model</a><br>
        <a href="/epoch_form">epochs</a><br>
        <a href="/start_train">train</a><br>
        <a href="/check_train">results</a><br>        
        <a href="/push_model">push model</a><br>        
        '''% (fnm, self.aws_host_ip, self.train_epochs))
        #<a href="/preview_model">preview model</a><br>
    manage_train.exposed = True

    def select_model(self):
        models = glob.glob("../models/*")
        models.sort()
        res = []
        res.append('<a href="/new_model_form">%s</a><br>' % ( "new model"))       
        for m in models:
            p, fnm = os.path.split(m)
            res.append('<a class="sel_model" href="/set_model?name=%s">%s</a><br>' % ( fnm, fnm))
        return self.easy_page(''.join(res))
    select_model.exposed = True

    def new_model_form(self):
        res = []
        res.append('''
        <form class="new_model_form" action="/make_new_model">
        Model Name:<br>
        <input class="padded" type="text" name="name" ><br><br>
        <input class="padded" type="submit" value="Submit">
        </form>
        ''')
        return self.easy_page(''.join(res))
    new_model_form.exposed = True

    def make_new_model(self, name):
        self.model_file = os.path.join("../models", name)
        raise cherrypy.HTTPRedirect('/manage_train')
    make_new_model.exposed = True

    def set_model(self, name):
        self.model_file = os.path.join('../models/', name)
        raise cherrypy.HTTPRedirect('/manage_train')
    set_model.exposed = True

    '''
    def preview_model(self):
        try:
            self.model = keras.models.load_model(self.model_file)
        except:
            self.model = None
        raise cherrypy.HTTPRedirect('/manage_robot')
    preview_model.exposed = True
    '''

    def push_model(self):
        cherrypy.response.headers["Content-Type"] = 'text/html'      

        def content():
            yield("sending model: %s to prediction engine." % self.model_file)
            res = []
            res.append('<pre>')
            #i'm splitting the path here because the path is relative to this cwd which is different
            #than the cwd. perhaps we could expand the relative path. but we know it's in the ./models dir
            #so do what is expediant.
            try:
                pth, name = os.path.split(self.model_file)
                command = '{ "command" : "load_model", "model_path" : "%s" }' % name
                context = zmq.Context()
                socket = context.socket(zmq.REQ)
                connect_str = "tcp://%s:%s" % (self.pred_control_address[0], self.pred_control_address[1])
                socket.connect(connect_str)
                socket.send_string(command.encode('utf-8'))
                reply = socket.recv()
                #led_status.blink(conf.status_pin, n_times=10, delay=0.2)
                res.append(reply)
            except:
                tb = traceback.format_exc()
                res.append(tb)

            res.append('</pre>')
            res.append("<br>")
            res.append(self.home_link())
            res.append('<a href="/manage_train">train</a><br>')
            yield self.easy_page("".join(res))

        return content()

    push_model.exposed = True
    push_model._cp_config = {'response.stream': True}

    def epoch_form(self):
        res = []
        res.append('''
        <form class="new_model_form" action="/set_epochs">
        Set the number of epochs to train:<br>
        <input class="padded" type="text" name="count" value="%d" ><br><br>
        <input class="padded" type="submit" value="Submit">
        </form>
        ''' % (self.train_epochs))
        return self.easy_page(''.join(res))
    epoch_form.exposed = True

    def set_epochs(self, count):
        self.train_epochs = int(count)
        raise cherrypy.HTTPRedirect('/manage_train')
    set_epochs.exposed = True

    def exec_command_via_shell_script(self, command):
        print("command: ", command)
        cmd_filename = "../scripts/command"
        command_file = open(cmd_filename, "wt")
        command_file.write(command)
        command_file.close()
        #this works but gives no feedback
        #res = os.system(cmd_filename)
        #return str(res)
        res = []
        p = Proc(cmd_filename, shell=True)
        p.get_output(res)
        return "".join(res)

    def prepare_host(self):
        if self.aws_host_ip is None:
            raise cherrypy.HTTPRedirect('/manage_ec2')
        
        res = []
        commands = []

        #copy the pub key so we don't have to pass the pem for each ssh command.        
        #cat ~/.ssh/id_rsa.pub | ssh -i "mysecret.pem" ubuntu@52.205.11.150 'cat >> .ssh/authorized_keys'
        if self.pem_file is not None:
            command = '''cat ~/.ssh/id_rsa.pub | ssh -oStrictHostKeyChecking=no -i "%s" %s@%s 'cat >> .ssh/authorized_keys' ''' % \
                (self.pem_file, self.aws_host_username, self.aws_host_ip)
            commands.append(command)            

        #copy the environment file.
        command = 'scp ../scripts/rem_env %s@%s:~/' % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        #make shark and log dir
        command = "ssh -oStrictHostKeyChecking=no %s@%s 'mkdir shark; cd shark; mkdir log;'" % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        #copy all the training python source
        command = "scp ../*.py %s@%s:~/shark" % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        #copy config.json
        command = "scp ../config.json %s@%s:~/shark" % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        command = "scp -r shadows %s@%s:~/shark" % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        res.append('<pre>')
        for command in commands:
            res.append(command)
            res.append("\n")
            res.append( self.exec_command_via_shell_script(command) )
            res.append("\n\n")
        res.append('</pre>')
        res.append(self.home_link())
        res.append('<a href="/manage_ec2">manage ec2</a><br>')
        return self.easy_page("".join(res))
    prepare_host.exposed = True

    def sync_code(self):
        if self.aws_host_ip is None:
            raise cherrypy.HTTPRedirect('/manage_ec2')
        
        res = []
        commands = []

        #copy all the training python source
        command = "scp ../*.py %s@%s:~/shark" % (self.aws_host_username, self.aws_host_ip)
        commands.append(command)

        res.append('<pre>')
        for command in commands:
            res.append(command)
            res.append("\n")
            res.append( self.exec_command_via_shell_script(command) )
            res.append("\n\n")
        res.append('</pre>')
        res.append(self.home_link())
        res.append('<a href="/manage_ec2">manage ec2</a><br>')
        return self.easy_page("".join(res))
    sync_code.exposed = True

    def upload_logs(self):
        '''
        The initial upload is best done as a single tar file.
        This tars the log file, no compression because we are sending
        jpg files that are already compressed.
        '''
        if self.aws_host_ip is None:
            raise cherrypy.HTTPRedirect('/manage_ec2')

        cherrypy.response.headers["Content-Type"] = 'text/html'
        def content():
            commands = []

            #make tar file
            command = 'cd .. && tar -cf log.tar %s' % conf.log_dir
            commands.append(command)

            #make shark and log dir
            #command = "cd .. && scp -C log.tar %s@%s:~/shark" % (self.aws_host_username, self.aws_host_ip)
            #command = 'rsync -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress -rt ../log.tar %s@%s:~/shark/' % (self.aws_host_username, self.aws_host_ip)            
            command = 'scp ../log.tar %s@%s:~/shark/' % (self.aws_host_username, self.aws_host_ip)
            commands.append(command)

            #expand tar
            command = "ssh -oStrictHostKeyChecking=no %s@%s 'cd shark; rm -rf log; tar -xf log.tar;'" % (self.aws_host_username, self.aws_host_ip)
            commands.append(command)

            #rm tar file
            #command = 'cd .. && rm log.tar'
            #commands.append(command)

            yield(self.stream_page(title="Shark Upload"))
            yield('<pre>')
            for com in commands:                
                cmd_filename = "../scripts/command"
                command_file = open(cmd_filename, "wt")
                yield(com + "\n\n")
                command_file.write(com)
                command_file.close()
                p = Proc(cmd_filename, shell=True)
                output, complete = p.poll()
                yield output
                while not complete:
                    output, complete = p.poll()
                    yield (output)            
            yield('</pre>')
            yield(self.home_link())
            yield('<br><a href="/manage_train">train</a><br>\n')
            yield('</body></html>\n')
            #led_status.blink(conf.status_pin, n_times=10, delay=0.2)
        return content()
    upload_logs.exposed=True
    upload_logs._cp_config = {'response.stream': True}
   
    def rsync_logs(self):
        if self.aws_host_ip is None:
            raise cherrypy.HTTPRedirect('/manage_ec2')
        cherrypy.response.headers["Content-Type"] = 'text/html'
        def content():
            commands = []

            #sync with delete in case we made edits
            command = 'rsync -e "ssh -oStrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress -ur --delete ../log/* %s@%s:~/shark/log' % (self.aws_host_username, self.aws_host_ip)
            commands.append(command)

            yield(self.stream_page(title="Shark Log Sync"))
            yield('<pre>')
            for com in commands:                
                cmd_filename = "../scripts/command"
                command_file = open(cmd_filename, "wt")
                yield(com + "\n\n")
                command_file.write(com)
                command_file.close()
                p = Proc(cmd_filename, shell=True)
                output, complete = p.poll()
                yield output
                while not complete:
                    output, complete = p.poll()
                    yield (output)
            yield('</pre>')
            yield(self.home_link())
            yield('<br><a href="/manage_train">train</a><br>\n')
            yield('</body></html>\n')
        return content()
    rsync_logs.exposed=True
    rsync_logs._cp_config = {'response.stream': True}

    def start_train(self):
        #ssh ubuntu@$AWS_HOST 'source .rem; cd shark; python train.py mymodel;'
        #scp ubuntu@$AWS_HOST:~/shark/mymodel .
        #scp ubuntu@$AWS_HOST:~/shark/*.png .
        cherrypy.response.headers["Content-Type"] = 'text/html'
        #use this local function as a generator to stream the output of the training.
        def content():

            commands = []
            path, model_name = os.path.split(self.model_file)

            local = self.aws_host_ip == "localhost"

            if local:
                #confirm training location
                command = 'echo "training locally"'
                commands.append(command)

                #kick off training
                command = "cd ..;python train.py ./models/%s --epochs %d;" % ( model_name, self.train_epochs)
                commands.append(command)

                #mv training graph
                command = "mv ../loss.png ./img/"
                commands.append(command)
                
            else:
                #kick off training
                command = "ssh -oStrictHostKeyChecking=no %s@%s '%s; cd shark; python train.py %s --epochs %d;'" % (self.aws_host_username, self.aws_host_ip, self.source_command, model_name, self.train_epochs)
                commands.append(command)

                #copy result model
                command = "scp %s@%s:~/shark/%s ../models/" % (self.aws_host_username, self.aws_host_ip, model_name)
                commands.append(command)

                #copy training graph
                command = "scp %s@%s:~/shark/loss.png ./img/" % (self.aws_host_username, self.aws_host_ip)
                commands.append(command)
                
            yield(self.stream_page(title="Shark Training"))
            yield('<pre>')
            for com in commands:                
                cmd_filename = "../scripts/command"
                command_file = open(cmd_filename, "wt")
                command_file.write(com)
                command_file.close()
                yield(com + "\n\n")
                p = Proc(cmd_filename, shell=True)
                output, complete = p.poll()                
                yield output
                while not complete:                    
                    output, complete = p.poll()
                    #get just the last 100 characters of the update lines.
                    if output.find('val_loss') != -1:
                        output = output[-100:]
                    yield (output)
                yield('\n\n')
            yield('</pre>')
            yield('<img src="/img/loss.png"></img>')
            yield(self.home_link())
            yield('<a href="/start_train">re-train</a><br>')
            yield('</body></html>\n')
            #led_status.blink(conf.status_pin, n_times=10, delay=0.2)

        return content()
    start_train.exposed = True
    start_train._cp_config = {'response.stream': True}

    def check_train(self):
        res = []
        res.append('<img src="/img/loss.png"></img>')
        res.append(self.home_link())
        res.append('<a href="/manage_train">train</a><br>')
        return self.easy_page("".join(res))
    check_train.exposed = True


if __name__ == "__main__":
    conf.init('../config.json')
    # Define the global configuration settings of CherryPy
    global_conf = {
        'global': {
            'server.socket_host': '0.0.0.0',
            'server.socket_port': conf.web_app_server_port,
            'server.protocol_version': 'HTTP/1.1'
        }
    }
    current_dir = os.path.dirname(os.path.abspath(__file__))
    print("current_dir ",current_dir)
    application_conf = {
        '/': {'tools.staticdir.root': current_dir},
        '/favicon.ico': {
            'tools.staticfile.on': True,
            'tools.staticfile.filename': os.path.join(current_dir, 'static/favicon.ico')
            },
        '/static/css': {
            'tools.gzip.on': True,
            'tools.gzip.mime_types':['text/css'],
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static/css',
        },
        '/static/js': {
            'tools.gzip.on': True,
            'tools.gzip.mime_types':['application/javascript'],
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static/js',
        },
        '/img': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'img',
        },
    }

    # Update the global CherryPy configuration
    cherrypy.config.update(global_conf)

    # Start the CherryPy HTTP server
    cherrypy.quickstart(WebSite(), config = application_conf)
