#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 0
#define W 1

//rw mean fd, as well as fd[] index
void redirect(int rw, int fd[]) {
	//close stand io fd 0 or 1
	close(rw);
	//dup it to get a new fd, just the close one
	dup(fd[rw]);
	//dup get two same file in different fd, close the latter one
	close(fd[rw]);
}

//always keep the process open file table in size 3
int filter(int infd) {
	int p;
	if(read(infd, &p, sizeof(p))) {
		printf("prime %d\n", p);

		int outfd[2];
		pipe(outfd);

		int id=fork();
		if(id == 0) {
			redirect(R, outfd);
			close(outfd[W]);
			filter(R);
		}else {
			int n;
			redirect(W, outfd);
			close(outfd[R]);
			while(read(infd, &n, sizeof(n))) {
				if(n%p != 0) {
					write(W, &n, sizeof(n));
				}
			}
			close(W);
			wait(0);
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int ffd[2];

	//allocate first,so parent and child can have fd open
	pipe(ffd);

	int id = fork();
	if(id == 0) {
		//must redirect first and then close
		redirect(R, ffd);
		//close write in child 
		close(ffd[W]);
		//read it and start new process
		filter(R);
	}else {
		redirect(W, ffd);
		//close read in parent
		close(ffd[R]);
		//generating process to send 2 to 35
		for(int i=2; i<36; i++) {
			write(W, &i, sizeof(i));
		}
		//close is to signal child read to end
		close(W);
		wait(0);
	}
	exit(0);
}
