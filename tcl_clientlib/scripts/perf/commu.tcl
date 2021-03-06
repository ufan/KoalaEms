#
# global vars in this file:
#
# enum    global_setup(commu_transport) / {default, unix, tcp}
# string  global_setup(commu_host)
# string  global_setup(commu_socket)
# int     global_setup(commu_port)
# boolean global_setup(offline)
#

proc connect_commu {} {
  global dialog_nr
  global global_setup
  if [ems_connected] {
    ems_disconnect
  }
  if [catch {
    switch $global_setup(commu_transport) {
      default {ems_connect}
      unix    {ems_connect $global_setup(commu_socket)}
      tcp     {ems_connect $global_setup(commu_host) $global_setup(commu_port)}
    }} mist] {
    set res -1
    set win .error_$dialog_nr
    incr dialog_nr
    tk_dialog $win Error "$mist" {} 0 Dismiss
  } else {
    set res 0
  }
  return $res
}
