//++
//      Copyright (c) 1996-99 Intel Corp.            
//
//
// Module Name:
//
//  palproc.s
//
// Abstract:
//
//  Contains an implementation for making PAL PROC calls on
//  IA-64 architecture.
//
//
//
// Revision History:
//
//--

	.file	"palproc.s"

#include "palproc.h"


//-----------------------------------------------------------------------------
//++
// MakeStaticPALCall
//
// This routine is called whenever an architected static calling convention
// based PAL call is to be made. This call does use RSE actually, but our policy
// in making static PAL calls before memory is available is to make sure that 
// we do not nest too deep and allocate beyond 96 banked registers. In other
// words we carefully code calls and control flow before memory is available.
//
// Arguments : All parameters set up to do static PAL call.
//
// On Entry :
//
// Return Value: 
// 
// As per static calling conventions. 
// 
//--
//---------------------------------------------------------------------------
PROCEDURE_ENTRY(MakeStaticPALCall)

        NESTED_SETUP (5,8,0,0)
        mov         loc3 = b5
        mov         loc4 = r2
        mov         loc7 = r1;;
        
        movl        loc6 = PAL_MC_CLEAR_LOG
        mov         r2 = psr;;
        mov         loc5 = r2

        cmp.eq      p6,p7 = r28,loc6;;
    (p7)movl        loc6 = PAL_MC_DYNAMIC_STATE;;
    (p7)cmp.eq      p6,p7 = r28,loc6;;        
        
    (p7)movl        loc6 = PAL_MC_ERROR_INFO;;
    (p7)cmp.eq      p6,p7 = r28,loc6;;        
         
    (p7)movl        loc6 = PAL_MC_RESUME;;
    (p7)cmp.eq      p6,p7 = r28,loc6        

        mov         loc6 = 0x1;;
    (p7)dep         r2 = loc6,r2,13,1;;         // psr.ic = 1

// p6 will be true, if it is one of the MCHK calls. There has been lots of debate
// on psr.ic for these values. For now, do not do any thing to psr.ic

//    (p6)dep         r2 = r0,r2,13,1;;         // psr.ic = 0
        dep         r2 = r0,r2,14,1;;           // psr.i = 0

        mov         psr.l = r2
        srlz.d;;                                // Needs data serailization.
        srlz.i;;                                // Needs instruction serailization.

StaticGetPALLocalIP:
        mov         loc2 = ip;;
        add         loc2 = StaticComeBackFromPALCall - StaticGetPALLocalIP,loc2;;
        mov         b0 = loc2                   // return address after Pal call
        mov         r28 = in1                   // get the input parameters to PAL call
        mov         r29 = in2
        mov         r30 = in3;;
        mov         r31 = in4
        mov         b5 =  in0;;                 // get the PalProcEntrypt from input
        br.sptk     b5                          // Take the plunge.

StaticComeBackFromPALCall:

        mov         psr.l = loc5;;
        srlz.d;;                                // Needs data serailization.
        srlz.i;;                                // Needs instruction serailization.

        mov         b5 = loc3
        mov         r2 = loc4
        mov         r1 = loc7
        
        NESTED_RETURN

PROCEDURE_EXIT(MakeStaticPALCall)


//-----------------------------------------------------------------------------
//++
// MakeStackedPALCall
//
// This routine is called whenever an architected stacked calling convention
// based PAL call is to be made. This call is made after memory is available.
// Although stacked calls could be made directly from 'C', there is a PAL 
// requirement which forces the index to be in GR28 and hence this stub is
// needed
//
// Arguments : All parameters set up to do stacted PAL call.
//
// On Entry :
//          in0:  PAL_PROC entrypoint 
//          in1-in4 : PAL_PROC arguments
//
// Return Value: 
// 
// As per stacked calling conventions. 
// 
//--
//---------------------------------------------------------------------------
PROCEDURE_ENTRY(MakeStackedPALCall)

        NESTED_SETUP (5,8,4,0)
        mov         loc3 = b5
        mov         loc4 = r2
        mov         loc7 = r1    
        mov         r2 = psr;;
        mov         loc5 = r2;;
        dep         r2 = r0,r2,14,1;;           // psr.i = 0
        mov         psr.l = r2
        srlz.d;;                                // Needs data serailization.
        srlz.i;;                                // Needs instruction serailization.

StackedGetPALLocalIP:
        mov         r28 = in1                   // get the input parameters to PAL call
        mov         out0 = in1
        mov         out1 = in2;;
        mov         out2 = in3
        mov         out3 = in4
        mov         b5 =  in0;;                 // get the PalProcEntrypt from input
        br.call.dpnt b0=b5;;                    // Take the plunge.

StackedComeBackFromPALCall:

        mov         psr.l = loc5;;
        srlz.d;;                                // Needs data serailization.
        srlz.i;;                                // Needs instruction serailization.
        mov         b5 = loc3
        mov         r2 = loc4
        mov         r1 = loc7
        
        NESTED_RETURN

PROCEDURE_EXIT(MakeStackedPALCall)

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
