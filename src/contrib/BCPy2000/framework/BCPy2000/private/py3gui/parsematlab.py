#!/usr/bin/python

import numpy as np

def parse(value):
    """
    Parses certain types of matlab types from strings. Specifically:
        * 2D arrays consisting of space/comma-separated integers or floats
        * ranges of values composed of start:stop or start:step:stop
        * integers or floats
    In the event of an error, a string is returned containing an error message.
    """
    original = value
    if isinstance(value, int) or isinstance(value, float):
        return value
    value = value.strip()
    try:
        return int(value)
    except ValueError:
        pass
    try:
        return float(value)
    except ValueError:
        pass
    value = value.replace(',', ' ')
    if ':' in value:
        value = value.split(':')
        if len(value) == 2:
            try:
                start = int(value[0].strip())
                stop = int(value[1].strip())
                step = 1
                return np.arange(start, stop + step, step, dtype = int)
            except ValueError:
                pass
            try:
                start = float(value[0].strip())
                stop = float(value[1].strip())
                step = 1.
                return np.arange(start, stop, step)
            except ValueError:
                return 'Range parameters must be numbers.\n' + \
                    'Instead, found:\n    "%s"' % original
        elif len(value) == 3:
            try:
                start = int(value[0].strip())
                stop = int(value[2].strip())
                step = int(value[1].strip())
                return np.arange(start, stop + step, step, dtype = int)
            except ValueError:
                pass
            try:
                start = float(value[0].strip())
                stop = float(value[2].strip())
                step = float(value[1].strip())
                return np.arange(start, stop, step)
            except ValueError:
                return 'Range parameters must be numbers.\n' + \
                    'Instead, found:\n    "%s"' % original
        else:
            return 'Ranges must be composed of start:stop or' + \
                ' start:step:stop.\nInstead, found:\n    "%s"' % original
    if ' ' in value:
        value = value.split()
        newvalue = []
        for subvalue in value:
            try:
                newvalue.append(int(subvalue))
                continue
            except ValueError:
                pass
            try:
                newvalue.append(float(subvalue))
                continue
            except ValueError:
                pass
            return 'Lists must be composed of numbers separated' + \
                ' by spaces.\nInstead, found:\n    %s' % original
        return np.asarray(newvalue)
    return 'Cannot recognize\n    "%s"\nas a valid value.' % original

def main(argv = []):
    pass

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
