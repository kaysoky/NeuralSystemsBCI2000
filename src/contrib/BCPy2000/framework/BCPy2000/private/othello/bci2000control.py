#!/usr/bin/python

import pygame
from pygame.locals import *
from pygame import surfarray
import numpy
from pygame import font
import thread
import socket
import time

lets="ABCDEFGH"
nums="12345678"
LETTERS=[]
for i in range(8):
    for j in range(8):
        LETTERS.append(lets[i]+nums[j])

#def get_blank_surface(width=chop_width,height=chop_height,color=[0,0,0],
#                      letter=None,lcolor=[255,255,255],fontsize=None):
#    if fontsize==None:
#        fontsize=int(height/2.5)
#    blank_surface=pygame.Surface((width,height))
#    blank_surface.fill(color)
#    if letter!=None:
#        f=font.Font(font.get_default_font(),fontsize)
#        fwidth,fheight=f.size(letter)
#        rendered=f.render(letter,True,lcolor)
#        blank_surface.blit(rendered,((width-fwidth)/2,(height-fheight)/2),
#                           (0,0,fwidth,fheight))
#    return blank_surface

def draw_outline(surface,color=(0,0,0),bwidth=1):
    top_left=(0,0)
    top_right=(surface.get_width()-1,0)
    bottom_left=(0,surface.get_height()-1)
    bottom_right=(surface.get_width()-1,
                  surface.get_height()-1)
    pygame.draw.polygon(surface,color,
                        [top_left,bottom_left,bottom_right,top_right],2*bwidth)

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

def drawletter(surface,letter,color=(255,255,255),fontsize=None):
    surface=copy(surface)
    width,height=surface.get_width(),surface.get_height()
    if fontsize==None:
        fontsize=int(height/2.5)
    f=font.Font(font.get_default_font(),fontsize)
    fwidth,fheight=f.size(letter)
    rendered=f.render(letter,True,color)
    surface.blit(rendered,((width-fwidth)/2,(height-fheight)/2),
                 (0,0,fwidth,fheight))
    return surface

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
        data=data.upper()[len("P3Speller_Output "):].strip()
        if not data:
            continue
        y=lets.index(data[0])
        x=nums.index(data[1])
        fakeevent=pygame.event.Event(KEYDOWN,{"unicode":"UDP","pos":(x,y)})
        pygame.event.post(fakeevent)

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
        copy=copysquare(screen,*spot)
        invert(copy)
        screen.blit(copy,spot[:2])

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

def init(thescreen,tplaces,c_width,c_height):
    global screen
    global tileplaces
    global chop_width
    global chop_height
    global displaylock
    screen=thescreen
    tileplaces=tplaces
    chop_width=c_width
    chop_height=c_height
    thread.start_new_thread(udpkeyboard,())
    displaylock=thread.allocate_lock()
    import __main__
    if __main__.flashspots:
        print "starting udpflashes"
        thread.start_new_thread(udpflashes,())
