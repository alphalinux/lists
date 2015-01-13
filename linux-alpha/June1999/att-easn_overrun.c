#include <sys/poll.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>


unsigned long one_page[1024];

int main(int argc,char** argv)
{
  int nr_of_asn;
  int i,status;
  unsigned long my_pid;
  int fds[2];
  struct pollfd pfd[2];
  int *exit_now;
  int shmid;

  shmid = shmget(IPC_PRIVATE,sizeof(int),IPC_CREAT|0777);
  exit_now=shmat(shmid,0,0);
  shmctl(shmid,IPC_RMID,NULL);

  *exit_now=0;
  nr_of_asn=atoi(argv[1]);

  pipe(fds);
  nice(19);

  for(i=0;i<nr_of_asn;i++)
    {
      switch(fork())
	{
	case 0:  /* child */
	  close(fds[0]);
	  my_pid = getpid();
	  for(i=0;i<1024;i++) one_page[i]=my_pid;
	  for(;;)
	    {
	      for(i=0;i<1024;i++)
		{
		  if(one_page[i]!=my_pid) 
		    {
		      printf("%d: found it!!\n",my_pid);
		      write(fds[1]," ",1);
		    }
		  if(*exit_now) exit(0);
		}
	    }
	case -1: /* error */
	default: /* parent */
	}
    }
  close(fds[1]);

  printf("Press a key to abort the test\n");

  pfd[0].fd=0;/*stdin*/
  pfd[0].events=POLLIN|POLLERR|POLLHUP|POLLNVAL;
  pfd[1].fd=fds[0];
  pfd[1].events=POLLIN|POLLERR|POLLHUP|POLLNVAL;

  poll(pfd,2,-1);
  *exit_now=1;

  printf("exiting.\n");

  for(i=0;i<nr_of_asn;i++)
    wait(&status);

  printf("end.\n");
  return 0;
}     
