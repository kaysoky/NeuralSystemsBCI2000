#!/usr/bin/python

import os
import sys
import random
import time
import thread
import Tkinter, tkSimpleDialog

import pygame
from pygame.locals import *
pygame.mixer.init()

import numpy as np

from udpcontrol import UdpControl
from gridgame import ImageSurf, GridGame

__file__ = os.path.abspath(sys.argv[0])
os.chdir(os.path.dirname(__file__))

def play(fname):
    if fname == None:
        return
    pygame.mixer.music.load(fname)
    time.sleep(0.1)
    while not pygame.mixer.music.get_busy():
        pygame.mixer.music.play()
        time.sleep(0.1)

class MyDialog(tkSimpleDialog.Dialog):

    ROOT = Tkinter.Tk()
    ROOT.withdraw()
    ROOT.title('Message')

    def __init__(self, message):
        self.message = message
        tkSimpleDialog.Dialog.__init__(self, self.ROOT)

    def body(self, master):
        self.textbox = Tkinter.Text(master)
        self.textbox.insert(Tkinter.INSERT, self.message)
        self.textbox.config(state = Tkinter.DISABLED)
        self.textbox.grid()
        return self.textbox

    def apply(self):
        pass

class WhackaMole(GridGame):

    def init(self, mole = None, hole = None, hammer = None, moles = -1,
        hit_music = None, miss_music = None, win_music = None, cheat = False):
        self.t1 = time.time()
        if mole == None:
            self.mole = os.path.join(os.path.dirname(__file__), 'mole1.jpg')
        else:
            self.mole = mole
        self.mole = ImageSurf(self.mole)
        if hole == None:
            self.hole = os.path.join(os.path.dirname(__file__), 'hole1.jpg')
        else:
            self.hole = hole
        self.hole = ImageSurf(self.hole)
        if hammer == None:
            self.hammer = os.path.join(os.path.dirname(__file__), 'hammer1.png')
        else:
            self.hammer = hammer
        self.hammer = ImageSurf(self.hammer)
        self.hit_music = hit_music
        self.miss_music = miss_music
        self.win_music = win_music
        self.cheat = cheat
        for row in range(self.rows):
            for col in range(self.cols):
                im = ImageSurf(self.hole, self.getShape(row, col))
                self.setImages(im, self.invertImage(im), row, col)
        if moles < 0 or moles > self.rows * self.cols:
            moles = self.rows * self.cols
        self.mole_count = moles
        self.mole_locs = []
        self.prng = random.Random()
        self.prng.seed(0)
        self.randomizeMoles()

    def win(self, *args, **kwds):
        play(self.win_music)
        GridGame.win(self, *args, **kwds)

    def invertImage(self, im):
        im = pygame.Surface(im.get_size())
        im.fill((20, 20, 20))
        rect = im.get_rect()
        sub_rect = pygame.Rect(rect)
        sub_rect.width //= 4
        sub_rect.height //= 4
        sub_rect.left = (rect.width - sub_rect.width) // 2
        sub_rect.top = (rect.height - sub_rect.height) // 2
        im.subsurface(sub_rect).fill((220, 220, 220))
        return im

    def randomizeMoles(self):
        if self.mole_count == 0:
            self.win()
            #thread.start_new_thread(self.win, ())
            #MyDialog('Time: %f' % (time.time() - self.t1))
            return
        indices = self.prng.sample(
            xrange(self.rows * self.cols),
            self.mole_count
        )
        clock = pygame.time.Clock()
        for row, col in self.mole_locs:
            clock.tick(self.mole_count * 2)
            im = ImageSurf(self.hole, self.getShape(row, col))
            self.setImages(im, self.invertImage(im), row, col)
        self.mole_locs = []
        for index in indices:
            clock.tick(self.mole_count * 2)
            row, col = index % self.rows, index // self.rows
            self.mole_locs.append((row, col))
            im = ImageSurf(self.mole, self.getShape(row, col))
            self.setImages(im, self.invertImage(im), row, col)

    def hit(self, row, col):
        im = ImageSurf(self.hammer, self.getShape(row, col))
        im.set_colorkey((0, 0, 0))
        nonflash, flash = self.getImages(row, col)
        nonflash.blit(im, (0, 0))
        flash.blit(im, (0, 0))
        self.setImages(nonflash, flash, row, col)
        if (row, col) in self.mole_locs:
            play(self.hit_music)
            self.mole_locs.remove((row, col))
            self.mole_count -= 1
            return True
        if self.cheat and random.randrange(2) == 0:
            play(self.hit_music)
            row, col = self.mole_locs.pop(0)
            im = ImageSurf(self.hole, self.getShape(row, col))
            self.setImages(im, self.invertImage(im), row, col)
            self.mole_count -= 1
            return True
        play(self.miss_music)
        return False

    def withdraw(self, row, col):
        if (row, col) in self.mole_locs:
            im = ImageSurf(self.mole, self.getShape(row, col))
        else:
            im = ImageSurf(self.hole, self.getShape(row, col))
        self.setImages(im, self.invertImage(im), row, col)

def eventloop():
    while True:
        events = [pygame.event.wait()] + pygame.event.get()
        for event in events:
            if event.type == QUIT:
                raise SystemExit
            if event.type == KEYDOWN and event.key == K_ESCAPE:
                raise SystemExit

def findP3SpellerWindow():
    try:
        import win32gui, pywintypes
    except ImportError:
        return None, None
    while True:
        try:
            try:
                p3speller = win32gui.FindWindow('TDisplayForm', '')
                break
            except pywintypes.error:
                time.sleep(0.2)
        except KeyboardInterrupt:
            return None, None
    wleft, wtop, wright, wbottom = win32gui.GetWindowRect(p3speller)
    return (wleft, wtop), (wright - wleft, wbottom - wtop)

def editPrmFile(rows, cols):
    output = ''
    fname = os.path.join(os.path.dirname(__file__), 'whackamole.prm')
    for line in open(fname, 'rb'):
        if line.startswith('Application:Speller%20Targets:P3SpellerTask mat' + \
            'rix TargetDefinitions= '):
            continue
        if line.startswith('Application:Speller%20Targets:P3SpellerTask int' + \
            'list NumMatrixColumns= '):
            continue
        if line.startswith('Application:Speller%20Targets:P3SpellerTask int' + \
            'list NumMatrixRows= '):
            continue
        output += line
    output += 'Application:Speller%20Targets:P3SpellerTask matrix TargetDef' + \
        'initions= %d 5 ' % (rows * cols)
    for i in range(1, rows * cols + 1):
        output += '%d %d 1 %% %% ' % (i, i)
    output += '// speller target properties\r\n'
    output += 'Application:Speller%20Targets:P3SpellerTask intlist NumMatri' + \
        'xColumns= 1 %d %d 1 %% // display matrices\' column number(s)\r\n' % \
        (cols, cols)
    output += 'Application:Speller%20Targets:P3SpellerTask intlist NumMatri' + \
        'xRows= 1 %d %d 0 %% // display matrices\' row number(s)\r\n' % \
        (rows, rows)
    open(fname, 'wb').write(output)

def main(argv = []):
    rows = 6
    cols = 6
    moles = -1
    if 'cheat' in argv:
        cheat = True
        argv.remove('cheat')
    else:
        cheat = False
    if len(argv) >= 1 and 'x' in argv[0].lower():
        dims = argv.pop(0).lower().split('x')
        argv.insert(0, dims[1])
        argv.insert(0, dims[0])
    if len(argv) >= 2 and argv[0].isdigit() and argv[1].isdigit():
        rows = int(argv[0])
        cols = int(argv[1])
    if len(argv) >= 3 and argv[2].isdigit():
        moles = int(argv[2])
    editPrmFile(rows, cols)
    loc, shape = findP3SpellerWindow()
    if loc == None:
        loc = (1400, 150)
    if shape == None:
        shape = (800, 700)
    game = WhackaMole(rows = rows, cols = cols, shape = shape,
        border = 3, moles = moles, loc = loc, cheat = cheat,
        hit_music = os.path.join(os.path.dirname(__file__), 'hit.ogg'),
        miss_music = os.path.join(os.path.dirname(__file__), 'miss.ogg'),
        win_music = os.path.join(os.path.dirname(__file__), 'win.ogg')
    )
    clock = pygame.time.Clock()
    def on_func(game, spots):
        if game.won:
            return
        game.stimuli.ravel()[spots] = True
        game.prepare()
    def off_func(game, spots):
        if game.won:
            return
        game.stimuli.ravel()[spots] = False
        game.prepare()
    def select_func(game, selection):
        if game.won:
            return
        col = selection % game.cols
        row = selection // game.cols
        hit = game.hit(row, col)
        time.sleep(2)
        game.withdraw(row, col)
        if hit:
            game.randomizeMoles()
    udpcontrol = UdpControl(on_func, (game,), off_func, (game,),
        game.flip, (), select_func, (game,))
    thread.start_new_thread(udpcontrol.mainloop, ())
    eventloop()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
