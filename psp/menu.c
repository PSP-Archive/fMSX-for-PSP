
#include "psp.h"


ini_info	ini_data;

char	title[]="fMSX for PSP 0.61";


char	path_main[MAXPATH];

int		DiskAutoSave_flag[2];
// ---------------------------------------------------------------------------------

dirent_t		dlist[MAXDIRNUM];
int				dlist_curpos;
int				dlist_num;
char			now_path[MAXPATH];
char			path_tmp[MAXPATH];
int				dlist_start;
int				cbuf_start[MAXDEPTH];
int				cbuf_curpos[MAXDEPTH];
int				now_depth;
char			target[MAXPATH];
char			now_path[MAXPATH];
char			path_main[MAXPATH];
unsigned long control_bef_ctl  = 0;
unsigned long control_bef_tick = 0;

// ---------------------------------------------------------------------------------
extern	unsigned char	Disk_buffer[2][737280];
int	DiskAutoSave_s(int i)
{
	if( !DSKName[i][0] ) return 1;
	if( (zipchk( DSKName[i] )) ){
		return	2;
	}else{
		int fd;
		fd = sceIoOpen( DSKName[i], O_CREAT|O_WRONLY|O_TRUNC, 0777);
		if(fd>=0){
			sceIoWrite(fd, Disk_buffer[i], 737280);
			sceIoClose(fd);
		}
	}
	DiskAutoSave_flag[i]=0;
	return	0;
}
void	DiskAutoSave(void)
{
	if( (ini_data.autosave&1) ){
		if( DiskAutoSave_flag[0] )DiskAutoSave_s(0);
		if( DiskAutoSave_flag[1] )DiskAutoSave_s(1);
	}
}

// ---------------------------------------------------------------------------------
char *mh_strncpy(char *s1,char *s2,int n) {
	char *rt = s1;

	while((*s2!=0) && ((n-1)>0)) {
		*s1 = *s2;
		s1++;
		s2++;
		n--;
	}
	*s1 = 0;

	return rt;
}

char *mh_strncat(char *s1,char *s2,int n) {
	char *rt = s1;

	while((*s1!=0) && ((n-1)>0)) {
		s1++;
		n--;
	}

	while((*s2!=0) && ((n-1)>0)) {
		*s1 = *s2;
		s1++;
		s2++;
		n--;
	}
	*s1 = 0;

	return rt;
}


typedef struct tagBITMAPINFOHEADER{
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImporant;
} BITMAPINFOHEADER;
#define		BmpBuffer_Max	(391736+1)
char	BmpBuffer[BmpBuffer_Max];
short	BmpBuffer2[3][480*272];
int	bmp_f[3];
int	bmp_i=-1;
void	BmpAkarusa(int i)
{
	if(!bmp_f[i])return;
	{
		int x,y;
		unsigned char *rb=(void *)&BmpBuffer;
		int	r,g,b;
		unsigned short *wb=(void *)&BmpBuffer2[i][0];
		int a=ini_data.WallPaper[i];
		rb+=14+40;
		wb+=(272-1)*480;
		for(y=0;y<272;y++){
			for(x=0;x<480;x++){
				b=*rb++;
				g=*rb++;
				r=*rb++;
				b=((b*a)+1)/101;
				g=((g*a)+1)/101;
				r=((r*a)+1)/101;
				*wb++=
					(((b>>3) & 0x1F)<<10)+
					(((g>>3) & 0x1F)<<5)+
					(((r>>3) & 0x1F)<<0);//+0x8000;
			}
			wb-=480*2;
		}
	}
}
int	bmpchk(const char *n)
{
	while(*n){
		if( n[0]=='.' &&
			(n[1]=='b'||n[1]=='B') &&
			(n[2]=='m'||n[2]=='M') &&
			(n[3]=='p'||n[3]=='P') &&
			!n[4] 
		)return 1;
		n++;
	}
	return 0;
}
int		BmpRead(int i,char *fname)
{
	int fd1,len;
	unsigned long	offset;
	BITMAPINFOHEADER	bm;
	int c;
	char *p;

	bmp_f[i]=0;
	
	if( (p=zipchk(fname)) ){
		zip_read_buff = BmpBuffer;
 		if( Unzip_execExtract(p, 1) == UZEXR_OK ){
		}else return 0;
	}else
	if( bmpchk(fname) ){
		fd1 = sceIoOpen( fname, O_RDONLY, 0777);
		len = sceIoRead(fd1, BmpBuffer, BmpBuffer_Max);
		sceIoClose(fd1);
	}else
		return 0;

	if( BmpBuffer[0]!='B' || BmpBuffer[1]!='M' )return 0;

//	if( len!=BmpBuffer_Max ){Error_mes("�t�@�C���T�C�Y��");return 0;}

	{
		int i=40;
		char *p=(char *)&bm,*g=(char *)&BmpBuffer[14];
		while(i--)*p++=*g++;
	}

	if(bm.biBitCount!=24){Error_mes("24�r�b�g�ȊO�Ή����Ă˂���I");return 0;}
	if(bm.biWidth !=480){Error_mes("���̃T�C�Y480�ȊO�Ή����Ă˂���I�I");return 0;}
	if(bm.biHeight!=272){Error_mes("�c�̃T�C�Y272�ȊO�Ή����Ă˂���I�I");return 0;}

	bmp_i=i;
	bmp_f[i]=1;
	BmpAkarusa(i);
	return 1;
}


int	file_type=0;
int	kakutyousichk(int z,char *n)
{
	char *p;
	p=n;
	while(*p){
		if( *p++=='.' )break;
	}
	if( !*p )return 1;
	p=n;
	while(*p){
		if( z )
		if(	p[0]=='.' &&
			( p[1]=='z' || p[1]=='Z' ) &&
			( p[2]=='i' || p[2]=='I' ) &&
			( p[3]=='p' || p[3]=='P' ) &&
			!p[4]
		)return 1;
		switch(file_type){
		case	0:
			if(	p[0]=='.' &&
				( p[1]=='r' || p[1]=='R' ) &&
				( p[2]=='o' || p[2]=='O' ) &&
				( p[3]=='m' || p[3]=='M' ) &&
				!p[4]
			)return 1;
			if( n[0]=='.' &&
				(n[1]=='b'||n[1]=='B') &&
				(n[2]=='i'||n[2]=='I') &&
				(n[3]=='n'||n[3]=='N') &&
				!n[4] 
			)return 1;
			break;
		case	1:
			if(	p[0]=='.' &&
				( p[1]=='d' || p[1]=='D' ) &&
				( p[2]=='s' || p[2]=='S' ) &&
				( p[3]=='k' || p[3]=='K' ) &&
				!p[4]
			)return 1;
			break;
		case	2:
			if(	p[0]=='.' &&
				( p[1]=='b' || p[1]=='B' ) &&
				( p[2]=='m' || p[2]=='M' ) &&
				( p[3]=='p' || p[3]=='P' ) &&
				!p[4]
			)return 1;
			break;
		}
		p++;
	}
	return 0;
}





void	Bmp_put(int i)
{

	unsigned long *vr;
	unsigned long *g=(void *)&BmpBuffer2[i][0];
	int x,y;

	if( !bmp_f[i] ){
		int a=ini_data.WallPaper[i]/4;
		pgFillvram(a+(a<<5)+(a<<10));
		return;
	}

	vr = (void *)pgGetVramAddr(0,0);
	for(y=0;y<272;y++){
		for(x=0;x<480/2;x++){
			*vr++=*g++;
		}
		vr+=(LINESIZE-480)/2;
	}

}




char	ini_filename[]="config.ini";
void	ini_read(void)
{
	char path_ini[MAXPATH];
	int fd;
	bmp_f[0]=bmp_f[1]=bmp_f[1]=0;
	_strcpy( path_ini, path_main);
	_strcat( path_ini, ini_filename);
	fd = sceIoOpen(path_ini,O_RDONLY, 0777);
	if(fd>=0){
		sceIoRead(fd, &ini_data, sizeof(ini_info));
		sceIoClose(fd);
		if( ini_data.ver == 50 ){
			BmpRead(0,ini_data.path_WallPaper[0]);
			BmpRead(1,ini_data.path_WallPaper[1]);
			BmpRead(2,ini_data.path_WallPaper[2]);
			return;
		}
		Error_mes("�R���t�B�O�t�@�C���̃o�[�W��������v���Ȃ��̂ŏ��������܂�");
	}
	
	ini_data.ver = 50;
	ini_data.debug = 0;
	_strcpy( ini_data.path, path_main);
	ini_data.msx_type = 2;
	ini_data.path_rom[0][0]=0;
	ini_data.path_rom[1][0]=0;
	ini_data.path_rom[2][0]=0;
	ini_data.mapper[0]=0;
	ini_data.mapper[1]=0;
	ini_data.mapper[2]=0;
	ini_data.path_disk[0][0]=0;
	ini_data.path_disk[1][0]=0;

	ini_data.WallPaper[0]=20;
	ini_data.WallPaper[1]=20;
	ini_data.WallPaper[2]=90;
	ini_data.path_WallPaper[0][0]=0;
	ini_data.path_WallPaper[1][0]=0;
	ini_data.path_WallPaper[2][0]=0;
	ini_data.sound=1;
	ini_data.autosave=0;
	ini_data.videomode=0;
	ini_data.screen7=0;
	ini_data.kana=0;
	ini_data.CpuClock=0;
	ini_data.CpuClock_game=3;
	ini_data.CpuClock_menu=3;

	ini_data.key[ 0] = CTRL_CIRCLE;	//	I
	ini_data.key[ 1] = CTRL_CROSS;	//	II
	ini_data.key[ 2] = CTRL_TRIANGLE;//	RI
	ini_data.key[ 3] = CTRL_SQUARE;	//	RII
	ini_data.key[ 4] = CTRL_LTRIGGER;//	MENU
	ini_data.key[ 5] = CTRL_RTRIGGER;//	KEYBOARD
	ini_data.key[ 6] = 0;			//	quick load
	ini_data.key[ 7] = 0;			//	quick save
	ini_data.key[ 8] = 0;			//	file load
	ini_data.key[ 9] = 0;			//	file save
	ini_data.key[10] = 0;			//	video mode
	ini_data.key[11] = 0;			//	TURBO
	{
		int i;
		for(i=12;i<100;i++)
			ini_data.key[i] = 0;
	}
}
void	ini_write(void)
{
	char path_ini[MAXPATH];
	int fd;

	mh_strncpy( ini_data.path, now_path, MAXPATH);

	_strcpy( path_ini, path_main);
	_strcat( path_ini, ini_filename);
	fd = sceIoOpen( path_ini, O_CREAT|O_WRONLY|O_TRUNC, 0777);
	if(fd>=0){
		sceIoWrite(fd, &ini_data, sizeof(ini_info));
		sceIoClose(fd);
	}
}




void	menu_file_Get_DirList(char *path) {
	int ret,fd;
	dlist_num = 0;
	char *p;

	if( now_depth>0 ){
		_strcpy(dlist[dlist_num].name,"..");
		dlist[dlist_num].type = TYPE_DIR;
		dlist_num++;
	}

	if( (p=zipchk( path )) ){
 		if( Unzip_execExtract( p, 2) == UZEXR_OK ){
		};
	}else{
		// Directory read
		fd = sceIoDopen(path);
		ret = 1;
		while((ret>0) && (dlist_num<MAXDIRNUM)) {
			_memset(&dlist[dlist_num],0,sizeof(dirent_t));
			ret = sceIoDread(fd, &dlist[dlist_num]);
			if (dlist[dlist_num].name[0] == '.') continue;
			if (!kakutyousichk(1,dlist[dlist_num].name)) continue;
			if (ret>0) dlist_num++;
		}
		sceIoDclose(fd);
	}
	// �f�B���N�g�����X�g�G���[�`�F�b�N
	if (dlist_start  >= dlist_num) { dlist_start  = dlist_num-1; }
	if (dlist_start  <  0)         { dlist_start  = 0;           }
	if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
	if (dlist_curpos <  0)         { dlist_curpos = 0;           }
}

void menu_file_Draw(void) 
{
	int			i,col;

	Bmp_put(0);
	text_print(480-(sizeof(title)*5), 0,title,rgb2col( 55,255,255),0,0);

	// ���ݒn
	mh_strncpy(path_tmp,now_path,40);
	text_print(0,0,path_tmp,rgb2col(255,255,255),0,0);
	text_print(480-(sizeof(title)*5), 0,title,rgb2col( 55,255,255),0,0);

	text_print(0,262,"���F����@�~�F�L�����Z���@���F�t�H���_�߂�",rgb2col( 255,255,255),0,0);

	// �f�B���N�g�����X�g
	i = dlist_start;
	while (i<(dlist_start+PATHLIST_H)) {
		if (i<dlist_num) {
			col = rgb2col(255,255,255);
			if (dlist[i].type & TYPE_DIR) {
				col = rgb2col(255,255,0);
			}
			if (i==dlist_curpos) {
				col = rgb2col(255,0,0);
			}
			mh_strncpy(path_tmp,dlist[i].name,40);
			text_print(32,((i-dlist_start)+2)*10,path_tmp,col,0,0);
		}
		i++;
	}
	pgScreenFlipV();
}

int menu_file_Control(void) {
	unsigned long key;
	int i;

	// wait key
	while(1) {
		key = Read_Key();
		if (key != 0) break;
		pgWaitV();
	}

	if (key & CTRL_UP) {
		if (dlist_curpos > 0) {
			dlist_curpos--;
			if (dlist_curpos < dlist_start) { dlist_start = dlist_curpos; }
		}
	}
	if (key & CTRL_DOWN) {
		if (dlist_curpos < (dlist_num-1)) {
			dlist_curpos++;
			if (dlist_curpos >= (dlist_start+PATHLIST_H)) { dlist_start++; }
		}
	}
	if (key & CTRL_LEFT) {
		dlist_curpos -= PATHLIST_H;
		if (dlist_curpos <  0)          { dlist_curpos = 0;           }
		if (dlist_curpos < dlist_start) { dlist_start = dlist_curpos; }
	}
	if (key & CTRL_RIGHT) {
		dlist_curpos += PATHLIST_H;
		if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
		while (dlist_curpos >= (dlist_start+PATHLIST_H)) { dlist_start++; }
	}

	if (key & CTRL_CIRCLE) {
		if( dlist[dlist_curpos].name[0]=='.' ){
			goto pathmove;
		}
		if( zipchk(now_path) ){
			mh_strncpy(target,now_path,MAXPATH);
			mh_strncat(target,dlist[dlist_curpos].name,MAXPATH);
			return	2;
		}
		if (dlist[dlist_curpos].type & TYPE_DIR) {
			if (now_depth<MAXDEPTH) {
				// path�ړ�
				mh_strncat(now_path,dlist[dlist_curpos].name,MAXPATH);
				mh_strncat(now_path,"/",MAXPATH);
				cbuf_start[now_depth] = dlist_start;
				cbuf_curpos[now_depth] = dlist_curpos;
				dlist_start  = 0;
				dlist_curpos = 1;
				now_depth++;
				return 1;
			}
		}else
		if( zipchk(dlist[dlist_curpos].name) ){
			if (now_depth<MAXDEPTH) {
				mh_strncat(now_path,dlist[dlist_curpos].name,MAXPATH);
				mh_strncat(now_path,"/",MAXPATH);
				cbuf_start[now_depth] = dlist_start;
				cbuf_curpos[now_depth] = dlist_curpos;
				dlist_start  = 0;
				dlist_curpos = 1;
				now_depth++;
				return 1;
			}
		}else{
			mh_strncpy(target,now_path,MAXPATH);
			mh_strncat(target,dlist[dlist_curpos].name,MAXPATH);
			return 2;
		}
	}
	if (key & CTRL_TRIANGLE){
		pathmove:
		if (now_depth > 0) {
			// path�ړ�
			for(i=0;i<MAXPATH;i++) {
				if (now_path[i]==0) break;
			}
			i--;
			while(i>4) {
				if (now_path[i-1]=='/') {
					now_path[i]=0;
					break;
				}
				i--;
			}
			now_depth--;
			dlist_start  = cbuf_start[now_depth];
			dlist_curpos = cbuf_curpos[now_depth];
			return 1;
		}
	}
	if (key & CTRL_CROSS) return 3;
	
	return 0;
}
int		menu_file(void)
{
	menu_file_Get_DirList(now_path);
	while(1){
		menu_file_Draw();
	//	if( control_bef_ctl&CTRL_TRIANGLE )return 0;
		switch(menu_file_Control()) {
		case 1:
			menu_file_Get_DirList(now_path);
			break;
		case 2:
			return 1;
			break;
		case 3:
			return 0;
		}
	}
	return 0;
}


int	menu_key_cur,menu_key_s;
#define	menu_key_max	85
int	menu_key_set=0;
void menu_key_Draw(void) 
{
	int			i,col;

	Bmp_put(0);
	text_print(480-(sizeof(title)*5), 0,title,rgb2col( 55,255,255),0,0);

	{
		int i,y;
		static char *t[menu_key_max]={
			"EXIT",
			"joy �h",
			"joy II",
			"joy �hrapid",
			"joy IIrapid",
			"MENU",
			"KEYBOARD",
			"QuickLord",
			"QuickSave",
			"FileLord",
			"FileSave",
			"VideoMode",
			"TURBO",

			"key SPACE",
			"key RETURN",
			"key ��",
			"key ��",
			"key ��",
			"key ��",
			"key ESC",
			"key F1",
			"key F2",
			"key F3",
			"key F4",
			"key F5",
			"key STOP",
			"key CLS",
			"key SELECT",
			"key INS",
			"key DEL",
			"key BS",
			"key 1",
			"key 2",
			"key 3",
			"key 4",
			"key 5",
			"key 6",
			"key 7",
			"key 8",
			"key 9",
			"key 0",
			"key A",
			"key B",
			"key C",
			"key D",
			"key E",
			"key F",
			"key G",
			"key H",
			"key I",
			"key J",
			"key K",
			"key L",
			"key M",
			"key N",
			"key O",
			"key P",
			"key Q",
			"key R",
			"key S",
			"key T",
			"key U",
			"key V",
			"key W",
			"key X",
			"key Y",
			"key Z",
			"key TAB",
			"key CTRL",
			"key SHIFT",
			"key CAPS",
			"key KANA",
			"key GRAPH",
			"key -",
			"key ^",
			"key \\",
			"key @",
			"key [",
			"key ]",
			"key �G",
			"key �F",
			"key �C",
			"key �D",
			"key �^",
			"key �_",
		};
		if( menu_key_set ){
			text_print( 10, 10+(menu_key_cur-menu_key_s)*10,"���ԓ��ɐݒ肵��",rgb2col( 255,55,55),0,0);
			text_counter(400,10+(menu_key_cur-menu_key_s)*10, menu_key_set, 4, -1);
		}else{
			text_print( 60, 10+(menu_key_cur-menu_key_s)*10,"(^��^)",rgb2col( 255,55,55),0,0);
		}
		i=menu_key_s;
		for(y=10;y<260;y+=10,i++){
			if( i>=0 && i<menu_key_max ){
				int x=200;
				text_print( 100, y, t[i],rgb2col( 255,255,255),0,0);
				if( i ){
					int ii=i-1;
					if( ini_data.key[ii]&CTRL_SQUARE   ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_TRIANGLE ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_CROSS    ){text_print( x, y,"�~",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_CIRCLE   ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_START    ){text_print( x, y,"ST",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_SELECT   ){text_print( x, y,"SE",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_LTRIGGER ){text_print( x, y,"�k",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_RTRIGGER ){text_print( x, y,"�q",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_LEFT     ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_RIGHT    ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_UP       ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
					if( ini_data.key[ii]&CTRL_DOWN     ){text_print( x, y,"��",rgb2col( 255,255,255),0,0);x+=12;}
				}
			}
		}
	}

	{
		static char *t[]={
			"�R���t�B�O�ɖ߂�",
			"�W���C�X�e�B�b�N�@�h�{�^��",
			"�W���C�X�e�B�b�N�@�h�h�{�^��",
			"�W���C�X�e�B�b�N�@�h�{�^���A��",
			"�W���C�X�e�B�b�N�@�h�h�{�^���A��",
			"���j���[�Ăяo��",
			"�L�[�{�[�h�Ăяo��",
			"�������@�N�C�b�N���[�h�i�t�@�C������ǂ݂܂���j",
			"�������@�N�C�b�N�Z�[�u�i�t�@�C���ɏ������݂܂���j",
			"�������@�t�@�C�����[�h",
			"�������@�t�@�C���Z�[�u�i�N�C�b�N�T�[�u�����ꂿ�Ⴂ�܂��j",
			"��ʏo�̓��[�h",
			"�������t���[���X�L�b�v�B������邩��g��Ȃ��ق����ǂ�",
		};
		if( menu_key_cur<13 )
			text_print( 200, 262, t[menu_key_cur],rgb2col( 155,255,155),0,0);
		text_print( 0, 262, "���Ń{�^���ݒ�@�~�ŃN���A�@����EXIT",rgb2col( 155,255,155),0,0);
	}
	pgScreenFlipV();
}

int menu_key_Control(void) 
{
	unsigned long key;
	int i,x=0,y=0;

	// wait key
	while(1) {
		key = Read_Key();
		if (key != 0) break;
		pgWaitV();
	}
	if (key & CTRL_UP   ) y=-1;
	if (key & CTRL_DOWN ) y= 1;
	if (key & CTRL_LEFT ) x=-1;
	if (key & CTRL_RIGHT) x= 1;

	menu_key_cur += y;
	if( menu_key_cur <  0 )menu_key_cur = menu_key_max-1;
	if( menu_key_cur > menu_key_max-1 )menu_key_cur =  0;
	if( menu_key_cur<menu_key_s+1 )menu_key_s=menu_key_cur-1;
	if( menu_key_cur>menu_key_s+23 )menu_key_s=menu_key_cur-23;

	if( key & CTRL_TRIANGLE )return 1;
	if( key==ini_data.key[4] )return 1;
	if( !menu_key_cur ){
		if (key & CTRL_CIRCLE) {
			if( ini_data.key[4] ){
				return 1;
			}else{
				Error_mes("���j���[�Ăяo���ɉ������蓖�Ă�");
			}
		}
	}else{
		if( key & CTRL_CROSS ){
			ini_data.key[menu_key_cur-1]=0;
		}
		if( key & CTRL_CIRCLE ){
			menu_key_set=60;
			while(--menu_key_set){
				if( (key = Read_Key()) )menu_key_set=60;
				menu_key_Draw();
				ini_data.key[menu_key_cur-1] ^= key;//&(0xf309+CTRL_LEFT+CTRL_RIGHT);
			}
		}
	}
	return	0;
}
int		menu_key(void)
{
	menu_key_cur=0;
	menu_key_s=0;
	while(1){
		menu_key_Draw();
		switch(menu_key_Control()) {
		case 1:
			return 1;
		}
	}
	return 0;
}





int	menu_page=0;
int	ini_bmp;
int	menu_config_cur=0;
void	menu_config_Draw(void)
{
	char t[MAXPATH];
	static int	cy[]={
		20,
		50,
		60,
		70,
		90,
		110,
		120,
		130,
		150,
		160,
		170,
		180,
		200,
		210,
		230,
		250,
	};
	static char	*videomode[]={
		"normal 272x228",
		"4:3    360x272",
		"4:3    360x272 shading",
		"wide   480x272",
		"wide   480x272 shading",
	};
	static char	*screen7[]={
		"272x228",
		"512x212",
	};
	static char	*kana[]={
		"�T�O���z��",
		"�i�h�r�z��",
	};
	static char	*mapper[]={
		"0 Generic 8kB",
		"1 Generic 16kB (MSXDOS2)",
		"2 Konami5 8kB",
		"3 Konami4 8kB",
		"4 ASCII 8kB",
		"5 ASCII 16kB",
		"6 GameMaster2",
		"7 FMPAC",
	};
	static char	*onoff[]={
		"OFF",
		"ON",
	};
	static char	*CpuClock[]={
		"133",
		"166",
		"190",
		"222",
		"266",
		"333",
	};

	if( menu_config_cur==1 ){
		Bmp_put(1);
		if( frame_count )
			PutImage2();
	}else
		Bmp_put(ini_bmp);

	text_print(480-(sizeof(title)*5), 0,title,rgb2col( 55,255,255),0,0);

	text_print(  30, cy[menu_config_cur],"(^��^)",rgb2col( 255,55,55),0,0);

	text_print(  70, cy[ 0],"MAIN MENU",rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 1],"VIDEO MODE",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[ 1], videomode[ini_data.videomode],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 2],"SCREEN 7",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[ 2], screen7[ini_data.screen7],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 3],"KANA",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[ 3], kana[ini_data.kana],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 4],"KEY CONFIG",rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 5],"WallPaper menu",rgb2col( 255,255,255),0,0);
	text_counter(190,cy[ 5], ini_data.WallPaper[0], 3, -1);
	text_print( 200, cy[ 5], ini_data.path_WallPaper[0],rgb2col( 255,255,155),0,0);
	text_print(  70, cy[ 6],"WallPaper game",rgb2col( 255,255,255),0,0);
	text_counter(190,cy[ 6], ini_data.WallPaper[1], 3, -1);
	text_print( 200, cy[ 6], ini_data.path_WallPaper[1],rgb2col( 255,255,155),0,0);
	text_print(  70, cy[ 7],"WallPaper keyboard",rgb2col( 255,255,255),0,0);
	text_counter(190,cy[ 7], ini_data.WallPaper[2 ], 3, -1);
	text_print( 200, cy[ 7], ini_data.path_WallPaper[2],rgb2col( 255,255,155),0,0);
	text_print(  70, cy[ 8],"DISK AUTO SAVE",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[ 8], onoff[ini_data.autosave&1],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 9],"STATE AUTO SAVE&LOAD",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[ 9], onoff[(ini_data.autosave&2)>>1],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[10],"CMOS AUTO SAVE",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[10], onoff[(ini_data.autosave&4)>>2],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[11],"SRAM AUTO SAVE&LOAD",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[11], onoff[(ini_data.autosave&8)>>3],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[12],"CpuClock game",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[12], CpuClock[ini_data.CpuClock_game],rgb2col( 255,255,255),0,0);
//	text_counter(190,cy[12], ini_data.CpuClock_game, 3, -1);
	text_print(  70, cy[13],"CpuClock menu",rgb2col( 255,255,255),0,0);
	text_print( 200, cy[13], CpuClock[ini_data.CpuClock_menu],rgb2col( 255,255,255),0,0);
	if( frame_count )
	text_print(  70, cy[14],"continue",rgb2col( 255,255,255),0,0);
	else
	text_print(  70, cy[14],"continue",rgb2col( 155,155,155),0,0);
	text_print(  70, cy[15],"DEBUG",rgb2col( 255,255,255),0,0);
	text_counter(200,cy[15], ini_data.debug, 3, -1);


	{
		static char *t[]={
			"���C�����j���[�ɖ߂�܂�",
			"��ʃ��[�h",
			"�X�N���[���V�̕`����@ 512�̕��͂Ƃ��Ă��o�M�[",
			"�J�i�̔z��",
			"�{�^���ݒ�",
			"���ŕǎ��t�@�C���I���B480x272x24��BMP�BZIP�Ή��B���E�Ŗ��邳����",
			"���ŕǎ��t�@�C���I���B480x272x24��BMP�BZIP�Ή��B���E�Ŗ��邳����",
			"���ŕǎ��t�@�C���I���B480x272x24��BMP�BZIP�Ή��B���E�Ŗ��邳����",
			"�f�B�X�N�I�[�g�Z�[�u�@���F���k�t�@�C���̓Z�[�u����܂���",
			"��ԃI�[�g�Z�[�u�����[�h�@�����AROM��p�B�T�C�Y�傫�����Ƃ��Ă��댯",
			"�����Ɠ����̂��s�� CMOS�� BIOS/CMOS.ROM ��",
			"�����Ɠ����̂��s�� SAVE�t�H���_�ɍ����C��",
			"CPU�̑��x�@�Q�[�����@�f�t�H���g222�@PSP������\��������܂��B���ȐӔC��",
			"CPU�̑��x�@���j���[���@�f�t�H���g222�@PSP������\��������܂��B���ȐӔC��",
			"�ĊJ",
			"�ЁE�݁E���c",
		};
		text_print( 40, 260, t[menu_config_cur],rgb2col( 155,255,155),0,0);
	}
	pgScreenFlipV();
}
int	disk_f[2];
int		menu_config_Control(void)
{
	unsigned long key;
	int x=0,y=0;

	while(1) {
		key = Read_Key();
		if (key != 0) break;
		pgWaitV();
	}
	if (key & CTRL_UP   ) y=-1;
	if (key & CTRL_DOWN ) y= 1;
	if (key & CTRL_LEFT ) x=-1;
	if (key & CTRL_RIGHT) x= 1;
	menu_config_cur += y;
	if( menu_config_cur <  0 )menu_config_cur = 15;
	if( menu_config_cur > 15 )menu_config_cur =  0;

	ini_bmp=0;

	if( key==ini_data.key[4] ){
		goto goto_continue;
	}

	switch( menu_config_cur ){
	case	0:
		if (key & CTRL_CIRCLE)
			return	2;
		break;
	case	1:	//	video mode
		ini_data.videomode+=x;
		if( ini_data.videomode <  0 )ini_data.videomode = 4;
		if( ini_data.videomode >  4 )ini_data.videomode = 0;
		break;
	case	2:	//	screen7
		if( x )ini_data.screen7^=1;
		break;
	case	3:	//	kana
		if( x )ini_data.kana^=1;
		break;
	case	4:	//	keyc onfig
		if (key & CTRL_CIRCLE)
			menu_key();
		break;
	case	5:	//	wallp
	case	6:	//	wallp
	case	7:	//	wallp
		{
			int	i=menu_config_cur-5;
			file_type=2;
			if (key & CTRL_CIRCLE){
				if( menu_file() ){
					if( BmpRead( i, target) ){
						_memcpy( ini_data.path_WallPaper[i], target, MAXPATH);
					}
				}
			}else
			if (key & CTRL_CROSS){
				ini_data.path_WallPaper[i][0]=0;
				bmp_f[i]=0;
			}else
			if( x ){
				ini_data.WallPaper[i]+=x;
				if( ini_data.WallPaper[i] <   0 )ini_data.WallPaper[i]=  0;
				if( ini_data.WallPaper[i] > 100 )ini_data.WallPaper[i]=100;
				if( bmp_i != i )BmpRead( i, ini_data.path_WallPaper[i]);
				bmp_i=i;
				BmpAkarusa(i);
			}
			ini_bmp=i;
		}
		break;
	case	8:	//	autosave
		if( x )ini_data.autosave^=1;
		break;
	case	9:	//	state autosave
		if( x )ini_data.autosave^=2;
		break;
	case	10:	//	cmos autosave
		if( x )ini_data.autosave^=4;
		break;
	case	11:	//	sram autosave
		if( x )ini_data.autosave^=8;
		break;

	case	12:	//	CpuClock game
		ini_data.CpuClock_game+=x;
		if( ini_data.CpuClock_game <  0 )ini_data.CpuClock_game = 5;
		if( ini_data.CpuClock_game >  5 )ini_data.CpuClock_game = 0;
		break;
	case	13:	//	CpuClock menu
		ini_data.CpuClock_menu+=x;
		if( ini_data.CpuClock_menu <  0 )ini_data.CpuClock_menu = 5;
		if( ini_data.CpuClock_menu >  5 )ini_data.CpuClock_menu = 0;
		break;

	case	14:	//	continue
		if (key & CTRL_CIRCLE){
	goto_continue:;
			if( frame_count ){
				return 3;
			}
		}
		break;
	case	15:	//	debug
		ini_data.debug+=x;
		if( ini_data.debug <  0 )ini_data.debug = 1;
		if( ini_data.debug >  1 )ini_data.debug = 0;
		break;
	}
	return	0;
}

int		menu_config(void)
{
//	disk_f[0]=disk_f[1]=0;
	wavoutStopPlay1();
//	menu_config_cur=0;
	while(1){
		menu_config_Draw();
		switch(menu_config_Control()) {
		case 1:
			break;
		case 2:
			menu_page=0;
			return 0;
		case 3:
			menu_page=1;
			return 3;
		}
	}
	return 0;
}


int	menu_main_cur=7;
void	menu_main_Draw(void)
{
	char t[MAXPATH];
	int	c;

	static int	cy[]={
		20,
		50,
		70,
		80,
		100,
		110,
		130,
		150,
		170,
		180,
		190,
		200,
		230,
	};
	static char	*msxtype[]={
		"MSX1",
		"MSX2",
		"MSX2+",
	//	"Matsushita's Panasonic FS-A1F (MSX2) (Made in 1987)",
	//	"Daewoo IQ-2000 CPC-400 (MSX2) (Made in 1987)",
	//	"Matsushita's Panasonic FS-A1WSX (MSX2 Plus) (Made in 1989)",
	};
	static char	*mapper[]={
		"0 Generic 8kB",
		"1 Generic 16kB (MSXDOS2)",
		"2 Konami5 8kB",
		"3 Konami4 8kB",
		"4 ASCII 8kB",
		"5 ASCII 16kB",
		"6 GameMaster2",
		"7 FMPAC",
	};

	Bmp_put(0);

	text_print(480-(sizeof(title)*5), 0,title,rgb2col( 55,255,255),0,0);

	text_print(  30, cy[menu_main_cur],"(^��^)",rgb2col( 255,55,55),0,0);

	text_print(  70, cy[ 0],"CONFIG",rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 1],"MSX TYPE",rgb2col( 255,255,255),0,0);
	text_print( 140, cy[ 1], msxtype[ini_data.msx_type],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 2],"ROM 1",rgb2col( 255,255,255),0,0);
	text_print( 110, cy[ 2], ini_data.path_rom[0],rgb2col( 255,255,155),0,0);
	text_print( 350, cy[ 2], mapper[ini_data.mapper[0]],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 3],"ROM 2",rgb2col( 255,255,255),0,0);
	text_print( 110, cy[ 3], ini_data.path_rom[1],rgb2col( 255,255,155),0,0);
	text_print( 350, cy[ 3], mapper[ini_data.mapper[1]],rgb2col( 255,255,255),0,0);
	text_print(  70, cy[ 4],"DRIVE A",rgb2col( 255,255,255),0,0);
	text_print( 110, cy[ 4], ini_data.path_disk[0],rgb2col( 255,255,155),0,0);
	text_print(  70, cy[ 5],"DRIVE B",rgb2col( 255,255,255),0,0);
	text_print( 110, cy[ 5], ini_data.path_disk[1],rgb2col( 255,255,155),0,0);
	if( frame_count )c=rgb2col( 255,255,255);else c=rgb2col( 155,155,155);
	text_print(  70, cy[ 6],"continue",c,0,0);
	text_print(  70, cy[ 7],"POWER",rgb2col( 255,255,255),0,0);
	if( frame_count && State_f )c=rgb2col( 255,255,255);else c=rgb2col( 155,155,155);
	text_print(  70, cy[ 8],"state quick load",c,0,0);
	if( frame_count )c=rgb2col( 255,255,255);else c=rgb2col( 155,155,155);
	text_print(  70, cy[ 9],"state quick save",c,0,0);
	if( StateName )c=rgb2col( 255,255,255);else c=rgb2col( 155,155,155);
	text_print(  70, cy[10],"state file load",c,0,0);
	text_print(  70, cy[11],"state file save",c,0,0);
	text_print(  70, cy[12],"EXIT",rgb2col( 255,255,255),0,0);

	{
		static char *t[]={
			"�F�X�ݒ�",
			"MSX�̋@��ʂ�BIOS�t�@�C�����K�v�ł��B",
			"ROM�J�Z�b�g�P�@���Ńt�@�C���Z���N�g(ZIP�Ή�)�@�~�Ŏ��O���@���E�Ń}�b�p�[�I��",
			"ROM�J�Z�b�g�Q�@���Ńt�@�C���Z���N�g(ZIP�Ή�)�@�~�Ŏ��O���@���E�Ń}�b�p�[�I��",
			"�h���C�u�`�@���Ńt�@�C���Z���N�g(ZIP�Ή�)�@�~�Ŏ��O��",
			"�h���C�u�a�@���Ńt�@�C���Z���N�g(ZIP�Ή�)�@�~�Ŏ��O��",
			"�ĊJ",
			"�l�r�w���s",
			"��ԓǂݍ��݁@�댯",
			"��ԏ������݁@�댯",
			"��ԃt�@�C���ǂݍ��݁@ROM��p�@�댯�@",
			"��ԃt�@�C���������݁@ROM��p�@�댯�@�T�C�Y�ł��[��fMSX",
			"�I��",
		};
		text_print( 40, 260, t[menu_main_cur],rgb2col( 155,255,155),0,0);
	}
	pgScreenFlipV();
}
int	disk_f[2];
int		menu_main_Control(void)
{
	unsigned long key;
	int x=0,y=0;

	while(1) {
		key = Read_Key();
		if (key != 0) break;
		pgWaitV();
	}
	if (key & CTRL_UP   ) y=-1;
	if (key & CTRL_DOWN ) y= 1;
	if (key & CTRL_LEFT ) x=-1;
	if (key & CTRL_RIGHT) x= 1;
	menu_main_cur += y;
	if( menu_main_cur <  0 )menu_main_cur = 12;
	if( menu_main_cur > 12 )menu_main_cur =  0;

	ini_bmp=0;

	if( key==ini_data.key[4] ){
		goto goto_continue;
	}

	switch( menu_main_cur ){
	case	0:	//	
		if (key & CTRL_CIRCLE){
			int r;
			if( (r=menu_config()) )return r;
		}
		break;
	case	1:
		ini_data.msx_type+=x;
	//	if( !ini_data.debug ){
		if(1){
			if( ini_data.msx_type <  0 )ini_data.msx_type = 2;
			if( ini_data.msx_type >  2 )ini_data.msx_type = 0;
		}else{
			if( ini_data.msx_type <  0 )ini_data.msx_type = 5;
			if( ini_data.msx_type >  5 )ini_data.msx_type = 0;
		}
		break;
	case	2:
	case	3:
		{
			int i=menu_main_cur-2;
			file_type=0;
			if (key & CTRL_CIRCLE)
			if( menu_file() ){
				_memcpy( ini_data.path_rom[i], target, MAXPATH);
			}
			if (key & CTRL_CROSS)ini_data.path_rom[i][0]=0;
			ini_data.mapper[i]+=x;
			if( ini_data.mapper[i] <  0 )ini_data.mapper[i] = 7;
			if( ini_data.mapper[i] >  7 )ini_data.mapper[i] = 0;
		}
		break;
	case	4:
	case	5:
		{
			int i=menu_main_cur-4;
			file_type=1;
			if (key & CTRL_CIRCLE){
				if( DiskAutoSave_flag[i] ){
					if( (ini_data.autosave&1) ){
						DiskAutoSave_s(i);
					}else{
						Bmp_put(0);
						text_print(100,110,"�f�B�X�N�������������Ă��܂����ǁA�ۑ����Ă����܂����H",rgb2col(255,2,2),0,0);
						text_print(150,130,"���F�ۑ�����@�w�F�ۑ����Ȃ�",rgb2col(255,2,2),0,0);
						pgScreenFlipV();
						while(1) {
							int key = Read_Key();
							if (key == 0) break;
							pgWaitV();
						}
						while(1) {
							int key = Read_Key();
							if (key & CTRL_CROSS )break;
							if (key & CTRL_CIRCLE){
								Bmp_put(0);
								text_print(200,110,"�ۑ���",rgb2col(255,2,2),0,0);
								pgScreenFlipV();
								switch(	DiskAutoSave_s(i) ){
								case	1:	Error_mes("�t�@�C����������");	break;
								case	2:	Error_mes("���k�t�@�C���̒��Ȃ̂Ŗ���");	break;
								}
								break;
							}
							pgWaitV();
						}
					}
				}
				if( menu_file() ){
					_memcpy( ini_data.path_disk[i], target, MAXPATH);
					disk_f[i]=1;
				}
			}else
			if (key & CTRL_CROSS){
				ini_data.path_disk[i][0]=0;
				disk_f[i]=1;
			}
		//	else
		//	if (key & CTRL_SQUARE){
		//		DiskAutoSave_s(i);
		//	}

		}
		break;

	case	6:	//	continue
		if (key & CTRL_CIRCLE){
	goto_continue:;
			if( frame_count ){
				return 3;
			}
		}
		break;
	case	7:	//	power
		if( !(key & CTRL_CIRCLE) )break;

		game_WallPaper_c=2;
		
		switch( ini_data.msx_type ){
		case	0:	
			MSXVersion = 0;	break;
		case	1:	
		case	3:	
		case	4:	
			MSXVersion = 1;	break;
		case	2:	
		case	5:
			MSXVersion = 2;	break;
		}
		MSXType = ini_data.msx_type;
		ROMName[0]=&ini_data.path_rom[0][0];
		ROMName[1]=&ini_data.path_rom[1][0];
		ROMName[2]=&ini_data.path_rom[2][0];
		ROMType[0]=ini_data.mapper[0];
		ROMType[1]=ini_data.mapper[1];
		ROMType[2]=ini_data.mapper[2];
		DSKName[0]=&ini_data.path_disk[0][0];
		DSKName[1]=&ini_data.path_disk[1][0];

				Bmp_put(0);
				pgScreenFlipV();

		return	4;
		break;

	case	8:	//	q state load
		if( key & CTRL_CIRCLE ){
			if( !frame_count )break;
			if( !State_f )break;
			Bmp_put(0);
			text_print(100,130, "��ԓǂݍ��ݒ�",rgb2col(255,255,255),0,0);
			pgScreenFlipV();
			LoadStateQ();
		}
		break;
	case	9:	//	q state save
		if( key & CTRL_CIRCLE ){
			if( !frame_count )break;
			Bmp_put(0);
			text_print(100,130, "��ԏ������ݒ�",rgb2col(255,255,255),0,0);
			pgScreenFlipV();
			SaveStateQ();
		}
		break;
	case	10:	//	state load
		if( key & CTRL_CIRCLE ){
			if( !StateName )break;
			Bmp_put(0);
			text_print(100,130, "��ԓǂݍ��ݒ�",rgb2col(255,255,255),0,0);
			text_print(150,150, StateName,rgb2col(155,255,255),0,0);
			pgScreenFlipV();
			LoadState(StateName);
		}
		break;
	case	11:	//	state save
		if( key & CTRL_CIRCLE ){
			if( !StateName )break;
			Bmp_put(0);
			text_print(100,130, "��ԏ������ݒ�",rgb2col(255,255,255),0,0);
			text_print(150,150, StateName,rgb2col(155,255,255),0,0);
			pgScreenFlipV();
			SaveState(StateName);
		}
		break;
	case	12:
		if (key & CTRL_CIRCLE)
			return	2;
		break;
	}
	return	0;
}

int		menu_main(void)
{
	CpuClockSet(ini_data.CpuClock_menu);

	disk_f[0]=disk_f[1]=0;
	wavoutStopPlay1();
//	menu_main_cur=0;
	if( menu_page ){
		int r;
		if( (menu_config()==3) )goto menuexit;
	}
	while(1){
		menu_main_Draw();
		switch(menu_main_Control()) {
		case 1:
			break;
		case 2:
			pgFillvram(0);
			text_print(100,130,"�I�����܂���H�@���F�I���@�w�F����ς��",rgb2col(255,2,2),0,0);
			pgScreenFlipV();
			while(1) {
				int key = Read_Key();
				if (key == 0) break;
				pgWaitV();
			}
			pgWaitV();
			while(1) {
				int key = Read_Key();
				if (key & CTRL_CIRCLE){
					pgFillvram(rgb2col(5,100,5));
					text_print(300,260,"�`�@�I���������@�����l�ł����@�`",rgb2col(255,255,255),0,0);
					pgScreenFlipV();

	TrashMSX();
	DiskAutoSave();
	ini_write();

			TrashMachine();
				ExitNow=1;

				sceCtrlRead(&ctl,1);
				syorioti=ctl.frame-(16666*3);
				syorioti_c=0;

					return 1;
				}
				if (key & CTRL_CROSS ) break;
				pgWaitV();
			}
			break;
		case 3:
		menuexit:
			CpuClockSet(ini_data.CpuClock_game);

/*

				ROMName[0]=&ini_data.path_rom[0][0];
				ROMName[1]=&ini_data.path_rom[1][0];
				ROMName[2]=&ini_data.path_rom[2][0];
				ROMType[0]=ini_data.mapper[0];
				ROMType[1]=ini_data.mapper[1];
				ROMType[2]=ini_data.mapper[2];
				DSKName[0]=ini_data.path_disk[0];
				DSKName[1]=ini_data.path_disk[1];
				{
					int J;
					for(J=0;J<MAXCARTS;J++) LoadCart(ROMName[J],J);
				}
				if(ChangeDisk(0,DSKName[0]));
				if(ChangeDisk(1,DSKName[1]));
				RunZ80(&CPU);

				wavoutStopPlay1();
*/				

				Bmp_put(0);
				pgScreenFlipV();

				DiskAutoSave();

				DSKName[0]=ini_data.path_disk[0];
				DSKName[1]=ini_data.path_disk[1];
				if(disk_f[0])if(ChangeDisk(0,DSKName[0]));
				if(disk_f[1])if(ChangeDisk(1,DSKName[1]));

				sceCtrlRead(&ctl,1);
				syorioti=ctl.frame-(16666*3);
				syorioti_c=0;
				wavoutStartPlay1();
				ExitNow=0;
				game_WallPaper_c=2;

			return 0;
		case 4:
			CpuClockSet(ini_data.CpuClock_game);
				DiskAutoSave();
				ExitNow=2;
			return 1;
		}
	}
	return 0;
}

void	menu_init(void)
{


	ini_read();
	mh_strncpy( now_path, ini_data.path, MAXPATH);


	{
		char *m = now_path;
		now_depth=0;
		while(1){
			if(!*m)break;
			else
			if(*m++=='/'){
				now_depth++;
			}
		}
		if( now_depth )now_depth--;

	}

	//
	dlist_start  = 0;
	dlist_curpos = 0;


}


