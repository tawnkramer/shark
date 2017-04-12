from __future__ import print_function
import os
import sys
import time
import glob
import argparse
import json
import zmq

class SyncSource(object):
    def __init__(self, _address, _path, dest_address):
        self.address = _address
        self.dest_address = dest_address
        self.path = _path
        self.path_poll_freq = 3.0 #seconds
        self.last_local_refresh = 0
        self.last_remote_refresh = 0
        self.local_files = []
        self.dest_files = []
        self.files_to_remove = []
        self.files_to_sync = []

        '''
        Start the server
        '''
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        connect_str = "tcp://%s:%s" % ('*', self.address[1])
        print('sync listening on', connect_str)
        rc = self.socket.bind(connect_str)

        self.dest_socket = self.context.socket(zmq.REQ)
        connect_str = "tcp://%s:%s" % (self.dest_address[0], self.dest_address[0])
        print('dest: ', connect_str)
        rc = self.dest_socket.connect(connect_str)
        
        self.poller = zmq.Poller()
        self.poller.register(self.socket, zmq.POLLIN)
        self.poller.register(self.dest_socket, zmq.POLLIN)

    def refresh_local_files(self):
        files = glob.glob(os.path.join(self.path, '*'))
        if len(self.local_files) == 0:
            self.local_files = files
            self.files_to_sync = files
        else:
            for f in files:
                if f not in self.local_files:
                    self.local_files.append(f)
                    self.files_to_sync.append(f)
            for f in self.local_files:
                if f not in files:
                    self.files_to_remove.append(f)
            for f in self.files_to_remove:
                self.local_files.remove(f)

        self.last_local_refresh = time.time()


    def refresh_remote_files(self):
        command = b"get_file_list"
        args = b'none'
        payload = b'none'
        
        #request file list
        self.dest_socket.send_multipart([command, args, payload])

        #get reply
        reply, args, payload = self.dest_socket.recv_multipart()

        if reply == "file_list":
            jsonObj = json.loads(payload)
            self.dest_files = jsonObj['files']
            print("remote has: ", self.dest_files)
        else:
            print("didn't recognize:", reply)
        
        self.last_remote_refresh = time.time()


    def send_file(self, filename):
        command = b"new_file"
        args = filename
        payload = open(filename, "rb").read()
        
        #send file
        self.dest_socket.send_multipart([command, args, payload])

        #get reply
        reply, args, payload = self.dest_socket.recv_multipart()

        return reply == "ok"


    def run(self):
        self.refresh_local_files()
        self.refresh_remote_files()

        while True:

            if len(self.files_to_sync) > 0:
                f = self.files_to_sync[0]
                if self.send_file(f):
                    self.files_to_sync.remove(f)



class SyncDest(object):
    def __init__(self, _address, _path):
        self.address = _address
        self.path = _path
        self.path_poll_freq = 3.0 #seconds
        self.last_local_refresh = 0
        self.last_remote_refresh = 0
        self.local_files = []
        
        '''
        Start the server
        '''
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        connect_str = "tcp://%s:%s" % ('*', self.address[1])
        print('sync listening on', connect_str)
        rc = self.socket.bind(connect_str)
        

    def refresh_local_files(self):
        files = glob.glob(os.path.join(self.path, '*'))
        if len(self.local_files) == 0:
            self.local_files = files
        else:
            for f in files:
                if f not in self.local_files:
                    self.local_files.append(f)        
        self.last_local_refresh = time.time()

    def run(self):
        command, args, payload = self.socket.recv_multipart()

        if(command == 'get_file_list'):
            self.refresh_local_files()
            reply = 'file_list'
            args = 'none'
            payload = json.dumps({ "files" : self.local_files})
            self.socket.send_multipart([reply, args, payload])
        elif(command == 'new_file'):
            filename = os.path.join(self.path, args)
            outfile = open(filename, "wb")
            outfile.write(payload)
            outfile.close()
            reply = 'ok'
            args = 'none'
            payload = 'none'
            self.socket.send_multipart([reply, args, payload])
        elif(command == 'rm_file'):
            filename = os.path.join(self.path, args)
            os.unlink(filename)
            reply = 'ok'
            args = 'none'
            payload = 'none'
            self.socket.send_multipart([reply, args, payload])



def go(address, path, dest_address):
    s = SyncSource(address, path, dest_address)
    s.run()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='sync')
    parser.add_argument('model', type=str, help='model name')
    parser.add_argument('--test_speed', dest='test_speed', action="store_true", help='option to trigger profiling from local images')
    parser.set_defaults(test_speed=False)
    args = parser.parse_args()

    address = ('0.0.0.0', 9898)
    path = './log'
    dest_address = None
    go(address, path, dest_address)
