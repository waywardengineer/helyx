
import wx
import serial
numBranches = 6
ID_START = wx.NewId()
class Frame(wx.Frame):
  def __init__(self, title):
    font1 = wx.Font(12, wx.ROMAN, wx.NORMAL, wx.NORMAL)

    wx.Frame.__init__(self, None, title=title, size=(1050,600))
    panel = wx.Panel(self)
    outerBox = wx.BoxSizer(wx.VERTICAL)
    commandBox = wx.BoxSizer(wx.HORIZONTAL)
    slidersGrid = wx.GridSizer(numBranches+1, 4)
    self.motorSliders = []
    self.motorPatterns = []
    self.valveSliders = []
    self.valvePatterns = []
    for heading in ['Motor Speeds', 'Pattern', 'Valve positions', 'Pattern']:    
      text=wx.StaticText(panel, -1, heading, size=(200, 20), style=wx.ALIGN_CENTRE)
      text.SetFont(font1)
      slidersGrid.Add(text)
    for i in range(0, numBranches):
      self.motorSliders.append(wx.Slider(panel, 0, 0, 0, 100, (0,0), (200, 50), style=wx.SL_HORIZONTAL|wx.SL_LABELS|wx.EXPAND))
      self.motorPatterns.append(wx.TextCtrl(panel, size=(100, 25)))
      self.valveSliders.append(wx.Slider(panel, 0, 0, 0, 100, (0,0), (200, 50), style=wx.SL_HORIZONTAL|wx.SL_LABELS|wx.EXPAND))
      self.valvePatterns.append(wx.TextCtrl(panel, size=(100, 25)))
      slidersGrid.Add(self.motorSliders[i], 0, wx.ALL, 5)
      slidersGrid.Add(self.motorPatterns[i], 0, wx.ALL, 5)
      slidersGrid.Add(self.valveSliders[i], 0, wx.ALL, 5)
      slidersGrid.Add(self.valvePatterns[i], 0, wx.ALL, 5)



    self.logTxt = wx.TextCtrl(panel, style=wx.TE_MULTILINE, pos=(100, 50), size=(600, 150))
    self.logTxt.SetBackgroundColour("Green")
    self.allOff = wx.Button(panel, ID_START, "All Off")
    self.transmit = wx.Button(panel, wx.ID_CLOSE, "Transmit Settings")
    self.allOff.Bind(wx.EVT_BUTTON, tree.allOff)
    self.transmit.Bind(wx.EVT_BUTTON, tree.transmitSettings)

    commandBox.Add(self.allOff, 0, wx.ALL, 10)
    commandBox.Add(self.transmit, 0, wx.ALL, 10)

    outerBox.Add(commandBox, 0, wx.ALL, 5)
    outerBox.Add(slidersGrid, 0, wx.ALL, 5)
    outerBox.Add(self.logTxt, 0, wx.ALL, 5)
    panel.SetSizer(outerBox)
    panel.Layout()
    
class Tree():
  def allOff():
    pass
  def transmitSettings():
    pass
tree = Tree()
app = wx.App(redirect=True,  filename="logfile.txt")
top = Frame("Helyx Control")
top.Show()
app.MainLoop()
