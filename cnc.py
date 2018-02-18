import serial
import time
from threading import Thread
from optparse import OptionParser
from PS2X import ps2

class Cnc(object):
    def __init__(self, port='COM3', baud=115200):
        self.port = port
        self.baud = baud
        self.ser = None
        self.jogging = False
        self.powerOn = False
        self.p2 = None
        
    def open(self):
        self.ser = serial.Serial(port=self.port, baudrate=self.baud,
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS, timeout=0)
        self.ser.write("\r\n\r\n".encode())
        time.sleep(2)
        self.ser.flushInput()
        if self.ser.isOpen():
            print("COM OPENED")
            #self.read()
            self.write('$x')
            self.read()
        
    def read(self, ok=False):
        time.sleep(0.1)
        while(True):
            output = self.ser.readline().decode().strip()
            if not output:
                break
            print("<<<"+output)
            if ok and output != 'ok':
                time.sleep(0.5)
                continue
        
    def write(self, msg, check=True):
        if not self.ser:
            self.open()
        self.ser.write((msg+'\n').encode())
        print(">>>"+msg)
        self.read(True)
        if check:
            self.ser.write("?\n".encode())
            self.read(True)
    
    def move(self, x=None, y=None, z=None, f=0):
        print('cnc:move x=%s y=%s z=%s f=%d' %(x, y, z, f))
        msg = 'G1 ' if f else 'G0 '
        if x != None:
            msg += 'X%d ' % (x)
        if y is not None:
            msg += 'Y%d ' % (y)
        if z is not None:
            msg += 'Z%d ' % (z)
        if f:
            msg += 'F%d ' % (f)
        self.write(msg)

    def gcode(self, fname=''):
        f = open(fname)
        for line in f:
            self.write(line)
            
    def sand(self, x=0, y=0, f=100, width=50):
        print('Sander:move x=%d y=%d feed=%d width=%d' %(x, y, f, width))
        self.write('M3')
        for v in range(0, abs(y)+1, width):
            print("v=%d"%v)
            self.move(y=v*y/abs(y), f=f*10)
            self.move(x, f=f)
            self.move(0, f=f)
        self.write('M5')
            
    def jogStart(self):
        self.p2 = ps2.PS2(bf=self.jogButton, sf=self.jogStick)
        if self.p2.setup():
            self.p2.loop()
        else:
            print('setup fail!!')
            return False
        return True
        
    def jogStick(self, x, y):
        #print('x:%d y:%d' %(x,y))
        if x==0 and y == 0:
            if self.jogging:
                self.write('!~')
            self.jogging = False
            return
        self.write('$J=G91 X%d Y%d F10000' %(x*10, y*10), False)
        self.jogging = True
        
    def jogButton(self, b):
        step = 1
        pos = {ps2.BUTTON_RIGHT:'X', ps2.BUTTON_LEFT:'X-', ps2.BUTTON_UP:'Y', ps2.BUTTON_DOWN:'Y-',
            ps2.BUTTON_L1:'Z', ps2.BUTTON_L2:'Z-'}
        if b in pos:
            self.write('$J=G91 %s%f f10000'%(pos[b], step), False)
        elif b == ps2.BUTTON_START:
            self.powerOn = not self.powerOn
            if self.powerOn:
                self.write('M3')
            else:
                self.write('M5')
        elif b == ps2.BUTTON_SELECT:
                self.p2.stop()
        elif self.jogging:
            self.write('!~')
            self.joggin = False
            



class Holes(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
        self.holes = []
        for p in args:
            self.holes.append(p.split(','))
            
    def move(self, depth=0):
        for p in self.holes:
            (x, y) = p
            super(Holes, self).move(x, y)
            super(Holes, self).move(z=depth*-1, f=50)
            super(Holes, self).move(z=0)
            
class BigHole(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
                    
        
class Gallery(Cnc):
    def move(self, x=0, y=0, z=0):
        xx = 30
        yy = 40
        print('Gallery:move x=%s y=%s z=%s' %(x, y, z))
        for i in range(int(y/yy)):
            super(Gallery, self).move(xx, yy*(i+1))
            super(Gallery, self).move(z=10)
            super(Gallery, self).move(0)
            super(Gallery, self).move(z=0)
            
class Test(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
            
    def move(self, x=0, y=0, z=0):
        for i in range(100):
            super(Test, self).move(x, y, z)
            time.sleep(5)
            super(Test, self).move(x*-1, y*-1, z*-1)
            time.sleep(5)
            
    
def main():
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='/dev/ttyUSB0')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=115200)
    parser.add_option('-s', '--sand', action='store_true', dest='s', help='sand second')    
    parser.add_option('-g', '--gcode', dest='g', help='gcode file')    
    parser.add_option('-f', '--feed', type='int', dest='f', help='feed rate', default=100)    
    parser.add_option('-t', '--test', action='store_true', dest='t', help='test')    
    parser.add_option('-w', '--write', dest='w', help='write command')    
    parser.add_option('-j', '--jog', dest='jog', action='store_true', help='jog mode')    
    options, args = parser.parse_args()
    
    cnc = Cnc(options.port)
    if options.s:
        sand(options.x, options.y, options.f)
        return
    elif options.g:
        cnc.gcode(options.g)
        return
    elif options.t:
        cnc = Test(options, args)
    elif options.w:
        cnc.write(options.w)
        return
    elif options.jog:
        cnc.jogStart()
        return
    cnc.move(options.x, options.y, options.z)

if __name__ == "__main__":
    main()
