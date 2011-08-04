#!/usr/bin/python

import sys
import os
import time
import subprocess

__file__ = os.path.abspath(sys.argv[0])
this_dir = os.path.dirname(__file__)

from Tkinter import *

from win32process import GetWindowThreadProcessId
from win32gui import EnumWindows

class SplashScreen(Frame):

    def __init__(self, master, image):
        Frame.__init__(self, None)
        self.pack(side = TOP, fill = BOTH, expand = YES)

        self.image = PhotoImage(file = image)
        width = self.image.width()
        height = self.image.height()

        left = (self.master.winfo_screenwidth() - width) // 2
        top = (self.master.winfo_screenheight() - height) // 2

        self.master.geometry('%ix%i+%i+%i' % (width, height, left, top))

        self.master.overrideredirect(True)

        Label(self, image = self.image, ).pack(side = TOP, expand = YES)

        self.lift()

SPLASHSCREEN = None
def Splash(image):
    global SPLASHSCREEN
    SPLASHSCREEN = Tk()
    SplashScreen(SPLASHSCREEN, image = image)
    SPLASHSCREEN.update()

def UnSplash():
    global SPLASHSCREEN
    if SPLASHSCREEN != None:
        SPLASHSCREEN.destroy()
        SPLASHSCREEN = None

def ListWindows():
    windowlist = []
    EnumWindows((lambda handle, windowlist: windowlist.append(handle) or True),
        windowlist)
    return windowlist

def HasWindow(pid):
    for window in ListWindows():
        if GetWindowThreadProcessId(window)[1] == pid:
            return True
    return False

def WaitWindow(pid):
    while not HasWindow(pid):
        time.sleep(0.05)

def main(argv = []):
    os.chdir(this_dir)
    Splash('logo.gif')
    process = subprocess.Popen(argv[0])
    WaitWindow(process.pid)
    UnSplash()
    process.wait()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
