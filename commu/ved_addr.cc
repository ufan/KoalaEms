/*
 * ved_addr.cc
 * 
 * created 21.01.95
 */

#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ved_addr.hxx>
#include <xdrstring.h>
#include <xdrstrdup.hxx>
#include <errors.hxx>
#include "versions.hxx"

VERSION("2014-07-11", __FILE__, __DATE__, __TIME__,
"$ZEL: ved_addr.cc,v 2.14 2014/07/14 15:12:20 wuestner Exp $")
#define XVERSION

using namespace std;

/*****************************************************************************/

C_VED_addr::C_VED_addr()
{}

/*****************************************************************************/

C_VED_addr::C_VED_addr(C_inbuf&)
{}

/*****************************************************************************/

C_VED_addr::C_VED_addr(const C_VED_addr&)
{}

/*****************************************************************************/

C_VED_addr::~C_VED_addr()
{}

/*****************************************************************************/

ostream& C_VED_addr::print(ostream& os) const
{
os << "invalid";
return(os);
}

/*****************************************************************************/

void C_VED_addr::out_ved_addr(C_outbuf& outbuf) const
{
outbuf << notvalid;
}

/*****************************************************************************/

int C_VED_addr::outsize() const
{
return(1);
}

/*****************************************************************************/

int C_VED_addr::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

C_VED_addr_builtin::C_VED_addr_builtin(const string& name)
:name_(name)
{}

/*****************************************************************************/

C_VED_addr_builtin::C_VED_addr_builtin(C_inbuf& inbuf)
{
inbuf >> name_;
}

/*****************************************************************************/

C_VED_addr_builtin::~C_VED_addr_builtin()
{}

/*****************************************************************************/

C_VED_addr_builtin::C_VED_addr_builtin(const C_VED_addr_builtin& addr)
:name_(addr.name_)
{}

/*****************************************************************************/

ostream& C_VED_addr_builtin::print(ostream& os) const
{
os << "builtin       : " << name_;
return(os);
}

/*****************************************************************************/

void C_VED_addr_builtin::out_ved_addr(C_outbuf& outbuf) const
{
outbuf << builtin;
outbuf << name_;
}

/*****************************************************************************/

int C_VED_addr_builtin::outsize() const
{
return(1);
}

/*****************************************************************************/

int C_VED_addr_builtin::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr_builtin::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

C_VED_addr_unix::C_VED_addr_unix(const string& socket)
:sockname_(socket)
{}

/*****************************************************************************/

C_VED_addr_unix::C_VED_addr_unix(C_inbuf& inbuf)
{
inbuf >> sockname_;
}

/*****************************************************************************/

C_VED_addr_unix::C_VED_addr_unix(const C_VED_addr_unix& addr)
:sockname_(addr.sockname_)
{}

/*****************************************************************************/

C_VED_addr_unix::~C_VED_addr_unix()
{}

/*****************************************************************************/

ostream& C_VED_addr_unix::print(ostream& os) const
{
os << "unix          : " << sockname_;
return(os);
}

/*****************************************************************************/

void C_VED_addr_unix::out_ved_addr(C_outbuf& outbuf) const
{
outbuf << local << sockname_;
}

/*****************************************************************************/

int C_VED_addr_unix::outsize() const
{
return(1+strxdrlen(sockname_.c_str()));
}

/*****************************************************************************/

int C_VED_addr_unix::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
C_VED_addr_unix* addr_=(C_VED_addr_unix*)&addr;
if (addr_->sockname_!=sockname_) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr_unix::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

C_VED_addr_inet::C_VED_addr_inet(const string& host, unsigned short port)
:hostname_(host), port_(port)
{}

/*****************************************************************************/

C_VED_addr_inet::C_VED_addr_inet(C_inbuf& inbuf)
{
(inbuf >> hostname_) >> port_;
}

/*****************************************************************************/

C_VED_addr_inet::C_VED_addr_inet(const C_VED_addr_inet& addr)
:hostname_(addr.hostname_), port_(addr.port_)
{}

/*****************************************************************************/

C_VED_addr_inet::~C_VED_addr_inet()
{}

/*****************************************************************************/

ostream& C_VED_addr_inet::print(ostream& os) const
{
os << "inet          : " << setw(12) << hostname_ << ": " << port_;
return(os);
}

/*****************************************************************************/

void C_VED_addr_inet::out_ved_addr(C_outbuf& outbuf) const
{
outbuf << inet << hostname_ << port_;
}

/*****************************************************************************/

int C_VED_addr_inet::outsize() const
{
return(1+strxdrlen(hostname_.c_str())+1);
}

/*****************************************************************************/

int C_VED_addr_inet::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
C_VED_addr_inet* addr_=(C_VED_addr_inet*)&addr;
if (addr_->hostname_!=hostname_) return(0);
if (addr_->port_!=port_) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr_inet::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

C_VED_addr_inet_path::C_VED_addr_inet_path(const string& host,
    unsigned short portnum, C_strlist* pathes)
:C_VED_addr_inet(host, portnum), pathes(pathes)
{}

/*****************************************************************************/

C_VED_addr_inet_path::C_VED_addr_inet_path(C_inbuf& inbuf)
:C_VED_addr_inet(inbuf)
{
int n, i;

pathes=new C_strlist(n=inbuf.getint());
for (i=0; i<n; i++)
  {
  string s;
  inbuf >> s;
  pathes->set(i, s.c_str());
  }
}

/*****************************************************************************/

C_VED_addr_inet_path::C_VED_addr_inet_path(const C_VED_addr_inet_path& addr)
:C_VED_addr_inet(addr), pathes(new C_strlist(*addr.pathes))
{}

/*****************************************************************************/

C_VED_addr_inet_path::~C_VED_addr_inet_path()
{
delete pathes;
}

/*****************************************************************************/

ostream& C_VED_addr_inet_path::print(ostream& os) const
{
int i;
os << "inet with path: " << setw(12) << hostname_ << " :" << port_;
for (i=0; i<pathes->size(); i++) os << " " << (*pathes)[i];
return(os);
}

/*****************************************************************************/

void C_VED_addr_inet_path::out_ved_addr(C_outbuf& outbuf) const
{
int i;

outbuf << inet_path << hostname_ << port_ << pathes->size();
for (i=0; i<pathes->size(); i++) outbuf << (*pathes)[i];
}

/*****************************************************************************/

int C_VED_addr_inet_path::outsize() const
{
int num, i;

num=3;
for (i=0; i<num; i++) num+=strxdrlen((*pathes)[i]);
return(num);
}

/*****************************************************************************/

int C_VED_addr_inet_path::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
C_VED_addr_inet_path* addr_=(C_VED_addr_inet_path*)&addr;
if (addr_->hostname_!=hostname_) return(0);
if (addr_->port_!=port_) return(0);
int size, i;
if ((size=addr_->pathes->size())!=pathes->size()) return(0);
for (i=0; i<size; i++)
  if (strcmp((*addr_->pathes)[i], (*pathes)[i])) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr_inet_path::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

C_VED_addr_vic::C_VED_addr_vic()
{}

/*****************************************************************************/
/*
C_VED_addr_vic::C_VED_addr_vic(const int*& ptr, int size)
{}
*/
/*****************************************************************************/

C_VED_addr_vic::C_VED_addr_vic(C_inbuf&)
{}

/*****************************************************************************/

C_VED_addr_vic::C_VED_addr_vic(const C_VED_addr_vic&)
{}

/*****************************************************************************/

C_VED_addr_vic::~C_VED_addr_vic()
{}

/*****************************************************************************/

ostream& C_VED_addr_vic::print(ostream& os) const
{
os << "VICbus";
return(os);
}

/*****************************************************************************/

void C_VED_addr_vic::out_ved_addr(C_outbuf& outbuf) const
{
outbuf << VICbus;
}

/*****************************************************************************/

int C_VED_addr_vic::outsize() const
{
return(1);
}

/*****************************************************************************/

int C_VED_addr_vic::operator ==(const C_VED_addr& addr) const
{
if (addr.type()!=type()) return(0);
return(1);
}

/*****************************************************************************/

int C_VED_addr_vic::operator !=(const C_VED_addr& addr) const
{
return(!operator==(addr));
}

/*****************************************************************************/

ostream& operator <<(ostream& os, const C_VED_addr& rhs)
{
return(rhs.print(os));
}

/*****************************************************************************/

ostream& operator <<(ostream& os, const C_VED_addr* rhs)
{
return(rhs->print(os));
}

/*****************************************************************************/

C_VED_addr* extract_ved_addr(C_inbuf& inbuf)
{
C_VED_addr* addr;
C_VED_addr::types typ;
switch (typ=(C_VED_addr::types)inbuf.getint())
  {
  case C_VED_addr::notvalid:  addr=new C_VED_addr(inbuf); break;
  case C_VED_addr::builtin:   addr=new C_VED_addr_builtin(inbuf); break;
  case C_VED_addr::local:     addr=new C_VED_addr_unix(inbuf); break;
  case C_VED_addr::inet:      addr=new C_VED_addr_inet(inbuf); break;
  case C_VED_addr::inet_path: addr=new C_VED_addr_inet_path(inbuf); break;
  case C_VED_addr::VICbus:    addr=new C_VED_addr_vic(inbuf); break;
  default:
    {
    ostringstream s;
    s << "extract_ved_addr(C_inbuf&): unknown type " << typ << ends;
    throw new C_program_error(s);
    }
  }
return(addr);
}

/*****************************************************************************/

C_outbuf& operator <<(C_outbuf& outbuf, const C_VED_addr& addr)
{
addr.out_ved_addr(outbuf);
return(outbuf);
}

/*****************************************************************************/
/*****************************************************************************/
