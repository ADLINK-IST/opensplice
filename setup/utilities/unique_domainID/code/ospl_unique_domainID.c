#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define unused(v) (void)v
static const int MIN_UNIQ_ID=0;
static const int MAX_UNIQ_ID=230;
const char *const lockdir="/tmp/ospllocks";
const char *const devlock="/tmp/ospllocks/ospl_id";

static sem_t done;

static void signal_handler( int status )
{
   unused(status);
   (void)sem_post( &done );
}


static char * claimid( int i )
{
   pid_t pidBytes=sizeof(pid_t);
   pid_t otherPid;
   pid_t progPid;
   size_t pidW;

   int lock_fd_rw;
   int lock_fd;
   int locknamelen;
   char *lockfile;

   progPid=getpid();

   locknamelen = strlen(devlock) + 4;
   lockfile = malloc( locknamelen );
   strcpy( lockfile, devlock );
   snprintf( lockfile+strlen(lockfile), locknamelen-strlen(lockfile), "%d", i );

   lock_fd = open(lockfile, O_WRONLY | O_CREAT | O_EXCL, 0666);
   if ( lock_fd == -1 && errno == EEXIST )
   {
      lock_fd_rw=open(lockfile, O_RDWR);
      if ( lock_fd_rw != -1 )
      {
         if ( read( lock_fd_rw, &otherPid, sizeof(pid_t) ) == sizeof(pid_t)
              && kill( otherPid, 0 ) == -1
              && errno == ESRCH )
         {
            /* Device locked but process owning lock has exited, reseting lock.*/
            fflush(stderr);
            (void)close(lock_fd_rw);
            (void)unlink(lockfile);
            lock_fd=open(lockfile, O_WRONLY | O_CREAT | O_EXCL, 0644);
         }
         else
         {
             (void)close(lock_fd_rw);
         }
      }
   }
   if ( lock_fd != -1 )
   {
      while( pidBytes > 0 )
      {
         pidW = write(lock_fd, &progPid, pidBytes);
         if ( pidW < 1 )
         {
            if ( errno != EINTR)
            {
               fflush(stderr);
               (void)close(lock_fd);
               lock_fd = -1;
               (void)unlink(lockfile);
               break;
            }
         }
         else
         {
            pidBytes -= pidW;
         }
      }
   }

   if ( lock_fd != -1 )
   {
       (void)close(lock_fd);
   }
   return(lock_fd != -1 ? lockfile : NULL);
}

int main(int argc, char **argv)
{
   int i;
   struct sigaction sat;

   char * lockfile;
   umask(0000);
   mkdir(lockdir, 0777 );

   unused(argc);

   for ( i = MAX_UNIQ_ID; i > MIN_UNIQ_ID; i-- )
   {
      lockfile = claimid( i );
      if ( lockfile != NULL )
      {
         fprintf(stdout, "%d\n", i);
         fflush(stdout);
         break;
      }
   }

   if ( i > MIN_UNIQ_ID )
   {
      (void)sem_init( &done, 0, 0 );

      sat.sa_handler = signal_handler;
      (void)sigemptyset( &sat.sa_mask);
      sat.sa_flags = 0;
      sigaction(SIGTERM,&sat,NULL);
      sigaction(SIGINT,&sat,NULL);
      sigaction(SIGHUP,&sat,NULL);

      (void)sem_wait( &done );

      (void)unlink(lockfile);
   }
   else
   {
      fprintf(stderr, "%s ERROR: no unique id's available.\n", argv[0]);
   }

   return( i > MIN_UNIQ_ID ? 0 : 1 );
}
