"""!
@package gcp.toolbars

@brief Georectification module - toolbars

Classes:
 - toolbars::GCPMapToolbar
 - toolbars::GCPDisplayToolbar

(C) 2007-2011 by the GRASS Development Team

This program is free software under the GNU General Public License
(>=v2). Read the file COPYING that comes with GRASS for details.

@author Markus Metz
"""

import os
import sys

import wx

from core              import globalvar
from gui_core.toolbars import BaseToolbar, BaseIcons
from icon              import MetaIcon
    
class GCPManToolbar(BaseToolbar):
    """!Toolbar for managing ground control points

    @param parent reference to GCP widget
    """
    def __init__(self, parent):
        BaseToolbar.__init__(self, parent)
        
        self.InitToolbar(self._toolbarData())
        
        # realize the toolbar
        self.Realize()

    def _toolbarData(self):
        icons = {
            'gcpSave'    : MetaIcon(img = 'gcp-save',
                                    label = _('Save GCPs to POINTS file')),
            'gcpReload'  : MetaIcon(img = 'reload',
                                    label = _('Reload GCPs from POINTS file')),
            'gcpAdd'     : MetaIcon(img = 'gcp-add',
                                    label = _('Add new GCP')),
            'gcpDelete'  : MetaIcon(img = 'gcp-delete',
                                    label = _('Delete selected GCP')),
            'gcpClear'   : MetaIcon(img = 'gcp-remove',
                                    label = _('Clear selected GCP')),
            'gcpRms'     : MetaIcon(img = 'gcp-rms',
                                    label = _('Recalculate RMS error')),
            'georectify' : MetaIcon(img = 'georectify',
                                    label = _('Georectify')),
            }
        
        return self._getToolbarData((('gcpSave', icons["gcpSave"],
                                      self.parent.SaveGCPs),
                                     ('gcpReload', icons["gcpReload"],
                                      self.parent.ReloadGCPs),
                                     (None, ),
                                     ('gcpAdd', icons["gcpAdd"],
                                      self.parent.AddGCP),
                                     ('gcpDelete', icons["gcpDelete"],
                                      self.parent.DeleteGCP),
                                     ('gcpClear', icons["gcpClear"],
                                      self.parent.ClearGCP),
                                     (None, ),
                                     ('rms', icons["gcpRms"],
                                      self.parent.OnRMS),
                                     ('georect', icons["georectify"],
                                      self.parent.OnGeorect))
                                    )
    
class GCPDisplayToolbar(BaseToolbar):
    """!GCP Display toolbar
    """
    def __init__(self, parent):
        """!GCP Display toolbar constructor
        """
        BaseToolbar.__init__(self, parent)
        
        self.InitToolbar(self._toolbarData())
        
        # add tool to toggle active map window
        self.togglemapid = wx.NewId()
        self.togglemap = wx.Choice(parent = self, id = self.togglemapid,
                                   choices = [_('source'), _('target')],
                                   style = wx.CB_READONLY)

        self.InsertControl(10, self.togglemap)

        self.SetToolShortHelp(self.togglemapid, '%s %s %s' % (_('Set map canvas for '),
                                                              BaseIcons["zoomBack"].GetLabel(),
                                                              _(' / Zoom to map')))

        # realize the toolbar
        self.Realize()
        
        self.action = { 'id' : self.gcpset }
        self.defaultAction = { 'id' : self.gcpset,
                               'bind' : self.parent.OnPointer }
        
        self.OnTool(None)
        
        self.EnableTool(self.zoomback, False)
        
    def _toolbarData(self):
        """!Toolbar data"""
        icons = {
            'gcpSet'    : MetaIcon(img = 'gcp-create',
                                   label = _('Set GCP'),
                                   desc = _('Define GCP (Ground Control Points)')),
            'quit'      : BaseIcons['quit'].SetLabel(_('Quit georectification tool')),
            'settings'  : BaseIcons['settings'].SetLabel( _('Georectifier settings')),
            'help'      : BaseIcons['help'].SetLabel(_('Georectifier manual')),
            }
        
        return self._getToolbarData((("displaymap", BaseIcons["display"],
                                      self.parent.OnDraw),
                                     ("rendermap", BaseIcons["render"],
                                      self.parent.OnRender),
                                     ("erase", BaseIcons["erase"],
                                      self.parent.OnErase),
                                     (None, ),
                                     ("gcpset", icons["gcpSet"],
                                      self.parent.OnPointer),
                                     ("pan", BaseIcons["pan"],
                                      self.parent.OnPan),
                                     ("zoomin", BaseIcons["zoomIn"],
                                      self.parent.OnZoomIn),
                                     ("zoomout", BaseIcons["zoomOut"],
                                      self.parent.OnZoomOut),
                                     ("zoommenu", BaseIcons["zoomMenu"],
                                      self.parent.OnZoomMenuGCP),
                                     (None, ),
                                     ("zoomback", BaseIcons["zoomBack"],
                                      self.parent.OnZoomBack),
                                     ("zoomtomap", BaseIcons["zoomExtent"],
                                      self.parent.OnZoomToMap),
                                     (None, ),
                                     ('settings', icons["settings"],
                                      self.parent.OnSettings),
                                     ('help', icons["help"],
                                      self.parent.OnHelp),
                                     (None, ),
                                     ('quit', icons["quit"],
                                      self.parent.OnQuit))
                                    )
