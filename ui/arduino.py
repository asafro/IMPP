import serial
import os
import fcntl
import subprocess

from time import sleep

# Equivalent of the _IO('U', 20) constant in the linux kernel.
USBDEVFS_RESET = ord('U') << (4*2) | 20



def resetArduino():
    # TODO (asafro): this assumes there exactly one arduino device connected.
    #       need to wrap in loop or be more explicit

    arduinoBusAndDev = None

    proc = subprocess.Popen(['lsusb'], stdout=subprocess.PIPE)
    out = proc.communicate()[0]
    lines = out.split('\n')
    for line in lines:
        if 'Arduino' in line:
            parts = line.split()
            bus = parts[1]
            dev = parts[3][:3]
            arduinoBusAndDev = '/dev/bus/usb/%s/%s' % (bus, dev)


    if arduinoBusAndDev:
        print 'Opening %s so we can reset it.' % arduinoBusAndDev
        _fd = os.open(arduinoBusAndDev, os.O_WRONLY)
        with os.fdopen(_fd, 'wt') as fd:
            fcntl.ioctl(fd, USBDEVFS_RESET, 0)
    else:
        raise Exception("Could not find Arduino")


def writeTask(command, arduino):
  sleep(2)
  arduino.write(command);

def writeBatchTask(commands, arduino):
  for cmd in commands:
    sleep(2)
    arduino.write(cmd)

class Arduino(object):
  def __init__(self, devicePath, baudRate):

    resetArduino()

    self.devicePath = devicePath
    self.baudRate = baudRate
    self.arduino = serial.Serial(devicePath, baudRate, timeout=0.1 )
    self.busy = False;

  def write(self, commandString):
    while self.busy:
        pass

    # Busy will fail.... change to a decent lock
    self.busy = True
    # TODO (asaf): read num of written chars and iterate until completion
    writeBuf = commandString + '\n'
    numchars = self.arduino.write(writeBuf)
    self.busy = False
  

  def read(self):
    return self.arduino.readline()



    





