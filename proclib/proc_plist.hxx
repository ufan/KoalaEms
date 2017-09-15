/*
 * proc_plist.hxx
 * 
 * created: 05.09.95 PW
 * 12.06.1998 PW: adapted for STD_STRICT_ANSI
 * 24.02.1999 PW: add_par(float) and add_par(double) added
 * 16.03.1999 PW: is added to arguments for C_proclist::C_proclist
 */

#ifndef _proc_plist_hxx_
#define _proc_plist_hxx_

#include "config.h"
#include "cxxcompat.hxx"

#include <ems_err.h>
#include "caplib.hxx"
#include <outbuf.hxx>
#include "proc_modes.hxx"
#include "smartptr_t.hxx"

class C_VED;

/*****************************************************************************/
class C_instr_system;

class C_proclist
  {
  public:
    //C_proclist(C_VED* ved, C_capability_list* caplist);
    C_proclist(const char* text, C_VED* ved, C_instr_system* is,
        T_smartptr<C_capability_list>& caplist);
    C_proclist(const C_proclist&);
    ~C_proclist();

  private:
    C_VED* ved;
    C_instr_system* is;
    STRING pl_name;
    T_smartptr<C_capability_list>& caplist;
    ems_u32* list;
    int listsize;
    int list_idx;
    int proc_idx;
    int proc_num_;
    void listend();
    void growlist(int);

  public:
    STRING name() const;
    exec_mode execmode;
    void clear();
    void printlist(ostream&);
    int add_proc(const char*);
    int add_proc(int);
    int add_par(int);
    int mod_par(int pos, int);
    int add_par(ems_u32);
    int mod_par(int pos, ems_u32);
    //int add_par(float);
    //int add_par(double);
    int add_par(int, int, ...);
    int add_par(int, ems_u32*);
    int add_par(const char*);
    int send_request(int is);
    int getpos(void) const {return list_idx;}
    void getlist(ems_u32** list, int* size);
    int proc_num() const {return proc_num_;}
  };

#endif

/*****************************************************************************/
/*****************************************************************************/
