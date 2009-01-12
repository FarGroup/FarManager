#include <all_lib.h>
#pragma hdrstop
#pragma package(smart_init)

USERES("std_sP.res");
USEUNIT("tr_sock.cpp");
USEUNIT("cs.cpp");
USEUNIT("dataptr.cpp");
USEUNIT("db_math.cpp");
USEUNIT("disk_io.cpp");
USEUNIT("fhold.cpp");
USEUNIT("fielddef.cpp");
USEUNIT("hbtree.cpp");
USEUNIT("hcvvalue.cpp");
USEUNIT("hmdpool.cpp");
USEUNIT("hregtree.cpp");
USEUNIT("hstvalue.cpp");
USEUNIT("htree.cpp");
USEUNIT("hvalue.cpp");
USEUNIT("io_attr.cpp");
USEUNIT("io_cfg.cpp");
USEUNIT("io_cmp.cpp");
USEUNIT("io_dent.cpp");
USEUNIT("io_err.cpp");
USEUNIT("io_exec.cpp");
USEUNIT("io_execc.cpp");
USEUNIT("io_fent.cpp");
USEUNIT("io_fenum.cpp");
USEUNIT("io_find.cpp");
USEUNIT("io_info.cpp");
USEUNIT("io_part.cpp");
USEUNIT("io_patt.cpp");
USEUNIT("io_qpath.cpp");
USEUNIT("io_read.cpp");
USEUNIT("io_relp.cpp");
USEUNIT("io_sch.cpp");
USEUNIT("io_scr.cpp");
USEUNIT("io_split.cpp");
USEUNIT("mc_hsobj.cpp");
USEUNIT("mc_msg.cpp");
USEUNIT("mclasses.cpp");
USEUNIT("mu_args.cpp");
USEUNIT("mu_case.cpp");
USEUNIT("mu_clock.cpp");
USEUNIT("mu_cp.cpp");
USEUNIT("mu_itoa.cpp");
USEUNIT("mu_key.cpp");
USEUNIT("mu_os.cpp");
USEUNIT("mu_runch.cpp");
USEUNIT("mu_sal.cpp");
USEUNIT("mu_scol.cpp");
USEUNIT("mu_str.cpp");
USEUNIT("mu_text.cpp");
USEUNIT("mutils.cpp");
USEUNIT("parsr.cpp");
USEUNIT("period.cpp");
USEUNIT("plog.cpp");
USEUNIT("sock.cpp");
USEUNIT("SQLTypes.cpp");
USEUNIT("std_mem.cpp");
USEUNIT("std_sprt.cpp");
USEUNIT("threads.cpp");
USEUNIT("timer.cpp");
USEUNIT("chknew.cpp");
USEUNIT("htime.cpp");
USEUNIT("mcString.cpp");
USEUNIT("2asm.cpp");
USEUNIT("bsArray.cpp");
USEUNIT("xxSym.cpp");
USEUNIT("xxH.cpp");
USEUNIT("xxStack.cpp");
USEUNIT("std_strc.cpp");
USEUNIT("shcfg1.cpp");
USEUNIT("plog_in.cpp");
USEPACKAGE("Vcl50.bpi");
USEUNIT("lzhs.cpp");
USEUNIT("lzh.cpp");
USEUNIT("lzhf.cpp");
USEUNIT("mu_crc32.cpp");
USEUNIT("hstream.cpp");
USEUNIT("hs_file.cpp");
USEUNIT("xml.cpp");
USEUNIT("hdbase.cpp");
USEUNIT("hdb_mem.cpp");
USEUNIT("hdb_rd.cpp");
USEUNIT("hdb_wr.cpp");
USEUNIT("plog_s.cpp");
USEUNIT("hs_ofile.cpp");
USEUNIT("mu_sig.cpp");
USEUNIT("fio.cpp");
USEUNIT("HThread.cpp");
USEUNIT("xxExcptU.cpp");
USEUNIT("hcfg_Sh.cpp");
USEUNIT("HTreeCfg.cpp");
USEUNIT("io_dpout.cpp");
USEUNIT("mu_bprnt.cpp");
USEUNIT("xxExcpt.cpp");
USEUNIT("Win\WFileIO.cpp");
USEUNIT("Win\hwinmod.cpp");
USEUNIT("Win\HSend.cpp");
USEUNIT("sprintf_.cpp");
//---------------------------------------------------------------------------
namespace Std_sp {
  void MYRTLEXP PACKAGE Register() {
  };
};

//   Package source.
//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    if ( reason == DLL_PROCESS_ATTACH ) {
      char nm[ MAX_PATH_SIZE ];
      nm[ GetModuleFileName(NULL,nm,sizeof(nm)-1) ] = 0;
      FILELog( "STDS(%08X) Loaded by: [%s]",GetHInstance(),nm );
    }
 return 1;
}
//---------------------------------------------------------------------------
