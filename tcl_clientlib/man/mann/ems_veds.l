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
.TH ems_veds n
.SH NAME
.PP
\*Lems_veds\*O \- listet bekannte VEDs
.SH SYNOPSIS
.PP
.sS
\*Lems_veds\*O
.sE
.SH PARAMETERS
.PP
.VL 4m
.LI "\*Okeine"
.LE
.SH DESCRIPTION
.PP
\*Lems_veds\*O erzeugt eine Liste mit allen VEDs, die der Kommunikationsproze�
kennt.
.SH RETURN VALUE
Bei Erfolg wird ein leerer String zur�ckgegeben.
.nL
Im Fehlerfall wird eine Fehlerbeschreibung zur�ckgegeben.
.SH RELATED INFORMATION
\*Lems_connect\*O, \*Lems_open\*O
.EQ
delim off
.EN
