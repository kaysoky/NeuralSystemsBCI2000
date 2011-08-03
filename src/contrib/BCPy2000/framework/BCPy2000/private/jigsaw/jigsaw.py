#!/usr/bin/python

#----------------begin default parameters----------------#

window_x=1000 #800 #600 # window width in pixels
window_y=800 #600 #540 # window height in pixels
preplaced=18
spots="abcdefghijklmnopqrstuvwxyz1234567890,."
representations="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890<>"
flashspots=True
IMAGE_PATH="../images"
DEBUG=False

#-----------------end default parameters-----------------#

import pygame
from pygame.locals import *
from pygame import surfarray
from pygame import font
import numpy
import random
import os
import sys
import thread,socket
import time
import optparse

os.environ['SDL_VIDEODRIVER'] = 'windib'

__file__ = os.path.abspath(sys.argv[0])
os.chdir(os.path.dirname(__file__))

IMAGE_PATH=os.path.join(os.path.dirname(__file__),IMAGE_PATH)

parser=optparse.OptionParser()
parser.add_option("--pre-placed","-e",
                  action="store",
                  type="int",
                  dest="preplaced",
                  default=preplaced,
                  help="set the number of pre-placed pieces in the puzzle")
parser.add_option("--width","-x",
                  action="store",
                  type="int",
                  dest="width",
                  default=window_x,
                  help="set window width")
parser.add_option("--height","-y",
                  action="store",
                  type="int",
                  dest="height",
                  default=window_y,
                  help="set window height")
parser.add_option("--flash","-f",
                  action="store_true",
                  dest="flashspots",
                  default=flashspots,
                  help="set whether or not to flash the grid within the game")
parser.add_option("--noflash","-n",
                  action="store_false",
                  dest="flashspots",
                  default=flashspots,
                  help="set whether or not to flash the grid within the game")
parser.add_option("--image-path","-i",
                  action="store",
                  dest="imagepath",
                  default=IMAGE_PATH,
                  help="set the path in which to look for images")
parser.add_option("--profile","-p",
                  action="store_true",
                  dest="debug",
                  default=DEBUG,
                  help="tell the program to profile itself; not for normal use")
parser.add_option("--map","-m",
                  action="store",
                  dest="spots",
                  default=spots,
                  help="set the grid mapping; should not be altered")
parser.add_option("--representations","-r",
                  action="store",
                  dest="representations",
                  default=representations,
                  help="set the grid representations of characters; should not be altered")
options,remainder=parser.parse_args()

window_x=options.width
window_y=options.height
preplaced=options.preplaced
spots=list(options.spots)
representations=list(options.representations)
flashspots=options.flashspots
IMAGE_PATH=options.imagepath
DEBUG=options.debug
PREV=spots[-2]
NEXT=spots[-1]
NEW_IMAGE=-1

if DEBUG:
    print window_x
    print window_y
    print spots
    print representations
    print flashspots
    print IMAGE_PATH
    print DEBUG
    print PREV
    print NEXT

chop_width=window_x/10
chop_height=window_y/10
image_width=chop_width*6
image_height=chop_height*6
border=(chop_width+chop_height)/2/14

font.init()
pygame.display.init()
#print pygame.display.Info()
screen=pygame.display.set_mode((window_x,window_y))
pygame.display.set_caption("P300 Jigsaw")

tileplaces=[]
for y in range(0,image_height,chop_height):
    for x in range(0,image_width,chop_width):
        tileplaces.append((chop_width+x,chop_height+y))
tileplaces.append((chop_width,0))
tileplaces.append((6*chop_width,0))

storage=[]
for top_left_x in range(0,chop_width*8,chop_width):
    for top_left_y in [chop_height*8,chop_height*9]:
        storage.append((top_left_x,top_left_y))
for top_left_x in [chop_width*8,chop_width*9]:
    for top_left_y in range(0,chop_height*10,chop_height):
        storage.append((top_left_x,top_left_y))

def randomize(l):
    ret=l[:]
    random.shuffle(ret)
    return ret

def draw_outline(surface,color=(0,0,0),bwidth=1):
    top_left=(0,0)
    top_right=(surface.get_width()-1,0)
    bottom_left=(0,surface.get_height()-1)
    bottom_right=(surface.get_width()-1,
                  surface.get_height()-1)
    pygame.draw.polygon(surface,color,
                        [top_left,bottom_left,bottom_right,top_right],2*bwidth)

def get_blank_surface(width=chop_width,height=chop_height,color=[0,0,0],
                      letter=None,lcolor=[255,255,255],fontsize=None):
    if fontsize==None:
        fontsize=int(height/2.5)
    blank_surface=pygame.Surface((width,height))
    blank_surface.fill(color)
    if letter!=None:
        f=font.Font(font.get_default_font(),fontsize)
        fwidth,fheight=f.size(letter)
        rendered=f.render(letter,True,lcolor)
        blank_surface.blit(rendered,((width-fwidth)/2,(height-fheight)/2),
                           (0,0,fwidth,fheight))
    return blank_surface

def get_placeholder(color=(255,255,255),letter=None):
    placeholder=get_blank_surface(letter=letter)
    draw_outline(placeholder,color)
    return placeholder

def copysquare(surface,x,y,width,height):
    chop_surface=pygame.surface.Surface((width,height))
    chop_border=(x,y,width,height)
    chop_surface.blit(surface,(0,0),chop_border)
    return chop_surface

def copy(surface):
    width=surface.get_width()
    height=surface.get_height()
    chop_surface=pygame.surface.Surface((width,height))
    chop_border=(0,0,width,height)
    chop_surface.blit(surface,(0,0),chop_border)
    return chop_surface

def invert(surface):
    arr=surfarray.array2d(surface)
    arr=(1<<24)-1-arr
    surfarray.blit_array(surface,arr)

def udpkeyboard():
    ##os.system("python P3spellerproxy.py")
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    ##sock.bind(("",30001))
    sock.bind(("",19711))
    while True:
        data=sock.recv(64)
        print data # needed in non-DEBUG to make sure BCI2000 is still sending
            # selected keys to this port -- sometimes it stops for no reason
        data=data.lower()[len("P3Speller_Output "):].strip()
        if not data:
            continue
        fakeevent=pygame.event.Event(KEYDOWN,{"unicode":unicode(data),
                                              "key":ord(data[0])})
        pygame.event.post(fakeevent)
thread.start_new_thread(udpkeyboard,())

def getblipspots(squares):
    ret=[]
    for square in squares:
        if len(tileplaces)<=square:
            continue
        x,y=tileplaces[square]
        x+=chop_width/4
        y+=chop_height/4
        ret.append((x,y,chop_width/2,chop_height/2))
    return ret

def blip(spots):
    for spot in spots:
        copy=copysquare(screen, *spot)
        invert(copy)
        screen.blit(copy, spot[:2])

def disruptflashes():
    sock=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto("Autoflash 1", ("127.0.0.1", 12000))
    sock.sendto("Conceal", ("127.0.0.1", 12000))

def udpflashes():
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    sock.bind(('', 12000))
    sock.settimeout(5)
    data = None
    while True:
        try:
            spots = getblipspots([int(a) - 1 for a in data.split(',')])
        except:
            try:
                data=sock.recv(4096)
                continue
            except socket.timeout:
                continue
        displaylock.acquire()
        blip(spots)
        while not data.startswith("Autoflash 1"):
            try:
                data = sock.recv(4096)
            except socket.timeout:
                continue
        pygame.display.update(spots)
        blip(spots)
        while not data.startswith('Conceal'):
            try:
                data = sock.recv(4096)
            except socket.timeout:
                continue
        pygame.display.update(spots)
        displaylock.release()
        data = None
displaylock = thread.allocate_lock()
if flashspots:
    thread.start_new_thread(udpflashes,())

def setstatus(words):
    statusbar=get_blank_surface(width=4*chop_width,letter=words,
                                fontsize=int(chop_height/2.8))
    draw_outline(statusbar,(255,255,255))
    screen.blit(statusbar,(2*chop_width,0))
    pygame.display.update((2*chop_width,0,statusbar.get_width(),statusbar.get_height()))

def toswitch(reset=False):
    if reset:
        global to_switch
        to_switch=3
    if to_switch<1:
        return True
    setstatus("select \"<\" %i times to switch"%to_switch)
    to_switch-=1
    return False

def feedback(letter = None,correctness = None):
    lcolor = (0, 200, 0) if correctness else (200, 0, 0)
    if correctness == None:
        lcolor = (255, 255, 255)
    displayletter=get_blank_surface(letter = letter, lcolor = lcolor)
    screen.blit(displayletter, (0, 0))
    pygame.display.update((0, 0), (chop_width, chop_height))

def input(event,curr_tile,curr_loc):
    #print event
    if event.type == QUIT:
        raise SystemExit
    elif event.type == KEYDOWN:
        #print event
        #print event.unicode
        if event.key == K_ESCAPE:
            raise SystemExit
        if event.unicode == PREV:
            if toswitch():
                return NEW_IMAGE
        else:
            toswitch(reset = True)
        if event.unicode in spots:
            #print "in spots"
            choice = spots.index(event.unicode)
            if event.unicode in (PREV, NEXT):
                #print "prev,next"
                feedback(representations[choice], None)
                return event.unicode
            spot=tiles.index(curr_tile)
            if spot==choice:
                #print "correct"
                feedback(representations[choice], True)
                blank=get_blank_surface()
                screen.blit(blank, curr_loc)
                pygame.display.update(curr_loc, (chop_width, chop_height))
                for i in range(10):
                    if displaylock.locked():
                        time.sleep(.1)
                while displaylock.locked():
                    time.sleep(.1)
                    disruptflashes()
                displaylock.acquire()
                screen.blit(curr_tile, tileplaces[spot])
                pygame.display.update()
                #pygame.display.flip()
                displaylock.release()
                return True
            else:
                #print "incorrect"
                feedback(representations[choice], False)
        #else:
        #    #print "invalid"
        #    feedback(event.unicode, False)
    return False

def main():
    global tiles
    pygame.event.set_allowed([QUIT,KEYDOWN])
    pygame.event.set_blocked([KEYUP,MOUSEBUTTONDOWN,MOUSEBUTTONUP,MOUSEMOTION,
                              NOEVENT,USEREVENT,VIDEOEXPOSE,VIDEORESIZE])
    while True:
        image=random.choice(os.listdir(IMAGE_PATH))
        while True:
            try:
                surface=pygame.image.load(os.path.join(IMAGE_PATH,image))
            except pygame.error:
                continue
            break
        surface=pygame.transform.scale(surface,(image_width,image_height))

        while displaylock.locked():
            time.sleep(.1)
            disruptflashes()
        displaylock.acquire()
        tiles=[]
        for place in range(len(tileplaces)):
            x,y=tileplaces[place]
            chop_surface=copysquare(surface,x-chop_width,y-chop_height,
                                    chop_width,chop_height)
            draw_outline(chop_surface)
            screen.blit(get_placeholder(letter=representations[place]),(x,y))
            if place < 36:
                tiles.append(chop_surface)

        strg=storage[:]
        rtiles=randomize(tiles)
        for tile in rtiles:
            screen.blit(tile,strg.pop(0))

        strg=storage[:]
        for i in range(preplaced):
            which=random.randrange(len(rtiles))
            curr_tile=rtiles.pop(which)
            curr_loc=strg.pop(which)
            spot=tiles.index(curr_tile)
            blank=get_blank_surface()
            screen.blit(blank,curr_loc)
            screen.blit(curr_tile,tileplaces[spot])
        curr_tile=rtiles.pop(0)
        curr_loc=strg.pop(0)
        curr_copy=copy(curr_tile)
        draw_outline(curr_copy,(200,0,0),border)
        screen.blit(curr_copy,curr_loc)
        toswitch(reset=True)
        pygame.display.flip()
        displaylock.release()
        while True:
            result=input(pygame.event.wait(),curr_tile,curr_loc)
            if result==NEXT:
                screen.blit(curr_tile,curr_loc)
                pygame.display.update(curr_loc,(chop_width,chop_height))
                rtiles.append(curr_tile)
                strg.append(curr_loc)
                result=True
            elif result==PREV:
                screen.blit(curr_tile,curr_loc)
                pygame.display.update(curr_loc,(chop_width,chop_height))
                rtiles.insert(0,curr_tile)
                rtiles.insert(0,rtiles.pop(-1))
                strg.insert(0,curr_loc)
                strg.insert(0,strg.pop(-1))
                result=True
            elif result==NEW_IMAGE:
                break
            if result:
                if len(rtiles)==0 or len(strg)==0:
                    time.sleep(2)
                    break
                curr_tile=rtiles.pop(0)
                curr_copy=copy(curr_tile)
                curr_loc=strg.pop(0)
                draw_outline(curr_copy,(200,0,0),border)
                screen.blit(curr_copy,curr_loc)
                pygame.display.update(curr_loc,(chop_width,chop_height))

if __name__=="__main__":
    if DEBUG:
        import profile,pstats
        profile.run("main()","profile")
        stats=pstats.Stats("profile")
        stats.sort_stats("cumulative")
        stats.print_stats()
    else:
        main()

