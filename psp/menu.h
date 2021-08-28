

#define MAXDEPTH  16
#define MAXDIRNUM 1024
#define MAXPATH   0x108
#define PATHLIST_H 23
#define REPEAT_TIME 0x40000

extern	char	target[MAXPATH];
extern	char	path_main[MAXPATH];

int		menu_file(void);
void	menu_init(void);

typedef struct {
	int		ver;
	int		debug;
	char	path[MAXPATH];
	int		msx_type;
	char	path_rom[3][MAXPATH];
	int		mapper[3];
	char	path_disk[2][MAXPATH];
	int		WallPaper[3];
	char	path_WallPaper[3][MAXPATH];
	int		sound;
	int		autosave;	//	bit:1 disk
						//	bit:2 state
						//	bit:3 cmos
						//	bit:4 sram
	int		CpuClock;
	int		CpuClock_game;
	int		CpuClock_menu;
	int		videomode;
	int		screen7;
	int		kana;
	int		key[100];
} ini_info;
extern	ini_info	ini_data;
extern	dirent_t		dlist[MAXDIRNUM];
extern	int				dlist_num;
extern	int	bmp_f[3];
extern	short	BmpBuffer2[3][480*272];
extern	int	DiskAutoSave_flag[2];
extern	int	DiskAutoSave_name[2][MAXPATH];


void	ini_read(void);
void	ini_write(void);

int	kakutyousichk(int z,char *n);



