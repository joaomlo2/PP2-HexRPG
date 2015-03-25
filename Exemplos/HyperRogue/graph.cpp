#define GL
#define GL_GLEXT_PROTOTYPES

// #define PANDORA

// Hyperbolic Rogue
// Copyright (C) 2011-2012 Zeno Rogue, see 'hyper.cpp' for details

// basic graphics:

// disable this if you have no access to SDL_gfx
#ifndef STEAM
#define GFX
#endif

// scale the Euclidean
#define EUCSCALE 2.3

// disable this if you have no access to SDL_mixer
#ifndef MOBILE
#define AUDIO
#endif

#define BTOFF 0x404040
#define BTON  0xC0C000


#ifndef MOBILE
#include <SDL/SDL.h>

#ifdef AUDIO
#include <SDL/SDL_mixer.h>
#endif
bool audio;
int audiovolume = 60;

#ifndef MAC
#undef main
#endif

#include <SDL/SDL_ttf.h>

#ifdef GFX
#include <SDL/SDL_gfxPrimitives.h>
#endif
#endif

#ifndef MOBILE
#ifdef GL
#ifdef MAC
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef MAC
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif
#endif
#endif

#ifdef ANDROID
#ifndef FAKE
#ifdef GL
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#endif
#endif
#endif


#ifndef MOBILE

// x resolutions

#define NUMMODES 7

SDL_Surface *s;
TTF_Font *font[256];
SDL_Joystick *stick, *panstick;

#endif

// R:239, G:208, B:207 

struct videopar {
  ld scale, eye, alpha, aspeed;
  bool full;
  bool goteyes;
  bool quick;
  
  int xres, yres, framelimit;
  
  int xscr, yscr;
  
  // paramaters calculated from the above
  int xcenter, ycenter;
  int radius;
  ld alphax, beta;
  
  int fsize;
  int flashtime;
  
  int wallmode, monmode, axes;

  // for OpenGL
  float scrdist;
  
  bool usingGL;
  bool usingAA;

  int joyvalue, joyvalue2, joypanthreshold;
  float joypanspeed;
  
  bool female;
  int language;
  int boardmode;
  
  int skincolor, haircolor, dresscolor, swordcolor;
  };

int skincolors[]  = { 7, 0xD0D0D0FF, 0xEFD0C9FF, 0xC77A58FF, 0xA58869FF, 0x602010FF, 0xFFDCB1FF, 0xEDE4C8FF };
int haircolors[]  = { 8, 0x686868FF, 0x8C684AFF, 0xF2E1AEFF, 0xB55239FF, 0xFFFFFFFF, 0x804000FF, 0x502810FF, 0x301800FF };
int dresscolors[] = { 6, 0xC00000FF, 0x00C000FF, 0x0000C0FF, 0xC0C000FF, 0xC0C0C0FF, 0x202020FF };
int swordcolors[] = { 6, 0xC0C0C0FF, 0xFFFFFFFF, 0xFFC0C0FF, 0xC0C0FFFF, 0x808080FF, 0x202020FF };

// is the player using mouse? (used for auto-cross)
bool mousing = true;

// is the mouse button pressed?
bool mousepressed = false;

// 
int axestate;

int ticks;

hyperpoint ccenter; ld crad;

videopar vid;
int default_language;

int playergender() { return vid.female ? GEN_F : GEN_M; }
int lang() { 
  if(vid.language >= 0)
    return vid.language; 
  return default_language;
  }

int sightrange = 7;

cell *mouseover, *lmouseover, *centerover, *lcenterover; ld modist, centdist;
string mouseovers;

int mousex, mousey, mousedist, mousedest, joyx, joyy, joydir, panjoyx, panjoyy;
bool autojoy = true;
hyperpoint mouseh;

bool leftclick, rightclick;
bool gtouched;
bool revcontrol;

int getcstat; ld getcshift;

int ZZ;

string help;

enum emtype {emNormal, emHelp, emVisual1, emVisual2, 
  emChangeMode, emCustomizeChar,
  emQuit, emDraw, emScores, emPickEuclidean} cmode;

int andmode = 0;

int darken = 0;

#ifndef MOBILE
int& qpixel(SDL_Surface *surf, int x, int y) {
  if(x<0 || y<0 || x >= surf->w || y >= surf->h) return ZZ;
  char *p = (char*) surf->pixels;
  p += y * surf->pitch;
  int *pi = (int*) (p);
  return pi[x];
  }

int qpixel3(SDL_Surface *surf, int x, int y) {
  if(x<0 || y<0 || x >= surf->w || y >= surf->h) return ZZ;
  char *p = (char*) surf->pixels;
  p += y * surf->pitch;
  p += x;
  int *pi = (int*) (p);
  return pi[0];
  }

void loadfont(int siz) {
  if(!font[siz]) {
    font[siz] = TTF_OpenFont("DejaVuSans-Bold.ttf", siz);
    if (font[siz] == NULL) {
      printf("error: Font file not found\n");
      exit(1);
      }
    }
  }

int textwidth(int siz, const string &str) {
  if(size(str) == 0) return 0;
  
  loadfont(siz);
  
  int w, h;
  TTF_SizeText(font[siz], str.c_str(), &w, &h);
  // printf("width = %d [%d]\n", w, size(str));
  return w;
  }
#endif

#ifdef IOS

int textwidth(int siz, const string &str) {
  return mainfont->getSize(str, siz / 36.0).width;
  }

#endif

int darkened(int c) {
  for(int i=0; i<darken; i++) c &= 0xFEFEFE, c >>= 1;
  return c;
  }

int darkenedby(int c, int lev) {
  for(int i=0; i<lev; i++) c &= 0xFEFEFE, c >>= 1;
  return c;
  }

int darkena(int c, int lev, int a) {
  for(int i=0; i<lev; i++) c &= 0xFEFEFE, c >>= 1;
  return (c << 8) + a;
  }

#ifdef GL

int lalpha = 0xFF;

void glcolor(int color) {
  unsigned char *c = (unsigned char*) (&color);
  glColor4f(c[2] / 255.0, c[1] / 255.0, c[0]/255.0, lalpha / 255.0); // c[3]/255.0);
  }

void selectEyeGL(int ed) {
  float ve = ed*vid.eye;
  ve *= 2; // vid.xres; ve /= vid.radius;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float lowdepth = 0.1;
  float hidepth = 10000;
  
  // simulate glFrustum
  GLfloat frustum[16] = {
    vid.yres * 1./vid.xres, 0, 0, 0, 
    0, 1, 0, 0, 
    0, 0, -(hidepth+lowdepth)/(hidepth-lowdepth), -1,
    0, 0, -2*lowdepth*hidepth/(hidepth-lowdepth), 0};

  if(ve)
    glTranslatef(-(ve * vid.radius) * (vid.alpha - (vid.radius*1./vid.xres) * vid.eye) / vid.xres, 0, 0);

  glTranslatef((vid.xcenter*2.)/vid.xres - 1, 1 - (vid.ycenter*2.)/vid.yres, 0);

  glMultMatrixf(frustum);  

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  float sc = vid.radius / (vid.yres/2.);
  GLfloat mat[16] = {sc,0,0,0, 0,-sc,0,0, 0,0,-1,0, 0,0,-vid.alpha,1};
  glMultMatrixf(mat);
  
  if(ve) glTranslatef(ve, 0, vid.eye);

  vid.scrdist = -vid.alpha + vid.yres * sc / 2;
  }

void selectEyeMask(int ed) {
  if(ed == 0) {    
    glColorMask( GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE );
    }
  else if(ed == 1) {
    glColorMask( GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE );
    }
  else if(ed == -1) {
    glColorMask( GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE );
    }
  }

void setGLProjection() {

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);     

  selectEyeGL(0);
  }

GLfloat glcoords[1500][3];
int qglcoords;

bool GL_initialized = false;
void buildpolys();

#ifndef MOBILE

#define NUMEXTRA 49
const char* natchars[NUMEXTRA] = {
/* "°", "é",
  "ą", "ć", "ę", "ł", "ń", "ó", "ś", "ź", "ż",
  "Ą", "Ć", "Ę", "Ł", "Ń", "Ó", "Ś", "Ź", "Ż",
  "ç", "ğ", "ı", "ö", "ş", "ü", "â", "û", "î",
  "Ç", "Ğ", "İ", "Ö", "Ş", "Ü" */
  // "°", "Ç", "Ö", "Ü", "â", "ç", "é", "î", "ó", "ö", "ü", "ą", "ć", "ę", "ğ", "İ", "ı", "Ł", "ł", "ń", "Ś", "ś", "Ş", "ş", "ź", "Ż", "ż"
  
   "°", "Ç", "Í", "Ö", "Ú", "Ü", "á", "â", "ç", "é", "í", "î", "ó", "ö", "ú", "ü", "ý", "ą", "ć", "Č", "č", "Ď", "ď", "ę", "ě", "ğ", "İ", "ı", "Ł", "ł", "ń", "ň", "Ř", "ř", "Ś", "ś", "Ş", "ş", "Š", "š", "ť", "ů", "ź", "Ż", "ż", "Ž", "ž",
   "Á", "Ě"
  };


struct glfont_t {
  GLuint * textures;                                  // Holds The Texture Id's   
//GLuint list_base;                                   // Holds The First Display List ID  
  int widths[128+NUMEXTRA];
  int heights[128+NUMEXTRA];
  float tx[128+NUMEXTRA];
  float ty[128+NUMEXTRA];
  };

glfont_t *glfont[256];

void init(const char * fname, unsigned int h);

inline int next_p2 (int a )
{
    int rval=1;
    // rval<<=1 Is A Prettier Way Of Writing rval*=2;
    while(rval<a) rval<<=1;
    return rval;
}

void glError(const char* GLcall, const char* file, const int line) {
  GLenum errCode = glGetError();
  if(errCode!=GL_NO_ERROR) {
    fprintf(stderr, "OPENGL ERROR #%i: in file %s on line %i :: %s\n",errCode,file, line, GLcall);
    }
  }
#define GLERR(call) glError(call, __FILE__, __LINE__)

void init_glfont(int size) {
  if(glfont[size]) return;
  
  glfont[size] = new glfont_t;
  
  glfont_t& f(*(glfont[size]));

  f.textures = new GLuint[128+NUMEXTRA];
//f.list_base = glGenLists(128);
  glGenTextures( 128+NUMEXTRA, f.textures );

  loadfont(size);
  if(!font[size]) return;

  char str[2]; str[1] = 0;
  
  SDL_Color white;
  white.r = white.g = white.b = 255;
  
  glListBase(0);
 
  for(unsigned char ch=1;ch<128+NUMEXTRA;ch++) {
    // printf("init size=%d ch=%d\n", size, ch);
  
    SDL_Surface *txt;
    
    if(ch < 128) {
      str[0] = ch;
      txt = TTF_RenderText_Blended(font[size], str, white);
      }
    else {
      txt = TTF_RenderUTF8_Blended(font[size], natchars[ch-128], white);
      }
    if(txt == NULL) continue;
//  printf("a\n");
  
    int twidth = next_p2( txt->w );
    int theight = next_p2( txt->h );
   
    Uint16 expanded_data[twidth * theight];

    for(int j=0; j <theight;j++) for(int i=0; i < twidth; i++) {
      expanded_data[(i+j*twidth)] = 
          (i>=txt->w || j>=txt->h) ? 0 : ((qpixel(txt, i, j)>>24)&0xFF) * 0x101;
      }

//  printf("b\n");
    f.widths[ch] = txt->w;
    f.heights[ch] = txt->h;
  
    glBindTexture( GL_TEXTURE_2D, f.textures[ch]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, twidth, theight, 0,
      GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );
   
//  printf("c\n");

    float x=(float)txt->w / (float)twidth;
    float y=(float)txt->h / (float)theight;
    f.tx[ch] = x;
    f.ty[ch] = y;

/*  glNewList(f.list_base+ch,GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, f.textures[ch]);
   
    //glPushMatrix();
   
    // glTranslatef(bitmap_glyph->left,0,0);
    // glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);
   
//  printf("d\n");
  
    glBegin(GL_QUADS);
    float eps=0;
    glTexCoord2d(eps,eps); glVertex2f(0, txt->h);
    glTexCoord2d(eps,y-0); glVertex2f(0, 0);
    glTexCoord2d(x-eps,y-0); glVertex2f(txt->w, 0);
    glTexCoord2d(x-eps,0); glVertex2f(txt->w, txt->h);
    glEnd();
    glEndList(); */
    //glPopMatrix();

    SDL_FreeSurface(txt);    
    }

//printf("init size=%d ok\n", size);
  GLERR("initfont");
  }

int getnext(const char* s, int& i) {
  for(int k=0; k<NUMEXTRA; k++)
    if(s[i] == natchars[k][0] && s[i+1] == natchars[k][1]) {
      i += 2; return 128+k;
      }
  if(s[i] < 0 && s[i+1] < 0) {
    printf("Unknown character: '%c%c'\n", s[i], s[i+1]);
    i += 2; return '?';
    }
  return s[i++];
  }

bool gl_print(int x, int y, int shift, int size, const char *s, int color, int align) {

  // printf("gl_print: %s\n", s.c_str());
  
  // We Want A Coordinate System Where Distance Is Measured In Window Pixels.
  
  /*
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 0, vid.xscr, vid.yscr); */
  
  init_glfont(size);
  if(!font[size]) return false;

  glfont_t& f(*glfont[size]);
  
  glDisable(GL_LIGHTING);
  
  glEnable(GL_TEXTURE_2D);

  glMatrixMode(GL_MODELVIEW);
  
  glcolor(color);

  int tsize = 0;
  for(int i=0; s[i];) tsize += f.widths[getnext(s,i)];
  x -= tsize * align / 16;
  y += f.heights[32]/2;

  bool clicked = (mousex >= x && mousey <= y && mousex <= x+tsize && mousey >= y-f.heights[32]);
  
  for(int i=0; s[i];) {
  
    // glListBase(f.list_base);
    // glCallList(s[i]); // s[i]);
    
    int tabid = getnext(s,i);
    float fx=f.tx[tabid];
    float fy=f.ty[tabid];
    int wi = f.widths[tabid];
    int hi = f.heights[tabid];

    for(int ed = (vid.goteyes && shift)?-1:0; ed<2; ed+=2) {
      glPushMatrix();
      glTranslatef(x-ed*shift-vid.xcenter,y-vid.ycenter, vid.scrdist);
      selectEyeMask(ed);
      glBindTexture(GL_TEXTURE_2D, f.textures[tabid]);
      glBegin(GL_QUADS);
      glTexCoord2d(0,0); glVertex2f(0, -hi);
      glTexCoord2d(0,fy); glVertex2f(0, 0);
      glTexCoord2d(fx,fy); glVertex2f(wi, 0);
      glTexCoord2d(fx,0); glVertex2f(wi, -hi);
      glEnd();
      glPopMatrix();
      }
    
    if(vid.goteyes) selectEyeMask(0);
    
    GLERR("print");
    
    // int tabid = s[i];
    x += f.widths[tabid];
    
/*  
    printf("point %d,%d\n", x, y);
    glBegin(GL_POINTS);
    glVertex3f(rand() % 100 - rand() % 100, rand() % 100 - rand() % 100, 100);
    glEnd(); */

    }
 
  glDisable(GL_TEXTURE_2D);
  
  return clicked;
  // printf("gl_print ok\n");
  }
#endif

void resetGL() {
  GL_initialized = false;
#ifndef MOBILE
  for(int i=0; i<128; i++) if(glfont[i]) {
    delete glfont[i];
    glfont[i] = NULL;
    }
#endif
  buildpolys();
  }

#endif

#ifndef MOBILE
bool displaystr(int x, int y, int shift, int size, const char *str, int color, int align) {

  if(strlen(str) == 0) return false;

  if(size <= 0 || size > 255) {
    // printf("size = %d\n", size);
    return false;
    }
  
#ifdef GL
  if(vid.usingGL) return gl_print(x, y, shift, size, str, color, align);
#endif
  
  SDL_Color col;
  col.r = (color >> 16) & 255;
  col.g = (color >> 8 ) & 255;
  col.b = (color >> 0 ) & 255;
  
  col.r >>= darken; col.g >>= darken; col.b >>= darken;

  loadfont(size);

  SDL_Surface *txt = (vid.usingAA?TTF_RenderText_Blended:TTF_RenderText_Solid)(font[size], str, col);
  
  if(txt == NULL) return false;

  SDL_Rect rect;

  rect.w = txt->w;
  rect.h = txt->h;

  rect.x = x - rect.w * align / 16;
  rect.y = y - rect.h/2;
  
  bool clicked = (mousex >= rect.x && mousey >= rect.y && mousex <= rect.x+rect.w && mousey <= rect.y+rect.h);
  
  if(shift) {
    SDL_Surface* txt2 = SDL_DisplayFormat(txt);
    SDL_LockSurface(txt2);
    SDL_LockSurface(s);
    int c0 = qpixel(txt2, 0, 0);
    for(int yy=0; yy<rect.h; yy++)
    for(int xx=0; xx<rect.w; xx++) if(qpixel(txt2, xx, yy) != c0)
      qpixel(s, rect.x+xx-shift, rect.y+yy) |= color & 0xFF0000,
      qpixel(s, rect.x+xx+shift, rect.y+yy) |= color & 0x00FFFF;
    SDL_UnlockSurface(s);
    SDL_UnlockSurface(txt2);
    SDL_FreeSurface(txt2);
    }
  else {
    SDL_BlitSurface(txt, NULL, s,&rect); 
    }
  SDL_FreeSurface(txt);
  
  return clicked;
  }
                  
bool displaystr(int x, int y, int shift, int size, const string &s, int color, int align) {
  return displaystr(x, y, shift, size, s.c_str(), color, align);
  }

bool displayfr(int x, int y, int b, int size, const string &s, int color, int align) {
  displaystr(x-b, y, 0, size, s, 0, align);
  displaystr(x+b, y, 0, size, s, 0, align);
  displaystr(x, y-b, 0, size, s, 0, align);
  displaystr(x, y+b, 0, size, s, 0, align);
  return displaystr(x, y, 0, size, s, color, align);
  }

bool displaychr(int x, int y, int shift, int size, char chr, int col) {

  char buf[2];
  buf[0] = chr; buf[1] = 0;
  return displaystr(x, y, shift, size, buf, col, 8);
  }

#else

vector<int> graphdata;

void gdpush(int t) {
  graphdata.push_back(t);
  }

bool displaychr(int x, int y, int shift, int size, char chr, int col) {
  gdpush(2); gdpush(x); gdpush(y); gdpush(8);
  gdpush(col); gdpush(size); gdpush(0);
  gdpush(1); gdpush(chr); 
  return false;
  }

void gdpush_utf8(const string& s) {
  int g = graphdata.size(), q = 0;
  gdpush((int) s.size()); for(int i=0; i<s.size(); i++) {
    unsigned char uch = (unsigned char) s[i];
    if(uch >= 192 && uch < 224) {
      int u = ((s[i] - 192)&31) << 6;
      i++;
      u += (s[i] - 128) & 63;
      gdpush(u); q++;
      }
    else {
      gdpush(s[i]); q++;
      }
    }
  graphdata[g] = q;
  }

bool displayfr(int x, int y, int b, int size, const string &s, int color, int align) {
  gdpush(2); gdpush(x); gdpush(y); gdpush(align);
  gdpush(color); gdpush(size); gdpush(b);
  gdpush_utf8(s);
  int mx = x - mousex;
  int my = y - mousey;
  return 
    mx >= -3*size   && mx <= +3*size   && 
    my >= -size*3/4 && my <= +size*3/4;
  }

bool displaystr(int x, int y, int shift, int size, const string &s, int color, int align) {
  gdpush(2); gdpush(x); gdpush(y); gdpush(align);
  gdpush(color); gdpush(size); gdpush(0);
  gdpush_utf8(s);
  int mx = x - mousex;
  int my = y - mousey;
  return 
    mx >= -3*size   && mx <= +3*size   && 
    my >= -size*3/4 && my <= +size*3/4;
  }

#endif

bool displaynum(int x, int y, int shift, int size, int col, int val, string title) {
  char buf[64];
  sprintf(buf, "%d", val);
  bool b1 = displayfr(x-8, y, 1, size, buf, col, 16);
  bool b2 = displayfr(x, y, 1, size, title, col, 0);
  if((b1 || b2) && gtouched) {
    col ^= 0x00FFFF;
    displayfr(x-8, y, 1, size, buf, col, 16);
    displayfr(x, y, 1, size, title, col, 0);
    }
  return b1 || b2;
  }

vector<pair<pair<int, int>, string> > msgs;

vector<string> gamelog;

void flashMessages() {
  for(int i=0; i<size(msgs); i++) 
    if(msgs[i].first.first < ticks - 1000 && !msgs[i].first.second) {
      msgs[i].first.second = 1;
      msgs[i].first.first = ticks;
      }
  }

void addMessage(string s) {
  if(gamelog.size() == 20) {
    for(int i=0; i<19; i++) gamelog[i] = gamelog[i+1];
    gamelog[19] = s;
    }
  else gamelog.push_back(s);
  msgs.push_back(make_pair(make_pair(ticks, 0), s));
  // printf("%s\n", s.c_str());
  }

void drawmessages() {
  int i = 0;
  int t = ticks;
  for(int j=0; j<size(msgs); j++) {
    int age = t - msgs[j].first.first;
    if(msgs[j].first.second == 0) age = 0;
    if(age < 256*vid.flashtime) {
      int x = vid.xres / 2;
      int y = vid.yres - vid.fsize * (size(msgs) - j) - (ISIOS ? 4 : 0);
      displayfr(x, y, 1, vid.fsize, msgs[j].second, 0x10101 * (255 - age/vid.flashtime), 8);
      msgs[i++] = msgs[j];
      }
    }
  msgs.resize(i);
  }

hyperpoint gethyper(ld x, ld y) {
  ld hx = (x - vid.xcenter) / vid.radius;
  ld hy = (y - vid.ycenter) / vid.radius;
  
  if(euclid)
    return hpxy(hx * (EUCSCALE + vid.alphax), hy * (EUCSCALE + vid.alphax));
    
  ld hr = hx*hx+hy*hy;
  
  if(hr > .9999) return Hypc;
  
  // hz*hz-(hx/(hz+alpha))^2 - (hy/(hz+alpha))^2 =
  
  // hz*hz-hr*(hz+alpha)^2 == 1
  // hz*hz - hr*hr*hz*Hz
  
  ld A = 1-hr;
  ld B = 2*hr*vid.alphax;
  ld C = 1 + hr*vid.alphax*vid.alphax;
  
  // Az^2 - Bz = C
  B /= A; C /= A;
  
  // z^2 - Bz = C
  // z^2 - Bz + (B^2/4) = C + (B^2/4)
  // z = (B/2) + sqrt(C + B^2/4)
  
  ld hz = B / 2 + sqrt(C + B*B/4);
  hyperpoint H;
  H[0] = hx * (hz+vid.alphax);
  H[1] = hy * (hz+vid.alphax);
  H[2] = hz;
  
  return H;
  }

void getcoord(const hyperpoint& H, int& x, int& y, int &shift) {
  
  if(H[2] < 0.999) {
    printf("error: %s\n", display(H));
    // exit(1);
    }
  ld tz = euclid ? (EUCSCALE+vid.alphax) : vid.alphax+H[2];
  if(tz < 1e-3 && tz > -1e-3) tz = 1000;
  x = vid.xcenter + int(vid.radius * H[0] / tz);
  y = vid.ycenter + int(vid.radius * H[1] / tz);
#ifndef MOBILE
  shift = vid.goteyes ? int(vid.eye * vid.radius * (1 - vid.beta / tz)) : 0;
#endif
  }

int dlit;

int polyi;

#define POLYMAX 60000

#ifdef MOBILE
short polyx[POLYMAX], polyy[POLYMAX];
#else
Sint16 polyx[POLYMAX], polyxr[POLYMAX], polyy[POLYMAX];
#endif

void drawline(const hyperpoint& H1, int x1, int y1, int s1, const hyperpoint& H2, int x2, int y2, int col) {
  dlit++; if(dlit>500) return;
  
  int dst = (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);

  #ifdef GL
  if(vid.usingGL && dst <= (ISMOBILE ? 100 : 20)) {
    if(euclid) {
      for(int i=0; i<2; i++) glcoords[qglcoords][i] = H1[i]; glcoords[qglcoords][2] = EUCSCALE;
      }
    else {
      for(int i=0; i<3; i++) glcoords[qglcoords][i] = H1[i];
      }
    qglcoords++;
    return;
    }
  #endif

  #ifdef MOBILE
  if(dst <= 400 && !vid.usingGL) {
    if(polyi >= POLYMAX) return;
    polyx[polyi] = x1;
    polyy[polyi] = y1;
    polyi++;
    return;
    }
  #else
  #ifdef GFX
  if(dst <= 20 && !vid.usingGL) {
    if(col == -1) {
      if(polyi >= POLYMAX) return;
      polyx[polyi] = x1-s1;
      polyxr[polyi] = x1+s1;
      polyy[polyi] = y1;
      polyi++;
      }
    else (vid.usingAA?aalineColor:lineColor) (s, x1, y1, x2, y2, col);
    return;
    }
  #endif
  if(dst <= 2) {
    return;
    }
  #endif
  
  hyperpoint H3 = mid(H1, H2);
  int x3, y3, s3; getcoord(H3, x3, y3, s3);
  #ifndef GFX
  if(!vid.usingGL) {
    qpixel(s, x3-s3, y3) |= col & 0xFF0000;
    qpixel(s, x3+s3, y3) |= col & 0x00FFFF;
    }
  #endif

  drawline(H1, x1, y1, s1, H3, x3, y3, col);
  drawline(H3, x3, y3, s3, H2, x2, y2, col);
  }

void drawline(const hyperpoint& H1, const hyperpoint& H2, int col) {
  if(vid.usingGL) {
    qglcoords = 0;
    }

  // printf("line\n");
  if(col != -1) {
    col = (col << 8) + lalpha;
    if(col == -1) col = -2;
    polyi = 0;
    #ifndef GFX
    if(!vid.usingGL) {
      SDL_LockSurface(s);
      col >>= 8;
      }
    #endif
    }
    
  dlit = 0;
  int x1, y1, s1; getcoord(H1, x1, y1, s1);
  int x2, y2, s2; getcoord(H2, x2, y2, s2);
  drawline(H1, x1, y1, s1, H2, x2, y2, col);

  if(vid.usingGL) {

    // EUCLIDEAN
    if(euclid) {
      for(int i=0; i<2; i++) glcoords[qglcoords][i] = H2[i]; glcoords[qglcoords][2] = EUCSCALE;
      }
    else {
      for(int i=0; i<3; i++) glcoords[qglcoords][i] = H2[i];
      }
    qglcoords++;

    glVertexPointer(3, GL_FLOAT, 0, glcoords);
    glEnableClientState(GL_VERTEX_ARRAY);
    glcolor(col >> 8);
    
    if(vid.goteyes) {
      selectEyeGL(-1);
      selectEyeMask(-1);
      glDrawArrays(GL_LINE_STRIP, 0, qglcoords);
      selectEyeGL(+1);
      selectEyeMask(+1);
      glDrawArrays(GL_LINE_STRIP, 0, qglcoords);
      selectEyeGL(0);
      selectEyeMask(0);
      }
    else glDrawArrays(GL_LINE_STRIP, 0, qglcoords);
    }
  
  #ifdef MOBILE
  else if(col != -1) {
    gdpush(3); gdpush(col); 
    gdpush(polyi+1);
    for(int i=0; i<polyi; i++) gdpush(polyx[i]), gdpush(polyy[i]);
    gdpush(x2), gdpush(y2);
    }
  #endif
  #ifndef GFX
  if(col != -1) {
    SDL_UnlockSurface(s);
    }
  #endif
  // printf("ok\n");
  }

// game-related graphics

transmatrix View; // current rotation, relative to viewctr
transmatrix cwtV; // player-relative view
heptspin viewctr; // heptagon and rotation where the view is centered at
bool playerfound; // has player been found in the last drawing?

#include "geometry.cpp"

ld spina(cell *c, int dir) {
  return 2 * M_PI * dir / c->type;
  }

int gradient(int c0, int c1, ld v0, ld v, ld v1) {
  int vv = int(256 * ((v-v0) / (v1-v0)));
  int c = 0;
  for(int a=0; a<3; a++) {
    int p0 = (c0 >> (8*a)) & 255;
    int p1 = (c1 >> (8*a)) & 255;
    int p = (p0*(256-vv) + p1*vv + 127) >> 8;
    c += p << (8*a);
    }
  return c;
  }

int flashat, lightat, safetyat;

void drawFlash() { flashat = ticks; }
void drawLightning() { lightat = ticks; }
void drawSafety() { safetyat = ticks; }

#include "polygons.cpp"

bool drawplayer = true;

transmatrix playertrans;

double eurad = 0.52;

bool outofmap(hyperpoint h) {
  if(euclid) 
    return h[0] * h[0] + h[1] * h[1] > 15 * eurad;
  else
    return h[2] < .5;
  }

bool drawMonster(const transmatrix& V, int ct, cell *c, int col) {

  eMonster m = c->monst;
    
  if(c->cpdist == 0) {

    if(items[itOrbShield] > 1) {
      float ds = ticks / 300.;
      int col = darkened(iinf[itOrbShield].color);
      for(int a=0; a<84*5; a++)
        drawline(V*ddi(a, hexf + sin(ds + M_PI*2*a/20)*.1)*C0, V*ddi((a+1), hexf + sin(ds + M_PI*2*(a+1)/20)*.1)*C0, col);
      }

    if(items[itOrbSpeed]) {
      ld ds = ticks  / 10.;
      int col = darkened(iinf[itOrbSpeed].color);
      for(int b=0; b<84; b+=14)
      for(int a=0; a<84; a++)
        drawline(V*ddi(ds+b+a, hexf*a/84)*C0, V*ddi(ds+b+(a+1), hexf*(a+1)/84)*C0, col);
      }

    int ct = c->type;
    
    if(items[itOrbSafety]) {
      ld ds = ticks / 50.;
      int col = darkened(iinf[itOrbSafety].color);
      for(int a=0; a<ct; a++)
        drawline(V*ddi(ds+a*84/ct, 2*hexf)*C0, V*ddi(ds+(a+(ct-1)/2)*84/ct, 2*hexf)*C0, col);
      }

    if(items[itOrbFlash]) {
      float ds = ticks / 300.;
      int col = darkened(iinf[itOrbFlash].color);
      col &= ~1;
      for(int u=0; u<5; u++) {
        ld rad = hexf * (2.5 + .5 * sin(ds+u*.3));
        for(int a=0; a<84; a++)
          drawline(V*ddi(a, rad)*C0, V*ddi(a+1, rad)*C0, col);
        }
      }

    if(items[itOrbWinter]) {
      int hdir = 42 - cwt.spin * 84 / c->type;
      float ds = ticks / 300.;
      int col = darkened(iinf[itOrbWinter].color);
      for(int u=0; u<20; u++) {
        ld rad = 6 * sin(ds+u * 2 * M_PI / 20);
        drawline(V*ddi(hdir+rad, hexf*.5)*C0, V*ddi(hdir+rad, hexf*3)*C0, col);
        }
      }
    
    if(items[itOrbLightning] > 1) {
      int col = iinf[itOrbLightning].color;
      for(int u=0; u<20; u++) {
        ld leng = 0.5 / (0.1 + (rand() % 100) / 100.0);
        ld rad = rand() % 84;
        drawline(V*ddi(rad, hexf*0.3)*C0, V*ddi(rad, hexf*leng)*C0, col);
        }
      }
    
    if(flashat > 0) {
      int tim = ticks - flashat;
      if(tim > 1000) flashat = 0;
      for(int u=0; u<=tim; u++) {
        if((u-tim)%50) continue;
        if(u < tim-150) continue;
        ld rad = u * 3 / 1000.;
        rad = rad * (5-rad) / 2;
        rad *= hexf;
        int col = iinf[itOrbFlash].color;
        if(u > 500) col = gradient(col, 0, 500, u, 1100);
        for(int a=0; a<84; a++)
          drawline(V*ddi(a, rad)*C0, V*ddi(a+1, rad)*C0, col);
        }
      }
      
    if(safetyat > 0) {
      int tim = ticks - safetyat;
      if(tim > 2500) safetyat = 0;
      for(int u=tim; u<=2500; u++) {
        if((u-tim)%250) continue;
        ld rad = hexf * u / 250;
        int col = iinf[itOrbSafety].color;
        for(int a=0; a<84; a++)
          drawline(V*ddi(a, rad)*C0, V*ddi(a+1, rad)*C0, col);
        }
      }
      
    transmatrix cV2 = c == cwt.c ? cwtV : V;
    if(flipplayer) cV2 = cV2 * spin(M_PI);
    
    if(vid.monmode < 2) return true;
  
    bool havus = shUser[0][2].s;
    playertrans = cV2;

    for(int i=0; i<8; i++) if(shUser[i][2].s) 
      queuepoly(cV2, ct, shUser[i][2], 0xFFFFFFFF);
    
    if(drawplayer && !havus) {
      if(vid.female) {
        queuepoly(cV2, ct, shFemaleBody, vid.skincolor);
        if(items[itOrbThorns])
          queuepoly(cV2, ct, shHedgehogBladePlayer, 0x00FF00FF);
        else
          queuepoly(cV2, ct, shPSword, vid.swordcolor);
        queuepoly(cV2, ct, shFemaleHair, vid.haircolor);
        queuepoly(cV2, ct, shPFace, vid.skincolor);
        queuepoly(cV2, ct, shFemaleDress, vid.dresscolor);
        }
      else if(c->monst == moGolem) {
        queuepoly(cV2, ct, shPBody, darkena(col, 0, 0XC0));
        queuepoly(cV2, ct, shGolemhead, darkena(col, 1, 0XFF));
        }
      else {
        queuepoly(cV2, ct, shPBody, vid.skincolor);
        if(items[itOrbThorns])
          queuepoly(cV2, ct, shHedgehogBladePlayer, 0x00FF00FF);
        else
          queuepoly(cV2, ct, shPSword, vid.swordcolor);
        if(cheater) {
          queuepoly(cV2, ct, shDemon, darkena(0xFFFF00, 0, 0xFF));
          queuepoly(cV2, ct, shHood, darkena(0xFF00, 1, 0xFF));
          }
        else {
          queuepoly(cV2, ct, shPHead, vid.haircolor);
          queuepoly(cV2, ct, shPFace, vid.skincolor);
          }
        }
      }
    }

  if(isIvy(c) || isWorm(c)) {
    
    transmatrix V2 = V;
    
    if(c->mondir != NODIR) {
      int hdir = 42 - c->mondir * 84 / c->type;
      
      if(vid.monmode > 1) {
        V2 = V2 * spin(hdir * M_PI / 42);
        if(isIvy(c))
          queuepoly(V2, ct, shIBranch, (col << 8) + 0xFF);
        else if(c->monst < moTentacle) {
          queuepoly(V2, ct, shTentacleX, 0xFF);
          queuepoly(V2, ct, shTentacle, (col << 8) + 0xFF);
          }
        else {
          queuepoly(V2, ct, shTentacleX, 0xFFFFFFFF);
          queuepoly(V2, ct, shTentacle, (col << 8) + 0xFF);
          }
        }
        
      else for(int u=-1; u<=1; u++)
        drawline(V*ddi(hdir+21, u*crossf/5)*C0, V*ddi(hdir, crossf)*ddi(hdir+21, u*crossf/5)*C0, 0x606020 >> darken);
      }

    if(vid.monmode > 1) {
      if(isIvy(c)) 
        queuepoly(V, ct, shILeaf[ct-6], darkena(col, 0, 0xFF));
      else if(m == moWorm || m == moWormwait) {
        queuepoly(V2 * spin(M_PI), ct, shWormHead, darkena(col, 0, 0xFF));
        queuepoly(V2 * spin(M_PI), ct, shEyes, 0xFF);
        }
      else if(m == moTentacle || m == moTentaclewait || m == moTentacleEscaping)
        queuepoly(V2 * spin(M_PI), ct, shTentHead, darkena(col, 0, 0xFF));
      else
        queuepoly(V2, ct, shJoint, darkena(col, 0, 0xFF));
      }

    return vid.monmode < 2;
    }
  
  else if(isMimic(c)) {
  
    int hdir = 42 - c->mondir * 84 / c->type;

    transmatrix mirrortrans = Id;    
    if(c->monst == moMirror) mirrortrans[1][1] = -1;
    transmatrix V2 = V * spin(M_PI*hdir/42) * mirrortrans;
    
    if(vid.monmode > 1) {
      if(flipplayer) V2 = V2 * spin(M_PI);
      queuepoly(V2, ct, vid.female ? shFemaleBody : shPBody,  darkena(col, 0, 0X80));
      queuepoly(V2, ct, shPSword, darkena(col, 0, 0XC0));
      queuepoly(V2, ct, vid.female ? shFemaleHair : shPHead,  darkena(col, 1, 0XC0));
      queuepoly(V2, ct, shPFace,  darkena(col, 0, 0XC0));
      if(vid.female)
        queuepoly(V2, ct, shFemaleDress,  darkena(col, 1, 0XC0));
      if(flipplayer) V2 = V2 * spin(M_PI);
      }
    
    if(!outofmap(mouseh)) {
      hyperpoint P2 = V2 * inverse(cwtV) * mouseh;    
      int xc, yc, sc;
      getcoord(P2, xc, yc, sc);
      displaychr(xc, yc, sc, 10, 'x', 0xFF00);
      }
    
    return vid.monmode < 2;
    }
  
  else if(c->monst && vid.monmode < 2) return true;
  
  else if(isFriendly(c)) {
    // golems don't face player
    queuepoly(V, ct, shPBody, darkena(col, 0, 0XC0));
    queuepoly(V, ct, shGolemhead, darkena(col, 1, 0XFF));
    }

  else if(c->monst) {
  
    // face the player
    transmatrix VL;
    
    if(false) {
      // hyperpoint V0 = cwtV * C0;
      hyperpoint V1 = V * C0;
      VL = V * spin(hypot(V1[0], V1[1]));
      }
    else {
      hyperpoint V0 = inverse(cwtV) * V * C0;
      hyperpoint V1 = spintox(V0) * V0;
      VL = cwtV * rspintox(V0) * rpushxto0(V1) * spin(M_PI);
      }
    
    char xch = minf[m].glyph;

    if(m == moWolf) {
      queuepoly(VL, ct, shWolfLegs, darkena(col, 0, 0xFF));
      queuepoly(VL, ct, shWolfBody, darkena(col, 0, 0xFF));
      queuepoly(VL, ct, shWolfHead, darkena(col, 0, 0xFF));
      queuepoly(VL, ct, shWolfEyes, darkena(col, 3, 0xFF));
      }
    else if(m == moRunDog) {
      queuepoly(VL, ct, shWolf, darkena(col, 0, 0xFF));
      }
    else if(m == moShark || m == moGreaterShark)
      queuepoly(VL, ct, shShark, darkena(col, 0, 0xFF));
    else if(m == moEagle)
      queuepoly(VL, ct, shEagle, darkena(col, 0, 0xFF));
    else if(m == moZombie)
      queuepoly(VL, ct, shPBody, darkena(col, 0, 0xFF));
    else if(m == moDesertman) {
      queuepoly(VL, ct, shPBody, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shPSword, 0xFFFF00FF);
      queuepoly(VL, ct, shHood, 0xD0D000C0);
      }
    else if(m == moCrystalSage) {
      queuepoly(VL, ct, shPBody, 0xFFFFFFFF);
      queuepoly(VL, ct, shPHead, 0xFFFFFFFF);
      queuepoly(VL, ct, shPFace, 0xFFFFFFFF);
      }
    else if(m == moHedge) {
      queuepoly(VL, ct, shPBody, col);
      queuepoly(VL, ct, shHedgehogBlade, 0xC0C0C0FF);
      queuepoly(VL, ct, shPHead, 0x804000FF);
      queuepoly(VL, ct, shPFace, 0xF09000FF);
      }
    else if(m == moYeti || m == moMonkey) {
      queuepoly(VL, ct, shYeti, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shPHead, darkena(col, 0, 0xFF));
      }
    else if(m == moShadow) {
      queuepoly(VL, ct, shPBody,  darkena(col, 0, 0X80));
      queuepoly(VL, ct, shPSword, darkena(col, 0, 0XC0));
      queuepoly(VL, ct, shPHead,  darkena(col, 1, 0XC0));
      queuepoly(VL, ct, shPFace,  darkena(col, 0, 0XC0));
      }
    else if(m == moRanger) {
      queuepoly(VL, ct, shPBody, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shPSword, darkena(col, 0, 0xFF));
      queuepoly(VL, ct, shArmor, darkena(col, 1, 0xFF));
      }
    else if(m == moGhost || m == moSeep) {
      queuepoly(VL, ct, shGhost, darkena(col, 0, 0x80));
      queuepoly(VL, ct, shEyes, 0xFF);
      }
    else if(m == moFireFairy) {
      col = gradient(0xFFFF00, 0xFF0000, -1, sin(ticks/100.0), 1);
      queuepoly(VL, ct, shFemaleBody, darkena(col, 0, 0XC0));
//    queuepoly(cV2, ct, shPSword, darkena(col, 0, 0XFF));
      queuepoly(VL, ct, shFemaleHair, darkena(col, 1, 0xFF));
      queuepoly(VL, ct, shPFace, darkena(col, 0, 0XFF));
//    queuepoly(cV2, ct, shFemaleDress, 0xC00000FF);
      }
    else if(m == moSlime) {
      queuepoly(VL, ct, shSlime, darkena(col, 0, 0x80));
      queuepoly(VL, ct, shEyes, 0xFF);
      }
    else if(m == moCultist || m == moPyroCultist) {
      queuepoly(VL, ct, shPBody, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shPSword, darkena(col, 2, 0xFF));
      queuepoly(VL, ct, shHood, darkena(col, 1, 0xFF));
      }
    else if(m == moNecromancer) {
      queuepoly(VL, ct, shPBody, 0xC00000C0);
      queuepoly(VL, ct, shHood, darkena(col, 1, 0xFF));
      }
    else if(m == moGoblin) {
      queuepoly(VL, ct, shYeti, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shArmor, darkena(col, 1, 0XFF));
      }
    else if(m == moTroll) {
      queuepoly(VL, ct, shYeti, darkena(col, 0, 0xC0));
      queuepoly(VL, ct, shPHead, darkena(col, 1, 0XFF));
      queuepoly(VL, ct, shPFace, darkena(col, 2, 0XFF));
      }        
    else if(xch == 'd' || xch == 'D') {
      queuepoly(VL, ct, shPBody, darkena(col, 1, 0xC0));
      int acol = col;
      if(xch == 'D') acol = 0xD0D0D0;
      queuepoly(VL, ct, shDemon, darkena(acol, 0, 0xFF));
      }
    else return true;
    }
  
  return false;
  }

cell *keycell;
int keycelldist;

void drawCircle(int x, int y, int size, int color) {
  #ifdef GL
  if(vid.usingGL) {
    qglcoords = 0;
    glcolor(color);
    x -= vid.xcenter; y -= vid.ycenter;
    int pts = size * 4;
    if(pts > 1500) pts = 1500;
    if(ISMOBILE && pts > 72) pts = 72;
    for(int r=0; r<pts; r++) {
      float rr = (M_PI * 2 * r) / pts;
      glcoords[r][0] = x + size * sin(rr);
      glcoords[r][1] = y + size * cos(rr);
      glcoords[r][2] = vid.scrdist;
      }
    
    glVertexPointer(3, GL_FLOAT, 0, glcoords);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_LOOP, 0, pts);
    return;
    }
  #endif

#ifdef MOBILE
  gdpush(4); gdpush(color); gdpush(x); gdpush(y); gdpush(size);
#else
#ifdef GFX
  (vid.usingAA?aacircleColor:circleColor) (s, x, y, size, (color << 8) | 0x80);
#else
  int pts = size * 4;
  if(pts > 1500) pts = 1500;
  for(int r=0; r<pts; r++)
    qpixel(s, x + int(size * sin(r)), y + int(size * cos(r))) = color;
#endif
#endif
  }

void drawcell(cell *c, transmatrix V, int spinv) {

  // todo: fix when scrolling                 
  if(c->land != laGameBoard && playermoved && c->cpdist > sightrange && sightrange < 10) return;
  
  // draw a web-like map
  if(0) {
    if(c->type == 6) {
      for(int a=0; a<3; a++)
      drawline(V*Crad[a*7], V*Crad[a*7+21], 0xd0d0 >> darken);
      }
    else {
      for(int a=0; a<7; a++)
      drawline(V*C0, V*Crad[(21+a*6)%42], 0xd0d0 >> darken);
      }
    }

  if(0) if(c->type == 7) {
    drawline(V*C0, V*xpush(tessf)*C0, 0xd0d0 >> darken);
    }
  
  // save the player's view center
  if(c == cwt.c) {
    playerfound = true;
    cwtV = V * spin(-cwt.spin * 2*M_PI/c->type) * spin(M_PI);
    }

  if(1) {
  
    hyperpoint VC0 = V*C0;
  
    if(intval(mouseh, VC0) < modist) {
      modist = intval(mouseh, VC0);
      mouseover = c;
      }

    if(VC0[2] < centdist) {
      centdist = VC0[2];
      centerover = c;
      }

    int xc, yc, sc, xs, ys, ss;
    getcoord(VC0, xc, yc, sc);
    getcoord(V*xpush(.5)*C0, xs, ys, ss);
    // int col = 0xFFFFFF - 0x20 * c->maxdist - 0x2000 * c->cpdist;

    if(c->mpdist > 8) return; // not yet generated

    char ch = winf[c->wall].glyph;
    int col = winf[c->wall].color;
    
    if(c->land == laCrossroads && c->wall == waNone) col = (vid.goteyes ? 0xFF3030 : 0xFF0000);
    if(c->land == laDesert && c->wall == waNone) col = 0xEDC9AF;
    if(c->land == laJungle) col = (vid.goteyes ? 0x408040 : 0x008000);
    if(c->land == laDryForest) {
      if(c->wall == waDryTree)
        col = (vid.goteyes ? 0xC0C060 : 0xC0C000);
      else if(c->wall == waWetTree)
        col = (vid.goteyes ? 0x60C060 : 0x00C000);
      else if(c->wall == waNone) {
        col = gradient(0x008000, 0x800000, 0, c->heat, 10);
        }
      }
    if(c->land == laMirror && c->wall == waNone) col = 0x808080;
    if(c->land == laMotion && c->wall == waNone) col = 0xF0F000;
    if(c->land == laGraveyard && c->wall == waNone) col = 0x107010;
    if(c->land == laRlyeh && c->wall == waNone) col = (vid.goteyes ? 0x4080C0 : 0x004080);
    if(c->land == laHell && c->wall == waNone) col = (vid.goteyes ? 0xC03030 : 0xC00000);
    if(c->land == laGameBoard) {
      col = linf[c->land].color;
      if(c->wall != waNone)
        col = winf[c->wall].color;
      if(c->wall == waLake) col = 0x202020;
      if(c->wall == waFrozenLake) col = 0xC0C0C0;

      if(c->type == 7 && vid.boardmode == '3') col = 0;
      if(c->type == 6 && vid.boardmode == '7') col = 0;
      }

    if(isIcyLand(c) && isIcyWall(c)) {
      if(c->heat < -0.4)
        col = gradient(0x4040FF, 0x0000FF, -0.4, c->heat, -1);
      else if(c->heat < 0)
        col = gradient(0x8080FF, 0x4040FF, 0, c->heat, -0.4);
      else if(c->heat < 0.2)
        col = gradient(0x8080FF, 0xFFFFFF, 0, c->heat, 0.2);
      // else if(c->heat < 0.4)
      //  col = gradient(0xFFFFFF, 0xFFFF00, 0.2, c->heat, 0.4);
      else if(c->heat < 0.6)
        col = gradient(0xFFFFFF, 0xFF0000, 0.2, c->heat, 0.6);
      else if(c->heat < 0.8)
        col = gradient(0xFF0000, 0xFFFF00, 0.6, c->heat, 0.8);
      else
        col = 0xFFFF00;
      if(c->wall == waNone) 
        col = (col & 0xFEFEFE) >> 1;
      if(c->wall == waLake)
        col = (col & 0xFCFCFC) >> 2;
      }
    
    if(c->wall == waBonfire && c->tmp == 0)
      col = 0x404040;
      
    if(c->wall == waBonfire && c->tmp > 0)
      col = gradient(0xFF0000, 0xFFFF00, -1, sin(ticks/100.0), 1);
      
    if(c->wall == waThumper && c->tmp == 0)
      col = 0xEDC9AF;
      
    if(c->wall == waThumper && c->tmp > 0) {
      int ds = ticks;
      for(int u=0; u<5; u++) {
        ld rad = hexf * (.3 * u + (ds%1000) * .0003);
        int col = gradient(0xFFFFFF, 0, 0, rad, 1.5 * hexf);
        for(int a=0; a<84; a++)
          drawline(V*ddi(a, rad)*C0, V*ddi(a+1, rad)*C0, col);
        }
      }
    
    int xcol = col;
    
    if(c->item) 
      ch = iinf[c->item].glyph, col = iinf[c->item].color;
    
    int icol = col;
    
    if(c->item && c->land == laAlchemist)
      xcol = col;

    if(c->monst)
      ch = minf[c->monst].glyph, col = minf[c->monst].color;
    
    if(c->cpdist == 0 && drawplayer) { ch = '@'; col = cheater ? 0xFF3030 : 0xD0D0D0; }
    
    if(c->monst == moSlime) {
      col = winf[c->wall].color;
      col |= (col>>1);
      }
    
    if(c->ligon) {
      int tim = ticks - lightat;
      if(tim > 1000) tim = 800;
      for(int t=0; t<7; t++) if(c->mov[t] && c->mov[t]->ligon) {
        int hdir = 42 - t * 84 / c->type;
        int col = gradient(iinf[itOrbLightning].color, 0, 0, tim, 1100);
        drawline(V*ddi(ticks, hexf/2)*C0, V*ddi(hdir, crossf)*C0, col);
        }
      }
    
    int ct = c->type;

    bool error = false;
      
    if(vid.wallmode) {
    
      // floor
      
      int fd = 
        c->land == laAlchemist || c->land == laIce || c->land == laGraveyard ||
        c->land == laRlyeh ? 1 : 2;
      
      if(shUser[0][ct-6].s) {
        for(int i=0; i<8; i++) if(shUser[i][ct-6].s)
          queuepoly(V, ct, shUser[i][ct-6], darkena(xcol, 2, 0xC0));
        }
      
      else if(c->wall == waChasm)
        ;
      
      else if(vid.wallmode == 1 && c->land == laAlchemist)
        queuepoly(V, ct, shFloor[ct-6], darkena(xcol, 1, 0xFF));
      
      else if(vid.wallmode == 1 && c->land != laGameBoard)
        queuepoly(V, ct, shBFloor[ct-6], darkena(xcol, 0, 0xFF));
      
      else if(vid.wallmode == 2 && c->land != laGameBoard)
        queuepoly(V, ct, shFloor[ct-6], darkena(xcol, fd, 0xFF));
      
      else if(c->land == laRlyeh)
        queuepoly(V, ct, (euclid ? shFloor: shTriFloor)[ct-6], darkena(xcol, 1, 0xFF));

      else if(c->land == laAlchemist)
        queuepoly(V, ct, shCloudFloor[ct-6], darkena(xcol, 1, 0xFF));

      else if(c->land == laJungle)
        queuepoly(V, ct, (euclid ? shStarFloor : shFeatherFloor)[ct-6], darkena(xcol, 2, 0xFF));

      else if(c->land == laGraveyard)
        queuepoly(V, ct, (euclid ? shFloor : shCrossFloor)[ct-6], darkena(xcol, 1, 0xFF));

      else if(c->land == laMotion)
        queuepoly(V, ct, shMFloor[ct-6], darkena(xcol, 2, 0xFF));

      else if(c->land == laHell)
        queuepoly(V, ct, (euclid ? shStarFloor : shDemonFloor)[ct-6], darkena(xcol, 2, 0xFF));

      else if(c->land == laIce)
//      queuepoly(V, ct, shFloor[ct-6], darkena(xcol, 2, 0xFF));
        queuepoly(V, ct, shStarFloor[ct-6], darkena(xcol, 1, 0xFF));

      else if(c->land == laCocytus)
        queuepoly(V, ct, (euclid ? shCloudFloor : shDesertFloor)[ct-6], darkena(xcol, 1, 0xFF));

      else if(c->land == laCaves)
        queuepoly(V, ct, (euclid ? shCloudFloor : shCaveFloor)[ct-6], darkena(xcol, 2, 0xFF));

      else if(c->land == laGameBoard) {
        if(c->type == 7 && vid.boardmode == '7')
          queuepoly(V, ct, vid.wallmode == 1 ? shBFloor[ct-6] : shBigHepta, darkena(xcol, 2, 0xFF));

        if(c->type == 6 && vid.boardmode == '3')
          queuepoly(V, ct, vid.wallmode == 1 ? shBFloor[ct-6] : shBigTriangle, darkena(xcol, 2, 0xFF));

        if(vid.boardmode == 'h')
          queuepoly(V, ct, vid.wallmode == 1 ? shBFloor[ct-6] : shFloor[c->type - 6], darkena(xcol, 2, 0xFF));
        }

      else if(c->land == laDesert)
        queuepoly(V, ct, (euclid ? shCloudFloor : shDesertFloor)[ct-6], darkena(xcol, 2, 0xFF));

      else if(c->land == laDryForest)
        queuepoly(V, ct, (euclid ? shStarFloor : shDesertFloor)[ct-6], darkena(xcol, 2, 0xFF));

      else 
        queuepoly(V, ct, shFloor[ct-6], darkena(xcol, 2, 0xFF));
      // walls
      
      char xch = winf[c->wall].glyph;
      
      if(c->wall == waSulphurC)
        queuepoly(V, ct, shGiantStar[ct-6], darkena(xcol, 0, 0xFF));
    
      else if(c->wall == waFrozenLake || c->wall == waLake)
        ;
      
      else if(xch == '#')
        queuepoly(V, ct, shWall[ct-6], darkena(xcol, 0, 0xFF));
      
      else if(xch == '%')
        queuepoly(V, ct, shMirror, darkena(xcol, 0, 0xC0));
      
      else if(isActiv(c)) {
        ld sp = c->tmp > 0 ? ticks / 500. : 0;
        queuepoly(V * spin(sp), ct, shStar, darkena(col, 0, 0xF0));
        }
      
      else if(xch == '+' && c->land == laGraveyard)
        queuepoly(V, ct, shCross, darkena(xcol, 0, 0xFF));

      else if(xch != '.' && xch != '+' && xch != '>' && c->wall != waSulphur)
        error = true;
      }
    else if(!(c->item || c->monst || c->cpdist == 0)) error = true;
    
    // treasure

    char xch = iinf[c->item].glyph;
    hpcshape *xsh = 
      xch == '*' ? &shGem[ct-6] : xch == '%' ? &shDaisy : xch == '$' ? &shStar : xch == ';' ? &shTriangle :
      xch == '!' ? &shTriangle : c->item == itBone ? &shNecro : c->item == itStatue ? &shStatue :
      c->item == itKey ? &shKey : NULL;
    
    if(vid.monmode == 0 && c->item)
      error = true;
    
    else if(xsh)
      queuepoly(V * spin(ticks / 1500.), ct, *xsh, darkena(icol, 0, 0xF0));
    
    else if(xch == 'o') {
      queuepoly(V, ct, shDisk, darkena(icol, 0, 0xC0));
      queuepoly(V, ct, shRing, darkena(icol, 0, int(0x80 + 0x70 * sin(ticks / 300.))));
      }

    else if(c->item) error = true;
    
    // monsters
    
    error |= drawMonster(V, ct, c, col);

/*    if(ch == '.') {
      col = darkened(col);
      for(int t=0; t<ct; t++)
        drawline(V*ddi(t*84/ct, hexf/3)*C0, V*ddi((t+1)*84/ct, hexf/3)*C0, col);
      }
      
    else if(ch == '#') {
      col = darkened(col);
      for(int u=1; u<6; u++)
      for(int t=0; t<ct; t++)
        drawline(V*ddi(0 + t*84/ct, u*hexf/6)*C0, V*ddi(0 + (t+1)*84/ct, u*hexf/6)*C0, col);
      }
      
    else */
    
    if(error) {
      int siz = int(sqrt(squar(xc-xs)+squar(yc-ys)));
      if(vid.wallmode >= 2) {
        displaychr(xc-2, yc, sc, siz, ch, 0);
        displaychr(xc+2, yc, sc, siz, ch, 0);
        displaychr(xc, yc-2, sc, siz, ch, 0);
        displaychr(xc, yc+2, sc, siz, ch, 0);
        }
      displaychr(xc, yc, sc, siz, ch, col);
      }
    
    if(c == keycell) {
      displaychr(xc, yc, sc, 2*vid.fsize, 'X', 0x10101 * int(128 + 100 * sin(ticks / 150.)));
      displaystr(xc, yc, sc, vid.fsize, its(keycelldist), 0x10101 * int(128 - 100 * sin(ticks / 150.)), 8);
      }
    
#if defined(ANDROID) || defined(PANDORA) || defined(IOS)
    if(c == lmouseover && (mousepressed || ISANDROID || ISMOBILE)) {
      drawCircle(xc, yc, int(sqrt(squar(xc-xs)+squar(yc-ys)) * .8), c->cpdist > 1 ? 0x00FFFF : 0xFF0000);
      }
#endif

    if(c == lmouseover && c->land == laGameBoard) {
      drawCircle(xc, yc, int(sqrt(squar(xc-xs)+squar(yc-ys)) * .8), 0x00FFFF);
      }
    
    if(joydir >= 0 && c == cwt.c->mov[(joydir+cwt.spin) % cwt.c->type])
      drawCircle(xc, yc, int(sqrt(squar(xc-xs)+squar(yc-ys)) * (.78 - .02 * sin(ticks/199.0))), 0x00FF00);

#ifndef MOBILE    
    if(c == lcenterover && !playermoved)
      drawCircle(xc, yc, int(sqrt(squar(xc-xs)+squar(yc-ys)) * (.70 - .06 * sin(ticks/200.0))), int(175 + 25 * sin(ticks / 200.0)));
#endif
    
    if(cmode == emDraw && cwt.c->type == 6 && ct == 6) for(int a=0; a<dsCur->rots; a++) {

      transmatrix V2 = V * spin(M_PI + 2*M_PI*a/dsCur->rots);

      if(outofmap(mouseh)) break;

      hyperpoint P2 = V2 * inverse(cwtV) * mouseh;
    
      int xc, yc, sc;
      getcoord(P2, xc, yc, sc);
      displaychr(xc, yc, sc, 10, 'x', 0xFF);

      if(crad > 0 && c->cpdist <= 3) {
        lalpha = 0x80;
        transmatrix movtocc = V2 * inverse(cwtV) * rgpushxto0(ccenter);
        for(int d=0; d<84; d++) 
          drawline(movtocc * ddi(d+1, crad) * C0, movtocc * ddi(d, crad) * C0, 0xC00000);
        lalpha = 0xFF;
        }
      }

    // process mouse
    
    for(int i=-1; i<cwt.c->type; i++) if(i == -1 ? cwt.c == c : cwt.c->mov[i % cwt.c->type] == c) {
      int mx = mousex, my = mousey;
      if(revcontrol) mx = vid.xcenter*2-mx, my = vid.ycenter*2-my;
      
      int ndist = (xc-mx) * (xc-mx) + (yc-my) * (yc-my);
      if(ndist < mousedist) mousedist = ndist, mousedest = i;
      }
    
    // drawline(V*C0, V*Crad[0], 0xC00000);
    if(c->bardir != NODIR) {
      drawline(V*C0, V*heptmove[c->bardir]*C0, 0x505050 >> darken);
      drawline(V*C0, V*hexmove[c->bardir]*C0, 0x505050 >> darken);
      }

    }
  }

string buildCredits();

string buildHelpText() {
  string h;
  h += XLAT("Welcome to HyperRogue");
#ifdef ANDROID  
  h += XLAT(" for Android");
#endif
#ifdef IOS
  h += XLAT(" for iOS");
#endif
  h += XLAT("! (version %1)\n\n", VER);
  
  h += XLAT(
MC  "You have been trapped in a strange, non-Euclidean world. Collect as much treasure as possible "
MC  "before being caught by monsters. The more treasure you collect, the more "
MC  "monsters come to hunt you, as long as you are in the same land type. The "
M   "Orbs of Yendor are the ultimate treasure; get at least one of them to win the game!"
    );
  h += XLAT(" (press ESC for some hints about it).");
  h += "\n\n";
  
  h += XLAT(
MC  "You can fight most monsters by moving into their location. "
MC  "The monster could also kill you by moving into your location, but the game "
M   "automatically cancels all moves which result in that.\n\n"
    );
#ifdef MOBILE
  h += XLAT(
MC  "Usually, you move by touching somewhere on the map; you can also touch one "
MC  "of the four buttons on the map corners to change this (to scroll the map "
MC  "or get information about map objects). You can also touch the "
M   "numbers displayed to get their meanings.\n"
    );
#else
  h += XLAT(
MC  "Move with mouse, num pad, qweadzxc, or hjklyubn. Wait by pressing 's' or '.'. Spin the world with arrows, PageUp/Down, and Home/Space. "
MC  "To save the game you need an Orb of Safety. Press 'v' for config, ESC for the quest status and menu.\n\n"
    );
#endif
  h += XLAT("See more on the website: ") 
    + "http//roguetemple.com/z/hyper.php\n\n";
  
#ifdef MOBILE
  h += buildCredits();
#else
  h += XLAT("Press 'c' for credits.");
#endif
  return h;
  }

string musiclicense;

string buildCredits() {
  string h;
  h += XLAT("game design, programming, texts and graphics by Zeno Rogue <zeno@attnam.com>\n\n");
  if(lang() != 0)
    h += XLAT("add credits for your translation here");
#ifndef NOLICENSE
  h += XLAT(
MC  "released under GNU General Public License version 2 and thus "
M   "comes with absolutely no warranty; see COPYING for details\n\n"
    );
#endif
  h += XLAT(
M   "special thanks to the following people for their bug reports, feature requests, porting, and other help:\n\n%1\n\n",
    "Konstantin Stupnik, ortoslon, chrysn, Adam Borowski, Damyan Ivanov, Ryan Farnsley, mcobit, Darren Grey, tricosahedron, Maciej Chojecki, Marek Čtrnáct"
    );
#ifdef EXTRALICENSE
  h += EXTRALICENSE;
#endif
  if(musiclicense != "") h += musiclicense;
  return h;
  }

void describeMouseover() {
  cell *c = mousing ? mouseover : playermoved ? NULL : centerover;
  string out = mouseovers;
  if(!c) { }
  else if(cmode == emNormal) {
    out = XLAT1(linf[c->land].name);
    help = XLAT(linf[c->land].help);
    
    // if(c->land == laIce) out = "Icy Lands (" + fts(60 * (c->heat - .4)) + " C)";
    if(c->land == laIce) out += " (" + fts(60 * (c->heat-.4)) + " °C)";
    if(c->land == laCocytus) out += " (" + fts(60 * (c->heat-.8)) + " °C)";
    if(c->land == laDryForest && c->heat > .5) out += " (" + its(int(c->heat+.1))+"/10)";
  
    if(c->wall && !((c->wall == waFloorA || c->wall == waFloorB) && c->item)) { 
      out += ", "; out += XLAT1(winf[c->wall].name); help = XLAT(winf[c->wall].help);
      }
    
    if(isActiv(c)) {
      if(c->tmp < 0) out += XLAT(" (touch to activate)");
      if(c->tmp == 0) out += XLAT(" (expired)");
      if(c->tmp > 0) out += XLAT(" [%1 turns]", its(c->tmp));
      }
  
    if(c->monst) {out += ", "; out += XLAT1(minf[c->monst].name); help = XLAT(minf[c->monst].help);}
  
    if(c->item) {out += ", "; out += XLAT1(iinf[c->item].name); if(!c->monst) help = XLAT(iinf[c->item].help);}
    
    if(!c->cpdist) out += XLAT(", you");
    }
  else if(cmode == emVisual1) {
    if(getcstat == 'p') {
      out = XLAT("0 = Klein model, 1 = Poincaré model");
      if(vid.alpha < -0.5)
        out = XLAT("you are looking through it!");
      }
    else if(getcstat == 'r') {
      out = XLAT("simply resize the window to change resolution");
      }
    else if(getcstat == 'f') {
      out = XLAT("[+] keep the window size, [-] use the screen resolution");
      }
    else if(getcstat == 'a' && vid.aspeed > -4.99)
      out = XLAT("+5 = center instantly, -5 = do not center the map");
    else if(getcstat == 'a')
      out = XLAT("press Space or Home to center on the PC");
    else if(getcstat == 'w' || getcstat == 'm')
      out = XLAT("You can choose one of the several modes");
    else if(getcstat == 'c')
      out = XLAT("The axes help with keyboard movement");
    else if(getcstat == 'g')
      out = XLAT("Affects looks and grammar");
#ifndef MOBILE
    else if(getcstat == 's')
      out = XLAT("Config file: %1", conffile);
#endif
    else out = "";
    }
  else if(cmode == emVisual2) {
    if(getcstat == 'p') { // stick) {
      if(autojoy) 
        out = XLAT("joystick mode: automatic (release the joystick to move)");
      if(!autojoy) 
        out = XLAT("joystick mode: manual (press a button to move)");
      }
    else if(getcstat == 'e')
      out = XLAT("You need special glasses to view the game in 3D");
    else if(getcstat == 'f')
      out = XLAT("Reduce the framerate limit to conserve CPU energy");
    }
  
  mouseovers = out;
  #ifndef MOBILE
  displayfr(vid.xres/2, vid.fsize,   2, vid.fsize, out, linf[cwt.c->land].color, 8);
  if(mousey < vid.fsize * 3/2) getcstat = SDLK_F1;
  #endif
  }

void drawrec(const heptspin& hs, int lev, hstate s, transmatrix V) {
  
  cell *c = hs.h->c7;
  
  drawcell(c, V * spin(hs.spin*2*M_PI/7), hs.spin);
  
  if(lev <= 0) return;
  
  for(int d=0; d<7; d++) {
    int ds = fixrot(hs.spin + d);
    // createMov(c, ds);
    if(c->mov[ds] && c->spn[ds] == 0)
      drawcell(c->mov[ds], V * hexmove[d], 0);
    }

  if(lev <= 1) return;
  
  for(int d=0; d<7; d++) {
    hstate s2 = transition(s, d);
    if(s2 == hsError) continue;
    heptspin hs2 = hsstep(hsspin(hs, d), 0);
    drawrec(hs2, lev-2, s2, V * heptmove[d]);
    }
  
  }

void drawEuclidean() {
  eucoord px, py;
  decodeMaster(cwt.c->master, px, py);
  for(int dx=-10; dx<=10; dx++)
  for(int dy=-10; dy<=10; dy++) {
    eucoord x = dx+px;
    eucoord y = dy+py;
    cell *c = euclideanAt(x,y);
    if(!c) continue;
    transmatrix Mat = View;
    Mat[2][2] = 1;
    Mat[0][2] += (x + y * .5) * eurad;
    double q3 = sqrt(double(3));
    Mat[1][2] += y * q3 /2 * eurad;
    while(Mat[0][2] <= -16384 * eurad) Mat[0][2] += 32768 * eurad;
    while(Mat[0][2] >= 16384 * eurad) Mat[0][2] -= 32768 * eurad;
    while(Mat[1][2] <= -16384 * q3 * eurad) Mat[1][2] += 32768 * q3 * eurad;
    while(Mat[1][2] >= 16384 * q3 * eurad) Mat[1][2] -= 32768 * q3 * eurad;
    Mat[0][0] = -1;
    Mat[1][1] = -1;
    // Mat[2][0] = x*x/10;
    // Mat[2][1] = y*y/10;
    // Mat = Mat * xpush(x-30) * ypush(y-30);
    drawcell(c, Mat, 0);
    }
  }

void drawthemap() {

  keycell = NULL;

  if(yii < size(yi)) {
    if(!yi[yii].found) for(int i=0; i<YDIST; i++) if(yi[yii].path[i]->cpdist <= sightrange) {
      keycell = yi[yii].path[i];
      keycelldist = YDIST - i;
      }
    }
  
  #ifndef MOBILE
  lmouseover = mouseover;
  #endif
  mousedist = 1000000;
  mousedest = -1;
  modist = 1e20; mouseover = NULL; mouseovers = XLAT("Press F1 or right click for help");
  centdist = 1e20; lcenterover = centerover; centerover = NULL; 
  #ifdef MOBILE
  mouseovers = XLAT("No info about this...");
  #endif
  if(outofmap(mouseh)) 
    modist = -5;
  playerfound = false;
  
  if(euclid)
    drawEuclidean();
  else
    drawrec(viewctr, 
      (!playermoved) ? sightrange+1 : sightrange + 4,
      hsOrigin, View);
  }

void centerpc(ld aspd) { 
  hyperpoint H = cwtV * C0;
  ld R = sqrt(H[0] * H[0] + H[1] * H[1]);
  if(R < 1e-9) {
    return;
    }
  
  if(euclid) {
    // Euclidean
    aspd *= (2+3*R*R);
    if(aspd > R) aspd = R;
  
    View[0][2] -= cwtV[0][2] * aspd / R;
    View[1][2] -= cwtV[1][2] * aspd / R;
    }
  
  else {
    aspd *= (1+R);

    if(R < aspd) {
      View = gpushxto0(H) * View;
      }
    else 
      View = rspintox(H) * xpush(-aspd) * spintox(H) * View;
    }
  }

void drawmovestar() {

  if(!playerfound) return;
  
  if(vid.axes == 0 || (vid.axes == 1 && mousing)) return;

  hyperpoint H = cwtV * C0;
  ld R = sqrt(H[0] * H[0] + H[1] * H[1]);
  transmatrix Centered = Id;

  if(euclid) 
    Centered = eupush(H[0], H[1]);
  else if(R > 1e-9) Centered = rgpushxto0(H);
  
  int starcol = (vid.goteyes? 0xE08060 : 0xC00000);
  
  if(vid.axes == 3 || (vid.wallmode == 2 && vid.axes == 1))
    queuepoly(Centered, 7, shMovestar, darkena(starcol, 0, 0xFF));
  
  else for(int d=0; d<8; d++) {
    int col = starcol;
#ifdef PANDORA
    if(leftclick && (d == 2 || d == 6 || d == 1 || d == 7)) col >>= 2;
    if(rightclick && (d == 2 || d == 6 || d == 3 || d == 5)) col >>= 2;
    if(!leftclick && !rightclick && (d&1)) col >>= 2;
#endif
//  EUCLIDEAN
    if(euclid)
      drawline(Centered * C0, Centered * ddi(d * 10.5, 0.5) * C0, col >> darken);
    else
      drawline(Centered * C0, Centered * spin(M_PI*d/4)* xpush(.5) * C0, col >> darken);
    }
  }

void optimizeview() {
  
  int turn = 0;
  ld best = INF;
  
  transmatrix TB;
  
  for(int i=-1; i<7; i++) {

    ld trot = -i * M_PI * 2 / 7.0;
    transmatrix T = i < 0 ? Id : spin(trot) * xpush(tessf) * spin(M_PI);
    hyperpoint H = View * T * C0;
    if(H[2] < best) best = H[2], turn = i, TB = T;
    }
  
  if(turn >= 0) {
    View = View * TB;
    fixmatrix(View);
    viewctr = hsspin(viewctr, turn);
    viewctr = hsstep(viewctr, 0);
    }
  }

int vectodir(const hyperpoint& P) {
  hyperpoint H = cwtV * C0;
  ld R = sqrt(H[0] * H[0] + H[1] * H[1]);
  transmatrix Centered = cwtV;
  if(!euclid)
    Centered = gpushxto0(H)  * Centered;
  else if(R > 1e-9)
    Centered = eupush(-H[0], -H[1]) * Centered;
  int bdir = -1;
  ld binv = 99;
  for(int i=0; i<cwt.c->type; i++) {
    ld inv = intval(Centered * spin(-i * 2 * M_PI /cwt.c->type) * xpush(1) * C0, P);
    if(inv < binv) binv = inv, bdir = i;
    }
  return bdir;
  }

void movepckeydir(int d) {
  // EUCLIDEAN
  if(euclid)
    movepcto(vectodir(spin(-d * M_PI/4) * eupush(1, 0) * C0));
  else
    movepcto(vectodir(spin(-d * M_PI/4) * xpush(1) * C0));
  }

void checkjoy() {
  ld joyvalue1 = vid.joyvalue * vid.joyvalue;
  ld joyvalue2 = vid.joyvalue2 * vid.joyvalue2;
  
  ld jx = joyx;
  ld jy = joyy;
  ld sq = jx*jx+jy*jy;
  
  if(autojoy) {
    if(sq < joyvalue1) { if(joydir >= 0) movepcto(joydir); joydir = -1; return; }
    if(sq < joyvalue2 && joydir == -1) return;
    }
  else {
    if(sq < joyvalue1) { joydir = -1; return; }
    }
  
  joydir = vectodir(hpxy(jx, jy));
  }

void checkpanjoy(double t) {

  if(panjoyx * panjoyx + panjoyy * panjoyy < vid.joypanthreshold * vid.joypanthreshold)
    return;
  
  ld jx = panjoyx * t * vid.joypanspeed;
  ld jy = panjoyy * t * vid.joypanspeed;
  
  playermoved = false;
  View = gpushxto0(hpxy(jx, jy)) * View;
  }

void calcparam() {
  vid.xcenter = vid.xres / 2;
  vid.ycenter = vid.yres / 2;
  vid.radius = int(vid.scale * vid.ycenter) - (ISANDROID ? 2 : ISIOS ? 40 : 40);
  
  if(vid.xres < vid.yres) {
    vid.radius = int(vid.scale * vid.xcenter) - (ISIOS ? 10 : 2);
    vid.ycenter = vid.yres - vid.radius - vid.fsize - (ISIOS ? 10 : 0);
    }

  vid.beta = 1 + vid.alpha + vid.eye;
  vid.alphax = vid.alpha + vid.eye;
  vid.goteyes = vid.eye > 0.001 || vid.eye < -0.001;
  }

#ifndef MOBILE
void displayStat(int y, const string& name, const string& val, char mkey) {
  
  int dy = vid.fsize * y + vid.yres/4;
  int dx = vid.xres/2 - 100;
  
  bool xthis = (mousey >= dy-vid.fsize/2 && mousey <= dy + vid.fsize/2);
  int xcol = 0x808080;
  
  if(xthis) {
    getcstat = mkey; getcshift = 0;
    int mx = mousex - dx;
    if(mx >= 0 && mx <= 100) {
      if(mx < 20) getcshift = -1   , xcol = 0xFF0000;
      else if(mx < 40) getcshift = -0.1 , xcol = 0x0000FF;
      else if(mx < 50) getcshift = -0.01, xcol = 0x00FF00;
      if(mx > 80) getcshift = +1   , xcol = 0xFF0000;
      else if(mx > 60) getcshift = +0.1 , xcol = 0x0000FF;
      else if(mx > 50) getcshift = +0.01, xcol = 0x00FF00;
      }
    }
  
  if(val != "") {
    displaystr(dx,    dy, 0, vid.fsize, val, xthis ? 0xFFFF00 : 0x808080, 16);
    displaystr(dx+25, dy, 0, vid.fsize, "-", xthis && getcshift < 0 ? xcol : 0x808080, 8);
    displaystr(dx+75, dy, 0, vid.fsize, "+", xthis && getcshift > 0 ? xcol : 0x808080, 8);
    }

  displaystr(dx+100, dy, 0, vid.fsize, s0 + mkey, xthis ? 0xFFFF00 : 0xC0F0C0, 0);

  displaystr(dx+125, dy, 0, vid.fsize, name, xthis ? 0xFFFF00 : 0x808080, 0);
  }

void displayStatHelp(int y, string name) {
  
  int dy = vid.fsize * y + vid.yres/4;
  int dx = vid.xres/2 - 100;
  
  displaystr(dx+100, dy, 0, vid.fsize, name, 0xC0C0C0, 0);
  }

void displayButton(int x, int y, const string& name, int key, int align, int rad = 0) {
  if(displayfr(x, y, rad, vid.fsize, name, 0x808080, align)) {
    displayfr(x, y, rad, vid.fsize, name, 0xFFFF00, align);
    getcstat = key;
    }
  }

void quitOrAgain() {
  int y = vid.yres * (618) / 1000;
  displayButton(vid.xres/2, y + vid.fsize*1/2, 
    (items[itOrbSafety] && havesave) ? 
      XLAT("Press Enter or F10 to save") : 
      XLAT("Press Enter or F10 to quit"), 
    SDLK_RETURN, 8, 2);
  displayButton(vid.xres/2, y + vid.fsize*2, XLAT("or 'r' or F5 to restart"), 'r', 8, 2);
  displayButton(vid.xres/2, y + vid.fsize*7/2, XLAT("or 't' to see the top scores"), 't', 8, 2);
  if(canmove) displayButton(vid.xres/2, y + vid.fsize*10/2, XLAT("or another key to continue"), ' ', 8, 2);
  }
#endif

int calcfps() {
  #define CFPS 30
  static int last[CFPS], lidx = 0;
  int ct = ticks;
  int ret = ct - last[lidx];
  last[lidx] = ct;
  lidx++; lidx %= CFPS;
  if(ret == 0) return 0;
  return (1000 * CFPS) / ret;
  }

int msgscroll = 0;

void showGameover() {
  int y = vid.yres * (1000-618) / 1000 - vid.fsize * 7/2;
  displayfr(vid.xres/2, y, 4, vid.fsize*2, 
    cheater ? XLAT("It is a shame to cheat!") : 
    showoff ? XLAT("Showoff mode") :
    canmove ? XLAT("Quest status") : 
    XLAT("GAME OVER"), 0xC00000, 8
    );
  displayfr(vid.xres/2, y + vid.fsize*2, 2, vid.fsize,   XLAT("Your score: %1", its(gold())), 0xD0D0D0, 8);
  displayfr(vid.xres/2, y + vid.fsize*3, 2, vid.fsize,   XLAT("Enemies killed: %1", its(tkills())), 0xD0D0D0, 8);
  
  if(!timerstopped && !canmove) {
    savetime += time(NULL) - timerstart;
    timerstopped = true;
    }
  if(canmove && !timerstart)
    timerstart = time(NULL);
  
  if(items[itOrbYendor]) {
    displayfr(vid.xres/2, y + vid.fsize*4, 2, vid.fsize,  XLAT("Orbs of Yendor found: %1", its(items[itOrbYendor])), 0xFF00FF, 8);
    displayfr(vid.xres/2, y + vid.fsize*5, 2, vid.fsize,  XLAT("CONGRATULATIONS!"), 0xFFFF00, 8);
    }
  else {
    if(gold() < 30)
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Collect 30 $$$ to access more worlds"), 0xC0C0C0, 8);
    else if(gold() < 60)
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Collect 60 $$$ to access R'Lyeh and Dry Forest"), 0xC0C0C0, 8);
    else if(!hellUnlocked())
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Collect at least 10 treasures in each of 9 types to access Hell"), 0xC0C0C0, 8);
    else if(items[itHell] < 10)
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Collect at least 10 Demon Daisies to find the Orbs of Yendor"), 0xC0C0C0, 8);
    else if(size(yi) == 0)
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Look for the Orbs of Yendor in Hell or in the Crossroads!"), 0xC0C0C0, 8);
    else 
      displayfr(vid.xres/2, y+vid.fsize*5, 2, vid.fsize, XLAT("Unlock the Orb of Yendor!"), 0xC0C0C0, 8);
    }
  if(tkills() < 100)
    displayfr(vid.xres/2, y+vid.fsize*6, 2, vid.fsize, XLAT("Defeat 100 enemies to access the Graveyard"), 0xC0C0C0, 8);
  else if(hellUnlocked()) {
    eLand seq[13] = {
      laHell, laCocytus, laGraveyard, 
      laRlyeh, laDryForest, laIce, laCaves, 
      laJungle, laDesert, laAlchemist, laMotion, laMirror, laCrossroads
      };
    bool b = true;
    for(int i=0; i<13; i++)
      if(b && items[treasureType(seq[i])] < 10) {
        displayfr(vid.xres/2, y+vid.fsize*6, 2, vid.fsize, XLAT("Hyperstone Quest: collect at least 10 %1 in %2", treasureType(seq[i]), seq[i]), 0xC0C0C0, 8);
        b = false;
        }
    if(b)
      displayfr(vid.xres/2, y+vid.fsize*6, 2, vid.fsize, XLAT("Hyperstone Quest completed!"), 0xC0C0C0, 8);
    }
  if((!canmove) && (!ISMOBILE))
    displayfr(vid.xres/2, y+vid.fsize*7, 2, vid.fsize, XLAT("(press ESC during the game to review your quest)"), 0xB0B0B0, 8);
  if(cheater)
    displayfr(vid.xres/2, y+vid.fsize*8, 2, vid.fsize, XLAT("you have cheated %1 times", its(cheater)), 0xFF2020, 8);
  if(!cheater) {
    int timespent = savetime + (timerstopped ? 0 : (time(NULL) - timerstart));
    char buf[20];
    sprintf(buf, "%d:%02d", timespent/60, timespent % 60);
    displayfr(vid.xres/2, y + vid.fsize*8, 2, vid.fsize,   
      XLAT("%1 turns (%2)", its(turncount), buf), 0xC0C0C0, 8);
    }
  
  msgs.clear();
  if(msgscroll < 0) msgscroll = 0;
  int gls = size(gamelog) - msgscroll;
  int mnum = 0;
  for(int i=gls-5; i<gls; i++) 
    if(i>=0) 
      mnum++,
      msgs.push_back(make_pair(make_pair(ticks-128*vid.flashtime-128*(gls-i), 1), gamelog[i]));

  if(mnum)
    displayfr(vid.xres/2, vid.yres-vid.fsize*(mnum+1), 2, vid.fsize/2,  XLAT("last messages:"), 0xC0C0C0, 8);
  
  #ifndef MOBILE
  quitOrAgain();
  #endif
  }

void displayabutton(int px, int py, string s, int col) {
  // TMP
  int siz = vid.yres > vid.xres ? vid.fsize*2 : vid.fsize * 3/2;
  int x = vid.xcenter + px * (vid.radius);
  int y = vid.ycenter + py * (vid.radius - siz/2);
  if(gtouched && !mouseover
    && abs(mousex - vid.xcenter) < vid.radius
    && abs(mousey - vid.ycenter) < vid.radius
    && hypot(mousex-vid.xcenter, mousey-vid.ycenter) > vid.radius
    && px == (mousex > vid.xcenter ? 1 : -1)
    && py == (mousey > vid.ycenter ? 1 : -1)
    ) col = 0xFF0000;
  displayfr(x, y, 0, siz, s, col, 8+8*px);
  }

vector<score> scores;

#define POSSCORE 65

int scoresort = 2;
int scoredisplay = 1;
int scorefrom = 0;
bool scorerev = false;

bool scorecompare(const score& s1, const score &s2) {
  return s1.tab[scoresort] > s2.tab[scoresort];
  }

string displayfor(score* S) {
  printf("S=%p, scoredisplay = %d\n", S, scoredisplay);
  if(S == NULL) {
    string names[6] = {"time elapsed", "date", "treasure collected", "total kills", "turn count", "cells generated"};
    if(scoredisplay < 6) return XLAT(names[scoredisplay]);
    if(scoredisplay == 6) return XLATN(iinf[itFernFlower].name);
    else if(scoredisplay-6 < itOrbLightning) return XLATN(iinf[scoredisplay-6].name);
    else if(scoredisplay >= 66 && scoredisplay < 70) return XLATN(minf[scoredisplay-65+43].name);
    else {
      eMonster m = eMonster(scoredisplay-6-itOrbLightning);
      if(m == moWormwait) return XLATN(minf[moCrystalSage].name);
      if(m == moWormtail) return XLATN(minf[moFireFairy].name);
      return XLATN(minf[m].name);
      }
    }
  if(scoredisplay == 0) {
    char buf[10];
    snprintf(buf, 10, "%d:%02d", S->tab[0]/60, S->tab[0]%60);
    return buf;
    }
  if(scoredisplay == 1) {
    time_t tim = S->tab[1];
    char buf[128]; strftime(buf, 128, "%c", localtime(&tim));
    return buf;
    }
//if(scoredisplay == 65) return its(S->tab3[1]);
//if(scoredisplay >= 66 && scoredisplay < 70) return its(S->tab3[scoredisplay - 66 + 2]);
  return its(S->tab[scoredisplay]);
  }

#ifndef ANDROID
void loadScores() {
  scores.clear();
  FILE *f = fopen(scorefile, "rt");
  if(!f) {
    printf("Could not open the score file '%s'!\n", scorefile);
    addMessage(s0 + "Could not open the score file: " + scorefile);
    return;
    }
  while(!feof(f)) {
    char buf[120];
    if(fgets(buf, 120, f) == NULL) break;
    if(buf[0] == 'H' && buf[1] == 'y') {
      score sc;
      if(fscanf(f, "%s", buf) <= 0) break; sc.ver = buf;
      bool ok = true;
      for(int i=0; i<SCSIZE; i++) {
        if(fscanf(f, "%d", &sc.tab[i]) <= 0) ok = false;
        }
      if(ok && sc.ver >= "4.4") {
        for(int i=0; i<SCSIZE2; i++) {
          if(fscanf(f, "%d", &sc.tab2[i]) <= 0) ok = false;
          }
        sc.tab[0] = sc.tab2[0];
        // the first executable on Steam included a corruption
        if(sc.tab2[0] > 1420000000 && sc.tab2[0] < 1430000000) {
          sc.tab[0] = sc.tab2[0] - sc.tab[1];
          sc.tab2[0] = sc.tab[0];
          }
        // do not include saves
        if(sc.tab2[4 + itOrbSafety - itOrbLightning]) ok = false;

//      if(sc.ver >= "4.9")
//        for(int i=0; i<SCSIZE3; i++)
//          if(fscanf(f, "%d", &sc.tab3[i]) <= 0) ok = false;
        }
      else
        sc.tab[0] = sc.tab[1] - sc.tab[0]; // could not save then
      if(ok) scores.push_back(sc);
      }
    }
  fclose(f);
  addMessage(its(size(scores))+" games have been recorded in "+scorefile);
  cmode = emScores;
  scoresort = 2; reverse(scores.begin(), scores.end());
  scorefrom = 0;
  stable_sort(scores.begin(), scores.end(), scorecompare);
  }

bool notgl = false;

void showScores() {
  int y = vid.fsize * 7/2;
  int bx = vid.fsize;

  mouseovers = XLAT("left/right - change display, up/down - scroll, s - sort by");

  displaystr(bx*4, vid.fsize*2, 0, vid.fsize, "#", 0xFFFFFF, 16);
  displaystr(bx*8, vid.fsize*2, 0, vid.fsize, "$$$", 0xFFFFFF, 16);
  displaystr(bx*12, vid.fsize*2, 0, vid.fsize, XLAT("kills"), 0xFFFFFF, 16);
  displaystr(bx*16, vid.fsize*2, 0, vid.fsize, XLAT("time"), 0xFFFFFF, 16);
  displaystr(bx*20, vid.fsize*2, 0, vid.fsize, XLAT("ver"), 0xFFFFFF, 16);
  displaystr(bx*21, vid.fsize*2, 0, vid.fsize, displayfor(NULL), 0xFFFFFF, 0);
  if(scorefrom < 0) scorefrom = 0;
  int id = scorefrom;
  while(y < (ISIOS ? vid.yres - 3*vid.fsize : vid.yres)) {
    if(id >= size(scores)) break;
    score& S(scores[id]);
    char buf[16];
    
    sprintf(buf, "%d", id+1);
    displaystr(bx*4,  y, 0, vid.fsize, buf, 0xC0C0C0, 16);
    
    sprintf(buf, "%d", S.tab[2]);
    displaystr(bx*8,  y, 0, vid.fsize, buf, 0xC0C0C0, 16);
    
    sprintf(buf, "%d", S.tab[3]);
    displaystr(bx*12,  y, 0, vid.fsize, buf, 0xC0C0C0, 16);

    sprintf(buf, "%d:%02d", S.tab[0]/60, S.tab[0] % 60);
    displaystr(bx*16,  y, 0, vid.fsize, buf, 0xC0C0C0, 16);

    displaystr(bx*20,  y, 0, vid.fsize, S.ver, 0xC0C0C0, 16);

    displaystr(bx*21, y, 0, vid.fsize, displayfor(&S), 0xC0C0C0, 0);
    
    y += vid.fsize*5/4; id++;
    }

#ifdef IOS
  displayabutton(-1, +1, XLAT("SORT"), BTON);
  displayabutton(+1, +1, XLAT("PLAY"), BTON);
#endif
  }

void sortScores() {
  if(scorerev) reverse(scores.begin(), scores.end());
  else {
    scorerev = true;
    scoresort = scoredisplay; 
    stable_sort(scores.begin(), scores.end(), scorecompare);
    }
  }

void shiftScoreDisplay(int delta) {
  scoredisplay = (scoredisplay + POSSCORE + delta) % POSSCORE, scorerev = false;
  }
#endif

#ifndef MOBILE
void handleScoreKeys(int sym, SDL_Event& ev) {
  if(sym == SDLK_LEFT || sym == SDLK_KP4 || sym == 'h' || sym == 'a')
    shiftScoreDisplay(-1);
  else if(sym == SDLK_RIGHT || sym == SDLK_KP6 || sym == 'l' || sym == 'd')
    shiftScoreDisplay(1);
  else if(sym == SDLK_UP || sym == 'k' || sym == 'w')
    scorefrom -= 5;
  else if(sym == SDLK_DOWN || sym == 'j' || sym == 'x')
    scorefrom += 5;
  else if(sym == 's') sortScores(); 
  else if(sym != 0 || ev.type == SDL_MOUSEBUTTONDOWN) cmode = emNormal;
  }
        
#endif

void drawStats() {

  DEB("stats");
  
#ifdef IOS
  if(cmode != emNormal && cmode != emQuit)
    return;
#endif
  
  int vx, vy;
  
  if(vid.xres < vid.yres) {
    vx = vid.fsize * 3;
    vy = vid.fsize * 2;
    }
  else {
    vx = vid.xres - vid.fsize * 3;
    vy = vid.fsize;
    }
  
  #define ADV(z) \
    if(vid.xres < vid.yres) { \
      vx += vid.fsize*4; \
      if(vx > vid.xres - vid.fsize*2) vx = vid.fsize * 3, vy += vid.fsize; \
      } \
    else vy += vid.fsize * z/2
  
  DEB("$$$");
  if(displaynum(vx, vy, 0, vid.fsize, 0xFFFFFF, gold(), "$$$")) {
    mouseovers = XLAT("Your total wealth"),
    help = XLAT(
MC    "The total value of the treasure you have collected.\n\n"
MC    "Every world type contains a specific type of treasure, worth 1 $$$; "
MC    "your goal is to collect as much treasure as possible, but every treasure you find "
MC    "causes more enemies to hunt you in its native land.\n\n"
M     "Orbs of Yendor are worth 50 $$$ each.\n\n"
      );
    }
      
  DEB("$$$Z");
  ADV(3);
  
  int oldz = 0;
  
  for(int z=0; z<3; z++) for(int i=0; i<ittypes; i++) if(itemclass(eItem(i)) == z) {
    #ifndef MOBILE
    if(z != oldz) { ADV(1); oldz = z; }
    #endif
    if(items[i])  {
      bool b = displaynum(vx, vy, 0, vid.fsize, iinf[i].color, items[i], s0 + iinf[i].glyph);
      ADV(2);
      if(b) {
        int t = itemclass(eItem(i));
        if(t == 0)
          mouseovers = XLAT("treasure collected: %1", eItem(i));
        if(t == 1)
          mouseovers = XLAT("objects found: %1", eItem(i));
        if(t == 2)
          mouseovers = XLAT("orb power: %1", eItem(i));
        if(i == itGreenStone) {
          mouseovers += XLAT(" (click to drop)");
          getcstat = 'g';
          }
        help = XLAT(iinf[i].help);
#ifndef ANDROID
        if(i == itOrbTeleport)
          help += XLAT("You can also scroll to the desired location and then press 't'.");
        if(i == itOrbSafety)
          help += XLAT("Thus, it is potentially useful for extremely long games, which would eat all the memory on your system otherwise.\n");
        if(i == itGreenStone)
          help += XLAT("You can touch the Dead Orb in your inventory to drop it.");
#else
        if(i == itOrbSafety)
          help += XLAT("This might be useful for Android devices with limited memory.");
        if(i == itGreenStone)
          help += XLAT("You can press 'g' or click them in the list to drop a Dead Orb.");
#endif
        }
      }
    }

  if(vid.xres < vid.yres) {
    vx = vid.fsize * 3;
    vy = vid.fsize * 5;
    }
  else {
    vx = vid.fsize * 3;
    vy = vid.fsize;
    }
  
  if(displaynum(vx, vy, 0, vid.fsize, 0xFFFFFF, calcfps(), "fps"))
    mouseovers = XLAT("frames per second"),
    help = 
      XLAT(
MC    "The higher the number, the smoother the animations in the game. "
MC    "If you find that animations are not smooth enough, you can try "
M     "to change the options "
      ) +
#ifdef ANDROID
XLAT(
MC    "(Menu button) and select the ASCII mode, which runs much faster. "
MC    "Depending on your device, turning the OpenGL mode on or off might "
M     "make it faster, slower, or cause glitches.");
#else
#ifdef IOS
XLAT(
MC    "(in the MENU). You can reduce the sight range, this should make "
M     "the animations smoother.");
#else
XLAT(
MC    "(press v) and change the wall/monster mode to ASCII, or change "
M     "the resolution.");
#endif
// todo: iOS
#endif
    
  ADV(3);
  
  for(int i=1; i<motypes; i++) if(kills[i])  {
    if(displaynum(vx, vy, 0, vid.fsize, minf[i].color, kills[i], s0 + minf[i].glyph))
      mouseovers = s0 + XLAT("monsters killed: %1", eMonster(i)),
      help = XLAT(minf[i].help);
    ADV(2);
    }
  
  DEB("stats OK");  
  achievement_display();
  }
  
void drawscreen() {

#ifdef GL
  if(vid.usingGL) setGLProjection();
#endif
  
  calcparam();
  if(cmode != emHelp) help = "@";
  
  #ifndef MOBILE
  // SDL_LockSurface(s);
  // unsigned char *b = (unsigned char*) s->pixels;
  // int n = vid.xres * vid.yres * 4;
  // while(n) *b >>= 1, b++, n--;
  // memset(s->pixels, 0, vid.xres * vid.yres * 4);
  if(!vid.usingGL) SDL_FillRect(s, NULL, 0);
  #endif
  
  if(!canmove) darken = 1;
  if(cmode != emNormal && cmode != emDraw && cmode != emCustomizeChar) darken = 2;
  
  if(!vid.goteyes && !euclid)
    drawCircle(vid.xcenter, vid.ycenter, vid.radius, 0xFF);
    
  if(vid.wallmode < 2 && !euclid && !(vid.boardmode != 'h' && cwt.c->land == laGameBoard)) {
    int ls = size(lines);
    if(ISMOBILE) ls /= 10;
    for(int t=0; t<ls; t++) drawline(View * lines[t].P1, View * lines[t].P2, lines[t].col >> darken);
    }

  drawqueue(); 
  ptds.clear();
  
  DEB("dmap");
    
  drawthemap();
  
  DEB("mstar");
  #ifndef MOBILE
  if(cmode == emNormal) drawmovestar();
  #endif
  
  if(cmode == emDraw) {
    lalpha = 0x20;
    for(int d=0; d<84; d++) 
      drawline(C0, spin(M_PI*d/42)* xpush(crossf) * C0, 0xC0C0C0);
    for(int d=0; d<84; d++) for(int u=2; u<=20; u++)
      drawline(spin(M_PI*d/42)* xpush(crossf*u/20) * C0, spin(M_PI*(d+1)/42)* xpush(crossf*u/20) * C0, 0xC0C0C0);

    lalpha = 0x80;
    if(crad > 0) {
      transmatrix movtocc = rgpushxto0(ccenter);
      for(int d=0; d<84; d++) 
        drawline(movtocc * ddi(d+1, crad) * C0, movtocc * ddi(d, crad) * C0, 0x00C000);
      }
      // spin(M_PI*(d+1)/42) * xpush(crad) * spin(-M_PI*(d+1)/42) * ccenter, spin(M_PI*d/42) * xpush(crad) * spin(-M_PI*d/42) * ccenter, 0xC0C0C0);
    lalpha = 0xFF;
    }

  getcstat = 0;
  
  drawStats();

  #ifdef MOBILE
  
  if(cmode == emNormal) {
    displayabutton(-1, -1, XLAT("MOVE"),  andmode == 0 ? BTON : BTOFF);
    displayabutton(+1, -1, XLAT(andmode == 1 ? "BACK" : "DRAG"),  andmode == 1 ? BTON : BTOFF);
    displayabutton(-1, +1, XLAT("INFO"),  andmode == 2 ? BTON : BTOFF);
    displayabutton(+1, +1, XLAT(ISIOS ? "MENU" : andmode == 3 ? "QUEST" : "HELP"), 
      andmode == 3 ? (ISIOS ? BTON : 0xFF00FF) : BTOFF);
    }
  
  if(cmode == emQuit) {
    displayabutton(-1, +1, XLAT("NEW"), BTON);
    displayabutton(+1, +1, XLAT(canmove ? "PLAY" : ISIOS ? " " : "SHARE"), BTON);
    }
  #endif
  
  if(cmode == emDraw) {
    mouseovers = 
      floordraw ? (cwt.c->type == 6 ? "hex floor" : "hepta floor") : "character";
    
    mouseovers = 
      XLAT("Drawing %1 (layer %2), F1 for help", XLAT(mouseovers), its(dslayer));
    }
 
  // displaynum(vx,100, 0, 24, 0xc0c0c0, celldist(cwt.c), ":");

  darken = 0;
  drawmessages();
  
    DEB("msgs1");
  if(cmode == emNormal) {
    #ifdef MOBILE
    if(!canmove) cmode = emQuit;
    #endif
    if(!canmove) showGameover();
    #ifndef MOBILE
    if(!canmove)
      displayButton(vid.xres-8, vid.yres-vid.fsize*2, XLAT("ESC for menu/quest"), SDLK_ESCAPE, 16);
    #endif
    }
  
  #ifndef ANDROID
  if(cmode == emScores)
    showScores();
  #endif
  
  #ifndef MOBILE
  if(cmode == emChangeMode) {
    displayStat(2, XLAT("vector graphics editor"), "", 'g');
    displayStat(3, XLAT("Euclidean mode"), "", 'e');
    displayStat(4, XLAT("cheat mode"), "", 'c');
    displayStat(6, XLAT("heptagonal game board"), "", '7');
    displayStat(7, XLAT("triangular game board"), "", '3');
    displayStat(8, XLAT("HyperRogue game board"), "", 'h');

    displayStat(18, XLAT("first page [Space]"), "", ' ');
    displayStat(19, XLAT("exit configuration"), "", 'v');
    }

  if(cmode == emCustomizeChar) {
    displayStatHelp(0, XLAT("Customize character"));

    displayStat(2, XLAT("gender"), XLAT(vid.female ? "female" : "male"), 'g');
    displayStat(3, XLAT("skin color"), "?", 's');
    displayStat(4, XLAT("weapon color"), "?", 'w');
    displayStat(5, XLAT("hair color"), "?", 'h');
    if(vid.female)
      displayStat(6, XLAT("dress color"), "?", 'd');

    displayStatHelp(16, XLAT("Shift=random, Ctrl=mix"));

    displayStat(19, XLAT("exit configuration"), "", 'v');
    }

  if(cmode == emPickEuclidean) {
    displayStatHelp(0, XLAT("Euclidean mode"));
    landvisited[laCrossroads] = true;
    landvisited[laIce] = true;
    landvisited[laMirror] = true;
    for(int i=2; i<landtypes; i++) if(landvisited[i])
      displayStat(i, XLAT1(linf[i].name), "", 'a' + (i-2));
    displayStat(landtypes+1, XLAT("Return to the hyperbolic world"), "", 'z');
    displayStatHelp(landtypes+3, XLAT("Choose from the lands visited this game."));
    displayStatHelp(landtypes+5, XLAT("Scores and achievements are not"));
    displayStatHelp(landtypes+6, XLAT("saved in the Euclidean mode!"));
    }
    
  if(cmode == emVisual1 || cmode == emVisual2) {
    displayStatHelp(0, XLAT("Configuration:"));
    
    if(cmode == emVisual1) {
      displayStat(2, XLAT("video resolution"), its(vid.xres) + "x"+its(vid.yres), 'r');
      displayStat(3, XLAT("fullscreen mode"), vid.full ? "ON" : "OFF", 'f');
      displayStat(4, XLAT("animation speed"), fts(vid.aspeed), 'a');
      displayStat(5, XLAT("dist from hyperboloid ctr"), fts(vid.alpha), 'p');
      displayStat(6, XLAT("scale factor"), fts(vid.scale), 'z');
  
      const char *wdmodes[4] = {"ASCII", "black", "plain", "Escher"};
      const char *mdmodes[3] = {"ASCII", "items only", "items and monsters"};
      const char *axmodes[4] = {"no axes", "auto", "light", "heavy"};
      
      displayStat(8, XLAT("wall display mode"), 
        XLAT(wdmodes[vid.wallmode]), 'w');
      displayStat(9, XLAT("monster display mode"), 
        XLAT(mdmodes[vid.monmode]), 'm');
      displayStat(10, XLAT("cross display mode"), 
        XLAT(axmodes[vid.axes]), 'c');
      displayStat(11, XLAT("background music volume"), 
        its(audiovolume), 'b');
      displayStat(13, XLAT("language"), XLAT("EN"), 'l');
      displayStat(14, XLAT("player character"), 
        XLAT(vid.female ? "female" : "male"), 'g');
      }
    else {
      #ifdef GL
      displayStat(2, XLAT("openGL & antialiasing mode"), vid.usingGL ? "OpenGL" : vid.usingAA ? "AA" : "OFF", 'o');
      #endif
      displayStat(3, XLAT("distance between eyes"), fts(vid.eye * 10), 'e');
      displayStat(4, XLAT("framerate limit"), its(vid.framelimit), 'f');

      displayStat(6, XLAT("joystick mode"), XLAT(autojoy ? "automatic" : "manual"), 'p');
        
      displayStat(8, XLAT("first joystick: movement threshold"), its(vid.joyvalue), 'a');
      displayStat(9, XLAT("first joystick: execute movement threshold"), its(vid.joyvalue2), 'b');
      displayStat(10, XLAT("second joystick: pan threshold"), its(vid.joypanthreshold), 'c');
      displayStat(11, XLAT("second joystick: panning speed"), fts(vid.joypanspeed * 1000), 'd');
      displayStat(13, XLAT("message flash time"), its(vid.flashtime), 't');
      }

    displayStatHelp(16, XLAT("use Shift to decrease and Ctrl to fine tune "));
    displayStatHelp(17, XLAT("(e.g. Shift+Ctrl+Z)"));

    if(cmode == emVisual1)
      displayStat(18, XLAT("the second page [Space]"), "", ' ');
    else
      displayStat(18, XLAT("special features [Space]"), "", ' ');

    displayStat(19, XLAT("exit configuration"), "", 'v');
    displayStat(20, XLAT("see the help screen"), "", 'h');
    displayStat(21, XLAT("save the current config"), "", 's');
    }
  #endif
  
  describeMouseover();

  #ifndef MOBILE
  if(cmode == emNormal || cmode == emVisual1 || cmode == emVisual2 || cmode == emChangeMode)
    displayButton(vid.xres-8, vid.yres-vid.fsize, XLAT("(v) config"), 'v', 16);
  #endif
  
  if(cmode == emQuit) {
    showGameover();
    }

#ifndef ANDROID
  if(cmode == emHelp) {
    int last = 0;
    int lastspace = 0;
    int siz = vid.fsize;
    
    if(help == "@") help = buildHelpText();

    if(ISIOS && size(help) >= 500) siz = (siz * 9)/10;
    else if(ISIOS) siz = (siz * 3+1)/2;
    else if(size(help) >= 500) siz = siz * 2/3;
    
    int vy = vid.fsize * 4;
    
    int xs = vid.xres * 618/1000;
    int xo = vid.xres * 186/1000;
    
    for(int i=0; i<=size(help); i++) {
      int ls = 0;
      int prev = last;
      if(help[i] == ' ') lastspace = i;
      if(textwidth(siz, help.substr(last, i-last)) > xs) {
        if(lastspace == last) ls = i-1, last = i-1;
        else ls = lastspace, last = ls+1;
        }
      if(help[i] == 10 || i == size(help)) ls = i, last = i+1;
      if(ls) {
        displayfr(xo, vy, 2, siz, help.substr(prev, ls-prev), 0xC0C0C0, 0);
        if(ls == prev) vy += siz/2;
        else vy += siz;
        lastspace = last;
        }
      }
    }
#endif

#ifndef MOBILE  
  if(items[itOrbTeleport] && mouseover && cwt.c != mouseover && !isNeighbor(cwt.c, mouseover))
    displaychr(mousex, mousey, 0, vid.fsize, '@', iinf[itOrbTeleport].color);
#endif
  
  #ifndef MOBILE
  DEB("msgs3");
  // SDL_UnlockSurface(s);

#ifdef GL
  if(vid.usingGL) SDL_GL_SwapBuffers(); else
#endif
  SDL_UpdateRect(s, 0, 0, vid.xres, vid.yres);

  #endif
  
  if(playermoved && vid.aspeed > 4.99) { 
    centerpc(1000);
    playermoved = false; 
    return; 
    }

  }

#ifndef MOBILE
bool setfsize = false;

void setvideomode() {  
  
  if(!vid.full) {
    if(vid.xres > vid.xscr) vid.xres = vid.xscr * 9/10, setfsize = true;
    if(vid.yres > vid.yscr) vid.yres = vid.yscr * 9/10, setfsize = true;    
    }
  
  if(setfsize) vid.fsize = min(vid.yres / 32, vid.xres / 48), setfsize = false;

  int flags = 0;
  
#ifdef GL
  if(vid.usingGL) {
    flags = SDL_OPENGL | SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER;
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }
#endif

  int sizeflag = (vid.full ? SDL_FULLSCREEN : SDL_RESIZABLE);
  
  s= SDL_SetVideoMode(vid.xres, vid.yres, 32, flags | sizeflag);
  
  if(vid.full && !s) {
    vid.xres = vid.xscr;
    vid.yres = vid.yscr;
    vid.fsize = min(vid.yres / 32, vid.xres / 48);
    s = SDL_SetVideoMode(vid.xres, vid.yres, 32, flags | SDL_FULLSCREEN);
    }

  if(!s) {
    addMessage("Failed to set the graphical mode: "+its(vid.xres)+"x"+its(vid.yres)+(vid.full ? " fullscreen" : " windowed"));
    vid.xres = 640;
    vid.yres = 480;
    s = SDL_SetVideoMode(vid.xres, vid.yres, 32, flags | SDL_RESIZABLE);
    }

#ifdef GL
  glViewport(0, 0, vid.xres, vid.yres);
  resetGL();
#endif
  }
#endif

void restartGraph() {
  viewctr.h = &origin;
  viewctr.spin = 0;
  View = Id;
  }

#ifndef ANDROID
void saveConfig() {
  FILE *f = fopen(conffile, "wt");
  if(!f) {
    addMessage(s0 + "Could not open the config file: " + conffile);
    return;
    }
  fprintf(f, "%d %d %d %d\n", vid.xres, vid.yres, vid.full, vid.fsize);
  fprintf(f, "%f %f %f %f\n", float(vid.scale), float(vid.eye), float(vid.alpha), float(vid.aspeed));
  fprintf(f, "%d %d %d %d %d %d %d\n", vid.wallmode, vid.monmode, vid.axes, audiovolume, vid.framelimit, vid.usingGL, vid.usingAA);
  fprintf(f, "%d %d %d %f %d %d\n", vid.joyvalue, vid.joyvalue2, vid.joypanthreshold, vid.joypanspeed, autojoy, vid.flashtime);
  fprintf(f, "%d %d %08x %08x %08x %08x\n", 
    vid.female?1:0, vid.language, vid.skincolor, vid.haircolor, vid.swordcolor, vid.dresscolor);

  fprintf(f, "\n\nThis is a configuration file for HyperRogue (version "VER")\n");
  fprintf(f, "\n\nThe numbers are:\n");
  fprintf(f, "screen width & height, fullscreen mode (0=windowed, 1=fullscreen), font size\n");
  fprintf(f, "scale, eye distance, parameter, animation speed\n");
  fprintf(f, "wallmode, monster mode, cross mode, audiovolume, framerate limit, usingGL, usingAA\n");
  fprintf(f, "calibrate first joystick (threshold A, threshold B), calibrate second joystick (pan threshold, pan speed), joy mode\n");
  fprintf(f, "gender (1=female), language, skin color, hair color, sword color, dress color\n");
  
  fclose(f);
  addMessage(s0 + "Configuration saved to: " + conffile);
  }

void loadConfig() {
  vid.xres = 9999; vid.yres = 9999; vid.framelimit = 300;
  FILE *f = fopen(conffile, "rt");
  if(f) {
    int fs, gl=1, aa=1;
    int err;
    err=fscanf(f, "%d%d%d%d", &vid.xres, &vid.yres, &fs, &vid.fsize);
    vid.full = fs;
    float a, b, c, d;
    err=fscanf(f, "%f%f%f%f\n", &a, &b, &c, &d);
    err=fscanf(f, "%d%d%d%d%d%d%d", &vid.wallmode, &vid.monmode, &vid.axes, &audiovolume, &vid.framelimit, &gl, &aa);
    vid.usingGL = gl; vid.usingAA = aa;
    err=fscanf(f, "%d%d%d%f%d%d", &vid.joyvalue, &vid.joyvalue2, &vid.joypanthreshold, &vid.joypanspeed, &aa, &vid.flashtime);
    autojoy = aa; aa = 0;
    err=fscanf(f, "%d%d%x%x%x%x", &aa, &vid.language, &vid.skincolor, &vid.haircolor, &vid.swordcolor, &vid.dresscolor);
    vid.female = aa;

    if(err);
    vid.scale = a; vid.eye = b; vid.alpha = c; vid.aspeed = d;
    fclose(f);
    printf("Loaded configuration: %s\n", conffile);
    }

#ifndef MOBILE
  if(clWidth) vid.xres = clWidth;
  if(clHeight) vid.yres = clHeight;
  if(clFont) vid.fsize = clFont;
  
  for(int k=0; k<int(commandline.size()); k++) switch(commandline[k]) {
    case 'f': vid.full = true; break;
    case 'w': vid.full = false; break;
    case 'e': vid.wallmode = 3; vid.monmode = 2; break;
    case 'p': vid.wallmode = 2; vid.monmode = 2; break;
    case 'a': vid.wallmode = 0; vid.monmode = 0; break;
    case 'o': vid.usingGL = !vid.usingGL; break;
    }
#endif
  }
#endif

#ifndef MOBILE
string musfname[landtypes];

bool loadMusicInfo(string dir) {
  if(dir == "") return false;
  FILE *f = fopen(dir.c_str(), "rt");
  if(f) {
    string dir2;
    for(int i=0; i<size(dir); i++) if(dir[i] == '/' || dir[i] == '\\')
      dir2 = dir.substr(0, i+1);
    char buf[1000];
    while(fgets(buf, 800, f) > 0) {
      for(int i=0; buf[i]; i++) if(buf[i] == 10 || buf[i] == 13) buf[i] = 0;
      if(buf[0] == '[' && buf[3] == ']') {
        int id = (buf[1] - '0') * 10 + buf[2] - '0';
        if(id >= 0 && id < landtypes) {
          if(buf[5] == '*' && buf[6] == '/') musfname[id] = dir2 + (buf+7);
          else musfname[id] = buf+5;
          }
        else {
          fprintf(stderr, "warning: bad soundtrack id, use the following format:\n");
          fprintf(stderr, "[##] */filename\n");
          fprintf(stderr, "where ## are two digits, and */ is optional and replaced by path to the music\n");
          fprintf(stderr, "alternatively LAST = reuse the last track instead of having a special one");
          }
        // printf("[%2d] %s\n", id, buf);
        }
      else if(buf[0] == '#') {
        }
      else {
        musiclicense += buf;
        musiclicense += "\n";
        }
      }
    fclose(f);
    return true;
    }
  return false;
  }
#endif

void initgraph() {

  vid.usingGL = true;
  vid.usingAA = true;
  vid.flashtime = 8;
  vid.scale = 1;
  vid.alpha = 1;
  vid.aspeed = 0;
  vid.eye = 0;
  vid.full = false;
  vid.quick = true;
#ifdef ANDROID
  vid.wallmode = 2;
#else
  vid.wallmode = 3;
#endif

  vid.joyvalue = 4800;
  vid.joyvalue2 = 5600;
  vid.joypanthreshold = 2500;
  vid.joypanspeed = 0.0001;

  vid.framelimit = 75;
  vid.monmode = 2;
  vid.axes = 1;
  
  vid.skincolor = 0xD0D0D0FF;
  vid.haircolor = 0x686868FF;
  vid.dresscolor= 0xC00000FF;
  vid.swordcolor= 0xD0D0D0FF;
  
  vid.female = false;
  vid.language = -1;
  vid.boardmode = 'h';
  
  joyx = joyy = 0; joydir = -1;

  restartGraph();
  
  initgeo();

  buildpolys();
  for(int i=0; i<8; i++) 
    dsUser[i][0].rots = 6,
    dsUser[i][1].rots = 7,
    dsUser[i][2].rots = 1, dsUser[i][2].sym = true;

  #ifndef MOBILE  
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1)
  {
    printf("Failed to initialize video.\n");
    exit(2);
  }
  const SDL_VideoInfo *inf = SDL_GetVideoInfo();
  vid.xscr = vid.xres = inf->current_w;
  vid.yscr = vid.yres = inf->current_h;
  
  SDL_WM_SetCaption("HyperRogue " VER, "HyperRogue "VER);
  
  loadConfig();

  setvideomode();
  if(!s) {
    printf("Failed to initialize graphics.\n");
    exit(2);
    }
    
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(1);
  
  if(TTF_Init() != 0) {
    printf("Failed to initialize TTF.\n");
    exit(2);
    }
  
  if(SDL_NumJoysticks()) stick = SDL_JoystickOpen(0);
  if(SDL_NumJoysticks() >= 2) panstick = SDL_JoystickOpen(1);

  #ifdef AUDIO

  audio = 
    loadMusicInfo(musicfile)
    || loadMusicInfo("./hyperrogue-music.txt") 
    || loadMusicInfo("music/hyperrogue-music.txt")
#ifdef FHS
    || loadMusicInfo("/usr/share/hyperrogue/hyperrogue-music.txt") 
    || loadMusicInfo(s0 + getenv("HOME") + "/.hyperrogue-music.txt")
#endif
    ;

  if(audio) {
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
      fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
      audio = false;
      }
    else {
      audio = true;
      Mix_AllocateChannels(4);
      }
    }
  #endif
    
  #endif
  }

int frames;
bool outoffocus = false;

#ifdef AUDIO

bool loaded[landtypes];
Mix_Music* music[landtypes];
int musicpos[landtypes];
int musstart;
int musfadeval = 2000;

eLand cid = laNone;

void handlemusic() {
  if(audio && audiovolume) {
    eLand id = cwt.c->land;
    if(outoffocus) id = eLand(0);
    if(musfname[id] == "LAST") id = cid;
    if(!loaded[id]) {
      loaded[id] = true;
      // printf("loading (%d)> %s\n", id, musfname[id].c_str());
      if(musfname[id] != "") {
        music[id] = Mix_LoadMUS(musfname[id].c_str());
        if(!music[id]) {
           printf("Mix_LoadMUS: %s\n", Mix_GetError());
           }
        }
      }
    if(cid != id && !musfadeval) {
      musicpos[cid] = SDL_GetTicks() - musstart;
      musfadeval = musicpos[id] ? 500 : 2000;
      Mix_FadeOutMusic(musfadeval);
      // printf("fadeout %d, pos %d\n", musfadeval, musicpos[cid]);
      }
    if(music[id] && !Mix_PlayingMusic()) {
      cid = id;
      Mix_VolumeMusic(audiovolume);
      Mix_FadeInMusicPos(music[id], -1, musfadeval, musicpos[id] / 1000.0);
      // printf("fadein %d, pos %d\n", musfadeval, musicpos[cid]);
      musstart = SDL_GetTicks() - musicpos[id];
      musicpos[id] = 0;
      musfadeval = 0;
      }
    }
  }


void resetmusic() {
  if(audio && audiovolume) {
    Mix_FadeOutMusic(3000);
    cid = laNone;
    for(int i=0; i<landtypes; i++) musicpos[i] = 0;
    musfadeval = 2000;
    }
  }

#else
void resetmusic() {}
#endif

void panning(hyperpoint hf, hyperpoint ht) {
  View = 
    rgpushxto0(hf) * rgpushxto0(gpushxto0(hf) * ht) * gpushxto0(hf) * View;
  playermoved = false;
  }

#ifdef SHOWOFF
#include "showoff.cpp"
#endif

void switchcolor(int& c, int* cs, int mod) {
  int id = 0;
  int q = cs[0]; cs++;
  for(int i=0; i<q; i++) if(c == cs[i]) id = i;
  if(mod == 1)
    c = ((rand() % 0x1000000) << 8) | 0xFF;
  else if(mod == 2)
    c = (gradient(cs[rand() % q] >> 8, cs[rand() % q] >> 8, 0, rand() % 101, 100) << 8) + 0xFF;
  else
    c = cs[(id+1) % q];
  }

const char* drawhelp = 
 "In this mode you can draw your own player character and floors. "
 "Mostly for the development purposes, but you can have fun too.\n\n"
 "f - floor, p - player (repeat 'p' for layers)\n\n"
 "n - new shape, u - copy the 'player body' shape\n\n"
 "1-9 - rotational symmetries, 0 - toggle axial symmetry\n\n"
 "point with mouse and: a - add point, m - move nearest point, d - delete nearest point, c - nearest point again, b - add after nearest\n\n"
 "s - save in C++ format (but cannot be loaded yet without editing source)\n\n"
 "z - zoom, o - Poincaré model\n";
 
#ifndef MOBILE

// Warning: a very long function! todo: refactor

void mainloop() {
  int lastt = 0;
  while(true) {

    #ifndef GFX
    #ifndef GL
    vid.wallmode = 0;
    vid.monmode = 0;
    #endif
    #endif
    
    DEB("screen");
    optimizeview();
    ticks = SDL_GetTicks();
      
    dslayer %= 8;
    dsCur = &(dsUser[dslayer][floordraw ? cwt.c->type-6 : 2]);
    
    int cframelimit = vid.framelimit;
    if((cmode == emVisual1 || cmode == emVisual2 || cmode == emHelp || cmode == emQuit) && cframelimit > 15)
      cframelimit = 15;
    if(outoffocus && cframelimit > 10) cframelimit = 10;
    
    int timetowait = lastt + 1000 / cframelimit - ticks;
    if(timetowait > 0)
      SDL_Delay(timetowait);
    else {
      if(playermoved && vid.aspeed > -4.99 && !outoffocus)
        centerpc((ticks - lastt) / 1000.0 * exp(vid.aspeed));
     if(panstick || panjoyx || panjoyy) 
        checkpanjoy((ticks - lastt) / 1000.0);
      lastt = ticks;
      frames++;
      if(!outoffocus) drawscreen();
      }      

    Uint8 *keystate = SDL_GetKeyState(NULL);
    rightclick = keystate[SDLK_RCTRL];
    leftclick = keystate[SDLK_RSHIFT];

#ifdef AUDIO
    if(audio) handlemusic();
#endif
    SDL_Event ev;
    DEB("react");
    
    while(SDL_PollEvent(&ev)) {
      int sym = 0;
      int uni = 0;
      ld shift = 1;

      if(ev.type == SDL_ACTIVEEVENT) {
        if(ev.active.state & SDL_APPINPUTFOCUS) {
          if(ev.active.gain) {
            outoffocus = false;
            }
          else {
            outoffocus = true;
            }
          }
        }
      
      if(ev.type == SDL_VIDEORESIZE) {
        vid.xres = ev.resize.w;
        vid.yres = ev.resize.h;
        setfsize = true;
        setvideomode();
#ifdef GL
        glViewport(0, 0, vid.xres, vid.yres);
#endif
        }
      
      if(ev.type == SDL_VIDEOEXPOSE) {
        drawscreen();
        }
      
      if(ev.type == SDL_JOYAXISMOTION) {
        if(ev.jaxis.value != 0 &&
          ev.jaxis.axis >= 2)
        printf("which = %d axis = %d value = %d\n",
          ev.jaxis.which,
          ev.jaxis.axis,
          ev.jaxis.value);
        if(ev.jaxis.which == 0) {
          if(ev.jaxis.axis == 0)
            joyx = ev.jaxis.value;
          else if(ev.jaxis.axis == 1)
            joyy = ev.jaxis.value;
          else if(ev.jaxis.axis == 3)
            panjoyx = ev.jaxis.value;
          else if(ev.jaxis.axis == 4)
            panjoyy = ev.jaxis.value;
          checkjoy();
          printf("panjoy = %d,%d\n", panjoyx, panjoyy);
          }
        else {
          if(ev.jaxis.axis == 0)
            panjoyx = ev.jaxis.value;
          else 
            panjoyy = ev.jaxis.value;
          }
        }

      if(ev.type == SDL_JOYBUTTONDOWN) {
        movepcto(joydir);
        checkjoy();
        }

      if(ev.type == SDL_KEYDOWN) {
        flashMessages();
        mousing = false;
        sym = ev.key.keysym.sym;
        uni = ev.key.keysym.unicode;
        if(ev.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) shift = -1;
        if(ev.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) shift /= 10;
        }

      if(ev.type == SDL_MOUSEBUTTONDOWN) {
        flashMessages();
        mousepressed = true;
        mousing = true;
        if(ev.button.button==SDL_BUTTON_RIGHT || leftclick) 
          sym = SDLK_F1;
        else if(ev.button.button==SDL_BUTTON_MIDDLE || rightclick) 
          sym = 1;
        else if(ev.button.button==SDL_BUTTON_WHEELUP) {  
          ld jx = (mousex - vid.xcenter - .0) / vid.radius / 10;
          ld jy = (mousey - vid.ycenter - .0) / vid.radius / 10;
          playermoved = false;
          View = gpushxto0(hpxy(jx, jy)) * View;
          sym = 1;
          }
        else {
          sym = getcstat, uni = getcstat, shift = getcshift;
          }
        }

      if(ev.type == SDL_MOUSEBUTTONUP) 
        mousepressed = false;

      if(cmode != emScores) {
#ifndef PANDORA
        if(sym == SDLK_RIGHT) View = xpush(-0.2*shift) * View, playermoved = false;
        if(sym == SDLK_LEFT) View = xpush(+0.2*shift) * View, playermoved = false;
        if(sym == SDLK_UP) View = ypush(+0.2*shift) * View, playermoved = false;
        if(sym == SDLK_DOWN) View = ypush(-0.2*shift) * View, playermoved = false;
#endif
        if(sym == SDLK_PAGEUP && !euclid) View = spin(M_PI/21*shift) * View;
        if(sym == SDLK_PAGEDOWN && !euclid) View = spin(-M_PI/21*shift) * View;
        }
      
      if(ev.type == SDL_MOUSEMOTION) {
        hyperpoint mouseoh = mouseh;
        
        mousing = true;
        mousex = ev.motion.x;
        mousey = ev.motion.y;
        mouseh = gethyper(mousex, mousey);
        
        if((rightclick || (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MMASK)) && 
          !outofmap(mouseh) && !outofmap(mouseoh) && 
           mouseh[2] < 50 && mouseoh[2] < 50) {
          panning(mouseoh, mouseh);
          }

#ifdef SIMULATE_JOYSTICK
        // pretend that both joysticks are present
        stick = panstick = (SDL_Joystick*) (&vid);
        panjoyx = 20 * (mousex - vid.xcenter);
        panjoyy = 20 * (mousey - vid.ycenter);
        checkjoy();
#endif
        }

      DEB("r1");
      if(sym == SDLK_F7) {

        time_t timer;
        timer = time(NULL);
        char buf[128]; strftime(buf, 128, "shot-%y%m%d-%H%M%S.bmp", localtime(&timer));

        SDL_SaveBMP(s, buf);
        addMessage(XLAT("Screenshot saved to %1", buf));
        }

      DEB("r2");
      
      if(cmode == emDraw) {
      
        hyperpoint mh = inverse(playertrans) * mouseh;
        // inverse(playertrans) * 
        // mouseh;
        
        // spin(-M_PI/2) * mouseh;
        
        if(uni == 'n') {
          dsCur->list.clear();
          dsCur->list.push_back(mh);
#ifdef GL
          GL_initialized = false;
#endif
          }

        if(uni == 'a') {
          dsCur->list.push_back(mh);
#ifdef GL
          GL_initialized = false;
#endif
          }

        if(uni == 'u') {
          dsCur = &(dsUser[0][2]);
          dsCur->list.clear();
          for(int i=shPBody.s; i < (shPBody.s + shPBody.e)/2; i++)
            dsCur->list.push_back(hpc[i]);

          dsCur = &(dsUser[1][2]);
          dsCur->sym = false;
          dsCur->list.clear();
          for(int i=shPSword.s; i < shPSword.e; i++)
            dsCur->list.push_back(hpc[i]);

          dsCur = &(dsUser[2][2]);
          dsCur->list.clear();
          for(int i=shPHead.s; i < (shPHead.s + shPHead.e)/2; i++)
            dsCur->list.push_back(hpc[i]);

          dsCur = &(dsUser[3][2]);
          dsCur->list.clear();
          for(int i=shPFace.s; i < (shPFace.s + shPFace.e)/2; i++)
            dsCur->list.push_back(hpc[i]);

#ifdef GL
          GL_initialized = false;
#endif
          }

        if(uni == 'm' || uni == 'd' || uni == 'c' || uni == 'b') {
          int i = 0;
          if(size(dsCur->list) < 2) continue;
          for(int j=1; j<size(dsCur->list); j++) 
            if(intval(mh, dsCur->list[j]) < intval(mh, dsCur->list[i]))
              i = j;
          if(uni == 'm') 
            dsCur->list[i] = mh;
          if(uni == 'd')
            dsCur->list.erase(dsCur->list.begin() + i);
          if(uni == 'c')
            dsCur->list.push_back(dsCur->list[i]);
          if(uni == 'b') {
            while(i) {
              dsCur->list.push_back(dsCur->list[0]);
              dsCur->list.erase(dsCur->list.begin());
              i--;
              }
            }
#ifdef GL
          GL_initialized = false;
#endif
          }

        if(uni == 'f') {
          if(floordraw) dslayer++; else floordraw = true;
          }
        if(uni == 'p') {
          if(!floordraw) dslayer++; else floordraw = false;
          }
        if(uni == 'g') ccenter = mouseh, crad += shift / 20;
        
        if(uni == '1') dsCur->rots = 1;
        if(uni == '2') dsCur->rots = 2;
        if(uni == '3') dsCur->rots = 3;
        if(uni == '4') dsCur->rots = 4;
        if(uni == '5') dsCur->rots = 5;
        if(uni == '6') dsCur->rots = 6;
        if(uni == '7') dsCur->rots = 7;
        if(uni == '8') dsCur->rots = 8;
        if(uni == '9') dsCur->rots = 21;
        if(uni == '0') dsCur->sym = !dsCur->sym;

        if(uni == 's') {
          for(int i=prehpc; i < qhpc; i++) {
            for(int k=0; k<8; k++)
            for(int j=0; j<3; j++) if(i == shUser[k][j].s) 
              printf("\n  // group %d layer %d\n\n", j, k);
            printf("  hpcpush(hpxyz(%f,%f,%f));\n", double(hpc[i][0]), double(hpc[i][1]), double(hpc[i][2]));
            }
          }

        if(uni == 'z') vid.alpha = -.5;
        if(uni == 'o') vid.alpha = 1;

        saveImages();

        if(sym == SDLK_F6) {
          cmode = emNormal;
          sym = 0;
          }

        if(sym == SDLK_ESCAPE) cmode = emNormal;

        if(sym == SDLK_F1) {
          cmode = emHelp;
          sym = 0;
          help = XLAT(drawhelp);
          }

        if(sym == SDLK_F2) {
          cmode = emVisual1;
          sym = 0;
          }

        if(sym == SDLK_F10) cmode = emNormal;
        }
      
      if(cmode == emNormal) {
      
        if(!(uni >= 'A' && uni <= 'Z') && cwt.c->land != laGameBoard) {
          if(sym == 'l' || sym == 'd' || sym == SDLK_KP6) movepckeydir(0);
          if(sym == 'n' || sym == 'c' || sym == SDLK_KP3) movepckeydir(1);
          if(sym == 'j' || sym == 'x' || sym == SDLK_KP2) movepckeydir(2);
          if(sym == 'b' || sym == 'z' || sym == SDLK_KP1) movepckeydir(3);
          if(sym == 'h' || sym == 'a' || sym == SDLK_KP4) movepckeydir(4);
          if(sym == 'y' || sym == 'q' || sym == SDLK_KP7) movepckeydir(5);
          if(sym == 'k' || sym == 'w' || sym == SDLK_KP8) movepckeydir(6);
          if(sym == 'u' || sym == 'e' || sym == SDLK_KP9) movepckeydir(7);
          }

#ifdef PANDORA
        if(sym == SDLK_RIGHT) movepckeydir(0);
        if(sym == SDLK_LEFT) movepckeydir(4);
        if(sym == SDLK_DOWN) movepckeydir(2 + (leftclick?1:0) - (rightclick?1:0));
        if(sym == SDLK_UP) movepckeydir(6 - (leftclick?1:0) + (rightclick?1:0));
#endif

        if(cheater) applyCheat(uni, mouseover);
        if(cwt.c->land == laGameBoard)
          applyGameBoard(uni, sym, mouseover);
        else {
          if(sym == '.' || sym == 's') movepcto(-1);
          if(sym == SDLK_DELETE || sym == SDLK_KP_PERIOD || sym == 'g') movepcto(-2);
          if(sym == 't') {
            if(!items[itOrbTeleport])
              addMessage(XLAT("You need an Orb of Teleport to teleport."));
            else if(centerover && cwt.c != centerover && !isNeighbor(cwt.c, centerover))
              teleportpc(centerover);
            else
              addMessage(XLAT("Use arrow keys to choose the teleport location."));
            }
          }

        if(sym == SDLK_KP5) movepcto(-1);

        // if(sym == SDLK_F4)  restartGameSwitchEuclid();

        if(sym == SDLK_F5)  restartGame();
        if(sym == SDLK_ESCAPE) {
          cmode = emQuit;
          achievement_final(false);
          msgscroll = 0;
          }
        if(sym == SDLK_F10) return;
        
        if(!canmove) {
          if(sym == SDLK_RETURN) return;
          else if(uni == 'r') restartGame();
          else if(uni == 't') {
            restartGame();
            loadScores();
            }
          }
        
        if(sym == SDLK_HOME || sym == SDLK_F3 || sym == ' ') {
          if(playerfound) centerpc(INF);
          else {
            View = Id;
            // EUCLIDEAN
            if(!euclid) viewctr.h = cwt.c->master;
            // SDL_LockSurface(s);
            drawthemap(); 
            // SDL_UnlockSurface(s);
            centerpc(INF);
            }
          playermoved = true;
          }
        
        if(sym == SDLK_F6) {
          View = spin(M_PI/2) * inverse(cwtV) * View;
          if(flipplayer) View = spin(M_PI) * View;
          cmode = emDraw;
          }

        if(sym == 'v' || sym == SDLK_F2) {
          cmode = emVisual1;
          }

#ifdef PANDORA
        if(ev.type == SDL_MOUSEBUTTONUP && sym == 0 && !rightclick) {
#else
        if(ev.type == SDL_MOUSEBUTTONDOWN && sym == 0 && !rightclick) {
#endif
          if(items[itOrbTeleport] && cwt.c != mouseover && !isNeighbor(cwt.c, mouseover) && mouseover) {
            teleportpc(mouseover);
            }
          else if(mousedest == -1)
            movepcto(mousedest);
          else
            movepcto((mousedest + 42 - cwt.spin)%42);
          }
        
        if(sym == SDLK_F1) {
          cmode = emHelp;
          }
        }

      else if(cmode == emVisual1) {
      
        char xuni = uni | 96;
        
        if(uni == ' ') cmode = emVisual2;
      
        if(xuni == 'p') vid.alpha += shift * 0.1;        
        if(xuni == 'z') vid.scale += shift * 0.1;
        if(xuni == 'a') vid.aspeed += shift;
        if(xuni == 'f') {
          vid.full = !vid.full;
          if(shift > 0) {
            vid.xres = vid.full ? vid.xscr : 9999;
            vid.yres = vid.full ? vid.yscr : 9999;
            setfsize = true;
            }
          setvideomode();
          }

        if(xuni == 'v' || sym == SDLK_F2) cmode = emNormal;
        if(xuni == 's') saveConfig();

        if(xuni == 'w') { vid.wallmode += 60 + (shift > 0 ? 1 : -1); vid.wallmode %= 4; }
        if(xuni == 'm') { vid.monmode += 60 + (shift > 0 ? 1 : -1); vid.monmode %= 3; }
        if(xuni == 'c') { vid.axes += 60 + (shift > 0 ? 1 : -1); vid.axes %= 4; }
        if(xuni == 'b') {
          audiovolume += int(10.5 * shift);
          if(audiovolume < 0) audiovolume = 0;
          if(audiovolume > MIX_MAX_VOLUME) audiovolume = MIX_MAX_VOLUME;
          Mix_VolumeMusic(audiovolume);
          }
        
        if(sym == SDLK_ESCAPE) cmode = emNormal;

#ifdef SHOWOFF
        process_showoff(sym);
#endif

        if(sym == SDLK_F1 || sym == 'h') {
          cmode = emHelp;
          }

        if(xuni == 'l') {
          vid.language++;
          vid.language %= 4;
          printf("lan = %d\n", vid.language);
          }

        if(xuni == 'g') {
          vid.female = !vid.female;
          switchcolor(vid.skincolor, skincolors, 0);
          switchcolor(vid.haircolor, haircolors, 0);
          switchcolor(vid.dresscolor, dresscolors, 0);
          switchcolor(vid.swordcolor, swordcolors, 0);
          cmode = emCustomizeChar;
          }

        }

      else if(cmode == emCustomizeChar) {
        char xuni = uni | 96;
        int mod = ev.key.keysym.mod;
        if(shift < -.5)
          mod = 1;
        else if(shift > -.2 && shift < .2)
          mod = 2;
        else mod = 0;
        if(xuni == 'g') vid.female = !vid.female;
        if(xuni == 's') switchcolor(vid.skincolor, skincolors, mod);
        if(xuni == 'h') switchcolor(vid.haircolor, haircolors, mod);
        if(xuni == 'w') switchcolor(vid.swordcolor, swordcolors, mod);
        if(xuni == 'd') switchcolor(vid.dresscolor, dresscolors, mod);
        if(xuni == 'v' || sym == SDLK_F2 || sym == SDLK_ESCAPE) cmode = emNormal;
        }
        
      else if(cmode == emVisual2) {
        char xuni = uni | 96;
        
#ifdef GL          
        if(xuni == 'v' || sym == SDLK_F2 || sym == SDLK_ESCAPE) cmode = emNormal;

        if(xuni == 'o' && shift > 0) {
          vid.usingGL = !vid.usingGL;
          if(vid.usingGL) addMessage(XLAT("openGL mode enabled"));
          if(!vid.usingGL) addMessage(XLAT("openGL mode disabled"));
          setvideomode();
          }

        if(xuni == 'o' && shift < 0 && !vid.usingGL) {
          vid.usingAA = !vid.usingAA;
          if(vid.usingAA) addMessage(XLAT("anti-aliasing enabled"));
          if(!vid.usingAA) addMessage(XLAT("anti-aliasing disabled"));
          }
#endif

        if(xuni == 'f') { 
          vid.framelimit += int(10.5 * shift);
          if(vid.framelimit < 5) vid.framelimit = 5;
          }
        
        if(xuni == 'a') vid.joyvalue += int(shift * 100);
        if(xuni == 'b') vid.joyvalue2 += int(shift * 100);
        if(xuni == 'c') vid.joypanthreshold += int(shift * 100);
        if(xuni == 'd') vid.joypanspeed += shift / 50000;
        if(xuni == 'e') vid.eye += shift * 0.01;        
        if(xuni == 't') vid.flashtime += shift>0?1:-1;

        if(xuni == 'p') autojoy = !autojoy;

        if(uni == ' ') cmode = emChangeMode;
        }

      else if(cmode == emChangeMode) {

        char xuni = uni | 96;
        if(xuni == 'v' || sym == SDLK_F2 || sym == SDLK_ESCAPE) cmode = emNormal;
        if(uni == ' ') cmode = emVisual1;

        if(sym == 'c') {
          cheater++;
          addMessage(XLAT("You activate your demonic powers!"));
          cmode = emNormal;
          }

        if(xuni == 'g') cmode = emDraw;
        if(xuni == 'e') cmode = emPickEuclidean;
        
        if(uni == 'h' || uni == '3' || uni == '7') {
          vid.boardmode = uni;
          cheater = 1;
          showid = 0; firstland = laGameBoard;
          for(int i=0; i<landtypes; i++) landcount[i] = 0;
          for(int i=0; i<ittypes; i++) items[i] = 0;
          for(int i=0; i<motypes; i++) kills[i] = 0;
          saveStats();
          if(euclid) restartGameSwitchEuclid();
          clearMemory();
          initcells();
          initgame();
          restartGraph();
          cmode = emNormal;
          }
        
        }

      else if(cmode == emPickEuclidean) {
        char xuni = uni | 96;
        if(xuni == 'z') {
          if(euclid) restartGameSwitchEuclid();
          cmode = emNormal;
          }
        else if(xuni >= 'a' && xuni < 'a' + landtypes) {
          euclidland = eLand(2 + (xuni - 'a'));
          if(landvisited[euclidland]) {
            if(euclid) restartGame();
            else restartGameSwitchEuclid();
            cmode = emNormal;
            }
          else euclidland = laIce;
          }
        }
      
      else if(cmode == emHelp) {
        if(sym == SDLK_F1 && help != "@") 
          help = "@";
        else if(uni == 'c')
          help = buildCredits();
        else if((sym != 0 && sym != SDLK_F12) || ev.type == SDL_MOUSEBUTTONDOWN)
          cmode = (help == XLAT(drawhelp) ? emDraw : emNormal);
        }
        
      else if(cmode == emQuit) {
        if(sym == SDLK_RETURN || sym == SDLK_F10) return;
        else if(uni == 'r' || sym == SDLK_F5) {
          restartGame(), cmode = emNormal;
          msgs.clear();
          }
        else if(sym == SDLK_UP || sym == SDLK_KP8) msgscroll++;
        else if(sym == SDLK_DOWN || sym == SDLK_KP2) msgscroll--;
        else if(sym == SDLK_PAGEUP || sym == SDLK_KP9) msgscroll+=5;
        else if(sym == SDLK_PAGEDOWN || sym == SDLK_KP3) msgscroll-=5;
        else if(uni == 't') {
          if(!canmove) restartGame();
          loadScores();
          msgs.clear();
          }
        
        else if((sym != 0 && sym != SDLK_F12) || ev.type == SDL_MOUSEBUTTONDOWN) {
          cmode = emNormal;
          msgs.clear();
          }
        }
      
      else if(cmode == emScores) 
        handleScoreKeys(sym, ev);

      if(ev.type == SDL_QUIT)
        return;

      DEB("r3");
      }
    
    if(playerdead) break;
    }
  
  }
#endif

#ifndef MOBILE
void cleargraph() {
  for(int i=0; i<256; i++) if(font[i]) TTF_CloseFont(font[i]);
  for(int i=0; i<128; i++) if(glfont[i]) delete glfont[i];
#ifndef SIMULATE_JOYSTICK
  if(stick) SDL_JoystickClose(stick);
  if(panstick) SDL_JoystickClose(panstick);
#endif
  SDL_Quit();
  }
#endif

void cleargraphmemory() {
  mouseover = centerover = lmouseover = NULL;  
  }

