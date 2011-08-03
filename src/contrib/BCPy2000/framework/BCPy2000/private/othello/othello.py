#!/usr/bin/env python
"""
othello.py Humberto Henrique Campos Pinheiro
Game initialization and main loop
Modifications by Collin Stocks

"""

#----------------begin default parameters----------------#

window_x=1000 #800 #640 # window width in pixels
window_y=800 #600 #480 # window height in pixels
preplaced=18
flashspots=True
DEBUG=False
computer = True

#-----------------end default parameters-----------------#


import optparse
parser=optparse.OptionParser()
parser.add_option("--computer","-c",
                  action="store_true",
                  dest="computer",
                  default=computer,
                  help="play against a computer instead of a human")
parser.add_option("--reverse","-r",
                  action="store_true",
                  dest="reverse",
                  default=False,
                  help="switch player 1 and player 2; only useful with -c")
parser.add_option("--level","-l",
                  action="store",
                  type="int",
                  dest="level",
                  default=3,
                  help="if playing against a computer, level of the computer; valid levels are 1, 2, and 3")
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
parser.add_option("--simulation","-s",
                  action="store_true",
                  dest="simulation",
                  default=False,
                  help="simulates a game of reversi; not for normal use")
parser.add_option("--debug","-d",
                  action="store_true",
                  dest="debug",
                  default=DEBUG,
                  help="tell the program to profile itself; not for normal use")
options,remainder=parser.parse_args()

player1="human" if not options.simulation else "computer"
player2="computer" if options.computer or options.simulation else "human"
if options.reverse:
    player1,player2=player2,player1
level=options.level
DEBUG=options.debug
window_x=options.width
window_y=options.height
flashspots=options.flashspots

import time
import ui
import player
import board
import sys
import os

os.environ['SDL_VIDEODRIVER'] = 'windib'

__file__ = os.path.abspath(sys.argv[0])
os.chdir(os.path.dirname(__file__))

BLACK = board.BLACK
WHITE = board.WHITE

class Othello:
    """ 
    Game main class.	
    """
	
    
    def __init__( self,*args ):
        """ Show options screen and start game modules
        """

        # start
        self.gui = ui.Gui(window_x,window_y,*args)
        #print args
        self.board = board.Board()
        
        # set up players
        #player1, player2, level = self.gui.show_options()
        if player1 == "human":
            self.now_playing = player.Human ( self.gui, BLACK )
        else:
            self.now_playing = player.Computer ( BLACK, level )
        if player2 == "human":
            self.other_player = player.Human ( self.gui, WHITE )
        else:
            self.other_player = player.Computer ( WHITE, level )
                        
        self.gui.show_game()

    def run ( self ):
        """ Execute the game """

        quit = False

        # main loop
        while not quit:

            self.now_playing.get_current_board ( self.board )
            
            # Color positions
            #print "Jogadas possiveis da cor :", self.now_playing.color
            valid_moves = self.board.get_valid_moves( self.now_playing.color )
            
            for pos in valid_moves:
                self.gui.put_stone ( pos, "tip_color", self.now_playing.color )
                        
            if valid_moves == []:
		# there is no possible moves to this player, pass turn
                self.now_playing, self.other_player = \
                    self.other_player, self.now_playing
		# if opponent also cannot do moves the game is over
                valid_moves = self.board.get_valid_moves( self.now_playing.color )
                if valid_moves == []:
                    break
                continue
            # asks player to do a move
            if self.now_playing.__class__==player.Computer:
                while True:
                    move = self.now_playing.get_move()            
                    if move in valid_moves:
                        # ok, the move is valid
                        break
                # update board
            else:
                move=self.now_playing.get_move()
                while True:
                    self.gui.P3select(move,self.board,good=(move in valid_moves))
                    newmove=self.now_playing.get_move()
                    if move==newmove and move in valid_moves:
                        break
                    else:
                        self.gui.P3select(move,self.board,undo=True,
                            player=self.now_playing.color,
                            good=(move in valid_moves))
                        move=newmove
                        continue
                    move=self.now_playing.get_move()
            
            self.board.apply_move ( move, self.now_playing.color )
            # update graphics
            board, n_blacks, n_whites = self.board.get_changes()
            if self.now_playing.__class__==player.Computer:
                self.gui.update(board,n_blacks,n_whites,preview=True,move=move)
            else:
                self.gui.update(board,n_blacks,n_whites)
            self.gui.wait_quit()
            
            # is the game ended?
            quit = self.board.game_ended()            

            # avoid 100% cpu load
            #time.sleep ( .05 )

	        # erase helping move places
            for pos in valid_moves:
                if pos != move: # nao apaga a pedra do jogador
                    self.gui.clear_square ( pos )
            
            # pass turn
            self.now_playing, self.other_player = \
                self.other_player, self.now_playing

        if DEBUG:
            print "done"
            sys.exit()#-----------------------------------------------------
        #while True:
        #    self.gui.wait_quit()
        #    time.sleep ( .05 )
        for i in range(40):
            self.gui.wait_quit()
            time.sleep(0.1)
        if options.simulation:
            while True:
                self.gui.wait_quit()
                time.sleep(0.2)
        return


def main():
    game=Othello()
    while True:
        screen=game.gui.screen
        game.run()
        game = Othello(screen)

if __name__ == '__main__':
    if DEBUG:
        import profile,pstats
        profile.run("main()","profile")
        stats=pstats.Stats("profile")
        stats.sort_stats("cumulative")
        stats.print_stats()
    else:
        main()
