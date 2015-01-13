/*
//  ALPHA_IMPLVER_AMASK_FEATURES.C V1.1
//  Retrieve the Alpha microprocessor family and features
//
//  Author: Steve Hoffman, Digital Equipment Corporation
//  hoffman@xdelta.enet.dec.com, 3-Feb-1997.
//
//  DEC C V5 and later under OpenVMS.
//  Also operates under DEC C on Digital UNIX.
//
//  See the Alpha Architecture Reference Manual for the current
//  microprocessor family codes, as well as the instruction set
//  extension definition codes.
*/
#include <stdio.h>
#include <c_asm.h>
main()
    {
    int implver = asm("implver %r0;");
    int amask = asm("amask %a0,%r0;", -1 );
    int definedbits = 0;

    printf("Alpha Microprocessor Family:\n");
    /*
    //  The "microprocessor family" includes the named microprocessor,
    //  and various microprocessors (if any) derived from it.
    */
    printf("  DECchip 21%1.1d64 or variant\n", implver );
    printf("  (EV%1.1d microprocessor core)\n", implver + 4 );

    /*
    //  The amask bit is *clear* if the capability is *present*...
    */
    printf("Alpha extensions available:\n");
    if ( !~amask ) printf("  none.\n");
    definedbits |= 1L<<0;
    if ( ~amask & 1L<<0 ) printf("  byte-word, et al.\n");
    definedbits |= 1L<<1;
    if ( ~amask & 1L<<1 ) printf("  square root, et al.\n");
    definedbits |= 1L<<8;
    if ( ~amask & 1L<<8 ) printf("  Audio-video, et al.\n");
    if ( ~amask & ~definedbits ) printf("  Unknown capability bit(s) present.");
    return 1;
    }

