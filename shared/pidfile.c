/*___________________________________________________
 |  _____                       _____ _ _       _    |
 | |  __ \                     |  __ (_) |     | |   |
 | | |__) |__ _ __   __ _ _   _| |__) || | ___ | |_  |
 | |  ___/ _ \ '_ \ / _` | | | |  ___/ | |/ _ \| __| |
 | | |  |  __/ | | | (_| | |_| | |   | | | (_) | |_  |
 | |_|   \___|_| |_|\__, |\__,_|_|   |_|_|\___/ \__| |
 |                   __/ |                           |
 |  GNU/Linux based |___/  Multi-Rotor UAV Autopilot |
 |___________________________________________________|
  
 Process Daemonizer with Pidfile Support
 
 Copyright (c) 1995  Martin Schulze <Martin.Schulze@Linux.DE>
 Copyright (C) 2014 Tobias Simon, Integrated Communication Systems Group, TU Ilmenau

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details. */


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#include <signal.h>


/* read_pid
 *
 * Reads the specified pidfile and returns the read pid.
 * 0 is returned if either there's no pidfile, it's empty
 * or no pid can be read.
 */
int read_pid(char *pidfile)
{
   FILE *f;
   int pid;

   if (!(f = fopen(pidfile, "r")))
      return 0;

   if (fscanf(f, "%d", &pid) != 1)
   {
      pid = 0;
      goto out;
   }

out:
   fclose(f);
   return pid;
}


/* check_pid
 *
 * Reads the pid using read_pid and looks up the pid in the process
 * table (using /proc) to determine if the process already exists. If
 * so 1 is returned, otherwise 0.
 */
int check_pid(char *pidfile)
{
   int pid = read_pid(pidfile);

   /* Amazing ! _I_ am already holding the pid file... */
   if ((!pid) || (pid == getpid ()))
      return 0;

   /*
    * The 'standard' method of doing this is to try and do a 'fake' kill
    * of the process.  If an ESRCH error is returned the process cannot
    * be found -- GW
    */
   /* But... errno is usually changed only on error.. */
   if (kill(pid, 0) && errno == ESRCH)
      return 0;

   return pid;
}


/* write_pid
 *
 * Writes the pid to the specified file. If that fails 0 is
 * returned, otherwise the pid.
 */
int write_pid(char *pidfile)
{
   FILE *f;
   int fd;
   int pid;

   if ( ((fd = open(pidfile, O_RDWR | O_CREAT, 0644)) == -1)
         || ((f = fdopen(fd, "r+")) == NULL) )
   {
      fprintf(stderr, "Can't open or create %s.\n", pidfile);
      return 0;
   }

   pid = getpid();
   if (!fprintf(f, "%d\n", pid))
   {
      printf("Can't write pid , %s.\n", strerror(errno));
      close(fd);
      return 0;
   }
   fflush(f);

   close(fd);
   return pid;
}

/* remove_pid
 *
 * Remove the the specified file. The result from unlink(2)
 * is returned
 */
int remove_pid(char *pidfile)
{
   return unlink(pidfile);
}

