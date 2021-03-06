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
.TH ved_name n
.SH NAME
.PP
\*L<VED> name\*O \- liefert den Namen eines VED
.SH SYNOPSIS
.PP
.sS
\*L<VED> name\*O
.sE
.SH PARAMETERS
.PP
.VL 4m
.LI "\*Okeine"
.LE
.SH DESCRIPTION
.PP
\*L<VED> name\*O liefert den namen eines ge�ffneten VEDs.
.SH RETURN VALUE
Bei Erfolg wird der VED-Name zur�ckgegeben.
.nL
Im Fehlerfall wird eine Fehlerbeschreibung zur�ckgegeben.
.SH RELATED INFORMATION
\*L<VED> open\*O
.EQ
delim off
.EN
