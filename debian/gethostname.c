/* $Id: gethostname.c,v 1.1 2005-10-04 13:32:32 pkruse Exp $ */
/* shared library that overrides the gethostname() system call */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define SLURM_BASE_PATH "/etc/qlustar/common/slurm-llnl"
#define SLURMDBD SLURM_BASE_PATH"/slurmdbd.conf"
#define SLURMCONF "/slurm.conf"

int gethostname(char *name, size_t len) {
  static char hostname[HOST_NAME_MAX] = { 0, };

  // first call of the function?
  if (hostname[0] == 0) {
    char path[PATH_MAX + 1];
    ssize_t res = readlink(SLURMDBD, path, PATH_MAX);
    if (res > 0) {
      // symlink, strip filename
      char *p = rindex(path, '/');
      if (p == NULL) {
	// relative link to file in the same dir, use base path
	strcpy(path, SLURM_BASE_PATH);
      } else {
	// strip filename
	*p = 0;
      }
      if (path[0] != '/') {
	// relative link, combine paths
	size_t base_len= strlen(SLURM_BASE_PATH);
	size_t path_len = strlen(path);
	assert(path_len < PATH_MAX - base_len - 1);
	memmove(path + base_len + 1, path, path_len + 1);
	strcpy(path, SLURM_BASE_PATH);
	path[base_len] = '/';
      }
    } else if (errno == EINVAL) {
      // normal file
      strcpy(path, SLURM_BASE_PATH);
    } else {
      // other error
      perror("readlink("SLURMDBD")");
      return res;
    }

    // open conffile and get hostname
    char line[1024];
    strncat(path, SLURMCONF, PATH_MAX);
    FILE *file = fopen(path,"r");

    if(file == NULL) {
      fprintf(stderr,"error opening file '%s': %s\n", path, strerror(errno));
      return -1;
    }
    while (fgets(line,sizeof(line),file) != NULL) {
      if(strncasecmp(line,"ControlMachine=",15) == 0) {
	char *ptr = strtok(line,"=\n");
	ptr = strtok(NULL,"=\n");
	snprintf(hostname, HOST_NAME_MAX, "%s", ptr);	
	break;
      }
    }
    fclose(file);

    if (hostname[0] == 0) {
      fprintf(stderr, "ControlMachine option missing in '%s'\n", path);
      return EINVAL;
    }
  }
  
  // copy the cached hostname to name
  size_t full_len = snprintf(name, len, "%s", hostname);
  if (len < full_len) return ENAMETOOLONG;
  return 0;
}
