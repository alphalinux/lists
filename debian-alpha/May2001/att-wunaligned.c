#include <errno.h>
#include <stdio.h>

#ifndef __linux__
#include <sys/sysinfo.h>
#else
#include <asm/sysinfo.h>
#include <asm/unistd.h>

static int setsysinfo(unsigned long op, void *buffer, unsigned long size,
		      int *start, void *arg, unsigned long flag)
{
  syscall(__NR_osf_setsysinfo, op, buffer, size, start, arg, flag);
}
#endif


static void usage(void)
{
        fprintf(stderr,
		"usage: unaligned  <command-path> [command-args...]\n\n"
		"  This program is designed to assist debugging of\n"
		"  unaligned traps by running the program in gdb\n"
		"  and causing it to get SIGBUS when it encounters\n"
		"  an unaligned trap.\n\n"
		"  It is free software written by Sean Hunter <sean@uncarved.co.uk>\n"
		"  based on code by Richard Henderson and Andrew Morgan.  It is provided\n"
		"  under the gnu public license without warrantees of any kind.\n\n");

	exit(1);
}


void trap_unaligned(void)
{
  unsigned int buf[2];
  buf[0] = SSIN_UACPROC;
  buf[1] = UAC_SIGBUS | UAC_NOPRINT;
  setsysinfo(SSI_NVPAIRS, buf, 1, 0, 0, 0);

}


int main(int argc, char **argv)
{
    char* tmp_filename;
    char* my_debugger = "/usr/bin/gdb";
    FILE* tmp_file;
    int curr_arg;

    /* check that we have at least 1 argument */
    if (argc < 2) {
        usage();
    }

    trap_unaligned();

    if (argc > 2) {
        /* add the extra args to a file to pass to gdb */
        tmp_filename = tmpnam(NULL);
	tmp_file = fopen(tmp_filename, "w+");
	if (!tmp_file) {
    	    fprintf(stderr, "Unable to create temp file %s reason: %s\n", 
	            tmp_filename,
                    strerror(errno));
        }

	fprintf(tmp_file, "file %s\n", argv[1]);
	fprintf(tmp_file, "set args");
	for(curr_arg = 2; curr_arg < argc; curr_arg++) {
	    fprintf(tmp_file, " %s", argv[curr_arg]);
	}
	fprintf(tmp_file, "\n");
#ifndef NOAUTORUN
	fprintf(tmp_file, "run\n");
#endif
	fclose(tmp_file);

	printf("Extra arguments passed to gdb in file %s.\n"
	       "Be sure to delete it when you're done.\n\n",
	       tmp_filename);
	
        execlp(my_debugger, argv[1], "-x", tmp_filename, NULL);

    }
    else {
	execlp(my_debugger, argv[1], NULL);
    }	

    /* if we fall through to here, our exec failed -- announce the fact */
    fprintf(stderr, "Unable to execute command: %s\n", strerror(errno));

    usage();

}

/* use gcc unaligned.c -o unaliged to compile.  Add -DNOAUTORUN if you
don't want gdb to automatically run the program */
