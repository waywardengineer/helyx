
import wx
import serial
import colorsys
import os
numBranches = 6
ID_START = wx.NewId()
class SliderControlGroup(wx.Panel):

  def __init__(self, parent, title):
    font1 = wx.Font(12, wx.ROMAN, wx.NORMAL, wx.NORMAL)
    wx.Panel.__init__(self, parent=parent, id=wx.NewId())
    slidersGrid = wx.GridSizer(numBranches+1, 2)
    text=wx.StaticText(self, -1, title, size=(200, 20), style=wx.ALIGN_CENTRE)
    text.SetFont(font1)
    slidersGrid.Add(text)
    text=wx.StaticText(self, -1, 'Pattern', size=(200, 20), style=wx.ALIGN_CENTRE)
    text.SetFont(font1)
    slidersGrid.Add(text)
    self.sliders = []
    self.patterns = []
    for i in range(0, numBranches):
      self.sliders.append(wx.Slider(self, 0, 0, 0, 100, (0,0), (200, 50), style=wx.SL_HORIZONTAL|wx.SL_LABELS|wx.EXPAND))
      self.patterns.append(wx.TextCtrl(self, size=(100, 25)))
      slidersGrid.Add(self.sliders[i], 0, wx.ALL, 5)
      slidersGrid.Add(self.patterns[i], 0, wx.ALL, 5)    
    self.SetSizer(slidersGrid) 
class Fire(wx.Panel):
  def __init__(self, parent):
    wx.Panel.__init__(self, parent, id=wx.NewId())
    box = wx.BoxSizer(wx.HORIZONTAL)
    self.motorControls = SliderControlGroup(self,'Motor Speed')
    self.valveControls = SliderControlGroup(self, 'Propane Flow')
    box.Add(self.motorControls)
    box.Add(self.valveControls)
    self.transmit = wx.Button(self, wx.ID_CLOSE, "Transmit Settings")
    self.transmit.Bind(wx.EVT_BUTTON, tree.transmitSettings)
    self.runPattern = wx.Button(self, wx.ID_CLOSE, "Run Pattern")
    self.runPattern.Bind(wx.EVT_BUTTON, tree.runPattern)
    self.stopPattern = wx.Button(self, wx.ID_CLOSE, "Stop Pattern")
    self.stopPattern.Bind(wx.EVT_BUTTON, tree.stopPattern)
    commandBox = wx.BoxSizer(wx.VERTICAL)
    commandBox.Add(self.transmit, 0, wx.ALL, 10)
    commandBox.Add(self.runPattern, 0, wx.ALL, 10)
    commandBox.Add(self.stopPattern, 0, wx.ALL, 10)
    box.Add(commandBox)
    self.SetSizer(box) 
class LED(wx.Panel):
  def __init__(self, parent):
    wx.Panel.__init__(self, parent, id=wx.NewId())
    font1 = wx.Font(12, wx.ROMAN, wx.NORMAL, wx.NORMAL)
    box = wx.BoxSizer(wx.HORIZONTAL)
    grid = wx.GridSizer(4, 5)
    self.colors = []
    self.colorSamples = []
    self.colorValues = []
    for heading in ['', 'Preview', 'Hue', 'Saturation', 'Brightness']:
      text=wx.StaticText(self, -1, heading, style=wx.ALIGN_CENTRE)
      text.SetFont(font1)
      grid.Add(text)
    for i in range(0, 3):    
      self.colors.append(wx.Button(self, wx.ID_CLOSE, "Color Chooser"))
      grid.Add(self.colors[i])
      self.colorSamples.append(wx.Panel(self, size=(75, 20)))
      grid.Add(self.colorSamples[i])
      self.colorValues.append([])
      for j in range(0, 3):
        self.colorValues[i].append(wx.TextCtrl(self, size=(75, 25)))
        grid.Add(self.colorValues[i][j])
    self.colors[0].Bind(wx.EVT_BUTTON, self.chooseColor0)
    self.colors[1].Bind(wx.EVT_BUTTON, self.chooseColor1)
    self.colors[2].Bind(wx.EVT_BUTTON, self.chooseColor2)
    
    self.transmit = wx.Button(self, wx.ID_CLOSE, "Transmit Colors")
    self.transmit.Bind(wx.EVT_BUTTON, tree.transmitColors)
    self.save = wx.Button(self, wx.ID_CLOSE, "Save Current Colors")
    self.save.Bind(wx.EVT_BUTTON, tree.saveColors)
    self.runSequence = wx.Button(self, wx.ID_CLOSE, "Open Sequence File")
    self.runSequence.Bind(wx.EVT_BUTTON, self.openFile)
    commandBox = wx.BoxSizer(wx.VERTICAL)
    commandBox.Add(self.transmit, 0, wx.ALL, 10)
    commandBox.Add(self.save, 0, wx.ALL, 10)
    commandBox.Add(self.runSequence, 0, wx.ALL, 10)

    box.Add(grid)
    box.Add(commandBox)
    self.SetSizer(box) 
  def chooseColor0(self, event):
    self.chooseColor(0)
  def chooseColor1(self, event):
    self.chooseColor(1)
  def chooseColor2(self, event):
    self.chooseColor(2)
  def chooseColor(self, index):
    dlg = wx.ColourDialog(self)
    dlg.GetColourData().SetChooseFull(True)
    if dlg.ShowModal() == wx.ID_OK:
      data = dlg.GetColourData().GetColour().Get()
      self.colorSamples[index].SetBackgroundColour(data)
      self.colorSamples[index].Refresh()
      hsv = colorsys.rgb_to_hsv(data[0]/255., data[1]/255., data[2]/255.)
      for i in range(0, 3):
        self.colorValues[index][i].ChangeValue(str(hsv[i]*255))
    dlg.Destroy()
  def openFile(self, event):
    dlg = wx.FileDialog(self, "Choose a file", os.getcwd(), "", "*.*", wx.OPEN)
    if dlg.ShowModal() == wx.ID_OK:
      path = dlg.GetPath()
      mypath = os.path.basename(path)
      tree.runLedSequence(mypath)
    dlg.Destroy()
class TabGroup(wx.Notebook):
  def __init__(self, parent):
    wx.Notebook.__init__(self, parent, id=wx.NewId(), style=wx.BK_DEFAULT)
    parent.fire = Fire(self)
    parent.led = LED(self)
    self.AddPage(parent.fire, "Fire")
    self.AddPage(parent.led, "Led")
    
class Frame(wx.Frame):
  def __init__(self, title):

    wx.Frame.__init__(self, None, title=title, size=(1050, 720))
    self.panel = wx.Panel(self)
    outerBox = wx.BoxSizer(wx.VERTICAL)
    commandBox = wx.BoxSizer(wx.HORIZONTAL)

    self.panel.logTxt = wx.TextCtrl(self.panel, style=wx.TE_MULTILINE, pos=(100, 50), size=(600, 150))
    self.panel.allOff = wx.Button(self.panel, ID_START, "All Off")
    self.panel.allOff.Bind(wx.EVT_BUTTON, tree.allOff)
    commandBox.Add(self.panel.allOff, 0, wx.ALL, 10)
    tabGroup = TabGroup(self.panel)
    outerBox.Add(commandBox, 0, wx.ALL|wx.EXPAND, 5) 
    outerBox.Add(tabGroup, 0, wx.ALL|wx.EXPAND, 5) 
    outerBox.Add(self.panel.logTxt, 0, wx.ALL, 5)
    self.panel.SetSizer(outerBox)
    self.panel.Layout()
    
class Tree():
  def allOff():
    pass
  def transmitSettings():
    pass
  def transmitColors():
    pass
  def saveColors():
    pass
  def runPattern():
    pass
  def stopPattern():
    pass
  def runLedPattern(filePath):
    pass
tree = Tree()
app = wx.App(redirect=True,  filename="logfile.txt")
gui = Frame("Helyx Control")
gui.Show()
app.MainLoop()
