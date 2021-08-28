/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                           MSX.h                         **/
/**                                                         **/
/** This file contains declarations relevant to the drivers **/
/** and MSX emulation itself. See Z80.h for #defines        **/
/** related to Z80 emulation.                               **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2005                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef MSX_H
#define MSX_H

#ifndef PIXEL_TYPE_DEFINED
#define PIXEL_TYPE_DEFINED
#ifdef BPP32
typedef unsigned int pixel;
#else
#ifdef BPP24
typedef unsigned int pixel;
#else
#ifdef BPP16
typedef unsigned short pixel;
#else
#ifdef BPP8
typedef unsigned char pixel;
#endif
#endif
#endif
#endif
#endif

typedef int FILE;

#include "Z80.h"            /* Z80 CPU emulation             */
#include "V9938.h"          /* V9938 VDP opcode emulation    */
#include "AY8910.h"         /* AY8910 PSG emulation          */
#include "YM2413.h"         /* YM2413 OPLL emulation         */
#include "SCC.h"            /* Konami SCC chip emulation     */
#include "I8255.h"          /* Intel 8255 PPI emulation      */
#include "I8251.h"          /* Intel 8251 UART emulation     */

//#include <stdio.h>

#define CPU_CLOCK    3580        /* CPU clock frequency, kHz */
#define VDP_CLOCK    21480       /* VDP clock frequency, kHz */

#define HPERIOD      1368        /* HPeriod, VDP cycles      */
#define VPERIOD_PAL  (HPERIOD*313) /* PAL VPeriod, VDP ccls  */
#define VPERIOD_NTSC (HPERIOD*262) /* NTSC VPeriod, VDP ccls */ 
#define HREFRESH_240 960         /* 240dot scanline refresh  */
#define HREFRESH_256 1024        /* 256dot scanline refresh  */

#define CPU_VPERIOD  (VPERIOD_NTSC/6)
#define CPU_V262     (VPERIOD_NTSC/6)
#define CPU_V313     (VPERIOD_PAL/6)
#define CPU_HPERIOD  (HPERIOD/6)
#define CPU_H240     (HREFRESH_240/6)
#define CPU_H256     (HREFRESH_256/6)

#define INT_IE0     0x01
#define INT_IE1     0x02
#define INT_IE2     0x04

                            /* ROM mapper types:             */
#define MAP_GEN8     0      /* Generic switch, 8kB pages     */
#define MAP_GEN16    1      /* Generic switch, 16kB pages    */
#define MAP_KONAMI5  2      /* Konami 5000/7000/9000/B000h   */
#define MAP_KONAMI4  3      /* Konami 4000/6000/8000/A000h   */
#define MAP_ASCII8   4      /* ASCII 6000/6800/7000/7800h    */
#define MAP_ASCII16  5      /* ASCII 6000/7000h              */
#define MAP_GMASTER2 6      /* Konami GameMaster2 cartridge  */
#define MAP_FMPAC    7      /* Panasonic FMPAC cartridge     */
#define MAP_GUESS    MAXMAPPERS /* Guess mapper type on load */

#define MAP_SRAM(N) \
  (((N)==MAP_ASCII8)||((N)==MAP_ASCII16)|| \
   ((N)==MAP_GMASTER2)||((N)==MAP_FMPAC))

#define FMPAC_MAGIC 0x694D  /* FMPAC SRAM "magic value"      */

#define PAGESIZE    0x4000L /* Size of a RAM page            */
#define NORAM       0xFF    /* Byte to be returned from      */
                            /* non-existing pages and ports  */
#define MAXSCREEN   12      /* Highest screen mode supported */
#define MAXSPRITE1  4       /* Sprites/line in SCREEN 1-3    */
#define MAXSPRITE2  8       /* Sprites/line in SCREEN 4-8    */
#define MAXDRIVES   2       /* Number of disk drives         */
#define MAXDISKS    32      /* Number of disks for a drive   */
#define MAXCARTS    3       /* Number of cartridge slots     */
#define MAXMAPPERS  8       /* Total defined MegaROM mappers */

#define MAXCHANNELS (AY8910_CHANNELS+YM2413_CHANNELS)
  /* Number of sound channels used by the emulation */

/** Following macros can be used in screen drivers ***********/
#define BigSprites    (VDP[1]&0x01)   /* Zoomed sprites      */
#define Sprites16x16  (VDP[1]&0x02)   /* 16x16/8x8 sprites   */
#define ScreenON      (VDP[1]&0x40)   /* Show screen         */
#define SpritesOFF    (VDP[8]&0x02)   /* Don't show sprites  */
#define SolidColor0   (VDP[8]&0x20)   /* Solid/Tran. COLOR 0 */
#define PALVideo      (VDP[9]&0x02)   /* PAL/NTSC video      */
#define FlipEvenOdd   (VDP[9]&0x04)   /* Flip even/odd pages */
#define InterlaceON   (VDP[9]&0x08)   /* Interlaced screen   */
#define ScanLines212  (VDP[9]&0x80)   /* 212/192 scanlines   */
#define HScroll512    (VDP[25]&0x01)  /* HScroll both pages  */
#define MaskEdges     (VDP[25]&0x02)  /* Mask 8-pixel edges  */
#define ModeYJK       (VDP[25]&0x08)  /* YJK screen mode     */
#define ModeYAE       (VDP[25]&0x10)  /* YJK/YAE screen mode */
#define VScroll       VDP[23]
#define HScroll       ((VDP[27]&0x07)|((int)(VDP[26]&0x3F)<<3))
#define VAdjust       (-((signed char)(VDP[18])>>4))
#define HAdjust       (-((signed char)(VDP[18]<<4)>>4))
/*************************************************************/

/** Variables used to control emulator behavior **************/
extern int Verbose;                  /* Debug msgs ON/OFF   */
extern byte MSXVersion;               /* 0=MSX,1=MSX2,2=MSX2+*/
extern byte MSXType;  	             /* 0=MSX,1=MSX2,2=MSX2+*/
extern byte ROMType[MAXCARTS];        /* MegaROM types       */
extern int  RAMPages,VRAMPages;       /* Number of RAM pages */
extern int  VPeriod;                  /* CPU cycles / VBlank */
extern int  HPeriod;                  /* CPU cycles / HBlank */
extern byte UPeriod;                  /* Int-pts/Scr. update */
extern byte JoyTypeA,JoyTypeB;        /* 0=No,1=Jstk,2/3=Mse */
extern byte AutoFire;                 /* Autofire on [SPACE] */
extern byte UseDrums;                 /* Drums for PSG noise */
/*************************************************************/

extern Z80  CPU;                      /* CPU state/registers */
extern byte *VRAM;                    /* Video RAM           */
extern byte VDP[64];                  /* VDP control reg-ers */
extern byte VDPStatus[16];            /* VDP status reg-ers  */
extern byte *ChrGen,*ChrTab,*ColTab;  /* VDP tables (screen) */
extern byte *SprGen,*SprTab;          /* VDP tables (sprites)*/
extern int  ChrGenM,ChrTabM,ColTabM;  /* VDP masks (screen)  */
extern int  SprTabM;                  /* VDP masks (sprites) */
extern byte FGColor,BGColor;          /* Colors              */
extern byte XFGColor,XBGColor;        /* Alternative colors  */
extern byte ScrMode;                  /* Current screen mode */
extern int  ScanLine;                 /* Current scanline    */

extern byte KeyMap[16];               /* Keyboard map        */
extern byte ExitNow;                  /* 1: Exit emulator    */

extern byte PSLReg;                   /* Primary slot reg.   */
extern byte SSLReg;                   /* Secondary slot reg. */

extern char *ProgDir;                 /* Program directory   */
extern char *ROMName[MAXCARTS];       /* Cart A/B ROM files  */
extern char *DSKName[MAXDRIVES];      /* Disk A/B images     */
extern char *SndName;                 /* Soundtrack log file */
extern char *PrnName;                 /* Printer redir. file */
extern char *CasName;                 /* Tape image file     */
extern char *ComName;                 /* Serial redir. file  */
extern char *StateName;               /* State save name     */

extern FILE CasStream;               /* Cassette I/O stream */

extern char *FontName;                /* Font file for text  */ 
extern byte *FontBuf;                 /* Font for text modes */
extern byte UseFont;                  /* 1: Use external font*/

/** StartMSX() ***********************************************/
/** Allocate memory, load ROM image, initialize hardware,   **/
/** CPU and start the emulation. This function returns 0 in **/
/** the case of failure.                                    **/
/*************************************************************/
int StartMSX(void);

/** TrashMSX() ***********************************************/
/** Free memory allocated by StartMSX().                    **/
/*************************************************************/
void TrashMSX(void);

/** SaveState() **********************************************/
/** Save emulation state to a .STA file.                    **/
/*************************************************************/
extern	int	State_f;
int SaveState(const char *FileName);
int SaveStateQ(void);

/** LoadState() **********************************************/
/** Load emulation state from a .STA file.                  **/
/*************************************************************/
int LoadState(const char *FileName);
int LoadStateQ(void);

/** ChangeDisk() *********************************************/   
/** Change disk image in a given drive. Closes current disk **/
/** image if Name=0 was given. Returns 1 on success or 0 on **/
/** failure. This function is part of generic disk drivers  **/
/** in Disk.c. It is compiled when DISK is #defined.        **/
/*************************************************************/
#ifdef DISK
byte ChangeDisk(byte ID,char *Name);
#endif

/** InitMachine() ********************************************/
/** Allocate resources needed by the machine-dependent code.**/
/************************************ TO BE WRITTEN BY USER **/
int InitMachine(void);

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/************************************ TO BE WRITTEN BY USER **/
void TrashMachine(void);

/** Keyboard() ***********************************************/
/** This function is periodically called to poll keyboard.  **/
/************************************ TO BE WRITTEN BY USER **/
void Keyboard(void);

/** Joystick() ***********************************************/
/** Query position of a joystick connected to port N.       **/
/** Returns 0.0.F2.F1.R.L.D.U.                              **/
/************************************ TO BE WRITTEN BY USER **/
byte Joystick(byte N);

/** Mouse() **************************************************/
/** Query coordinates of a mouse connected to port N.       **/
/** Returns F2.F1.Y.Y.Y.Y.Y.Y.Y.Y.X.X.X.X.X.X.X.X.          **/
/************************************ TO BE WRITTEN BY USER **/
int Mouse(byte N);

/** DiskPresent()/DiskRead()/DiskWrite() *********************/
/*** These three functions are called to check for floppyd  **/
/*** disk presence in the "drive", and to read/write given  **/
/*** sector to the disk.                                    **/
/************************************ TO BE WRITTEN BY USER **/
byte DiskPresent(byte ID);
byte DiskRead(byte ID,byte *Buf,int N);
byte DiskWrite(byte ID,byte *Buf,int N);

/** SetColor() ***********************************************/
/** Set color N (0..15) to (R,G,B).                         **/
/************************************ TO BE WRITTEN BY USER **/
void SetColor(byte N,byte R,byte G,byte B);

/** RefreshScreen() ******************************************/
/** Refresh screen. This function is called in the end of   **/
/** refresh cycle to show the entire screen.                **/
/************************************ TO BE WRITTEN BY USER **/
void RefreshScreen(void);

/** RefreshLine#() *******************************************/
/** Refresh line Y (0..191/211), on an appropriate SCREEN#, **/
/** including sprites in this line.                         **/
/************************************ TO BE WRITTEN BY USER **/
void RefreshLineTx80(byte Y);
void RefreshLine0(byte Y);
void RefreshLine1(byte Y);
void RefreshLine2(byte Y);
void RefreshLine3(byte Y);
void RefreshLine4(byte Y);
void RefreshLine5(byte Y);
void RefreshLine6(byte Y);
void RefreshLine7(byte Y);
void RefreshLine8(byte Y);
void RefreshLine10(byte Y);
void RefreshLine12(byte Y);

#endif /* MSX_H */
