#include <stdio.h>

int printing = 0 ;

/* on alpha build with cc -g -mieee */

int main()
{
	int errout;
#if defined(__alpha__)
	unsigned long fpcr;
	long l;
	for ( l = 0; l < 4; l++ ) {
	    fpcr = ( l << 58 );	/*  */
	    asm volatile ( "mt_fpcr %0 ; excb" : : "f"(fpcr));
	    underflow();
	}
#else
	underflow();
#endif
} 
void printF( float, char *);
void printD( double, char *);
 
int underflow()
{
	int dsb=32-9, dme=-126;

	int expLimit = dme - dsb + 1;
	float divisor = 2.0;
	float  d = 1.0, e;
	float  prev=0;
	int i;
	double conversion;

	for (i=0; i>=expLimit; i--) {
		if ( printing ) {
		   printf("I=%d\t prev=%16.14g, d=%16.14g\n",i, prev, d);
		   printF( d, "Value:\t");
		}
		prev = d;
		if ((d /= divisor) >= prev) {
			printf("Oops. growing larger ? \n");
			return;
		}
		else if (d == 0)
			break;
		conversion = d;		/* double <= float */
		e = conversion;		/* float <= double */
		printF( d, "V\\=2.0\t");
		printD( conversion, "Conversion:");
		printF( e, "reverted:");
		if ( d != e ){		/* Still same ?	   */
		   printf(" Conversion failed \n");
		   break;
		}
	};

	if (i > expLimit)
		printf("Oops, didnt do all denormal numbers\n");

	return;
}

union _FP_UNION_D
{
  double flt;
  long	 l;
  struct {
    unsigned long long frac : 64-12;
    unsigned exp  : 11 ;
    unsigned sign : 1;
  } bits __attribute__((packed));
} ud;
union _FP_UNION_S
{
  float flt;
  int	 i;
  struct {
    unsigned int frac : 32-9;
    unsigned exp  : 8 ;
    unsigned sign : 1;
  } bits __attribute__((packed));
} uf;

void
printF( float d, char *msg )
{
	uf.flt = d;
	if ( printing )
	    printf("%s\tf=%x, s=(%d,%x), exp=(%d,%x), frac=(%d,%x)\n", 
		msg,
		uf.i,
		uf.bits.sign, uf.bits.sign,
		uf.bits.exp, uf.bits.exp,
		uf.bits.frac, uf.bits.frac );	
}

void
printD( double d, char *msg)
{
	ud.flt  = d;
	if( printing )
	    printf("%s\tf=%lx, s=(%d,%x), exp=(%d,%x), frac=(%ld,%lx)\n", 
		msg,
		ud.l,
		ud.bits.sign, ud.bits.sign,
		ud.bits.exp, ud.bits.exp,
		ud.bits.frac, ud.bits.frac );		
}

