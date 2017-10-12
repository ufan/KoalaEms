# $Id: commu.tcl,v 1.4 1999/04/09 12:32:30 wuestner Exp $
# copyright:
# 1998 P. W�stner; Zentrallabor f�r Elektronik; Forschungszentrum J�lich
#
# global vars:
#
# enum    global_setup(commu_transport) / {default, unix, tcp}
# string  global_setup(commu_host)
# string  global_setup(commu_socket)
# int     global_setup(commu_port)
#

proc setup_commu {} {
#  global commu

  set confmode [ems_confmode synchron]
  if [catch {set commu [ems_open commu_l]} mist] {
    output "open commu_l: $mist"
  } else {
    if [catch {$commu command {globalunsol {1}}} mist] {
      output "command globalunsol 1: $mist"
      output_append "Your commu seems to be too old."
    }
  }
  ems_confmode $confmode
}

proc connect_commu {} {
  global global_setup global_commuerror_x

  if [catch {
    switch $global_setup(commu_transport) {
      default {ems_connect}
      unix    {ems_connect $global_setup(commu_socket)}
      tcp     {ems_connect $global_setup(commu_host) $global_setup(commu_port)}
    }} mist] {
    set res -1
    output "connect to commu: $mist" tag_red
  } else {
    output "connected to commu via [ems_connection]"
#    setup_commu
    ems_unsolcommand ServerDisconnect {server_disconnected %v}
    ems_unsolcommand Bye commu_bye
    set res 0
  }
  return $res
}
