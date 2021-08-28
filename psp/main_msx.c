

#include "psp.h"
#include "..\msx\Sound.h"



/** Public parameters ****************************************/
//char *Title="fMSX Unix/X 3.0";  /* Window/command line title */
//int SaveCPU   = 1;              /* 1 = freeze when focus out */
//int UseSHM    = 1;              /* 1 = use MITSHM            */
//int UseSound  = 44100;          /* Sound driver frequency    */
//int UseZoom   = 1;              /* Window zoom factor        */
//int UseStatic = 1;              /* 1 = use static palette    */
//int SyncFreq  = 0;              /* Screen update frequency   */
//char *Disks[2][MAXDISKS+1];     /* Disk names for each drive */

/** Various variables ****************************************/
#define WIDTH  272//272
#define HEIGHT 228

//static volatile char TimerReady;
static unsigned int BPal[256],XPal[80],XPal0; 
static char *XBuf;
//static Port Prt;
static char JoyState;
static char XKeyMap[16];
static int CurDisk[2];
static int FastForward;

/** Various X-related variables ******************************/
//static Display *Dsp;
//static Colormap DefaultCMap;
//static GC DefaultGC;
static unsigned long White,Black;

/** Sound-related definitions ********************************/
#ifdef SOUND
static int SndSwitch = (1<<MAXCHANNELS)-1;
static int SndVolume = 255/MAXCHANNELS;
#endif


int	softkeyboard_f=0;


int	frame_count=0;
void PutImage2(void)
{
	switch( ini_data.videomode ){
	case	0:
			{
				int x,y;
				int mx,my;
				unsigned int *vr;
				unsigned int *v = (void *)XBuf;
				mx=(480-272)/2;
				my=(272-228)/2;
				vr = (void *)pgGetVramAddr(mx,my);
				for(y=0;y<228;y++){
					for(x=0;x<272/2;x++){
						*vr++=*v++;
					}
					vr+=(LINESIZE-272)/2;
				}
			}
		break;
	case	1:
		if(	ini_data.screen7 && ScrMode==7 )
		{
			int x,y;
			unsigned int *vr;
			unsigned int *v = (void *)XBuf,*vb;
			unsigned	int g,s;
			int yy,yc;
			vr = (void *)pgGetVramAddr( 48, 0);
			yc=yy=(2720)/((272+0)-(212));
			for(y=0;y<272;y++){
				if((yc-=10)<=0){
					yc+=yy;v-=(384/2);
				}
				vb=v;
				for(x=0;x<384/2;x++){
					*vr++=*v++;
				}
				vr+=(LINESIZE-384)/2;
				v=vb+(384/2);
			}
		}else{
			int x,y;
			unsigned int *vr;
			unsigned short *v = (void *)XBuf,*vb;
			unsigned	int g,s;
			int yy,yc;
			int xx,xc;
			int mx,my;
			mx=60;
			my=0;
			vr = (void *)pgGetVramAddr(mx,my);
		//	v+=((WIDTH*(0+0))+((272-272)/2));
			yc=yy=(2720)/((272)-(228));
			xc=xx=3600/(360-(272));
			for(y=0;y<272;y++){
				if((yc-=10)<=0){
					yc+=yy;v-=(WIDTH);
				}
				vb=v;
				xc=xx;
				for(x=0;x<360/2;x++){
					if((xc-=10)<=0){
						xc+=xx;
						s=*v;
					}else s=(*v++);
					if((xc-=10)<=0){
						xc+=xx;
						s+=(s)<<16;
					}else s+=(*v++)<<16;
					
					*vr++=s;
				}
				vr+=(LINESIZE-360)/2;
				v=vb+WIDTH;
			}
		}
		break;
	case	2:
		if(	ini_data.screen7 && ScrMode==7 )
		{
			int x,y;
			unsigned int *vr;
			unsigned int *v = (void *)XBuf,*vb;
			unsigned	int g,s,g2;
			int yy,yc;
			int xx,xc;
			unsigned int b[480/2],*bp,bf;
			vr = (void *)pgGetVramAddr( 48, 0);
			yc=yy=(2720)/((272)-(212));
			for(y=0;y<272;y++){
				bp=b;
				if((yc-=10)<=0){
					yc+=yy;	bf=1;
				}else bf=0;
				vb=v;	xc=xx;	g=0;
				for(x=0;x<384/2;x++){
					s=*v++;
					if( bf ){
						*vr++=
							((((s&0x7c007c00)+(*bp&0x7c007c00))>> 1)&0x7c007c00)+
							((((s&0x001f001f)+(*bp&0x001f001f))>> 1)&0x001f001f)+
							((((s&0x03e003e0)+(*bp&0x03e003e0))>> 1)&0x03e003e0);
						bp++;
					}else{
						*vr++=*bp++=s;//palt[s];
					}
				}
				if( bf )vb-=(384/2);
				vr+=(LINESIZE-384)/2;
				v=vb+(384/2);
			}
		}else{
			int x,y;
			unsigned int *vr;
			unsigned short *v = (void *)XBuf,*vb;
			unsigned	int g,s,g2;
			int yy,yc;
			int xx,xc;
			int mx,my;
			unsigned int b[480/2],*bp,bf;
			mx=60;
			my=0;
			vr = (void *)pgGetVramAddr(mx,my);
		//	v+=((WIDTH*(0+0))+((272-272)/2));
			yc=yy=(2720)/((272+0)-(228));
			xc=xx=3600/(360-(272));
			for(y=0;y<272;y++){
				bp=b;
				if((yc-=10)<=0){
					yc+=yy;	bf=1;
				}else bf=0;
				vb=v;	xc=xx;	g=0;
				for(x=0;x<360/2;x++){
					if((xc-=10)<=0){
						xc+=xx;
						s=
							((((g&0x7c007c00)+(*v&0x7c007c00))>> 1)&0x7c007c00)+
							((((g&0x001f001f)+(*v&0x001f001f))>> 1)&0x001f001f)+
							((((g&0x03e003e0)+(*v&0x03e003e0))>> 1)&0x03e003e0);
					}else{
						g=(*v++);	s=g;
					}
					if((xc-=10)<=0){
						xc+=xx;
						s+=(
							((((g&0x7c007c00)+(*v&0x7c007c00))>> 1)&0x7c007c00)+
							((((g&0x001f001f)+(*v&0x001f001f))>> 1)&0x001f001f)+
							((((g&0x03e003e0)+(*v&0x03e003e0))>> 1)&0x03e003e0))<<16;
					}else{
						g=(*v++);	s+=g<<16;
					}
					if( bf ){
						*vr++=
							((((s&0x7c007c00)+(*bp&0x7c007c00))>> 1)&0x7c007c00)+
							((((s&0x001f001f)+(*bp&0x001f001f))>> 1)&0x001f001f)+
							((((s&0x03e003e0)+(*bp&0x03e003e0))>> 1)&0x03e003e0);
						bp++;
					}else{
						*vr++=*bp++=s;//palt[s];
					}
				}
				if( bf )vb-=(WIDTH);
				vr+=(LINESIZE-360)/2;
				v=vb+WIDTH;
			}
		}
		break;
	case	3:
		if(	ini_data.screen7 && ScrMode==7 )
		{
			int x,y;
			unsigned int *vr;
			unsigned int *v = (void *)XBuf,*vb;
			unsigned	int g,s;
			int yy,yc;
			vr = (void *)pgGetVramAddr( 0, 0);
			yc=yy=(2720)/((272+0)-(212));
			for(y=0;y<272;y++){
				if((yc-=10)<=0){
					yc+=yy;v-=(480/2);
				}
				vb=v;
				for(x=0;x<480/2;x++){
					*vr++=*v++;
				}
				vr+=(LINESIZE-480)/2;
				v=vb+(480/2);
			}
		}else{
			int x,y;
			unsigned int *vr;
			unsigned short *v = (void *)XBuf,*vb;
			unsigned	int g,s;
			int yy,yc;
			int xx,xc;
			int mx,my;
			mx=0;
			my=0;
			vr = (void *)pgGetVramAddr(mx,my);
			v+=(WIDTH*(0+20));
			yc=yy=(2720)/((272+36)-(228));
			xc=xx=4800/(480-(272));
			for(y=0;y<272;y++){
				if((yc-=10)<=0){
					yc+=yy;v-=(WIDTH);
				}
				vb=v;
				xc=xx;
				for(x=0;x<480/2;x++){
					if((xc-=10)<=0){
						xc+=xx;
						s=*v;
					}else s=(*v++);
					if((xc-=10)<=0){
						xc+=xx;
						s+=(s)<<16;
					}else s+=(*v++)<<16;
					
					*vr++=s;
				}
				vr+=(LINESIZE-480)/2;
				v=vb+WIDTH;
			}
		}
		break;
	case	4:
		if(	ini_data.screen7 && ScrMode==7 )
		{
			int x,y;
			unsigned int *vr;
			unsigned int *v = (void *)XBuf,*vb;
			unsigned	int g,s,g2;
			int yy,yc;
			int xx,xc;
			unsigned int b[480/2],*bp,bf;
			vr = (void *)pgGetVramAddr( 0, 0);
			yc=yy=(2720)/((272)-(212));
			for(y=0;y<272;y++){
				bp=b;
				if((yc-=10)<=0){
					yc+=yy;	bf=1;
				}else bf=0;
				vb=v;	xc=xx;	g=0;
				for(x=0;x<480/2;x++){
					s=*v++;
					if( bf ){
						*vr++=
							((((s&0x7c007c00)+(*bp&0x7c007c00))>> 1)&0x7c007c00)+
							((((s&0x001f001f)+(*bp&0x001f001f))>> 1)&0x001f001f)+
							((((s&0x03e003e0)+(*bp&0x03e003e0))>> 1)&0x03e003e0);
						bp++;
					}else{
						*vr++=*bp++=s;//palt[s];
					}
				}
				if( bf )vb-=(480/2);
				vr+=(LINESIZE-480)/2;
				v=vb+(480/2);
			}
		}else{
			int x,y;
			unsigned int *vr;
			unsigned short *v = (void *)XBuf,*vb;
			unsigned	int g,s,g2;
			int yy,yc;
			int xx,xc;
			int mx,my;
			unsigned int b[480/2],*bp,bf;
			mx=0;
			my=0;
			vr = (void *)pgGetVramAddr(mx,my);
			v+=(WIDTH*(0+20));
			yc=yy=(2720)/((272+36)-(228));
			xc=xx=4800/(480-(272));
			for(y=0;y<272;y++){
				bp=b;
				if((yc-=10)<=0){
					yc+=yy;	bf=1;
				}else bf=0;
				vb=v;	xc=xx;	g=0;
				for(x=0;x<480/2;x++){
					if((xc-=10)<=0){
						xc+=xx;
						s=
							((((g&0x7c007c00)+(*v&0x7c007c00))>> 1)&0x7c007c00)+
							((((g&0x001f001f)+(*v&0x001f001f))>> 1)&0x001f001f)+
							((((g&0x03e003e0)+(*v&0x03e003e0))>> 1)&0x03e003e0);
					}else{
						g=(*v++);	s=g;
					}
					if((xc-=10)<=0){
						xc+=xx;
						s+=(
							((((g&0x7c007c00)+(*v&0x7c007c00))>> 1)&0x7c007c00)+
							((((g&0x001f001f)+(*v&0x001f001f))>> 1)&0x001f001f)+
							((((g&0x03e003e0)+(*v&0x03e003e0))>> 1)&0x03e003e0))<<16;
					}else{
						g=(*v++);	s+=g<<16;
					}
					if( bf ){
						*vr++=
							((((s&0x7c007c00)+(*bp&0x7c007c00))>> 1)&0x7c007c00)+
							((((s&0x001f001f)+(*bp&0x001f001f))>> 1)&0x001f001f)+
							((((s&0x03e003e0)+(*bp&0x03e003e0))>> 1)&0x03e003e0);
						bp++;
					}else{
						*vr++=*bp++=s;//palt[s];
					}
				}
				if( bf )vb-=(WIDTH);
				vr+=(LINESIZE-480)/2;
				v=vb+WIDTH;
			}
		}
		break;
	}
}
int	game_WallPaper_c;
int	mouse_x=12*16*16/2,mouse_y=6*16*16/2;

int	key_push=0;
unsigned short Keys[] =
{
	0x0704,0x0620,0x0660,0x0680,0x0701,0x0702,0x0000,0x0710,0x0802,0x0740,0x0804,0x0808,0x0720,
	0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0101,0x0102,0x0001,0x0104,0x0108,0x0110,
	0x0440,0x0510,0x0304,0x0480,0x0502,0x0540,0x0504,0x0340,0x0410,0x0420,0x0120,0x0140,0x0780,
	0x0240,0x0501,0x0302,0x0308,0x0310,0x0320,0x0380,0x0401,0x0402,0x0180,0x0201,0x0202,0x0780,
	0x0580,0x0520,0x0301,0x0508,0x0280,0x0408,0x0404,0x0204,0x0208,0x0210,0x0220,0x0780,0x0780,
	0x0708,0x0708,0x0602,0x0602,0x0601,0x0601,0x0608,0x0608,0x0000,0x0820,0x0820,0x0610,0x0610,
	0x0604,0x0604,0x0801,0x0801,0x0801,0x0801,0x0801,0x0810,0x0810,0x0840,0x0840,0x0880,0x0880
};
unsigned short Keys_ini[] =
{
	0x0801,0x0780,
	0x0820,0x0840,0x0810,0x0880,
	0x0704,0x0620,0x0660,0x0680,0x0701,0x0702,0x0710,0x0802,0x0740,0x0804,0x0808,0x0720,
	0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0101,0x0102,0x0001,
	0x0240,0x0280,0x0301,0x0302,0x0304,0x0308,0x0310,
	0x0320,0x0340,0x0380,0x0401,0x0402,0x0404,0x0408,
	0x0410,0x0420,0x0440,0x0480,0x0501,0x0502,0x0504,
	0x0508,0x0510,0x0520,0x0540,0x0580,
	0x0708,0x0602,0x0601,0x0608,0x0610,0x0604,
	0x0104,0x0108,0x0110,0x0120,0x0140,0x0202,0x0180,0x0201,0x0204,0x0208,0x0210,0x0220,
	
};
int	key_shift=0,key_ctrl=0,key_graph=0;
	extern	byte *RAM[8];                     /* RAM Mapper contents    */
void	key_push_put(int i)
{
	unsigned long *vr;
	int x,y;
	vr = (void *)pgGetVramAddr(272+((i%13)*16),16+((i/13)*32));
	for(y=0;y<32;y++){
		for(x=0;x<16/2;x++){
			*vr++^=-1;
		}
		vr+=(LINESIZE-16)/2;
	}
}
void	key_put(int i)
{
	unsigned long *vr;
	int x,y;
	unsigned long	c=(22+(22<<5)+(22<<10))+((22+(22<<5)+(22<<10))<<16);
	vr = (void *)pgGetVramAddr(272+((i%13)*16),16+((i/13)*32));
	for(y=0;y<32;y++){
		for(x=0;x<16/2;x++){
			*vr=
				((((*vr&0x7c007c00)+(c&0x7c007c00))>> 1)&0x7c007c00)+
				((((*vr&0x001f001f)+(c&0x001f001f))>> 1)&0x001f001f)+
				((((*vr&0x03e003e0)+(c&0x03e003e0))>> 1)&0x03e003e0);
			vr++;
		}
		vr+=(LINESIZE-16)/2;
	}
}
void PutImage(void)
{
	
	if( game_WallPaper_c ){
		if( softkeyboard_f ){
			Bmp_put(2);
		}else{
			Bmp_put(1);
			if(ini_data.debug){
				text_print( 0, 00, "fps", rgb2col(155,155,155),rgb2col(0,0,0),0);
				text_print( 0, 10, "skip frame", rgb2col(155,155,155),rgb2col(0,0,0),0);
				text_print( 0, 20, "video mode", rgb2col(155,155,155),rgb2col(0,0,0),0);
			}
		}
		game_WallPaper_c--;
	}
	

	if( softkeyboard_f ){
		{
			unsigned long *vr;
			unsigned long *g;
			int x,y;
    
			RAM[7][0x1CAD]=ini_data.kana;
			if( bmp_f[2] ){

				vr = (void *)pgGetVramAddr(272,16);
				g = (long *)&BmpBuffer2[2][0]+(272/2)+(16*480/2);
				for(y=0;y<240;y++){
					for(x=0;x<208/2;x++){
						*vr++=*g++;
					}
					vr+=(LINESIZE-208)/2;
					g+=(480-208)/2;
				}
				if( key_shift ){
					vr = (void *)pgGetVramAddr(336,176);
					g = (long *)&BmpBuffer2[2][0]+(0/2)+(16*480/2);
					for(y=0;y<32;y++){
						for(x=0;x<32/2;x++){	*vr++=*g++;	}
						vr+=(LINESIZE-32)/2;	g+=(480-32)/2;
					}
				}
				if( key_ctrl ){
					vr = (void *)pgGetVramAddr(304,176);
					g = (long *)&BmpBuffer2[2][0]+(32/2)+(16*480/2);
					for(y=0;y<32;y++){
						for(x=0;x<32/2;x++){	*vr++=*g++;	}
						vr+=(LINESIZE-32)/2;	g+=(480-32)/2;
					}
				}
				if( key_graph ){
					vr = (void *)pgGetVramAddr(272,208);
					g = (long *)&BmpBuffer2[2][0]+(128/2)+(16*480/2);
					for(y=0;y<32;y++){
						for(x=0;x<32/2;x++){	*vr++=*g++;	}
						vr+=(LINESIZE-32)/2;	g+=(480-32)/2;
					}
				}
				if( RAM[7][0x1CAB] ){	//	caps
					vr = (void *)pgGetVramAddr(368,176);
					g = (long *)&BmpBuffer2[2][0]+(64/2)+(16*480/2);
					for(y=0;y<32;y++){
						for(x=0;x<32/2;x++){	*vr++=*g++;	}
						vr+=(LINESIZE-32)/2;	g+=(480-32)/2;
					}
				}
				if( RAM[7][0x1CAC] ){	//	kana
					vr = (void *)pgGetVramAddr(448,176);
					g = (long *)&BmpBuffer2[2][0]+(96/2)+(16*480/2);
					for(y=0;y<32;y++){
						for(x=0;x<32/2;x++){	*vr++=*g++;	}
						vr+=(LINESIZE-32)/2;	g+=(480-32)/2;
					}
				}
				if( key_push ){
					int i;
					for(i=0;i<13*7;i++){
						if( Keys[i]==key_push )key_push_put(i);
					}
					key_push=0;
				}else{
					int i;
					int c=Keys[((mouse_x/16)/16)+(((mouse_y/16)/32)*13)];
					for(i=0;i<13*7;i++){
						if( Keys[i]==c && c )key_put(i);
					}
				}
			}
    
		}
		
		
		text_print( (mouse_x/16)+272-4, (mouse_y/16)+32-16, "Бе", rgb2col(155+((frame_count&31)*3),115,115),rgb2col(0,0,0),0);
	}

	if( softkeyboard_f ){
			{
				int x,y;
				unsigned int *vr;
				unsigned int *v = (void *)XBuf;
				v+=4;
				vr = (void *)pgGetVramAddr(0,16);
				for(y=0;y<228;y++){
					for(x=0;x<256/2;x++){
						*vr++=*v++;
					}
					vr+=(LINESIZE-256)/2;
					v+=(272-256)/2;
				}
			}
	}else
		PutImage2();

			if(ini_data.debug){
				static unsigned int f=0,c[60];
				unsigned int i,a;
				for(i=0;i<60-1;i++)c[i]=c[i+1];
				c[59]=ctl.frame-f;
				for(i=0,a=0;i<60;i++)a+=c[i];
				text_counter( 60, 00, 1000000/(a/60), 3, -1);
				text_counter( 60, 10, syorioti_c, 2, -1);
				text_counter( 60, 20, ScrMode, 1, -1);


				sound_image();
			//	if( ini_frame ){
			//		text_counter( 70, 80, ini_frame, 4, -1);
			//	}else{
			//		text_print( 50, 80, "auto", rgb2col(255,255,255),rgb2col(0,0,0),-1);
			//	}
				f=ctl.frame;
			}

			{
				if( (frame_count++)&63 )
					pgScreenFlip();
				else
					pgScreenFlipV();
			}

}


unsigned int X11SetColor(unsigned char N,unsigned char R,unsigned char G,unsigned char B)
{
int	RGB;
  RGB=((int)(R>>3)<<0)|((int)(G>>3)<<5)|((int)(B>>3)<<10);

  return(RGB);
}
void SetColor(register byte N,register byte R,register byte G,register byte B)
{
  unsigned int J;

//  if(UseStatic) J=BPal[((7*R/255)<<2)|((7*G/255)<<5)|(3*B/255)];
//  else J=X11SetColor(N,R,G,B);

	J=X11SetColor(N,R,G,B);

  if(N) XPal[N]=J; else XPal0=J;
}

extern	byte ExitNow;
int	key_c=0,key_r=0;
int	key_f=0;
void Keyboard(void)
{
	unsigned long key;
	int i;

		key = Read_Key();

	JoyState=0;


	if( softkeyboard_f ){
/*
		int x,y,f=0;
		x = (mouse_x/16)/16;
		y = (mouse_y/16)/32;
		i=Keys[x+(y*13)];
		if( key_r&CTRL_LEFT   ){
			while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
				if( x<= 0 ){break;}
				x--;f++;
			}
		}
		if( key_r&CTRL_RIGHT  ){
			while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
				if( x>=12 ){break;}
				x++;f++;
			}
		}
		i=Keys[x+(y*13)];
		if( key_r&CTRL_UP     ){
			while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
				if( y<= 0 ){break;}
				y--;f++;
			}
		}
		if( key_r&CTRL_DOWN   ){
			while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
				if( y>= 6 ){break;}
				y++;f++;
			}
		}
		if( f ){
			mouse_x=(x*16*16)+( 8*16);
			mouse_y=(y*16*32)+(28*16);
		}

		f=0;
		if( ctl.buttons&CTRL_CIRCLE  ){
			if( !key_f ){
				if( i==0x0601 ){	//	shift
					key_shift^=1;
					key_f++;
					f++;
				}
				if( i==0x0602 ){	//	ctrl
					key_ctrl^=1;
					key_f++;
					f++;
				}
				if( i==0x0604 ){	//	graph
					key_graph^=1;
					key_f++;
					f++;
				}
			}
		}
		if( !f )
		if( ctl.buttons&CTRL_CIRCLE ){
			i = x+(y*13);
			XKeyMap[Keys[i]>>8]&=-1-Keys[i]&255;
			key_push = Keys[x+(y*13)];

		}
		if( ctl.buttons&CTRL_TRIANGLE )XKeyMap[0x06]&=-1-0x01;
		if( ctl.buttons&CTRL_SQUARE   )XKeyMap[0x06]&=-1-0x02;
		if( key_shift )	XKeyMap[0x06]&=-1-0x01;
		if( key_ctrl )	XKeyMap[0x06]&=-1-0x02;
		if( key_graph )	XKeyMap[0x06]&=-1-0x04;
*/
	}else{
		if( ctl.buttons&CTRL_LEFT   )JoyState|=0x04;
		if( ctl.buttons&CTRL_RIGHT  )JoyState|=0x08;
		if( ctl.buttons&CTRL_UP     )JoyState|=0x01;
		if( ctl.buttons&CTRL_DOWN   )JoyState|=0x02;
		if( ctl.analog[0] < 127-84  )JoyState|=0x04;
		if( ctl.analog[0] > 127+84  )JoyState|=0x08;
		if( ctl.analog[1] < 127-84  )JoyState|=0x01;
		if( ctl.analog[1] > 127+84  )JoyState|=0x02;
		if( ctl.buttons&ini_data.key[0] )JoyState|=0x20;
		if( ctl.buttons&ini_data.key[1] )JoyState|=0x10;
	}



	if( ctl.buttons ){
		if( !key_f ){
		//	if( ctl.buttons==CTRL_SELECT )ExitNow=1;
			if( ctl.buttons==ini_data.key[5] ){
				softkeyboard_f^=1;
				game_WallPaper_c=2;
				key_f=1;
			}else
			if( ctl.buttons==ini_data.key[10] ){
				key_f=1;
				ini_data.videomode = (++ini_data.videomode)%5;
				game_WallPaper_c=2;
			}else
			if( ctl.buttons==ini_data.key[6] ){
				wavoutStopPlay1();
				LoadStateQ();
				wavoutStartPlay1();
			}else
			if( ctl.buttons==ini_data.key[7] ){
				wavoutStopPlay1();
				SaveStateQ();
				wavoutStartPlay1();
			}else
			if( ctl.buttons==ini_data.key[8] ){
				wavoutStopPlay1();
				LoadState(StateName);
				wavoutStartPlay1();
			}else
			if( ctl.buttons==ini_data.key[9] ){
				wavoutStopPlay1();
				SaveState(StateName);
				wavoutStartPlay1();
			}else
			if( ctl.buttons==ini_data.key[4] ){
				key_f=1;
				if( menu_main()==1 ){
				}
			}
		}
	}else key_f=0;
}
void	Joystick_sync(void) 
{
	{
		int i;
		key_r=0;
		if( ctl.buttons ){
			if( (--key_c)<=0 ){
				key_r=ctl.buttons;
				if( key_c<0 )key_c=20;else key_c=2;
			}
		}else{
			key_c=0;
		}
		for( i=0;i<16;i++ )XKeyMap[i]=-1;
		if( softkeyboard_f ){
			int x,y,f=0;
			x = (mouse_x/16)/16;
			y = (mouse_y/16)/32;
			i=Keys[x+(y*13)];
			if( key_r&CTRL_LEFT   ){
				while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
					if( x<= 0 ){break;}
					x--;f++;
				}
			}
			if( key_r&CTRL_RIGHT  ){
				while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
					if( x>=12 ){break;}
					x++;f++;
				}
			}
			i=Keys[x+(y*13)];
			if( key_r&CTRL_UP     ){
				while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
					if( y<= 0 ){break;}
					y--;f++;
				}
			}
			if( key_r&CTRL_DOWN   ){
				while(i==Keys[x+(y*13)]||!Keys[x+(y*13)]){
					if( y>= 6 ){break;}
					y++;f++;
				}
			}
			if( f ){
				mouse_x=(x*16*16)+( 8*16);
				mouse_y=(y*16*32)+(28*16);
			}
    
			f=0;
			if( ctl.buttons&CTRL_CIRCLE  ){
				if( !key_f ){
					if( i==0x0601 ){	//	shift
						key_shift^=1;
						key_f++;
						f++;
					}
					if( i==0x0602 ){	//	ctrl
						key_ctrl^=1;
						key_f++;
						f++;
					}
					if( i==0x0604 ){	//	graph
						key_graph^=1;
						key_f++;
						f++;
					}
				}
			}
			if( !f )
			if( ctl.buttons&CTRL_CIRCLE ){
				i = x+(y*13);
				XKeyMap[Keys[i]>>8]&=-1-Keys[i]&255;
				key_push = Keys[x+(y*13)];
    
			}
			if( ctl.buttons&CTRL_TRIANGLE )XKeyMap[0x06]&=-1-0x01;
			if( ctl.buttons&CTRL_SQUARE   )XKeyMap[0x06]&=-1-0x02;
			if( key_shift )	XKeyMap[0x06]&=-1-0x01;
			if( key_ctrl )	XKeyMap[0x06]&=-1-0x02;
			if( key_graph )	XKeyMap[0x06]&=-1-0x04;
    
			if( ctl.analog[0]<127-40 )mouse_x+=(ctl.analog[0]-127+40);
			if( ctl.analog[0]>127+40 )mouse_x+=(ctl.analog[0]-127-40);
			if( ctl.analog[1]<127-40 )mouse_y+=(ctl.analog[1]-127+40);
			if( ctl.analog[1]>127+40 )mouse_y+=(ctl.analog[1]-127-40);
			if( mouse_x<  8*16 )mouse_x=  8*16;
			if( mouse_x>198*16 )mouse_x=198*16;
			if( mouse_y<  8*16 )mouse_y=  8*16;
			if( mouse_y>220*16 )mouse_y=220*16;
		}else{
		{
			int i;
			for(i=12;i<84;i++){
				if( ctl.buttons&ini_data.key[i] ){
					XKeyMap[Keys_ini[i-12]>>8]&=-1-Keys_ini[i-12]&255;
				}
			}
		}
		}
		memcpy(KeyMap,XKeyMap,sizeof(XKeyMap));
	}
	{
		static unsigned int r;
		if( ((r++)&3) > 1 ){
			if( ctl.buttons&ini_data.key[2] )JoyState&=-1-0x20;
			if( ctl.buttons&ini_data.key[3] )JoyState&=-1-0x10;
		}else{
			if( ctl.buttons&ini_data.key[2] )JoyState|=0x20;
			if( ctl.buttons&ini_data.key[3] )JoyState|=0x10;
		}
	}
}
byte Joystick(register byte N) 
{ 
	return(JoyState); 
}

char	XBuf_buff[512*228*2];
int		InitMachine(void)
{
	XBuf = XBuf_buff;

//	// Default disk images //
//	Disks[0][1]=Disks[1][1]=0;
//	Disks[0][0]=DSKName[0];
//	Disks[1][0]=DSKName[1];

	{
	  int J,I;

	  White=4234;
	  Black=865;
	  // Reset the palette //
	  for(J=0;J<16;J++) XPal[J]=Black;
	  XPal0=Black;
	
	  // Set SCREEN8 colors //
	  for(J=0;J<64;J++)
	  {
	    I=(J&0x03)+(J&0x0C)*16+(J&0x30)/2;
	    XPal[J+16]=X11SetColor(J+16,(J&0x30)*255/48,(J&0x0C)*255/12,(J&0x03)*255/3);
	    BPal[I]=BPal[I|0x04]=BPal[I|0x20]=BPal[I|0x24]=XPal[J+16];
	  }
	}

#ifdef SOUND
  /* Initialize sound */   
//  if(InitSound(UseSound,Verbose))
  //  SetChannels(SndVolume,SndSwitch);
#endif

	return	0;
}
void	TrashMachine(void)
{
}






#include "..\msx\Common.h"










