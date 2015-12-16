/* $Id: gethostname.c,v 1.1 2005-10-04 13:32:32 pkruse Exp $ */
/* shared library that overrides the gethostname() system call */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int gethostname(char *name, size_t len) {
  char line[1024];
  char *ptr;
  FILE *file = fopen("/etc/qlustar/common/slurm-llnl/slurm.conf","r");

  if(file == NULL) {
    fprintf(stderr,"error opening file\n");
    return -1;
  }
  while (fgets(line,sizeof line,file) != NULL) {
    if(strncasecmp(line,"ControlMachine=",15) == 0) {
      ptr = strtok(line,"=\n");
      ptr = strtok(NULL,"=\n");

      snprintf(name,sizeof line,"%s",ptr);
      break;
    }
  }
  fclose(file);
  return 0;
}
