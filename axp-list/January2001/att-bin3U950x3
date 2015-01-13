/*
 * These are the definitions for a DOS/Windows style partition table.
 * Some of this is from disklabel.h, other bits are from the Linux kernel.
 * - 01/01/01 - Alan Young (ayoung@teleport.com)
 */

#ifndef __doslabel_h__
#define __doslabel_h__

#ifndef __KERNEL_STRICT_NAMES
  /* ask kernel to be careful about name-space pollution: */
# define __KERNEL_STRICT_NAMES
# define fd_set kernel_fd_set
#endif

#include <linux/types.h>

#define DOSLABELMAGIC (0x0AA55UL)		/* Magic bytes to ID partition table */

#define DOSMAXPARTITIONS	4		/* max. # of primary partitions */
						/* This is temporary for now... */

/*
 * Filesystem type and version.  Used to interpret other
 * filesystem-specific per-partition information.
 */

/*
 * This is a map of the root partition sector (sector 1, head 0, cylinder 0)
 * (it is referred to as the master boot record (MBR)):
 *
 * Offset             Description
 *
 * 0x000-0x1BD        x86 executable boot code
 * 0x1BE-0x1CD        First primary partition descriptor
 * 0x1CE-0x1DD        Second primary partition descriptor
 * 0x1DE-0x1ED        Third primary partition descriptor
 * 0x1EE-0x1FD        Fourth primary partition descriptor
 * 0x1FE-0x1FF        Partition table magic bytes (0xAA55)
 *
 * Boot code is the Intel x86 executable code that is used to bootstrap a
 * Intel based PC.  The partition descriptors are a structure as described
 * below.
 *
 * Any of the primary partitions can be defined as an "extended" partition.
 * An extended partition creates a linked list of partition entries to
 * exceed the limit of four partitions per disk.
 */

struct DOS_partition_table_entry {
   __u8 boot_indicator;   /* In the DOS world, this contains 0x80 to indicate bootable. */
   __u8 head;             /* partition starts on this disk head			*/
   __u8 sector;           /* ...                           sector		*/
   __u8 cyl;              /* ...                           cylinder		*/
   __u8 sys_ind;          /* This indentifies the filesystem on the partition.	*/
                          /* Any of the primary partitions can be defined as	*/
                          /* an "extended" partition.  An extended partition	*/
                          /* creates a linked list of partition entries to	*/
                          /* exceed the limit of four partitions per disk.	*/
   __u8 end_head;         /* partition ends on this disk head			*/
   __u8 end_sector;       /* ...                         sector			*/
   __u8 end_cyl;          /* ...                         cylinder		*/
   __u32 start_sector;    /* starting sector counting from zero			*/
   __u32 num_sects;       /* number of disk sectors in partition		*/
} __attribute__((packed));


struct doslabel {
       __u8			  boot_code[446];	/* filler for boot code */

       struct DOS_partition_table_entry  DOS_entry[DOSMAXPARTITIONS];

       __u16			  DOS_magic;		/* magic */

} __attribute__((packed));


#ifdef DKTYPENAMES
static char *DOSfstypenames[] = {
	"unused",
	"swap",
	"ext2",
	"fat12",
	"fat16",
	"fat32",
	0
};
#define DOSFSMAXTYPES	(sizeof(DOSfstypenames) / sizeof(DOSfstypenames[0]) - 1)
#endif

#endif /* __doslabel_h__ */
