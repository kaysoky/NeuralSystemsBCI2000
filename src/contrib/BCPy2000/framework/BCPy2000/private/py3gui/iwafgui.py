#!/usr/bin/python

import os

import Tkinter
from Tkinter import *
from tkMessageBox import showwarning
import tkSimpleDialog
from tkFileDialog import askopenfilename, askopenfilenames, asksaveasfilename

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

def SaveAs(filetypes = [('All Files', '.*')], defaultextension = ''):
    return asksaveasfilename(filetypes = filetypes,
        defaultextension = defaultextension)

class Error(object):

    def __init__(self, message):
        showwarning('Error', message)

class Info(Tk):

    def __init__(self, message):
        self.message = message
        Tk.__init__(self, None)
        self.title('Information')
        self.initialize()
        self.focus()

    def initialize(self):
        self.grid_rowconfigure(0, weight = 1)
        self.grid_columnconfigure(0, weight = 1)
        scrollframe = Frame(self)
        scrollframe.grid(column = 0, row = 0, sticky = 'NSEW')
        scrollframe.grid_columnconfigure(0, weight = 1)
        scrollframe.grid_rowconfigure(0, weight = 1)
        yscrollbar = Scrollbar(scrollframe, orient = VERTICAL)
        xscrollbar = Scrollbar(scrollframe, orient = HORIZONTAL)
        self.textbox = Text(scrollframe,
            yscrollcommand = yscrollbar.set, xscrollcommand = xscrollbar.set)
        yscrollbar.config(command = self.textbox.yview)
        xscrollbar.config(command = self.textbox.xview)
        yscrollbar.grid(row = 0, column = 1, sticky = 'NS')
        xscrollbar.grid(row = 1, column = 0, sticky = 'EW')
        self.textbox.grid(column = 0, row = 0, sticky = 'NSEW')
        self.textbox.insert(INSERT, self.message)
        self.textbox.config(state = DISABLED)
        Button(self, text = 'Okay', command = self.destroy). \
            grid(column = 0, row = 1)

class Widget(object):

    def __init__(self, name, label = None):
        self.name = name
        self.label = label if label != None else name

    def getName(self):
        return self.name

    def getContents(self):
        return None

class Quit(Widget):

    def __init__(self, callback = None):
        self.name = None
        self.callback = callback

    def initialize(self, parent, row):
        self.parent_quit = parent.quit
        parent.quit = self.process
        Button(parent, text = 'Quit', command = self.process). \
            grid(column = 0, row = row)

    def process(self):
        do_quit = True
        if self.callback != None:
            do_quit = self.callback()
        if do_quit:
            self.parent_quit()

class Arguments(Widget):

    def __init__(self, name, parameters):
        """
        ``parameters'' is of the form ('name', 'label', 'default'),
        all strings.
        """
        self.name = name
        self.parameters = parameters
        self.values = {}

    def initialize(self, parent, row):
        frame = Frame(parent)
        frame.grid(column = 0, row = row, sticky = 'EW')
        frame.grid_columnconfigure(1, weight = 1)
        i = 0
        for name, label, default in self.parameters:
            Label(frame, text = label).grid(column = 0, row = i, sticky = 'E')
            if isinstance(default, str):
                subwidget = Entry(frame)
                subwidget.insert(0, default)
                self.values[name] = label, subwidget
                subwidget.grid(column = 1, row = i, sticky = 'EW')
            elif isinstance(default, list):
                var = StringVar()
                var.set(default[0])
                subwidget = OptionMenu(frame, var, *default)
                self.values[name] = label, var
                subwidget.grid(column = 1, row = i, sticky = 'EW')
            elif isinstance(default, bool):
                var = IntVar()
                var.set(default)
                subwidget = Checkbutton(frame, variable = var)
                self.values[name] = label, var
                subwidget.grid(column = 1, row = i, sticky = 'W')
            else:
                raise ValueError('Don\'t know what to do with %s.' % default)
            i += 1

    def getContents(self):
        ret = {}
        for name in self.values:
            label, var = self.values[name]
            ret[name] = label, var.get()
        return ret

class Action(Widget):

    def __init__(self, label = None, func = None):
        self.name = None
        self.label = label if label != None else name
        self.func = func

    def initialize(self, parent, row):
        self.button = Button(parent, text = self.label, command = self.process)
        self.button.grid(column = 0, row = row, sticky = 'EW')
        self.contentfunc = parent.getContents

    def process(self):
        self.button.config(state = DISABLED, text = 'Working...')
        self.button.update()
        try:
            self.func(self.name, self.contentfunc())
        finally:
            self.button.update()
            self.button.config(state = NORMAL, text = self.label)

class Browse(Widget):

    def initialize(self, parent, row):
        frame = Frame(parent)
        frame.grid(column = 0, row = row, sticky = 'EW')
        frame.grid_columnconfigure(1, weight = 1)
        Label(frame, text = self.label).grid(column = 0, row = 0)
        self.value = StringVar()
        Entry(frame, textvariable = self.value, state = 'readonly'). \
            grid(column = 1, row = 0, sticky = 'EW')
        Button(frame, text = 'Browse', command = self.askopen). \
            grid(column = 2, row = 0)

    def askopen(self):
        fname = askopenfilename()
        if fname:
            self.value.set(fname)

    def setContents(self, string):
        self.value.set(string)

    def getContents(self):
        return self.value.get()

class MultiBrowse(Widget):

    def __init__(self, name, label = 'Select Files',
        filetypes = [('All Files', '.*')]):
        self.name = name
        self.label = label
        self.fnames = []
        self.filetypes = filetypes

    def initialize(self, parent, row):
        parent.grid_rowconfigure(row, weight = 1)
        frame = Frame(parent)
        frame.grid(column = 0, row = row, sticky = 'NSEW')
        frame.grid_columnconfigure(0, weight = 1)
        frame.grid_rowconfigure(1, weight = 1)
        Button(frame, text = self.label, command = self.askopen). \
            grid(column = 0, row = 0, sticky = 'EW')
        listframe = Frame(frame)
        listframe.grid(column = 0, row = 1, sticky = 'NSEW')
        listframe.grid_columnconfigure(0, weight = 1)
        listframe.grid_rowconfigure(0, weight = 1)
        yscrollbar = Scrollbar(listframe, orient = VERTICAL)
        xscrollbar = Scrollbar(listframe, orient = HORIZONTAL)
        self.flist = Listbox(listframe, selectmode = EXTENDED,
            yscrollcommand = yscrollbar.set, xscrollcommand = xscrollbar.set)
        yscrollbar.config(command = self.flist.yview)
        xscrollbar.config(command = self.flist.xview)
        yscrollbar.grid(row = 0, column = 1, sticky = 'NS')
        xscrollbar.grid(row = 1, column = 0, sticky = 'EW')
        for fname in self.fnames:
            self.flist.insert(END, fname)
        self.flist.grid(column = 0, row = 0, sticky = 'NSEW')
        Button(frame, text = 'Remove Selected Files From List',
            command = self.clearselected).grid(column = 0, row = 2)

    def clearselected(self):
        selected = list(self.flist.curselection())
        if len(selected) == 0:
            return
        if isinstance(selected[0], str):
            for i in range(len(selected)):
                selected[i] = int(selected[i])
        for selection in reversed(sorted(selected)):
            self.fnames.pop(selection)
        self.flist.delete(0, END)
        for fname in self.fnames:
            self.flist.insert(END, fname)

    def askopen(self):
        fnames = askopenfilenames(filetypes = self.filetypes)
        if isinstance(fnames, tuple) and len(fnames) != 0:
            os.chdir(os.path.dirname(fnames[-1]))
            self.fnames.extend(fnames)
            split = fnames[-1].split('.')
            if len(split) > 1:
                extension = split[-1]
            best = 0
            i = 0
            for filetype, extensions in self.filetypes:
                if ('.' + extension) in \
                    extensions.replace('*', extension).split():
                    best = i
                i += 1
            self.filetypes.insert(0, self.filetypes.pop(best))
        else:
            return
        self.fnames = sorted(set(self.fnames))
        self.flist.delete(0, END)
        for fname in self.fnames:
            self.flist.insert(END, fname)

    def getContents(self):
        return self.fnames

class Iwaf(Tk):

    def __init__(self, title = 'Iwaf', contents = [], size = (600, 400)):
        UnSplash()
        Tk.__init__(self, None)
        self.geometry('%ix%i' % size)
        self.parent = None
        self.title(title)
        self.initialize(contents)

    def initialize(self, contents):
        self.contents = contents
        self.grid()
        self.grid_columnconfigure(0, weight = 1)
        i = 0
        for widget in contents:
            widget.initialize(self, i)
            i += 1

    def getContents(self):
        ret = {}
        for widget in self.contents:
            content = widget.getContents()
            if content != None:
                ret[widget.getName()] = (widget, content)
        return ret

    def destroy(self):
        self.quit()

def main(argv = []):
    def display(name, parameters):
        print name
        print parameters
    app = Iwaf(
        contents = [
            FileList('flist', 'Select Training Data'),
            Arguments(
                'args',
                [
                    ('responsewindow', 'Response Window [begin end] (ms): ',
                        '0 800'),
                    ('randompercent', '% Random Sample for Training: ',
                        '100'),
                    ('decimationfrequency', 'Decimation Frequency (Hz): ',
                        '20'),
                    ('maxmodelfeatures', 'Max Model Features: ',
                        '60'),
                    ('channelset', 'Channel Set: ', '1:8'),
                    ('penter', 'Threshold to Add Features: ', '0.1'),
                    ('premove', 'Threshold to Remove Features: ', '0.15')
                ]
            ),
            Action('print', 'Print', display),
            Quit()
        ]
    )
    app.mainloop()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
