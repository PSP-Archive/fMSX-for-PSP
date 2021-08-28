
#include "psp.h"




char *Disks[2][MAXDISKS+1];     /* Disk names for each drive */





int timer_count = 0;
int old_timer_count = 0;
int paused = 0;


int exit_callback(void) 
{ 
// Cleanup the games resources etc (if required) 
	scePowerSetClockFrequency(222,222,111);
	TrashMSX();
	DiskAutoSave();
	ini_write();

// Exit game 
   sceKernelExitGame(); 
   return 0; 
} 

#define POWER_CB_POWER      0x80000000 
#define POWER_CB_HOLDON      0x40000000 
#define POWER_CB_STANDBY   0x00080000 
#define POWER_CB_RESCOMP   0x00040000 
#define POWER_CB_RESUME      0x00020000 
#define POWER_CB_SUSPEND   0x00010000 
#define POWER_CB_EXT      0x00001000 
#define POWER_CB_BATLOW      0x00000100 
#define POWER_CB_BATTERY    0x00000080 
#define POWER_CB_BATTPOWER   0x0000007F 
void power_callback(int unknown, int pwrflags) 
{ 
   // Combine pwrflags and the above defined masks 
} 

// Thread to create the callbacks and then begin polling 
int CallbackThread(void *arg) 
{ 
   int cbid; 

	pgWaitV();
   cbid = sceKernelCreateCallback("Exit Callback", exit_callback); 
   SetExitCallback(cbid); 
   cbid = sceKernelCreateCallback("Power Callback", power_callback); 
   scePowerSetCallback(0, cbid); 

   KernelPollCallbacks(); 
} 

/* Sets up the callback thread and returns its thread id */ 
int SetupCallbacks(void) 
{ 
   int thid = 0; 

	pgWaitV();
   thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0); 
   if(thid >= 0) 
   { 
      sceKernelStartThread(thid, 0, 0); 
   } 

   return thid; 
} 


#define		Vtimer	16666
unsigned int	syorioti=0;
unsigned int	syorioti_c;
	ctrl_data_t ctl;
int      VSyncTimer()
{
	int l=0;
	{
		static int f=0;
		static int t=0;
		int	c;

		if( ctl.frame+(Vtimer*30) < syorioti ){	//カウンタ折り返し	4294967295
/*
				wavoutStopPlay1();
				sceCtrlRead(&ctl,1);
				syorioti=ctl.frame-(16666*3);
				syorioti_c=0;
				wavoutStartPlay1();
			syorioti-=(DWORD)1073741824;
			syorioti-=(DWORD)1073741824;
			syorioti-=(DWORD)1073741824;
			syorioti-=(DWORD)1073741824;
*/
				syorioti=ctl.frame-(16666*3);
				syorioti_c=0;
		}

		if( (++t)==60 ){syorioti += 40;	t=0;}
		syorioti += Vtimer;
		if( ctl.frame < syorioti ){	//合格
			if( !f ){
			//	while( ctl.frame < syorioti ){
			//		sceCtrlRead(&ctl,1);
			//	}
			}else{
				f=0;
			}
			l++;
		}else{
			l++;
			f=1;
			while( ctl.frame >= syorioti ){
				l++;
				if( (++t)==60 ){syorioti += 40;	t=0;}
				syorioti += Vtimer;
			}
		}
	}
	{
		static int	fs=0;
		if( ctl.buttons&ini_data.key[11] ){
		//	syorioti += Vtimer*20;
			wavoutStartPlay1();
			fs=1;
			l+=50;
		}else
		if( fs ){
			wavoutStartPlay1();
			fs=0;
		}
	}
	//		FrameSkip2 = l;//FrameSkip;
	syorioti_c=l;
	return l;

}

int	msxchk(const char *n)
{
	while(*n){
		if( n[0]=='.' &&
			(n[1]=='r'||n[1]=='R') &&
			(n[2]=='o'||n[2]=='O') &&
			(n[3]=='m'||n[3]=='M') &&
			!n[4] 
		)return 1;
		if( n[0]=='.' &&
			(n[1]=='b'||n[1]=='B') &&
			(n[2]=='i'||n[2]=='I') &&
			(n[3]=='n'||n[3]=='N') &&
			!n[4] 
		)return 1;
		if( n[0]=='.' &&
			(n[1]=='d'||n[1]=='D') &&
			(n[2]=='s'||n[2]=='S') &&
			(n[3]=='k'||n[3]=='K') &&
			!n[4] 
		)return 1;
		n++;
	}
	return 0;
}
int		zip_read_size;
char	*zip_read_buff;
int funcUnzipCallback(int nCallbackId,
                      unsigned long ulExtractSize,
		      unsigned long ulCurrentPosition,
                      const void *pData,
                      unsigned long ulDataSize,
                      unsigned long ulUserData)
{
    const char *pszFileName;
    const unsigned char *pbData;

    switch(nCallbackId) {
    case UZCB_FIND_FILE:
		pszFileName = (const char *)pData;
		switch( ulUserData ){
		case	0:	//	汎用
		{
			if( zip_path_buffer2[0] ){
				if( !_strcmp( zip_path_buffer2, pszFileName) ){
			zip_read_size=0;
					return  UZCBR_OK;
				}else 
					return  UZCBR_PASS;
			}else
			if( !msxchk( pszFileName) ){
				return  UZCBR_PASS;
			}else
			if( ulExtractSize < 524288+1 ){
				zip_read_size=0;
				return  UZCBR_OK;
			}else{
				return  UZCBR_CANCEL;
			}

		}
			break;
		case	1:	//	480x272x32 bmp
			if( zip_path_buffer2[0] ){
				if( !_strcmp( zip_path_buffer2, pszFileName) ){
			zip_read_size=0;
					return  UZCBR_OK;
				}else 
					return  UZCBR_PASS;
			}else
			if( ulExtractSize == 391736 ){
			zip_read_size=0;
				return  UZCBR_OK;
			}else{
				return  UZCBR_PASS;
			}
			break;
		case	2:	//	menu_file zipの中身
			if( (dlist_num>=MAXDIRNUM) )return  UZCBR_CANCEL;
			if(	!kakutyousichk(0,(void*)pszFileName) )return  UZCBR_PASS;
			memcpy(dlist[dlist_num].name, pszFileName, MAXDEPTH);
			dlist[dlist_num].type = TYPE_DIR;
			dlist_num++;
			return  UZCBR_PASS;
		}
        break;
    case UZCB_EXTRACT_PROGRESS:
		pbData = (const unsigned char *)pData;
		switch( ulUserData ){
		case	0:
		case	1:
			{
				_memcpy( zip_read_buff+zip_read_size, (void *)pData, ulDataSize);
				zip_read_size += ulDataSize;
			}
			break;
		}
		return  UZCBR_OK;
        break;
    default: // unknown...
        break; 
    }
    return 1;
}
int		ini_frame=0;
int xmain(int argc, char *argv)
{
	{
		char *p;
		_strcpy(path_main, argv);
		p = _strrchr(path_main, '/');
		*++p = 0;
	}
	{
		char savedir[MAXPATH];
		_strcpy(savedir,path_main);
		_strcat(savedir,"SAVE");
		sceIoMkdir(savedir,0777);
	}

	pgInit();
	SetupCallbacks();
	pgScreenFrame(2,0);
	Unzip_setCallback(funcUnzipCallback);
	wavoutInit();

	menu_init();

	
//	text_print( 0, 0, "テスト", rgb2col(255,255,55),rgb2col(50,0,0),0);
//	Error_mes("テスト２");
	
	
	
	
	
	if(InitMachine()) return 0;


	menu_main();
	while(ExitNow==2){
		StartMSX();
	}



	scePowerSetClockFrequency(222,222,111);

	return 0;
}


