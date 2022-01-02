#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int match(char *path, char *pattern) {
  char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
	  return 1;

  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));

  if(strlen(buf) != strlen(pattern)) {
	  return -1;
  }

  for(int i=0; i<strlen(buf); i++) {
	  if(buf[i] == pattern[i]) {
		  if((i+1 == strlen(buf))) {
			  return 0;
		  }
	  }else {
		  break;
	  }
  }

  return 1;
}

void
find(char *path, char *pattern)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if(match(path, pattern) == 0) {
	    printf("%s\n", path);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

      if(de.name[0] == '.') {
	      continue;
      }

      memmove(p, de.name, strlen(de.name));
      p[strlen(de.name)] = 0;

      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      find(buf, pattern);
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[])
{
	if(argc != 3) {
		printf("error argument");
		exit(1);
	}

	find(argv[1], argv[2]);

	exit(0);
}
