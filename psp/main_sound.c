
#include "psp.h"
#include "main_sound.h"
#include "..\msx\Sound.h"




#define AUDIO_CONV(A) (ULAW[0xFF&(128+(A))]) 
static const unsigned char ULAW[256] =
{
    31,   31,   31,   32,   32,   32,   32,   33,
    33,   33,   33,   34,   34,   34,   34,   35,
    35,   35,   35,   36,   36,   36,   36,   37,
    37,   37,   37,   38,   38,   38,   38,   39,
    39,   39,   39,   40,   40,   40,   40,   41,
    41,   41,   41,   42,   42,   42,   42,   43,
    43,   43,   43,   44,   44,   44,   44,   45,
    45,   45,   45,   46,   46,   46,   46,   47,
    47,   47,   47,   48,   48,   49,   49,   50,
    50,   51,   51,   52,   52,   53,   53,   54,
    54,   55,   55,   56,   56,   57,   57,   58,
    58,   59,   59,   60,   60,   61,   61,   62,
    62,   63,   63,   64,   65,   66,   67,   68,
    69,   70,   71,   72,   73,   74,   75,   76,
    77,   78,   79,   81,   83,   85,   87,   89,
    91,   93,   95,   99,  103,  107,  111,  119,
   255,  247,  239,  235,  231,  227,  223,  221,
   219,  217,  215,  213,  211,  209,  207,  206,
   205,  204,  203,  202,  201,  200,  199,  198,
   219,  217,  215,  213,  211,  209,  207,  206,
   205,  204,  203,  202,  201,  200,  199,  198,
   197,  196,  195,  194,  193,  192,  191,  191,
   190,  190,  189,  189,  188,  188,  187,  187,
   186,  186,  185,  185,  184,  184,  183,  183,
   182,  182,  181,  181,  180,  180,  179,  179,
   178,  178,  177,  177,  176,  176,  175,  175,
   175,  175,  174,  174,  174,  174,  173,  173,
   173,  173,  172,  172,  172,  172,  171,  171,
   171,  171,  170,  170,  170,  170,  169,  169,
   169,  169,  168,  168,  168,  168,  167,  167,
   167,  167,  166,  166,  166,  166,  165,  165,
   165,  165,  164,  164,  164,  164,  163,  163
};
static int PipeFD[2];
static int PeerPID      = 0;
static int SoundFD      = -1;
static int SoundRate    = 0;
static int MasterVolume = 200;
static int MasterSwitch = (1<<SND_CHANNELS)-1;
static int LoopFreq     = 25;
static int NoiseGen     = 1;
static int Verbose      = 1;

static void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate);
static void UnixSetSound(int Channel,int NewType);
static void UnixDrum(int Type,int Force);
static void UnixSetChannels(int Volume,int Switch);
static void UnixSound(int Channel,int NewFreq,int NewVolume);


int wavout_snd1_playing[SND1_MAXSLOT];
int wavout_snd1_c[SND1_MAXSLOT];
wavout_wavinfo_t wavout_snd1_wavinfo[SND1_MAXSLOT];
unsigned short fcpsoundbuff[SND1_MAXSLOT][735*60];


int Wave[SND_CHANNELS][SND_BUFSIZE];


static struct   
{
  int Type;                       /* Channel type (SND_*)             */
  int Freq;                       /* Channel frequency (Hz)           */
  int Volume;                     /* Channel volume (0..255)          */

//  signed 
  char Data[SND_SAMPLESIZE]; /* Wave data (-128..127 each)     */
  int Length;                     /* Wave length in Data              */
  int Rate;                       /* Wave playback rate (or 0Hz)      */
  int Pos;                        /* Wave current position in Data    */

  int Count;                      /* Phase counter                    */
  
  int c;
} CH[SND_CHANNELS];

void	sound_image(void)
{
				{
					int i;
					for( i=0;i<SND_CHANNELS;i++ ){
						text_counterh( 10, (i*10)+60, CH[i].Volume, 2, -1);
					//	text_counterh( (i*13)+40, 252, CH[i].c, 2, -1);
						CH[i].c=0;
					}
				}
}
void    msx_waveout(void)
{
	int ii;
	int J,I,K,L,M,N,L1,L2,A1,A2,V;
	
    for(J=0,M=MasterSwitch;M&&(J<SND_CHANNELS);J++,M>>=1){
      if(CH[J].Freq&&(V=CH[J].Volume)&&(M&1)){
        switch(CH[J].Type) {
          case SND_NOISE: /* White Noise */
            /* For high frequencies, recompute volume */
            if(CH[J].Freq<=SoundRate) K=0x10000*CH[J].Freq/SoundRate;
            else { V=V*SoundRate/CH[J].Freq;K=0x10000; }
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++){
              L1+=K;
              if(L1&0xFFFF0000){
                L1&=0xFFFF;
                if((NoiseGen<<=1)&0x80000000) NoiseGen^=0x08000001;  
              }
              Wave[J][I]=NoiseGen&1? V:-V;
            }
            CH[J].Count=L1;
            break;
              
          case SND_WAVE: /* Custom Waveform */
            /* Waveform data must have correct length! */
            if(CH[J].Length<=0) break;
            /* Start counting */
            K  = CH[J].Rate>0? (SoundRate<<15)/CH[J].Freq/CH[J].Rate
                             : (SoundRate<<15)/CH[J].Freq/CH[J].Length;
            L1 = CH[J].Pos%CH[J].Length;
            L2 = CH[J].Count;
            A1 = CH[J].Data[L1]*V;
            /* If expecting interpolation... */
            if(L2<K) {
              /* Compute interpolation parameters */
              A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
              L  = (L2>>15)+1;
              N  = ((K-(L2&0x7FFF))>>15)+1;
			}
            /* Add waveform to the buffer */  
            for(I=0;I<SND_BUFSIZE;I++)
              if(L2<K) {
                /* Interpolate linearly */
                Wave[J][I]=A1+L*(A2-A1)/N;
                /* Next waveform step */
                L2+=0x8000;  
                /* Next interpolation step */
                L++;
              } else {
                L1 = (L1+L2/K)%CH[J].Length;
                L2 = (L2%K)+0x8000;
                A1 = CH[J].Data[L1]*V;
               Wave[J][I]=A1;


                /* If expecting interpolation... */
                if(L2<K) {
                  /* Compute interpolation parameters */
                  A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
                  L  = 1;
                  N  = ((K-L2)>>15)+1;  
                }
              }
            /* End counting */
            CH[J].Pos   = L1;
            CH[J].Count = L2;
            break;
                
          case SND_MELODIC:  /* Melodic Sound   */
          case SND_TRIANGLE: /* Triangular Wave */
          default:           /* Default Sound   */
            /* Triangular wave has twice less volume */
            if(CH[J].Type==SND_TRIANGLE) V=(V+1)>>1;
            /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=(0x10000)*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++){
              L2=L1+K;
             Wave[J][I]=L1&0x8000? (L2&0x8000? V:0):(L2&0x8000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;  
            break;
        }
		}else{
            for(I=0;I<SND_BUFSIZE;I++){
             Wave[J][I]=0;
            }
		}
	}
    /* Mix and convert waveforms */   
	for(J=0;J<SND_CHANNELS;J++)
	for(I=0;I<SND_BUFSIZE;I++){
		int i;
		char c;
		short s;

		s = ((Wave[J][I]*MasterVolume)/(1<<7));
		if( J>=6 ){
			s*=2;
		}

		fcpsoundbuff[J][wavout_snd1_c[J]]=s;//AUDIO_CONV(I);//(short)(AUDIO_CONV(I)*256);
		if( (++wavout_snd1_c[J]) >= (SND_BUFSIZE*60) )wavout_snd1_c[J] = 0;

		Wave[J][I]=0;
	}

	
}




/** UnixSound() **********************************************/
/** Generate sound of given frequency (Hz) and volume       **/
/** (0..255) via given channel.                             **/
/*************************************************************/
void UnixSound(int Channel,int NewFreq,int NewVolume)
{

  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  if(!SoundRate||!(MasterSwitch&(1<<Channel))) return;
  if(!NewVolume||!NewFreq) { NewVolume=0;NewFreq=0; }

//  if((CH[Channel].Volume!=NewVolume)||(CH[Channel].Freq!=NewFreq))
//  {
    CH[Channel].Volume = NewVolume;
    CH[Channel].Freq   = NewFreq;
//  }
    CH[Channel].c++;
}

/** UnixSetChannels() ****************************************/
/** Set master volume (0..255) and turn channels on/off.    **/
/** Each bit in Toggle corresponds to a channel (1=on).     **/
/*************************************************************/
void UnixSetChannels(int MVolume,int MSwitch)
{
  int J;

  if(!SoundRate) return;

  /* Switching channels on/off */
  for(J=0;J<SND_CHANNELS;J++)
    if((MSwitch^MasterSwitch)&(1<<J)) {
      /* Set volume/frequency */
      if(!(MSwitch&(1<<J))){
        CH[J].Volume=0;
        CH[J].Freq=0;
      } else {
      }
    }

  /* Set new MasterSwitch value */
  MasterSwitch=MSwitch;
  MasterVolume=MVolume;
}

/** UnixSetSound() *******************************************/
/** Set sound type (SND_NOISE/SND_MELODIC) for a given      **/
/** channel.                                                **/
/*************************************************************/
void UnixSetSound(int Channel,int NewType)
{

  if(!SoundRate) return;
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  CH[Channel].Type = NewType;
/*
	if( NewType== SND_NOISE){
		text_print(0, 0,"ÉmÉCÉYèâä˙âª",rgb2col( 55,255,255),0,0);
	}
		text_print(0, 10,"âΩÇ©ÇµÇÁèâä˙âªÇµÇΩ",rgb2col( 55,255,255),0,0);
*/
}

/** UnixSetWave() ********************************************/
/** Set waveform for a given channel. The channel will be   **/
/** marked with sound type SND_WAVE. Set Rate=0 if you want **/
/** waveform to be an instrument or set it to the waveform  **/
/** own playback rate.                                      **/
/*************************************************************/
void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate)
{

  if(!SoundRate) return;
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  if((Length<=0)||(Length>SND_SAMPLESIZE)) return;

                         CH[Channel].Type   = SND_WAVE;
                         CH[Channel].Rate   = Rate;
                         CH[Channel].Length = Length;
                         CH[Channel].Count  = 0;
                         CH[Channel].Pos    = 0;
                         _memcpy( (char*)CH[Channel].Data, (char*)Data, Length );

}


/** UnixDrum() ***********************************************/
/** Hit a drum of a given type with given force.            **/
/*************************************************************/
void UnixDrum(int Type,int Force)
{
  /* This function is currently empty */
}







static void wavout_snd1_callback(short *_buf, unsigned long _reqn)
{
	unsigned long i,slot;
	wavout_wavinfo_t *wi;
	unsigned long ptr,frac;
	short *buf=_buf;
	
	for (i=0; i<_reqn; i++) {
		int outr=0,outl=0;
		for (slot=0; slot<SND1_MAXSLOT; slot++) {
			if (!wavout_snd1_playing[slot]) continue;
			wi=&wavout_snd1_wavinfo[slot];
			frac=wi->playptr_frac+wi->rateratio;
			wi->playptr=ptr=wi->playptr+(frac>>16);
			wi->playptr_frac=(frac&0xffff);
			if (ptr>=wi->samplecount) {
				if( wi->playloop ){
					wi->playptr=0;
				}else{
					wavout_snd1_playing[slot]=0;
				}
				break;
			}
			short *src=(short *)wi->wavdata;
			if (wi->channels==1) {
				outl+=src[ptr];
				outr+=src[ptr];
			} else {
				outl+=src[ptr*2];
				outr+=src[ptr*2+1];
			}
		}
		if (outl<-32768) outl=-32768;
		else if (outl>32767) outl=32767;
		if (outr<-32768) outr=-32768;
		else if (outr>32767) outr=32767;
		*(buf++)=outl;
		*(buf++)=outr;
	}
}
void wavoutStopPlay1()
{
	int i;
	for (i=0; i<SND1_MAXSLOT; i++){
		 wavout_snd1_playing[i]=0;
	}
	_memset(fcpsoundbuff,0,SND1_MAXSLOT*735*60*2);


	{
		int J,I;
		for(J=0;J<SND_CHANNELS;J++){
			for(I=0;I<SND_BUFSIZE;I++) Wave[J][I]=0;
		}
	}

}
extern int  UCount;
void wavoutStartPlay1()
{
	int i;

	for (i=0; i<SND1_MAXSLOT; i++){
		wavout_snd1_c[i]=SND_BUFSIZE*3;
		wavout_snd1_playing[i]=1;
		wavout_snd1_wavinfo[i].playptr=0;
//		_memset(&fcpsoundbuff[i][0],0,735*60*2);

	}

	UCount = 1;

}
void wavout_1secInits(int i)
{
	wavout_wavinfo_t *w=&wavout_snd1_wavinfo[i];
	w->channels = 1;
	w->samplerate = SND_BUFSIZE*60;//15360;//30720;
	w->samplecount = (SND_BUFSIZE*60);
	w->datalength = (SND_BUFSIZE*60)*2;
	w->wavdata = (void *)&fcpsoundbuff[i][0];
	w->rateratio = (SND_BUFSIZE*60*0x4000)/11025;
	w->playptr = 0;
	w->playptr_frac = 0;
	w->playloop = 1;
//	wavout_snd1_playing[i]=1;
}
void wavoutCHInit(void)
{
		int i;
		for (i=0; i<SND_CHANNELS; i++){
		    CH[i].Type   = SND_MELODIC;
		    CH[i].Count  = 0;
		    CH[i].Volume = 0;
		    CH[i].Freq   = 0;
		}
}
int wavoutInit()
{
	int i;


	
	wavoutStopPlay1();


	SoundRate = SND_BUFSIZE*60;//15360;//30720;
	SoundFD   = -1;
	PeerPID   = 0;
	Verbose   = 1;
  SndDriver.SetSound    = UnixSetSound;
  SndDriver.Drum        = UnixDrum;
  SndDriver.SetChannels = UnixSetChannels;
  SndDriver.Sound       = UnixSound;
  SndDriver.SetWave     = UnixSetWave;

	SetChannels(192,0xFF);

	SetChannels(255/MAXCHANNELS,(1<<MAXCHANNELS)-1);

	pgaSetChannelCallback(1,wavout_snd1_callback);

	for (i=0; i<SND1_MAXSLOT; i++) wavout_1secInits(i);


	return 0;
}


