

typedef struct {
	unsigned long channels;
	unsigned long samplerate;
	unsigned long samplecount;
	unsigned long datalength;
	char *wavdata;
	unsigned long rateratio;		// samplerate / 44100 * 0x10000
	unsigned long playptr;
	unsigned long playptr_frac;
	int playloop;
} wavout_wavinfo_t;



int wavoutInit();
void wavoutStopPlay1();
void wavoutStartPlay1();

#define SND1_MAXSLOT 16
extern	unsigned short fcpsoundbuff[SND1_MAXSLOT][735*60];
void    msx_waveout(void);
void wavoutCHInit(void);
void	sound_image(void);

