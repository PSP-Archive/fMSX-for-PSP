
#include "psp.h"




static	unsigned long control_bef_ctl  ;
static	unsigned long control_bef_tick ;

#define REPEAT_TIME 0x40000
unsigned long Read_Key(void)
{
//	ctrl_data_t ctl;

	sceCtrlRead(&ctl,1);
	if (ctl.buttons == control_bef_ctl) {
		if ((ctl.frame - control_bef_tick) > REPEAT_TIME) {
			return control_bef_ctl;
		}
		return 0;
	}
	control_bef_ctl  = ctl.buttons;
	control_bef_tick = ctl.frame;
	return control_bef_ctl;
}

void	Error_mes(const char *t)
{
	pgFillvram(0);
	text_print(0,100,t,rgb2col(255,0,0),0,0);
	pgScreenFlipV();
	
	{
		int i=10;
		while(i--)pgWaitV();
	}
	while(1) {
		unsigned long key;
		key = Read_Key();
		if ( !key ) break;
		pgWaitV();
	}
	while(1) {
		unsigned long key;
		key = Read_Key();
		if (key ) break;
		pgWaitV();
	}
}

char	zip_path_buffer[MAXPATH];
char	zip_path_buffer2[MAXPATH];
char *zipchk(const char *nn)
{
	char	*n;
	_memcpy((void*)zip_path_buffer, (void*)nn,MAXPATH);
	n=zip_path_buffer;
	while(*n){
		if( n[0]=='.' &&
			(n[1]=='z'||n[1]=='Z') &&
			(n[2]=='i'||n[2]=='I') &&
			(n[3]=='p'||n[3]=='P') &&
			(!n[4]||n[4]=='/') 
		){
			if( n[4]=='/' )
				memcpy( zip_path_buffer2, &n[5],MAXPATH);
			else 
				zip_path_buffer2[0]=0;
			n[4]=0;
			return zip_path_buffer;
		}
		n++;
	}
	return 0;
}

void	counter(int x,int y,unsigned int c,int k,int col)
{
	char m[2];
	m[1]=0;
	do{
		m[0]='0'+(c%10);
		text_print(x-=5,y,m,col,0,1);
		c/=10;
		if(!(--k))c=0;
	}while(c);
	while(k--){
		text_print(x-=5,y," ",col,0,1);
	}

}
void	Error_count(int c)
{
	pgFillvram(0);
	counter( 50, 0, c, 8, -1);
	pgScreenFlipV();
	{
		int i=30;
		while(i--)pgWaitV();
	}
	while(1) {
		unsigned long key;
		key = Read_Key();
		if (key ) break;
		pgWaitV();
	}
}


void	CpuClockSet(int i)
{
	static int c[]={
		133, 166, 190, 222, 266, 333, 
	};
	if( i< 0 || i>5 ){
		Error_mes("CpuClock setting error");
	}else{
		scePowerSetClockFrequency(c[i],c[i],c[i]/2);
	}
}



