#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#define BUFFER_SIZE 512

int copy_file(char *,char *,struct stat *);
int copy_dir(char *,char *);
char *path_cat(char *,char *);
void print_err(const char *str,...);
void sys_err(char *str);

int main(int argc,char *argv[])
{
	if(argc>2)
		for(int i=1;i<argc-1;i++)
			copy_dir(argv[i],argv[argc-1]);
	else 
		print_err("argument error\n");
}

//fsin and fsout are regular file
int copy_file(char *fsin,char *fsout,struct stat *finfo_s)
{
	int fin,fout;
	char buf[BUFFER_SIZE];
	
	fin=open(fsin,O_RDONLY);
	if(fin<0)
		print_err("file open error:%s\n",fsin);
	fout=open(fsout,O_WRONLY|O_CREAT|O_EXCL,finfo_s->st_mode);
	if(fout<0)
		print_err("file open error:%s\n",fsout);
	if(fchown(fout,finfo_s->st_uid,finfo_s->st_gid)<0)
	{
		unlink(fsout);
		sys_err(fsout);
	}
	int count;
	while(count=read(fin,buf,BUFFER_SIZE))
		write(fout,buf,count);
	fprintf(stdout,"copy file \"%s\" successfully\n",fsin);
	close(fin);
	close(fout);
	return 0;
}

int copy_dir(char *source,char *desti)
{
	struct stat finfo_s;
	struct stat finfo_d;
	DIR *path;
	struct dirent *dir_s;
	char *spath_s;
	char *spath_d;
	if(lstat(source,&finfo_s))
		sys_err(source);
	if(S_ISDIR(finfo_s.st_mode))
	{
		if(!lstat(desti,&finfo_d))
		{
			if(!S_ISDIR(finfo_d.st_mode))
				print_err("can not copy files to \"%s\"\n",desti);
		}
		else if(mkdir(desti,finfo_s.st_mode))
			sys_err(desti);
		path=opendir(source);
		dir_s=readdir(path);
		while(dir_s!=NULL)
			if(strcmp(dir_s->d_name,".")!=0 && 
					strcmp(dir_s->d_name,"..")!=0)
			{
				spath_s=(char *)malloc(strlen(source)+256);
				spath_d=(char *)malloc(strlen(desti)+256);
				strcpy(spath_s,source);
				path_cat(spath_s,dir_s->d_name);
				strcpy(spath_d,desti);
				path_cat(spath_d,dir_s->d_name);
				copy_dir(spath_s,spath_d);
				free(spath_s);
				free(spath_d);
				readdir_r(path,dir_s,&dir_s);
			}
	}
	else 
	{
		if(S_ISLNK(finfo_s.st_mode)) //check if is a link file
			print_err("reach link file \"%s\",do nothing\n",source);
		spath_d=(char *)malloc(strlen(desti)+256);
		strcpy(spath_d,desti);
		if(!lstat(desti,&finfo_d))
		{
			if(S_ISDIR(finfo_d.st_mode))
			{
				char *pos=strrchr(source,'/');
				if(pos==NULL) pos=source;
				else pos++;
				path_cat(spath_d,pos);
			}
			else 
				print_err("%s is not a directory\n",desti);
		}
		copy_file(source,spath_d,&finfo_s);
		free(spath_d);
	}
}

char *path_cat(char *des,char *src)
{
	char *tmp=strrchr(des,'/');
	char c='/';
	if(tmp==NULL || (tmp-des)!=(strlen(des)-1))
		strcat(des,&c);
	strcat(des,src);
	return des;
}

void print_err(const char *str,...)
{
	va_list ap;
	va_start(ap,str);
	char *tmp=va_arg(ap,char *);
	va_end(ap);
	fprintf(stderr,str,tmp);
	exit(1);
}

void sys_err(char *str)
{
	perror(str);
	exit(1);
}
