from __future__ import print_function
import os
import sys
import time
import glob
import argparse
import json
from threading import Thread
import traceback
import zmq

class SyncSource(object):
    def __init__(self, _address, _path, dest_address, prog_address, verbose=False):
        self.address = _address
        self.dest_address = dest_address
        self.verbose = verbose
        self.path = _path
        self.path_poll_freq = 3.0 #seconds
        self.last_local_refresh = 0
        self.last_remote_refresh = 0
        self.local_files = {}
        self.dest_files = {}
        self.files_to_remove = {}
        self.files_to_sync = {}
        self.errors = []

        '''
        Start the server
        '''
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        connect_str = "tcp://%s:%s" % ('*', self.address[1])
        print('sync listening on', connect_str)
        rc = self.socket.bind(connect_str)

        self.dest_socket = self.context.socket(zmq.REQ)
        connect_str = "tcp://%s:%s" % (self.dest_address[0], self.dest_address[1])
        print('dest: ', connect_str)
        rc = self.dest_socket.connect(connect_str)

        if self.verbose:
            print("connect err:", rc)

        self.prog_thread = None
        #self.prog_thread = Thread(target=self.run_prog, args=(prog_address,))
        #self.prog_thread.daemon = True
        #self.prog_thread.start()

    def refresh_local_files(self):
        if self.verbose:
             print("listing local files")
        files = glob.glob(os.path.join(self.path, '*'))
        if self.verbose:
             print("sorting local files")
        files.sort()
        if self.verbose:
             print("building dict of local files")
        for f in files:
            rem_filename = f[len(self.path) + 1 : ]
            self.local_files[rem_filename] = rem_filename
            self.files_to_sync[rem_filename] = rem_filename
        
        self.last_local_refresh = time.time()
        if self.verbose:
            print("found %d files locally" % len(self.local_files.keys()))


    def refresh_remote_files(self):
        command = b"get_file_list"
        args = b'none'
        payload = b'none'
        
        if self.verbose:
            print("refreshing remote")

        #request file list
        self.dest_socket.send_multipart([command, args, payload])

        #get reply
        reply, args, payload = self.dest_socket.recv_multipart()

        if reply == "file_list":
            jsonObj = json.loads(payload)
            destArr = jsonObj['files']
            for f in destArr:
                self.dest_files[f] = 1
            if self.verbose:
                print("found %d files on remote" % len(self.dest_files.keys()))
        elif reply == 'failed':
            self.errors.append("failed to get remote file list")
            print("failed to get remote file list")
            self.errors.append(args)
            self.dest_files = {}
        
        self.last_remote_refresh = time.time()


    def send_file(self, filename):
        command = b"new_file"        
        args = filename
        try:
            fullpath = os.path.join(self.path, filename)
            payload = open(fullpath, "rb").read()
        except:
            err = traceback.format_exc()
            self.errors.append('failed to open file: %s' % fullpath)
            print('failed to open file: %s' % fullpath)
            self.errors.append(err)
            return False

        #send file
        self.dest_socket.send_multipart([command, args, payload])

        #get reply
        reply, args, payload = self.dest_socket.recv_multipart()

        if reply == 'failed':
            self.errors.append("sent %s failed" % filename)
            print("sent %s failed" % filename)
            self.errors.append(args)
        
        return reply == "ok"

    def remote_remove(self, filename):
        command = b"rm_file"        
        args = filename
        payload = 'none'

        #send command
        self.dest_socket.send_multipart([command, args, payload])

        #get reply
        reply, args, payload = self.dest_socket.recv_multipart()

        if reply == 'failed':
            self.errors.append("rm %s failed" % filename)
            print("rm %s failed" % filename)
            self.errors.append(args)

        return reply == "ok"

    def get_report(self):
        report = { "local" : len(self.local_files.keys()), \
                "queued" : len(self.files_to_sync.keys()), \
                "remote" : len(self.dest_files.keys()), \
                "remove" : len(self.files_to_remove).keys(), \
                "errors" : self.errors }

        #reset errors after reporting
        if len(self.errors) > 0:
            self.errors = []

        return report

    def run_prog(self, prog_address):
        context = zmq.Context()
        self.prog_socket = context.socket(zmq.REP)
        connect_str = "tcp://%s:%s" % ('*', prog_address[1])
        print('progress listening on', connect_str)
        rc = self.prog_socket.bind(connect_str)
        self.poller = zmq.Poller()
        self.poller.register(self.prog_socket, zmq.POLLIN)
        while(True):
            #print("waiting for a progress req")
            command = self.prog_socket.recv()

            reply = "ok"
            try:
                if command == "report":
                    reply = json.dumps(self.get_report())
                else:
                    print("what?", command)
            except:
                self.errors.append(traceback.format_exc())
                reply = 'failed'

            self.prog_socket.send(reply)

    def prune(self):
        if self.verbose:
            print("starting prune")
        '''
        for files we plan to sync that are already on the dest, remove from queued
        '''
        for k, v in self.dest_files.iteritems():
            if k in self.files_to_sync:
                del self.files_to_sync[k]
        '''
        for files on remote but not local, add to list to delete
        '''
        self.files_to_remove = {}
        #for k, v in self.dest_files.iteritems():
        #    if k not in self.local_files:
        #        self.files_to_remove[k] = v
        if self.verbose:
            print("done w prune")
        
        
    def run(self):
        self.refresh_local_files()
        self.refresh_remote_files()
        self.prune()

        while True:
            if len(self.files_to_sync.keys()) > 0:
                k = self.files_to_sync.keys()[0]
                f = self.files_to_sync[k]
                if f in self.dest_files.keys():
                    del self.files_to_sync[k]
                elif self.send_file(f):
                    if self.verbose:
                        print("remain: %d sent %s" % (len(self.files_to_sync.keys()), f))
                    del self.files_to_sync[k]
            elif len(self.files_to_remove.keys()) > 0:
                k = self.files_to_remove.keys()[0]
                f = self.files_to_remove[k]
                if self.remote_remove(f):
                    del self.files_to_remove[k]
            elif len(self.files_to_sync.keys()) == 0:
                elapsed = time.time() - self.last_local_refresh
                if elapsed > self.path_poll_freq:
                    self.refresh_local_files()
                    self.refresh_remote_files()
                    self.prune()


class SyncDest(object):
    def __init__(self, _address, _path):
        self.path = _path
        self.local_files = []
        
        '''
        Start the server
        '''
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        connect_str = "tcp://%s:%s" % ('*', address[1])
        print('sync listening on', connect_str)
        rc = self.socket.bind(connect_str)
        

    def refresh_local_files(self):
        files = glob.glob(os.path.join(self.path, '*'))
        files.sort()
        for f in files:
            rem_filename = f[len(self.path) + 1 : ]
            if rem_filename not in self.local_files:
                    self.local_files.append(rem_filename) 


    def run(self):
        breadcrumb = open(os.path.join(self.path, 'sync_listening.pid'), "wt")
        breadcrumb.write('%d' % os.getpid())
        breadcrumb.close()

        while(True):
            command, args, payload = self.socket.recv_multipart()

            if(command == 'get_file_list'):
                try:
                    self.refresh_local_files()
                    reply = 'file_list'
                    args = 'none'
                except:
                    reply = 'failed'
                    args = traceback.format_exc()
                    self.local_files = []
                payload = json.dumps({ "files" : self.local_files})
                self.socket.send_multipart([reply, args, payload])
            elif(command == 'new_file'):
                try:
                    filename = os.path.join(self.path, args)
                    #print('creating new file', filename)
                    outfile = open(filename, "wb")
                    outfile.write(payload)
                    outfile.close()
                    reply = 'ok'
                    args = 'none'
                except:
                    reply = 'failed'
                    args = traceback.format_exc()
                payload = 'none'
                self.socket.send_multipart([reply, args, payload])
            elif(command == 'rm_file'):
                try:
                    filename = os.path.join(self.path, args)
                    os.unlink(filename)
                    reply = 'ok'
                    args = 'none'
                except:
                    reply = 'failed'
                    args = traceback.format_exc()
                payload = 'none'
                self.socket.send_multipart([reply, args, payload])



def go_src(address, path, dest_address, prog_address, verbose):
    print('running source sync', address)   
    s = SyncSource(address, path, dest_address, prog_address, verbose)
    s.run()

def go_dest(address, path):
    print('running dest sync', address)
    s = SyncDest(address, path)
    s.run()

def go_progress(address):
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    connect_str = "tcp://%s:%s" % (address[0], address[1])
    print('get progress from: ', connect_str)
    rc = socket.connect(connect_str)
    
    #request report
    socket.send("report")

    #get reply
    reply = socket.recv()

    print(reply)
    return reply


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='sync')
    parser.add_argument('--src', dest='src', action="store_true", help='are we the source of files')
    parser.add_argument('--dest', dest='dest', action="store_true", help='are we the dest of files')
    parser.add_argument('--progress', dest='progress', action="store_true", help='get a progress report')
    parser.add_argument('--path', type=str, default='.', help='path to files')
    parser.add_argument('--dest_ip', type=str, default='127.0.0.1', help='what ip for dest')
    parser.add_argument('--dest_port', type=int, default='9898', help='what port for dest')
    parser.add_argument('--src_port', type=int, default='9797', help='what port for src')    
    parser.add_argument('--prog_port', type=int, default='9696', help='what port for progress')
    parser.add_argument('--verbose', dest='verbose', action="store_true", help='do print progress')   
    args = parser.parse_args()

    if args.src:
        address = ('0.0.0.0', args.src_port)
        path = args.path
        dest_address = (args.dest_ip, args.dest_port)
        prog_address = ('0.0.0.0', args.prog_port)
        go_src(address, path, dest_address, prog_address, args.verbose)
    elif args.dest:
        address = ('0.0.0.0', args.dest_port)
        path = args.path
        go_dest(address, path)
    elif args.progress:
        address = ('pi.local', args.prog_port)
        go_progress(address)
