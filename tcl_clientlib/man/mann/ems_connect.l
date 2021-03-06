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
.TH ems_connect n
.SH NAME
.PP
\*Lems_connect\*O \- baut die Verbindung zu einem Kommunikationsproze� auf
.SH SYNOPSIS
.PP
.sS
\*Lems_connect\*O
.PP
\*Lems_connect\*O socket
.PP
\*Lems_connect\*O host port
.sE
.SH PARAMETERS
.PP
.VL 4m
.LI "\*Ostring \*Vsocket\*O"
Name eines Unix-Domain-Sockets
.LI "\*Ostring \*Vhost\*O"
Hostname oder IP-Adresse des Rechners, auf dem der Kommunikationsproze�
l�uft.
.LI "\*Oint \*Vport\*O"
Port, der f�r die Verbindung verwendet werden soll.
.LE
.SH DESCRIPTION
.PP
\*Lems_connect\*O stellt die Verbindung zu einem Kommunikationsproze� ('commu')
her. Wenn keine Argumente angegeben sind, werden die Environmentvariablen
\*LEMSSOCKET\*O, \*LEMSHOST\*O und \*LEMSPORT\*O ausgewertet:
.VL 4m
.LI "\*Okeine Angabe:"
es wird socket=/var/tmp/emscomm verwendet.
.LI "\*LEMSSOCKET\*O definiert:"
\*LEMSSOCKET\*O wird als socket verwendet.
.LI "\*LEMSHOST\*O definiert:"
\*LEMSSOCKET\*O wird als host und \*LEMSPORT\*O als port verwendet. Default
f�r port ist 4096.
.LE
.PP
\*LEMSSOCKET\*O und \*LEMSHOST\*O d�rfen nicht gleichzeitig definiert sein.
.nL
Es kann zu jeder Zeit nur eine Verbindung zu einem Kommunikationsproze�
existieren.
.nL
Wenn \*Lems_open\*O oder \*Lems_veds\*O aufgerufen wird, ohne da� vorher
\*Lems_connect\*O aufgerufen wurde, wird \*Lems_connect\*O ohne Argumente
implizit aufgerufen.
.SH RETURN VALUE
Bei Erfolg wird ein leerer String zur�ckgegeben.
.nL
Im Fehlerfall wird eine Fehlerbeschreibung zur�ckgegeben.
.SH EXAMPLES
ems_connect
.nL
ems_connect /var/tmp/communication
.nL
ems_connect ikpalex 4000
.SH RELATED INFORMATION
\*Lems_disconnect\*O, \*Lems_connected\*O, \*Lems_connection\*O,
\*Lems_veds\*O, \*Lems_open\*O
.EQ
delim off
.EN
