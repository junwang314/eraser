#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include <sys/param.h> /* for MAXPATHLEN */

static pthread_t e_dumper;

int dump_heap(int pid)
{
	FILE *f;
	char mapsfilename[PATH_MAX];
	sprintf(mapsfilename, "/proc/%d/maps", pid);
	if ((f = fopen (mapsfilename, "r")) == NULL) {
		printf ("Could not open %s\n", mapsfilename);
		return -1;
	}

	char buf[PATH_MAX+100], perm[5], dev[6], mapname[PATH_MAX];
	unsigned long begin, end, inode, foo;
	unsigned long addr=0, endaddr=0;

	while(!feof(f)) {
		if(fgets(buf, sizeof(buf), f) == 0)
			break;
		mapname[0] = '\0';
		sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &begin, &end, perm,
				&foo, dev, &inode, mapname);
		printf("%lx, %lx, %s\n", begin, end, mapname);
		if (strcmp(mapname, "[heap]")==0) {
			addr = begin;
			endaddr = end;
			break;
		}
	}
	fclose(f);

	if (addr==0 || endaddr==0) {
		return -1;
	}

	char dumpfilename[PATH_MAX];
	static int i=0;
	sprintf(dumpfilename, "heap_dump.%d.%d", pid, i++);
	FILE *dump = fopen(dumpfilename, "w+");
	fwrite((void *)addr, 1, endaddr-addr, dump);
	fclose(dump);
	return 0;
}

void *dumper()
{
	e_ON = 0;
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);

	while (!e_exit) {
		sleep(20);
		dump_heap(getpid());
	}
	return NULL;
}

//int main(){
//
//	char *p;
//	int i;
//	dump_heap(getpid());
//	p = malloc(sizeof(int) * 5);
//	for(i = 0; i < 5; i++){
//		p[i] = i;
//	}
//
//	printf("finished\n");
//	dump_heap(getpid());
//	sleep(1000);
//	free(p);
//}
