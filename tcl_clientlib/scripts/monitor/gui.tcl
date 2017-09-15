# $Id: gui.tcl,v 1.1 1998/09/13 19:19:29 wuestner Exp $
# � 1998 P. W�stner; Zentrallabor f�r Elektronik; Forschungszentrum J�lich
#
# global vars:
#
# 

#-----------------------------------------------------------------------------#
proc start_gui {} {
  crate_main_win
  update idletasks
  wm deiconify .
}
#-----------------------------------------------------------------------------#
proc crate_main_win {} {
  global global_dispwin global_setup

  frame .menuframe
  frame .bar -height 3 -background #f00
  frame .disp
  message .message -relief ridge
  label .time -textvariable global_time -relief ridge
  set filemenu [create_menu_file .menuframe]
  pack $filemenu -side left -padx 1m

  pack .menuframe .bar -side top -fill x
  pack .time -side bottom -fill both
  pack .message -side bottom -fill both
  pack .disp -side bottom -fill both -expand 1
  create_display
}
#-----------------------------------------------------------------------------#
proc putm {text} {
  .message configure -text $text
}
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
