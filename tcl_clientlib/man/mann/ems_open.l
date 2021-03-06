.\" This manpage source uses rsml coding.
.so /usr/share/lib/tmac/sml
.so /usr/share/lib/tmac/rsml
.\"
.\"
.\" *********************************************************************
.\" *                                                                   *
.\" *      Forschungszentrum J�lich GmbH                                *
.\" *      Zentrallabor f�r Elektronik                                  *
.\" *      Peter W�stner                                                *
.\" *                                                                   *
.\" *********************************************************************
.\"
.TH ems_open n
.SH NAME
.PP
\*Lems_open\*O \- �ffnet eine Verbindung zu einem VED
.SH SYNOPSIS
.PP
.sS
\*Lems_open\*O name
.PP
\*Lems_open\*O name type
.PP
\*Lems_open\*O name type priority
.sE
.SH PARAMETERS
.PP
.VL 4m
.LI "\*Ostring \*Vname\*O"
VED-name
.LI "\*Oenum \*Vtype\*O"
{any, eventmanager, controller, camac, fastbus, chimaere, camac_chimaere,
fastbus_chimaere}
.nL
default: any
.LI "\*Oint \*Vpriority\*O"
Festlegung der Reihenfolge beim Starten des Readouts.
.nL
default: je nach typ:
.TS
tab(@);
l n.
    any@0
    eventmanager@4000
    andere@1000
.TE
.SH DESCRIPTION
.PP
\*Lems_open\*O �ffnet die Verbindung zu einer VED.
Der Typ des VED wird ben�tigt, damit die EMS-TCL-Bibliothek die passenden
Kommandos zur Verf�gung stellen kann.
F�r die Typen gibt es folgende Hierarchie:
.nL
                                any 
.nL
                               /   \\
.nL
                              /     \\
.nL
                             /       \\
.nL
                     eventmanager controller
.nL
                           \\       /   |  \\
.nL
                            \\     /    |   \\
.nL
                             \\   /     |    \\
.nL
                              \\ /      |     \\
.nL
                          chimaere   camac  fastbus
.nL
                              |   \\   /       |
.nL
                              |    \\ /        |
.nL
                              |     X         |
.nL
                              |    / \\        |
.nL
                              |   /   \\       |
.nL
                      camac_chimaere  fastbus_chimaere
.PP

.nL
Die priority ist nur n�tig, wenn der Readout f�r alle VEDs des Systems mit
einem Kommando gestartet werden soll. Das ist noch nicht implementiert.
\*Lems_open\*O erzeugt ein neues TCL-Kommando, das alle VED-Funktionen zur
Verf�gung stellt. Der Name des Kommandos ist der VED-Name oder der VED-Name
und ein Index. Er kann mittels \*Lrename\*O ge�ndert werden. Wenn das
VED-Kommando mit rename gel�scht wird, wird auch die Verbindung zum VED beendet.

.SH RETURN VALUE
Bei Erfolg wird der Name des erzeugten TCL-Kommandos zurckgegeben.
.nL
Im Fehlerfall wird eine Fehlerbeschreibung zur�ckgegeben.
.SH EXAMPLES
ems_open commu
.nL
ems_open commu controller
.nL
ems_open ikpf01 fastbus 200
.SH RELATED INFORMATION
\*Lems_connect\*O, \*Lems_veds\*O, \*L<VED> close\*O, \*Lrename\*O
.EQ
delim off
.EN
