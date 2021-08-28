/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                          Disk.c                         **/
/**                                                         **/
/** This file contains standard disk access drivers working **/
/** with disk images from files.                            **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2005                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef DISK

#include "MSX.h"
#include "Floppy.h"
#include "Disk.h"
#include "..\psp\psp.h"
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifdef MSDOS
#include <io.h>
#endif

#ifdef WINDOWS
#include <io.h>
#endif

#ifdef UNIX
#include <unistd.h>
#endif
*/
#ifndef O_BINARY
#define O_BINARY 0
#endif

static int Drives[2] = { -1,-1 };      /* Disk image handles */
static byte *Images[2] = { 0,0 };      /* Disk image buffers */
static int RdOnly[2];                  /* 1 = read-only      */ 

unsigned char	Disk_buffer[2][737280];
unsigned char	*Disk_p[2];

/** DiskPresent() ********************************************/
/** Return 1 if disk drive with a given ID is present.      **/
/*************************************************************/
byte DiskPresent(byte ID)
{
  return((ID>=0)&&(ID<MAXDRIVES)&&((Drives[ID]>=0)||Images[ID]));
}

/** DiskRead() ***********************************************/
/** Read requested sector from the drive into a buffer.     **/
/*************************************************************/
byte DiskRead(byte ID,byte *Buf,int N)
{
  if((ID>=0)&&(ID<MAXDRIVES))
  {
    if(Images[ID]&&(N>=0)&&(N<DSK_SECS_PER_DISK))
    { _memcpy(Buf,Images[ID]+N*512,512);return(1); }
/*
    if(Drives[ID]>=0)
      if(lseek(Drives[ID],N*512L,0)==N*512L)
        return(read(Drives[ID],Buf,512)==512);
*/
		if(Drives[ID]>=0){
			Disk_p[ID] = &Disk_buffer[ID][N*512L];
			_memcpy( Buf, Disk_p, 512);
			Disk_p[ID]+=512;
			return	1;
		}
  }
  return(0);
}

int				DiskAutoSave_zip[2];
/** DiskWrite() **********************************************/
/** Write contents of the buffer into a given sector of the **/
/** disk.                                                   **/
/*************************************************************/
byte DiskWrite(byte ID,byte *Buf,int N)
{
//	Error_mes("ディスク書き込みしてるだろ？");
  if((ID>=0)&&(ID<MAXDRIVES)&&!RdOnly[ID])
  {
    if(Images[ID]&&(N>=0)&&(N<DSK_SECS_PER_DISK)){
			if( !DiskAutoSave_zip[ID] )DiskAutoSave_flag[ID]=1;	//	書き込みしたかのフラグ
		 _memcpy(Images[ID]+N*512,Buf,512);return(1); 
	 }
/*
    if(Drives[ID]>=0)
      if(lseek(Drives[ID],N*512L,0)==N*512L)
        return(write(Drives[ID],Buf,512)==512);
*/
		if(Drives[ID]>=0){
			Disk_p[ID] = &Disk_buffer[ID][N*512L];
			_memcpy( Disk_p, Buf, 512);
			Disk_p[ID]+=512;
			if( !DiskAutoSave_zip[ID] )DiskAutoSave_flag[ID]=1;	//	書き込みしたかのフラグ
			return	1;
		}
  }
  return(0);
}

/** ChangeDisk() *********************************************/
/** Change disk image in a given drive. Closes current disk **/
/** image if Name=0 was given. Returns 1 on success or 0 on **/
/** failure.                                                **/
/*************************************************************/
byte ChangeDisk(byte ID,char *Name)
{
	char *p;
  byte Magic[8];

  // We only have MAXDRIVES drives //
  if(ID>=MAXDRIVES) return(0);

  // Close previous disk image //
//  if(Drives[ID]>=0) { close(Drives[ID]);Drives[ID]=-1; }
//  if(Images[ID])    { free(Images[ID]);Images[ID]=0; }
  if(Drives[ID]>=0) { Drives[ID]=-1; }
  if(Images[ID])    { Images[ID]=0; }

  // If no disk image given, consider drive empty //
  if(!Name) return(1);


	if( (p=zipchk( Name )) ){
		zip_read_buff = Disk_buffer[ID];
 		if( Unzip_execExtract( p, 0) == UZEXR_OK ){
		//	Len = size = zip_read_size;
	//		Drives[ID]=1;
			DiskAutoSave_zip[ID]=1;	
		}else{
		//	return	0;
		}
	}else{
		int fd;
		fd = sceIoOpen( Name,O_RDONLY, 0777);
		if(fd>=0){
			sceIoRead(fd, Disk_buffer[ID], 737280);
			sceIoClose(fd);
	//		Drives[ID]=1;
			DiskAutoSave_zip[ID]=0;	
		}
		
	}
	RdOnly[ID]=0;
	Disk_p[ID] = &Disk_buffer[ID][0];
	if(Drives[ID]>=0){
		Magic[0] = *Disk_p[ID]++;
		if( (Magic[0]!=0xEB) ){
			RdOnly[ID]=0;Drives[ID]=-1; 
		}
	}
	Images[ID] = &Disk_buffer[ID][0];
/*
  // Open new disk image //
  Drives[ID]=open(Name,O_RDWR|O_BINARY);
  RdOnly[ID]=0;

  // If failed to open for writing, open read-only //
  if(Drives[ID]<0)
  {
    Drives[ID]=open(Name,O_RDONLY|O_BINARY);
    RdOnly[ID]=1;
  }

  // Verify that open file is really a disk image //
  if(Drives[ID]>=0)
    if((read(Drives[ID],&Magic,1)!=1)||(Magic[0]!=0xEB))
    { close(Drives[ID]);RdOnly[ID]=0;Drives[ID]=-1; }
*/
  // If failed to open as a plain file, //
  // try creating disk image in memory  //
//  if(Drives[ID]<0) Images[ID]=DSKLoad(Name,0);

  // Return operation result //
  return(Images[ID]||(Drives[ID]>=0));

	return 0;
}

#endif /* DISK */
