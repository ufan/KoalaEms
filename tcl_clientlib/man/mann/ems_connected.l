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
.TH ems_connected n
.SH NAME
.PP
\*Lems_connected\*O \- testet Existenz einer Verbindung zum
Kommunikationsproze�
.SH SYNOPSIS
.PP
.sS
\*Lems_connected\*O
.sE
.SH PARAMETERS
.PP
.VL 4m
.LI "\*Okeine"
.LE
.SH DESCRIPTION
.PP
\*Lems_connected\*O stellt fest, ob eine Verbindung zu einem
Kommunikationsproze� aufgenommen wurde.
.SH RETURN VALUE
Wenn eine Verbindung existiert, wird "1" zur�ckgegeben.
.nL
Wenn keine Verbindung existiert, wird "0" zur�ckgegeben.
.SH RELATED INFORMATION
\*Lems_connect\*O, \*Lems_disconnect\*O
.EQ
delim off
.EN
