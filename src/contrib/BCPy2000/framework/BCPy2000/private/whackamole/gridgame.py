#!/usr/bin/python

import os
import time

import numpy as np
import Image
import pygame
pygame.font.init()

class ImageSurf(pygame.Surface):

    def __init__(self, fname, shape = None, **kwds):
        if isinstance(fname, pygame.Surface):
            surf = fname
        else:
            im = Image.open(fname).convert()
            surf = pygame.image.fromstring(
                im.tostring(),
                im.size, im.mode
            ).convert()
        if shape != None:
            pygame.Surface.__init__(self, shape, **kwds)
            self.convert()
            pygame.transform.smoothscale(surf, shape, self)
        else:
            pygame.Surface.__init__(self, surf.get_size(), **kwds)
            self.convert()
            self.blit(surf, (0, 0))

    def load(self, fname):
        im = Image.open(fname).convert()
        surf = pygame.image.fromstring(
            im.tostring(),
            im.size, im.mode
        ).convert()
        pygame.transform.scale(surf, self.get_size(), self)

    def invert(self):
        return pygame.surfarray.make_surface(
            -pygame.surfarray.array3d(self) - 1
        )

    def invertBrightness(self):
        array = np.array(pygame.surfarray.array3d(self), dtype = float) / 256
        array2 = 1 - array

        maximum = np.expand_dims(array.max(axis = 2), 2)
        array = array / maximum - array
        array[np.tile(maximum == 0, (1, 1, 3))] = 1
        weight = np.expand_dims((array ** 2).sum(axis = 2), 2) ** 0.5

        maximum2 = np.expand_dims(array2.max(axis = 2), 2)
        array2 = array2 / maximum2 - array2
        array2[np.tile(maximum2 == 0, (1, 1, 3))] = 1
        weight2 = np.expand_dims((array2 ** 2).sum(axis = 2), 2) ** 0.5

        array2 = 1 - array2
        array = np.array(
            (array * weight2 + array2 * weight) / (weight + weight2) * 256,
            dtype = int
        )
        array[array >= 256] = 255
        array[array < 0] = 0
        return self.__class__(
            pygame.surfarray.make_surface(array),
            self.get_size()
        )

class GridGame(object):

    def __init__(self, rows = 6, cols = 6, shape = (800, 600), loc = None,
        board_rect = None, border = 0, *args, **kwds):
        self.won = False
        self.shape = shape
        self.show(loc)
        self.board_rect = pygame.Rect(board_rect) if board_rect != None else \
            pygame.Rect((0, 0), shape)
        self.board_rect.left += border - border // 2
        self.board_rect.top += border - border // 2
        self.board_rect.width -= border
        self.board_rect. height -= border
        self.setupGameBoard(rows, cols, border)
        self.setupStimuli()
        self.init(*args, **kwds)

    def init(self):
        pass

    def win(self):
        if self.won:
            return
        self.won = True
        message = 'YOU WIN! '
        i = 0
        font = pygame.font.get_default_font()
        font = pygame.font.Font(font, int(self.getShape(0, 0)[0] * 0.8))
        clock = pygame.time.Clock()
        for row in range(self.rows):
            for col in range(self.cols):
                pygame.event.pump()
                im = pygame.Surface(self.getShape(row, col))
                im.fill((220, 220, 220))
                color = pygame.Color(0, 0, 0)
                color.hsva = ((16 * i) % 360, 100, 100, 100)
                character = font.render(message[i % len(message)], True,color)
                im_width, im_height = im.get_size()
                char_width, char_height = character.get_size()
                left = (im_width - char_width) // 2 + 1
                top = (im_height - char_height) // 2 + 4
                im.blit(character, (left, top))
                self.setImages(im, im, row, col)
                clock.tick(5)
                i += 1

    def imageTest(self):
        for row in range(self.rows):
            for col in range(self.cols):
                im = ImageSurf(self.getShape(row, col), 'Google.png')
                self.setImages(im, im.invert(), row, col)

    def getShape(self, row, col):
        return self.piece_rects[row, col, 2:]

    def setupStimuli(self):
        shape = (self.rows, self.cols)
        self.nonflash = np.zeros(shape, dtype = pygame.Surface)
        self.flash = np.zeros(shape, dtype = pygame.Surface)
        self.stimuli = np.zeros(shape, dtype = bool)
        self.preparing = False

    def setupGameBoard(self, rows, cols, border):
        self.rows = rows
        self.cols = cols
        self.piece_rects = np.zeros((rows, cols, 4), dtype = int)
        dims = self.board_rect
        floor_border = border // 2
        ceil_border = border - floor_border
        for i in range(cols):
            left = i * dims.width // cols + dims.left + floor_border
            right = (i + 1) * dims.width // cols + dims.left - ceil_border
            self.piece_rects[:, i, 0] = left
            self.piece_rects[:, i, 2] = right - left
        for i in range(rows):
            top = i * dims.height // rows + dims.top + floor_border
            bottom = (i + 1) * dims.height // rows + dims.top - ceil_border
            self.piece_rects[i, :, 1] = top
            self.piece_rects[i, :, 3] = bottom - top

    def setImages(self, nonflash, flash, row, col):
        self.nonflash[row, col] = nonflash
        self.flash[row, col] = flash
        if not self.preparing:
            if self.stimuli[row, col]:
                self.blit(flash, row, col)
            else:
                self.blit(nonflash, row, col)

    def getImages(self, row, col):
        return self.nonflash[row, col], self.flash[row, col]

    def show(self, loc = None):
        if os.name == 'nt':
            os.environ['SDL_VIDEODRIVER'] = 'windib'
        if loc == None:
            if 'SDL_VIDEO_WINDOW_POS' in os.environ:
                del os.environ['SDL_VIDEO_WINDOW_POS']
        else:
            os.environ['SDL_VIDEO_WINDOW_POS'] = '%i,%i' % loc
        pygame.display.quit()
        pygame.display.init()
        self.screen = pygame.display.set_mode(self.shape, pygame.NOFRAME)

    def prepare(self):
        self.preparing = True
        for row in range(self.rows):
            for col in range(self.cols):
                if self.stimuli[row, col]:
                    self.blit(self.flash[row, col], row, col)
                else:
                    self.blit(self.nonflash[row, col], row, col)

    def flip(self):
        pygame.display.flip()
        self.preparing = False

    def blit(self, surf, row, col):
        left, top, width, height = self.piece_rects[row, col, :]
        self.screen.blit(surf, (left, top), (0, 0, width, height))
        if not self.preparing:
            pygame.display.update((left, top, width, height))

