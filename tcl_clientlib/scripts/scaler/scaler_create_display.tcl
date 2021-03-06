# $ZEL: scaler_create_display.tcl,v 1.7 2011/11/26 23:57:30 wuestner Exp $
# copyright 1998
#   P. Wuestner; Zentralinstitut fuer Elektronik; Forschungszentrum Juelich
#
# global vars:
#
# global_setup(show_index)
# global_setup(show_name)
# global_setup(show_content)
# global_setup(show_rate)
# global_setup(show_average_rate)
# global_setup(show_max_rate)
# global_setup_names
# global_setup_names()
# scaler_cont()
# scaler_rate_round()
# scaler_max()
# 

#-----------------------------------------------------------------------------#

proc add_header {frame} {
  global global_setup

  set column 0
  if {$global_setup(show_index)} {
    label $frame.index_head -text "Idx" -anchor w
    grid $frame.index_head -row 0 -column $column
    incr column
  }
  if {$global_setup(show_name)} {
    label $frame.name_head -text "Name" -anchor w
    grid $frame.name_head -row 0 -column $column
    incr column
  }
  if {$global_setup(show_content)} {
    label $frame.content_head -text "Content" -anchor w
    grid $frame.content_head -row 0 -column $column
    incr column
  }
  if {$global_setup(show_rate)} {
    label $frame.rate_head -text "Rate" -anchor w
    grid $frame.rate_head -row 0 -column $column
    incr column
  }
  if {$global_setup(show_max_rate)} {
    label $frame.max_head -text "Max" -anchor w
    grid $frame.max_head -row 0 -column $column
    incr column
  }
}

#-----------------------------------------------------------------------------#

proc add_chan {chan frame row} {
  global global_setup entry_ro

  set column 0
  if {$global_setup(show_index)} {
    label $frame.index_$chan -text $chan -anchor e -relief ridge
    grid $frame.index_$chan -row $row -column $column -sticky ew
    incr column
  }
  if {$global_setup(show_name)} {
    entry $frame.name_$chan -textvariable global_setup_names($chan)\
      -state $entry_ro -relief ridge -width 0
    grid $frame.name_$chan -row $row -column $column -sticky ew
    incr column
  }
  if {$global_setup(show_content)} {
    entry $frame.content_$chan -textvariable scaler_cont_disp($chan)\
      -relief ridge -state $entry_ro -justify right\
      -font $global_setup(termfont) -width 0
    grid $frame.content_$chan -row $row -column $column -sticky ew
    incr column
  }
  if {$global_setup(show_rate)} {
    entry $frame.rate_$chan -textvariable scaler_rate_disp($chan)\
      -relief ridge -state $entry_ro -justify right\
      -font $global_setup(termfont) -width 0
    grid $frame.rate_$chan -row $row -column $column -sticky ew
    incr column
  }
  if {$global_setup(show_max_rate)} {
    entry $frame.max_$chan -textvariable scaler_max_disp($chan)\
      -relief ridge -state $entry_ro -justify right\
      -font $global_setup(termfont) -width 0
    grid $frame.max_$chan -row $row -column $column -sticky ew
    incr column
  }
  
}

#-----------------------------------------------------------------------------#

proc scaler_win_scale_canvas {} {
    global global_dispwin

    set sframe $global_dispwin(sframe)
    set width [winfo reqwidth $sframe]
    set height [winfo reqheight $sframe]

    $global_dispwin(cv) configure -scrollregion "0 0 $width $height"
    set screenheight [winfo screenheight .]
    set screenwidth [winfo screenwidth .]
    if {$height>$screenheight-100} {
        set height [expr $screenheight-100]
    }
    if {$width>$screenwidth-100} {
        set width [expr $screenwidth-100]
    }
    $global_dispwin(cv) configure -width $width -height $height
}

#-----------------------------------------------------------------------------#

proc create_display {} {
  global global_setup global_dispwin global_num_channels
  global global_setup_pane global_setup_vis
  global   entry_ro

  set f $global_dispwin(frame)

# Eventrate
  if [winfo exists $f.ev] {destroy $f.ev}
  frame $f.ev -relief ridge
  label $f.ev.l0 -text "Events:" -anchor w
  label $f.ev.l1 -text "Count" -anchor w
  entry $f.ev.e1 -textvariable ro_status(count) -width 0 -state $entry_ro\
    -font $global_setup(termfont)
  label $f.ev.l2 -text "Rate" -anchor w
  entry $f.ev.e2 -textvariable ro_status(rate_disp) -width 0 -state $entry_ro\
    -font $global_setup(termfont)
  label $f.ev.l3 -text "Status" -anchor w
  entry $f.ev.e3 -textvariable ro_status(status_disp) -width 0 -state $entry_ro
  if {$global_setup(view_deadtime)} {
    label $f.ev.l4 -text "Deadtime" -anchor w
    entry $f.ev.e4 -textvariable ro_status(reldt_disp) -width 0 -state $entry_ro\
      -font $global_setup(termfont)
    label $f.ev.l5 -text "% =" -anchor w
    entry $f.ev.e5 -textvariable ro_status(dt_disp) -width 0 -state $entry_ro\
      -font $global_setup(termfont)
    label $f.ev.l6 -text "ms" -anchor w
  }
  pack $f.ev.l0 $f.ev.l1 $f.ev.e1 $f.ev.l2 $f.ev.e2 $f.ev.l3 $f.ev.e3 -side left
  if {$global_setup(view_deadtime)} {
    pack $f.ev.l4 $f.ev.e4 $f.ev.l5 $f.ev.e5 $f.ev.l6 -side left
  }
  pack $f.ev -side top

# Scaler
  if [winfo exists $f.sc] {destroy $f.sc}
  frame $f.sc
  set global_dispwin(cv) [canvas $f.sc.cv]
  scrollbar $f.sc.v -orient vertical -width 10 -command "$f.sc.cv yview"
  scrollbar $f.sc.h -orient horizontal -width 10 -command "$f.sc.cv xview"
  $f.sc.cv configure -xscrollcommand "$f.sc.h set"
  $f.sc.cv configure -yscrollcommand "$f.sc.v set"

  set sframe [frame $f.sc.cv.sc]
  set global_dispwin(sframe) $sframe
  for {set pane 0} {$pane<$global_setup(num_panes)} {incr pane} {
    frame $sframe.p$pane -relief ridge -borderwidth 2
    set frame [frame $sframe.p$pane.p]
    add_header $frame
    set row 0
    for {set chan 0} {$chan<$global_num_channels} {incr chan} {
      if {$global_setup_vis($chan) && $global_setup_pane($chan)==$pane} {
        incr row
        add_chan $chan $frame $row
      }
    }
    pack $sframe.p$pane -side left -fill y
    pack $frame -side top
  }

  $f.sc.cv create window 0 0 -window $sframe -anchor nw
  grid $f.sc.cv -sticky nsew
  grid $f.sc.v -row 0 -column 1 -sticky ns
  grid $f.sc.h -sticky ew
  grid columnconfigure $f.sc 0 -weight 1
  grid rowconfigure $f.sc 0 -weight 1
  pack $f.sc -side bottom -expand 1 -fill both
  update idletasks
  scaler_win_scale_canvas
}

#-----------------------------------------------------------------------------#

proc destroy_display {} {
  global global_dispwin

  set f $global_dispwin(frame)
  pack forget $f
  destroy $f.ev
  destroy $f.sc
}

#-----------------------------------------------------------------------------#

proc change_display {} {
  global global_running

  if {$global_running(numerical_display)} {
    destroy_display
    create_display
    repack_all
  }
}

#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
