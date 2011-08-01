#!/usr/bin/python

"""
This class supports a slightly limited approach to writing BCI2000 dat files.
Most likely, it should never be used.

"""

import struct

import numpy as np

def get_state_changes(state_array):
    flattened = state_array.flatten()
    return (flattened[1:] != flattened[:-1]).nonzero()[0] + 1

class BCIFileWriter(object):

    def __init__(self, channels):
        self.states = {}
        self.parameters = {}
        self.data = []
        self.index = 0
        self.channels = channels

    def append(self, data):
        assert data.shape[0] == self.channels
        data = np.asarray(data, dtype = np.float32)
        self.data.append(data)
        self.index += data.shape[1]

    def changeState(self, name, value):
        if name not in self.states:
            self.states[name] = {}
        self.states[name][self.index] = value

    def includeParameterStr(self, parameter):
        parameter = parameter.strip()
        parts = parameter.split()
        assert 2 < len(parts) # at the very least we need a section, a type, and a name!
        section = parts[0]
        type = parts[1]
        name = parts[2]
        self.parameters[" ".join((section, name))] = parameter

    def includeParameterFile(self, fname):
        for line in open(fname):
            self.includeParameterStr(line)

    def write(self, fname):
        channels = self.channels
        states = self.states
        parameters = sorted(self.parameters.values())
        data = self.data
        newline = "\r\n"
        version = "1.1"
        format = "float32"
        signalstruct = struct.Struct("<%if" % channels)

        statenames = sorted(states.keys())
        bits = []
        for statename in statenames:
            max_val = 1
            for time in states[statename]:
                value = states[statename][time]
                if value > max_val:
                    max_val = value
            size = 1
            while 1 << size <= max_val:
                size += 1
            bits.append(size)
        statevectordefinition = ["[ State Vector Definition ]"]
        statevectorlogicaldefinition = []
        byteindex = 0
        bitindex = 0
        for statename, size in zip(statenames, bits):
            if 0 in states[statename]:
                initial_value = states[statename][0]
            else:
                initial_value = 0
            definition = "%s %i %i %i %i" % (statename, size, initial_value, byteindex, bitindex)
            statevectordefinition.append(definition)
            statevectorlogicaldefinition.append((byteindex, bitindex))
            bitindex += size
            quotient, bitindex = divmod(bitindex, 8)
            byteindex += quotient
        statevectorlen = byteindex + 1

        parameterdefinition = ["[ Parameter Definition ]"] + parameters

        headertail = newline.join(statevectordefinition + parameterdefinition) + newline + newline
        headertaillen = len(headertail)
        headerhead = "BCI2000V= %s HeaderLen= %%i SourceCh= %i StatevectorLen= %i DataFormat= %s" % \
            (version, channels, statevectorlen, format) + newline
        headerlen = headertaillen
        while headerlen != len(headerhead % headerlen) + headertaillen:
            headerlen = len(headerhead % headerlen) + headertaillen
        headerhead %= headerlen
        header = headerhead + headertail

        f = open(fname, "wb")
        f.write(header)

        statestruct = struct.Struct("<%iB" % (statevectorlen or 1))
        currentstate = [0] * len(statenames)
        index = 0
        for datablock in data:
            for column in datablock.T:
                f.write(signalstruct.pack(*column.tolist()))
                statebytes = [0] * statevectorlen
                for i in range(len(statenames)):
                    statechanges = states[statenames[i]]
                    if index in statechanges:
                        currentstate[i] = statechanges[index]
                    byteindex, bitindex = statevectorlogicaldefinition[i]
                    stateval = currentstate[i] << bitindex
                    while stateval:
                        statebytes[byteindex] |= stateval
                        statebytes[byteindex] &= 255
                        stateval >>= 8
                        byteindex += 1
                f.write(statestruct.pack(*statebytes))
                index += 1

def main(argv = []):
    from BCPy2000.BCI2000Tools.FileReader import bcistream as bs
    import BCPy2000.Paths
    from BCI2000Tools.DataFiles import load
    import BCI2000Tools.SpellerTools
    import os
    import sys
    speller_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), './speller'))
    if speller_dir not in sys.path: sys.path.append(speller_dir)
    """
    import mpiCodes
    a = load("c:/Documents and Settings/bci/Desktop/side-by-side/20100714_JV_002/20100714_JV_S002R01.pk")  # dict contains elements p, y, x, yt and xt
    x = a["x"]
    y = a["y"]
    w = BCIFileWriter(x.shape[1])
    w.changeState("Running", 1)
    w.changeState("Recording", 1)
    w.append(np.zeros((w.channels, 1)))
    w.changeState("PhaseInSequence", 1)
    w.append(np.zeros((w.channels, 1)))
    w.changeState("PhaseInSequence", 2)
    for i in range(x.shape[0]):
        w.changeState("StimulusBegin", 1)
        w.changeState("StimulusCode", (y[i] > 0) + 1)
        w.changeState("StimulusType", y[i] > 0)
        w.append(x[i])
        w.changeState("StimulusBegin", 0)
        w.changeState("StimulusCode", 0)
        w.changeState("StimulusType", 0)
        w.append(np.zeros((w.channels, 1)))
    w.includeParameterFile("c:/Documents and Settings/bci/Desktop/pyparms.prm")
    w.write("c:/Documents and Settings/bci/Desktop/output.dat")
    return
    """
    b = bs("c:/Documents and Settings/bci/Desktop/classicdata.dat")
    w = BCIFileWriter(b.channels())
    w.changeState("SelectedColumn", 0)
    w.changeState("SelectedRow", 0)
    w.changeState("SelectedTarget", 0)
    size = b.samples()
    priorstates = {}
    for i in range(100000):
        sig, states = b.decode(1)
        if sig.size == 0:
            break
        for statename in states:
            if statename in priorstates and priorstates[statename] == states[statename]:
                continue
            priorstates[statename] = states[statename]
            if statename in ["SelectedColumn", "SelectedRow", "SelectedTarget"]:
                continue
            if statename in ["Targ%02i" % i for i in range(20)]:
                continue
            #if statename in ["StimulusCode"]:
            #    if states["StimulusType"][0,0]:
            #        w.changeState("StimulusCode", 1 * states["StimulusBegin"][0, 0])
            #    else:
            #        w.changeState("StimulusCode", (np.random.randint(17) + 1) * states["StimulusBegin"][0, 0])
            #    continue
            w.changeState(statename, states[statename][0, 0])
        w.append(sig)
    w.includeParameterFile("c:/Documents and Settings/bci/Desktop/classicparms.prm")
    w.write("c:/Documents and Settings/bci/Desktop/output.dat")

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
