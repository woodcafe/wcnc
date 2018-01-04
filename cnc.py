import serial
import time
from threading import Thread
from optparse import OptionParser

class Cnc(object):
    def __init__(self, opts, args=None, baud=9600):
#    def __init__(self, port='COM3', baud=9600):
        self.ser = None
        if isinstance(opts, str):
            self.port = opts
            self.baud = baud
        else:
            self.port = opts.port
            self.baud = opts.baud
        
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
        
    def write(self, msg):
        if not self.ser:
            self.open()
        self.ser.write((msg+'\n').encode())
        print(">>>"+msg)
        self.read(True)
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

class Sander(Cnc):
    def __init__(self, port, feed=100, width=100):
        Cnc.__init__(self, port)
        self.feed = feed
        self.width = width
            
    def move(self, x=0, y=0, z=0):
        print('Sander:move x=%d y=%d feed=%d width=%d' %(x, y, self.feed, self.width))
        for v in range(0, abs(y)+1, ystep):
            print("v=%d"%v)
            super().move(y=v*y/abs(y))
            super().move(x, f=self.feed)
            super().move(0, f=self.feed)

class Holes(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
        self.holes = []
        for p in args:
            self.holes.append(p.split(','))
            
    def move(self, depth=0):
        for p in self.holes:
            (x, y) = p
            super().move(x, y)
            super().move(z=depth*-1, f=50)
            super().move(z=0)
            
class BigHole(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
                    
        
class Gallery(Cnc):
    def move(self, x=0, y=0, z=0):
        xx = 30
        yy = 40
        print('Gallery:move x=%s y=%s z=%s' %(x, y, z))
        for i in range(int(y/yy)):
            super().move(xx, yy*(i+1))
            super().move(z=10)
            super().move(0)
            super().move(z=0)
            
class Test(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self, opts)
            
    def move(self, x=0, y=0, z=0):
        for i in range(100):
            super().move(x, y, z)
            time.sleep(5)
            super().move(x*-1, y*-1, z*-1)
            time.sleep(5)
    
def main():
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='COM3')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=9600)
    parser.add_option('-s', '--sand', action='store_true', dest='s', help='sand second')    
    parser.add_option('-g', '--gcode', dest='g', help='gcode file')    
    parser.add_option('-f', '--feed', dest='f', help='feed rate', default=100)    
    parser.add_option('-t', '--test', action='store_true', dest='t', help='test')    
    options, args = parser.parse_args()
    
    cnc = Cnc(options.port)
    if options.s:
        cnc = Sander(options.port, options.f)
    elif options.g:
        cnc.gcode(options.g)
        return
    elif options.t:
        cnc = Test(options, args)
    cnc.move(options.x, options.y, options.z)

if __name__ == "__main__":
    main()
