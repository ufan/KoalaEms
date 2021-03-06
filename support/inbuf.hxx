/*
 * support/inbuf.hxx
 * 
 * created 04.02.95 PW
 * 
 * $ZEL: inbuf.hxx,v 2.21 2016/05/02 15:20:06 wuestner Exp $
 */

#ifndef _inbuf_hxx_
#define _inbuf_hxx_

#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <sys/time.h>

#include "buf.hxx"

using namespace std;

/*****************************************************************************/

class C_inbuf: public C_buf
  {
  public:
    C_inbuf();
    C_inbuf(const int*, int);
    C_inbuf(const unsigned int*, int);
    C_inbuf(const int*);
    C_inbuf(const unsigned int*);
    C_inbuf(int);
    C_inbuf(const string&); // filename
    C_inbuf(const C_buf&);
    virtual ~C_inbuf();
  private:
    typedef C_inbuf& (*inbuf_manip) (C_inbuf&);
  protected:
  public:
    virtual int empty() const;// {return idx>=size_;}
    int index() const {return idx;}
    int index(int);
    int remaining(void) const {return dsize-idx;}
//    const unsigned int& operator * () const;
    unsigned int getint();
    ems_u32* wbuf() const {return buffer;}
    operator int() {return idx<dsize;}
    C_inbuf& operator>>(int&);
    C_inbuf& operator>>(unsigned int&);
    C_inbuf& operator>>(ems_i64&);
    C_inbuf& operator>>(ems_u64&);
    //C_inbuf& operator>>(float&);
    //C_inbuf& operator>>(double&);
    C_inbuf& operator>>(short&);
    C_inbuf& operator>>(unsigned short&);
    C_inbuf& operator>>(char&);
    C_inbuf& operator>>(char*&);
    C_inbuf& operator>>(string&);
    C_inbuf& operator>>(struct timeval&);
    C_inbuf& operator>>(inbuf_manip);
    C_inbuf& operator ++();
    C_inbuf& operator ++(int);
    C_inbuf& operator --();
    C_inbuf& operator --(int);
    C_inbuf& operator +=(int);
    C_inbuf& operator -=(int);
    void getopaque(void* arr, int size);
  };

#endif

/*****************************************************************************/
/*****************************************************************************/
