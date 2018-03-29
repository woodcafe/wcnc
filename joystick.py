#import evdev
from evdev import InputDevice, categorize, ecodes

class Joystick(object):
    A = 272
    B = 1
    C = 115
    D = 114
    UP = 1
    DOWN = 0
    HOLD = 2
    
    def __init__(self, dev='/dev/input/event2'):
        self.dev = InputDevice(dev)
        
    def jread(self):
        for e in self.dev.read_loop():
            s = '%s %s' % (categorize(e), repr(e))
            print(s)
            x = y = 0
            if e.type == 2:
                if e.code == 0:
                    x = e.value
                if e.code == 1:
                    y = e.value
                self.jmove(x, y)
            elif e.type == 1:
                self.jkey(e.code, e.value)
    
    def jmove(self, x, y):
        pass
    
    def jkey(self, button, state):
        pass
    
    
if __name__ == '__main__':
    joy = Joystick()
    joy.jread()
