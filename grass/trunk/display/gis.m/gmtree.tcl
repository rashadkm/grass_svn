###############################################################
# gmtree.tcl - GRASS GIS Manager procedures for creating and managing
# layer tree
# January 2006 Michael Barton, Arizona State University
###############################################################

namespace eval GmTree {
    variable count
    variable node
    variable selected ""
    variable dblclick
    variable legend_height 20
    variable legend_width 30
    variable treeht 6
	global array tree # mon
	global array filename # mon
	variable array pg # mon

}



###############################################################################


# redraw tree
proc GmTree::redraw { mon } {
    variable tree
  
    Tree::_update_scrollregion  $tree($mon) 
}

###############################################################################

# create new layer tree
proc GmTree::create { mon } {
	variable tree
    variable legend_height
    variable legend_width
    variable treeht
    variable pg
    variable page
    variable node
    global treefile
	global tree_pane
	global pgs
	global sw
	global options
	global win
	global filename
	global keycontrol
	
	set currpg ""
	set pth ""
	set node 0
	set filename($mon) ""


	# add new page	
	set pg($mon) [$pgs add page_$mon]
	$pgs raise page_$mon

	# destroy old panel with options
    destroy $options.fr

	set sw    [ScrolledWindow $pg($mon).sw \
		-relief flat -borderwidth 0 ]
    set lw [expr $legend_width + 27]
    set lh [expr $legend_height + 6]
	
    set tree($mon)  [Tree $sw.tree_$mon \
            -relief flat -borderwidth 0 -highlightthickness 0 \
            -redraw 1 -dropenabled 1 -dragenabled 1 \
            -dragevent 1 -dropcmd "GmTree::drop" \
            -opencmd   "GmTree::open_node $sw.tree_$mon" \
            -closecmd  "GmTree::close_node $sw.tree_$mon" \
            -deltay $lh -padx $lw ]
            
	$tree($mon) configure -height $treeht
	$pgs configure -height [expr $treeht * $lh]
	
    $sw setwidget $tree($mon)
    	
    pack $sw  -side top -expand yes -fill both
    pack $tree($mon)  -side top -expand yes -fill both

    $tree($mon) configure -redraw 1

    $tree($mon) bindText  <ButtonPress-1> "GmTree::selectn $tree($mon)"
    $tree($mon) bindImage <ButtonPress-1> "GmTree::selectn $tree($mon)"
    $tree($mon) bindText  <Double-ButtonPress-1> "GmTree::edit $tree($mon)"
	$tree($mon) bindText <$keycontrol-Key-x> "GmTree::delete"
    
    return $tree($mon)
}

###############################################################################
# switch page when monitor selected
proc GmTree::switchpage { mon } {
	global pgs
	global options.fr
	global opt
	variable tree
	
	$pgs raise "page_$mon"

    pack $tree($mon)  -side top -expand yes -fill both
    
	set sel [ lindex [$tree($mon)  selection get] 0 ]
    if { $sel == "" } { return }

    GmTree::selectn $tree($mon) $sel
    update

}

###############################################################################

# ScrollView
proc GmTree::scrollview { mon } {
	variable tree

    toplevel .top -relief raised -borderwidth 2
    wm protocol .top WM_DELETE_WINDOW {
        # don't kill me
    }
    wm overrideredirect .top 1
    wm withdraw .top
    wm transient .top .
    ScrollView .top.sv -window $tree($mon) -fill black
    pack .top.sv -fill both -expand yes

}

###############################################################################

proc GmTree::drop { from to where operation type data } {
    variable tree
    global mon


    set old_parent [$from parent $data]
    set old_index [$from index $data]
    if { [lindex $where 0] == "position" } { 
        set new_parent [lindex $where 1]
        set new_index [lindex $where 2]
    } elseif { [lindex $where 0] == "widget" } {
        set new_parent "root"
        set new_index [llength [$from nodes "root"] ]
    } else {
        set node [lindex $where 1]
        if { [GmTree::node_type $node] == "group" } {
	    set new_parent $node
	    set new_index 0
       } else { 
	    set new_parent [$from parent $node]
	    set new_index [$from index $node]
	    incr new_index
       }
    }

    # test if new is not in childrens
    set parent $new_parent
    while { $parent != "root" } {
        if { $parent == $data } { return }
	set parent [$from parent $parent]
    }

    if { ($old_parent == $new_parent) && ($new_index > $old_index) } { 
        set new_index [expr $new_index - 1]
    }

    $from move $new_parent $data $new_index

}

###############################################################################

proc GmTree::open_node { tree node } {
    global mon

    GmGroup::open [GmTree::node_id $node]
}

###############################################################################


proc GmTree::close_node { tree node } {
    global mon

    GmGroup::close [GmTree::node_id $node]
}

###############################################################################


proc GmTree::selectn { tree node } {
    variable selected 

	$tree selection set $node
	update
	set selected $node
	GmTree::select $node
}


###############################################################################

proc GmTree::edit { tree node } {
	#global mon
	#global tree
	
    set res [$tree edit $node [$tree itemcget $node -text]]
    if { $res != "" } {
	$tree itemconfigure $node -text $res
    }
}


###############################################################################

proc GmTree::autoname { tree node name} {

	$tree itemconfigure $node -text $name
}

###############################################################################

# create new empty
proc GmTree::new { } {
    variable tree
    global options
    global new_root_node
    global mon
    global filename
    
    $tree($mon) delete [$tree($mon) nodes root]
    destroy $options.fr

    catch {unset filename($mon)}
    #GmPrint::init
    set new_root_node [GmGroup::create $tree($mon) "root"]
    $tree($mon) itemconfigure $new_root_node -text "UNTITLED_$mon"
    
    set filename($mon) Untitled_$mon.grc 
}

###############################################################################

#Ctrl-W to close file
proc GmTree::FileClose { stay_alive} {
    variable tree
    global options
 	global mon
    global filename
 	
    $tree($mon) delete [$tree($mon) nodes root]
    destroy $options.fr

    if { $stay_alive == ""} {
    	catch {unset filename($mon)}
    }
}


###############################################################################

# add new group/layer to tree
proc GmTree::add { type } {
	variable tree
    global new_root_node
    global mon

    if { [catch {match string {} $new_root_node}] } {
    set new_root_node root
    }
    
    # selected node
    set parent_node [ lindex [$tree($mon) selection get] 0 ]
    if { $parent_node == "" } {
       set parent_node $new_root_node
    } 

    set parent_type [GmTree::node_type $parent_node]
    
    if { $parent_type != "group" } {
        set parent_node [$tree($mon) parent $parent_node]
    }

    switch $type {
        group {
            GmGroup::create $tree($mon)  $parent_node
        }
        raster {
            GmRaster::create $tree($mon) $parent_node
        }
        vector {
            GmVector::create $tree($mon)  $parent_node
        }
        labels {
            GmLabels::create $tree($mon)  $parent_node
        }
        cmd {
            GmCmd::create $tree($mon)  $parent_node
        }
        gridline {
            GmGridline::create $tree($mon)  $parent_node
        }
        rgbhis {
            GmRgbhis::create $tree($mon)  $parent_node
        }
        hist {
            GmHist::create $tree($mon)  $parent_node
        }
        rnums {
            GmRnums::create $tree($mon)  $parent_node
        }
        arrows {
            GmArrows::create $tree($mon)  $parent_node
        }
        legend {
            GmLegend::create $tree($mon)  $parent_node
        }
        dframe {
            GmDframe::create $tree($mon)  $parent_node
        }
        barscale {
            GmBarscale::create $tree($mon)  $parent_node
        }
        chart {
            GmChart::create $tree($mon)  $parent_node
        }
        thematic {
            GmThematic::create $tree($mon)  $parent_node
        }
        fttext {
            GmFTtext::create $tree($mon)  $parent_node
        }
        dtext {
            GmDtext::create $tree($mon)  $parent_node
        }
    }
}

###############################################################################

# autoname layer when a map is selected
proc GmTree::autonamel { name } {
    variable tree
    variable node
    global mon
    
    set node [ lindex [$tree($mon) selection get] 0 ]
    GmTree::autoname $tree($mon) $node $name
}

###############################################################################
# selected node ( show options )
proc GmTree::select { node } {
    variable tree
    global options
	
    set type [GmTree::node_type $node]
    set id [GmTree::node_id $node]

    # destroy old panel with options
    destroy $options.fr
 
    set opt [frame $options.fr ]
    pack $opt -fill both -expand yes
    
    switch $type {
        raster {
            GmRaster::options $id $opt
        }
        vector {
            GmVector::options $id $opt
        }
        labels {
            GmLabels::options $id $opt
        }
        cmd {
            GmCmd::options $id $opt
        }
        gridline {
            GmGridline::options $id $opt
        }
        rgbhis {
            GmRgbhis::options $id $opt
        }
        hist {
            GmHist::options $id $opt
        }
        rnums {
            GmRnums::options $id $opt
        }
        arrows {
            GmArrows::options $id $opt
        }
        legend {
            GmLegend::options $id $opt
        }
        dframe {
            GmDframe::options $id $opt
        }
        barscale {
            GmBarscale::options $id $opt
        }
        chart {
            GmChart::options $id $opt
        }
        thematic {
            GmThematic::options $id $opt
        }
        fttext {
            GmFTtext::options $id $opt
        }
        dtext {
            GmDtext::options $id $opt
        }
    }
}

###############################################################################

# deselect ( hide options )
proc GmTree::deselect { node } {
    variable tree
    global options
	global mon
	
    destroy $options.fr
}

###############################################################################

# delete selected node
proc GmTree::delete { } {
    variable tree
    global options
    global mon

    set sel [ lindex [$tree($mon)  selection get] 0 ]
    if { $sel == "" } { return }

    $tree($mon)  delete $sel
    destroy $options.fr
}


###############################################################################

# display node
proc GmTree::display_node { node } {
    variable tree

    set type [GmTree::node_type $node]

    switch $type {
        group {
            GmGroup::display $node
		}
		raster {
			GmRaster::display $node
		}
		labels {
			GmLabels::display $node
		}
		vector {
			GmVector::display $node
		}
		cmd {
			GmCmd::display $node
		}
		gridline {
			GmGridline::display $node
		}
		rgbhis {
			GmRgbhis::display $node
		}
		hist {
			GmHist::display $node
		}
		rnums {
			GmRnums::display $node
		}
		arrows {
			GmArrows::display $node
		}
		legend {
			GmLegend::display $node
		}
		dframe {
			GmDframe::display $node
		}
		barscale {
			GmBarscale::display $node
		}
		chart {
			GmChart::display $node
		}
		thematic {
			GmThematic::display $node
		}
		fttext {
			GmFTtext::display $node
		}
		dtext {
			GmDtext::display $node
		}
    } 
}

###############################################################################

# duplicate selected layer
proc GmTree::duplicate { } {
    variable tree
    global options
    variable id
    global new_root_node mon

    if { [catch {match string {} $new_root_node}] } {
    set new_root_node root
    }
    # selected node
    set parent_node [ lindex [$tree($mon)  selection get] 0 ]
    if { $parent_node == "" } {
       set parent_node $new_root_node
    } 

    set parent_type [GmTree::node_type $parent_node]
    if { $parent_type != "group" } {
        set parent_node [$tree($mon)  parent $parent_node]
    }

    set sel [ lindex [$tree($mon)  selection get] 0 ]
    if { $sel == "" } { return }
    
    set type [GmTree::node_type $sel]
    set id [GmTree::node_id $sel]

    switch $type {
        raster {
            GmRaster::duplicate $tree($mon)  $parent_node $sel $id
        }
        labels {
            GmLabels::duplicate $tree($mon)  $parent_node $sel $id
        }
        vector {
            GmVector::duplicate $tree($mon)  $parent_node $sel $id
        }
        cmd {
            GmCmd::duplicate $tree($mon)  $parent_node $sel $id
        }
        gridline {
            GmGridline::duplicate $tree($mon)  $parent_node $sel $id
        }
        rgbhis {
            GmRgbhis::duplicate $tree($mon) $parent_node $sel $id
        }
        hist {
            GmHist::duplicate $tree($mon) $parent_node $sel $id
        }
        rnums {
            GmRnums::duplicate $tree($mon) $parent_node $sel $id
        }
        arrows {
            GmArrows::duplicate $tree($mon) $parent_node $sel $id
        }
        legend {
            GmLegend::duplicate $tree($mon) $parent_node $sel $id
        }
        dframe {
            GmDframe::duplicate $tree($mon) $parent_node $sel $id
        }
        barscale {
            GmBarscale::duplicate $tree($mon) $parent_node $sel $id
        }
        chart {
            GmChart::duplicate $tree($mon) $parent_node $sel $id
        }
        thematic {
            GmThematic::duplicate $tree($mon) $parent_node $sel $id
        }
        fttext {
            GmFTtext::duplicate $tree($mon) $parent_node $sel $id
        }
        dtext {
            GmDtext::duplicate $tree($mon) $parent_node $sel $id
        }
        group {
            GmGroup::duplicate $tree($mon) $parent_node $sel $id
        }
    }
}


###############################################################################

# save tree/options to file
proc GmTree::save { spth } {
    global gisdbase location_name mapset
    global env mon
    variable rcfile
    variable tree

    set fpath $spth
    
    set rcfile [open $fpath w]

    GmGroup::save $tree($mon) 0 "root"

    close $rcfile
}


###############################################################################

# save node to file
proc GmTree::save_node { depth node } {
    variable rcfile
    variable tree
    global mon
	global filename


    set type [GmTree::node_type $node]
    set name [$tree($mon) itemcget $node -text]

    if { $type == "group" && $name == "UNTITLED_$mon" } {
    	set name "File $filename($mon)"
    }

    switch $type {
		group {
            GmTree::rc_write $depth Group $name
            incr depth
			GmGroup::save $tree($mon) $depth $node
		}
		raster {
            GmTree::rc_write $depth Raster $name
            incr depth
	    	GmRaster::save $tree($mon) $depth $node
		}
		labels {
            GmTree::rc_write $depth Labels $name
            incr depth
			GmLabels::save $tree($mon) $depth $node
		}
		vector {
            GmTree::rc_write $depth Vector $name
            incr depth
		    GmVector::save $tree($mon) $depth $node
		}
		cmd {
            GmTree::rc_write $depth Cmd $name
            incr depth
		    GmCmd::save $tree($mon) $depth $node
		}
		gridline {
            GmTree::rc_write $depth gridline $name
            incr depth
	    	GmGridline::save $tree($mon) $depth $node
		}
		rgbhis {
            GmTree::rc_write $depth rgbhis $name
            incr depth
	    	GmRgbhis::save $tree($mon) $depth $node
		}
		hist {
            GmTree::rc_write $depth hist $name
            incr depth
	    	GmHist::save $tree($mon) $depth $node
		}
		rnums {
            GmTree::rc_write $depth rnums $name
            incr depth
	    	GmRnums::save $tree($mon) $depth $node
		}
		arrows {
            GmTree::rc_write $depth arrows $name
            incr depth
	    	GmArrows::save $tree($mon) $depth $node
		}
		legend {
            GmTree::rc_write $depth legend $name
            incr depth
	    	GmLegend::save $tree($mon) $depth $node
		}
		dframe {
            GmTree::rc_write $depth dframe $name
            incr depth
	    	GmDframe::save $tree($mon) $depth $node
		}
		barscale {
            GmTree::rc_write $depth barscale $name
            incr depth
	    	GmBarscale::save $tree($mon) $depth $node
		}
		chart {
            GmTree::rc_write $depth chart $name
            incr depth
	    	GmChart::save $tree($mon) $depth $node
		}
		thematic {
            GmTree::rc_write $depth thematic $name
            incr depth
	    	GmThematic::save $tree($mon) $depth $node
		}
		fttext {
            GmTree::rc_write $depth fttext $name
            incr depth
	    	GmFTtext::save $tree($mon) $depth $node
		}
		dtext {
            GmTree::rc_write $depth dtext $name
            incr depth
	    	GmDtext::save $tree($mon) $depth $node
		}
    } 
    set depth [expr $depth - 1]
    GmTree::rc_write $depth End
}

###############################################################################

# load tree/options from file
proc GmTree::load { lpth } {
    global gisdbase location_name mapset
    global env mon
    global max_prgindic
    global prgtext
    global prgindic
    global filename
    global mon

    variable rcfile
    variable tree

    set prgtext "Loading layers..."

    set fpath $lpth

    if { ![file exist $fpath] || ![file readable $fpath] } { 
            return 
    }

    set rcfile [open $fpath r]
    set file_size [file size $fpath]
    set nrows [expr $file_size / 16]

    set parent root
    set row 0
    while { [gets $rcfile in] > -1 } {
		set key ""
		set val ""
        set in [string trim $in " "] 
		if { $in == "" } { continue }
		if { ![regexp -- {([^ ]+) (.+)$} $in r key val] } {set key $in}
        
		# Tree of layers	
		switch $key {
			Group {
				if {[regexp -- {^File (.+)} $val r leftover]  && ($leftover != $filename($mon))} {
					set val "<-- $leftover"
				}
				set current_node [GmGroup::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
				set parent $current_node
			}
			Raster {
				set current_node [GmRaster::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			Labels {
				set current_node [GmLabels::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			Vector {
				set current_node [GmVector::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			Cmd {
				set current_node [GmCmd::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			gridline {
				set current_node [GmGridline::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			rgbhis {
				set current_node [GmRgbhis::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			hist {
				set current_node [GmHist::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			rnums {
				set current_node [GmRnums::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			arrows {
				set current_node [GmArrows::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			legend {
				set current_node [GmLegend::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			dframe {
				set current_node [GmDframe::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			barscale {
				set current_node [GmBarscale::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			chart {
				set current_node [GmChart::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			thematic {
				set current_node [GmThematic::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			fttext {
				set current_node [GmFTtext::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			dtext {
				set current_node [GmDtext::create $tree($mon) $parent]
				$tree($mon) itemconfigure $current_node -text $val 
			}
			End {
				set type [GmTree::node_type $current_node]
				if { $type == "group"  } {
					set parent [$tree($mon) parent $parent]
				}
				set current_node [$tree($mon) parent $current_node]
			}
			default {
				if {[catch {GmTree::node_type $current_node}] } {
					tk_messageBox -type ok -message "Can't open $fpath - bad file format"
					break
				} else {
					set type [GmTree::node_type $current_node]
					switch $type {
						group { 
						GmGroup::set_option $current_node $key $val
						}
						raster { 
						GmRaster::set_option $current_node $key $val
						}
						labels { 
						GmLabels::set_option $current_node $key $val
						}
						vector { 
						GmVector::set_option $current_node $key $val
						}
						cmd { 
						GmCmd::set_option $current_node $key $val
						}
						gridline { 
						GmGridline::set_option $current_node $key $val
						}
						rgbhis { 
						GmRgbhis::set_option $current_node $key $val
						}
						hist { 
						GmHist::set_option $current_node $key $val
						}
						rnums { 
						GmRnums::set_option $current_node $key $val
						}
						arrows { 
						GmArrows::set_option $current_node $key $val
						}
						legend { 
						GmLegend::set_option $current_node $key $val
						}
						dframe { 
						GmDframe::set_option $current_node $key $val
						}
						barscale { 
						GmBarscale::set_option $current_node $key $val
						}
						chart { 
						GmChart::set_option $current_node $key $val
						}
						thematic { 
						GmThematic::set_option $current_node $key $val
						}
						fttext { 
						GmFTtext::set_option $current_node $key $val
						}
						dtext { 
						GmDtext::set_option $current_node $key $val
						}
					} 
				}
			} 
		}			
		incr row
		set prg [expr $max_prgindic * $row / $nrows]
		if { $prg > $max_prgindic } { set prg $max_prgindic }
		set Gm::prgindic $prg
	} 
    close $rcfile
    set Gm::prgindic $max_prgindic
    set prgtext "Layers loaded"
}

###############################################################################

# write one row
proc GmTree::rc_write { depth args } {
    variable rcfile

    set offset [string repeat "  " $depth]

    set key [lindex $args 0]
    if { [llength $args] > 1 } {
       set value [lindex $args 1]
       set row "$offset$key $value"
    } else {
       set row "$offset$key"
    }
    puts $rcfile $row
}

###############################################################################

# returns node type
proc GmTree::node_type { node } {
    variable tree
    global mon
    global type

    if { [string compare $node "root"] == 0 } {
       return "group"
    }  
    if { [string match group* $node] } {
       return "group"
    }  
    if { [string match raster* $node] } {
       return "raster"
    }  
    if { [string match labels* $node] } {
       return "labels"
    }  
    if { [string match vector* $node] } {
       return "vector"
    }  
    if { [string match cmd* $node] } {
       return "cmd"
    }  
    if { [string match gridline* $node] } {
       return "gridline"
    }  
    if { [string match rgbhis* $node] } {
       return "rgbhis"
    }  
    if { [string match hist* $node] } {
       return "hist"
    }  
    if { [string match rnums* $node] } {
       return "rnums"
    }  
    if { [string match arrows* $node] } {
       return "arrows"
    }  
    if { [string match legend* $node] } {
       return "legend"
    }  
    if { [string match dframe* $node] } {
       return "dframe"
    }  
    if { [string match barscale* $node] } {
       return "barscale"
    }  
    if { [string match chart* $node] } {
       return "chart"
    }  
    if { [string match thematic* $node] } {
       return "thematic"
    }  
    if { [string match fttext* $node] } {
       return "fttext"
    }  
    if { [string match dtext* $node] } {
       return "dtext"
    }  
    
    return ""
}

###############################################################################


#digitize
proc GmTree::vedit { } {
    variable tree
#    variable options
	global options
    global mon

    set sel [ lindex [$tree($mon) selection get] 0 ]
    if { $sel == "" } {
        set message "No map selected"
        tk_messageBox -type ok -icon warning -message $message
        return
    }

    set type [GmTree::node_type $sel]

    switch $type {
        raster {
        term r.digit $sel
            return
        }
        vector {
	    GmVector::WorkOnVector $sel
        }
    }

}

###############################################################################


# returns node id
proc GmTree::node_id { node } {
    variable tree
	global mon
	
    if { ![regexp {[^:]+:(.+)$} $node x id] } {
        return 0
    } else {
        return $id
    }
}

