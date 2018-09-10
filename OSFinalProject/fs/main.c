/*************************************************************************//**
 *****************************************************************************
 * @file   main.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2007
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "config.h"
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

#include "hd.h"

///////////////////////////////////////////////////////////


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



////////////////////////////////////////////////////////////////

PRIVATE void init_fs();
PRIVATE void mkfs();
PRIVATE void read_super_block(int dev);
PRIVATE int fs_fork();
PRIVATE int fs_exit();

/*****************************************************************************
 *                                task_fs
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK FS.
 * 
 *****************************************************************************/
PUBLIC void task_fs()
{
	printl("{FS} Task FS begins.\n");

	init_fs();

	while (1) {
		send_recv(RECEIVE, ANY, &fs_msg);

		int msgtype = fs_msg.type;
		int src = fs_msg.source;
		pcaller = &proc_table[src];

		switch (msgtype) {
		case OPEN:
			fs_msg.FD = do_open();
			break;
		case CLOSE:
			fs_msg.RETVAL = do_close();
			break;
		case READ:
		case WRITE:
			fs_msg.CNT = do_rdwt();
			break;
		case UNLINK:
			fs_msg.RETVAL = do_unlink();
			break;
		case RESUME_PROC:
			src = fs_msg.PROC_NR;
			break;
		case FORK:
			fs_msg.RETVAL = fs_fork();
			break;
		case EXIT:
			fs_msg.RETVAL = fs_exit();
			break;
		/* case LSEEK: */
		/* 	fs_msg.OFFSET = do_lseek(); */
		/* 	break; */
		case STAT:
			fs_msg.RETVAL = do_stat();
			break;
		default:
			dump_msg("FS::unknown message:", &fs_msg);
			assert(0);
			break;
		}

#ifdef ENABLE_DISK_LOG
		char * msg_name[128];
		msg_name[OPEN]   = "OPEN";
		msg_name[CLOSE]  = "CLOSE";
		msg_name[READ]   = "READ";
		msg_name[WRITE]  = "WRITE";
		msg_name[LSEEK]  = "LSEEK";
		msg_name[UNLINK] = "UNLINK";
		/* msg_name[FORK]   = "FORK"; */
		/* msg_name[EXIT]   = "EXIT"; */
		/* msg_name[STAT]   = "STAT"; */

		switch (msgtype) {
		case UNLINK:
			//dump_fd_graph("%s just finished. (pid:%d)", msg_name[msgtype], src);
			//panic("");
		case OPEN:
		case CLOSE:
		case READ:
		case WRITE:
		case FORK:
		case EXIT:
		/* case LSEEK: */
		case STAT:
			break;
		case RESUME_PROC:
			break;
		default:
			assert(0);
		}
#endif

		/* reply */
		if (fs_msg.type != SUSPEND_PROC) {
			fs_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &fs_msg);
		}
	}
}

/*****************************************************************************
 *                                init_fs
 *****************************************************************************/
/**
 * <Ring 1> Do some preparation.
 * 
 *****************************************************************************/
PRIVATE void init_fs()
{
	int i;

	/* f_desc_table[] */
	for (i = 0; i < NR_FILE_DESC; i++)
		memset(&f_desc_table[i], 0, sizeof(struct file_desc));

	/* inode_table[] */
	for (i = 0; i < NR_INODE; i++)
		memset(&inode_table[i], 0, sizeof(struct inode));

	/* super_block[] */
	struct super_block * sb = super_block;
	for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
		sb->sb_dev = NO_DEV;

	/* open the device: hard disk */
	MESSAGE driver_msg;
	driver_msg.type = DEV_OPEN;
	driver_msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	/* make FS */
	mkfs();

	/* load super block of ROOT */
	read_super_block(ROOT_DEV);

	sb = get_super_block(ROOT_DEV);
	assert(sb->magic == MAGIC_V1);

	root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

/*****************************************************************************
 *                                mkfs
 *****************************************************************************/
/**
 * <Ring 1> Make a available Orange'S FS in the disk. It will
 *          - Write a super block to sector 1.
 *          - Create three special files: dev_tty0, dev_tty1, dev_tty2
 *          - Create a file cmd.tar
 *          - Create the inode map
 *          - Create the sector map
 *          - Create the inodes of the files
 *          - Create `/', the root directory
 *****************************************************************************/
PRIVATE void mkfs()
{
	MESSAGE driver_msg;
	int i, j;

	/************************/
	/*      super block     */
	/************************/
	/* get the geometry of ROOTDEV */
	struct part_info geo;
	driver_msg.type		= DEV_IOCTL;
	driver_msg.DEVICE	= MINOR(ROOT_DEV);
	driver_msg.REQUEST	= DIOCTL_GET_GEO;
	driver_msg.BUF		= &geo;
	driver_msg.PROC_NR	= TASK_FS;
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	printl("{FS} dev size: 0x%x sectors\n", geo.size);

	int bits_per_sect = SECTOR_SIZE * 8; /* 8 bits per byte */
	/* generate a super block */
	struct super_block sb;
	sb.magic	  = MAGIC_V1; /* 0x111 */
	sb.nr_inodes	  = bits_per_sect;
	sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
	sb.nr_sects	  = geo.size; /* partition size in sector */
	sb.nr_imap_sects  = 1;
	sb.nr_smap_sects  = sb.nr_sects / bits_per_sect + 1;
	sb.n_1st_sect	  = 1 + 1 +   /* boot sector & super block */
		sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
	sb.root_inode	  = ROOT_INODE;
	sb.inode_size	  = INODE_SIZE;
	struct inode x;
	sb.inode_isize_off= (int)&x.i_size - (int)&x;
	sb.inode_start_off= (int)&x.i_start_sect - (int)&x;
	sb.dir_ent_size	  = DIR_ENTRY_SIZE;
	struct dir_entry de;
	sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
	sb.dir_ent_fname_off = (int)&de.name - (int)&de;

	memset(fsbuf, 0x90, SECTOR_SIZE);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

	/* write the super block */
	WR_SECT(ROOT_DEV, 1);

	printl("{FS} devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
	       "        inodes:0x%x00, 1st_sector:0x%x00\n", 
	       geo.base * 2,
	       (geo.base + 1) * 2,
	       (geo.base + 1 + 1) * 2,
	       (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
	       (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
	       (geo.base + sb.n_1st_sect) * 2);

	/************************/
	/*       inode map      */
	/************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	for (i = 0; i < (NR_CONSOLES + 3); i++)
		fsbuf[0] |= 1 << i;

	assert(fsbuf[0] == 0x3F);/* 0011 1111 :
				  *   || ||||
				  *   || |||`--- bit 0 : reserved
				  *   || ||`---- bit 1 : the first inode,
				  *   || ||              which indicates `/'
				  *   || |`----- bit 2 : /dev_tty0
				  *   || `------ bit 3 : /dev_tty1
				  *   |`-------- bit 4 : /dev_tty2
				  *   `--------- bit 5 : /cmd.tar
				  */
	WR_SECT(ROOT_DEV, 2);

	/************************/
	/*      secter map      */
	/************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	int nr_sects = NR_DEFAULT_FILE_SECTS + 1;
	/*             ~~~~~~~~~~~~~~~~~~~|~   |
	 *                                |    `--- bit 0 is reserved
	 *                                `-------- for `/'
	 */
	for (i = 0; i < nr_sects / 8; i++)
		fsbuf[i] = 0xFF;

	for (j = 0; j < nr_sects % 8; j++)
		fsbuf[i] |= (1 << j);

	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

	/* zeromemory the rest sector-map */
	memset(fsbuf, 0, SECTOR_SIZE);
	for (i = 1; i < sb.nr_smap_sects; i++)
		WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);

	/* cmd.tar */
	/* make sure it'll not be overwritten by the disk log */
	assert(INSTALL_START_SECT + INSTALL_NR_SECTS < 
	       sb.nr_sects - NR_SECTS_FOR_LOG);
	int bit_offset = INSTALL_START_SECT -
		sb.n_1st_sect + 1; /* sect M <-> bit (M - sb.n_1stsect + 1) */
	int bit_off_in_sect = bit_offset % (SECTOR_SIZE * 8);
	int bit_left = INSTALL_NR_SECTS;
	int cur_sect = bit_offset / (SECTOR_SIZE * 8);
	RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
	while (bit_left) {
		int byte_off = bit_off_in_sect / 8;
		/* this line is ineffecient in a loop, but I don't care */
		fsbuf[byte_off] |= 1 << (bit_off_in_sect % 8);
		bit_left--;
		bit_off_in_sect++;
		if (bit_off_in_sect == (SECTOR_SIZE * 8)) {
			WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
			cur_sect++;
			RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
			bit_off_in_sect = 0;
		}
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);

	/************************/
	/*       inodes         */
	/************************/
	/* inode of `/' */
	memset(fsbuf, 0, SECTOR_SIZE);
	struct inode * pi = (struct inode*)fsbuf;
	pi->i_mode = I_DIRECTORY;
	pi->i_size = DIR_ENTRY_SIZE * 5; /* 5 files:
					  * `.',
					  * `dev_tty0', `dev_tty1', `dev_tty2',
					  * `cmd.tar'
					  */
	pi->i_start_sect = sb.n_1st_sect;
	pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	/* inode of `/dev_tty0~2' */
	for (i = 0; i < NR_CONSOLES; i++) {
		pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
		pi->i_mode = I_CHAR_SPECIAL;
		pi->i_size = 0;
		pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
		pi->i_nr_sects = 0;
	}
	/* inode of `/cmd.tar' */
	pi = (struct inode*)(fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));
	pi->i_mode = I_REGULAR;
	pi->i_size = INSTALL_NR_SECTS * SECTOR_SIZE;
	pi->i_start_sect = INSTALL_START_SECT;
	pi->i_nr_sects = INSTALL_NR_SECTS;
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

	/************************/
	/*          `/'         */
	/************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	struct dir_entry * pde = (struct dir_entry *)fsbuf;

	pde->inode_nr = 1;
	strcpy(pde->name, ".");

	/* dir entries of `/dev_tty0~2' */
	for (i = 0; i < NR_CONSOLES; i++) {
		pde++;
		pde->inode_nr = i + 2; /* dev_tty0's inode_nr is 2 */
		sprintf(pde->name, "dev_tty%d", i);
	}
	(++pde)->inode_nr = NR_CONSOLES + 2;
	sprintf(pde->name, "cmd.tar", i);
	WR_SECT(ROOT_DEV, sb.n_1st_sect);
}

/*****************************************************************************
 *                                rw_sector
 *****************************************************************************/
/**
 * <Ring 1> R/W a sector via messaging with the corresponding driver.
 * 
 * @param io_type  DEV_READ or DEV_WRITE
 * @param dev      device nr
 * @param pos      Byte offset from/to where to r/w.
 * @param bytes    r/w count in bytes.
 * @param proc_nr  To whom the buffer belongs.
 * @param buf      r/w buffer.
 * 
 * @return Zero if success.
 *****************************************************************************/
PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr,
		     void* buf)
{
	MESSAGE driver_msg;

	driver_msg.type		= io_type;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= pos;
	driver_msg.BUF		= buf;
	driver_msg.CNT		= bytes;
	driver_msg.PROC_NR	= proc_nr;
	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

	return 0;
}


/*****************************************************************************
 *                                read_super_block
 *****************************************************************************/
/**
 * <Ring 1> Read super block from the given device then write it into a free
 *          super_block[] slot.
 * 
 * @param dev  From which device the super block comes.
 *****************************************************************************/
PRIVATE void read_super_block(int dev)
{
	int i;
	MESSAGE driver_msg;

	driver_msg.type		= DEV_READ;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= SECTOR_SIZE * 1;
	driver_msg.BUF		= fsbuf;
	driver_msg.CNT		= SECTOR_SIZE;
	driver_msg.PROC_NR	= TASK_FS;
	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

	/* find a free slot in super_block[] */
	for (i = 0; i < NR_SUPER_BLOCK; i++)
		if (super_block[i].sb_dev == NO_DEV)
			break;
	if (i == NR_SUPER_BLOCK)
		panic("super_block slots used up");

	assert(i == 0); /* currently we use only the 1st slot */

	struct super_block * psb = (struct super_block *)fsbuf;

	super_block[i] = *psb;
	super_block[i].sb_dev = dev;
}


/*****************************************************************************
 *                                get_super_block
 *****************************************************************************/
/**
 * <Ring 1> Get the super block from super_block[].
 * 
 * @param dev Device nr.
 * 
 * @return Super block ptr.
 *****************************************************************************/
PUBLIC struct super_block * get_super_block(int dev)
{
	struct super_block * sb = super_block;
	for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
		if (sb->sb_dev == dev)
			return sb;

	panic("super block of devie %d not found.\n", dev);

	return 0;
}


/*****************************************************************************
 *                                get_inode
 *****************************************************************************/
/**
 * <Ring 1> Get the inode ptr of given inode nr. A cache -- inode_table[] -- is
 * maintained to make things faster. If the inode requested is already there,
 * just return it. Otherwise the inode will be read from the disk.
 * 
 * @param dev Device nr.
 * @param num I-node nr.
 * 
 * @return The inode ptr requested.
 *****************************************************************************/
PUBLIC struct inode * get_inode(int dev, int num)
{
	if (num == 0)
		return 0;

	struct inode * p;
	struct inode * q = 0;
	for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
		if (p->i_cnt) {	/* not a free slot */
			if ((p->i_dev == dev) && (p->i_num == num)) {
				/* this is the inode we want */
				p->i_cnt++;
				return p;
			}
		}
		else {		/* a free slot */
			if (!q) /* q hasn't been assigned yet */
				q = p; /* q <- the 1st free slot */
		}
	}

	if (!q)
		panic("the inode table is full");

	q->i_dev = dev;
	q->i_num = num;
	q->i_cnt = 1;

	struct super_block * sb = get_super_block(dev);
	int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
		((num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(dev, blk_nr);
	struct inode * pinode =
		(struct inode*)((u8*)fsbuf +
				((num - 1 ) % (SECTOR_SIZE / INODE_SIZE))
				 * INODE_SIZE);
	q->i_mode = pinode->i_mode;
	q->i_size = pinode->i_size;
	q->i_start_sect = pinode->i_start_sect;
	q->i_nr_sects = pinode->i_nr_sects;
	return q;
}

/*****************************************************************************
 *                                put_inode
 *****************************************************************************/
/**
 * Decrease the reference nr of a slot in inode_table[]. When the nr reaches
 * zero, it means the inode is not used any more and can be overwritten by
 * a new inode.
 * 
 * @param pinode I-node ptr.
 *****************************************************************************/
PUBLIC void put_inode(struct inode * pinode)
{
	assert(pinode->i_cnt > 0);
	pinode->i_cnt--;
}

/*****************************************************************************
 *                                sync_inode
 *****************************************************************************/
/**
 * <Ring 1> Write the inode back to the disk. Commonly invoked as soon as the
 *          inode is changed.
 * 
 * @param p I-node ptr.
 *****************************************************************************/
PUBLIC void sync_inode(struct inode * p)
{
	struct inode * pinode;
	struct super_block * sb = get_super_block(p->i_dev);
	int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
		((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(p->i_dev, blk_nr);
	pinode = (struct inode*)((u8*)fsbuf +
				 (((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE))
				  * INODE_SIZE));
	pinode->i_mode = p->i_mode;
	pinode->i_size = p->i_size;
	pinode->i_start_sect = p->i_start_sect;
	pinode->i_nr_sects = p->i_nr_sects;
	WR_SECT(p->i_dev, blk_nr);
}

/*****************************************************************************
 *                                fs_fork
 *****************************************************************************/
/**
 * Perform the aspects of fork() that relate to files.
 * 
 * @return Zero if success, otherwise a negative integer.
 *****************************************************************************/
PRIVATE int fs_fork()
{
	int i;
	struct proc* child = &proc_table[fs_msg.PID];
	for (i = 0; i < NR_FILES; i++) {
		if (child->filp[i]) {
			child->filp[i]->fd_cnt++;
			child->filp[i]->fd_inode->i_cnt++;
		}
	}

	return 0;
}


/*****************************************************************************
 *                                fs_exit
 *****************************************************************************/
/**
 * Perform the aspects of exit() that relate to files.
 * 
 * @return Zero if success.
 *****************************************************************************/
PRIVATE int fs_exit()
{
	int i;
	struct proc* p = &proc_table[fs_msg.PID];
	for (i = 0; i < NR_FILES; i++) {
		if (p->filp[i]) {
			/* release the inode */
			p->filp[i]->fd_inode->i_cnt--;
			/* release the file desc slot */
			if (--p->filp[i]->fd_cnt == 0)
				p->filp[i]->fd_inode = 0;
			p->filp[i] = 0;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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


