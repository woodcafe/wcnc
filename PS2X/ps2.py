import ctypes
import time
import struct
import sys, os

BUTTON_SELECT = 0
BUTTON_L3 = 1
BUTTON_R3 = 2
BUTTON_START = 3
BUTTON_UP = 4
BUTTON_RIGHT = 5
BUTTON_DOWN = 6
BUTTON_LEFT = 7
BUTTON_L2 = 8
BUTTON_R2 = 9
BUTTON_L1 = 10
BUTTON_R1 = 11
BUTTON_TRIANGLE = 12
BUTTON_O = 13
BUTTON_X = 14
BUTTON_SQUARE = 15

try:
    ps2x = ctypes.CDLL('./libPs2.so')
except:
    ps2x = ctypes.CDLL('/home/pi/Projects/wcnc/PS2X/libPs2.so')

class PS2(object):
    BUTTONS = [['SELECT', 2, 0x01], ['L3', 2, 0x02], ['R3', 2, 0x04], ['START', 2, 0x08],
               ['UP', 2, 0x10], ['RIGHT', 2, 0x20], ['DOWN', 2, 0x40], ['LEFT', 2, 0x80],
               ['L2', 3, 0x01], ['R2', 3, 0x02], ['L1', 3, 0x04], ['R1', 3, 0x08],
               ['TRIANGLE', 3, 0x10], ['O', 3, 0x20], ['X', 3, 0x40], ['SQUARE', 3, 0x80]
               ]
    
    def __init__(self, bf=None, sf=None):
        self.ps2 = ps2x.new_ps2()
        self.cread = ps2x.read_ps2
        self.cread.restype = ctypes.c_double
        self.buttonFunc = bf
        self.stickFunc = sf
        self.bstop = False
        
    def setup(self):
        if ps2x.setup_ps2(self.ps2):
            return False
        return True
        
    def read(self):
        btns = self.cread(self.ps2)
        ba = bytearray(struct.pack("d", btns))   
        #print([ "0x%02x" % b for b in ba ])
        return ba 
            
    def checkButtons(self, btns):
        for idx, b in enumerate(self.BUTTONS):
            if (~btns[b[1]] & b[2]):
                if self.buttonFunc:
                    self.buttonFunc(idx)
                    
    def checkStick(self, s):
        lh = int(float(s[6]-127)/20)
        lv = int(float(s[7]-127)/20)*-1
        #print('x:%d(%d) y:%d(%d)' % (s[6], lh, s[7],lv))
        if self.stickFunc:
            self.stickFunc(lh, lv)
              
    def loop(self):
        while(not self.bstop):
            btns = self.read()
            self.checkButtons(btns)
            self.checkStick(btns)
            time.sleep(0.1)
            
    def stop(self):
        self.bstop = True

            

if __name__ == '__main__':
    ps2 = PS2()
    ps2.loop()
