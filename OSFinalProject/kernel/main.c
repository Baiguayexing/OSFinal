
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

void loading();
void clear();
void welcome();
////////////////////////////////////////////

#define	MAX_DIRS		32


char location[MAX_FILENAME_LEN] = "root";
int dirmap[MAX_DIRS];
char dir[MAX_DIRS][1024];

PUBLIC int initFS();

PRIVATE void ls();
PRIVATE void cd(char * arg1);
PRIVATE void help();

PRIVATE void newFile(char *filename, char * buf);
PRIVATE void newDirectory(char *filename);
PRIVATE void rmFile(char * filename);
PRIVATE void rmDirectory(char * filename);

PRIVATE int _Read(char * fullname, char * buf);
PRIVATE int _Write(char * fullname, char * buf);
PRIVATE void readFile(char * filename);
PRIVATE void rewriteFile(char * filename);

PRIVATE int checkName(const char * arg1);
PRIVATE int renewDirectory(char * filename, int op);
PRIVATE void getPath(char * fullname, char * filename);
PRIVATE int checkPath(const char * filepath);

////////////////////////////////////////////////
/*****************************************************************************
 *                               kernel_main
 *****************************************************************************/
/**
 * jmp from kernel.asm::_start. 
 * 
 *****************************************************************************/
PUBLIC int kernel_main()
{
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		 "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	int i, j, eflags, prio;
        u8  rpl;
        u8  priv; /* privilege */

	struct task * t;
	struct proc * p = proc_table;

	char * stk = task_stack + STACK_SIZE_TOTAL;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p++,t++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p->p_flags = FREE_SLOT;
			continue;
		}

	        if (i < NR_TASKS) {     /* TASK */
                        t	= task_table + i;
                        priv	= PRIVILEGE_TASK;
                        rpl     = RPL_TASK;
                        eflags  = 0x1202;/* IF=1, IOPL=1, bit 2 is always 1 */
			prio    = 15;
                }
                else {                  /* USER PROC */
                        t	= user_proc_table + (i - NR_TASKS);
                        priv	= PRIVILEGE_USER;
                        rpl     = RPL_USER;
                        eflags  = 0x202;	/* IF=1, bit 2 is always 1 */
			prio    = 5;
                }

		strcpy(p->name, t->name);	/* name of the process */
		p->p_parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0) {
			p->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			/* change the DPLs */
			p->ldts[INDEX_LDT_C].attr1  = DA_C   | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else {		/* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_desc(&p->ldts[INDEX_LDT_C],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_desc(&p->ldts[INDEX_LDT_RW],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 |	SA_TIL | rpl;
		p->regs.ds =
			p->regs.es =
			p->regs.fs =
			p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip	= (u32)t->initial_eip;
		p->regs.esp	= (u32)stk;
		p->regs.eflags	= eflags;

		p->ticks = p->priority = prio;

		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p->filp[j] = 0;

		stk -= t->stacksize;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/**
 * @struct posix_tar_header
 * Borrowed from GNU `tar'
 */
struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

/*****************************************************************************
 *                                untar
 *****************************************************************************/
/**
 * Extract the tar file and store them.
 * 
 * @param filename The tar file.
 *****************************************************************************/
void untar(const char * filename)
{
	printf("[extract `%s'\n", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;

		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;

		/* calculate the file size */
		char * p = phdr->size;
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); /* octal */

		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT | O_RDWR);
		if (fdout == -1) {
			printf("    failed to extract file: %s\n", phdr->name);
			printf(" aborted]");
			return;
		}
		printf("    %s (%d bytes)\n", phdr->name, f_len);
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
			     ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);

	printf(" done]\n");
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

/*****************************************************************************
 *                                Init
 *****************************************************************************/
/**
 * The hen.
 * 
 *****************************************************************************/
void Init()
{
	int fd_stdin  = open("/dev_tty0", O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Init() is running ...\n");

	/* extract `cmd.tar' */
	untar("/cmd.tar");
			

	char * tty_list[] = {"/dev_tty0"};

	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) {
		int pid = fork();
		if (pid != 0) { /* parent process */
			printf("[parent is running, child pid:%d]\n", pid);
		}
		else {	/* child process */
			printf("[child is running, pid:%d]\n", getpid());
			close(fd_stdin);
			close(fd_stdout);
			
			shabby_shell(tty_list[i]);
			assert(0);
		}
	}

	while (1) {
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}


/*======================================================================*
				TestA
*======================================================================*/
void TestA()
{
	for (;;);
}

/*======================================================================*
				TestB
*======================================================================*/
void TestB()
{
	for (;;);
}

/*======================================================================*
				TestC
*======================================================================*/
void TestC()
{
	for (;;);
}


/*****************************************************************************
 *                                shabby_shell
 *****************************************************************************/
/**
 * A very very simple shell.
 * 
 * @param tty_name  TTY file name.
 *****************************************************************************/
void shabby_shell(const char * tty_name)
{
	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
	char cmd[64];

        clear();
	loading();
        welcome();

	while (1) {
		memset(rdbuf, 0, 128);
		memset(cmd, 0, 64);


		printf("$ ");

		int r = read(0, rdbuf, 128);
		rdbuf[r] = 0;

		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;
                int fd = -1; 
                fd = open(argv[0], O_RDWR);

		if (fd == -1) {
			if (rdbuf[0]) {
				int i = 0;
				while (rdbuf[i] != ' ' && rdbuf[i] != 0)
				{
					cmd[i] = rdbuf[i];
					i++;
				}
				/* welcome */
				if (strcmp(cmd, "welcome") == 0)
				{
					welcome();
				}

				/* clear */
				else if (strcmp(cmd, "clear") == 0)
				{
					clear();
				}

				/* help */
				else if (strcmp(cmd, "help") == 0)
				{
					help();
				}
				/* FS */
				else if (strcmp(cmd, "initFS") == 0)
				{
					initFS();
				}

			        else
				{
					printf("Command not found\n");
				}


			}
		}
		else {
			close(fd);
			int pid = fork();
			if (pid != 0) { /* parent */
				int s;
				wait(&s);
			}
			else {	/* child */
				execv(argv[0], argv);
			}
		}
	}

	close(1);
	close(0);
}

/*======================================================================*
 *======================================================================*/

void loading()
{
        printf(" ::::::::  :::::::::      :::     ::::    :::  ::::::::  ::::::::::  :::::::: \n"); 
        printf(":+:    :+: :+:    :+:   :+: :+:   :+:+:   :+: :+:    :+: :+:        :+:    :+:\n"); 
        printf("+:+    +:+ +:+    +:+  +:+   +:+  :+:+:+  +:+ +:+        +:+        +:+       \n"); 
        printf("+#+    +:+ +#++:++#:  +#++:++#++: +#+ +:+ +#+ :#:        +#++:++#   +#++:++#++\n"); 
        printf("+#+    +#+ +#+    +#+ +#+     +#+ +#+  +#+#+# +#+   +#+# +#+               +#+\n"); 
        printf("#+#    #+# #+#    #+# #+#     #+# #+#   #+#+# #+#    #+# #+#        #+#    #+#\n"); 
        printf(" ########  ###    ### ###     ### ###    ####  ########  ##########  ######## \n");
        printf("\n\n\n\n");

}
void clear()
{
	int i = 0;
	for (i = 0; i < 25; i++)
		printf("\n");
}
void welcome()
{
	printf(" *****************************************************************************\n");
	printf(" *                                                                           *\n");
	printf(" *                          1652743    Xu Hao                                *\n");
	printf(" *                          1551615    Bi Jia Rui                            *\n");
	printf(" *                                                                           *\n");
	printf(" *****************************************************************************\n");
	printf(" *                       Input 'help' for more details                       *\n");
	printf(" *****************************************************************************\n");
	printf("\n\n\n\n");
}

void help()
{
	printf("===============================================================================\n");
	printf("=       Command                        Description                            =\n");
	printf("=-----------------------------------------------------------------------------=\n");
	printf("=       welcome                        Print welcome page                     =\n");
	printf("=       clear                          Clean the screen                       =\n");
	printf("=       help                           List all commands                      =\n");
	printf("===============================================================================\n");

}

///////////////////////////////////////////


/*****************************************************************************
*							 initFS
*****************************************************************************/
int initFS()
{
	char rdbuf[128];
	char cmd[64];
	char arg1[MAX_FILENAME_LEN];
	char arg2[MAX_FILENAME_LEN];

	int fd = -1, n = 0;
	char bufr[1024];
	char filepath[1024];
	char dirpath[1024];
	char filename[1024];
	
	fd = open("root", O_CREAT | O_RDWR);
	close(fd);

	fd = open("root", O_RDWR);
	for(int i=0;i<1024;i++){
		bufr[i]=0;
	}
	write(fd, bufr, 1024);
	close(fd);
	
	memset(dirmap, 0, MAX_DIRS);
	for(int i=0;i<MAX_DIRS;i++){
		dirmap[i]=0;
		strcpy(dir[i], "");
	}
	dirmap[0]=1;
	strcpy(dir[0], "root");

	while (1) {

		memset(rdbuf, 0, 128);
		memset(cmd, 0, 64);
		memset(arg1, 0, 1024);
		memset(arg2, 0, 1024);

		printf("%s$ ", location);
		int r = read(0, rdbuf, 128);
		rdbuf[r] = 0;


		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while (ch);
		argv[argc] = 0;
		if (rdbuf[0]) {
			int i = 0, j = 0;
			/* get cmd */
			while (rdbuf[i] != ' ' && rdbuf[i] != 0)
			{
				cmd[i] = rdbuf[i];
				i++;
			}
			i++;
			while (rdbuf[i] != ' ' && rdbuf[i] != 0)
			{
				arg1[j] = rdbuf[i];
				i++;
				j++;
			}
			i++;
			j = 0;
			while (rdbuf[i] != ' ' && rdbuf[i] != 0)
			{
				arg2[j] = rdbuf[i];
				i++;
				j++;
			}

			/////////////////////////////////
			if (strcmp(cmd, "newfile") == 0)
			{
				if(checkName(arg1)==0)continue;
				newFile(arg1, arg2);
			}
			else if (strcmp(cmd, "newdir") == 0)
			{
				if(checkName(arg1)==0)continue;
				newDirectory(arg1);
			}
			else if (strcmp(cmd, "read") == 0)
			{
				if(checkName(arg1)==0)continue;
				readFile(arg1);
			}
			else if (strcmp(cmd, "rmfile") == 0)
			{
				if(checkName(arg1)==0)continue;
				rmFile(arg1);
			}
			else if (strcmp(cmd, "rmdir") == 0)
			{
				if(checkName(arg1)==0)continue;
                rmDirectory(arg1);
			}
			else if (strcmp(cmd, "rwfile") == 0)
			{
				if (checkName(arg1) == 0)continue;
				rewriteFile(arg1);
			}
			else if (strcmp(cmd, "ls") == 0)
			{
				ls();
			}
			else if (strcmp(cmd, "cd") == 0)
			{
				cd(arg1);
			}
			else if (strcmp(cmd, "help") == 0)
			{
				myhelp();
			}
			else if (strcmp(cmd, "q") == 0)
			{
				return 0;
			}
			else
			{
				printf("Command not found\n");
			}
		}
	}
	

}

/*****************************************************************************
*							 checkPath
*****************************************************************************/
int checkPath(const char * dirpath)
{
	for(int i=0;i<MAX_DIRS;i++){
		if(strcmp(dir[i], dirpath)==0)
			return 1;
	}
	return 0;
}

/*****************************************************************************
*							 newFile
*****************************************************************************/
void newFile(char * filename, char * buf)
{
	int fd = -1;
	char fullname[1024];
	getPath(fullname, filename);
	fd = open(fullname, O_CREAT | O_RDWR);
	if (fd == -1)
	{
		//printf("Error!\n");
		return;
	}
	
	write(fd, buf, strlen(buf)+1);

	close(fd);

	renewDirectory(filename,1);
	
}

/*****************************************************************************
*							 renewDirectory
*****************************************************************************/
int renewDirectory(char * filename, int op)
{

	char * dirpath;

	strcpy(dirpath, location);
	strrpl(dirpath,'/','_');

	char bufr[256];

	if(_Read(dirpath, bufr) != 0)
	{
		return -1;
	}

	int namelen = strlen(filename);
	char temp[1024];
	char * t = temp;	
	char * it = bufr;
	switch (op) {
		case 1:/* add */
			strcat(bufr, filename);
			strcat(bufr, " ");
			break;
		case 2:/* remove */
			while(*it){
				if(*it == ' ')
				{
					if(strcmp(filename,temp)==0)
					{
						it++;
						do {
							*(it-namelen-1)=*it;
							it++;
						}while(*it);
						*(it-namelen-1)=0;
						break;
					}
					it++;
					t = temp;
					memset(temp, 0, MAX_FILENAME_LEN);
				}
				else 
				{
					*t++ = *it++;
				}
			}
			break;
		default:
			break;
	}

	
	_Write(dirpath, bufr);

	char bufcheck[256];
	_Read(dirpath, bufcheck);

	return 0;
}
/*****************************************************************************
*							 newDirectory
*****************************************************************************/
void newDirectory(char * filename)
{

	char fullname[1024];
	getPath(fullname, filename);

	int i=0;
	for(;i<MAX_DIRS;i++){
		if(dirmap[i]==0)
			break;
	}
	if(i>=MAX_DIRS)	/* {FS} is full */
	{
		printf("Error! No room!");
		return;
	}

	dirmap[i]=1;
	strcpy(dir[i], fullname);

	int fd = -1;

	fd = open(fullname, O_CREAT | O_RDWR);

	if (fd == -1)
	{
		//printf("Error!\n");
		for(int k=0;k<MAX_DIRS;k++){
			if(strcmp(dir[k], fullname)==0)
			{
				memset(dir[k], 0, MAX_FILENAME_LEN);
				dirmap[k]=0;
				break;
			}
		}
		return;
	}

	char dirbuf[256];
	memset(dirbuf, 0, 256);
	write(fd, dirbuf, 256);


	close(fd);

	renewDirectory(filename,1);
}

/*****************************************************************************
*								readFile
*****************************************************************************/
void readFile(char * filename)
{

	char fullname[1024];
	getPath(fullname, filename);
	if (checkPath(fullname)==1)
	{
		printf("\'%s\' is not exist!\n",filename);
		return;
	}

	char bufr[256];
	if(_Read(fullname, bufr) != 0)
	{
		return;
	}


	char pathname[MAX_FILENAME_LEN];
	memset(pathname, 0, MAX_FILENAME_LEN);
	strcpy(pathname, fullname);
	strrpl(pathname,'_','/');
	printf("==================================================\n");
	printf("Content: \n%s\n", bufr);
	printf("==================================================\n");

}



/*****************************************************************************
*							   rmFile
*****************************************************************************/
void rmFile(char * filename)
{

	char fullname[1024];
	getPath(fullname, filename);

	if (checkPath(fullname) == 1)
	{
		printf("\'%s\' is not a file!\n", filename);
		return;
	}
	if (unlink(fullname) != 0)
	{
		printf("Error !\n");
		return;
	}

	renewDirectory(filename,2);
	
}

/*****************************************************************************
*							 rmDirectory
*****************************************************************************/
void rmDirectory(char * filename)
{
	char fullname[1024];
	getPath(fullname, filename);

	if (checkPath(fullname) == 0)
	{
		printf("\'%s\' is not exist!\n", filename);
		return;
	}

	cd(filename);
	char buf[256];
	_Read(fullname, buf);
	char temp[MAX_FILENAME_LEN];
	char * t = temp;
	char * it = buf;

	while (*it) {
		if (*it == ' ')
		{
			*t = 0;
			getPath(fullname, temp);
			unlink(fullname);
			printf("delete [%s/%s]\n", location, temp);
			if (checkPath(fullname) == 1)
			{

				int i = 0;
				for (; i<MAX_DIRS; i++) {
					if (strcmp(dir[i], fullname) == 0)
						break;
				}
				dirmap[i] = 0;
				memset(dir[i], 0, MAX_FILENAME_LEN);
			}


			it++;
			t = temp;
			memset(temp, 0, MAX_FILENAME_LEN);
		}
		else
		{
			*t++ = *it++;
		}
	}


	cd("..");
	getPath(fullname, filename);
	unlink(fullname);

	renewDirectory(filename, 2);

	int i = 0;
	for (; i<MAX_DIRS; i++) {
		if (strcmp(dir[i], fullname) == 0)
			break;
	}
	dirmap[i] = 0;
	memset(dir[i], 0, MAX_FILENAME_LEN);
}

/*****************************************************************************
*					               ls
*****************************************************************************/
void ls()
{

	char dirpath[1024];
	memset(dirpath, 0, 1024);
	strcpy(dirpath, location);
	strrpl(dirpath,'/','_');

	char bufr[256];
	_Read(dirpath, bufr);

	printf("%s\n", bufr);
}

/*****************************************************************************
*			                                 cd
*****************************************************************************/
void cd(char * arg1)
{
	char location2[1024];
	memset(location2, 0, 1024);
	strcpy(location2, location);
	char * lo = location2+strlen(location2);
	char * ar = arg1;
	if(strcmp(arg1, "/")==0)
	{
		strcpy(location, "root");
		return;
	}


	if(*ar == '/')
	{
		strcpy(location2, "root/");
		lo=location2+strlen(location2);
		ar++;
	} 
	else
	{
		*lo = '/';
		lo++;
	}
	

	while(*ar){
		if(*ar == '.' && *(ar+1) == '.')	
		{

			lo -= 2;
			*(lo+2) = 0;
			*(lo+1) = 0;

			while(*lo != '/'){
				lo--;
				*(lo+1)=0;
				if(lo == location2)
				{
					printf("cd \'%s\':path invalid !\n", arg1);
					return;
				}
			}
			*lo = 0;
			ar += 2;		
		}
		else
		{
			*lo++ = *ar++;
		}

	}

	lo = location2+strlen(location2)-1;
	if(*lo == '/')
	{
		lo--;
		*(lo+1) = 0;
	}

	char dirpath[1024];
	strcpy(dirpath, location2);
	strrpl(dirpath,'/','_');
	if (checkPath(dirpath)==0)
	{
		printf("cd \'%s\':No such directory!\n", location2);
		return;
	}
	

	strcpy(location, location2);
	
}

/*****************************************************************************
*					myhelp
*****************************************************************************/
void myhelp()
{
	printf("-------------------------------------------------------------------------------\n");
	printf("*     cd      [path]                   Get into the path                      *\n");
	printf("*     ls                               List all the files in current directory*\n");
	printf("*     newfile [filename] [content]     Create a file                          *\n");
	printf("*     newdir  [dirname]                Create a directory                     *\n");
	printf("*     read    [filename]               Read a file                            *\n");
	printf("*     rmfile  [filename]               Remove a file                          *\n");
	printf("*     rmdir   [dirname]                Remove a directory                     *\n");
	printf("*     rwfile  [filename]               Rewrite the content                    *\n");
	printf("*     q                                Return                                 *\n");
	printf("-------------------------------------------------------------------------------\n");

}

/*****************************************************************************
*					_Read
*****************************************************************************/
int _Read(char * fullname, char * buf)
{
	int fd = -1;
	int n;

	fd = open(fullname, O_RDWR);
	memset(buf, 0, 256);
	
	if (fd == -1)
	{
		printf("File not found!\n");
		return -1;
	}

	n = read(fd, buf, 256);
	buf[n] = 0;

	close(fd);
	return 0;	
}
/*****************************************************************************
*					_Write
*****************************************************************************/
int _Write(char * fullname, char * buf)
{
	int fd = -1;

	fd = open(fullname, O_RDWR);

	if (fd == -1)
	{
		printf("File not found!\n");
		return -1;
	}

	write(fd, buf, 256);

	close(fd);
	return 0;
}

/*****************************************************************************
*					rewriteFile
*****************************************************************************/
void rewriteFile(char * filename)
{

	char fullname[1024];
	memset(fullname, 0, 1024);
	getPath(fullname, filename);

	char content[256];
	memset(content, 0, 256);
	printf("Input content:\n");
	int r2 = read(0, content, 256);
	content[r2] = 0;

	if(_Write(fullname, content) != 0)
	{
		return;
	}	

	char bufr[256];
	_Read(fullname, bufr);
	printf("--------------------------------------------------\n");
	printf("New content:\n%s\n", bufr);
	printf("--------------------------------------------------\n");
}

/*****************************************************************************
/*					        checkName
*****************************************************************************/
int checkName(const char * arg1)
{
	int n = strlen(arg1);
	if(n>MAX_FILENAME_LEN)
	{
		printf("Filename length error!\n");
		return 0;
	}
	if(n<=0)
	{
		printf("Filename empty\n");
		return 0;
	}
	char ch;
	for(int i=0;i<n;i++){
		ch = *(arg1+i);
		if(ch=='/' || ch=='.' || ch==' ' || ch=='#')
		{
			printf("Filename error!\n");
			return 0;
		}
	}
	
	return 1;
}

/*****************************************************************************
/*					        getPath
*****************************************************************************/
void getPath(char * fullname, char * filename)
{
	memset(fullname, 0, 1024);
	strcpy(fullname, location);
	strcat(fullname, "/");
	strcat(fullname, filename);
	strrpl(fullname, '/', '_');
}

