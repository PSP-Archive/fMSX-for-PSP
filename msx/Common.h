/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                         Common.h                        **/
/**                                                         **/
/** This file contains standard screen refresh drivers      **/
/** common for X11, VGA, and other "chunky" bitmapped video **/
/** implementations. It also includes dummy sound drivers   **/
/** for fMSX.                                               **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2005                 **/
/**               John Stiles     1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/






/** INLINE ***************************************************/
/** Different compilers inline C functions differently.     **/
/*************************************************************/
#ifdef __GNUC__
#define INLINE inline
#else

#define INLINE
#endif

static int FirstLine = 18;     /* First scanline in the XBuf */

static void  Sprites(byte Y,pixel *Line);
static void  ColorSprites(byte Y,byte *ZBuf);
static pixel *RefreshBorder(byte Y,pixel C);
static void  ClearLine(pixel *P,pixel C);
static pixel YJKColor(int Y,int J,int K);

/** RefreshScreen() ******************************************/
/** Refresh screen. This function is called in the end of   **/
/** refresh cycle to show the entire screen.                **/
/*************************************************************/
void RefreshScreen(void) { PutImage(); }

/** ClearLine() **********************************************/
/** Clear 256 pixels from P with color C.                   **/
/*************************************************************/
static void ClearLine(register pixel *P,register pixel C)
{
  register int J;

  for(J=0;J<256;J++) P[J]=C;
}

/** YJKColor() ***********************************************/
/** Given a color in YJK format, return the corresponding   **/
/** palette entry.                                          **/
/*************************************************************/
static INLINE pixel YJKColor(register int Y,register int J,register int K)
{
  register int R,G,B;
		
  R=Y+J;
  G=Y+K;
  B=((5*Y-2*J-K)/4);

  R=R<0? 0:R>31? 31:R;
  G=G<0? 0:G>31? 31:G;
  B=B<0? 0:B>31? 31:B;

  return(BPal[(R&0x1C)|((G&0x1C)<<3)|(B>>3)]);
}

/** RefreshBorder() ******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** the screen border. It returns a pointer to the start of **/
/** scanline Y in XBuf or 0 if scanline is beyond XBuf.     **/
/*************************************************************/
pixel *RefreshBorder(register byte Y,register pixel C)
{
  register pixel *P;
  register int H;

  /* First line number in the buffer */
  if(!Y) FirstLine=(ScanLines212? 8:18)+VAdjust;

  /* Return 0 if we've run out of the screen buffer due to overscan */
  if(Y+FirstLine>=HEIGHT) return(0);

  /* Set up the transparent color */
  XPal[0]=(!BGColor||SolidColor0)? XPal0:XPal[BGColor];

  /* Start of the buffer */
  P=(pixel *)XBuf;

  /* Paint top of the screen */
  if(!Y) for(H=WIDTH*FirstLine-1;H>=0;H--) P[H]=C;

  /* Start of the line */
  P+=WIDTH*(FirstLine+Y);

  /* Paint left/right borders */
  for(H=(WIDTH-256)/2+HAdjust;H>0;H--) P[H-1]=C;
  for(H=(WIDTH-256)/2-HAdjust;H>0;H--) P[WIDTH-H]=C;

  /* Paint bottom of the screen */
  H=ScanLines212? 212:192;
  if(Y==H-1) for(H=WIDTH*(HEIGHT-H-FirstLine+1)-1;H>=WIDTH;H--) P[H]=C;

  /* Return pointer to the scanline in XBuf */
  return(P+(WIDTH-256)/2+HAdjust);
}
pixel *RefreshBorder480(register byte Y,register pixel C)
{
  register pixel *P;

  /* First line number in the buffer */
  if(!Y) FirstLine=(ScanLines212? 8:18)+VAdjust;

  /* Return 0 if we've run out of the screen buffer due to overscan */
  if(Y+FirstLine>=HEIGHT) return(0);

  /* Set up the transparent color */
  XPal[0]=(!BGColor||SolidColor0)? XPal0:XPal[BGColor];

  /* Start of the buffer */
  P=(pixel *)XBuf;

  /* Start of the line */
  P+=480*(Y);


  /* Return pointer to the scanline in XBuf */
  return P;
}
pixel *RefreshBorder384(register byte Y,register pixel C)
{
  register pixel *P;

  /* First line number in the buffer */
  if(!Y) FirstLine=(ScanLines212? 8:18)+VAdjust;

  /* Return 0 if we've run out of the screen buffer due to overscan */
  if(Y+FirstLine>=HEIGHT) return(0);

  /* Set up the transparent color */
  XPal[0]=(!BGColor||SolidColor0)? XPal0:XPal[BGColor];

  /* Start of the buffer */
  P=(pixel *)XBuf;

  /* Start of the line */
  P+=384*(Y);


  /* Return pointer to the scanline in XBuf */
  return P;
}

/** Sprites() ************************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** sprites in SCREENs 1-3.                                 **/
/*************************************************************/
void Sprites(register byte Y,register pixel *Line)
{
  register pixel *P,C;
  register byte H,*PT,*AT;
  register unsigned int M;
  register int L,K;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;
  Y+=VScroll;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;    /* Iterating through SprTab      */
    K=AT[0];            /* K = sprite Y coordinate       */
    if(K==208) break;   /* Iteration terminates if Y=208 */
    if(K>256-H) K-=256; /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE1 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==MAXSPRITE1) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      C=AT[3];                  /* C = sprite attributes */
      L=C&0x80? AT[1]-32:AT[1]; /* Sprite may be shifted left by 32 */
      C&=0x0F;                  /* C = sprite color */

      if((L<256)&&(L>-H)&&C)
      {
        K=AT[0];                /* K = sprite Y coordinate */
        if(K>256-H) K-=256;     /* Y coordinate may be negative */

        P=Line+L;
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+Y-K-1;
        C=XPal[C];

        /* Mask 1: clip left sprite boundary */
        K=L>=0? 0x0FFFF:(0x10000>>-L)-1;
        /* Mask 2: clip right sprite boundary */
        if(L>256-H) K^=((0x00200>>(H-8))<<(L-257+H))-1;
        /* Get and clip the sprite data */
        K&=((int)PT[0]<<8)|(H>8? PT[16]:0x00);

        /* Draw left 8 pixels of the sprite */
        if(K&0xFF00)
        {
          if(K&0x8000) P[0]=C;if(K&0x4000) P[1]=C;
          if(K&0x2000) P[2]=C;if(K&0x1000) P[3]=C;
          if(K&0x0800) P[4]=C;if(K&0x0400) P[5]=C;
          if(K&0x0200) P[6]=C;if(K&0x0100) P[7]=C;
        }

        /* Draw right 8 pixels of the sprite */
        if(K&0x00FF)
        {
          if(K&0x0080) P[8]=C; if(K&0x0040) P[9]=C;
          if(K&0x0020) P[10]=C;if(K&0x0010) P[11]=C;
          if(K&0x0008) P[12]=C;if(K&0x0004) P[13]=C;
          if(K&0x0002) P[14]=C;if(K&0x0001) P[15]=C;
        }
      }
    }
}

/** ColorSprites() *******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** color sprites in SCREENs 4-8. The result is returned in **/
/** ZBuf, whose size must be 304 bytes (32+256+16).         **/
/*************************************************************/
void ColorSprites(register byte Y,byte *ZBuf)
{
  register byte C,H,J,OrThem;
  register byte *P,*PT,*AT;
  register int L,K;
  register unsigned int M;

  /* Clear ZBuffer and exit if sprites are off */
  _memset(ZBuf+32,0,256);
  if(SpritesOFF) return;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;
  OrThem=0x00;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;          /* Iterating through SprTab      */
    K=AT[0];                  /* Read Y from SprTab            */
    if(K==216) break;         /* Iteration terminates if Y=216 */
    K=(byte)(K-VScroll);      /* Sprite's actual Y coordinate  */
    if(K>256-H) K-=256;       /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE2 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==MAXSPRITE2) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      K=(byte)(AT[0]-VScroll); /* K = sprite Y coordinate */
      if(K>256-H) K-=256;      /* Y coordinate may be negative */

      J=Y-K-1;
      C=SprTab[-0x0200+((AT-SprTab)<<2)+J];
      OrThem|=C&0x40;

      if(C&0x0F)
      {
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+J;
        P=ZBuf+AT[1]+(C&0x80? 0:32);
        C&=0x0F;
        J=PT[0];

        if(OrThem&0x20)
        {
          if(J&0x80) P[0]|=C;if(J&0x40) P[1]|=C;
          if(J&0x20) P[2]|=C;if(J&0x10) P[3]|=C;
          if(J&0x08) P[4]|=C;if(J&0x04) P[5]|=C;
          if(J&0x02) P[6]|=C;if(J&0x01) P[7]|=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[ 8]|=C;if(J&0x40) P[ 9]|=C;
            if(J&0x20) P[10]|=C;if(J&0x10) P[11]|=C;
            if(J&0x08) P[12]|=C;if(J&0x04) P[13]|=C;
            if(J&0x02) P[14]|=C;if(J&0x01) P[15]|=C;
          }
        }
        else
        {
          if(J&0x80) P[0]=C;if(J&0x40) P[1]=C;
          if(J&0x20) P[2]=C;if(J&0x10) P[3]=C;
          if(J&0x08) P[4]=C;if(J&0x04) P[5]=C;
          if(J&0x02) P[6]=C;if(J&0x01) P[7]=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[ 8]=C;if(J&0x40) P[ 9]=C;
            if(J&0x20) P[10]=C;if(J&0x10) P[11]=C;
            if(J&0x08) P[12]=C;if(J&0x04) P[13]=C;
            if(J&0x02) P[14]=C;if(J&0x01) P[15]=C;
          }
        }
      }

      /* Update overlapping flag */
      OrThem>>=1;
    }
}
void ColorSprites480(register byte Y,byte *ZBuf)
{
  register byte C,H,J,OrThem;
  register byte *P,*PT,*AT;
  register int L,K;
  register unsigned int M;

  /* Clear ZBuffer and exit if sprites are off */
  _memset(ZBuf+32,0,512);
  if(SpritesOFF) return;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;
  OrThem=0x00;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;          /* Iterating through SprTab      */
    K=AT[0];                  /* Read Y from SprTab            */
    if(K==216) break;         /* Iteration terminates if Y=216 */
    K=(byte)(K-VScroll);      /* Sprite's actual Y coordinate  */
    if(K>256-H) K-=256;       /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE2 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==MAXSPRITE2) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      K=(byte)(AT[0]-VScroll); /* K = sprite Y coordinate */
      if(K>256-H) K-=256;      /* Y coordinate may be negative */

      J=Y-K-1;
      C=SprTab[-0x0200+((AT-SprTab)<<2)+J];
      OrThem|=C&0x40;

      if(C&0x0F)
      {
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+J;
        P=ZBuf+AT[1]+(C&0x80? 0:32);
        C&=0x0F;
        J=PT[0];

        if(OrThem&0x20)
        {
          if(J&0x80) P[0]|=C;if(J&0x40) P[1]|=C;
          if(J&0x20) P[2]|=C;if(J&0x10) P[3]|=C;
          if(J&0x08) P[4]|=C;if(J&0x04) P[5]|=C;
          if(J&0x02) P[6]|=C;if(J&0x01) P[7]|=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[ 8]|=C;if(J&0x40) P[ 9]|=C;
            if(J&0x20) P[10]|=C;if(J&0x10) P[11]|=C;
            if(J&0x08) P[12]|=C;if(J&0x04) P[13]|=C;
            if(J&0x02) P[14]|=C;if(J&0x01) P[15]|=C;
          }
        }
        else
        {
          if(J&0x80) P[0]=C;if(J&0x40) P[1]=C;
          if(J&0x20) P[2]=C;if(J&0x10) P[3]=C;
          if(J&0x08) P[4]=C;if(J&0x04) P[5]=C;
          if(J&0x02) P[6]=C;if(J&0x01) P[7]=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[ 8]=C;if(J&0x40) P[ 9]=C;
            if(J&0x20) P[10]=C;if(J&0x10) P[11]=C;
            if(J&0x08) P[12]=C;if(J&0x04) P[13]=C;
            if(J&0x02) P[14]=C;if(J&0x01) P[15]=C;
          }
        }
      }

      /* Update overlapping flag */
      OrThem>>=1;
    }
}

/** RefreshLineF() *******************************************/
/** Dummy refresh function called for non-existing screens. **/
/*************************************************************/
void RefreshLineF(register byte Y)
{
  register pixel *P;

  if(Verbose>1)
    _printf
    (
      "ScrMODE %d: ChrTab=%X ChrGen=%X ColTab=%X SprTab=%X SprGen=%X\n",
      ScrMode,ChrTab-VRAM,ChrGen-VRAM,ColTab-VRAM,SprTab-VRAM,SprGen-VRAM
    );

  P=RefreshBorder(Y,XPal[BGColor]);
  if(P) ClearLine(P,XPal[BGColor]);
}

/** RefreshLine0() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN0.                 **/
/*************************************************************/
void RefreshLine0(register byte Y)
{
  register pixel *P,FC,BC;
  register byte X,*T,*G;


  BC=XPal[BGColor];
  P=RefreshBorder(Y,BC);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BC);
  else
  {
    P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=P[8]=BC;

    G=(UseFont&&FontBuf? FontBuf:ChrGen)+((Y+VScroll)&0x07);
    T=ChrTab+40*(Y>>3);
    FC=XPal[FGColor];
    P+=9;

    for(X=0;X<40;X++,T++,P+=6)
    {
      Y=G[(int)*T<<3];
      P[0]=Y&0x80? FC:BC;P[1]=Y&0x40? FC:BC;
      P[2]=Y&0x20? FC:BC;P[3]=Y&0x10? FC:BC;
      P[4]=Y&0x08? FC:BC;P[5]=Y&0x04? FC:BC;
    }

    P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=BC;
  }
}

/** RefreshLine1() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN1, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine1(register byte Y)
{
  register pixel FC,BC;
  register byte K,X,*T,*G;
  register unsigned int *P;

  P=(void*)RefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) ClearLine((void*)P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    G=(UseFont&&FontBuf? FontBuf:ChrGen)+(Y&0x07);
    T=ChrTab+((int)(Y&0xF8)<<2);

    for(X=0;X<32;X++,T++)
    {
      K=ColTab[*T>>3];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=G[(int)*T<<3];
      *P++=(K&0x80? FC:BC)+((K&0x40? FC:BC)<<16);
      *P++=(K&0x20? FC:BC)+((K&0x10? FC:BC)<<16);
      *P++=(K&0x08? FC:BC)+((K&0x04? FC:BC)<<16);
      *P++=(K&0x02? FC:BC)+((K&0x01? FC:BC)<<16);
/*
      P[0]=K&0x80? FC:BC;P[1]=K&0x40? FC:BC;
      P[2]=K&0x20? FC:BC;P[3]=K&0x10? FC:BC;
      P[4]=K&0x08? FC:BC;P[5]=K&0x04? FC:BC;
      P[6]=K&0x02? FC:BC;P[7]=K&0x01? FC:BC;
*/
    }

    if(!SpritesOFF) Sprites(Y,(short*)P-(256));
  }
}

/** RefreshLine2() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN2, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine2(register byte Y)
{
  register pixel FC,BC;
  register byte K,X,*T;
  register int I,J;
  register unsigned int *P;

  P=(void*)RefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) ClearLine((void*)P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    I=((int)(Y&0xC0)<<5)+(Y&0x07);

    for(X=0;X<32;X++,T++)
    {
      J=(int)*T<<3;
      K=ColTab[(I+J)&ColTabM];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=ChrGen[(I+J)&ChrGenM];

      *P++=(K&0x80? FC:BC)+((K&0x40? FC:BC)<<16);
      *P++=(K&0x20? FC:BC)+((K&0x10? FC:BC)<<16);
      *P++=(K&0x08? FC:BC)+((K&0x04? FC:BC)<<16);
      *P++=(K&0x02? FC:BC)+((K&0x01? FC:BC)<<16);
/*
      P[0]=K&0x80? FC:BC;P[1]=K&0x40? FC:BC;
      P[2]=K&0x20? FC:BC;P[3]=K&0x10? FC:BC;
      P[4]=K&0x08? FC:BC;P[5]=K&0x04? FC:BC;
      P[6]=K&0x02? FC:BC;P[7]=K&0x01? FC:BC;
*/
    }

    if(!SpritesOFF) Sprites(Y,(short*)P-(256));
  }
}

/** RefreshLine3() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN3, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine3(register byte Y)
{
  register pixel *P;
  register byte X,K,*T,*G;

  P=RefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) ClearLine((void*)P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    G=ChrGen+((Y&0x1C)>>2);

    for(X=0;X<32;X++,T++,P+=8)
    {
      K=G[(int)*T<<3];
      P[0]=P[1]=P[2]=P[3]=XPal[K>>4];
      P[4]=P[5]=P[6]=P[7]=XPal[K&0x0F];
    }

    if(!SpritesOFF) Sprites(Y,(short*)P-(256));
  }
}

/** RefreshLine4() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN4, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine4(register byte Y)
{
  register pixel FC,BC;
  register byte K,X,C,*T,*R;
  register int I,J;
  byte ZBuf[304];
	unsigned int *Ps;

  Ps=(void*)RefreshBorder(Y,XPal[BGColor]);
  if(!Ps) return;
  if(!ScreenON) ClearLine((void*)Ps,XPal[BGColor]);
  else{
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    I=((int)(Y&0xC0)<<5)+(Y&0x07);
	if( (int)Ps&3 ){
		register unsigned short *P=(void*)Ps;

    for(X=0;X<32;X++,R+=8,P+=8,T++)
    {
      J=(int)*T<<3;
      K=ColTab[(I+J)&ColTabM];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=ChrGen[(I+J)&ChrGenM];

      C=R[0];P[0]=C? XPal[C]:(K&0x80)? FC:BC;
      C=R[1];P[1]=C? XPal[C]:(K&0x40)? FC:BC;
      C=R[2];P[2]=C? XPal[C]:(K&0x20)? FC:BC;
      C=R[3];P[3]=C? XPal[C]:(K&0x10)? FC:BC;
      C=R[4];P[4]=C? XPal[C]:(K&0x08)? FC:BC;
      C=R[5];P[5]=C? XPal[C]:(K&0x04)? FC:BC;
      C=R[6];P[6]=C? XPal[C]:(K&0x02)? FC:BC;
      C=R[7];P[7]=C? XPal[C]:(K&0x01)? FC:BC;
    }

	}else{
		register unsigned int *P=Ps,s;

    for(X=0;X<32;X++,R+=8,T++)
    {
      J=(int)*T<<3;
      K=ColTab[(I+J)&ColTabM];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=ChrGen[(I+J)&ChrGenM];

      C=R[0];       s=C? XPal[C]:(K&0x80)? FC:BC;
      C=R[1];*P++=s+((C? XPal[C]:(K&0x40)? FC:BC)<<16);
      C=R[2];       s=C? XPal[C]:(K&0x20)? FC:BC;
      C=R[3];*P++=s+((C? XPal[C]:(K&0x10)? FC:BC)<<16);
      C=R[4];       s=C? XPal[C]:(K&0x08)? FC:BC;
      C=R[5];*P++=s+((C? XPal[C]:(K&0x04)? FC:BC)<<16);
      C=R[6];       s=C? XPal[C]:(K&0x02)? FC:BC;
      C=R[7];*P++=s+((C? XPal[C]:(K&0x01)? FC:BC)<<16);
    }

	}
  }
}

/** RefreshLine5() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN5, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine5(register byte Y)
{
  register byte I,X,*T,*R;
  byte ZBuf[304];
  unsigned int *Ps;

  Ps=(void*)RefreshBorder(Y,XPal[BGColor]);
  if(!Ps) return;
	if( (int)Ps&3 ){
	  register unsigned short *P=(void*)Ps;
	  if(!ScreenON) ClearLine((void*)P,XPal[BGColor]);
	  else {
	    ColorSprites(Y,ZBuf);
	    R=ZBuf+32;
	    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);
	
	    for(X=0;X<16;X++,R+=16,P+=16,T+=8)
	    {
	      I=R[0];P[0]=XPal[I? I:T[0]>>4];
	      I=R[1];P[1]=XPal[I? I:T[0]&0x0F];
	      I=R[2];P[2]=XPal[I? I:T[1]>>4];
	      I=R[3];P[3]=XPal[I? I:T[1]&0x0F];
	      I=R[4];P[4]=XPal[I? I:T[2]>>4];
	      I=R[5];P[5]=XPal[I? I:T[2]&0x0F];
	      I=R[6];P[6]=XPal[I? I:T[3]>>4];
	      I=R[7];P[7]=XPal[I? I:T[3]&0x0F];
	      I=R[8];P[8]=XPal[I? I:T[4]>>4];
	      I=R[9];P[9]=XPal[I? I:T[4]&0x0F];
	      I=R[10];P[10]=XPal[I? I:T[5]>>4];
	      I=R[11];P[11]=XPal[I? I:T[5]&0x0F];
	      I=R[12];P[12]=XPal[I? I:T[6]>>4];
	      I=R[13];P[13]=XPal[I? I:T[6]&0x0F];
	      I=R[14];P[14]=XPal[I? I:T[7]>>4];
	      I=R[15];P[15]=XPal[I? I:T[7]&0x0F];
	    }
	  }
	}else{
	  register unsigned int *P=Ps,s;
	  if(!ScreenON) ClearLine((void*)P,XPal[BGColor]);
	  else {
	    ColorSprites(Y,ZBuf);
	    R=ZBuf+32;
	    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);
	
	    for(X=0;X<16;X++,R+=16,T+=8)
	    {
	      I=R[0];       s=XPal[I? I:T[0]>>4];
	      I=R[1];*P++=s+((XPal[I? I:T[0]&0x0F])<<16);
	      I=R[2];       s=XPal[I? I:T[1]>>4];
	      I=R[3];*P++=s+((XPal[I? I:T[1]&0x0F])<<16);
	      I=R[4];       s=XPal[I? I:T[2]>>4];
	      I=R[5];*P++=s+((XPal[I? I:T[2]&0x0F])<<16);
	      I=R[6];       s=XPal[I? I:T[3]>>4];
	      I=R[7];*P++=s+((XPal[I? I:T[3]&0x0F])<<16);
	      I=R[8];       s=XPal[I? I:T[4]>>4];
	      I=R[9];*P++=s+((XPal[I? I:T[4]&0x0F])<<16);
	      I=R[10];       s=XPal[I? I:T[5]>>4];
	      I=R[11];*P++=s+((XPal[I? I:T[5]&0x0F])<<16);
	      I=R[12];       s=XPal[I? I:T[6]>>4];
	      I=R[13];*P++=s+((XPal[I? I:T[6]&0x0F])<<16);
	      I=R[14];       s=XPal[I? I:T[7]>>4];
	      I=R[15];*P++=s+((XPal[I? I:T[7]&0x0F])<<16);
	    }
	  }
	}
}

/** RefreshLine8() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN8, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine8(register byte Y)
{
  static byte SprToScr[16] =
  {
    0x00,0x02,0x10,0x12,0x80,0x82,0x90,0x92,
    0x49,0x4B,0x59,0x5B,0xC9,0xCB,0xD9,0xDB
  };
  register byte C,X,*T,*R;
  byte ZBuf[304];
  register unsigned int *P,s;

  P=(void*)RefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) ClearLine((void*)P,BPal[VDP[7]]);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    for(X=0;X<32;X++,T+=8,R+=8)
    {
      C=R[0];       s=BPal[C? SprToScr[C]:T[0]];
      C=R[1];*P++=s+((BPal[C? SprToScr[C]:T[1]])<<16);
      C=R[2];       s=BPal[C? SprToScr[C]:T[2]];
      C=R[3];*P++=s+((BPal[C? SprToScr[C]:T[3]])<<16);
      C=R[4];       s=BPal[C? SprToScr[C]:T[4]];
      C=R[5];*P++=s+((BPal[C? SprToScr[C]:T[5]])<<16);
      C=R[6];       s=BPal[C? SprToScr[C]:T[6]];
      C=R[7];*P++=s+((BPal[C? SprToScr[C]:T[7]])<<16);
    }
  }
}

/** RefreshLine10() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN10/11, including   **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine10(register byte Y)
{
  register pixel *P;
  register byte C,X,*T,*R;
  register int J,K;
  byte ZBuf[304];

  P=RefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BPal[VDP[7]]);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    /* Draw first 4 pixels */
    C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
    C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
    C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
    C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
    R+=4;P+=4;

    for(X=0;X<63;X++,T+=4,R+=4,P+=4)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];Y=T[0]>>3;P[0]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[1];Y=T[1]>>3;P[1]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[2];Y=T[2]>>3;P[2]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[3];Y=T[3]>>3;P[3]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
    }
  }
}

/** RefreshLine12() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN12, including      **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine12(register byte Y)
{
  register pixel *P;
  register byte C,X,*T,*R;
  register int J,K;
  byte ZBuf[304];

  P=RefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BPal[VDP[7]]);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    if(HScroll512&&(HScroll>255)) T=(byte *)((int)T^0x10000);
    T+=HScroll&0xFC;

    /* Draw first 4 pixels */
    C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
    C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
    C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
    C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
    R+=4;P+=4;

    for(X=1;X<64;X++,T+=4,R+=4,P+=4)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];P[0]=C? XPal[C]:YJKColor(T[0]>>3,J,K);
      C=R[1];P[1]=C? XPal[C]:YJKColor(T[1]>>3,J,K);
      C=R[2];P[2]=C? XPal[C]:YJKColor(T[2]>>3,J,K);
      C=R[3];P[3]=C? XPal[C]:YJKColor(T[3]>>3,J,K);
    }
  }
}

//#ifdef NARROW

/** RefreshLine6() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN6, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
int	kasaneawase(int c1,int c2)
{
	return
			((((c1&0x7c007c00)+(c2&0x7c007c00))>> 1)&0x7c007c00)+
			((((c1&0x001f001f)+(c2&0x001f001f))>> 1)&0x001f001f)+
			((((c1&0x03e003e0)+(c2&0x03e003e0))>> 1)&0x03e003e0);
	//	(((((c1    )&31)+((c2    )&31))>>1)    )+
	//	(((((c1>> 5)&31)+((c2>> 5)&31))>>1)<< 5)+
	//	(((((c1>>10)   )+((c2>>10)   ))>>1)<<10);
}
void RefreshLine6(register byte Y)
{
  register pixel *P;
  register byte X,*T,*R,C;
  byte ZBuf[304];

  P=RefreshBorder(Y,XPal[BGColor&0x03]);
  if(!P) return;

	if(!ScreenON){
		ClearLine(P,XPal[BGColor&0x03]);
	}else{
		ColorSprites(Y,ZBuf);
		R=ZBuf+32;
		T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);

		for(X=0;X<32;X++){
	      C=R[0];P[0]=kasaneawase(XPal[C? C: T[0]>>6]      ,XPal[C? C:(T[0]>>4)&0x03]);
	      C=R[1];P[1]=kasaneawase(XPal[C? C:(T[0]>>2)&0x03],XPal[C? C:(T[0]   )&0x03]);
	      C=R[2];P[2]=kasaneawase(XPal[C? C: T[1]>>6]      ,XPal[C? C:(T[1]>>4)&0x03]);
	      C=R[3];P[3]=kasaneawase(XPal[C? C:(T[1]>>2)&0x03],XPal[C? C:(T[1]   )&0x03]);
	      C=R[4];P[4]=kasaneawase(XPal[C? C: T[2]>>6]      ,XPal[C? C:(T[2]>>4)&0x03]);
	      C=R[5];P[5]=kasaneawase(XPal[C? C:(T[2]>>2)&0x03],XPal[C? C:(T[2]   )&0x03]);
	      C=R[6];P[6]=kasaneawase(XPal[C? C: T[3]>>6]      ,XPal[C? C:(T[3]>>4)&0x03]);
	      C=R[7];P[7]=kasaneawase(XPal[C? C:(T[3]>>2)&0x03],XPal[C? C:(T[3]   )&0x03]);
	      R+=8;P+=8;T+=4;
		}
	}
}
  
/** RefreshLine7() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN7, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine7(register byte Y)
{

	if(!ScreenON){
		register pixel *P;
		P=RefreshBorder(Y,XPal[BGColor]);
		if(!P) return;
		ClearLine(P,XPal[BGColor]);
	}else
	if( softkeyboard_f || !ini_data.videomode || !ini_data.screen7 ){
if(VDP[9]&12)
	{
		register unsigned int *P;
		register byte C,X;
		register unsigned int	s1,t1,*T1,r,*R,*T2,t2,s2;
		byte ZBuf[304];
		P=(void *)RefreshBorder(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=(void *)ZBuf+32;
		T1=(void *)ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);
		T2=(void *)ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF)-65536;

	    for(X=0;X<32;X++) {
			t1=*T1++;	t2=*T2++;
			r=*R++;
      C=(r    )&255; s1= kasaneawase(XPal[C? C:(t1>> 4)&15],XPal[C? C:(t1    )&15]);
					 s2= kasaneawase(XPal[C? C:(t2>> 4)&15],XPal[C? C:(t2    )&15]);
      C=(r>> 8)&255;s1+=(kasaneawase(XPal[C? C:(t1>>12)&15],XPal[C? C:(t1>> 8)&15])<<16);
					s2+=(kasaneawase(XPal[C? C:(t2>>12)&15],XPal[C? C:(t2>> 8)&15])<<16);
			*P++=	((((s1&0x7c007c00)+(s2&0x7c007c00))>> 1)&0x7c007c00)+
					((((s1&0x001f001f)+(s2&0x001f001f))>> 1)&0x001f001f)+
					((((s1&0x03e003e0)+(s2&0x03e003e0))>> 1)&0x03e003e0);
      C=(r>>16)&255; s1= kasaneawase(XPal[C? C:(t1>>20)&15],XPal[C? C:(t1>>16)&15]);
					 s2= kasaneawase(XPal[C? C:(t2>>20)&15],XPal[C? C:(t2>>16)&15]);
      C=(r>>24)&255;s1+=(kasaneawase(XPal[C? C:(t1>>28)&15],XPal[C? C:(t1>>24)&15])<<16);
					s2+=(kasaneawase(XPal[C? C:(t2>>28)&15],XPal[C? C:(t2>>24)&15])<<16);
			*P++=	((((s1&0x7c007c00)+(s2&0x7c007c00))>> 1)&0x7c007c00)+
					((((s1&0x001f001f)+(s2&0x001f001f))>> 1)&0x001f001f)+
					((((s1&0x03e003e0)+(s2&0x03e003e0))>> 1)&0x03e003e0);
			t1=*T1++;	t2=*T2++;
			r=*R++;
      C=(r    )&255; s1= kasaneawase(XPal[C? C:(t1>> 4)&15],XPal[C? C:(t1    )&15]);
					 s2= kasaneawase(XPal[C? C:(t2>> 4)&15],XPal[C? C:(t2    )&15]);
      C=(r>> 8)&255;s1+=(kasaneawase(XPal[C? C:(t1>>12)&15],XPal[C? C:(t1>> 8)&15])<<16);
					s2+=(kasaneawase(XPal[C? C:(t2>>12)&15],XPal[C? C:(t2>> 8)&15])<<16);
			*P++=	((((s1&0x7c007c00)+(s2&0x7c007c00))>> 1)&0x7c007c00)+
					((((s1&0x001f001f)+(s2&0x001f001f))>> 1)&0x001f001f)+
					((((s1&0x03e003e0)+(s2&0x03e003e0))>> 1)&0x03e003e0);
      C=(r>>16)&255; s1= kasaneawase(XPal[C? C:(t1>>20)&15],XPal[C? C:(t1>>16)&15]);
					 s2= kasaneawase(XPal[C? C:(t2>>20)&15],XPal[C? C:(t2>>16)&15]);
      C=(r>>24)&255;s1+=(kasaneawase(XPal[C? C:(t1>>28)&15],XPal[C? C:(t1>>24)&15])<<16);
					s2+=(kasaneawase(XPal[C? C:(t2>>28)&15],XPal[C? C:(t2>>24)&15])<<16);
			*P++=	((((s1&0x7c007c00)+(s2&0x7c007c00))>> 1)&0x7c007c00)+
					((((s1&0x001f001f)+(s2&0x001f001f))>> 1)&0x001f001f)+
					((((s1&0x03e003e0)+(s2&0x03e003e0))>> 1)&0x03e003e0);
		}
	}
else
	{
		register unsigned int *P;
		register byte C,X;
		register unsigned int	s,t,*T,r,*R;
		byte ZBuf[304];
		P=(void *)RefreshBorder(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=(void *)ZBuf+32;
		T=(void *)ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);
//	20
//	21
//	23
//	23
	    for(X=0;X<32;X++) {
			t=*T++;
			r=*R++;
      C=(r    )&255;     s= kasaneawase(XPal[C? C:(t>> 4)&15],XPal[C? C:(t    )&15]);
      C=(r>> 8)&255;*P++=s+(kasaneawase(XPal[C? C:(t>>12)&15],XPal[C? C:(t>> 8)&15])<<16);
      C=(r>>16)&255;     s= kasaneawase(XPal[C? C:(t>>20)&15],XPal[C? C:(t>>16)&15]);
      C=(r>>24)    ;*P++=s+(kasaneawase(XPal[C? C:(t>>28)   ],XPal[C? C:(t>>24)&15])<<16);
			t=*T++;
			r=*R++;
	  C=(r    )&255;     s= kasaneawase(XPal[C? C:(t>> 4)&15],XPal[C? C:(t    )&15]);
      C=(r>> 8)&255;*P++=s+(kasaneawase(XPal[C? C:(t>>12)&15],XPal[C? C:(t>> 8)&15])<<16);
      C=(r>>16)&255;     s= kasaneawase(XPal[C? C:(t>>20)&15],XPal[C? C:(t>>16)&15]);
      C=(r>>24)    ;*P++=s+(kasaneawase(XPal[C? C:(t>>28)   ],XPal[C? C:(t>>24)&15])<<16);
/*			
      C=R[0];     s= kasaneawase(XPal[C? C:T[0]>>4],XPal[C? C:T[0]&15]);
      C=R[1];*P++=s+(kasaneawase(XPal[C? C:T[1]>>4],XPal[C? C:T[1]&15])<<16);
      C=R[2];     s= kasaneawase(XPal[C? C:T[2]>>4],XPal[C? C:T[2]&15]);
      C=R[3];*P++=s+(kasaneawase(XPal[C? C:T[3]>>4],XPal[C? C:T[3]&15])<<16);
      C=R[4];     s= kasaneawase(XPal[C? C:T[4]>>4],XPal[C? C:T[4]&15]);
      C=R[5];*P++=s+(kasaneawase(XPal[C? C:T[5]>>4],XPal[C? C:T[5]&15])<<16);
      C=R[6];     s= kasaneawase(XPal[C? C:T[6]>>4],XPal[C? C:T[6]&15]);
      C=R[7];*P++=s+(kasaneawase(XPal[C? C:T[7]>>4],XPal[C? C:T[7]&15])<<16);
*/
		//	P+=8;
		//	R+=8;
		//	T+=8;
		}
	}
	}else
	if( ini_data.videomode>2 ){
if(VDP[9]&12){
		register pixel *P;
		register byte C,X,*T1,*T2,*R;
		byte ZBuf[512+64];
		P = RefreshBorder480(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=ZBuf+32;
		T1=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);
		T2=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF)-65536;

	    for(X=0;X<32;X++) {
      C=R[ 0];P[ 0]=kasaneawase(XPal[C? C:T1[0]>>4],XPal[C? C:T2[0]>>4]);
      C=R[ 0];P[ 1]=kasaneawase(XPal[C? C:T1[0]&15],XPal[C? C:T2[0]&15]);
      C=R[ 1];P[ 2]=kasaneawase(XPal[C? C:T1[1]>>4],XPal[C? C:T2[1]>>4]);
      C=R[ 1];P[ 3]=kasaneawase(XPal[C? C:T1[1]&15],XPal[C? C:T2[1]&15]);
      C=R[ 2];P[ 4]=kasaneawase(XPal[C? C:T1[2]>>4],XPal[C? C:T2[2]>>4]);
      C=R[ 2];P[ 5]=kasaneawase(XPal[C? C:T1[2]&15],XPal[C? C:T2[2]&15]);
      C=R[ 3];P[ 6]=kasaneawase(XPal[C? C:T1[3]>>4],XPal[C? C:T2[3]>>4]);
      C=R[ 3];P[ 7]=kasaneawase(XPal[C? C:T1[3]&15],XPal[C? C:T2[3]&15]);
      C=R[ 4];P[ 8]=kasaneawase(XPal[C? C:T1[4]>>4],XPal[C? C:T2[4]>>4]);
      C=R[ 4];P[ 9]=kasaneawase(XPal[C? C:T1[4]&15],XPal[C? C:T2[4]&15]);
      C=R[ 5];P[10]=kasaneawase(XPal[C? C:T1[5]>>4],XPal[C? C:T2[5]>>4]);
      C=R[ 5];P[11]=kasaneawase(XPal[C? C:T1[5]&15],XPal[C? C:T2[5]&15]);
      C=R[ 6];P[12]=kasaneawase(XPal[C? C:T1[6]>>4],XPal[C? C:T2[6]>>4]);
      C=R[ 6];P[13]=kasaneawase(XPal[C? C:T1[6]&15],XPal[C? C:T2[6]&15]);
      C=R[ 7];P[14]=kasaneawase(kasaneawase(XPal[C? C:T1[7]>>4],XPal[C? C:T1[7]&15]),kasaneawase(XPal[C? C:T2[7]>>4],XPal[C? C:T2[7]&15]));
			R+=8;P+=15;T1+=8;T2+=8;
		}
}else{
		register pixel *P;
		register byte C,X,*T,*R;
		byte ZBuf[512+64];
		P = RefreshBorder480(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=ZBuf+32;
		T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

	    for(X=0;X<32;X++) {
      C=R[ 0];P[ 0]=XPal[C? C:T[0]>>4];
      C=R[ 0];P[ 1]=XPal[C? C:T[0]&15];
      C=R[ 1];P[ 2]=XPal[C? C:T[1]>>4];
      C=R[ 1];P[ 3]=XPal[C? C:T[1]&15];
      C=R[ 2];P[ 4]=XPal[C? C:T[2]>>4];
      C=R[ 2];P[ 5]=XPal[C? C:T[2]&15];
      C=R[ 3];P[ 6]=XPal[C? C:T[3]>>4];
      C=R[ 3];P[ 7]=XPal[C? C:T[3]&15];
      C=R[ 4];P[ 8]=XPal[C? C:T[4]>>4];
      C=R[ 4];P[ 9]=XPal[C? C:T[4]&15];
      C=R[ 5];P[10]=XPal[C? C:T[5]>>4];
      C=R[ 5];P[11]=XPal[C? C:T[5]&15];
      C=R[ 6];P[12]=XPal[C? C:T[6]>>4];
      C=R[ 6];P[13]=XPal[C? C:T[6]&15];
      C=R[ 7];P[14]=kasaneawase(XPal[C? C:T[7]>>4],XPal[C? C:T[7]&15]);
			R+=8;P+=15;T+=8;
		}
}
	}else{
if(VDP[9]&12){
		register byte C,X,*T1,*T2,*R;
		register unsigned int	s1,s2,*P;
		byte ZBuf[512+64];
		P = (void *)RefreshBorder384(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=ZBuf+32;
		T1=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);
		T2=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF)-65536;

	    for(X=0;X<32;X++) {
      C=R[ 0];   s1=   XPal[C? C:T1[0]>>4];   s2=   XPal[C? C:T2[0]>>4];
      C=R[ 0];*P++=kasaneawase( s1+(XPal[C? C:T1[0]&15]<<16),s2+(XPal[C? C:T2[0]&15]<<16) );
      C=R[ 1];   s1=   kasaneawase(XPal[C? C:T1[1]>>4],XPal[C? C:T1[1]&15]); s2= kasaneawase(XPal[C? C:T2[1]>>4],XPal[C? C:T2[1]&15]) ;
      C=R[ 2];*P++=kasaneawase( s1+(XPal[C? C:T1[2]>>4]<<16),s2+(XPal[C? C:T2[2]>>4]<<16) );
      C=R[ 2];   s1=   XPal[C? C:T1[2]&15];   s2=   XPal[C? C:T2[2]&15];
      C=R[ 3];*P++=kasaneawase( s1+(kasaneawase(XPal[C? C:T1[3]>>4],XPal[C? C:T1[3]&15])<<16), s2+(kasaneawase(XPal[C? C:T2[3]>>4],XPal[C? C:T2[3]&15])<<16) );
      C=R[ 4];   s1=   XPal[C? C:T1[4]>>4];   s2=   XPal[C? C:T2[4]>>4];
      C=R[ 4];*P++=kasaneawase( s1+(XPal[C? C:T1[4]&15]<<16),s2+(XPal[C? C:T2[4]&15]<<16) );
      C=R[ 5];   s1=   kasaneawase(XPal[C? C:T1[5]>>4],XPal[C? C:T1[5]&15]); s2= kasaneawase(XPal[C? C:T2[5]>>4],XPal[C? C:T2[5]&15]) ;
      C=R[ 6];*P++=kasaneawase( s1+(XPal[C? C:T1[6]>>4]<<16),s2+(XPal[C? C:T2[6]>>4]<<16) );
      C=R[ 6];   s1=   XPal[C? C:T1[6]&15];   s2=   XPal[C? C:T2[6]&15];
      C=R[ 7];*P++=kasaneawase( s1+(kasaneawase(XPal[C? C:T1[7]>>4],XPal[C? C:T1[7]&15])<<16), s2+(kasaneawase(XPal[C? C:T2[7]>>4],XPal[C? C:T2[7]&15])<<16) );
			R+=8;
			T1+=8;
			T2+=8;
		}
}else{
		register byte C,X,*T,*R;
		register unsigned int	s,*P;
		byte ZBuf[512+64];
		P = (void *)RefreshBorder384(Y,XPal[BGColor]);
		if(!P) return;
		ColorSprites(Y,ZBuf);
		R=ZBuf+32;
		T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

	    for(X=0;X<32;X++) {
      C=R[ 0];   s=   XPal[C? C:T[0]>>4];
      C=R[ 0];*P++=s+(XPal[C? C:T[0]&15]<<16);
      C=R[ 1];   s=   kasaneawase(XPal[C? C:T[1]>>4],XPal[C? C:T[1]&15]);
      C=R[ 2];*P++=s+(XPal[C? C:T[2]>>4]<<16);
      C=R[ 2];   s=   XPal[C? C:T[2]&15];
      C=R[ 3];*P++=s+(kasaneawase(XPal[C? C:T[3]>>4],XPal[C? C:T[3]&15])<<16);
      C=R[ 4];   s=   XPal[C? C:T[4]>>4];
      C=R[ 4];*P++=s+(XPal[C? C:T[4]&15]<<16);
      C=R[ 5];   s=   kasaneawase(XPal[C? C:T[5]>>4],XPal[C? C:T[5]&15]);
      C=R[ 6];*P++=s+(XPal[C? C:T[6]>>4]<<16);
      C=R[ 6];   s=   XPal[C? C:T[6]&15];
      C=R[ 7];*P++=s+(kasaneawase(XPal[C? C:T[7]>>4],XPal[C? C:T[7]&15])<<16);
			R+=8;
		//	P+=12;
			T+=8;
		}
}
	}
}

/** RefreshLineTx80() ****************************************/
/** Refresh line Y (0..191/211) of TEXT80.                  **/
/*************************************************************/
void RefreshLineTx80(register byte Y)
{
  register pixel *P,FC,BC;
  register byte X,M,*T,*C,*G;

  BC=XPal[BGColor];
  P=RefreshBorder(Y,BC);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BC);
  else
  {
    P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=P[8]=BC;
    G=(UseFont&&FontBuf? FontBuf:ChrGen)+((Y+VScroll)&0x07);
    T=ChrTab+((80*(Y>>3))&ChrTabM);
    C=ColTab+((10*(Y>>3))&ColTabM);
    P+=9;

    for(X=0,M=0x00;X<80;X++,T++,P+=3)
    {
      if(!(X&0x07)) M=*C++;
      if(M&0x80) { FC=XPal[XFGColor];BC=XPal[XBGColor]; }
      else       { FC=XPal[FGColor];BC=XPal[BGColor]; }
      M<<=1;
      Y=*(G+((int)*T<<3));
      P[0]=Y&0xC0? FC:BC;
      P[1]=Y&0x30? FC:BC;
      P[2]=Y&0x0C? FC:BC;
    }

    P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=XPal[BGColor];
  }
}

//#endif /* NARROW */
