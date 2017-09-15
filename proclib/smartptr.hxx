/*
 * smartptr.hxx
 * 
 * created: 15.06.97
 * 25.03.1998 PW: adapded for <string>
 */

#ifndef _smartptr_hxx_
#define _smartptr_hxx_

#include "config.h"
#include "cxxcompat.hxx"
#include <errors.hxx>
#include <nocopy.hxx>

/*****************************************************************************/

template <class T>
class T_smartptr: public nocopy
  {
  protected:
    T* p;
    STRING name;
  public:
    T_smartptr(STRING n) :p(0), name(n) {}
    ~T_smartptr() {delete p;}
    operator T*();    // cast auf T*
    operator T&();    // cast auf T&
    //operator int() {return p!=0;} // cast auf int
    T* operator ->();
    T& operator *();
    T* operator =(T* pp) {delete p; p=pp; return pp;}
    T** operator &() {return &p;}
    //int operator ==(T_smartptr&);
    //int operator !=(T_smartptr&);
    //int operator ==(void* i) {cerr<<"smart=="<<endl; return i==p;}
    //int operator !=(void* i) {cerr<<"smart!="<<endl; return i==p;}
    int operator !() {return !p;}
    int valid() const {return p!=0;}
  };

class C_ptr_error: public C_error
  {
  public:
    C_ptr_error(const C_ptr_error&);
    C_ptr_error(ostrstream&);
    C_ptr_error(STRING);
    C_ptr_error(const char*);
    virtual ~C_ptr_error();
  protected:
    STRING message_;
  public:
    static const int e_ptr;
    virtual int type() const {return(e_ptr);}
    virtual ostream& print(ostream&) const;
    virtual C_outbuf& bufout(C_outbuf&) const;
    virtual STRING message() const;
  };

#endif

/*****************************************************************************/
/*****************************************************************************/
