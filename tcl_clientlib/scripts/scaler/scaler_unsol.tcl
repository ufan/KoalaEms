# $ZEL: scaler_unsol.tcl,v 1.4 2014/08/19 20:20:50 wuestner Exp $
# copyright 1998
#   P. Wuestner; Zentralinstitut fuer Elektronik; Forschungszentrum Juelich
#
# global vars in this file:
#
# global_running(outer_loop)
# global_after(outer_loop)
#

proc ems_unsol_bye {} {
  global global_after global_running

  output "ems_bye received from commu"
  update idletasks
  if {$global_running(outer_loop)} {
    foreach i [array names global_after] {
      after cancel $global_after($i)
    }
    disconnect_commu
    output "disconnected; will try to reconnect in 60 seconds"
    set global_after(outer_loop) [after 60000 outer_loop]
  }
}
