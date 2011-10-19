/*

Warning: ad-hoc code, not really intended for reuse.
Use it any way you want (Public Domain).
Just don't force mee to look at it again!

*/

#include <SDL/SDL.h>
#include <stdint.h>
#include <math.h>

SDL_PixelFormat*fmt;
SDL_Surface*s;
SDL_Overlay*o;

#define WIDTH 320
#define HEIGHT 240

#define LWIDTH 512
#define LHEIGHT 256

#define XMARGIN ((LWIDTH-WIDTH)/2)
#define YMARGIN ((LHEIGHT-HEIGHT)/2)

uint8_t pixels[LWIDTH*LHEIGHT]; // use 256x256 logical size to prevent clipping?

void writeToTextScreen(uint8_t*d,uint8_t*s, int attrs,int progress);

#define SWAP(a,b) {int t=a;a=b;b=t;}

uint8_t romPalette[16]=
{20,244,47,152,95,128,28,133, 70,135,143,186,140,202,124,228};
//12

uint8_t romCharset[128*8];

void renderTextChar(uint8_t*d,uint8_t*s,uint8_t fg,uint8_t bg,int zoom)
{
  int y,x;
  for(y=0;y<8;y++)
  {
    int bitmap=s[y>>(zoom&1)];
    uint8_t*dd=d+LWIDTH*y;
    if(!(zoom&2))
    for(x=0;x<8;x++)
    {
      dd[x]=(bitmap&128)?fg:bg;
      bitmap<<=1;
    }
    else
    for(x=0;x<8;x++)
    {
      dd[x*2]=dd[x*2+1]=(bitmap&128)?fg:bg;
      bitmap<<=1;
    }
  }
}

void renderTextScreen(uint8_t*s,uint8_t*cs,uint8_t*pal)
{
  int x,y;
  uint8_t*d=pixels+XMARGIN+LWIDTH*(YMARGIN-3);
  for(y=0;y<31;y++){ int lineattr=s[1]&128?1:0;
  int linelgt=40>>lineattr;
  for(x=0;x<linelgt;x++)
  {
    int idx = *s++;
    int fg = *s&15;
    int bg = *s>>4;    
    uint8_t*dd = d+y*8*LWIDTH+x*(8<<lineattr); //((lineattr&1)?16:8);
    s++;
    if(!(idx&128))
       renderTextChar(dd,romCharset+idx*8,
                      pal[bg],pal[fg],0|(lineattr*2)); //|((bg>>3)));
    else
       renderTextChar(dd,romCharset+(idx&127)*8+((y&1)*4),
                      pal[bg],pal[fg],1|(lineattr*2)); // |((bg>>3)));
  }
  if(lineattr)s+=40;
  }
}

uint8_t greenseries[]={
 11,  3, 10, 43,  2, 35,  9, 42,
  1, 34,  8, 41,  0, 33, 66, 40,
 73, 32, 65, 72,105, 64, 97,104,
 96,129,136,128,192,161,224,224,224
};

#if (0)
#define SPOT0 252
#define SPOT1 244
#endif

#if (0)
#define GRAY0 13
#define GRAY1 6
#define GRAY2 14
#define GRAY3 7
#define GRAY4 15
#define GRAY5 39
#define GRAY6 47
#define GRAY7 110
#endif

#define GRAY0 5
#define GRAY1 45
#define GRAY2 77
#define GRAY3 109
#define GRAY4 141
#define GRAY5 173
#define GRAY6 205
#define GRAY7 237

// 213 246 253 245 142 174 206
#define SPOT0 142
#define SPOT1 174
#define SPOT2 206

char*lines[16],*nlines[16];
int linestat[16];

void plotDotPart(int x,int y)
{
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT)
  {
    uint8_t*d=pixels+XMARGIN+x+LWIDTH*(YMARGIN+y);
    if(*d==GRAY7 || *d==SPOT0 || *d==SPOT1 || *d==SPOT2) return;
    else
    if(*d==GRAY6) *d=GRAY7;
    else
    if(*d==GRAY5) *d=GRAY6;
    else
    if(*d==GRAY4) *d=GRAY5;
    else
    if(*d==GRAY3) *d=GRAY4;
    else
    if(*d==GRAY2) *d=GRAY3;
    else
    if(*d==GRAY1) *d=GRAY2;
    else
    if(*d==GRAY0) *d=GRAY1;
    else
    *d=GRAY0;
  }
}

void plotDotSpot(int x,int y)
{
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT)
  {
    uint8_t*d=pixels+XMARGIN+x+LWIDTH*(YMARGIN+y);
    if(*d==SPOT2) return;
    else
    if(*d==SPOT1) *d=SPOT2;
    else
    if(*d==SPOT0) *d=SPOT1;
    else
    {
      //if(*d!=GRAY7 && *d!=GRAY6) plotDotPart(x,y);
      //else
      *d=SPOT0;
    }
  }
}

int rdr_t=0,rdr_t0=0;
int startpoint=0;
int switchpoint=87040;
int switchlgt=20000;
int currentformula=0;
int samplerate=8000;

unsigned char aubuf[1470];

void presentFormula(int f)
{
  switch(f)
  {
    case(1):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="(t*5&t>>7)|(t*3&t>>10)";
      nlines[2]=nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
      nlines[10]=NULL;
      nlines[11]="From: viznut";
      nlines[12]=NULL;
      break;
    case(2):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="(int)(t/1e7*t*t+t)%127";
      nlines[2]="|t>>4|t>>5|t%127+(t>>16)|t";
      nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
//      nlines[10]=NULL;
      nlines[11]="From: bst";
      nlines[12]=NULL;
      break;
    case(3):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="((t/2*(15&(0x234568a0>>(t>>8&28))))";
      nlines[2]="|t/2>>(t>>11)^t>>12)+(t/16&t&24)";
      nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
//      nlines[10]=NULL;
      nlines[11]="From: kb";
      nlines[12]="44.1 kHz output rate";
      break;
    case(4): 
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="(t&t%255)-(t*3&t>>13&t>>6)";
      nlines[2]=nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
//      nlines[10]=NULL;
      nlines[11]="From: viznut";
      nlines[12]=NULL;
      break;
    case(5):
      nlines[0]="JS/AS expression:";
      nlines[1]="t>>4|t&(t>>5)";
      nlines[2]="/(t>>7-(t>>15)&-t>>7-(t>>15))";
      nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=NULL;
      nlines[10]=NULL;
      nlines[11]="From: droid";
      nlines[12]="In C, guard against divide-by-zero.";
      break;
    case(6):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="((t*(\"36364689\"[t>>13&7]&15))/12&128)";
      nlines[2]="+(((((t>>12)^(t>>12)-2)%11*t)/4";
      nlines[3]="|t>>13)&127)";
      nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
      nlines[10]=NULL;
      nlines[11]="From: ryg";
      nlines[12]="44.1 kHz output rate";
      break;      
    case(7): 
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="(t*9&t>>4|t*5&t>>7|t*3&t/1024)-1";
      nlines[2]=NULL;
      nlines[3]=NULL;
      nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
      nlines[10]=NULL;
      nlines[11]="From: stephth";
      nlines[12]="Percussion (-1) added by viznut";
      break;      
    case(8):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="((t*(t>>12)&(201*t/100)&(199*t/100))";
      nlines[2]="&(t*(t>>14)&(t*301/100)&(t*399/100)))";
      nlines[3]="+((t*(t>>16)&(t*202/100)&(t*198/100))";
      nlines[4]="-(t*(t>>17)&(t*302/100)&(t*298/100)))";
      nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
      nlines[10]="a.k.a. \"Dante's Inferno\"";
      nlines[11]="From: viznut";
      nlines[12]="by modifying oasiz's formula";
      break;
    case(9):
      nlines[0]="Javascript expression:";
      nlines[1]="w=t>>9,k=32,m=2048,a=1-t/m%1,d=(14*t*t";
      nlines[2]="^t)%m*a,y=[3,3,4.7,2][p=w/k&3]*t/4,h=";
      nlines[3]="\"IQNNNN!!]]!Q!IW]WQNN??!!W]WQNNN?\"";
      nlines[4]=".charCodeAt(w/2&15|p/3<<4)/33*t-t,s=y*";
      nlines[5]=".98%80+y%80+(w>>7&&a*((5*t%m*a&128)*(";
      nlines[6]="0x53232323>>w/4&1)+(d&127)*(0xa444c444";
      nlines[7]=">>w/4&1)*1.5+(d*w&1)+(h%k+h*1.99%k+h*.4";
      nlines[8]="9%k+h*.97%k-64)*(4-a-a))),s*s>>14?127:s";
      nlines[9]=NULL;
      nlines[10]="Covers Chaos Theory by Conspiracy.";
      nlines[11]="From: mu6k";
      nlines[12]="Optimized by ryg, p01 et al.";
      break;
    case(10):
      nlines[0]="main(t){for(;;t++)putchar(";
      nlines[1]="t*(t^t+(t>>15|1)^(t-1280^t)>>10)";
      nlines[2]=nlines[3]=nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=");}";
      nlines[10]=NULL;
      nlines[11]="From: 216";
      nlines[12]=NULL;
      break;
    case(11):
      nlines[0]="Javascript expression:";
      nlines[1]="(3e3/(y=t&16383)&1)*35";
      nlines[2]="+(x=t*\"6689\"[t>>16&3]/24&127)*y/4e4";
      nlines[3]="+((t>>8^t>>10|t>>14|x)&63)";
      nlines[4]=nlines[5]=nlines[6]=nlines[7]=nlines[8]=nlines[9]=NULL;
      nlines[9]=NULL;
      nlines[10]=NULL;
      nlines[11]="From: mu6k";
      nlines[12]="Optimized by ryg; 32 kHz";
      break;      
    default:
      {int i;for(i=0;i<13;i++)nlines[i]=NULL;}
  }
}

void initFormula(int f)
{
  switch(f)
  {
    case(0): switchpoint=87040; switchlgt=20000; break;
    case(1): switchpoint += ((34*8000)&~255); switchlgt=20000; break;
    case(2): switchpoint += ((20*8000)&~255); switchlgt=20000; break;      
    case(3): switchpoint += ((28*44100)&~1023);
      switchlgt=100000;
      break;
    case(4): switchpoint += ((50*8000)&~255); switchlgt=20000; break;
    case(5): switchpoint += ((34*8000)&~255); switchlgt=20000; 
      switchpoint&=~1023;
      break;
    case(6): switchpoint += ((30*44100)&~1023);
      switchlgt=100000; break;
    case(7): switchpoint += ((38*8000)&~255); switchlgt=20000; break;
    case(8): switchpoint += ((70*8000)&~255); switchlgt=20000; break;
    case(9): switchpoint += ((30*8000)&~255); switchlgt=20000; break;
    case(10): switchpoint += ((50*8000)&~255); switchlgt=20000; 
      switchpoint&=~1023; break;
    case(11): switchpoint += ((40*32000)&~255); switchlgt=100000; break;
    case(12): exit(0);
    default: switchpoint+=1000000; break;
  }
}

int DIV(int a,int b)
{
  if(b==0) return 0;
  return a/b;
}

unsigned char rygtune[2000000];
unsigned char mu6ktune[2000000];

int getFromFormula(int f,int t)
{
  switch(f)
  {
    case(0): samplerate=8000; return 0;
    case(1): samplerate=8000; return (t*5&t>>7)|(t*3&t>>10);
    case(2): samplerate=8000; return (int)(t/1e7*t*t+t)%127
      |t>>4|t>>5|t%127+(t>>16)|t;
    case(3): samplerate=44100;
      return((t/2*(15&(0x234568a0>>(t>>8&28))))|t/2>>(t>>11)^t>>12)+(t/16&t&24);
    case(4): samplerate=8000; return (t&t%255)-(t*3&t>>13&t>>6);
    case(5): samplerate=8000;
      return t>>4|t&DIV((t>>5),(t>>7-(t>>15)&-t>>7-(t>>15)));
    case(6): samplerate=44100;
      return ((t*("36364689"[t>>13&7]&15))/12&128)
      +(((((t>>12)^(t>>12)-2)%11*t)/4
      |t>>13)&127);
    case(7): samplerate=8000;
      return (t*9&t>>4|t*5&t>>7|t*3&t/1024)-1;
    case(8): samplerate=8000;
      t+=16000;
      return   ((t*(t>>12)&(201*t/100)&(199*t/100))
        &(t*(t>>14)&(t*301/100)&(t*399/100)))
          +((t*(t>>16)&(t*202/100)&(t*198/100))
            -(t*(t>>17)&(t*302/100)&(t*298/100)));
    case(9): samplerate=8000;
      return rygtune[t];
    case(10): samplerate=8000;
      return t*(t^t+(t>>15|1)^(t-1280^t)>>10);
    case(11): samplerate=32000;
      return mu6ktune[t];

    default: samplerate=8000; return 0;
  }
}

int mystrcmp(char*a,char*b)
{
  if(a==NULL && b==NULL) return 0;
  if(a==NULL || b==NULL) return 1;
  return strcmp(a,b);
}

void renderLines(uint8_t*scr)
{
  int i,linenum,nlinenum,linemode,nlinemode;
  for(i=0;i<13;i++)
  {
    linenum=nlinenum=17;
    linemode=nlinemode=0xF0;
    if(i>=1 && i<=8)
    {
      if(!lines[5])
      {
        linenum=19+2*(i-1);
        linemode=0x1F0;
      } else linenum=19+i-1;
      if(!nlines[5])
      {
        nlinenum=19+2*(i-1);
        nlinemode=0x1F0;
      } else nlinenum=19+i-1;
    }
    if(i==9 || i==10)
    {
      if(lines[4]) linenum=26+(i-9)*1; else
      if(lines[3]) linenum=24+(i-9)*2; else
      if(lines[2]) linenum=22+(i-9)*2; else
      linenum=20+(i-9)*2;

      if(nlines[4]) nlinenum=26+(i-9)*1; else
      if(nlines[3]) nlinenum=24+(i-9)*2; else
      if(nlines[2]) nlinenum=22+(i-9)*2; else
      nlinenum=20+(i-9)*2;
    }
    if(i>=11) linenum=nlinenum=27+i-10;
    if(mystrcmp(lines[i],nlines[i]))
    {
      if(linestat[i]<40)
      {
        if(lines[i])
        writeToTextScreen(scr+(linenum*40+1)*2,lines[i],linemode,40-linestat[i]);
      }
      else
      {
        if(nlines[i])
        writeToTextScreen(scr+(nlinenum*40+1)*2,nlines[i],nlinemode,linestat[i]-40);
      }
      if(linestat[i]<80) linestat[i]++;
      else
      {
        lines[i]=nlines[i];
      }
    }
    else
    {
      if(lines[i])
      writeToTextScreen(scr+(nlinenum*40+1)*2,lines[i],linemode,80);
      linestat[i]=0;
    }
  }
}

void resetReader()
{
  if(rdr_t0>=switchpoint)
  {
    currentformula++;
    startpoint = switchpoint;
    initFormula(currentformula);
  }
  rdr_t=rdr_t0;
}

int readSample()
{
  int t=rdr_t-startpoint;
  int a;
  
  if(rdr_t>=switchpoint)
  {
    t=rdr_t-switchpoint;
    a=getFromFormula(currentformula+1,t);
  }
  else
  {
    a=getFromFormula(currentformula,t);

  if(rdr_t>=switchpoint-switchlgt)
  {
    int v=((switchpoint-rdr_t)*1023)/switchlgt;
    v-=768;
    if(v<0)v=0;
    if(v>255)v=255;
    a&=255;
    a=(v*a)>>8;
  }
  }

  rdr_t++;
  return a&255;
}

void plotDot(int x,int y)
{
  plotDotPart(x/2,y/2);
  plotDotPart(x/2,y/2+1);
  plotDotPart(x/2+1,y/2);
  plotDotPart(x/2+1,y/2+1);
  plotDotSpot(x/2+(x&1),y/2+(y&1));
}

int dithermask[8]={0,6,2,4,1,7,3,5};

int aur_ctr=-1;
int aur_smp0=0;
int aur_smp1=0;
int au_t=0;
void renderAudio(int t0)
{
  int i,a;
  rdr_t0=t0;

  resetReader();
  if(samplerate==22050 || samplerate==44100) aur_ctr=-1;
  for(i=0;i<1470;i++)
  {
    if(aur_ctr<=0)
    {
      aur_ctr+=44100;
      aur_smp0=aur_smp1;
      aur_smp1=readSample();
    }
    a=((aur_ctr*aur_smp1)+((44100-aur_ctr)*aur_smp0))/44100;
    aur_ctr-=samplerate;
    //fputc(aur_smp0,stderr);
  }
  au_t=rdr_t;
}

void renderSongVisualization(int t0)
{
  uint8_t*d=pixels+XMARGIN+LWIDTH*YMARGIN;
  int x,y,t,py=-1;

  rdr_t0=t0;

  if(rdr_t>=switchpoint-switchlgt)
     presentFormula(currentformula+1);

  resetReader();
  {
  int ss=256,step;
  if(samplerate>=16000) ss=512; else
  if(samplerate>=32000) ss=1024;
  rdr_t=startpoint+((rdr_t-startpoint)&~(ss-1));
  step=(ss/128)-1;
  for(x=0;x<318;x++)
  {
  for(y=0;y<128;y++)
  {
    int a=readSample();

    a+=dithermask[y&7];
    if(a>255)a=255;
    
    d[LWIDTH*y+x] = greenseries[(a>>3)&31];
    if(samplerate>=16000) rdr_t++;
    if(samplerate>=32000) rdr_t+=2;
  }
  rdr_t|=ss-1;rdr_t++;
  }
  }
  resetReader();
  y=0;
  for(x=0;x<318;x++)
  {
    int sz=(64*samplerate)/60;
    int step=2-1;
    for(y=0;y<sz;y+=64)
    {
      int a=readSample();
      a^=255;
       plotDot(x*2+(y&64?1:0),a);
       y+=(samplerate-8000)/(8000/64);
       rdr_t+=(samplerate-8000)/8000;
    }
  }

  {  
  int step=256;
  if(samplerate>=16000) step=512; else
  if(samplerate>=32000) step=1024;
  resetReader();
  rdr_t=startpoint+((rdr_t-startpoint)&~(step-1));
  for(x=0;x<318;x++)
  {
  int y0,y1;
    int a=readSample();
    rdr_t+=(samplerate-8000)/8000;
    a&=255;
    a/=2;

    y0=a-4;
    y1=a;
    if(py>=0)
    {
      if(py<y0)y0=py;
      if(py>y1)y1=py;
    }
    if(y0<0)y0=0;
    py=a;

    for(y=y0;y<y1;y++)
    {
      int c=219;
      if(y==y0 || y==y1-1) c=19;
      d[LWIDTH*(128-y)+x-1]=c;
      d[LWIDTH*(128-y)+x]=c;
      d[LWIDTH*(128-y)+x+1]=19;
    }
  }
  }
  for(x=0;x<320;x++)
    d[LWIDTH*128+x]=d[(LWIDTH+1)*128+x]=19;
}

void writeToTextScreen(uint8_t*d,uint8_t*s, int attrs,int progress)
{
  int i=0;
  while(*s)
  {
    int a=*s;
    if(i>=progress) break;
    if(i+4>=progress) a^=(rand()&rand()&127);
    if(attrs&256){a|=128;d[-40*2]=a;d[-40*2+1]=attrs;}
    d[0]=a;d[1]=attrs;
    d+=2;
    s++;
    i++;
  }
}

 
/////////////////////////////////////////////////////////////////////////

void makeTestStuff666()
{
  static uint8_t scr[40*31*2];
  static int t=0;
  {
    int i;
    int pam=0xF0;
    for(i=0;i<40*31*2;i+=2) { scr[i+1]=pam; scr[i]=32; }
    if(t>9*30)
    for(i=0;i<40*31*2;i+=40) { scr[i+1]=0x70; }
  }

  if(t<5*30)
  {int p=t;
   if(t>5*30-64)p=5*30-t;
  writeToTextScreen(scr+(19*40+1)*2,  "Hello there!",0x1F0,p);
  writeToTextScreen(scr+(23*40+1)*2,  "Another bunch of",0x1F0,p);
  writeToTextScreen(scr+(25*40+1)*2,  "very short",0x1F0,p);
  writeToTextScreen(scr+(27*40+1)*2,  "musical programs",0x1F0,p);
  writeToTextScreen(scr+(29*40+1)*2,  "coming up.",0x1F0,p);
  } else
  if(t<9*30)
  {int p=t-5*30;
   if(t>9*30-64)p=9*30-t;
  writeToTextScreen(scr+(23*40+1)*2,  "Now with",0x1F0,p);
  writeToTextScreen(scr+(27*40+1)*2,  "visualization!",0x1F0,p);
  }
  
  if(t==15*30)
  {
    nlines[10]="Visualization key:";
  }
  if(t==20*30)
  {
    nlines[10]="Background/green = pixel per sample";
  }
  if(t==25*30)
  {
    nlines[10]="Middle/red = y-dot per sample";
  }
  if(t==30*30)
  {
    nlines[10]="Front/blue = local wave (~1/30 s)";
  }
  if(t==35*30)
  {
    nlines[10]=NULL;
  }
  if(t==50*30)
  {
    nlines[10]="All the programs output 8-kHz U8 -";
  }
  if(t==55*30)
  {
    nlines[10]="- unless otherwise specified.";
  }
  if(t==60*30)
  {
    nlines[10]="All of them are compatible -";
  }
  if(t==65*30)
  {
    nlines[10]="- with the on-line JS tool.";
  }
  if(t==70*30)
  {
    nlines[10]="Most also with C and the Flash tool.";
  }
  if(t==75*30)
  {
    nlines[10]="I have taken the liberty -";
  }
  if(t==80*30)
  {
    nlines[10]="- to slightly modify JS/AS formulas -";
  }
  if(t==85*30)
  {
    nlines[10]="- to make them C-compatible.";
  }
  if(t==90*30)
  {
    nlines[10]=NULL;
  }

  renderLines(scr);
  renderTextScreen(scr,romCharset,romPalette);
  renderSongVisualization(au_t);
  renderAudio(au_t);
  t++;
}

void dumpYuvFrame(SDL_Overlay*o)
{
  static int isfirst=1;
  int x,y;
  if(isfirst)
  {
    printf("YUV4MPEG2 W%d H%d F%d:%d Ip A0:0 C420mpeg2 XYSCSS=420MPEG2\n",
      WIDTH*2,
      HEIGHT*2,
      30,1);
    isfirst=0;
  }
  printf("FRAME\n");

  for(y=0;y<HEIGHT*2;y++)
  for(x=0;x<WIDTH;x++)
  {
    char*oo=(char*)(o->pixels[0])+y*WIDTH*4+x*4;
    putchar(oo[0]);
    putchar(oo[2]);
  }

  for(y=0;y<HEIGHT*2;y+=2)
  for(x=0;x<WIDTH;x++)
  {
    char*oo=(char*)(o->pixels[0])+y*WIDTH*4+x*4;
    putchar(oo[1]);
  }

  for(y=0;y<HEIGHT*2;y+=2)
  for(x=0;x<WIDTH;x++)
  {
    char*oo=(char*)(o->pixels[0])+y*WIDTH*4+x*4;
    putchar(oo[3]);
  }
}

void scaleToYUY2()
{
  int x,y,prevY=0,prevU=0,prevV=0;
  int err = 0;
  static int t = 0;
  int h=0;
  int bw=0;
  h=(t&1023)>>2; t++;
  if(t&1024)h^=0xff;
  t++;
  h=7;

  for(y=0;y<HEIGHT-1;y++)
  for(x=0;x<WIDTH;x++)
  {
    int c = pixels[(y+YMARGIN)*LWIDTH+x+XMARGIN], r = 0xFFFF;
    char*oo=(char*)(o->pixels[0])+y*2*WIDTH*4+x*4 + err;
    int cY = c ^ (r&0x1f),
        cU = (c<<3) ^ (r&0x1f), // | (c>>5),
        cV = (c<<5) ^ (r&0x1f); // | (c>>3);
    if(bw) cU=cV=0x80;
    if((r&255)<h) { cU=prevU; cV=prevV; } else r=0; //&=rand(); // colorspill
    oo[0] = prevY;
    oo[1] = prevU; // ^ (r&0x1f);
    oo[2] = (cY ^ ((r>>1)&0x1f));
    oo[3] = prevV; // ^ (r&0x1f);
    oo[0+WIDTH*4] = (prevY / 2)|0;
    oo[1+WIDTH*4] = (prevU+cU) / 2; ///2; // cU ^ (r&0x1f);
    oo[2+WIDTH*4] = (prevY / 2)|0; prevY = (cY ^ ((r>>8)&0x1f)); // (cY ^ (r&0x7));
    oo[3+WIDTH*4] = prevV; // (prevV+cV) / 2; // /2; // cV ^ (r&0x1f);

    prevU = cU; // ^ (r&0x1f);
    prevV = cV; // ^ (r&0x1f);
  }
}

main()
{
  int j=0xff;
  int x,y;
  SDL_Init(SDL_INIT_VIDEO);
  s=SDL_SetVideoMode(1024,768,32,0);
  o=SDL_CreateYUVOverlay(WIDTH*2,HEIGHT*2,SDL_YUY2_OVERLAY,s);

  x=0;

  for(;;)
  {
    int i; j++;
    SDL_Rect area={0,0,1024,768};
    SDL_Event e;
    SDL_PollEvent(&e);
    if(e.type==SDL_KEYDOWN) break;

    makeTestStuff666();
    scaleToYUY2();

    SDL_DisplayYUVOverlay(o,&area);
    //dumpYuvFrame(o);
  }
  
  SDL_Quit();
}
