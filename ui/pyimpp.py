#!/usr/bin/env python

import sys
from arduino import Arduino, writeBatchTask, writeTask
from time import sleep
import gobject

from multiprocessing import Process, Queue
from Queue import Empty

try:
    import pygtk
    pygtk.require("2.0")
except:
    pass
try:
    import gtk
    import gtk.glade
except:
    sys.exit(1)

class pyStages(object):
  def __init__(self, textQueue):
    #Set the Glade file
    self.gladefile = "pyimpp.glade"  
    self.stagesTree = gtk.glade.XML(self.gladefile, "mainWindow")
    self.queue = textQueue


    gobject.timeout_add(100, self._update_text)

    #Create our dictionay and connect it
    dic = {"on_mainWindow_destroy" : gtk.main_quit,
           "on_tbAdd_clicked" : self.OnAddStage,
           "on_tbClear_clicked" : self.OnClear,
           "on_tbPlay_clicked" : self.OnPlay,
           "on_tbStop_clicked" : self.OnStop,
           "on_tbClearBoxes_clicked" : self.OnCleanBoxes,
           }
    self.stagesTree.signal_autoconnect(dic)
    
    #Here are some variables that can be reused later
    self.cTemp = 0
    self.cTime = 1
    
    self.sTemp = "Temperature (c)"
    self.sTime = "Time (sec)"
        
    #Get the treeView from the widget Tree
    self.stagesView = self.stagesTree.get_widget("stagesView")
    #Add all of the List Columns to the stagesView
    self.AddStagesListColumn(self.sTemp, self.cTemp)
    self.AddStagesListColumn(self.sTime, self.cTime)
  
    #Create the listStore Model to use with the stagesView
    self.stagesList = gtk.ListStore(str, str)
    #Attache the model to the treeView
    self.stagesView.set_model(self.stagesList)  
    
  def AddStagesListColumn(self, title, columnId):
    column = gtk.TreeViewColumn(title, gtk.CellRendererText()
      , text=columnId)
    column.set_resizable(True)    
    column.set_sort_column_id(columnId)
    self.stagesView.append_column(column)

  def OnCleanBoxes(self, widget):
    temperatureTextWidget = self.stagesTree.get_widget("textTemperature")
    temperatureTextWidget.set_text('')

    timeTextWidget = self.stagesTree.get_widget("textTime")
    timeTextWidget.set_text('')

  def OnAddStage(self, widget):
    temperatureTextWidget = self.stagesTree.get_widget("textTemperature")
    temperatureText = temperatureTextWidget.get_text()
    temperature = None

    timeTextWidget = self.stagesTree.get_widget("textTime")
    timeText = timeTextWidget.get_text()
    time = None

    try:
      temperature = int(temperatureText)
      time = int(timeText)
    except:
      pass

    if time and temperature:
      self.stagesList.append(Stage(temperature, time).getList())
      self._set_status('stage added')
    else:
      self._set_status('Illegal stage: Expecting two numbers and got %s, %s' % (temperatureText, timeText))

  def OnStop(self, widget):
    p = Process(target=writeTask, args=('Stop', arduino))
    p.start()
    self._set_status('stopping experiment')

  def OnPlay(self, widget):
    commands = []
    for row in self.stagesList:
      line = '%s,%s' % (row[0], row[1])
      commands.append(line)
    if len(commands) > 0:
      commands.append('Done')
      p = Process(target=writeBatchTask, args=(commands, arduino))
      p.start()
      self._set_status('starting experiment')
    else:
      self._set_status('no stages to run')

  def OnClear(self, widget):
    self.stagesList.clear()
    self._set_status('cleared stages')

  def _update_text(self):
    try:
      line = self.queue.get_nowait()
      outputWidget = self.stagesTree.get_widget("devOutput")
      buf = outputWidget.get_buffer()
      buf.insert(buf.get_end_iter(), line)

      if line.startswith('curTemp:'):
        tempVal = line[9: -1]
        curTempWidget = self.stagesTree.get_widget("tempratureStatusbar")
        curTempWidget.push(curTempWidget.get_context_id(tempVal), tempVal)

    except:
      pass
    # If we return a Falsy value then gobject will not call this again
    return True

  def _set_status(self, message):
    statusBar = self.stagesTree.get_widget("statusbar1")
    statusBar.push(statusBar.get_context_id(message), message)



class Stage:
  def __init__(self, temperature="", time=""):
    self.temperature = temperature
    self.time = time
    
  def getList(self):
    return [self.temperature, self.time]    
   

def readTask(arduino, sharedQueue):
  while True:
    sleep(0.1)
    line = arduino.read()
    sharedQueue.put(line)


    
  
if __name__ == "__main__":
  arduinoOutputQueue = Queue()
  arduino = Arduino('/dev/ttyACM0', 9600)
  stages = pyStages(arduinoOutputQueue)

  # Create a process for reading from the arduino and writing to shared queue
  p = Process(target=readTask, args=(arduino, arduinoOutputQueue))
  p.start()

  gtk.main()

  # Cleanup after UI thread is done
  p.terminate()
