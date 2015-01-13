#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
	int fds, i;

	printf("Asynchronous I/O:\n");
	fds = open("./write.out", O_WRONLY | O_CREAT, 0600);
	printf("fds = %d (errno: %d = %s)\n", fds, errno, strerror(errno));

	i = write(fds, "Hello World!\n", 13);
	printf("i = %d (errno: %d = %s)\n", i, errno, strerror(errno));
	close(fds);

	printf("Synchronous I/O (O_SYNC):\n");
	fds = open("./write.out2", O_WRONLY | O_CREAT | O_SYNC, 0600);
	printf("fds = %d (errno: %d = %s)\n", fds, errno, strerror(errno));

	i = write(fds, "Hello World!\n", 13);
	printf("i = %d (errno: %d = %s)\n", i, errno, strerror(errno));
	close(fds);
}	
