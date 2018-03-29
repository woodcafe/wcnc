import socket
from PS2X import ps2
import urllib
import urllib2
from threading import Thread

WCNC_IP = '121.139.35.41'
UDP_PORT = 5005


class PS2_controller(object):
    def __init__(self):
        self.jogging = False
        self.powerOn = False
        self.p2 = None
        self.ip = WCNC_IP
        self.port = UDP_PORT
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def write(self, msg, check=True):
        print msg
        self.sock.sendto(msg, (self.ip, self.port))
    
    def jogStart(self):
        self.p2 = ps2.PS2(bf=self.jogButton, sf=self.jogStick)
        if self.p2.setup():
            self.p2.loop()
            self.write('stop')
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

    def http_connect(self, ip=WCNC_IP, port=9000):
        req = urllib2.Request("http://%s:%d/rjog"%(ip, port))
        res = urllib2.urlopen(req)
        #data = res.read()

if __name__ == '__main__':
    ps = PS2_controller()
    t = Thread(target=ps.http_connect)
    t.daemon = True
    t.start()
    ps.jogStart()
    
