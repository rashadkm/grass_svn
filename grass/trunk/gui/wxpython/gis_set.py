#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# generated by wxGlade 0.4.1 on Mon Feb 26 18:29:42 2007

import wx
import os
import glob
import shutil


def read_grassrc():
    """
    Read variables from $HOME/.grassrc6 file
    """

    grassrc = {}

    if os.path.isfile(os.getenv("GISRC")):
        rc = open(os.getenv("GISRC"), "r")
        for line in rc.readlines():
            key,val = line.split(":")
            grassrc[key.strip()] = val.strip()
        rc.close()

    return grassrc

class EpsgCode(wx.Frame):
    def __init__(self, parent, id, title, ):
        wx.Frame.__init__(self,parent, id , title, size=(50,600))

        self.parent = parent

        # sizers
        self.vsizer= wx.BoxSizer(wx.VERTICAL)
        self.sizer = wx.FlexGridSizer(5,4,5,5)

        # labels
        self.lname= wx.StaticText(self, -1, "Name of new Location: ",
                style=wx.ALIGN_RIGHT)
        self.lfile= wx.StaticText(self, -1, "Path to the EPSG-codes file: ",
                style=wx.ALIGN_RIGHT)
        self.lcode= wx.StaticText(self, -1, "EPSG code: ",
                style=wx.ALIGN_RIGHT)
        self.lsearch= wx.StaticText(self, -1, "Search in code description: ",
                style=wx.ALIGN_RIGHT)

        # text input
        self.tname = wx.TextCtrl(self,-1, "newLocation", size=(200,20))
        epsgdir = os.path.join(os.environ["GRASS_PROJSHARE"], 'epsg')
        self.tfile = wx.TextCtrl(self,-1, epsgdir, size=(200,20))

        self.tcode = wx.TextCtrl(self,-1, "", size=(200,20))

        # buttons
        self.bbrowse = wx.Button(self, -1, "Browse ...", size=(100,-1))
        self.bbcodes = wx.Button(self, -1, "Browse Codes")
        self.bcancel = wx.Button(self, -1, "Cancel", size=(100,-1))
        self.bcreate = wx.Button(self, -1, "Create", size=(100,-1))

        # empty panels
        self.epanel1 = wx.Panel(self,-1)
        self.epanel2 = wx.Panel(self,-1)
        self.epanel3 = wx.Panel(self,-1)
        self.epanel4 = wx.Panel(self,-1)

        # search box
        self.searchb = wx.SearchCtrl(self, size=(200,-1), style=wx.TE_PROCESS_ENTER)

        # table
        self.tablewidth=600
        self.epsgs = wx.ListCtrl(self, id=wx.ID_ANY,
                     size=(750,200),
                     style=wx.LC_REPORT|
                     wx.LC_HRULES|
                     wx.EXPAND)
        self.epsgs.InsertColumn(0, 'EPSG', wx.LIST_FORMAT_CENTRE)
        self.epsgs.InsertColumn(1, 'Description', wx.LIST_FORMAT_LEFT)
        self.epsgs.InsertColumn(2, 'Parameters', wx.LIST_FORMAT_LEFT)
        self.epsgs.SetColumnWidth(0, 50)
        self.epsgs.SetColumnWidth(1, 300)
        self.epsgs.SetColumnWidth(2, 400)

        # layout
        self.sizer.Add(self.lname, 0, wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 10)
        self.sizer.Add(self.tname, 0, wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 10)
        self.sizer.Add(self.epanel1, 0, wx.ALIGN_LEFT, 1)
        self.sizer.Add(self.epanel2, 0, wx.ALIGN_LEFT, 1)

        self.sizer.Add(self.lfile, 0 , wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 5)
        self.sizer.Add(self.tfile, 0 , wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 5)
        self.sizer.Add(self.bbrowse, 0 , wx.ALIGN_LEFT  |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.LEFT, 5)
        self.sizer.Add(self.epanel3, 0, wx.ALIGN_LEFT, 1)

        self.sizer.Add(self.lcode, 0, wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 5)
        self.sizer.Add(self.tcode, 0, wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP, 5)
        self.sizer.Add(self.bcreate, 0 , wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.LEFT, 5)
        self.sizer.Add(self.bcancel, 0 , wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.RIGHT, 5)

        self.sizer.Add(self.lsearch, 0, wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.BOTTOM, 5)
        self.sizer.Add(self.searchb, 0, wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.BOTTOM, 5)
        self.sizer.Add(self.bbcodes, 0 , wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL |
                       wx.TOP|wx.LEFT|wx.BOTTOM, 5)
        self.sizer.Add(self.epanel4, 0, wx.ALIGN_LEFT, 1)

        self.vsizer.Add(self.sizer,0, wx.ADJUST_MINSIZE, 1)
        self.vsizer.Add(self.epsgs, wx.ALIGN_LEFT|wx.EXPAND, 0)

        self.SetAutoLayout(True)
        self.SetSizer(self.vsizer)
        self.vsizer.Fit(self)
        self.vsizer.SetSizeHints(self)
        self.Layout()

        # events
        wx.EVT_BUTTON(self, self.bbrowse.GetId(), self.OnBrowse)
        wx.EVT_BUTTON(self, self.bcancel.GetId(), self.OnCancel)
        wx.EVT_BUTTON(self, self.bcreate.GetId(), self.OnCreate)
        wx.EVT_BUTTON(self, self.bbcodes.GetId(), self.OnBrowseCodes)
        self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected, self.epsgs)
        self.searchb.Bind(wx.EVT_TEXT_ENTER, self.OnDoSearch, self.searchb)

    def OnDoSearch(self,event):
        str =  self.searchb.GetValue()
        listItem  = self.epsgs.GetColumn(1)

        for i in range(self.epsgs.GetItemCount()):
            listItem = self.epsgs.GetItem(i,1)
            if listItem.GetText().find(str) > -1:
                epsgcode = self.epsgs.GetItem(i, 0)
                self.tcode.SetValue(epsgcode.GetText())
                break

        self.OnBrowseCodes(None,str)


    def OnBrowse(self, event):

        dlg = wx.FileDialog(self, "Locate EPSG codes file:",
        "/", "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
                    path = dlg.GetPath()
                    self.tfile.SetValue(path)
        dlg.Destroy()

    def OnCancel(self, event):
        self.Destroy()

    def OnItemSelected(self,event):
        item = event.GetItem()
        self.tcode.SetValue(str(item.GetText()))


    def OnBrowseCodes(self,event,search=None):
        try:
            self.epsgs.DeleteAllItems()
            f = open(self.tfile.GetValue(),"r")
            i=1
            j = 0
            descr = None
            code = None
            params = ""
            #self.epsgs.ClearAll()
            for line in f.readlines():
                line = line.strip()
                if line.find("#") == 0:
                    descr = line[1:].strip()
                elif line.find("<") == 0:
                    code = line.split(" ")[0]
                    for par in line.split(" ")[1:]:
                        params += par + " "
                    code = code[1:-1]
                if code == None: code = 'no code'
                if descr == None: descr = 'no description'
                if params == None: params = 'no parameters'
                if i%2 == 0:
                    if search and descr.lower().find(search.lower()) > -1 or\
                        not search:
                        index = self.epsgs.InsertStringItem(j, code)
                        self.epsgs.SetStringItem(index, 1, descr)
                        self.epsgs.SetStringItem(index, 2, params)
                        j  += 1
                    # reset
                    descr = None; code = None; params = ""
#                if i%2 == 0:
#                    self.epsgs.SetItemBackgroundColour(i, "grey")
                i += 1
            f.close()
            self.epsgs.SetColumnWidth(1, wx.LIST_AUTOSIZE)
            self.epsgs.SetColumnWidth(2, wx.LIST_AUTOSIZE)
            self.SendSizeEvent()
        except StandardError, e:
            dlg = wx.MessageDialog(self, "Could not read EPGS codes: %s "
                    % e,"Can not read file",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()

    def OnChange(self,event):
            self.item =  event.GetItem()

    def OnCreate(self, event):
        if not self.tcode.GetValue():
            dlg = wx.MessageDialog(self, "Could not create new location: EPSG Code value missing",
                    "Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        number = -1
        try:
            code = self.tcode.GetValue()
        except:
            dlg = wx.MessageDialog(self, "Could not create new location: '%s' is not a valid EPSG code" % code,
                    "Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        if os.path.isdir(os.path.join(self.parent.gisdbase,self.tname.GetValue())):
            dlg = wx.MessageDialog(self, "Could not create new location: %s already exists"
                    % os.path.join(self.parent.gisdbase,self.tname.GetValue()),"Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        # creating location
        # all credit to Michael Barton and his file_option.tcl and
        # Markus Neteler
        try:
            dtoptions = os.popen3(" g.proj epsg=%s datumtrans=-1" % (code))[1].read()
            if dtoptions != None:
                # open a dialog to select datum transform number
                dtoptions = 'Select the number of a datum transformation to use: \n'+dtoptions
                dlg = wx.TextEntryDialog(self, dtoptions)
                dlg.SetValue('1')

                if dlg.ShowModal() == wx.ID_OK:
                    dtrans = dlg.GetValue()

                dlg.Destroy()

                cmd = os.system("g.proj -c epsg=%s location=%s datumtrans=%s" % (code, self.tname.GetValue(), dtrans))
            else:
                os.system("g.proj -c epsg=%s location=%s datumtrans=1" % (code, self.tname.GetValue()))

                self.Destroy()
            self.parent.OnSetDatabase(None)

        except StandardError, e:
            dlg = wx.MessageDialog(self, "Could not create new location: %s "
                    % str(e),"Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

    def OnDoubleClick(self, event):
        print self.epsgs.GetValue()
        pass


class GeoreferencedFile(wx.Frame):
    def __init__(self, parent, id, title, ):
        wx.Frame.__init__(self,parent, id , title)

        self.parent = parent

        self.sizer = wx.FlexGridSizer(3,3,5,5)

        self.lname= wx.StaticText(self, -1, "Name of new Location: ",
                style=wx.ALIGN_RIGHT)

        self.lfile= wx.StaticText(self, -1, "Georeferenced file: ",
                style=wx.ALIGN_RIGHT)

        self.tname = wx.TextCtrl(self,-1, "newLocation", size=(200,20))
        self.tfile = wx.TextCtrl(self,-1, "", size=(200,20))

        self.bbrowse = wx.Button(self, -1, "Browse ...", size=(100,-1))
        self.bcancel = wx.Button(self, -1, "Cancel", size=(100,-1))
        self.bcreate = wx.Button(self, -1, "Create", size=(100,-1))

        self.gpanel1 = wx.Panel(self,-1)
        self.gpanel2 = wx.Panel(self,-1)

        self.sizer.Add(self.lname, 0, wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTRE_VERTICAL |
                       wx.TOP|wx.LEFT , 10)
        self.sizer.Add(self.tname, 0, wx.ALIGN_LEFT |
                       wx.ALIGN_CENTRE_VERTICAL |
                       wx.TOP , 10)
        self.sizer.Add(self.gpanel1, 0, wx.ALIGN_LEFT |
                       wx.ALIGN_CENTRE, 5)

        self.sizer.Add(self.lfile, 0 , wx.ALIGN_RIGHT |
                       wx.ALIGN_CENTRE_VERTICAL |
                       wx.TOP , 5)
        self.sizer.Add(self.tfile, 0 , wx.ALIGN_LEFT |
                       wx.ALIGN_CENTRE_VERTICAL |
                       wx.TOP , 5)
        self.sizer.Add(self.bbrowse, 0 , wx.ALIGN_CENTER, 10)

        self.sizer.Add(self.bcreate, 0 , wx.ALIGN_CENTER |
                       wx.ALL, 10)
        self.sizer.Add(self.gpanel2, 0 , wx.ALIGN_CENTER |
                       wx.ALL, 10)

        self.sizer.Add(self.bcancel, 0 , wx.ALIGN_CENTER |
                       wx.ALL, 10)

        self.SetAutoLayout(True)
        self.SetSizer(self.sizer)
        self.sizer.Fit(self)
        self.sizer.SetSizeHints(self)
        self.Layout()


        wx.EVT_BUTTON(self, self.bbrowse.GetId(), self.OnBrowse)
        wx.EVT_BUTTON(self, self.bcancel.GetId(), self.OnCancel)
        wx.EVT_BUTTON(self, self.bcreate.GetId(), self.OnCreate)

    def OnBrowse(self, event):

        dlg = wx.FileDialog(self, "Choose a georeferenced file:", os.getcwd(), "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
                    path = dlg.GetPath()
                    self.tfile.SetValue(path)
        dlg.Destroy()

    def OnCancel(self, event):
        self.Destroy()

    def OnCreate(self, event):
        if not os.path.isfile(self.tfile.GetValue()):
            dlg = wx.MessageDialog(self, "Could not create new location: %s not file"
                    % self.tfile.GetValue(),"Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        if not self.tname.GetValue():
            dlg = wx.MessageDialog(self, "Could not create new location: name not set",
                    "Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        if os.path.isdir(os.path.join(self.parent.gisdbase,self.tname.GetValue())):
            dlg = wx.MessageDialog(self, "Could not create new location: %s exists"
                    % os.path.join(self.parent.gisdbase,self.tname.GetValue()),"Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        # creating location
        # all credit to Michael Barton and his file_option.tcl and
        # Markus Neteler
        try:
            # FIXME: this does not need to work on windows
            os.system("g.proj -c georef=%s location=%s >&2" % (self.tfile.GetValue(), self.tname.GetValue()))

            self.parent.OnSetDatabase(None)
            self.Destroy()

        except StandardError, e:
            dlg = wx.MessageDialog(self, "Could not create new location: %s "
                    % str(e),"Can not create location",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()

class GRASSStartup(wx.Frame):
    def __init__(self, *args, **kwds):
        kwds["style"] = wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)

        #
        # variables
        #
        self.grassrc = read_grassrc()
        self.gisbase=os.getenv("GISBASE")
        self.gisdbase=self._getRCValue("GISDBASE")
        self.listOfLocations = []
        self.listOfMapsets = []

        #
        # graphical elements
        #
        try:
            self.hbitmap = wx.StaticBitmap(self, -1,
                    wx.Bitmap(os.path.join(self.gisbase,"etc","gintro.gif"), wx.BITMAP_TYPE_ANY))
        except:
            self.hbitmap = wx.StaticBitmap(self,  -1, wx.EmptyBitmap(530,150))

        # labels
        self.lwelcome = wx.StaticText(self, -1,
                "Welcome to GRASS GIS Version 6.3.cvs\n"+\
                "The world's leading open source GIS",
                style=wx.ALIGN_CENTRE)
        self.ltitle = wx.StaticText(self, -1,
                "Select an existing project location and mapset\n"+\
                "or define a new location",
                style=wx.ALIGN_CENTRE)
        self.ldbase = wx.StaticText(self, -1, "GIS Data Directory:")
        self.llocation = wx.StaticText(self, -1, "Project Location\n(projection/coordinate system)", style=wx.ALIGN_CENTRE)
        self.lmapset = wx.StaticText(self, -1, "Accessible Mapsets\n(directories of GIS files)", style=wx.ALIGN_CENTRE)
        self.lcreate = wx.StaticText(self, -1, "Create new mapset\nin selected location", style=wx.ALIGN_CENTRE)
        self.ldefine = wx.StaticText(self, -1, "Define new location with ...", style=wx.ALIGN_CENTRE)

        # buttons
        buttonsize1 = (150,-1)
        buttonsize2 = (150, -1)

        self.bstart = wx.Button(self, -1, "Start GRASS", size=buttonsize2)
        self.bstart.SetDefault()
        self.bexit = wx.Button(self, -1, "Exit", size=buttonsize2)
        self.bhelp = wx.Button(self, -1, "Help", size=buttonsize2)
        self.bbrowse = wx.Button(self, -1, "Browse ...", size=(-1,-1))
        self.bmapset = wx.Button(self, -1, "Create new mapset", size=buttonsize1)
        self.bgeoreferenced = wx.Button(self, -1, "Georeferenced file", size=buttonsize1)
        self.bepsg = wx.Button(self, -1, "EPSG codes", size=buttonsize1)
        self.bwizard = wx.Button(self, -1, "Run Wizard", size=buttonsize1)


        # textinputs
        self.tgisdbase = wx.TextCtrl(self, -1, "", size=(300, 20),
                style=wx.TE_LEFT)
        self.tnewmapset = wx.TextCtrl(self,-1, "", size=(150,20))

        # Locations
        self.lpanel = wx.Panel(self,-1)
        self.lblocations = wx.ListBox(self.lpanel,
                26, wx.DefaultPosition, (150, 200), self.listOfLocations, wx.LB_SINGLE)

        # Mapsets
        self.mpanel = wx.Panel(self,-1)
        self.lbmapsets = wx.ListBox(self.mpanel,
                26, wx.DefaultPosition, (150, 200), self.listOfMapsets, wx.LB_SINGLE)


        # layout & properties
        self.__set_properties()
        self.__do_layout()

        # events
        wx.EVT_BUTTON(self, self.bbrowse.GetId(), self.OnBrowse)
        wx.EVT_BUTTON(self, self.bstart.GetId(), self.OnStart)
        wx.EVT_BUTTON(self, self.bexit.GetId(), self.OnExit)
        wx.EVT_BUTTON(self, self.bhelp.GetId(), self.OnHelp)
        wx.EVT_BUTTON(self, self.bmapset.GetId(), self.OnCreateMapset)
        wx.EVT_BUTTON(self, self.bgeoreferenced.GetId(), self.OnGeoreferenced)
        wx.EVT_BUTTON(self, self.bepsg.GetId(), self.OnEpsg)
        wx.EVT_BUTTON(self, self.bwizard.GetId(), self.OnWizard)
        self.lblocations.Bind(wx.EVT_LISTBOX, self.OnSelectLocation)
        self.lbmapsets.Bind(wx.EVT_LISTBOX, self.OnSelectMapset)
        wx.EVT_KEY_DOWN(self.tgisdbase, self.OnKeyPressedInDbase)
        wx.EVT_KEY_DOWN(self.tnewmapset, self.OnKeyPressedInMapset)
        self.Bind(wx.EVT_CLOSE, self.onCloseWindow)

    def __set_properties(self):
        self.SetTitle("Welcome to GRASS GIS")
        self.SetIcon(wx.Icon(os.path.join(self.gisbase,"etc","dm","grass.gif"),
            wx.BITMAP_TYPE_GIF))
        self.lwelcome.SetForegroundColour(wx.Colour(35, 142, 35))
        self.lwelcome.SetFont(wx.Font(14, wx.DEFAULT, wx.NORMAL, wx.BOLD, 0, ""))
        self.bstart.SetForegroundColour(wx.Colour(35, 142, 35))
        self.bstart.SetToolTipString("Enter GRASS session")
        #self.bstart.Enable(False)
        #self.bmapset.Enable(False)

        # set database
        if not self.gisdbase: self.gisdbase = os.getenv("HOME")
        self.tgisdbase.SetValue(self.gisdbase)

        # list of locations
        self.UpdateLocations(self.tgisdbase.GetValue())
        self.OnSetDatabase(None)
        location = self._getRCValue("LOCATION_NAME")
        if location == "<UNKNOWN>":
            location = None
        if location:
            self.lblocations.SetSelection(self.listOfLocations.index(location))

            # list of mapsets
            self.UpdateMapsets(os.path.join(self.tgisdbase.GetValue(),self.listOfLocations[0]))
            mapset =self._getRCValue("MAPSET")
            if  mapset:
                self.lbmapsets.SetSelection(self.listOfMapsets.index(mapset))
                #self.bstart.Enable(True)

    def __do_layout(self):
        label_style = wx.ADJUST_MINSIZE | wx.ALIGN_CENTER_HORIZONTAL
        sizer = wx.BoxSizer(wx.VERTICAL)
        dbase_sizer=wx.BoxSizer(wx.HORIZONTAL)
        grid_sizer = wx.FlexGridSizer(4, 3, 4, 4)
        mapset_sizer = wx.BoxSizer(wx.VERTICAL)

        dbase_sizer.Add(self.ldbase, 0, wx.ALIGN_CENTER_VERTICAL|
                wx.ALIGN_CENTER_HORIZONTAL|wx.ALL, 5)
        dbase_sizer.Add(self.tgisdbase, 0,  wx.ALIGN_CENTER_VERTICAL
                |wx.ALIGN_CENTER_HORIZONTAL|wx.ALL, 5)
        dbase_sizer.Add(self.bbrowse, 0, wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL|wx.ALL, 5)

        mapset_sizer.Add(self.tnewmapset, 0, label_style|wx.BOTTOM, 5)
        mapset_sizer.Add(self.bmapset, 0, label_style|wx.BOTTOM, 10)
        mapset_sizer.Add(self.ldefine, 0, label_style|wx.RIGHT|wx.LEFT, 5)
        mapset_sizer.Add(self.bgeoreferenced, 0, label_style|wx.TOP, 5)
        mapset_sizer.Add(self.bepsg, 0, label_style|wx.TOP, 5)
        mapset_sizer.Add(self.bwizard, 0, label_style|wx.TOP, 5)
        mapset_sizer.Add((5,0))

        grid_sizer.Add(self.llocation, 0,label_style|wx.ALL, 5)
        grid_sizer.Add(self.lmapset, 0,label_style|wx.ALL, 5)
        grid_sizer.Add(self.lcreate, 0,label_style|wx.ALL, 5)

        grid_sizer.Add(self.lpanel, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_CENTER_VERTICAL|
                       wx.ALIGN_CENTER_HORIZONTAL, 0)
        grid_sizer.Add(self.mpanel, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_CENTER_VERTICAL|
                       wx.ALIGN_CENTER_HORIZONTAL, 0)
        grid_sizer.Add(mapset_sizer, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_CENTER_VERTICAL|
                       wx.ALIGN_CENTER_HORIZONTAL, 0)

        grid_sizer.Add(self.bstart, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_TOP|
                       wx.ALIGN_CENTER_HORIZONTAL|
                       wx.BOTTOM, 10)
        grid_sizer.Add(self.bexit, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_CENTER_VERTICAL|
                       wx.ALIGN_CENTER_HORIZONTAL|
                       wx.BOTTOM, 10)
        grid_sizer.Add(self.bhelp, 0, wx.ADJUST_MINSIZE|
                       wx.ALIGN_CENTER_VERTICAL|
                       wx.ALIGN_CENTER_HORIZONTAL|
                       wx.BOTTOM, 10)

        # adding to main VERTICAL sizer
        sizer.Add(self.hbitmap, 0, wx.ADJUST_MINSIZE |
                wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL |
                wx.BOTTOM, 5) # image
        sizer.Add(self.lwelcome, # welcome message
                0,wx.ADJUST_MINSIZE |
                wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL |
                wx.EXPAND |
                wx.BOTTOM, 10)
        sizer.Add(self.ltitle, # controls title
                0,wx.ADJUST_MINSIZE |
                wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL |
                wx.EXPAND |
                wx.BOTTOM, 5)
        sizer.Add(dbase_sizer,0,wx.ADJUST_MINSIZE |
                wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL |
                wx.RIGHT | wx.LEFT, 5) # GISDBASE setting
        sizer.Add(grid_sizer, 1, wx.ADJUST_MINSIZE |
                wx.ALIGN_CENTER_VERTICAL |
                wx.ALIGN_CENTER_HORIZONTAL |
                wx.RIGHT | wx.LEFT, 5)
        self.SetAutoLayout(True)
        self.SetSizer(sizer)
        sizer.Fit(self)
        sizer.SetSizeHints(self)
        self.Layout()
        # end wxGlade

    def _getRCValue(self,value):

        if self.grassrc.has_key(value):
            return self.grassrc[value]
        else:
            return None

    def OnWizard(self,event):
        import grass_wizard
        reload(grass_wizard)
        gWizard = grass_wizard.GWizard(self,   self.tgisdbase.GetValue())


    def UpdateLocations(self,dbase):

        self.listOfLocations = []
        for location in glob.glob(os.path.join(dbase,"*")):
            try:
                glob.glob(os.path.join(location,"*")).index(os.path.join(location,"PERMANENT"))
                self.listOfLocations.append(os.path.basename(location))
            except:
                pass
        self.listOfLocations
        return self.listOfLocations

    def UpdateMapsets(self,location):

        self.listOfMapsets = []
        for mapset in glob.glob(os.path.join(location,"*")):
            if os.path.isdir(mapset):
                self.listOfMapsets.append(os.path.basename(mapset))
        return self.listOfMapsets

    def OnSelectLocation(self,event):
        if self.lblocations.GetSelection() > -1:
            self.UpdateMapsets(os.path.join(
                    self.tgisdbase.GetValue(),self.listOfLocations[self.lblocations.GetSelection()]))
        else:
            self.listOfMapsets = []
        self.lbmapsets.Clear()
        self.lbmapsets.InsertItems(self.listOfMapsets,0)

    def OnSelectMapset(self,event):
        #self.bstart.Enable(True)
        pass

    def OnSetDatabase(self,event):
        self.UpdateLocations(self.tgisdbase.GetValue())
        self.lblocations.Clear()
        self.lblocations.InsertItems(self.listOfLocations,0)
        self.lblocations.SetSelection(0)
        self.OnSelectLocation(event)

    def OnBrowse(self, event):

        grassdata = None

        dlg = wx.DirDialog(self, "Choose a GRASS directory:",
                style=wx.DD_DEFAULT_STYLE | wx.DD_NEW_DIR_BUTTON)
        if dlg.ShowModal() == wx.ID_OK:
            grassdata = dlg.GetDirectory()
            self.tgisdbase.SetValue(grassdata)
        dlg.Destroy()

        self.OnSetDatabase(event)

    def OnKeyPressedInDbase(self,event):
        if wx.WXK_RETURN == event.KeyCode:
            self.OnSetDatabase(event)
        else:
            event.Skip()

    def OnCreateMapset(self,event):

        try:
            mapset = self.tnewmapset.GetValue()
            os.mkdir(os.path.join(
                self.tgisdbase.GetValue(),
                self.listOfLocations[self.lblocations.GetSelection()],
                mapset))
            self.OnSelectLocation(None)
            self.lbmapsets.SetSelection(self.listOfMapsets.index(mapset))
        except StandardError, e:
            dlg = wx.MessageDialog(self, "Could not create new mapset: %s"
                    % e,"Can not create mapset",  wx.OK|wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()

    def OnKeyPressedInMapset(self,event):
        if wx.WXK_RETURN == event.KeyCode:
            self.OnCreateMapset(None)
        else:
            #self.bmapset.Enable(True)
            event.Skip()

    def OnGeoreferenced(self,event):
        NewLocation = GeoreferencedFile(self, -1, "Define new Location")
        NewLocation.Show()

    def OnEpsg(self,event):
        NewLocation = EpsgCode(self, -1, "Define new Location")
        NewLocation.Show()


    def OnStart(self, event):
        print "g.gisenv set=GISDBASE='%s';" % self.tgisdbase.GetValue()
        print "g.gisenv set=LOCATION_NAME='%s';" % self.listOfLocations[self.lblocations.GetSelection()]
        print "g.gisenv set=MAPSET='%s';" % self.listOfMapsets[self.lbmapsets.GetSelection()]
        self.Destroy()

    def OnExit(self, event):
        print "exit"
        self.Destroy()

    def OnHelp(self, event):
        print "Event handler `OnHelp' not implemented!"
        event.Skip()

    def onCloseWindow(self, event):
        print "exit"
        event.Skip()



class StartUp(wx.App):
    def OnInit(self):
        wx.InitAllImageHandlers()
        StartUp = GRASSStartup(None, -1, "")
        self.SetTopWindow(StartUp)
        StartUp.Show()
        return 1

# end of class StartUp

if __name__ == "__main__":

    GRASSStartUp = StartUp(0)
    GRASSStartUp.MainLoop()
