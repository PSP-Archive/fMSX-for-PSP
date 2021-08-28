

byte DiskPresent(byte ID);
byte DiskRead(byte ID,byte *Buf,int N);
byte DiskWrite(byte ID,byte *Buf,int N);
byte ChangeDisk(byte ID,char *Name);


