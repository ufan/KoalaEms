# $ZEL: gui.tcl,v 1.1 2000/07/15 21:37:01 wuestner Exp $
# copyright:
# 1999 P. Wuestner; Zentrallabor fuer Elektronik; Forschungszentrum Juelich
#
# global_setup
# global_perfframe
#

namespace eval ::GUI {

proc start {} {
  crate_main_win
}

proc crate_main_win {} {
  global global_setup global_pframe_grid

  menu .menubar
  set filemenu [create_menu_file .menubar]
  set setupmenu [create_menu_setup .menubar]
  set controlmenu [create_menu_control .menubar]
  set vedmenu [create_menu_ved .menubar]
# set helpmenu [create_menu_help .menubar]

  .menubar add cascade -label File -underline 0 -menu $filemenu
  .menubar add cascade -label Setup -underline 0 -menu $setupmenu
  .menubar add cascade -label Control -underline 0 -menu $controlmenu
  .menubar add cascade -label VED -underline 0 -menu $vedmenu
# .menubar add cascade -label Help -underline 0 -menu $helpmenu

  frame .bar -height 4 -width 10c -background black
  image create bitmap sfzj -foreground white -data {
#define schrumpf-logo_width 36
#define schrumpf-logo_height 33
static char schrumpf-logo_bits[] = {
 0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x04,0x00,0xf0,0x00,0xe0,0xff,0x00,0xf0,
 0x00,0xf8,0x7f,0x00,0xf0,0x00,0xfe,0x7f,0x00,0xf0,0x00,0xff,0x3f,0x00,0xf0,
 0x80,0xff,0x3f,0x00,0xf0,0xc0,0xff,0x1f,0x00,0xf0,0xe0,0xff,0x1f,0xc0,0xf0,
 0xe0,0xff,0x1f,0xc0,0xf1,0xf0,0xff,0x0f,0xc0,0xf1,0xf0,0xff,0x0f,0xe0,0xf3,
 0xf8,0xff,0x07,0xe0,0xf3,0xf8,0xff,0x07,0xf0,0xf3,0xf8,0xff,0x07,0xf0,0xf7,
 0xf8,0xff,0x03,0xf8,0xf7,0xf8,0xff,0x03,0xf8,0xf7,0xfc,0xff,0x01,0xf8,0xf7,
 0xf8,0xf8,0x01,0xfc,0xf7,0x78,0xf8,0x00,0xfc,0xf7,0x18,0x60,0x00,0xfe,0xf7,
 0x08,0x00,0x00,0xfe,0xf3,0x00,0x00,0x00,0xff,0xf3,0x00,0x00,0x80,0xff,0xf3,
 0x00,0x00,0x80,0xff,0xf1,0x00,0x00,0xc0,0xff,0xf1,0x00,0x00,0xe0,0xff,0xf0,
 0x00,0x00,0xf0,0x7f,0xf0,0x00,0x00,0xfc,0x7f,0xf0,0x00,0x03,0xff,0x3f,0xf0,
 0x00,0xfe,0xff,0x1f,0xf0,0x00,0xfc,0xff,0x07,0xf0,0x00,0xf0,0xff,0x01,0xf0
 };}

  set global_pframe_head [frame .pfh -borderwidth 3 -relief ridge]
  label .pfh.l_trate -text {Triggered Rate [/s]}
  label .pfh.l_mrate -text {Measured Rate [/s]}
  label .pfh.l_tdtac -text {Total Dead Time (computed) [ms]}
  label .pfh.l_tdtam -text {Total Dead Time (measured) [ms]}
  label .pfh.l_tdtr -text {Total Dead Time (computed) [%]}
  entry .pfh.e_trate -textvariable global_trate -state disabled
  entry .pfh.e_mrate -textvariable global_mrate -state disabled
  entry .pfh.e_tdtac -textvariable global_deadtimec_ms -state disabled
  entry .pfh.e_tdtam -textvariable global_deadtimem_ms -state disabled
  entry .pfh.e_tdtr -textvariable global_deadtime_perc -state disabled
  grid .pfh.l_trate .pfh.e_trate -sticky ew
  grid .pfh.l_mrate .pfh.e_mrate -sticky ew
  grid .pfh.l_tdtac .pfh.e_tdtac -sticky ew
  grid .pfh.l_tdtam .pfh.e_tdtam -sticky ew
  grid .pfh.l_tdtr .pfh.e_tdtr -sticky ew
  set global_pframe_grid [frame .pfg -borderwidth 3 -relief ridge]

  #frame .pf.xbar -height 40 -width 10c -background blue
  #pack .pf.xbar -side top -fill x

  pack .bar -side top -fill x
  LOG::create .logframe
  frame .logo_ -borderwidth 2 -relief flat
  frame .logo_.logo -background #01a0c6
  label .logo_.logo.t -text "Forschungszentrum J�lich / Zentralinstitut f�r Elektronik"\
    -background #01a0c6 -foreground white -cursor {} -padx 2 -pady 2\
    -font {Helvetica -16 bold roman}
  label .logo_.logo.l -image sfzj -background #01a0c6
  pack .logo_.logo.t -side left -fill x -expand 1
  pack .logo_.logo.l -side left
  pack .logo_.logo -side bottom -fill x
  pack .logo_ -side bottom -fill x
#  pack .logframe -side bottom -fill both -expand 1
  . configure -menu .menubar
  pack .pfh -side top -expand 1 -fill x
  pack .pfg -side top -fill x -expand 1
}

}
