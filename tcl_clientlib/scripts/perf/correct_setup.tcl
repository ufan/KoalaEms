#
# Diese Prozedur vervollstaendigt die Variablen, die von einem Konfigfile
# gelesen wurden. Als Ergebnis sind alle Setupvariablen vorhanden und
# initialisiert, deren Existenz spaeter vorausgesetzt wird
#

proc correct_setup {} {
  global global_setup

  if {![info exists global_setup(auto_save)]} {
    set global_setup(auto_save) 0}

  if {![info exists global_setup(auto_start)]} {
    set global_setup(auto_start) 0}

  if {![info exists global_setup(commu_transport)]} {
    set global_setup(commu_transport) default}

  if {![info exists global_setup(commu_host)]} {
    set global_setup(commu_host) localhost}

  if {![info exists global_setup(commu_socket)]} {
    set global_setup(commu_socket) /var/tmp/emscomm}

  if {![info exists global_setup(commu_port)]} {
    set global_setup(commu_port) 4096}

  if {![info exists global_setup(veds)]} {
    set global_setup(veds) {}}

  if {[info exists global_setup(updateinterval)] == 0} {
    set global_setup(updateinterval) 20}
}
