#define MAIN
/*
Fast DGEMM routine with Blocking and without the inner submatrix copy. 
Smart OS should care of the shortage of TLB for programs like this.
If your machine/OS does not exploit good performance with this program
it may not be because of this program but because of the narrow bandwidth
of memory or too little TLB or dummb OS.

You can do benchmark of your system and/or OS with this program.
Compile with -DMAIN will produce the benchmark binary.
If you run on a specific machine/OS which is not described on my WEB,
please report the result to me with the information of the compiler
and operating system and their version.

						Last updated date : 99.09.14

Copyright 1998,1999 by Naohiko Shimizu <nshimizu@et.u-tokai.ac.jp>

All rights reserved.

The blocking algorithm and strategy for the blocking factor is my
original. If you want to use the same algorithm or to use the same
strategy for the blocking factor, you must preserve the copyright 
notice to the all programs and products which is derivable from this 
program and/or description.

Contact information:

Dr. Naohiko Shimizu
School of Engr. Tokai Univ.
1117 Kitakaname Hiratsuka
Kanagawa 259-1292 Japan
email: nshimizu@et.u-tokai.ac.jp
URL: http://shimizu-lab.et.u-tokai.ac.jp

You can call this subroutine from C or FORTRAN.
Only non-transposed routine is implemented.
This program is not alpha specific. You can compile and run
for many other architectures.

Some benchmarking results with my and my university's machines
are on the WEB:

http://shimizu-lab.et.u-tokai.ac.jp/pgm/gemm/bench.html

C:
       void dgemms_nn(int m, int n, int k, 
	     double alpha, double *a, int lda, double *b, int ldb, 
	     double beta, double *c, int ldc);

*CATION* C user should know that the matrices is assumed to be stored
in the FORTRAN order(Row Major).

FORTRAN:
       call dgemms_nn(m,n,k,alpha,a,lda,b,ldb,beta,c,ldc)

For benchmarking I placed main C routine. Use -DMAIN option to
compile for benchmaking.

Compile options:

-DMAIN	: compile for benchmarking
-Dev6	: compile for 21264
-Dev4	: compile for 21064(A)
-DRPCC	: use RPCC instruction to measure the time
-DNOPREFETCH : do not use prefetch macro

-DNOMULTDIR : do not use up-down loop on m,n
-DNOINLINECORE : do not use inline assembler code for alpha
-DKMNLOOP : use k->m->n major loop order(default is k->n->m)

The core routine of DGEMM is matrices multiply. There are known
way to use blocking algorithm for this problem. Usually they are
intended only to the cache optimization. But for a large problem it is
required to optimize the usage of translation buffer. Every matrix 
element which is adjucent by one column will require an entry on the 
translation buffer. Then buffer shortage will easily be occured on
most RISC processors. This program is to demonstrate the blocking
algorithm to reduce both the buffer miss and cache miss on a certain
processor.

Conventional method:

Now we are going to solve a problem : C = aAB + bC.

A=[A11 A12 A13 ... ; A21  A22 A23 ...; ... ; As1 As2 ...]
B=[B11 B12 B13 ... ; B21  B22 B23 ...; ... ; Bt1 Bt2 ...]
C=[C11 C12 C13 ... ; C21  C22 C23 ...; ... ; Cs1 Cs2 ...]

Cij = bCij + a*sum(Ail*Blj for l=1 to t)

More square the Aij will result in less memory load traffic. And
usually above algorithm is just implemented to reduce store traffic.

My method:

In fortran, the row direction is continuous on memory. Then if we
calculate Ail*Blj for i=1 to n, the translation miss will be reduced.
Of course this requires more temporaly result on Cij which induce to
occure more store traffic. For many implementation of RISC machines,
the second or third cache is organized as store-in cache. Then once
store is required it must first read a cache block from memoy. And the
store traffic is almost as heavy as the double of the same amount load
traffic. Since the store occurence of that computation completely 
depends on the number of column of Ail, we can control the traffic.
The traffic by Blj can be almost ignored, because once Blj is loaded
into the cache, many of Ail*Blj can be executed with the cached data.
But the load traffic of Ail will be another major traffic for this
problem. And we should keep the portion of Ail in cache as large as
possible. But if the store traffic and the load traffic will both be
affordable for a specific machine, we have a chance to induce the 
maximum performance.

To fit in the translation buffer entry, the program should use
blocking on Ail. The total number of the row times the sizeof double
in a block should be fit in the pagesize of the processor. And the
total number of the column of Ail, Blj, Cij should not be exceed the
number of translation buffer entries.

The number of column of the Ail is arbitrary. The more will decrease 
the store traffic and increase the load traffic, the less will increase
the store traffic but decrease the load traffic.  Then there is some
optimal point for a specific machine. 

The Ail will cause load traffic from the number of column of the Ail on
every the number of column of Ail by the number of column of Blj
submatrix multiplication. The Cij will cause store traffic on the
every row of the Cij.

The parameter depends on the processor and memory system
configuration, and you can tune for your system.
But if the number of the translation buffer is small and load/store
throughput is also small, the result will be hopeless. You should
think to replace your machine.

If the Ail is aligned to the page size, it is easy to treat. But for
general we cannot expect so. I assume that the translation buffer
entries used by Ail will be doubled effectively. To be hold the
all required page in the translation buffer.
Of course this not ideal solution. You may want to align the A
to the page boundary and set LDA to the factor of the pagesize.

The algorithm will be following:

do ii=1 to k, ail_clmn_size
 do jj=1 to m, ail_maj_blk_size
  do kk=1 to n, blj_clmn_size
   do  ll=jj to jj+ail_maj_blk_size
    ctmp_ll_kk += A_ll_ii*B_ii_kk
   enddo
  enddo
 enddo
enddo

For AlphaPC164LX parameters:
  DTB 64 entries
  Page size 8KB
  load throughput  almost 700MB/S
  store throughput almost 400MB/S

Then I think the following number is affordable.

1. The number of column of Ail is 20
2. The number of column of Blj,Cij is 12
3. The blocking factor of the Ail row is 512 (just fit into the half page)

You may have a good result for 21264 with the parameter of:

1. The number of column of Ail is 40
2. The number of column of Blj,Cij is 24
3. The blocking factor of the Ail row is 512 (just fit into the half page)

History:

99.09.14: revise for Compaq C compiler (use with NOPREFETCH, NOINLINECORE)
98.12.24: updown loop modified
98.12.24: Inner most loop is now coded with inline assembler(only for alhpa)
98.12.22: Introduce updown loop on the n.
98.12.21: Change major loop order, introduce updown loop on the m.
98.12.20: Change blocking default parameter, PA8K definitions added.
98.12.18: Revised boundary condition of the unrollings again.
Matrix B prefetch is designed not to hurt the load performance.
98.12.17(2): Revised boundary condition of the unrollings.
98.12.17: Revised not to use integer multiply for pointer traverse.
Force to set Ail_max to the factor of 8 to reduce the followup loop.
If you still need -DNOALGN may or may not help you.
98.12.16: Revised the handling of alpha and beta parameters.
98.12.15: Revised to do more execution in the inner loop.
First version available :98.12.12

ToDo:
1: Implement sub-matrix copy algorithm for comparing aid.
2: Processor/System specific Ail_max, Blj_max should be developped to
extract best performance. 

*/

#ifdef DEBUG
#include <stdio.h>
#endif
#ifdef NOMULDIR
#define NOMULDIRMM
#define NOMULDIRM
#define NOMULDIRN
#endif

#if defined(__alpha__) || defined(__alpha)
#if defined(ev4)
#define PAGESIZE 8192
#define CACHESIZE 16384
#define L2CACHESIZE 524288
#define TLBENTRY 32
#elif defined(ev6)
#define PREFETCHB
#define PREFETCHC
#define PAGESIZE 8192
#define CACHESIZE 65536
#define L2CACHESIZE 2097152
#define TLBENTRY 128
#else
#define PREFETCHB
#define PAGESIZE 8192
#define CACHESIZE 98304
#define L2CACHESIZE 2097152
#define TLBENTRY 64
#endif
#elif defined(__hppa)
#define PAGESIZE 4096
#define CACHESIZE 1048576
#define L2CACHESIZE 1048576
#define TLBENTRY 96
#define PREFETCHB
#define PREFETCHC
#elif defined(__i386__)
#define PAGESIZE 4096
#define CACHESIZE 8192
#define L2CACHESIZE 262144
#define TLBENTRY 64
#else
#define PAGESIZE 4096
#define CACHESIZE 16384
#define L2CACHESIZE 0
#define TLBENTRY 32
#endif
#ifndef Ail_max
#define Ail_max ((((TLBENTRY*3/8)>>2)<<2))
#endif
#ifndef Blj_max
#define Blj_max ((((TLBENTRY*3/16)>>1))<<1)
#endif
#ifndef A_row_block
#if (PAGESIZE/2 > CACHESIZE/(Ail_max/2))
#define A_row_block (CACHESIZE/(sizeof(double)*(Ail_max/2)))
#else
#define A_row_block (PAGESIZE/(2*sizeof(double)))
#endif
#endif


#if defined(__alpha__) && !defined(NOPREFETCH)
#define prefetch(x) asm volatile("ldt $f31, %0" : :"m"(x))
#elif defined(__hppa) && !defined(NOPREFETCH) && 0
#define prefetch(x) _asm("fldd",x,"%fr0")
#else
#define prefetch(x) 
#endif


void dgemm_x_nn(int m, int n, int k,
 double alpha, double *a, int lda, double *b, int ldb, 
 double beta, double *c, int ldc, int submatcpy, double *submat)
{
int A_block;
int i1,i3,i2,i4,i5,i6,i4end,i4ndx,i6end;
int ldbstep, ldcstep;
int ndir, mdir, mmdir, i1step, i3step, i2step;
double *pa0,*pa1,*pa2,*pa3,*pa4;
double *pb0i,*pb0,*pb1,*pb2;
double *pc0,*pc1,*pc2;
double a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12;
double b1,b2,b3,b4,b5,b6,b7,b8;
double c1,c2,c3,c4,c5,c6;
double t1,t2,t3,t4;

#ifndef SIMPLEABLOCK
A_block = (m>A_row_block)?
          (m/(int)((float)m/(float)A_row_block + 0.9)):
          m;
#else
A_block=(m>A_row_block)?A_row_block:m;
#endif
A_block = (A_block > 2)? A_block: 2;
#if defined(DEBUG)
printf("m: %d : A_block: %d:\n",m,A_block);
#endif
ndir = 0;
mdir=0;
i1=0;
i1step = Ail_max;
i1step = (k>Ail_max)?Ail_max:(k>>2)<<2;
do {
#ifdef DEBUG
printf("i1step(k): %d :\n",i1step);
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i1(k) loop: %d to %d by %d:\n",i1,k-i1step,i1step);
#endif
for(i1=i1; i1<=k-i1step ; i1+=i1step) { /*k*/
 mmdir = 0;
 pa0 = a+i1*lda;
#ifdef KMNLOOP
   i3=(mdir)?m-1:0;
   i3step = A_block;
   i3step = (m>A_block)?A_block:m;
  do {
#ifdef DEBUG
printf("i3step(m): %d :\n",i3step);
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i3(m) loop: %d to %d by %d:\n",i3,
(mdir)?(i3step):(m-i3step),(mdir)?-i3step:i3step);
#endif
  for(i3=i3; (mdir)?(i3>=i3step-1):(i3<=m-i3step); i3+=(mdir)?-i3step:i3step) { /*m*/
#endif
 i2step = (n>Blj_max)?Blj_max:n;
 i2=(ndir)?n-1:0;
 do {
#ifdef DEBUG
printf("i2step(n): %d :\n",i2step);
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i2(n) loop: %d to %d by %d:\n",i2,
(ndir)?(i2step):(n-i2step),(ndir)?-i2step:i2step);
#endif
for(i2=i2; (ndir)?(i2>=i2step-1):(i2<=n-i2step); i2+=(ndir)?-i2step:i2step) { /*n*/
   pc0=c+i2*ldc;
   pb0=b+i1+i2*ldb;
   ldcstep = (ndir)?-ldc:ldc;
   ldbstep = (ndir)?-ldb:ldb;
#ifndef KMNLOOP
#define NOMULTDIRMM
   i3=(mdir)?m-1:0;
   i3step = A_block;
  do {
#ifdef DEBUG
printf("i3step(m): %d :\n",i3step);
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i3(m) loop: %d to %d by %d:\n",i3,
(mdir)?(i3step-1):(m-i3step),(mdir)?-i3step:i3step);
#endif
  for(i3=i3; (mdir)?(i3>=i3step-1):(i3<=m-i3step); i3+=(mdir)?-i3step:i3step) { /*m*/
#endif
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i4(m) loop: %d to %d by %d:\n",
   (mmdir)?((mdir)?i3-i3step+1:i3+i3step-1):i3,
   (mmdir)?((mdir)?i3-3+1:i3+3-1):((mdir)?i3-i3step+3:i3+i3step-3),
       (mmdir)?-3:3);
#endif
   i4end=(mmdir)?((mdir)?i3-3+1:i3+3-1):((mdir)?i3-i3step+3:i3+i3step-3);
   for(i4= (mmdir)?((mdir)?i3-i3step+1:i3+i3step-1):i3;
       ((mmdir^mdir)?(i4>=i4end):(i4<=i4end)) && (i1step>=4); 
       i4+=(mmdir^mdir)?-3:3) { /* m */
    i4ndx=(mmdir^mdir)?i4-3+1:i4;
    pb0i = pb0;
    pc1 = pc0+i4ndx;
    pc2 = pc1+ldcstep;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("i6(n) loop: %d to %d by %d:\n",i2,(ndir)?i2-i2step+2:i2+i2step-2,(ndir)?-2:2);
#endif
    for(i6=i2; (ndir)?(i6>i2-i2step):(i6<=i2+i2step-2); i6+=(ndir)?-2:2) { /* n */
 /*    for(i6=i2; i6<=i2+i2step-2; i6+=2) { n */
     pa1 = pa0+i4ndx;
     pa2 = pa1 + lda;
     pa3 = pa2 + lda;
     pa4 = pa3 + lda;
     pb1 = pb0i;
     pb2 = pb1 + ldbstep;
#ifdef PREFETCHC
     prefetch(*pc1);
     prefetch(*pc2);
#endif
     a1 = *(pa1);
     a2 = *(pa2);
     a3 = *(pa3);
     a4 = *(pa4);
     a5 = *(pa1+ 1);
     a6 = *(pa2+ 1);
     a7 = *(pa3+ 1);
     a8 = *(pa4+ 1); 
     a9 = *(pa1+ 2);
     a10 = *(pa2+ 2);
     a11 = *(pa3+ 2);
     a12 = *(pa4+ 2); 
     b1 = *pb1;
     b2 = *(pb1 + 1);
     b3 = *(pb1 + 2);
     b4 = *(pb1 + 3);
     b5 = *pb2;
     b6 = *(pb2 + 1);
     b7 = *(pb2 + 2);
     b8 = *(pb2 + 3);
     t1 = a1*b1;
     t2 = a5*b1;
     t3 = a9*b1;
     t4 = a1*b5;
     c1 =  0.0;
     c2 =  0.0;
     c5 =  0.0;
     c3 =  0.0;
     c4 =  0.0;
     c6 =  0.0;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("i5(k) loop: %d to %d by %d:\n",i1,i1+i1step,4);
#endif
     for(i5=i1; i5<i1+i1step-4; i5+=4) { /* k */
#if defined(__alpha__) && !defined(NOINLINECORE) 
#ifdef DEBUG
printf("a1 %e a2 %e a3 %e a4 %e\n", a1,a2,a3,a4);
printf("a5 %e a6 %e a7 %e a8 %e\n", a5,a6,a7,a8);
printf("a9 %e a10 %e a11 %e a12 %e\n", a9,a10,a11,a12);
printf("b1 %e b2 %e b3 %e b4 %e\n", b1,b2,b3,b4);
printf("b5 %e b6 %e b7 %e b8 %e\n", b5,b6,b7,b8);
#endif
asm volatile(" addt %1,%2,%0":"f="(c1):"f"(t1),"f"(c1));
asm volatile(" s8addq %1,%2,%0":"r="(pa1):"r"(lda),"r"(pa4));
asm volatile(" mult %1,%2,%0":"f="(t1):"f"(a5),"f"(b5));
asm volatile(" s8addq %1,%2,%0":"r="(pa2):"r"(lda),"r"(pa1));
asm volatile(" addt %1,%2,%0":"f="(c2):"f"(t2),"f"(c2));
asm volatile(" addq %2,%1,%0":"r="(pb1):"i"(32),"r"(pb1));
asm volatile(" mult %1,%2,%0":"f="(t2):"f"(a9),"f"(b5));
asm volatile(" addq %2,%1,%0":"r="(pb2):"i"(32),"r"(pb2));
asm volatile(" addt %1,%2,%0":"f="(c3):"f"(t3),"f"(c3));
asm volatile(" ldt %0,%1":"f="(b1):"m"(*pb1));
asm volatile(" mult %1,%2,%0":"f="(t3):"f"(a2),"f"(b2));
asm volatile(" ldt %0,%1":"f="(a1):"m"(*pa1));
asm volatile(" addt %1,%2,%0":"f="(c4):"f"(t4),"f"(c4));
asm volatile(" s8addq %1,%2,%0":"r="(pa3):"r"(lda),"r"(pa2));
asm volatile(" mult %1,%2,%0":"f="(t4):"f"(a6),"f"(b2));
asm volatile(" ldt %0,%1":"f="(a5):"m"(*(pa1+1)));
asm volatile(" addt %1,%2,%0":"f="(c5):"f"(t1),"f"(c5));
asm volatile(" s8addq %1,%2,%0":"r="(pa4):"r"(lda),"r"(pa3));
asm volatile(" mult %1,%2,%0":"f="(t1):"f"(a10),"f"(b2));
asm volatile(" ldt %0,%1":"f="(a9):"m"(*(pa1+2)));
asm volatile(" addt %1,%2,%0":"f="(c6):"f"(t2),"f"(c6));
asm volatile(" ldt %0,%1":"f="(b5):"m"(*(pb2+0)));
asm volatile(" mult %1,%2,%0":"f="(t2):"f"(a2),"f"(b6));
asm volatile(" ldt %0,%1":"f="(b2):"m"(*(pb1+1)));
asm volatile(" addt %1,%2,%0":"f="(c1):"f"(t3),"f"(c1));
asm volatile(" ldt %0,%1":"f="(a2):"m"(*(pa2+0)));
asm volatile(" mult %1,%2,%0":"f="(t3):"f"(a6),"f"(b6));
asm volatile(" ldt %0,%1":"f="(a6):"m"(*(pa2+1)));
asm volatile(" addt %1,%2,%0":"f="(c2):"f"(t4),"f"(c2));
asm volatile(" mult %1,%2,%0":"f="(t4):"f"(a10),"f"(b6));
asm volatile(" ldt %0,%1":"f="(a10):"m"(*(pa2+2)));
asm volatile(" addt %1,%2,%0":"f="(c3):"f"(t1),"f"(c3));
asm volatile(" ldt %0,%1":"f="(b6):"m"(*(pb2+1)));
asm volatile(" mult %1,%2,%0":"f="(t1):"f"(a3),"f"(b3));
asm volatile(" addt %1,%2,%0":"f="(c4):"f"(t2),"f"(c4));
asm volatile(" mult %1,%2,%0":"f="(t2):"f"(a7),"f"(b3));
asm volatile(" addt %1,%2,%0":"f="(c5):"f"(t3),"f"(c5));
asm volatile(" mult %1,%2,%0":"f="(t3):"f"(a11),"f"(b3));
asm volatile(" ldt %0,%1":"f="(b3):"m"(*(pb1+2)));
asm volatile(" addt %1,%2,%0":"f="(c6):"f"(t4),"f"(c6));
asm volatile(" mult %1,%2,%0":"f="(t4):"f"(a3),"f"(b7));
asm volatile(" ldt %0,%1":"f="(a3):"m"(*(pa3+0)));
asm volatile(" addt %1,%2,%0":"f="(c1):"f"(t1),"f"(c1));
asm volatile(" mult %1,%2,%0":"f="(t1):"f"(a7),"f"(b7));
asm volatile(" ldt %0,%1":"f="(a7):"m"(*(pa3+1)));
asm volatile(" addt %1,%2,%0":"f="(c2):"f"(t2),"f"(c2));
asm volatile(" mult %1,%2,%0":"f="(t2):"f"(a11),"f"(b7));
asm volatile(" ldt %0,%1":"f="(b7):"m"(*(pb2+2)));
asm volatile(" addt %1,%2,%0":"f="(c3):"f"(t3),"f"(c3));
asm volatile(" ldt %0,%1":"f="(a11):"m"(*(pa3+2)));
asm volatile(" mult %1,%2,%0":"f="(t3):"f"(a4),"f"(b4));
asm volatile(" addt %1,%2,%0":"f="(c4):"f"(t4),"f"(c4));
asm volatile(" mult %1,%2,%0":"f="(t4):"f"(a8),"f"(b4));
asm volatile(" addt %1,%2,%0":"f="(c5):"f"(t1),"f"(c5));
asm volatile(" mult %1,%2,%0":"f="(t1):"f"(a12),"f"(b4));
asm volatile(" ldt %0,%1":"f="(b4):"m"(*(pb1+3)));
asm volatile(" addt %1,%2,%0":"f="(c6):"f"(t2),"f"(c6));
asm volatile(" mult %1,%2,%0":"f="(t2):"f"(a4),"f"(b8));
asm volatile(" ldt %0,%1":"f="(a4):"m"(*(pa4+0)));
asm volatile(" addt %1,%2,%0":"f="(c1):"f"(t3),"f"(c1));
asm volatile(" mult %1,%2,%0":"f="(t3):"f"(a8),"f"(b8));
asm volatile(" ldt %0,%1":"f="(a8):"m"(*(pa4+1)));
asm volatile(" addt %1,%2,%0":"f="(c2):"f"(t4),"f"(c2));
asm volatile(" mult %1,%2,%0":"f="(t4):"f"(a12),"f"(b8));
asm volatile(" ldt %0,%1":"f="(a12):"m"(*(pa4+2)));
/*
asm volatile(" addt %1,%2,%0":"f="(c3):"f"(t1),"f"(c3));
asm volatile(" addt %1,%2,%0":"f="(c4):"f"(t2),"f"(c4));
asm volatile(" addt %1,%2,%0":"f="(c5):"f"(t3),"f"(c5));
asm volatile(" addt %1,%2,%0":"f="(c6):"f"(t4),"f"(c6));
*/

  c3 = t1 + c3;
     t1 = a1*b1;
asm volatile(" ldt %0,%1":"f="(b8):"m"(*(pb2+3)));
  c4 = t2 + c4;
     t2 = a5*b1;
  c5 = t3 + c5;
     t3 = a9*b1;
  c6 = t4 + c6;
     t4 = a1*b5;
#ifdef DEBUG
printf("c1=%e,c2=%e,c3=%e,c4=%e,c5=%e,c6=%e\n", c1,c2,c3,c4,c5,c6);
#endif
#else
      c1 = c1 + (t1+a2*b2+a3*b3+a4*b4);
      c2 = c2 + (t2+a6*b2+a7*b3+a8*b4);
      c3 = c3 + (t3+a10*b2+a11*b3+a12*b4);
      c4 = c4 + (t4+a2*b6+a3*b7+a4*b8);
      c5 = c5 + (a5*b5+a6*b6+a7*b7+a8*b8);
      c6 = c6 + (a9*b5+a10*b6+a11*b7+a12*b8);
#ifdef DEBUG
printf("a1 %e a2 %e a3 %e a4 %e\n", a1,a2,a3,a4);
printf("a5 %e a6 %e a7 %e a8 %e\n", a5,a6,a7,a8);
printf("a9 %e a10 %e a11 %e a12 %e\n", a9,a10,a11,a12);
printf("b1 %e b2 %e b3 %e b4 %e\n", b1,b2,b3,b4);
printf("b5 %e b6 %e b7 %e b8 %e\n", b5,b6,b7,b8);
#endif
      pa1 = pa4 + lda;
      pa2 = pa1 + lda;
      pa3 = pa2 + lda;
      pa4 = pa3 + lda;
      pb1 += 4;
      pb2 += 4;
      b1 = *pb1;
      b2 = *(pb1 + 1);
      b3 = *(pb1 + 2);
      b4 = *(pb1 + 3);
      b5 = *pb2;
      b6 = *(pb2 + 1);
      b7 = *(pb2 + 2);
      b8 = *(pb2 + 3);
      a1 = *(pa1);
      a5 = *(pa1+ 1);
      a9 = *(pa1+ 2);
      a2 = *(pa2);
      a6 = *(pa2+ 1);
      a10 = *(pa2+ 2);
      a3 = *(pa3);
      a7 = *(pa3+ 1);
      a11 = *(pa3+ 2);
      a4 = *(pa4);
      a8 = *(pa4+ 1); 
      a12 = *(pa4+ 2); 
     t1 = a1*b1;
     t2 = a5*b1;
     t3 = a9*b1;
     t4 = a1*b5;
#endif
      } /*k*/
#ifdef PREFETCHB
      prefetch(*(pb2+ldbstep));
      prefetch(*(pb2+2*ldbstep));
#endif
      c1 = c1 + (t1+a2*b2+a3*b3+a4*b4);
      c2 = c2 + (t2+a6*b2+a7*b3+a8*b4);
      c3 = c3 + (t3+a10*b2+a11*b3+a12*b4);
      c4 = c4 + (t4+a2*b6+a3*b7+a4*b8);
      c5 = c5 + (a5*b5+a6*b6+a7*b7+a8*b8);
      c6 = c6 + (a9*b5+a10*b6+a11*b7+a12*b8);
#ifdef DEBUG
printf("a1 %e a2 %e a3 %e a4 %e\n", a1,a2,a3,a4);
printf("a5 %e a6 %e a7 %e a8 %e\n", a5,a6,a7,a8);
printf("a9 %e a10 %e a11 %e a12 %e\n", a9,a10,a11,a12);
printf("b1 %e b2 %e b3 %e b4 %e\n", b1,b2,b3,b4);
printf("b5 %e b6 %e b7 %e b8 %e\n", b5,b6,b7,b8);
#endif
     c1 = ((i1 == 0)? beta*(*(pc1+0)) : (*(pc1+0)))+c1*alpha;
     c2 = ((i1 == 0)? beta*(*(pc1+1)) : (*(pc1+1)))+c2*alpha;
     c3 = ((i1 == 0)? beta*(*(pc1+2)) : (*(pc1+2)))+c3*alpha;
     c4 = ((i1 == 0)? beta*(*(pc2+0)) : (*(pc2+0)))+c4*alpha;
     c5 = ((i1 == 0)? beta*(*(pc2+1)) : (*(pc2+1)))+c5*alpha;
     c6 = ((i1 == 0)? beta*(*(pc2+2)) : (*(pc2+2)))+c6*alpha;
#ifdef DEBUG
printf("c1=%e,c2=%e,c3=%e,c4=%e,c5=%e,c6=%e\n", c1,c2,c3,c4,c5,c6);
#endif
     *pc1     = c1;
     *(pc1+1) = c2;
     *(pc1+2) = c3;
     *pc2     = c4;
     *(pc2+1) = c5;
     *(pc2+2) = c6;
     pc1 += 2*ldcstep;
     pc2 += 2*ldcstep;
     pb0i += 2*ldbstep;
     }/*n*/
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("i6(n) loop: %d to %d by %d:\n",i6,(ndir)?i2-i2step:i2+i2step,(ndir)?-1:1);
#endif
    i6end = (ndir)?i2-i2step:i2+i2step;
    for(i6=i6; (ndir)?(i6>i6end):(i6<i6end); i6+=(ndir)?-1:1) { /* n */
/*     for(i6=i6; i6<i2+i2step; i6++) { n */
#ifdef PREFETCHC
     prefetch(*pc1);
#endif
     pa1 = pa0+i4ndx;
     pb1 = pb0i;
     c1 = 0.0;
     c2 = 0.0;
     c3 = 0.0;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("i5(k) loop: %d to %d by %d:\n",i1,i1+i1step,1);
#endif
     for(i5=i1; i5<i1+i1step; i5++) { /* k */
      a1 = (*pa1);
      a2 = *(pa1+1);
      a3 = *(pa1+2);
      b1 = (*pb1);
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("a1 %e b1 %e c1 %e\n", a1,b1,c1);
printf("a2 %e b1 %e c2 %e\n", a2,b1,c2);
#endif
      c1 = c1 + a1*b1;
      c2 = c2 + a2*b1;
      c3 = c3 + a3*b1;
      pa1 += lda;
      pb1 ++;
      }
#ifdef PREFETCHB
     prefetch(*(pb1+ldbstep));
#endif
     c1 = ((i1 == 0)? beta*(*(pc1+0)) : (*(pc1+0)))+c1*alpha;
     c2 = ((i1 == 0)? beta*(*(pc1+1)) : (*(pc1+1)))+c2*alpha;
     c3 = ((i1 == 0)? beta*(*(pc1+2)) : (*(pc1+2)))+c3*alpha;
#ifdef DEBUG
printf("c1= %e, c2= %e, c3 = %e\n", c1,c2,c3);
#endif
     *pc1 = c1;
     *(pc1+1) = c2;
     *(pc1+2) = c3;
     pc1 += ldcstep;
     pb0i += ldbstep;
     }
    }
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4ndx,i5,i6);
printf("i4 loop: %d to %d by %d:\n",
   i4,
   (mmdir)?(i3):((mdir)?i3-i3step:i3+i3step),
   (mmdir^mdir)?-1:1);
#endif
   i4end = (mmdir)?((mdir)?i3+1:i3-1):((mdir)?i3-i3step:i3+i3step); 
   for(i4=i4;
       (mmdir^mdir)? (i4>i4end):(i4<i4end);
       i4+=(mmdir^mdir)?-1:1) { /* m */
    i4ndx=i4;
    pc1 = pc0+i4ndx;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i6(n) loop: %d to %d by %d:\n",i2,(ndir)?i2-i2step:i2+i2step,(ndir)?-1:1);
#endif
    pb0i=pb0;
    for(i6=i2; (ndir)?(i6>i2-i2step):(i6<i2+i2step); i6+=(ndir)?-1:1) { /* n */
/*     for(i6=i2; i6<i2+i2step; i6+=1) { n */
     pa1 = pa0+i4ndx;
     pb1 = pb0i;
     c1 = 0.0;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i5(k) loop: %d to %d by %d:\n",i1,i1+i1step-4,4);
#endif
     for(i5=i1; i5<=i1+i1step-4; i5+=4) { /* k */
      a1 = *pa1;
      b1 = *pb1;
      a2 = *(pa1+lda);
      b2 = *(pb1+1);
      a3 = *(pa1+2*lda);
      b3 = *(pb1+2);
      a4 = *(pa1+2*lda+lda);
      b4 = *(pb1+3);
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("a1 %e b1 %e c1 %e\n", a1,b1,c1);
#endif
      c1 = c1 + a1*b1+a2*b2+a3*b3+a4*b4;
      pa1 += 4*lda;
      pb1 += 4;
      } /* i5 */
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("i5(k) loop: %d to %d by %d:\n",i1,i1+i1step,1);
#endif
     for(i5=i5; i5<i1+i1step; i5+=1) { /* k */
      a1 = *pa1;
      b1 = *pb1;
#ifdef DEBUG
printf("i1:%d:i3:%d:i2:%d:i4:%d:i5:%d:i6:%d\n",i1,i3,i2,i4,i5,i6);
printf("a1 %e b1 %e c1 %e\n", a1,b1,c1);
#endif
      c1 = c1 + a1*b1;
      pa1 += lda;
      pb1++;
      } /* i5 */
     c1 = ((i1 == 0)? beta*(*(pc1+0)) : (*(pc1+0)))+c1*alpha;
     *pc1 = c1;
#ifdef DEBUG
printf("c1= %e\n", c1);
#endif
     pc1 += ldcstep;
     pb0i += ldbstep;
    } /* i6 */
   } /* i4 */
#ifdef KMNLOOP
#ifndef NOMULTDIRMM
  mmdir = 1-mmdir;
#endif
 } /* i2 */
 i2step = (ndir)?i2+1:(n-i2);
 } while ((ndir)?(i2 >= 0):(i2<n));
#ifndef NOMULTDIRN
  ndir = 1-ndir;
#endif
#endif
  } /* i3 */
  i3step = (mdir)?i3+1:(m-i3);
  } while ((mdir)?(i3 >= 0):(i3<m));
#ifdef KMNLOOP
#ifndef NOMULTDIRM
  mdir = 1-mdir; 
#endif
#else
 } /* i2 */
 i2step = (ndir)?i2+1:(n-i2);
 } while ((ndir)?(i2 >= 0):(i2<n));
#ifndef NOMULTDIRM
  mdir = 1-mdir; 
#endif
#ifndef NOMULTDIRMM
  mmdir = 1-mmdir;
#endif
#ifndef NOMULTDIRN
 ndir = 1-ndir;
#endif
#endif
} /* i1 */
i1step = ((k-i1)>=4 && i1step>4)?  (((k-i1)>>2)<<2): 1;
} while (i1 < k);
}

void dgemmc_nn(int m, int n, int k,
 double alpha, double *a, int lda, double *b, int ldb, 
 double beta, double *c, int ldc)
{
double sub[Ail_max*A_row_block];
dgemm_x_nn(m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, 1, sub);
}
void dgemms_nn(int m, int n, int k,
 double alpha, double *a, int lda, double *b, int ldb, 
 double beta, double *c, int ldc)
{
dgemm_x_nn(m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, 0, (void *)0);
}

void dgemms_nn_(int *m, int *n, int *k,
 double *alpha, double *a, int *lda, double *b, int *ldb, 
 double *beta, double *c, int *ldc) {
   dgemms_nn(*m, *n, *k, *alpha, a, *lda, b, *ldb, *beta, c, *ldc);
  }

#ifdef MAIN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/mman.h>
#include <time.h>
#include <asm/system.h>

#define LDA    1001
#define LDB    1001
#define LDC    1001

#define SIZE 1000

#define SIZE_M SIZE
#define SIZE_N SIZE
#define SIZE_K SIZE

#define REPEAT 3


#if defined(RPCC)

static inline unsigned int realcc(void){
  unsigned int cc;

  asm volatile("rpcc %0" : "=r"(cc));
  return cc;
}

#ifndef CPS
#define CPS 300000000
#endif

#else
#define realcc() (int)clock()
#define CPS CLOCKS_PER_SEC
#endif

int main(int argc, char *argv[]){

  double *a, *b, *c;
  double alpha, beta;
  int i,j,repeat;
  int m, n, k;
  int lda, ldb, ldc;
  int start1, stop1;

  if(argc>1) {m=n=k=atoi(argv[1]);}
  else {
	  m=SIZE_M;
	  n=SIZE_N;
	  k=SIZE_K;
	  }
  if(argc>2) {lda=ldb=ldc=atoi(argv[2]);}
  else {
	  lda = LDA;
	  ldb = LDB;
	  ldc = LDC;
	  }

  alpha = 2.0;
  beta  = 0.5;
  if(mlockall(MCL_FUTURE)<0) {
	printf("mlock error\n");
	exit(1);
  }
  if ( (a = (double *)sbrk(sizeof(double) * ldc * lda + 
			 sizeof(double) * ldb * ldc + 
			 sizeof(double) * ldb * lda)) == NULL) {
      fprintf(stderr,"Out of Memory!!\n");exit(1);
    }
  b = a + ldc*lda;
  c = b + ldb*ldc;

//  printf("select event %p\n", (void *)wrperfmon(2UL,0xb7));
//  printf("select mode %p\n", (void *)wrperfmon(3UL,0UL));
//  printf("disable irq wrperfmon %p\n", (void *)wrperfmon(4UL,0UL));
//  printf("start wrperfmon %p\n", (void *)wrperfmon(7UL,7UL));
  for(i=0;i<ldc;i++) for(j=0;j<lda;j++) a[i*lda +j]=i;
  for(i=0;i<ldb;i++) for(j=0;j<ldc;j++) b[i*ldb +j]=j;

//  printf("stop wrperfmon %p\n", (void *)wrperfmon(0UL,7UL));
//  printf("value of wrperfmon %p\n", (void *)wrperfmon(5UL,7UL));

  printf("m:%3d n:%3d k:%3d\n", m, n, k);
  printf("Ail_max %d, Blj_max %d, A_row_block %d\n", 
	  Ail_max, Blj_max, (int)A_row_block);

  for(repeat=0;repeat<REPEAT;repeat++) {

	for(i=0;i<ldb;i++) for(j=0;j<lda;j++){
	  c[i*ldc +j]=1.0;
	}

//  printf("start wrperfmon %p\n", (void *)wrperfmon(7UL,7UL));
	start1 = realcc();
	dgemms_nn(m, n, k, alpha, a, lda, b, ldb, beta, c, ldc);
	stop1 = realcc();
//  printf("stop wrperfmon %p\n", (void *)wrperfmon(0UL,7UL));
//  printf("value of wrperfmon %p\n", (void *)wrperfmon(5UL,7UL));

	printf("Shimizu's DGEMM :%8.3f MFLOPS(%6.3f sec)\n", 
	 2.0*m*n*k*1e-6/((double)(stop1 - start1)/CPS),
	 (double)(stop1 - start1)/CPS);
   }

  return 0;
}
#endif

