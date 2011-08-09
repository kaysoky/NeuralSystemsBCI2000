#!/usr/bin/python

from Tkinter import *

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
    from win32gui import EnumWindows, IsWindowVisible
    def accumulate(handle, windowlist):
        if IsWindowVisible(handle):
            windowlist.append(handle)
        return True
    windowlist = []
    EnumWindows(accumulate, windowlist)
    return windowlist

def HasWindow(pid):
    from win32process import GetWindowThreadProcessId
    for window in ListWindows():
        if GetWindowThreadProcessId(window)[1] == pid:
            return True
    return False

def WaitWindow(process):
    import time
    while not HasWindow(process.pid):
        if process.returncode != None:
            return process.returncode
        time.sleep(0.1)

def main(argv = []):
    import os
    Splash(os.path.abspath(argv[0]))
    import subprocess
    process = subprocess.Popen(argv[1:])
    WaitWindow(process)
    UnSplash()
    process.wait()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
