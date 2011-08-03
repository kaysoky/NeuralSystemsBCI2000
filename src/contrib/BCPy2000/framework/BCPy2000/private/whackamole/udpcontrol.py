#!/usr/bin/python

import socket
import thread

class UdpControl(object):

    def __init__(self, on_func, on_args, off_func, off_args,
        flip_func, flip_args, select_func, select_args,
        port = 12000, keyboard_port = 19711):
        self.on_func = on_func
        self.on_args = on_args
        self.off_func = off_func
        self.off_args = off_args
        self.flip_func = flip_func
        self.flip_args = flip_args
        self.select_func = select_func
        self.select_args = select_args
        self.port = port
        self.keyboard_port = keyboard_port

    def udpKeyboard(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('', self.keyboard_port))
        sock.settimeout(5)
        while True:
            try:
                data = sock.recv(4096)
                while not data.endswith('\n'):
                    try:
                        data += sock.recv(4096)
                    except socket.timeout:
                        continue
            except socket.timeout:
                continue
            try:
                selected = int(data[len("P3Speller_Output "):].strip()) - 1
            except:
                print 'Error: "%s" is invalid data ???' % data.strip()
            args = self.select_args + (selected,)
            self.select_func(*args)

    def mainloop(self):
        thread.start_new_thread(self.udpKeyboard, ())
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('', self.port))
        sock.settimeout(5)
        data = None
        while True:
            try:
                spots = [int(a) - 1 for a in data.split(',')]
            except:
                try:
                    data = sock.recv(4096)
                    continue
                except socket.timeout:
                    continue
            args = self.on_args + (spots,)
            self.on_func(*args)
            while not data.startswith('Autoflash 1'):
                try:
                    data = sock.recv(4096)
                except socket.timeout:
                    continue
            self.flip_func(*self.flip_args)
            args = self.off_args + (spots,)
            self.off_func(*args)
            while not data.startswith('Conceal'):
                try:
                    data = sock.recv(4096)
                except socket.timeout:
                    continue
            self.flip_func(*self.flip_args)
            data = None
