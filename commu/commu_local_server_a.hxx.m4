/*
 * commu_local_server_a.hxx.m4
 * 
 * $ZEL: commu_local_server_a.hxx.m4,v 2.8 2009/08/21 22:02:28 wuestner Exp $
 * 
 * created 07.12.94 PW
 */

#ifndef _commu_local_server_a_hxx_
#define _commu_local_server_a_hxx_

#include "commu_local_server.hxx"

/*****************************************************************************/

class C_local_server_a: public C_local_server
  {
  public:
    C_local_server_a(const char*, en_policies);
    virtual ~C_local_server_a();

    typedef struct
      {
      plerrcode (C_local_server_a::*proc)(const ems_u32*);
      plerrcode (C_local_server_a::*test_proc)(const ems_u32*);
      const char* name_proc;
      int* ver_proc;
      } listproc;

    typedef struct
      {
      procprop* (C_local_server_a::*prop_proc)();
      } listprop;

  protected:
    C_VED_addr_builtin addr;

    define(`proc',`plerrcode proc_$1(const ems_u32* ptr);
    plerrcode test_proc_$1(const ems_u32* ptr);
    procprop* prop_proc_$1();
    static const char name_proc_$1[];
    static int ver_proc_$1;')
    include(SRC/commu_procs_a.inc)

    static int NrOfProcs;
    static listproc Proc[];
    static listprop Prop[];

    virtual errcode Nothing(const ems_u32* ptr, int num);
    virtual errcode Initiate(const ems_u32* ptr, int num);
    virtual errcode Conclude(const ems_u32* ptr, int num);
    virtual errcode Identify(const ems_u32* ptr, int num);
    virtual errcode GetNameList(const ems_u32* ptr, int num);
    virtual errcode GetCapabilityList(const ems_u32* ptr, int num);
    virtual errcode GetProcProperties(const ems_u32* ptr, int num);
    virtual errcode DoCommand(const ems_u32* ptr, int num);
    virtual errcode TestCommand(const ems_u32* ptr, int num);
    errcode test_proclist(const ems_u32* ptr, int num);
    errcode scan_proclist(const ems_u32* ptr);
  public:
    virtual const C_VED_addr& get_addr() const {return(addr);}
  };

#endif

/*****************************************************************************/
/*****************************************************************************/
