import serial
from time import sleep

def writeTask(command, arduino):
  sleep(2)
  arduino.write(command);

def writeBatchTask(commands, arduino):
  for cmd in commands:
    sleep(2)
    arduino.write(cmd)

class Arduino(object):
  def __init__(self, devicePath, baudRate):
    self.devicePath = devicePath
    self.baudRate = baudRate
    self.arduino = serial.Serial(devicePath, baudRate, timeout=0.1 )
    self.busy = False;

  def write(self, commandString):
    while self.busy:
        pass

    self.busy = True
    # TODO (asaf): read num of written chars and iterate until completion
    writeBuf = commandString + '\n'
    numchars = self.arduino.write(writeBuf)
    print 'numchars: %s' % numchars
    print 'commandString: %s' % commandString
    print 'commandString len: %s' % len(commandString)

    self.busy = False
  

  def read(self):
    pass
    #return self.arduino.readline()



    


