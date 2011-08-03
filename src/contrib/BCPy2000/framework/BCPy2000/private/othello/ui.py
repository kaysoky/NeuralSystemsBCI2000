#!/usr/bin/env python
""" Othello game GUI
    Humberto Henrique Campos Pinheiro
    Modifications by Collin Stocks

"""

import bci2000control
from bci2000control import disruptflashes,drawletter,LETTERS
import pygame
import sys
from pygame.locals import *
import time
import os
from board import BLACK,WHITE

class Gui :

    def __init__ ( self,width,height,screen=None ):
        """ Initializes graphics.
        """

        self.r8=range(8)

        # colors
        self.BLACK = ( 0, 0, 0 )
        self.BLUE = ( 0, 0, 255 )
        self.YELLOW = ( 255, 255, 0 )
        self.WHITE = ( 255, 255, 255 )

        # display
        self.SCREEN_SIZE = ( width, height )
        self.BOARD_POS = ( int(35/128.*width), int(1/24.*height) )
        self.BOARD = ( int(39/128.*width), int(1/12.*height) )
        #self.BOARD_SIZE = 400
        self.BOARD_WIDTH=int(5/8.*width)
        self.BOARD_HEIGHT=int(5/6.*height)
        #self.SQUARE_SIZE = 50
        self.SQUARE_WIDTH=int(5/64.*width)
        self.SQUARE_HEIGHT=int(5/48.*height)
        if screen==None:
            self.screen = pygame.display.set_mode ( self.SCREEN_SIZE )
            tplaces=[]
            for y in range(8):
                for x in range(8):
                    px = x*self.SQUARE_WIDTH + self.BOARD[0]
                    py = y*self.SQUARE_HEIGHT + self.BOARD[1]
                    tplaces.append((px,py))
            bci2000control.init(self.screen,tplaces,self.SQUARE_WIDTH,self.SQUARE_HEIGHT)
        else:
            self.screen=screen
        self.displaylock=bci2000control.displaylock

        # messages
        self.BLACK_LAB_POS = ( int(3/64.*width), height/4 )
        self.WHITE_LAB_POS = ( int(3/64.*width), height * 3 / 4 )
        pygame.font.init()
        self.font = pygame.font.Font(pygame.font.get_default_font(),int(11/320.*width))

        # image files
        black_img = pygame.image.load ( "preta.bmp" ).convert()
        self.black_img=pygame.transform.scale(black_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        white_img = pygame.image.load ( "branca.bmp" ).convert()
        self.white_img=pygame.transform.scale(white_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        tip_img = pygame.image.load ( "tip.bmp" ).convert()
        self.tip_img=pygame.transform.scale(tip_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        tip_img_black = pygame.image.load ( "tip_black.bmp" ).convert()
        self.tip_img_black=pygame.transform.scale(tip_img_black,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        tip_img_white = pygame.image.load ( "tip_white.bmp" ).convert()
        self.tip_img_white=pygame.transform.scale(tip_img_white,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        clear_img = pygame.image.load ( "nada.bmp").convert()
        self.clear_img=pygame.transform.scale(clear_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        preview_img = pygame.image.load ( "preview.bmp" ).convert()
        self.preview_img=pygame.transform.scale(preview_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        wrong_img = pygame.image.load ( "wrong.bmp" ).convert()
        self.wrong_img=pygame.transform.scale(wrong_img,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        wrong_img_black = pygame.image.load ( "wrong_black.bmp" ).convert()
        self.wrong_img_black=pygame.transform.scale(wrong_img_black,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))
        wrong_img_white = pygame.image.load ( "wrong_white.bmp" ).convert()
        self.wrong_img_white=pygame.transform.scale(wrong_img_white,
            (self.SQUARE_WIDTH,self.SQUARE_HEIGHT))

    def show_options ( self ):
        """ Shows game options screen and returns chosen options
        """
        # default values
        player1 = "human"
        player2 = "computer"
        level = 2
       
        while True:
            self.screen.fill ( self.BLACK )

            title_fnt = pygame.font.Font(pygame.font.get_default_font(), 34)
            title = title_fnt.render ( "Othello", True, self.BLUE )
            title_pos = title.get_rect ( centerx = self.screen.get_width()/2, \
                                         centery = 60 )

            start_txt = self.font.render ( "Start", True, self.WHITE )
            start_pos = start_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                             centery = 220 )
            player1_txt = self.font.render ( "First Player", True, self.WHITE )
            player1_pos = player1_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                                 centery = 260 )
            player2_txt = self.font.render ( "Second Player", True, self.WHITE )
            player2_pos = player2_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                                 centery = 300 )
            level_txt = self.font.render ( "Computer Level", True, self.WHITE ) 
            level_pos = level_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                        centery = 340 )
            human_txt = self.font.render ( "Human", True, self.WHITE )
            comp_txt = self.font.render ( "Computer", True, self.WHITE )

            self.screen.blit ( title, title_pos )
            self.screen.blit ( start_txt, start_pos )
            self.screen.blit ( player1_txt, player1_pos )
            self.screen.blit ( player2_txt, player2_pos )
            self.screen.blit ( level_txt, level_pos )

            for event in [pygame.event.wait()] + pygame.event.get():
                if event.type == QUIT:
                    sys.exit ( 0 )
                elif event.type == MOUSEBUTTONDOWN:
                    ( mouse_x, mouse_y ) = pygame.mouse.get_pos()
                    if start_pos.collidepoint ( mouse_x, mouse_y ):
                        return ( player1, player2, level )
                    elif player1_pos.collidepoint ( mouse_x, mouse_y ):
                        player1 = self.get_chosen_player()
                    elif player2_pos.collidepoint ( mouse_x, mouse_y ):
                        player2 = self.get_chosen_player()
                    elif level_pos.collidepoint ( mouse_x, mouse_y ):
                        level = self.get_chosen_level()

            pygame.display.flip()


    def get_chosen_player( self ):
        """ Asks for a player 
        """
        while True:
            self.screen.fill ( self.BLACK )
            title_fnt = pygame.font.Font(pygame.font.get_default_font(), 34)
            title = title_fnt.render ( "Othello", True, self.BLUE )
            title_pos = title.get_rect ( centerx = self.screen.get_width()/2, \
                                         centery = 60 )
            human_txt = self.font.render ( "Human", True, self.WHITE )
            human_pos = human_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                             centery = 120 )
            comp_txt = self.font.render ( "Computer", True, self.WHITE )
            comp_pos = comp_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                       centery = 360 )

            self.screen.blit ( title, title_pos )
            self.screen.blit ( human_txt, human_pos )
            self.screen.blit ( comp_txt, comp_pos )

            for event in pygame.event.get():
                if event.type == QUIT:
                    sys.exit ( 0 )
                elif event.type == MOUSEBUTTONDOWN:
                    ( mouse_x, mouse_y ) = pygame.mouse.get_pos()
                    if human_pos.collidepoint ( mouse_x, mouse_y ):
                        return "human"
                    elif comp_pos.collidepoint ( mouse_x, mouse_y ):
                        return "computer"
                   
            pygame.display.flip()
            # avoids 100% cpu load
            time.sleep ( .05 )      
            
    
    def get_chosen_level ( self ):
        """ Level options
        """

        while True:
            self.screen.fill ( self.BLACK )
            title_fnt = pygame.font.Font(pygame.font.get_default_font(), 34)
            title = title_fnt.render ( "Othello", True, self.BLUE )
            title_pos = title.get_rect ( centerx = self.screen.get_width()/2, \
                                         centery = 60 )
            one_txt = self.font.render ( "Level 1", True, self.WHITE )
            one_pos = one_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                             centery = 120 )
            two_txt = self.font.render ( "Level 2", True, self.WHITE )
            two_pos = two_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                       centery = 240 )

            three_txt = self.font.render ( "Level 3", True, self.WHITE )
            three_pos = three_txt.get_rect ( centerx = self.screen.get_width()/2, \
                                       centery = 360 )

            self.screen.blit ( title, title_pos )
            self.screen.blit ( one_txt, one_pos )
            self.screen.blit ( two_txt, two_pos )
            self.screen.blit ( three_txt, three_pos )

            for event in pygame.event.get():
                if event.type == QUIT:
                    sys.exit ( 0 )
                elif event.type == MOUSEBUTTONDOWN:
                    ( mouse_x, mouse_y ) = pygame.mouse.get_pos()
                    if one_pos.collidepoint ( mouse_x, mouse_y ):
                        return 1
                    elif two_pos.collidepoint ( mouse_x, mouse_y ):
                        return 2
                    elif three_pos.collidepoint ( mouse_x, mouse_y ):
                        return 3
                   
            pygame.display.flip()
            # desafoga a cpu
            time.sleep ( .05 )

    def show_game ( self ):
        """ Game screen. """
        self.displaylock.acquire()
        # draws initial screen
        pygame.display.set_caption ( "Othello" )
        text = self.font.render ( "BLACK", True, self.YELLOW )
        text2 = self.font.render ( "WHITE", True, self.YELLOW )
        self.background = pygame.Surface ( self.screen.get_size() ).convert()
        self.background.fill ( self.BLACK )
        self.score_size = 50
        self.score1 = pygame.Surface ( ( self.score_size, self.score_size ) ) 
        self.score2 = pygame.Surface ( ( self.score_size, self.score_size ) ) 
        self.background.blit ( text, self.BLACK_LAB_POS )
        self.background.blit ( text2, self.WHITE_LAB_POS )
        self.screen.blit ( self.background, ( 0, 0 ), self.background.get_rect() )
        pygame.display.flip()
        self.displaylock.release()

        for i in range(8):
            for j in range(8):
                self.clear_square((i,j))

        self.put_stone ( ( 3, 3 ), WHITE ) 
        self.put_stone ( ( 4, 4 ), WHITE ) 
        self.put_stone ( ( 3, 4 ), BLACK )
        self.put_stone ( ( 4, 3 ), BLACK )

    def put_stone ( self, pos, color, player=None ):
        """ draws piece with given position and color """
        if pos == None:
            return 

        self.displaylock.acquire()
        lcolor=False
        if color == BLACK:
            img = self.black_img
            lcolor=(0,0,200)
        elif color == WHITE:
            img = self.white_img
            lcolor=(0,0,200)
        else:
            if player==BLACK:
                img=self.tip_img_black
            elif player==WHITE:
                img=self.tip_img_white
            elif player=="preview":
                img=self.preview_img
                lcolor=(0,0,200)
            elif player=="wrong":
                img=self.wrong_img
            elif player=="wrong_black":
                img=self.wrong_img_black
                lcolor=(0,0,200)
            elif player=="wrong_white":
                img=self.wrong_img_white
                lcolor=(0,0,200)
            else:
                img = self.tip_img

        letter=LETTERS[pos[1]*8+pos[0]]
        img=drawletter(img,letter,
            color=lcolor if lcolor else (0,180,200))

        x = pos[0]*self.SQUARE_WIDTH + self.BOARD[0]
        y = pos[1]*self.SQUARE_HEIGHT + self.BOARD[1]

        self.screen.blit ( img, ( x, y ), img.get_rect() )
        pygame.display.update((x,y,img.get_width(),img.get_height()))
        self.displaylock.release()

    def clear_square ( self, pos ):
        """ Puts in the given position a background image, to simulate that the
            piece was removed.
        """
        self.displaylock.acquire()
        x = pos[0]*self.SQUARE_WIDTH + self.BOARD[0] 
        y = pos[1]*self.SQUARE_HEIGHT + self.BOARD[1]        

        img=self.clear_img

        letter=LETTERS[pos[1]*8+pos[0]]
        img=drawletter(img,letter,color=(0,180,200))

        self.screen.blit ( img, ( x, y ), img.get_rect() )
        pygame.display.update((x,y,img.get_width(),img.get_height()))
        self.displaylock.release()

    def get_mouse_input ( self ):
        """ Get place clicked by mouse
        """
        while True:
            for event in pygame.event.get():
                if event.type == MOUSEBUTTONDOWN:
                    ( mouse_x, mouse_y ) = event.pos #pygame.mouse.get_pos()

                    # click was out of board, ignores
                    if mouse_x > self.BOARD_WIDTH + self.BOARD[0] or \
                       mouse_x < self.BOARD[0] or \
                       mouse_y > self.BOARD_HEIGHT + self.BOARD[1] or \
                       mouse_y < self.BOARD[1] : 
                       continue 

                    # find place
                    position = ( (mouse_x - self.BOARD[0]) / self.SQUARE_WIDTH), \
                               ( (mouse_y - self.BOARD[1]) / self.SQUARE_HEIGHT)
                    return position
                elif event.type == KEYDOWN and event.unicode=="UDP":
                    position=event.pos
                    print position
                    return position
                elif event.type == QUIT:
                    sys.exit(0)                    

            time.sleep ( .05 )

    def P3select(self,pos,board,undo=False,player=None,good=True):
        color=board[pos]
        if undo:
            if good or color:
                self.put_stone(pos,color,player)
            else:
                self.clear_square(pos)
            return
        if good:
            self.put_stone(pos,None,"preview")
        else:
            if color==BLACK:
                self.put_stone(pos,None,"wrong_black")
            elif color==WHITE:
                self.put_stone(pos,None,"wrong_white")
            else:
                self.put_stone(pos,None,"wrong")

    def update ( self, board, blacks, whites, preview=False, move=None):
        """Updates screen
        """
        if preview or move:
            assert preview and move
            self.put_stone(move,None,"preview")
            for i in range(25):
                time.sleep(0.1)
                self.wait_quit()

        for i in self.r8:
            for j in self.r8:
                if board[i][j] != 0:
                    self.put_stone ( ( i, j ), board[i][j] )

        self.displaylock.acquire()
        # updates score   
        blacks_str = '%02d ' % int ( blacks )
        whites_str = '%02d ' % int ( whites )
        fheight=self.font.get_height()
        text = self.font.render( blacks_str,True,self.YELLOW, self.BLACK )
        text2 = self.font.render( whites_str,True,self.YELLOW, self.BLACK )        
        self.screen.blit ( text, ( self.BLACK_LAB_POS[0], self.BLACK_LAB_POS[1] + fheight) )
        self.screen.blit ( text2, ( self.WHITE_LAB_POS[0], self.WHITE_LAB_POS[1] + fheight) )
        pygame.display.flip()
        self.displaylock.release()
        for i in range(5):
            time.sleep(0.1)
            self.wait_quit()

    def wait_quit ( self ):
        # wait user to close window 
        for event in pygame.event.get():
            if event.type == QUIT:
                sys.exit ( 0 )
