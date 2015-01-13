#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

main()
{
	int i;
	int four;
	int *address_of_four = &four;
	int s;
	int ret;
	struct	sockaddr_in lcladdr;
	int port = 0;
	
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( s == -1 ) {
		printf("Canna open a socket\n");
		exit(2);
	}
	memset((char *) &lcladdr, 0, sizeof(lcladdr));
	lcladdr.sin_family      = AF_INET;
	lcladdr.sin_port        = htons((short)port);
	lcladdr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(s, (struct sockaddr *)&lcladdr, sizeof(lcladdr));
	if (ret==-1){
		printf("Bind failed \n");
		exit(2);
	}	
	for (i = 8196; i ; i--) 
	{
		int arg;
		arg = i;
		ret = setsockopt(s, SOL_SOCKET, SO_RCVBUF,
                       (char *)&arg, sizeof(arg));
		if ( ret < 0 ) {
			printf("Setsockopt:SO_RCVBUF failed for %d\n", arg);
			exit(3);
		}
		ret = getsockopt(s, SOL_SOCKET, SO_RCVBUF,
                       (char *)&arg, address_of_four);
                if ( ret < 0 ) {
                        printf("Getsockopt:SO_RCVBUF failed for %d\n", arg);
                        exit(4);
                }
		if ( arg != i ) {
			printf("SO_RCVBUF Failed to take: get:%d != set:%d \n",
				arg, i );
		}
	}
}
