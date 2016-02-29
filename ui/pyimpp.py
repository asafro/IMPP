#!/usr/bin/env python

import sys
from arduino import Arduino, writeBatchTask, writeTask
from time import sleep

from multiprocessing import Process
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
  def __init__(self):
    #Set the Glade file
    self.gladefile = "pyimpp.glade"  
    self.stagesTree = gtk.glade.XML(self.gladefile, "mainWindow") 
      
    #Create our dictionay and connect it
    dic = {"on_mainWindow_destroy" : gtk.main_quit,
           "on_tbAdd_clicked" : self.OnAddStage,
           "on_tbClear_clicked" : self.OnClear,
           "on_tbPlay_clicked" : self.OnPlay,
           "on_tbStop_clicked" : self.OnStop,
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
    
  def OnAddStage(self, widget):
    temperatureText = self.stagesTree.get_widget("textTemperature")
    temperature = temperatureText.get_text()

    timeText = self.stagesTree.get_widget("textTime")
    time = timeText.get_text()

    if time and temperature:
      self.stagesList.append(Stage(temperature, time).getList())

  def OnStop(self, widget):
    p = Process(target=writeTask, args=('Stop', arduino))
    p.start()

  def OnPlay(self, widget):
    commands = []
    for row in self.stagesList:
      line = '%s,%s' % (row[0], row[1])
      commands.append(line)

    if len(commands) > 0:
      commands.append('Done')
      p = Process(target=writeBatchTask, args=(commands, arduino))
      p.start()


  def OnClear(self, widget):
    self.stagesList.clear()


class Stage:
  def __init__(self, temperature="", time=""):
    self.temperature = temperature
    self.time = time
    
  def getList(self):
    return [self.temperature, self.time]    
    
if __name__ == "__main__":
  arduino = Arduino('/dev/ttyACM0', 19200)
  stages = pyStages()
  gtk.main()
