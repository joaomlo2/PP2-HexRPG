#define QHPC 5000      

int qhpc, prehpc;

hyperpoint hpc[QHPC];

bool first;

bool fatborder;

void hpcpush(const hyperpoint& h) { 
  if(vid.usingGL && !first && intval(hpc[qhpc-1], h) > 0.25) {
    hyperpoint md = mid(hpc[qhpc-1], h);
    hpcpush(md);
    }
  first = false;
  hpc[qhpc++] = h;
  }

struct hpcshape {
  int s, e, prio;
  };

struct polytodraw {
  transmatrix V;
  int col, start, end, prio;
  };

vector<polytodraw> ptds;

bool ptdsort(const polytodraw& p1, const polytodraw& p2) {
  return p1.prio < p2.prio;
  }

#ifndef MOBILE
SDL_Surface *aux;
#endif

vector<polytodraw*> ptds2;

#ifdef GL
#define USEPOLY

GLuint shapebuffer;

GLfloat *ourshape = NULL;

void initPolyForGL() {
  if(GL_initialized) return;
  GL_initialized = true;
  
  if(ourshape) delete[] ourshape;
  ourshape = new GLfloat[3 * qhpc];
  
  // GLfloat ourshape[3*qhpc];
  
  int id = 0;
  for(int i=0; i<qhpc; i++) {
    ourshape[id++] = hpc[i][0];
    ourshape[id++] = hpc[i][1];
    ourshape[id++] = hpc[i][2];
    }

#define GLF
#ifdef GLF
  glVertexPointer(3, GL_FLOAT, 0, ourshape);

#else
  glGenBuffers(1, &shapebuffer);
  glBindBuffer(GL_ARRAY_BUFFER, shapebuffer); 
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*qhpc*3, ourshape, GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
#endif

  }
#endif

#ifdef GFX
#define USEPOLY
#endif

#ifdef GL
#endif

void drawqueue() {
#ifdef USEPOLY

  int siz = size(ptds);

#ifdef GL
  if(vid.usingGL) {
    initPolyForGL();
    glVertexPointer(3, GL_FLOAT, 0, ourshape);
    glClear(GL_STENCIL_BUFFER_BIT);
    }
#endif
  
#ifdef STLSORT
  sort(ptds.begin(), ptds.end(), ptdsort);
  
#else
  
  int qp[64];
  for(int a=0; a<64; a++) qp[a] = 0;
  
  for(int i = 0; i<siz; i++) {
    qp[ptds[i].prio]++;
    }
  
  qp[0]--;
  for(int a=1; a<64; a++) qp[a] += qp[a-1];
  
  ptds2.resize(siz);
  
  for(int i = 0; i<siz; i++) ptds2[qp[ptds[i].prio]--] = &ptds[i];
#endif

#ifndef MOBILE
  if(vid.goteyes && !vid.usingGL) {

    if(aux && (aux->w != s->w || aux->h != s->h))
      SDL_FreeSurface(aux);
  
    if(!aux) {
      aux = SDL_CreateRGBSurface(SDL_SWSURFACE,s->w,s->h,32,0,0,0,0);
      }

    // SDL_LockSurface(aux);
    // memset(aux->pixels, 0, vid.xres * vid.yres * 4);
    // SDL_UnlockSurface(aux);
    SDL_BlitSurface(s, NULL, aux, NULL);
    }
#endif
  
  for(int i=0; i<siz; i++) {
  
#ifdef STLSORT
    polytodraw& ptd (ptds[i]);
#else
    polytodraw& ptd (*ptds2[i]);
#endif

    if(!vid.usingGL) {
      polyi = 0;
      for(int i=ptd.start; i<ptd.end-1; i++) 
        drawline(ptd.V*hpc[i], ptd.V*hpc[i+1], -1);
      if(polyi < 3) continue;
      }
    
    // if(ptd.prio == 46) printf("eye size %d\n", polyi);

    if(true) {
#ifdef GL
      if(vid.usingGL) {

#ifdef GLF
        glEnableClientState(GL_VERTEX_ARRAY);
#else
        glBindBuffer(GL_ARRAY_BUFFER, shapebuffer);
#endif
        
        unsigned char *c = (unsigned char*) (&ptd.col);

        GLfloat mat[16];
        int id = 0;
        
        for(int y=0; y<3; y++) {
          for(int x=0; x<3; x++) mat[id++] = ptd.V[x][y];
          mat[id++] = 0;
          }
        for(int x=0; x<3; x++) mat[id++] = 0; mat[id++] = 1;
        
        if(euclid) {
          mat[2] = 0; mat[6] = 0; mat[10] = 0; 
          mat[12] = mat[8]; mat[13] = mat[9]; mat[14] = EUCSCALE;
          mat[8] = mat[9] = 0;
          }
        
        for(int ed = vid.goteyes ? -1 : 0; ed<2; ed+=2) {
          if(ed) selectEyeGL(ed);
          
          glMatrixMode(GL_MODELVIEW);
          glPushMatrix();
          glMultMatrixf(mat);
          
          glEnable(GL_STENCIL_TEST);

          glColorMask( GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE );
          glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT);
          glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
          glColor4f(1,1,1,1);
          glDrawArrays(GL_TRIANGLE_FAN, ptd.start, ptd.end-ptd.start);
    
          selectEyeMask(ed);

          glColor4f(c[3] / 255.0, c[2] / 255.0, c[1]/255.0, c[0]/255.0);
          glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO);
          glStencilFunc( GL_EQUAL, 1, 1);
          glDrawArrays(GL_TRIANGLE_FAN, ptd.start, ptd.end-ptd.start);
          
          glDisable(GL_STENCIL_TEST);
          glColor4f(0, 0, 0, 1);
          glDrawArrays(GL_LINE_LOOP, ptd.start, ptd.end-ptd.start);
  
          glPopMatrix();
          }
        
        continue;
        }

#ifdef MOBILE
      gdpush(1); gdpush(ptd.col); gdpush(polyi); 
      for(int i=0; i<polyi; i++) gdpush(polyx[i]), gdpush(polyy[i]);

#else

#ifdef GFX      
      filledPolygonColor(s, polyx, polyy, polyi, ptd.col);
      if(vid.goteyes) filledPolygonColor(aux, polyxr, polyy, polyi, ptd.col);
      (vid.usingAA?aapolygonColor:polygonColor)(s, polyx, polyy, polyi, 0xFF);
      
      if(vid.xres >= 2000 || fatborder) {
        Sint16 xmi = 3000, xma = -3000;
        for(int t=0; t<polyi; t++) xmi = min(xmi, polyx[t]), xma = max(xma, polyx[t]);
        
        if(xma > xmi + 20) for(int x=-1; x<2; x++) for(int y=-1; y<=2; y++) if(x*x+y*y == 1) {
          for(int t=0; t<polyi; t++) polyx[t] += x, polyy[t] += y;
          aapolygonColor(s, polyx, polyy, polyi, 0xFF);
          for(int t=0; t<polyi; t++) polyx[t] -= x, polyy[t] -= y;
          }
        }
      if(vid.goteyes) aapolygonColor(aux, polyxr, polyy, polyi, 0xFF);
#endif
#endif
#endif
      }
    }

#ifndef MOBILE  
  if(vid.goteyes && !vid.usingGL) {
    int qty = s->w * s->h;
    int *a = (int*) s->pixels;
    int *b = (int*) aux->pixels;
    SDL_LockSurface(aux);
    while(qty) {
      *a = ((*a) & 0xFF0000) | ((*b) & 0x00FFFF);
      a++; b++; qty--;
      }
    SDL_UnlockSurface(aux);
    }

  if(vid.goteyes && vid.usingGL) selectEyeGL(0), selectEyeMask(0);

#endif
#endif
  }

hpcshape 
  shFloor[2], shBFloor[2], shWall[2], 
  shStarFloor[2], shCloudFloor[2], shTriFloor[2], 
  shFeatherFloor[2], shDemonFloor[2], shCrossFloor[2], shMFloor[2], shCaveFloor[2],
  shDesertFloor[2],
  shCross, shGiantStar[2], shLake, shMirror,
  shGem[2], shStar, shDisk, shRing, shDaisy, shTriangle, shNecro, shStatue, shKey,
  shBranch, shIBranch, shTentacle, shTentacleX, shILeaf[2], 
  shMovestar,
  shWolf, shYeti, shDemon, shGDemon, shEagle,
  shPBody, shPSword, shPHead, shPFace, shGolemhead, shHood, shArmor,
  shGhost, shEyes, shSlime, shJoint, shWormHead, shTentHead, shShark,
  shUser[8][3], shHedgehogBlade, shHedgehogBladePlayer,
  shWolfBody, shWolfHead, shWolfLegs, shWolfEyes,
  shBigHepta, shBigTriangle,
  shFemaleBody, shFemaleHair, shFemaleDress;

void drawTentacle(hpcshape &h, ld rad, ld var, ld divby) {
  for(int i=0; i<=20; i++)
    hpcpush(ddi(21, rad + var * sin(i * M_PI/divby)) * ddi(0, crossf * i/20.) * C0);
  for(int i=20; i>=0; i--)
    hpcpush(ddi(63, rad - var * sin(i * M_PI/divby)) * ddi(0, crossf * i/20.) * C0);
  }

hyperpoint hpxd(ld d, ld x, ld y, ld z) {
  hyperpoint H = hpxyz(d*x, d*y, z);
  H = mid(H, H);
  return H;
  }

void drawDemon(ld d) {
  hpcpush(hpxyz(0.098330,0.005996,1.004841));
  hpcpush(hpxyz(0.098350,0.015592,1.004946));
  hpcpush(hpxyz(0.097179,0.027594,1.005090));
  hpcpush(hpxyz(0.088731,0.041967,1.004806));
  hpcpush(hpxyz(0.082695,0.047939,1.004558));
  hpcpush(hpxyz(0.070682,0.061098,1.004355));
  hpcpush(hpxyz(0.075191,0.124914,1.010573));
  hpcpush(hpxyz(0.083954,0.132624,1.012244));
  hpcpush(hpxyz(0.097611,0.134215,1.013677));
  hpcpush(hpxyz(0.112530,0.132100,1.014945));
  hpcpush(hpxyz(0.126233,0.127459,1.015963));
  hpcpush(hpxyz(0.148804,0.116829,1.017738));
  hpcpush(hpxyz(0.170177,0.098653,1.019163));
  hpcpush(hpxyz(0.175203,0.092537,1.019441));
  hpcpush(hpxyz(0.167872,0.107389,1.019663));
  hpcpush(hpxyz(0.150410,0.125752,1.019037));
  hpcpush(hpxyz(0.132974,0.139130,1.018351));
  hpcpush(hpxyz(0.093435,0.163511,1.017578));
  hpcpush(hpxyz(0.064916,0.165353,1.015655));
  hpcpush(hpxyz(0.042697,0.159810,1.013589));
  hpcpush(hpxyz(0.026717,0.148159,1.011269));
  hpcpush(hpxyz(0.018123,0.130482,1.008640));
  hpcpush(hpxyz(0.016784,0.095911,1.004729));
  hpcpush(hpxyz(0.020324,0.078904,1.003314));
  hpcpush(hpxyz(0.017909,0.070441,1.002638));
  hpcpush(hpxyz(0.013123,0.065613,1.002236));
  hpcpush(hpxyz(0.000000,0.063197,1.001995));
  hpcpush(hpxyz(-0.015494,0.057207,1.001755));
  hpcpush(hpxyz(-0.032146,0.038099,1.001242));
  hpcpush(hpxyz(-0.063226,0.021473,1.002227));
  hpcpush(hpxyz(-0.122974,0.008439,1.007568));
  hpcpush(hpxyz(-0.197385,0.012337,1.019369));
  hpcpush(hpxyz(-0.283101,0.000000,1.039301));
  hpcpush(hpxyz(-0.283101,-0.000000,1.039301));
  hpcpush(hpxyz(-0.197385,-0.012337,1.019369));
  hpcpush(hpxyz(-0.122974,-0.008439,1.007568));
  hpcpush(hpxyz(-0.063226,-0.021473,1.002227));
  hpcpush(hpxyz(-0.032146,-0.038099,1.001242));
  hpcpush(hpxyz(-0.015494,-0.057207,1.001755));
  hpcpush(hpxyz(0.000000,-0.063197,1.001995));
  hpcpush(hpxyz(0.013123,-0.065613,1.002236));
  hpcpush(hpxyz(0.017909,-0.070441,1.002638));
  hpcpush(hpxyz(0.020324,-0.078904,1.003314));
  hpcpush(hpxyz(0.016784,-0.095911,1.004729));
  hpcpush(hpxyz(0.018123,-0.130482,1.008640));
  hpcpush(hpxyz(0.026717,-0.148159,1.011269));
  hpcpush(hpxyz(0.042697,-0.159810,1.013589));
  hpcpush(hpxyz(0.064916,-0.165353,1.015655));
  hpcpush(hpxyz(0.093435,-0.163511,1.017578));
  hpcpush(hpxyz(0.132974,-0.139130,1.018351));
  hpcpush(hpxyz(0.150410,-0.125752,1.019037));
  hpcpush(hpxyz(0.167872,-0.107389,1.019663));
  hpcpush(hpxyz(0.175203,-0.092537,1.019441));
  hpcpush(hpxyz(0.170177,-0.098653,1.019163));
  hpcpush(hpxyz(0.148804,-0.116829,1.017738));
  hpcpush(hpxyz(0.126233,-0.127459,1.015963));
  hpcpush(hpxyz(0.112530,-0.132100,1.014945));
  hpcpush(hpxyz(0.097611,-0.134215,1.013677));
  hpcpush(hpxyz(0.083954,-0.132624,1.012244));
  hpcpush(hpxyz(0.075191,-0.124914,1.010573));
  hpcpush(hpxyz(0.070682,-0.061098,1.004355));
  hpcpush(hpxyz(0.082695,-0.047939,1.004558));
  hpcpush(hpxyz(0.088731,-0.041967,1.004806));
  hpcpush(hpxyz(0.097179,-0.027594,1.005090));
  hpcpush(hpxyz(0.098350,-0.015592,1.004946));
  hpcpush(hpxyz(0.098330,-0.005996,1.004841));
  hpcpush(hpxyz(0.098330,0.005996,1.004841));
  }

hpcshape *last = NULL;

void bshape(hpcshape& sh, int p) {
  if(last) last->e = qhpc;
  last = &sh;
  last->s = qhpc, last->prio = p;
  first = true; 
  }

void bshapeend() {
  if(last) last->e = qhpc;
  last = NULL;
  }

hyperpoint hwolf(double x, double y, double z) {
  hyperpoint H = hpxyz(1.5*x, 1.6*y, z);
  H = mid(H,H);
  return H;
  }

void buildpolys() {

  qhpc = 0;

  bshape(shMovestar, 0);
  for(int i=0; i<8; i++) {
    hpcpush(spin(M_PI * i/4) * xpush(crossf) * spin(M_PI * i/4) * C0);
    hpcpush(spin(M_PI * i/4 + M_PI/8) * xpush(crossf/4) * spin(M_PI * i/4 + M_PI/8) * C0);
    }
  
  // floors
  
  bshape(shStarFloor[0], 10);
  hpcpush(hpxyz(0.267355,0.153145,1.046390));
  hpcpush(hpxyz(0.158858,0.062321,1.014455));
  hpcpush(hpxyz(0.357493,-0.060252,1.063688));
  hpcpush(hpxyz(0.266305,-0.154963,1.046390));
  hpcpush(hpxyz(0.133401,-0.106414,1.014455));
  hpcpush(hpxyz(0.126567,-0.339724,1.063688));
  hpcpush(hpxyz(-0.001050,-0.308108,1.046390));
  hpcpush(hpxyz(-0.025457,-0.168736,1.014455));
  hpcpush(hpxyz(-0.230926,-0.279472,1.063688));
  hpcpush(hpxyz(-0.267355,-0.153145,1.046390));
  hpcpush(hpxyz(-0.158858,-0.062321,1.014455));
  hpcpush(hpxyz(-0.357493,0.060252,1.063688));
  hpcpush(hpxyz(-0.266305,0.154963,1.046390));
  hpcpush(hpxyz(-0.133401,0.106414,1.014455));
  hpcpush(hpxyz(-0.126567,0.339724,1.063688));
  hpcpush(hpxyz(0.001050,0.308108,1.046390));
  hpcpush(hpxyz(0.025457,0.168736,1.014455));
  hpcpush(hpxyz(0.230926,0.279472,1.063688));
  hpcpush(hpxyz(0.267355,0.153145,1.046390));

  bshape(shStarFloor[1], 10);
  hpcpush(hpxyz(-0.012640,0.255336,1.032161));
  hpcpush(hpxyz(0.152259,0.386185,1.082738));
  hpcpush(hpxyz(0.223982,0.275978,1.061288));
  hpcpush(hpxyz(0.191749,0.169082,1.032161));
  hpcpush(hpxyz(0.396864,0.121741,1.082738));
  hpcpush(hpxyz(0.355418,-0.003047,1.061288));
  hpcpush(hpxyz(0.251747,-0.044494,1.032161));
  hpcpush(hpxyz(0.342622,-0.234376,1.082738));
  hpcpush(hpxyz(0.219218,-0.279777,1.061288));
  hpcpush(hpxyz(0.122175,-0.224565,1.032161));
  hpcpush(hpxyz(0.030378,-0.414004,1.082738));
  hpcpush(hpxyz(-0.082058,-0.345829,1.061288));
  hpcpush(hpxyz(-0.099398,-0.235534,1.032161));
  hpcpush(hpxyz(-0.304740,-0.281878,1.082738));
  hpcpush(hpxyz(-0.321543,-0.151465,1.061288));
  hpcpush(hpxyz(-0.246122,-0.069141,1.032161));
  hpcpush(hpxyz(-0.410384,0.062508,1.082738));
  hpcpush(hpxyz(-0.318899,0.156955,1.061288));
  hpcpush(hpxyz(-0.207511,0.149317,1.032161));
  hpcpush(hpxyz(-0.207000,0.359824,1.082738));
  hpcpush(hpxyz(-0.076118,0.347185,1.061288));
  hpcpush(hpxyz(-0.012640,0.255336,1.032161));

  bshape(shCloudFloor[0], 10);
  hpcpush(hpxyz(-0.249278,0.146483,1.040960));
  hpcpush(hpxyz(-0.242058,0.146003,1.039187));
  hpcpush(hpxyz(-0.209868,0.146655,1.032256));
  hpcpush(hpxyz(-0.180387,0.151575,1.027382));
  hpcpush(hpxyz(-0.148230,0.165669,1.024411));
  hpcpush(hpxyz(-0.131141,0.187345,1.025815));
  hpcpush(hpxyz(-0.127578,0.219788,1.031786));
  hpcpush(hpxyz(-0.130923,0.255428,1.040377));
  hpcpush(hpxyz(-0.116050,0.294689,1.048956));
  hpcpush(hpxyz(-0.093271,0.316597,1.053059));
  hpcpush(hpxyz(-0.050041,0.330532,1.054398));
  hpcpush(hpxyz(-0.007791,0.309060,1.046699));
  hpcpush(hpxyz(0.017835,0.271341,1.036313));
  hpcpush(hpxyz(0.017835,0.271341,1.036313));
  hpcpush(hpxyz(0.002219,0.289123,1.040960));
  hpcpush(hpxyz(0.005414,0.282630,1.039187));
  hpcpush(hpxyz(0.022073,0.255079,1.032256));
  hpcpush(hpxyz(0.041075,0.232008,1.027382));
  hpcpush(hpxyz(0.069359,0.211206,1.024411));
  hpcpush(hpxyz(0.096675,0.207244,1.025815));
  hpcpush(hpxyz(0.126553,0.220380,1.031786));
  hpcpush(hpxyz(0.155745,0.241096,1.040377));
  hpcpush(hpxyz(0.197183,0.247847,1.048956));
  hpcpush(hpxyz(0.227545,0.239074,1.053059));
  hpcpush(hpxyz(0.261229,0.208602,1.054398));
  hpcpush(hpxyz(0.263758,0.161278,1.046699));
  hpcpush(hpxyz(0.243906,0.120225,1.036313));
  hpcpush(hpxyz(0.243906,0.120225,1.036313));
  hpcpush(hpxyz(0.251497,0.142640,1.040960));
  hpcpush(hpxyz(0.247471,0.136627,1.039187));
  hpcpush(hpxyz(0.231941,0.108424,1.032256));
  hpcpush(hpxyz(0.221462,0.080432,1.027382));
  hpcpush(hpxyz(0.217589,0.045537,1.024411));
  hpcpush(hpxyz(0.227816,0.019899,1.025815));
  hpcpush(hpxyz(0.254131,0.000592,1.031786));
  hpcpush(hpxyz(0.286668,-0.014331,1.040377));
  hpcpush(hpxyz(0.313233,-0.046842,1.048956));
  hpcpush(hpxyz(0.320816,-0.077523,1.053059));
  hpcpush(hpxyz(0.311269,-0.121929,1.054398));
  hpcpush(hpxyz(0.271550,-0.147783,1.046699));
  hpcpush(hpxyz(0.226071,-0.151116,1.036313));
  hpcpush(hpxyz(0.226071,-0.151116,1.036313));
  hpcpush(hpxyz(0.249278,-0.146483,1.040960));
  hpcpush(hpxyz(0.242058,-0.146003,1.039187));
  hpcpush(hpxyz(0.209868,-0.146655,1.032256));
  hpcpush(hpxyz(0.180387,-0.151575,1.027382));
  hpcpush(hpxyz(0.148230,-0.165669,1.024411));
  hpcpush(hpxyz(0.131141,-0.187345,1.025815));
  hpcpush(hpxyz(0.127578,-0.219788,1.031786));
  hpcpush(hpxyz(0.130923,-0.255428,1.040377));
  hpcpush(hpxyz(0.116050,-0.294689,1.048956));
  hpcpush(hpxyz(0.093271,-0.316597,1.053059));
  hpcpush(hpxyz(0.050041,-0.330532,1.054398));
  hpcpush(hpxyz(0.007791,-0.309060,1.046699));
  hpcpush(hpxyz(-0.017835,-0.271341,1.036313));
  hpcpush(hpxyz(-0.017835,-0.271341,1.036313));
  hpcpush(hpxyz(-0.002219,-0.289123,1.040960));
  hpcpush(hpxyz(-0.005414,-0.282630,1.039187));
  hpcpush(hpxyz(-0.022073,-0.255079,1.032256));
  hpcpush(hpxyz(-0.041075,-0.232008,1.027382));
  hpcpush(hpxyz(-0.069359,-0.211206,1.024411));
  hpcpush(hpxyz(-0.096675,-0.207244,1.025815));
  hpcpush(hpxyz(-0.126553,-0.220380,1.031786));
  hpcpush(hpxyz(-0.155745,-0.241096,1.040377));
  hpcpush(hpxyz(-0.197183,-0.247847,1.048956));
  hpcpush(hpxyz(-0.227545,-0.239074,1.053059));
  hpcpush(hpxyz(-0.261229,-0.208602,1.054398));
  hpcpush(hpxyz(-0.263758,-0.161278,1.046699));
  hpcpush(hpxyz(-0.243906,-0.120225,1.036313));
  hpcpush(hpxyz(-0.243906,-0.120225,1.036313));
  hpcpush(hpxyz(-0.251497,-0.142640,1.040960));
  hpcpush(hpxyz(-0.247471,-0.136627,1.039187));
  hpcpush(hpxyz(-0.231941,-0.108424,1.032256));
  hpcpush(hpxyz(-0.221462,-0.080432,1.027382));
  hpcpush(hpxyz(-0.217589,-0.045537,1.024411));
  hpcpush(hpxyz(-0.227816,-0.019899,1.025815));
  hpcpush(hpxyz(-0.254131,-0.000592,1.031786));
  hpcpush(hpxyz(-0.286668,0.014331,1.040377));
  hpcpush(hpxyz(-0.313233,0.046842,1.048956));
  hpcpush(hpxyz(-0.320816,0.077523,1.053059));
  hpcpush(hpxyz(-0.311269,0.121929,1.054398));
  hpcpush(hpxyz(-0.271550,0.147783,1.046699));
  hpcpush(hpxyz(-0.226071,0.151116,1.036313));
  hpcpush(hpxyz(-0.226071,0.151116,1.036313));
  hpcpush(hpxyz(-0.249278,0.146483,1.040960));

  bshape(shCrossFloor[0], 10);
  hpcpush(hpxyz(-0.283927,0.089050,1.043333));
  hpcpush(hpxyz(-0.363031,0.095818,1.068163));
  hpcpush(hpxyz(-0.386170,0.136539,1.080634));
  hpcpush(hpxyz(-0.370773,0.209082,1.086825));
  hpcpush(hpxyz(-0.207773,0.157096,1.033368));
  hpcpush(hpxyz(-0.213111,0.255207,1.053825));
  hpcpush(hpxyz(-0.045690,0.195109,1.019880));
  hpcpush(hpxyz(-0.053263,0.305287,1.046918));
  hpcpush(hpxyz(0.090089,0.277990,1.041823));
  hpcpush(hpxyz(0.219083,0.201363,1.043333));
  hpcpush(hpxyz(0.264497,0.266484,1.068163));
  hpcpush(hpxyz(0.311331,0.266164,1.080634));
  hpcpush(hpxyz(0.366457,0.216558,1.086825));
  hpcpush(hpxyz(0.239936,0.101388,1.033368));
  hpcpush(hpxyz(0.327571,0.056956,1.053825));
  hpcpush(hpxyz(0.191815,-0.057986,1.019880));
  hpcpush(hpxyz(0.291017,-0.106516,1.046918));
  hpcpush(hpxyz(0.195702,-0.217014,1.041823));
  hpcpush(hpxyz(0.064844,-0.290413,1.043333));
  hpcpush(hpxyz(0.098534,-0.362303,1.068163));
  hpcpush(hpxyz(0.074839,-0.402702,1.080634));
  hpcpush(hpxyz(0.004316,-0.425640,1.086825));
  hpcpush(hpxyz(-0.032163,-0.258485,1.033368));
  hpcpush(hpxyz(-0.114460,-0.312163,1.053825));
  hpcpush(hpxyz(-0.146125,-0.137124,1.019880));
  hpcpush(hpxyz(-0.237755,-0.198770,1.046918));
  hpcpush(hpxyz(-0.285791,-0.060975,1.041823));
  hpcpush(hpxyz(-0.283927,0.089050,1.043333));

  bshape(shCrossFloor[1], 10);
  hpcpush(hpxyz(-0.254099,-0.080041,1.034878));
  hpcpush(hpxyz(-0.326144,-0.061810,1.053656));
  hpcpush(hpxyz(-0.326144,0.061810,1.053656));
  hpcpush(hpxyz(-0.254099,0.080041,1.034878));
  hpcpush(hpxyz(-0.221007,0.148758,1.034878));
  hpcpush(hpxyz(-0.251672,0.216452,1.053656));
  hpcpush(hpxyz(-0.155023,0.293527,1.053656));
  hpcpush(hpxyz(-0.095849,0.248567,1.034878));
  hpcpush(hpxyz(-0.021492,0.265539,1.034878));
  hpcpush(hpxyz(0.012314,0.331721,1.053656));
  hpcpush(hpxyz(0.132834,0.304213,1.053656));
  hpcpush(hpxyz(0.134577,0.229917,1.034878));
  hpcpush(hpxyz(0.194207,0.182364,1.034878));
  hpcpush(hpxyz(0.267027,0.197197,1.053656));
  hpcpush(hpxyz(0.320664,0.085820,1.053656));
  hpcpush(hpxyz(0.263664,0.038135,1.034878));
  hpcpush(hpxyz(0.263664,-0.038135,1.034878));
  hpcpush(hpxyz(0.320664,-0.085820,1.053656));
  hpcpush(hpxyz(0.267027,-0.197197,1.053656));
  hpcpush(hpxyz(0.194207,-0.182364,1.034878));
  hpcpush(hpxyz(0.134577,-0.229917,1.034878));
  hpcpush(hpxyz(0.132834,-0.304213,1.053656));
  hpcpush(hpxyz(0.012314,-0.331721,1.053656));
  hpcpush(hpxyz(-0.021492,-0.265539,1.034878));
  hpcpush(hpxyz(-0.095849,-0.248567,1.034878));
  hpcpush(hpxyz(-0.155023,-0.293527,1.053656));
  hpcpush(hpxyz(-0.251672,-0.216452,1.053656));
  hpcpush(hpxyz(-0.221007,-0.148758,1.034878));
  hpcpush(hpxyz(-0.254099,-0.080041,1.034878));
  
  bshape(shCloudFloor[1], 10);
  hpcpush(hpxyz(-0.216129,0.195545,1.041609));
  hpcpush(hpxyz(-0.209066,0.211647,1.043314));
  hpcpush(hpxyz(-0.195698,0.234578,1.045622));
  hpcpush(hpxyz(-0.193583,0.272597,1.054411));
  hpcpush(hpxyz(-0.202572,0.304529,1.064788));
  hpcpush(hpxyz(-0.198965,0.336605,1.073727));
  hpcpush(hpxyz(-0.167757,0.354608,1.074192));
  hpcpush(hpxyz(-0.132750,0.357613,1.070285));
  hpcpush(hpxyz(-0.104720,0.351753,1.065221));
  hpcpush(hpxyz(-0.079702,0.340062,1.059242));
  hpcpush(hpxyz(-0.064640,0.331114,1.055374));
  hpcpush(hpxyz(-0.040288,0.308007,1.047135));
  hpcpush(hpxyz(-0.023142,0.289269,1.041255));
  hpcpush(hpxyz(0.018129,0.290897,1.041609));
  hpcpush(hpxyz(0.035122,0.295414,1.043314));
  hpcpush(hpxyz(0.061385,0.299260,1.045622));
  hpcpush(hpxyz(0.092428,0.321310,1.054411));
  hpcpush(hpxyz(0.111789,0.348249,1.064788));
  hpcpush(hpxyz(0.139116,0.365426,1.073727));
  hpcpush(hpxyz(0.172649,0.352252,1.074192));
  hpcpush(hpxyz(0.196825,0.326757,1.070285));
  hpcpush(hpxyz(0.209719,0.301188,1.065221));
  hpcpush(hpxyz(0.216178,0.274339,1.059242));
  hpcpush(hpxyz(0.218573,0.256983,1.055374));
  hpcpush(hpxyz(0.215691,0.223538,1.047135));
  hpcpush(hpxyz(0.211731,0.198449,1.041255));
  hpcpush(hpxyz(0.238736,0.167197,1.041609));
  hpcpush(hpxyz(0.252862,0.156728,1.043314));
  hpcpush(hpxyz(0.272244,0.138593,1.045622));
  hpcpush(hpxyz(0.308838,0.128071,1.054411));
  hpcpush(hpxyz(0.341971,0.129729,1.064788));
  hpcpush(hpxyz(0.372439,0.119075,1.073727));
  hpcpush(hpxyz(0.383047,0.084643,1.074192));
  hpcpush(hpxyz(0.378187,0.049846,1.070285));
  hpcpush(hpxyz(0.366236,0.023822,1.065221));
  hpcpush(hpxyz(0.349271,0.002033,1.059242));
  hpcpush(hpxyz(0.337196,-0.010661,1.055374));
  hpcpush(hpxyz(0.309250,-0.029260,1.047135));
  hpcpush(hpxyz(0.287166,-0.041807,1.041255));
  hpcpush(hpxyz(0.279570,-0.082405,1.041609));
  hpcpush(hpxyz(0.280192,-0.099977,1.043314));
  hpcpush(hpxyz(0.278098,-0.126438,1.045622));
  hpcpush(hpxyz(0.292688,-0.161608,1.054411));
  hpcpush(hpxyz(0.314642,-0.186479,1.064788));
  hpcpush(hpxyz(0.325308,-0.216943,1.073727));
  hpcpush(hpxyz(0.305002,-0.246704,1.074192));
  hpcpush(hpxyz(0.274767,-0.264600,1.070285));
  hpcpush(hpxyz(0.246970,-0.271482,1.065221));
  hpcpush(hpxyz(0.219356,-0.271804,1.059242));
  hpcpush(hpxyz(0.201903,-0.270277,1.055374));
  hpcpush(hpxyz(0.169937,-0.260025,1.047135));
  hpcpush(hpxyz(0.146359,-0.250582,1.041255));
  hpcpush(hpxyz(0.109882,-0.269955,1.041609));
  hpcpush(hpxyz(0.096532,-0.281398,1.043314));
  hpcpush(hpxyz(0.074538,-0.296258,1.045622));
  hpcpush(hpxyz(0.056137,-0.329594,1.054411));
  hpcpush(hpxyz(0.050381,-0.362264,1.064788));
  hpcpush(hpxyz(0.033214,-0.389598,1.073727));
  hpcpush(hpxyz(-0.002715,-0.392278,1.074192));
  hpcpush(hpxyz(-0.035559,-0.379797,1.070285));
  hpcpush(hpxyz(-0.058270,-0.362355,1.065221));
  hpcpush(hpxyz(-0.075738,-0.340967,1.059242));
  hpcpush(hpxyz(-0.085427,-0.326369,1.055374));
  hpcpush(hpxyz(-0.097341,-0.294985,1.047135));
  hpcpush(hpxyz(-0.104659,-0.270663,1.041255));
  hpcpush(hpxyz(-0.142549,-0.254223,1.041609));
  hpcpush(hpxyz(-0.159819,-0.250920,1.043314));
  hpcpush(hpxyz(-0.185150,-0.242990,1.045622));
  hpcpush(hpxyz(-0.222686,-0.249388,1.054411));
  hpcpush(hpxyz(-0.251818,-0.265258,1.064788));
  hpcpush(hpxyz(-0.283891,-0.268878,1.073727));
  hpcpush(hpxyz(-0.308388,-0.242459,1.074192));
  hpcpush(hpxyz(-0.319108,-0.208999,1.070285));
  hpcpush(hpxyz(-0.319631,-0.180367,1.065221));
  hpcpush(hpxyz(-0.313800,-0.153375,1.059242));
  hpcpush(hpxyz(-0.308428,-0.136699,1.055374));
  hpcpush(hpxyz(-0.291320,-0.107816,1.047135));
  hpcpush(hpxyz(-0.276867,-0.086930,1.041255));
  hpcpush(hpxyz(-0.287638,-0.047056,1.041609));
  hpcpush(hpxyz(-0.295823,-0.031495,1.043314));
  hpcpush(hpxyz(-0.305417,-0.006746,1.045622));
  hpcpush(hpxyz(-0.333822,0.018612,1.054411));
  hpcpush(hpxyz(-0.364393,0.031493,1.064788));
  hpcpush(hpxyz(-0.387221,0.054313,1.073727));
  hpcpush(hpxyz(-0.381839,0.089937,1.074192));
  hpcpush(hpxyz(-0.362362,0.119180,1.070285));
  hpcpush(hpxyz(-0.340304,0.137441,1.065221));
  hpcpush(hpxyz(-0.315564,0.149712,1.059242));
  hpcpush(hpxyz(-0.299177,0.155909,1.055374));
  hpcpush(hpxyz(-0.265929,0.160541,1.047135));
  hpcpush(hpxyz(-0.240588,0.162264,1.041255));
  hpcpush(hpxyz(-0.216129,0.195545,1.041609));

  bshape(shTriFloor[0], 10);
  hpcpush(hpxyz(-0.353692,0.244649,1.088554));
  hpcpush(hpxyz(0.388718,0.183982,1.088554));
  hpcpush(hpxyz(-0.035026,-0.428630,1.088554));
  hpcpush(hpxyz(-0.353692,0.244649,1.088554));

  bshape(shTriFloor[1], 10);
  hpcpush(hpxyz(-0.352357,0.009788,1.060307));
  hpcpush(hpxyz(-0.212038,0.281586,1.060307));
  hpcpush(hpxyz(0.087949,0.341344,1.060307));
  hpcpush(hpxyz(0.321709,0.144063,1.060307));
  hpcpush(hpxyz(0.313216,-0.161700,1.060307));
  hpcpush(hpxyz(0.068864,-0.345700,1.060307));
  hpcpush(hpxyz(-0.227343,-0.269381,1.060307));
  hpcpush(hpxyz(-0.352357,0.009788,1.060307));

  bshape(shFeatherFloor[0], 10);
  hpcpush(hpxyz(0.227689,0.169556,1.039515));
  hpcpush(hpxyz(0.333673,0.157023,1.065830));
  hpcpush(hpxyz(0.251378,0.091850,1.035194));
  hpcpush(hpxyz(0.316741,0.073094,1.051507));
  hpcpush(hpxyz(0.260919,0.038655,1.034202));
  hpcpush(hpxyz(0.306256,0.038890,1.046568));
  hpcpush(hpxyz(0.231096,-0.033702,1.026909));
  hpcpush(hpxyz(0.255955,-0.043464,1.033152));
  hpcpush(hpxyz(0.226427,-0.077082,1.028208));
  hpcpush(hpxyz(0.266494,-0.101752,1.039891));
  hpcpush(hpxyz(0.217364,-0.144909,1.033560));
  hpcpush(hpxyz(0.223207,-0.194093,1.042830));
  hpcpush(hpxyz(0.158549,-0.144135,1.022699));
  hpcpush(hpxyz(0.179022,-0.208053,1.036984));
  hpcpush(hpxyz(0.105573,-0.172756,1.020289));
  hpcpush(hpxyz(0.120828,-0.236822,1.034739));
  hpcpush(hpxyz(0.033598,-0.201587,1.020669));
  hpcpush(hpxyz(0.067872,-0.281182,1.040995));
  hpcpush(hpxyz(0.032995,-0.281963,1.039515));
  hpcpush(hpxyz(-0.030851,-0.367481,1.065830));
  hpcpush(hpxyz(-0.046145,-0.263624,1.035194));
  hpcpush(hpxyz(-0.095069,-0.310853,1.051507));
  hpcpush(hpxyz(-0.096984,-0.245290,1.034202));
  hpcpush(hpxyz(-0.119449,-0.284670,1.046568));
  hpcpush(hpxyz(-0.144735,-0.183285,1.026909));
  hpcpush(hpxyz(-0.165618,-0.199931,1.033152));
  hpcpush(hpxyz(-0.179968,-0.157551,1.028208));
  hpcpush(hpxyz(-0.221367,-0.179914,1.039891));
  hpcpush(hpxyz(-0.234177,-0.115788,1.033560));
  hpcpush(hpxyz(-0.279693,-0.096256,1.042830));
  hpcpush(hpxyz(-0.204099,-0.065240,1.022699));
  hpcpush(hpxyz(-0.269690,-0.051011,1.036984));
  hpcpush(hpxyz(-0.202398,-0.005051,1.020289));
  hpcpush(hpxyz(-0.265508,0.013771,1.034739));
  hpcpush(hpxyz(-0.191378,0.071697,1.020669));
  hpcpush(hpxyz(-0.277447,0.081813,1.040995));
  hpcpush(hpxyz(-0.260684,0.112407,1.039515));
  hpcpush(hpxyz(-0.302822,0.210458,1.065830));
  hpcpush(hpxyz(-0.205233,0.171775,1.035194));
  hpcpush(hpxyz(-0.221672,0.237759,1.051507));
  hpcpush(hpxyz(-0.163935,0.206635,1.034202));
  hpcpush(hpxyz(-0.186807,0.245781,1.046568));
  hpcpush(hpxyz(-0.086362,0.216986,1.026909));
  hpcpush(hpxyz(-0.090337,0.243396,1.033152));
  hpcpush(hpxyz(-0.046459,0.234632,1.028208));
  hpcpush(hpxyz(-0.045127,0.281667,1.039891));
  hpcpush(hpxyz(0.016813,0.260697,1.033560));
  hpcpush(hpxyz(0.056486,0.290349,1.042830));
  hpcpush(hpxyz(0.045550,0.209375,1.022699));
  hpcpush(hpxyz(0.090668,0.259064,1.036984));
  hpcpush(hpxyz(0.096825,0.177807,1.020289));
  hpcpush(hpxyz(0.144680,0.223051,1.034739));
  hpcpush(hpxyz(0.157781,0.129890,1.020669));
  hpcpush(hpxyz(0.209575,0.199370,1.040995));
  hpcpush(hpxyz(0.227689,0.169556,1.039515));

  bshape(shFeatherFloor[1], 10);
  hpcpush(hpxyz(-0.097861,0.337622,1.059984));
  hpcpush(hpxyz(-0.034295,0.357645,1.062585));
  hpcpush(hpxyz(-0.014623,0.326578,1.052077));
  hpcpush(hpxyz(0.024557,0.373262,1.067674));
  hpcpush(hpxyz(0.063328,0.316638,1.050843));
  hpcpush(hpxyz(0.088328,0.358219,1.065890));
  hpcpush(hpxyz(0.111793,0.286773,1.046297));
  hpcpush(hpxyz(0.146987,0.328271,1.062717));
  hpcpush(hpxyz(0.170806,0.287930,1.054551));
  hpcpush(hpxyz(0.191056,0.303731,1.062429));
  hpcpush(hpxyz(0.202948,0.287015,1.059984));
  hpcpush(hpxyz(0.258236,0.249801,1.062585));
  hpcpush(hpxyz(0.246211,0.215050,1.052077));
  hpcpush(hpxyz(0.307139,0.213526,1.067674));
  hpcpush(hpxyz(0.287042,0.147909,1.050843));
  hpcpush(hpxyz(0.335138,0.154288,1.065890));
  hpcpush(hpxyz(0.293910,0.091397,1.046297));
  hpcpush(hpxyz(0.348297,0.089755,1.062717));
  hpcpush(hpxyz(0.331608,0.045980,1.054551));
  hpcpush(hpxyz(0.356588,0.039999,1.062429));
  hpcpush(hpxyz(0.350933,0.020280,1.059984));
  hpcpush(hpxyz(0.356310,-0.046149,1.062585));
  hpcpush(hpxyz(0.321644,-0.058414,1.052077));
  hpcpush(hpxyz(0.358439,-0.107000,1.067674));
  hpcpush(hpxyz(0.294608,-0.132199,1.050843));
  hpcpush(hpxyz(0.329582,-0.165824,1.065890));
  hpcpush(hpxyz(0.254707,-0.172803,1.046297));
  hpcpush(hpxyz(0.287333,-0.216349,1.062717));
  hpcpush(hpxyz(0.242703,-0.230594,1.054551));
  hpcpush(hpxyz(0.253601,-0.253853,1.062429));
  hpcpush(hpxyz(0.234659,-0.261726,1.059984));
  hpcpush(hpxyz(0.186075,-0.307347,1.062585));
  hpcpush(hpxyz(0.154871,-0.287892,1.052077));
  hpcpush(hpxyz(0.139827,-0.346952,1.067674));
  hpcpush(hpxyz(0.080328,-0.312758,1.050843));
  hpcpush(hpxyz(0.075845,-0.361068,1.065890));
  hpcpush(hpxyz(0.023704,-0.306879,1.046297));
  hpcpush(hpxyz(0.010001,-0.359537,1.062717));
  hpcpush(hpxyz(-0.028963,-0.333526,1.054551));
  hpcpush(hpxyz(-0.040352,-0.356548,1.062429));
  hpcpush(hpxyz(-0.058319,-0.346647,1.059984));
  hpcpush(hpxyz(-0.124278,-0.337107,1.062585));
  hpcpush(hpxyz(-0.128522,-0.300581,1.052077));
  hpcpush(hpxyz(-0.184077,-0.325642,1.067674));
  hpcpush(hpxyz(-0.194441,-0.257805,1.050843));
  hpcpush(hpxyz(-0.235006,-0.284420,1.065890));
  hpcpush(hpxyz(-0.225148,-0.209869,1.046297));
  hpcpush(hpxyz(-0.274862,-0.231987,1.062717));
  hpcpush(hpxyz(-0.278819,-0.185306,1.054551));
  hpcpush(hpxyz(-0.303920,-0.190756,1.062429));
  hpcpush(hpxyz(-0.307381,-0.170536,1.059984));
  hpcpush(hpxyz(-0.341047,-0.113018,1.062585));
  hpcpush(hpxyz(-0.315136,-0.086927,1.052077));
  hpcpush(hpxyz(-0.369368,-0.059118,1.067674));
  hpcpush(hpxyz(-0.322791,-0.008719,1.050843));
  hpcpush(hpxyz(-0.368892,0.006402,1.065890));
  hpcpush(hpxyz(-0.304459,0.045177,1.046297));
  hpcpush(hpxyz(-0.352748,0.070255,1.062717));
  hpcpush(hpxyz(-0.318719,0.102453,1.054551));
  hpcpush(hpxyz(-0.338629,0.118680,1.062429));
  hpcpush(hpxyz(-0.324979,0.133993,1.059984));
  hpcpush(hpxyz(-0.301001,0.196175,1.062585));
  hpcpush(hpxyz(-0.264446,0.192185,1.052077));
  hpcpush(hpxyz(-0.276517,0.251924,1.067674));
  hpcpush(hpxyz(-0.208074,0.246932,1.050843));
  hpcpush(hpxyz(-0.224995,0.292403,1.065890));
  hpcpush(hpxyz(-0.154507,0.266203,1.046297));
  hpcpush(hpxyz(-0.165008,0.319593,1.062717));
  hpcpush(hpxyz(-0.118617,0.313063,1.054551));
  hpcpush(hpxyz(-0.118344,0.338747,1.062429));
  hpcpush(hpxyz(-0.097861,0.337622,1.059984));

  bshape(shDemonFloor[1], 10);
  hpcpush(hpxyz(-0.226625,-0.098204,1.030050));
  hpcpush(hpxyz(-0.248360,-0.079830,1.033468));
  hpcpush(hpxyz(-0.282817,-0.064277,1.041209));
  hpcpush(hpxyz(-0.267165,-0.033078,1.035602));
  hpcpush(hpxyz(-0.265427,-0.008890,1.034665));
  hpcpush(hpxyz(-0.192052,-0.006156,1.018294));
  hpcpush(hpxyz(-0.183253,-0.047966,1.017783));
  hpcpush(hpxyz(-0.143185,-0.036403,1.010855));
  hpcpush(hpxyz(-0.143048,-0.019396,1.010366));
  hpcpush(hpxyz(-0.166019,-0.021973,1.013926));
  hpcpush(hpxyz(-0.166019,0.021973,1.013926));
  hpcpush(hpxyz(-0.143048,0.019396,1.010366));
  hpcpush(hpxyz(-0.143185,0.036403,1.010855));
  hpcpush(hpxyz(-0.183253,0.047966,1.017783));
  hpcpush(hpxyz(-0.192052,0.006156,1.018294));
  hpcpush(hpxyz(-0.265427,0.008890,1.034665));
  hpcpush(hpxyz(-0.267165,0.033078,1.035602));
  hpcpush(hpxyz(-0.282817,0.064277,1.041209));
  hpcpush(hpxyz(-0.248360,0.079830,1.033468));
  hpcpush(hpxyz(-0.226625,0.098204,1.030050));
  hpcpush(hpxyz(-0.218077,0.115953,1.030050));
  hpcpush(hpxyz(-0.217264,0.144403,1.033468));
  hpcpush(hpxyz(-0.226587,0.181040,1.041209));
  hpcpush(hpxyz(-0.192436,0.188254,1.035602));
  hpcpush(hpxyz(-0.172442,0.201977,1.034665));
  hpcpush(hpxyz(-0.124555,0.146314,1.018294));
  hpcpush(hpxyz(-0.151758,0.113367,1.017783));
  hpcpush(hpxyz(-0.117735,0.089250,1.010855));
  hpcpush(hpxyz(-0.104354,0.099746,1.010366));
  hpcpush(hpxyz(-0.120690,0.116099,1.013926));
  hpcpush(hpxyz(-0.086332,0.143499,1.013926));
  hpcpush(hpxyz(-0.074024,0.123933,1.010366));
  hpcpush(hpxyz(-0.060813,0.134643,1.010855));
  hpcpush(hpxyz(-0.076756,0.173179,1.017783));
  hpcpush(hpxyz(-0.114930,0.153990,1.018294));
  hpcpush(hpxyz(-0.158541,0.213062,1.034665));
  hpcpush(hpxyz(-0.140713,0.229501,1.035602));
  hpcpush(hpxyz(-0.126080,0.261191,1.041209));
  hpcpush(hpxyz(-0.092436,0.243949,1.033468));
  hpcpush(hpxyz(-0.064519,0.238411,1.030050));
  hpcpush(hpxyz(-0.045313,0.242795,1.030050));
  hpcpush(hpxyz(-0.022563,0.259897,1.033468));
  hpcpush(hpxyz(0.000268,0.290029,1.041209));
  hpcpush(hpxyz(0.027202,0.267827,1.035602));
  hpcpush(hpxyz(0.050396,0.260751,1.034665));
  hpcpush(hpxyz(0.036734,0.188606,1.018294));
  hpcpush(hpxyz(-0.005985,0.189332,1.017783));
  hpcpush(hpxyz(-0.003629,0.147695,1.010855));
  hpcpush(hpxyz(0.012921,0.143777,1.010366));
  hpcpush(hpxyz(0.015520,0.166746,1.013926));
  hpcpush(hpxyz(0.058365,0.156967,1.013926));
  hpcpush(hpxyz(0.050741,0.135145,1.010366));
  hpcpush(hpxyz(0.067352,0.131495,1.010855));
  hpcpush(hpxyz(0.087541,0.167986,1.017783));
  hpcpush(hpxyz(0.048737,0.185867,1.018294));
  hpcpush(hpxyz(0.067730,0.256794,1.034665));
  hpcpush(hpxyz(0.091698,0.253106,1.035602));
  hpcpush(hpxyz(0.125598,0.261424,1.041209));
  hpcpush(hpxyz(0.133094,0.224370,1.033468));
  hpcpush(hpxyz(0.146171,0.199090,1.030050));
  hpcpush(hpxyz(0.161573,0.186808,1.030050));
  hpcpush(hpxyz(0.189128,0.179684,1.033468));
  hpcpush(hpxyz(0.226921,0.180621,1.041209));
  hpcpush(hpxyz(0.226355,0.145720,1.035602));
  hpcpush(hpxyz(0.235285,0.123174,1.034665));
  hpcpush(hpxyz(0.170362,0.088874,1.018294));
  hpcpush(hpxyz(0.144294,0.122726,1.017783));
  hpcpush(hpxyz(0.113211,0.094924,1.010855));
  hpcpush(hpxyz(0.120466,0.079542,1.010366));
  hpcpush(hpxyz(0.140044,0.091830,1.013926));
  hpcpush(hpxyz(0.159111,0.052236,1.013926));
  hpcpush(hpxyz(0.137297,0.044591,1.010366));
  hpcpush(hpxyz(0.144800,0.029328,1.010855));
  hpcpush(hpxyz(0.185917,0.036295,1.017783));
  hpcpush(hpxyz(0.175703,0.077782,1.018294));
  hpcpush(hpxyz(0.242999,0.107155,1.034665));
  hpcpush(hpxyz(0.255059,0.086117,1.035602));
  hpcpush(hpxyz(0.282698,0.064799,1.041209));
  hpcpush(hpxyz(0.258402,0.035835,1.033468));
  hpcpush(hpxyz(0.246791,0.009850,1.030050));
  hpcpush(hpxyz(0.246791,-0.009850,1.030050));
  hpcpush(hpxyz(0.258402,-0.035835,1.033468));
  hpcpush(hpxyz(0.282698,-0.064799,1.041209));
  hpcpush(hpxyz(0.255059,-0.086117,1.035602));
  hpcpush(hpxyz(0.242999,-0.107155,1.034665));
  hpcpush(hpxyz(0.175703,-0.077782,1.018294));
  hpcpush(hpxyz(0.185917,-0.036295,1.017783));
  hpcpush(hpxyz(0.144800,-0.029328,1.010855));
  hpcpush(hpxyz(0.137297,-0.044591,1.010366));
  hpcpush(hpxyz(0.159111,-0.052236,1.013926));
  hpcpush(hpxyz(0.140044,-0.091830,1.013926));
  hpcpush(hpxyz(0.120466,-0.079542,1.010366));
  hpcpush(hpxyz(0.113211,-0.094924,1.010855));
  hpcpush(hpxyz(0.144294,-0.122726,1.017783));
  hpcpush(hpxyz(0.170362,-0.088874,1.018294));
  hpcpush(hpxyz(0.235285,-0.123174,1.034665));
  hpcpush(hpxyz(0.226355,-0.145720,1.035602));
  hpcpush(hpxyz(0.226921,-0.180621,1.041209));
  hpcpush(hpxyz(0.189128,-0.179684,1.033468));
  hpcpush(hpxyz(0.161573,-0.186808,1.030050));
  hpcpush(hpxyz(0.146171,-0.199090,1.030050));
  hpcpush(hpxyz(0.133094,-0.224370,1.033468));
  hpcpush(hpxyz(0.125598,-0.261424,1.041209));
  hpcpush(hpxyz(0.091698,-0.253106,1.035602));
  hpcpush(hpxyz(0.067730,-0.256794,1.034665));
  hpcpush(hpxyz(0.048737,-0.185867,1.018294));
  hpcpush(hpxyz(0.087541,-0.167986,1.017783));
  hpcpush(hpxyz(0.067352,-0.131495,1.010855));
  hpcpush(hpxyz(0.050741,-0.135145,1.010366));
  hpcpush(hpxyz(0.058365,-0.156967,1.013926));
  hpcpush(hpxyz(0.015520,-0.166746,1.013926));
  hpcpush(hpxyz(0.012921,-0.143777,1.010366));
  hpcpush(hpxyz(-0.003629,-0.147695,1.010855));
  hpcpush(hpxyz(-0.005985,-0.189332,1.017783));
  hpcpush(hpxyz(0.036734,-0.188606,1.018294));
  hpcpush(hpxyz(0.050396,-0.260751,1.034665));
  hpcpush(hpxyz(0.027202,-0.267827,1.035602));
  hpcpush(hpxyz(0.000268,-0.290029,1.041209));
  hpcpush(hpxyz(-0.022563,-0.259897,1.033468));
  hpcpush(hpxyz(-0.045313,-0.242795,1.030050));
  hpcpush(hpxyz(-0.064519,-0.238411,1.030050));
  hpcpush(hpxyz(-0.092436,-0.243949,1.033468));
  hpcpush(hpxyz(-0.126080,-0.261191,1.041209));
  hpcpush(hpxyz(-0.140713,-0.229501,1.035602));
  hpcpush(hpxyz(-0.158541,-0.213062,1.034665));
  hpcpush(hpxyz(-0.114930,-0.153990,1.018294));
  hpcpush(hpxyz(-0.076756,-0.173179,1.017783));
  hpcpush(hpxyz(-0.060813,-0.134643,1.010855));
  hpcpush(hpxyz(-0.074024,-0.123933,1.010366));
  hpcpush(hpxyz(-0.086332,-0.143499,1.013926));
  hpcpush(hpxyz(-0.120690,-0.116099,1.013926));
  hpcpush(hpxyz(-0.104354,-0.099746,1.010366));
  hpcpush(hpxyz(-0.117735,-0.089250,1.010855));
  hpcpush(hpxyz(-0.151758,-0.113367,1.017783));
  hpcpush(hpxyz(-0.124555,-0.146314,1.018294));
  hpcpush(hpxyz(-0.172442,-0.201977,1.034665));
  hpcpush(hpxyz(-0.192436,-0.188254,1.035602));
  hpcpush(hpxyz(-0.226587,-0.181040,1.041209));
  hpcpush(hpxyz(-0.217264,-0.144403,1.033468));
  hpcpush(hpxyz(-0.218077,-0.115953,1.030050));
  hpcpush(hpxyz(-0.226625,-0.098204,1.030050));

  bshape(shDemonFloor[0], 10);
  hpcpush(hpxyz(-0.301493,-0.143505,1.054273));
  hpcpush(hpxyz(-0.286951,-0.089187,1.044172));
  hpcpush(hpxyz(-0.265835,-0.083073,1.038060));
  hpcpush(hpxyz(-0.254617,-0.091662,1.035969));
  hpcpush(hpxyz(-0.235025,-0.098559,1.031964));
  hpcpush(hpxyz(-0.212515,-0.090006,1.026286));
  hpcpush(hpxyz(-0.195614,-0.068094,1.021226));
  hpcpush(hpxyz(-0.191005,-0.038201,1.018795));
  hpcpush(hpxyz(-0.190797,-0.019695,1.018230));
  hpcpush(hpxyz(-0.209527,-0.022317,1.021959));
  hpcpush(hpxyz(-0.209825,-0.043455,1.022700));
  hpcpush(hpxyz(-0.217026,-0.061116,1.025103));
  hpcpush(hpxyz(-0.229748,-0.071561,1.028545));
  hpcpush(hpxyz(-0.245216,-0.072048,1.032144));
  hpcpush(hpxyz(-0.259160,-0.060979,1.034834));
  hpcpush(hpxyz(-0.268975,-0.048441,1.036674));
  hpcpush(hpxyz(-0.279355,-0.049976,1.039489));
  hpcpush(hpxyz(-0.293619,-0.069848,1.044553));
  hpcpush(hpxyz(-0.299119,-0.088435,1.047517));
  hpcpush(hpxyz(-0.309915,-0.111622,1.052856));
  hpcpush(hpxyz(-0.326351,-0.118070,1.058511));
  hpcpush(hpxyz(-0.344537,-0.119314,1.064397));
  hpcpush(hpxyz(-0.371612,-0.115703,1.073072));
  hpcpush(hpxyz(-0.386986,-0.108411,1.077734));
  hpcpush(hpxyz(-0.395533,-0.101984,1.080207));
  hpcpush(hpxyz(-0.372081,-0.100489,1.071701));
  hpcpush(hpxyz(-0.352370,-0.089767,1.064059));
  hpcpush(hpxyz(-0.339559,-0.070299,1.058415));
  hpcpush(hpxyz(-0.338745,-0.051606,1.057077));
  hpcpush(hpxyz(-0.343230,-0.037106,1.057915));
  hpcpush(hpxyz(-0.346305,-0.026537,1.058599));
  hpcpush(hpxyz(-0.349398,-0.006643,1.059303));
  hpcpush(hpxyz(-0.344437,0.011923,1.057724));
  hpcpush(hpxyz(-0.344437,-0.011923,1.057724));
  hpcpush(hpxyz(-0.349398,0.006643,1.059303));
  hpcpush(hpxyz(-0.346305,0.026537,1.058599));
  hpcpush(hpxyz(-0.343230,0.037106,1.057915));
  hpcpush(hpxyz(-0.338745,0.051606,1.057077));
  hpcpush(hpxyz(-0.339559,0.070299,1.058415));
  hpcpush(hpxyz(-0.352370,0.089767,1.064059));
  hpcpush(hpxyz(-0.372081,0.100489,1.071701));
  hpcpush(hpxyz(-0.395533,0.101984,1.080207));
  hpcpush(hpxyz(-0.386986,0.108411,1.077734));
  hpcpush(hpxyz(-0.371612,0.115703,1.073072));
  hpcpush(hpxyz(-0.344537,0.119314,1.064397));
  hpcpush(hpxyz(-0.326351,0.118070,1.058511));
  hpcpush(hpxyz(-0.309915,0.111622,1.052856));
  hpcpush(hpxyz(-0.299119,0.088435,1.047517));
  hpcpush(hpxyz(-0.293619,0.069848,1.044553));
  hpcpush(hpxyz(-0.279355,0.049976,1.039489));
  hpcpush(hpxyz(-0.268975,0.048441,1.036674));
  hpcpush(hpxyz(-0.259160,0.060979,1.034834));
  hpcpush(hpxyz(-0.245216,0.072048,1.032144));
  hpcpush(hpxyz(-0.229748,0.071561,1.028545));
  hpcpush(hpxyz(-0.217026,0.061116,1.025103));
  hpcpush(hpxyz(-0.209825,0.043455,1.022700));
  hpcpush(hpxyz(-0.209527,0.022317,1.021959));
  hpcpush(hpxyz(-0.190797,0.019695,1.018230));
  hpcpush(hpxyz(-0.191005,0.038201,1.018795));
  hpcpush(hpxyz(-0.195614,0.068094,1.021226));
  hpcpush(hpxyz(-0.212515,0.090006,1.026286));
  hpcpush(hpxyz(-0.235025,0.098559,1.031964));
  hpcpush(hpxyz(-0.254617,0.091662,1.035969));
  hpcpush(hpxyz(-0.265835,0.083073,1.038060));
  hpcpush(hpxyz(-0.286951,0.089187,1.044172));
  hpcpush(hpxyz(-0.301493,0.143505,1.054273));
  hpcpush(hpxyz(0.026467,0.332853,1.054273));
  hpcpush(hpxyz(0.066237,0.293100,1.044172));
  hpcpush(hpxyz(0.060974,0.271757,1.038060));
  hpcpush(hpxyz(0.047927,0.266336,1.035969));
  hpcpush(hpxyz(0.032158,0.252817,1.031964));
  hpcpush(hpxyz(0.028310,0.229046,1.026286));
  hpcpush(hpxyz(0.038836,0.203454,1.021226));
  hpcpush(hpxyz(0.062419,0.184516,1.018795));
  hpcpush(hpxyz(0.078342,0.175083,1.018230));
  hpcpush(hpxyz(0.085437,0.192614,1.021959));
  hpcpush(hpxyz(0.067279,0.203441,1.022700));
  hpcpush(hpxyz(0.055584,0.218508,1.025103));
  hpcpush(hpxyz(0.052900,0.234748,1.028545));
  hpcpush(hpxyz(0.060213,0.248387,1.032144));
  hpcpush(hpxyz(0.076771,0.254928,1.034834));
  hpcpush(hpxyz(0.092536,0.257159,1.036674));
  hpcpush(hpxyz(0.096397,0.266917,1.039489));
  hpcpush(hpxyz(0.086320,0.289205,1.044553));
  hpcpush(hpxyz(0.072972,0.303262,1.047517));
  hpcpush(hpxyz(0.058290,0.324205,1.052856));
  hpcpush(hpxyz(0.060924,0.341663,1.058511));
  hpcpush(hpxyz(0.068939,0.358035,1.064397));
  hpcpush(hpxyz(0.085604,0.379677,1.073072));
  hpcpush(hpxyz(0.099606,0.389345,1.077734));
  hpcpush(hpxyz(0.109446,0.393534,1.080207));
  hpcpush(hpxyz(0.099014,0.372476,1.071701));
  hpcpush(hpxyz(0.098444,0.350045,1.064059));
  hpcpush(hpxyz(0.108898,0.329216,1.058415));
  hpcpush(hpxyz(0.124681,0.319165,1.057077));
  hpcpush(hpxyz(0.139480,0.315799,1.057915));
  hpcpush(hpxyz(0.150171,0.313177,1.058599));
  hpcpush(hpxyz(0.168947,0.305909,1.059303));
  hpcpush(hpxyz(0.182544,0.292330,1.057724));
  hpcpush(hpxyz(0.161893,0.304253,1.057724));
  hpcpush(hpxyz(0.180452,0.299267,1.059303));
  hpcpush(hpxyz(0.196134,0.286640,1.058599));
  hpcpush(hpxyz(0.203750,0.278693,1.057915));
  hpcpush(hpxyz(0.214064,0.267559,1.057077));
  hpcpush(hpxyz(0.230660,0.258917,1.058415));
  hpcpush(hpxyz(0.253926,0.260278,1.064059));
  hpcpush(hpxyz(0.273066,0.271987,1.071701));
  hpcpush(hpxyz(0.286087,0.291550,1.080207));
  hpcpush(hpxyz(0.287380,0.280934,1.077734));
  hpcpush(hpxyz(0.286008,0.263974,1.073072));
  hpcpush(hpxyz(0.275598,0.238721,1.064397));
  hpcpush(hpxyz(0.265427,0.223593,1.058511));
  hpcpush(hpxyz(0.251625,0.212583,1.052856));
  hpcpush(hpxyz(0.226146,0.214827,1.047517));
  hpcpush(hpxyz(0.207299,0.219357,1.044553));
  hpcpush(hpxyz(0.182958,0.216941,1.039489));
  hpcpush(hpxyz(0.176438,0.208718,1.036674));
  hpcpush(hpxyz(0.182389,0.193949,1.034834));
  hpcpush(hpxyz(0.185004,0.176339,1.032144));
  hpcpush(hpxyz(0.176847,0.163187,1.028545));
  hpcpush(hpxyz(0.161441,0.157392,1.025103));
  hpcpush(hpxyz(0.142545,0.159986,1.022700));
  hpcpush(hpxyz(0.124090,0.170298,1.021959));
  hpcpush(hpxyz(0.112455,0.155388,1.018230));
  hpcpush(hpxyz(0.128586,0.146315,1.018795));
  hpcpush(hpxyz(0.156778,0.135360,1.021226));
  hpcpush(hpxyz(0.184205,0.139040,1.026286));
  hpcpush(hpxyz(0.202867,0.154258,1.031964));
  hpcpush(hpxyz(0.206690,0.174674,1.035969));
  hpcpush(hpxyz(0.204861,0.188683,1.038060));
  hpcpush(hpxyz(0.220714,0.203913,1.044172));
  hpcpush(hpxyz(0.275026,0.189348,1.054273));
  hpcpush(hpxyz(0.275026,-0.189348,1.054273));
  hpcpush(hpxyz(0.220714,-0.203913,1.044172));
  hpcpush(hpxyz(0.204861,-0.188683,1.038060));
  hpcpush(hpxyz(0.206690,-0.174674,1.035969));
  hpcpush(hpxyz(0.202867,-0.154258,1.031964));
  hpcpush(hpxyz(0.184205,-0.139040,1.026286));
  hpcpush(hpxyz(0.156778,-0.135360,1.021226));
  hpcpush(hpxyz(0.128586,-0.146315,1.018795));
  hpcpush(hpxyz(0.112455,-0.155388,1.018230));
  hpcpush(hpxyz(0.124090,-0.170298,1.021959));
  hpcpush(hpxyz(0.142545,-0.159986,1.022700));
  hpcpush(hpxyz(0.161441,-0.157392,1.025103));
  hpcpush(hpxyz(0.176847,-0.163187,1.028545));
  hpcpush(hpxyz(0.185004,-0.176339,1.032144));
  hpcpush(hpxyz(0.182389,-0.193949,1.034834));
  hpcpush(hpxyz(0.176438,-0.208718,1.036674));
  hpcpush(hpxyz(0.182958,-0.216941,1.039489));
  hpcpush(hpxyz(0.207299,-0.219357,1.044553));
  hpcpush(hpxyz(0.226146,-0.214827,1.047517));
  hpcpush(hpxyz(0.251625,-0.212583,1.052856));
  hpcpush(hpxyz(0.265427,-0.223593,1.058511));
  hpcpush(hpxyz(0.275598,-0.238721,1.064397));
  hpcpush(hpxyz(0.286008,-0.263974,1.073072));
  hpcpush(hpxyz(0.287380,-0.280934,1.077734));
  hpcpush(hpxyz(0.286087,-0.291550,1.080207));
  hpcpush(hpxyz(0.273066,-0.271987,1.071701));
  hpcpush(hpxyz(0.253926,-0.260278,1.064059));
  hpcpush(hpxyz(0.230660,-0.258917,1.058415));
  hpcpush(hpxyz(0.214064,-0.267559,1.057077));
  hpcpush(hpxyz(0.203750,-0.278693,1.057915));
  hpcpush(hpxyz(0.196134,-0.286640,1.058599));
  hpcpush(hpxyz(0.180452,-0.299267,1.059303));
  hpcpush(hpxyz(0.161893,-0.304253,1.057724));
  hpcpush(hpxyz(0.182544,-0.292330,1.057724));
  hpcpush(hpxyz(0.168947,-0.305909,1.059303));
  hpcpush(hpxyz(0.150171,-0.313177,1.058599));
  hpcpush(hpxyz(0.139480,-0.315799,1.057915));
  hpcpush(hpxyz(0.124681,-0.319165,1.057077));
  hpcpush(hpxyz(0.108898,-0.329216,1.058415));
  hpcpush(hpxyz(0.098444,-0.350045,1.064059));
  hpcpush(hpxyz(0.099014,-0.372476,1.071701));
  hpcpush(hpxyz(0.109446,-0.393534,1.080207));
  hpcpush(hpxyz(0.099606,-0.389345,1.077734));
  hpcpush(hpxyz(0.085604,-0.379677,1.073072));
  hpcpush(hpxyz(0.068939,-0.358035,1.064397));
  hpcpush(hpxyz(0.060924,-0.341663,1.058511));
  hpcpush(hpxyz(0.058290,-0.324205,1.052856));
  hpcpush(hpxyz(0.072972,-0.303262,1.047517));
  hpcpush(hpxyz(0.086320,-0.289205,1.044553));
  hpcpush(hpxyz(0.096397,-0.266917,1.039489));
  hpcpush(hpxyz(0.092536,-0.257159,1.036674));
  hpcpush(hpxyz(0.076771,-0.254928,1.034834));
  hpcpush(hpxyz(0.060213,-0.248387,1.032144));
  hpcpush(hpxyz(0.052900,-0.234748,1.028545));
  hpcpush(hpxyz(0.055584,-0.218508,1.025103));
  hpcpush(hpxyz(0.067279,-0.203441,1.022700));
  hpcpush(hpxyz(0.085437,-0.192614,1.021959));
  hpcpush(hpxyz(0.078342,-0.175083,1.018230));
  hpcpush(hpxyz(0.062419,-0.184516,1.018795));
  hpcpush(hpxyz(0.038836,-0.203454,1.021226));
  hpcpush(hpxyz(0.028310,-0.229046,1.026286));
  hpcpush(hpxyz(0.032158,-0.252817,1.031964));
  hpcpush(hpxyz(0.047927,-0.266336,1.035969));
  hpcpush(hpxyz(0.060974,-0.271757,1.038060));
  hpcpush(hpxyz(0.066237,-0.293100,1.044172));
  hpcpush(hpxyz(0.026467,-0.332853,1.054273));
  hpcpush(hpxyz(-0.301493,-0.143505,1.054273));

  bshape(shCaveFloor[0], 10);
  hpcpush(hpxyz(-0.234236,0.156157,1.038870));
  hpcpush(hpxyz(-0.104249,0.246637,1.035228));
  hpcpush(hpxyz(-0.046397,0.291271,1.042589));
  hpcpush(hpxyz(0.016871,0.307576,1.046369));
  hpcpush(hpxyz(0.102393,0.312431,1.052662));
  hpcpush(hpxyz(0.175295,0.270792,1.050741));
  hpcpush(hpxyz(0.252411,0.200098,1.050595));
  hpcpush(hpxyz(0.252354,0.124776,1.038870));
  hpcpush(hpxyz(0.265719,-0.033037,1.035228));
  hpcpush(hpxyz(0.275447,-0.105454,1.042589));
  hpcpush(hpxyz(0.257933,-0.168399,1.046369));
  hpcpush(hpxyz(0.219377,-0.244891,1.052662));
  hpcpush(hpxyz(0.146865,-0.287206,1.050741));
  hpcpush(hpxyz(0.047084,-0.318643,1.050595));
  hpcpush(hpxyz(-0.018118,-0.280933,1.038870));
  hpcpush(hpxyz(-0.161470,-0.213601,1.035228));
  hpcpush(hpxyz(-0.229049,-0.185817,1.042589));
  hpcpush(hpxyz(-0.274804,-0.139177,1.046369));
  hpcpush(hpxyz(-0.321770,-0.067540,1.052662));
  hpcpush(hpxyz(-0.322160,0.016414,1.050741));
  hpcpush(hpxyz(-0.299495,0.118545,1.050595));
  hpcpush(hpxyz(-0.234236,0.156157,1.038870));

  bshape(shCaveFloor[1], 10);
  hpcpush(hpxyz(-0.275790,0.020429,1.037534));
  hpcpush(hpxyz(-0.270081,0.108032,1.041448));
  hpcpush(hpxyz(-0.241466,0.175612,1.043621));
  hpcpush(hpxyz(-0.155980,0.228358,1.037534));
  hpcpush(hpxyz(-0.083930,0.278515,1.041448));
  hpcpush(hpxyz(-0.013253,0.298278,1.043621));
  hpcpush(hpxyz(0.081286,0.264329,1.037534));
  hpcpush(hpxyz(0.165423,0.239270,1.041448));
  hpcpush(hpxyz(0.224940,0.196335,1.043621));
  hpcpush(hpxyz(0.257342,0.101255,1.037534));
  hpcpush(hpxyz(0.290208,0.019850,1.041448));
  hpcpush(hpxyz(0.293748,-0.053452,1.043621));
  hpcpush(hpxyz(0.239614,-0.138066,1.037534));
  hpcpush(hpxyz(0.196461,-0.214518,1.041448));
  hpcpush(hpxyz(0.141358,-0.262989,1.043621));
  hpcpush(hpxyz(0.041452,-0.273421,1.037534));
  hpcpush(hpxyz(-0.045225,-0.287349,1.041448));
  hpcpush(hpxyz(-0.117477,-0.274489,1.043621));
  hpcpush(hpxyz(-0.187924,-0.202884,1.037534));
  hpcpush(hpxyz(-0.252856,-0.143801,1.041448));
  hpcpush(hpxyz(-0.287850,-0.079294,1.043621));
  hpcpush(hpxyz(-0.275790,0.020429,1.037534));
  
  bshape(shDesertFloor[0], 10);
  hpcpush(hpxyz(-0.122282,0.287492,1.047666));
  hpcpush(hpxyz(-0.111151,0.302069,1.050524));
  hpcpush(hpxyz(-0.092261,0.322915,1.054887));
  hpcpush(hpxyz(-0.071518,0.368383,1.068092));
  hpcpush(hpxyz(-0.014386,0.321713,1.050574));
  hpcpush(hpxyz(0.014126,0.287661,1.040648));
  hpcpush(hpxyz(0.052673,0.283918,1.040857));
  hpcpush(hpxyz(0.144166,0.292263,1.051761));
  hpcpush(hpxyz(0.201841,0.240906,1.048225));
  hpcpush(hpxyz(0.236802,0.171166,1.041812));
  hpcpush(hpxyz(0.187940,0.142834,1.027484));
  hpcpush(hpxyz(0.179062,0.090148,1.019897));
  hpcpush(hpxyz(0.306355,-0.036347,1.046506));
  hpcpush(hpxyz(0.310117,-0.037847,1.047666));
  hpcpush(hpxyz(0.317175,-0.054775,1.050524));
  hpcpush(hpxyz(0.325783,-0.081557,1.054887));
  hpcpush(hpxyz(0.354788,-0.122255,1.068092));
  hpcpush(hpxyz(0.285805,-0.148398,1.050574));
  hpcpush(hpxyz(0.242058,-0.156064,1.040648));
  hpcpush(hpxyz(0.219544,-0.187575,1.040857));
  hpcpush(hpxyz(0.181024,-0.270983,1.051761));
  hpcpush(hpxyz(0.107711,-0.295252,1.048225));
  hpcpush(hpxyz(0.029834,-0.290659,1.041812));
  hpcpush(hpxyz(0.029728,-0.234177,1.027484));
  hpcpush(hpxyz(-0.011460,-0.200146,1.019897));
  hpcpush(hpxyz(-0.184655,-0.247137,1.046506));
  hpcpush(hpxyz(-0.187835,-0.249645,1.047666));
  hpcpush(hpxyz(-0.206024,-0.247294,1.050524));
  hpcpush(hpxyz(-0.233522,-0.241358,1.054887));
  hpcpush(hpxyz(-0.283270,-0.246127,1.068092));
  hpcpush(hpxyz(-0.271419,-0.173315,1.050574));
  hpcpush(hpxyz(-0.256185,-0.131597,1.040648));
  hpcpush(hpxyz(-0.272216,-0.096343,1.040857));
  hpcpush(hpxyz(-0.325190,-0.021280,1.051761));
  hpcpush(hpxyz(-0.309551,0.054346,1.048225));
  hpcpush(hpxyz(-0.266635,0.119493,1.041812));
  hpcpush(hpxyz(-0.217668,0.091343,1.027484));
  hpcpush(hpxyz(-0.167602,0.109998,1.019897));
  hpcpush(hpxyz(-0.121700,0.283485,1.046506));
  hpcpush(hpxyz(-0.122282,0.287492,1.047666));

  bshape(shDesertFloor[1], 10);
  hpcpush(hpxyz(-0.336141,-0.153280,1.066061));
  hpcpush(hpxyz(-0.336530,-0.140779,1.064458));
  hpcpush(hpxyz(-0.323000,-0.116492,1.057308));
  hpcpush(hpxyz(-0.295897,-0.085654,1.046371));
  hpcpush(hpxyz(-0.271537,-0.031871,1.036701));
  hpcpush(hpxyz(-0.271537,0.031871,1.036701));
  hpcpush(hpxyz(-0.295897,0.085654,1.046371));
  hpcpush(hpxyz(-0.323000,0.116492,1.057308));
  hpcpush(hpxyz(-0.336530,0.140779,1.064458));
  hpcpush(hpxyz(-0.336141,0.153280,1.066061));
  hpcpush(hpxyz(-0.329419,0.167237,1.066061));
  hpcpush(hpxyz(-0.319889,0.175335,1.064458));
  hpcpush(hpxyz(-0.292464,0.179900,1.057308));
  hpcpush(hpxyz(-0.251456,0.177937,1.046371));
  hpcpush(hpxyz(-0.194218,0.192425,1.036701));
  hpcpush(hpxyz(-0.144383,0.232167,1.036701));
  hpcpush(hpxyz(-0.117521,0.284746,1.046371));
  hpcpush(hpxyz(-0.110310,0.325163,1.057308));
  hpcpush(hpxyz(-0.099757,0.350884,1.064458));
  hpcpush(hpxyz(-0.089741,0.358374,1.066061));
  hpcpush(hpxyz(-0.074639,0.361821,1.066061));
  hpcpush(hpxyz(-0.062365,0.359418,1.064458));
  hpcpush(hpxyz(-0.041697,0.340824,1.057308));
  hpcpush(hpxyz(-0.017664,0.307538,1.046371));
  hpcpush(hpxyz(0.029351,0.271821,1.036701));
  hpcpush(hpxyz(0.091494,0.257638,1.036701));
  hpcpush(hpxyz(0.149350,0.269418,1.046371));
  hpcpush(hpxyz(0.185445,0.288980,1.057308));
  hpcpush(hpxyz(0.212135,0.296766,1.064458));
  hpcpush(hpxyz(0.224235,0.293605,1.066061));
  hpcpush(hpxyz(0.236346,0.283946,1.066061));
  hpcpush(hpxyz(0.242121,0.272853,1.064458));
  hpcpush(hpxyz(0.240469,0.245100,1.057308));
  hpcpush(hpxyz(0.229430,0.205557,1.046371));
  hpcpush(hpxyz(0.230819,0.146530,1.036701));
  hpcpush(hpxyz(0.258475,0.089101,1.036701));
  hpcpush(hpxyz(0.303758,0.051213,1.046371));
  hpcpush(hpxyz(0.341557,0.035189,1.057308));
  hpcpush(hpxyz(0.364285,0.019177,1.064458));
  hpcpush(hpxyz(0.369358,0.007745,1.066061));
  hpcpush(hpxyz(0.369358,-0.007745,1.066061));
  hpcpush(hpxyz(0.364285,-0.019177,1.064458));
  hpcpush(hpxyz(0.341557,-0.035189,1.057308));
  hpcpush(hpxyz(0.303758,-0.051213,1.046371));
  hpcpush(hpxyz(0.258475,-0.089101,1.036701));
  hpcpush(hpxyz(0.230819,-0.146530,1.036701));
  hpcpush(hpxyz(0.229430,-0.205557,1.046371));
  hpcpush(hpxyz(0.240469,-0.245100,1.057308));
  hpcpush(hpxyz(0.242121,-0.272853,1.064458));
  hpcpush(hpxyz(0.236346,-0.283946,1.066061));
  hpcpush(hpxyz(0.224235,-0.293605,1.066061));
  hpcpush(hpxyz(0.212135,-0.296766,1.064458));
  hpcpush(hpxyz(0.185445,-0.288980,1.057308));
  hpcpush(hpxyz(0.149350,-0.269418,1.046371));
  hpcpush(hpxyz(0.091494,-0.257638,1.036701));
  hpcpush(hpxyz(0.029351,-0.271821,1.036701));
  hpcpush(hpxyz(-0.017664,-0.307538,1.046371));
  hpcpush(hpxyz(-0.041697,-0.340824,1.057308));
  hpcpush(hpxyz(-0.062365,-0.359418,1.064458));
  hpcpush(hpxyz(-0.074639,-0.361821,1.066061));
  hpcpush(hpxyz(-0.089741,-0.358374,1.066061));
  hpcpush(hpxyz(-0.099757,-0.350884,1.064458));
  hpcpush(hpxyz(-0.110310,-0.325163,1.057308));
  hpcpush(hpxyz(-0.117521,-0.284746,1.046371));
  hpcpush(hpxyz(-0.144383,-0.232167,1.036701));
  hpcpush(hpxyz(-0.194218,-0.192425,1.036701));
  hpcpush(hpxyz(-0.251456,-0.177937,1.046371));
  hpcpush(hpxyz(-0.292464,-0.179900,1.057308));
  hpcpush(hpxyz(-0.319889,-0.175335,1.064458));
  hpcpush(hpxyz(-0.329419,-0.167237,1.066061));
  hpcpush(hpxyz(-0.336141,-0.153280,1.066061));

  bshape(shFloor[0], 11);
  for(int t=0; t<=6; t++) hpcpush(ddi(7 + t*14, hexf*.8) * C0);

  bshape(shFloor[1], 11);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*12, hexf*.94) * C0);
  
  bshape(shMFloor[0], 11);
  for(int t=0; t<=6; t++) hpcpush(ddi(7 + t*14, hexf*.7) * C0);

  bshape(shMFloor[1], 11);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*12, hexf*.8) * C0);
  
  bshape(shBFloor[0], 11);
  for(int t=0; t<=6; t++) hpcpush(ddi(7 + t*14, hexf*.1) * C0);

  bshape(shBFloor[1], 11);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*12, hexf*.1) * C0);
  
  // walls etc
  
  bshape(shGiantStar[1], 12);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*36, -hexf*2.4) * C0);

  bshape(shGiantStar[0], 12);
  for(int t=0; t<=6; t++) {
    hpcpush(ddi(t*14, -hexf*2.4) * C0);
    hpcpush(ddi(t*14+7, hexf*1.5) * C0);
    }
  hpcpush(ddi(0, -hexf*2.4) * C0);

  bshape(shMirror, 13);
  for(int t=0; t<=6; t++) hpcpush(ddi(7 + t*14, hexf*.7) * C0);

  bshape(shWall[0], 13);
  for(int t=0; t<=6; t++) {
    hpcpush(ddi(7 + t*14, hexf*.8) * C0);
    hpcpush(ddi(14 + t*14, hexf*.2) * C0);
    }
    
  bshape(shWall[1], 13);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*36, hexf*.94) * C0);

  bshape(shCross, 13);
  for(int i=0; i<=84; i+=7)
    hpcpush(ddi(i, hexf * (i%3 ? 0.8 : 0.3)) * C0);
  
  // items
  
  bshape(shDaisy, 21);
  for(int t=0; t<=6; t++) {
    hpcpush(ddi(t*14, hexf*.8*3/4) * C0);
    hpcpush(ddi(t*14+7, hexf*-.5*3/4) * C0);
    }
  hpcpush(ddi(0, hexf*.6) * C0);

  bshape(shTriangle, 21);
  for(int t=0; t<=4; t++) {
    hpcpush(ddi(t*28, hexf*.5) * C0);
    }

  bshape(shGem[0], 21);
  for(int t=0; t<=6; t++) {
    hpcpush(ddi(7 + t*14, hexf*.4) * C0);
    hpcpush(ddi(14 + t*14, hexf*.1) * C0);
    }
  
  bshape(shGem[1], 21);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*36, hexf*.5) * C0);

  bshape(shStar, 21);
  for(int t=0; t<84; t+=6) {
    hpcpush(ddi(t,   hexf*.2) * C0);
    hpcpush(ddi(t+3,   hexf*.6) * C0);
    }

  bshape(shDisk, 21);
  for(int i=0; i<=84; i+=3)
    hpcpush(ddi(i, crossf * .2) * C0);
  
  bshape(shRing, 21);
  for(int i=0; i<=84; i+=3)
    hpcpush(ddi(i, crossf * .25) * C0);
  for(int i=84; i>=0; i--)
    hpcpush(ddi(i, crossf * .30) * C0);
  hpcpush(ddi(0, crossf * .25) * C0);
  
  bshape(shNecro, 21);
    hpcpush(hpxyz(-0.280120,0.002558,1.038496));
  hpcpush(hpxyz(-0.252806,0.037921,1.032157));
  hpcpush(hpxyz(-0.063204,0.010733,1.002053));
  hpcpush(hpxyz(0.000000,0.061995,1.001920));
  hpcpush(hpxyz(0.057203,0.013109,1.001721));
  hpcpush(hpxyz(0.155766,0.023121,1.012323));
  hpcpush(hpxyz(0.174234,0.055215,1.016566));
  hpcpush(hpxyz(0.256152,0.057064,1.033862));
  hpcpush(hpxyz(0.286380,0.030821,1.040655));
  hpcpush(hpxyz(0.251170,0.025243,1.031370));
  hpcpush(hpxyz(0.227431,0.033740,1.026091));
  hpcpush(hpxyz(0.210837,0.017363,1.022132));
  hpcpush(hpxyz(0.227179,0.007489,1.025508));
  hpcpush(hpxyz(0.245429,0.020138,1.029874));
  hpcpush(hpxyz(0.286335,0.028248,1.040570));
  hpcpush(hpxyz(0.289108,0.001285,1.040954));
  hpcpush(hpxyz(0.189404,0.003690,1.017786));
  hpcpush(hpxyz(0.186892,0.025821,1.017642));
  hpcpush(hpxyz(0.176349,0.017145,1.015575));
  hpcpush(hpxyz(0.184160,0.011050,1.016876));
  hpcpush(hpxyz(0.177605,0.006124,1.015668));
  hpcpush(hpxyz(0.184138,0.001228,1.016813));
  hpcpush(hpxyz(0.184138,-0.001228,1.016813));
  hpcpush(hpxyz(0.177605,-0.006124,1.015668));
  hpcpush(hpxyz(0.184160,-0.011050,1.016876));
  hpcpush(hpxyz(0.176349,-0.017145,1.015575));
  hpcpush(hpxyz(0.186892,-0.025821,1.017642));
  hpcpush(hpxyz(0.189404,-0.003690,1.017786));
  hpcpush(hpxyz(0.289108,-0.001285,1.040954));
  hpcpush(hpxyz(0.286335,-0.028248,1.040570));
  hpcpush(hpxyz(0.245429,-0.020138,1.029874));
  hpcpush(hpxyz(0.227179,-0.007489,1.025508));
  hpcpush(hpxyz(0.210837,-0.017363,1.022132));
  hpcpush(hpxyz(0.227431,-0.033740,1.026091));
  hpcpush(hpxyz(0.251170,-0.025243,1.031370));
  hpcpush(hpxyz(0.286380,-0.030821,1.040655));
  hpcpush(hpxyz(0.256152,-0.057064,1.033862));
  hpcpush(hpxyz(0.174234,-0.055215,1.016566));
  hpcpush(hpxyz(0.155766,-0.023121,1.012323));
  hpcpush(hpxyz(0.057203,-0.013109,1.001721));
  hpcpush(hpxyz(0.000000,-0.061995,1.001920));
  hpcpush(hpxyz(-0.063204,-0.010733,1.002053));
  hpcpush(hpxyz(-0.252806,-0.037921,1.032157));
  hpcpush(hpxyz(-0.280120,-0.002558,1.038496));
  hpcpush(hpxyz(-0.280120,0.002558,1.038496));

  bshape(shStatue, 21);
  hpcpush(hpxyz(-0.047663,0.032172,1.001652));
  hpcpush(hpxyz(-0.047670,0.034561,1.001732));
  hpcpush(hpxyz(-0.054881,0.039371,1.002278));
  hpcpush(hpxyz(-0.080231,0.043109,1.004139));
  hpcpush(hpxyz(-0.103440,0.045706,1.006374));
  hpcpush(hpxyz(-0.148323,0.042552,1.011835));
  hpcpush(hpxyz(-0.188278,0.031995,1.018073));
  hpcpush(hpxyz(-0.239693,0.002510,1.028328));
  hpcpush(hpxyz(-0.257087,-0.036727,1.033171));
  hpcpush(hpxyz(-0.283314,-0.026921,1.039708));
  hpcpush(hpxyz(-0.290985,0.034764,1.042056));
  hpcpush(hpxyz(-0.262852,0.081663,1.037189));
  hpcpush(hpxyz(-0.198209,0.104714,1.024818));
  hpcpush(hpxyz(-0.140842,0.107775,1.015604));
  hpcpush(hpxyz(-0.088218,0.099094,1.008763));
  hpcpush(hpxyz(-0.057645,0.088869,1.005595));
  hpcpush(hpxyz(-0.035877,0.075342,1.003476));
  hpcpush(hpxyz(-0.023854,0.060828,1.002132));
  hpcpush(hpxyz(-0.022642,0.053625,1.001693));
  hpcpush(hpxyz(-0.011901,0.044034,1.001040));
  hpcpush(hpxyz(-0.007144,0.050008,1.001275));
  hpcpush(hpxyz(-0.010953,0.056452,1.001652));
  hpcpush(hpxyz(-0.009270,0.058147,1.001732));
  hpcpush(hpxyz(-0.010967,0.066646,1.002278));
  hpcpush(hpxyz(-0.026249,0.087215,1.004139));
  hpcpush(hpxyz(-0.040824,0.105462,1.006374));
  hpcpush(hpxyz(-0.074791,0.134969,1.011835));
  hpcpush(hpxyz(-0.110509,0.155757,1.018073));
  hpcpush(hpxyz(-0.167714,0.171263,1.028328));
  hpcpush(hpxyz(-0.207758,0.155818,1.033171));
  hpcpush(hpxyz(-0.219370,0.181297,1.039708));
  hpcpush(hpxyz(-0.181176,0.230339,1.042056));
  hpcpush(hpxyz(-0.128120,0.243609,1.037189));
  hpcpush(hpxyz(-0.066111,0.214199,1.024818));
  hpcpush(hpxyz(-0.023382,0.175799,1.015604));
  hpcpush(hpxyz(0.007691,0.132449,1.008763));
  hpcpush(hpxyz(0.022079,0.103601,1.005595));
  hpcpush(hpxyz(0.027906,0.078644,1.003476));
  hpcpush(hpxyz(0.026145,0.059880,1.002132));
  hpcpush(hpxyz(0.021909,0.053929,1.001693));
  hpcpush(hpxyz(0.022722,0.039552,1.001040));
  hpcpush(hpxyz(0.030310,0.040413,1.001275));
  hpcpush(hpxyz(0.032172,0.047663,1.001652));
  hpcpush(hpxyz(0.034561,0.047670,1.001732));
  hpcpush(hpxyz(0.039371,0.054881,1.002278));
  hpcpush(hpxyz(0.043109,0.080231,1.004139));
  hpcpush(hpxyz(0.045706,0.103440,1.006374));
  hpcpush(hpxyz(0.042552,0.148323,1.011835));
  hpcpush(hpxyz(0.031995,0.188278,1.018073));
  hpcpush(hpxyz(0.002510,0.239693,1.028328));
  hpcpush(hpxyz(-0.036727,0.257087,1.033171));
  hpcpush(hpxyz(-0.026921,0.283314,1.039708));
  hpcpush(hpxyz(0.034764,0.290985,1.042056));
  hpcpush(hpxyz(0.081663,0.262852,1.037189));
  hpcpush(hpxyz(0.104714,0.198209,1.024818));
  hpcpush(hpxyz(0.107775,0.140842,1.015604));
  hpcpush(hpxyz(0.099094,0.088218,1.008763));
  hpcpush(hpxyz(0.088869,0.057645,1.005595));
  hpcpush(hpxyz(0.075342,0.035877,1.003476));
  hpcpush(hpxyz(0.060828,0.023854,1.002132));
  hpcpush(hpxyz(0.053625,0.022642,1.001693));
  hpcpush(hpxyz(0.044034,0.011901,1.001040));
  hpcpush(hpxyz(0.050008,0.007144,1.001275));
  hpcpush(hpxyz(0.056452,0.010953,1.001652));
  hpcpush(hpxyz(0.058147,0.009270,1.001732));
  hpcpush(hpxyz(0.066646,0.010967,1.002278));
  hpcpush(hpxyz(0.087215,0.026249,1.004139));
  hpcpush(hpxyz(0.105462,0.040824,1.006374));
  hpcpush(hpxyz(0.134969,0.074791,1.011835));
  hpcpush(hpxyz(0.155757,0.110509,1.018073));
  hpcpush(hpxyz(0.171263,0.167714,1.028328));
  hpcpush(hpxyz(0.155818,0.207758,1.033171));
  hpcpush(hpxyz(0.181297,0.219370,1.039708));
  hpcpush(hpxyz(0.230339,0.181176,1.042056));
  hpcpush(hpxyz(0.243609,0.128120,1.037189));
  hpcpush(hpxyz(0.214199,0.066111,1.024818));
  hpcpush(hpxyz(0.175799,0.023382,1.015604));
  hpcpush(hpxyz(0.132449,-0.007691,1.008763));
  hpcpush(hpxyz(0.103601,-0.022079,1.005595));
  hpcpush(hpxyz(0.078644,-0.027906,1.003476));
  hpcpush(hpxyz(0.059880,-0.026145,1.002132));
  hpcpush(hpxyz(0.053929,-0.021909,1.001693));
  hpcpush(hpxyz(0.039552,-0.022722,1.001040));
  hpcpush(hpxyz(0.040413,-0.030310,1.001275));
  hpcpush(hpxyz(0.047663,-0.032172,1.001652));
  hpcpush(hpxyz(0.047670,-0.034561,1.001732));
  hpcpush(hpxyz(0.054881,-0.039371,1.002278));
  hpcpush(hpxyz(0.080231,-0.043109,1.004139));
  hpcpush(hpxyz(0.103440,-0.045706,1.006374));
  hpcpush(hpxyz(0.148323,-0.042552,1.011835));
  hpcpush(hpxyz(0.188278,-0.031995,1.018073));
  hpcpush(hpxyz(0.239693,-0.002510,1.028328));
  hpcpush(hpxyz(0.257087,0.036727,1.033171));
  hpcpush(hpxyz(0.283314,0.026921,1.039708));
  hpcpush(hpxyz(0.290985,-0.034764,1.042056));
  hpcpush(hpxyz(0.262852,-0.081663,1.037189));
  hpcpush(hpxyz(0.198209,-0.104714,1.024818));
  hpcpush(hpxyz(0.140842,-0.107775,1.015604));
  hpcpush(hpxyz(0.088218,-0.099094,1.008763));
  hpcpush(hpxyz(0.057645,-0.088869,1.005595));
  hpcpush(hpxyz(0.035877,-0.075342,1.003476));
  hpcpush(hpxyz(0.023854,-0.060828,1.002132));
  hpcpush(hpxyz(0.022642,-0.053625,1.001693));
  hpcpush(hpxyz(0.011901,-0.044034,1.001040));
  hpcpush(hpxyz(0.007144,-0.050008,1.001275));
  hpcpush(hpxyz(0.010953,-0.056452,1.001652));
  hpcpush(hpxyz(0.009270,-0.058147,1.001732));
  hpcpush(hpxyz(0.010967,-0.066646,1.002278));
  hpcpush(hpxyz(0.026249,-0.087215,1.004139));
  hpcpush(hpxyz(0.040824,-0.105462,1.006374));
  hpcpush(hpxyz(0.074791,-0.134969,1.011835));
  hpcpush(hpxyz(0.110509,-0.155757,1.018073));
  hpcpush(hpxyz(0.167714,-0.171263,1.028328));
  hpcpush(hpxyz(0.207758,-0.155818,1.033171));
  hpcpush(hpxyz(0.219370,-0.181297,1.039708));
  hpcpush(hpxyz(0.181176,-0.230339,1.042056));
  hpcpush(hpxyz(0.128120,-0.243609,1.037189));
  hpcpush(hpxyz(0.066111,-0.214199,1.024818));
  hpcpush(hpxyz(0.023382,-0.175799,1.015604));
  hpcpush(hpxyz(-0.007691,-0.132449,1.008763));
  hpcpush(hpxyz(-0.022079,-0.103601,1.005595));
  hpcpush(hpxyz(-0.027906,-0.078644,1.003476));
  hpcpush(hpxyz(-0.026145,-0.059880,1.002132));
  hpcpush(hpxyz(-0.021909,-0.053929,1.001693));
  hpcpush(hpxyz(-0.022722,-0.039552,1.001040));
  hpcpush(hpxyz(-0.030310,-0.040413,1.001275));
  hpcpush(hpxyz(-0.032172,-0.047663,1.001652));
  hpcpush(hpxyz(-0.034561,-0.047670,1.001732));
  hpcpush(hpxyz(-0.039371,-0.054881,1.002278));
  hpcpush(hpxyz(-0.043109,-0.080231,1.004139));
  hpcpush(hpxyz(-0.045706,-0.103440,1.006374));
  hpcpush(hpxyz(-0.042552,-0.148323,1.011835));
  hpcpush(hpxyz(-0.031995,-0.188278,1.018073));
  hpcpush(hpxyz(-0.002510,-0.239693,1.028328));
  hpcpush(hpxyz(0.036727,-0.257087,1.033171));
  hpcpush(hpxyz(0.026921,-0.283314,1.039708));
  hpcpush(hpxyz(-0.034764,-0.290985,1.042056));
  hpcpush(hpxyz(-0.081663,-0.262852,1.037189));
  hpcpush(hpxyz(-0.104714,-0.198209,1.024818));
  hpcpush(hpxyz(-0.107775,-0.140842,1.015604));
  hpcpush(hpxyz(-0.099094,-0.088218,1.008763));
  hpcpush(hpxyz(-0.088869,-0.057645,1.005595));
  hpcpush(hpxyz(-0.075342,-0.035877,1.003476));
  hpcpush(hpxyz(-0.060828,-0.023854,1.002132));
  hpcpush(hpxyz(-0.053625,-0.022642,1.001693));
  hpcpush(hpxyz(-0.044034,-0.011901,1.001040));
  hpcpush(hpxyz(-0.050008,-0.007144,1.001275));
  hpcpush(hpxyz(-0.056452,-0.010953,1.001652));
  hpcpush(hpxyz(-0.058147,-0.009270,1.001732));
  hpcpush(hpxyz(-0.066646,-0.010967,1.002278));
  hpcpush(hpxyz(-0.087215,-0.026249,1.004139));
  hpcpush(hpxyz(-0.105462,-0.040824,1.006374));
  hpcpush(hpxyz(-0.134969,-0.074791,1.011835));
  hpcpush(hpxyz(-0.155757,-0.110509,1.018073));
  hpcpush(hpxyz(-0.171263,-0.167714,1.028328));
  hpcpush(hpxyz(-0.155818,-0.207758,1.033171));
  hpcpush(hpxyz(-0.181297,-0.219370,1.039708));
  hpcpush(hpxyz(-0.230339,-0.181176,1.042056));
  hpcpush(hpxyz(-0.243609,-0.128120,1.037189));
  hpcpush(hpxyz(-0.214199,-0.066111,1.024818));
  hpcpush(hpxyz(-0.175799,-0.023382,1.015604));
  hpcpush(hpxyz(-0.132449,0.007691,1.008763));
  hpcpush(hpxyz(-0.103601,0.022079,1.005595));
  hpcpush(hpxyz(-0.078644,0.027906,1.003476));
  hpcpush(hpxyz(-0.059880,0.026145,1.002132));
  hpcpush(hpxyz(-0.053929,0.021909,1.001693));
  hpcpush(hpxyz(-0.039552,0.022722,1.001040));
  hpcpush(hpxyz(-0.040413,0.030310,1.001275));
  hpcpush(hpxyz(-0.047663,0.032172,1.001652));
  
  bshape(shKey, 21);
  hpcpush(hpxyz(-0.280212,-0.017913,1.038672));
  hpcpush(hpxyz(-0.279685,0.060299,1.040125));
  hpcpush(hpxyz(-0.258790,0.115302,1.039359));
  hpcpush(hpxyz(-0.228319,0.104699,1.031063));
  hpcpush(hpxyz(-0.239075,0.056623,1.029739));
  hpcpush(hpxyz(-0.213944,0.046023,1.023665));
  hpcpush(hpxyz(-0.200310,0.089579,1.023791));
  hpcpush(hpxyz(-0.168273,0.078609,1.017101));
  hpcpush(hpxyz(-0.179201,0.040504,1.016737));
  hpcpush(hpxyz(-0.150719,0.029171,1.011715));
  hpcpush(hpxyz(-0.146064,0.060860,1.012442));
  hpcpush(hpxyz(-0.115859,0.053102,1.008089));
  hpcpush(hpxyz(-0.119288,0.019279,1.007274));
  hpcpush(hpxyz(-0.087371,0.011969,1.003881));
  hpcpush(hpxyz(-0.080285,0.050328,1.004479));
  hpcpush(hpxyz(-0.068304,0.065908,1.004495));
  hpcpush(hpxyz(-0.052728,0.079091,1.004508));
  hpcpush(hpxyz(-0.033525,0.083811,1.004066));
  hpcpush(hpxyz(-0.007181,0.087363,1.003835));
  hpcpush(hpxyz(0.025157,0.089847,1.004343));
  hpcpush(hpxyz(0.044329,0.082667,1.004390));
  hpcpush(hpxyz(0.061077,0.068263,1.004186));
  hpcpush(hpxyz(0.074262,0.055098,1.004266));
  hpcpush(hpxyz(0.088682,0.034754,1.004526));
  hpcpush(hpxyz(0.093450,0.008387,1.004392));
  hpcpush(hpxyz(0.092228,-0.005989,1.004262));
  hpcpush(hpxyz(0.092278,-0.023968,1.004535));
  hpcpush(hpxyz(0.085107,-0.045550,1.004648));
  hpcpush(hpxyz(0.070661,-0.058685,1.004210));
  hpcpush(hpxyz(0.045297,-0.040529,1.001846));
  hpcpush(hpxyz(0.057205,-0.014301,1.001737));
  hpcpush(hpxyz(0.057207,0.015494,1.001755));
  hpcpush(hpxyz(0.041704,0.039321,1.001641));
  hpcpush(hpxyz(0.023823,0.048837,1.001475));
  hpcpush(hpxyz(0.003575,0.058394,1.001710));
  hpcpush(hpxyz(-0.021441,0.050029,1.001480));
  hpcpush(hpxyz(-0.047692,0.040538,1.001957));
  hpcpush(hpxyz(-0.054813,0.017874,1.001661));
  hpcpush(hpxyz(-0.054802,-0.010722,1.001558));
  hpcpush(hpxyz(-0.036924,-0.039306,1.001453));
  hpcpush(hpxyz(-0.020254,-0.052421,1.001578));
  hpcpush(hpxyz(0.011915,-0.056002,1.001638));
  hpcpush(hpxyz(0.034561,-0.047670,1.001732));
  hpcpush(hpxyz(0.045297,-0.040529,1.001846));
  hpcpush(hpxyz(0.070661,-0.058685,1.004210));
  hpcpush(hpxyz(0.055051,-0.068215,1.003835));
  hpcpush(hpxyz(0.033505,-0.080172,1.003768));
  hpcpush(hpxyz(0.011971,-0.088587,1.003988));
  hpcpush(hpxyz(-0.010772,-0.087369,1.003867));
  hpcpush(hpxyz(-0.035904,-0.080185,1.003852));
  hpcpush(hpxyz(-0.061039,-0.063432,1.003867));
  hpcpush(hpxyz(-0.071750,-0.041854,1.003444));
  hpcpush(hpxyz(-0.082525,-0.015548,1.003520));
  hpcpush(hpxyz(-0.280212,-0.017913,1.038672));
  hpcpush(hpxyz(-0.280212,-0.017913,1.038672));

  // monsters
  
  bshape(shTentacleX, 31);
  drawTentacle(shTentacleX, crossf * .25, crossf * .1, 10);

  bshape(shBranch, 32);
  hpcpush(ddi(21, crossf/5) * C0);
  hpcpush(ddi(21, -crossf/5) * C0);
  hpcpush(ddi(21, -crossf/5) * ddi(0, crossf) * C0);
  hpcpush(ddi(21, crossf/5) * ddi(0, crossf) * C0);
  
  bshape(shIBranch, 32);
  drawTentacle(shIBranch, crossf * .1, crossf * .2, 5);

  bshape(shTentacle, 32);
  drawTentacle(shTentacle, crossf * .2, crossf * .1, 10);
  
  shJoint = shDisk; shJoint.prio = 33;

  bshape(shGhost, 33);
  hpcpush(hpxyz(-0.312167,0.001301,1.047592));
  hpcpush(hpxyz(-0.185499,0.015970,1.017185));
  hpcpush(hpxyz(-0.273136,0.038290,1.037338));
  hpcpush(hpxyz(-0.184484,0.043046,1.017786));
  hpcpush(hpxyz(-0.296331,0.093578,1.047172));
  hpcpush(hpxyz(-0.174561,0.070070,1.017537));
  hpcpush(hpxyz(-0.222751,0.104454,1.029820));
  hpcpush(hpxyz(-0.162371,0.098407,1.017864));
  hpcpush(hpxyz(-0.272077,0.156217,1.048060));
  hpcpush(hpxyz(-0.140182,0.126656,1.017690));
  hpcpush(hpxyz(-0.161729,0.148045,1.023755));
  hpcpush(hpxyz(-0.119271,0.146323,1.017662));
  hpcpush(hpxyz(-0.231927,0.213686,1.048548));
  hpcpush(hpxyz(-0.001244,0.218928,1.023685));
  hpcpush(hpxyz(0.016184,0.220354,1.024118));
  hpcpush(hpxyz(0.031094,0.216414,1.023622));
  hpcpush(hpxyz(0.045998,0.212585,1.023381));
  hpcpush(hpxyz(0.064748,0.211675,1.024206));
  hpcpush(hpxyz(0.079530,0.201310,1.023157));
  hpcpush(hpxyz(0.094597,0.199151,1.024016));
  hpcpush(hpxyz(0.110770,0.190424,1.023978));
  hpcpush(hpxyz(0.123174,0.181651,1.023801));
  hpcpush(hpxyz(0.132996,0.171528,1.023284));
  hpcpush(hpxyz(0.149284,0.160480,1.023738));
  hpcpush(hpxyz(0.156499,0.147805,1.022907));
  hpcpush(hpxyz(0.171645,0.135575,1.023642));
  hpcpush(hpxyz(0.180157,0.119276,1.023075));
  hpcpush(hpxyz(0.188931,0.106895,1.023290));
  hpcpush(hpxyz(0.197759,0.093283,1.023626));
  hpcpush(hpxyz(0.201351,0.080789,1.023264));
  hpcpush(hpxyz(0.209030,0.067188,1.023820));
  hpcpush(hpxyz(0.212718,0.052246,1.023708));
  hpcpush(hpxyz(0.215145,0.037308,1.023562));
  hpcpush(hpxyz(0.217714,0.026126,1.023759));
  hpcpush(hpxyz(0.217612,0.014922,1.023512));
  hpcpush(hpxyz(0.217612,-0.014922,1.023512));
  hpcpush(hpxyz(0.217714,-0.026126,1.023759));
  hpcpush(hpxyz(0.215145,-0.037308,1.023562));
  hpcpush(hpxyz(0.212718,-0.052246,1.023708));
  hpcpush(hpxyz(0.209030,-0.067188,1.023820));
  hpcpush(hpxyz(0.201351,-0.080789,1.023264));
  hpcpush(hpxyz(0.197759,-0.093283,1.023626));
  hpcpush(hpxyz(0.188931,-0.106895,1.023290));
  hpcpush(hpxyz(0.180157,-0.119276,1.023075));
  hpcpush(hpxyz(0.171645,-0.135575,1.023642));
  hpcpush(hpxyz(0.156499,-0.147805,1.022907));
  hpcpush(hpxyz(0.149284,-0.160480,1.023738));
  hpcpush(hpxyz(0.132996,-0.171528,1.023284));
  hpcpush(hpxyz(0.123174,-0.181651,1.023801));
  hpcpush(hpxyz(0.110770,-0.190424,1.023978));
  hpcpush(hpxyz(0.094597,-0.199151,1.024016));
  hpcpush(hpxyz(0.079530,-0.201310,1.023157));
  hpcpush(hpxyz(0.064748,-0.211675,1.024206));
  hpcpush(hpxyz(0.045998,-0.212585,1.023381));
  hpcpush(hpxyz(0.031094,-0.216414,1.023622));
  hpcpush(hpxyz(0.016184,-0.220354,1.024118));
  hpcpush(hpxyz(-0.001244,-0.218928,1.023685));
  hpcpush(hpxyz(-0.231927,-0.213686,1.048548));
  hpcpush(hpxyz(-0.119271,-0.146323,1.017662));
  hpcpush(hpxyz(-0.161729,-0.148045,1.023755));
  hpcpush(hpxyz(-0.140182,-0.126656,1.017690));
  hpcpush(hpxyz(-0.272077,-0.156217,1.048060));
  hpcpush(hpxyz(-0.162371,-0.098407,1.017864));
  hpcpush(hpxyz(-0.222751,-0.104454,1.029820));
  hpcpush(hpxyz(-0.174561,-0.070070,1.017537));
  hpcpush(hpxyz(-0.296331,-0.093578,1.047172));
  hpcpush(hpxyz(-0.184484,-0.043046,1.017786));
  hpcpush(hpxyz(-0.273136,-0.038290,1.037338));
  hpcpush(hpxyz(-0.185499,-0.015970,1.017185));
  hpcpush(hpxyz(-0.312167,-0.001301,1.047592));
  hpcpush(hpxyz(-0.312167,0.001301,1.047592));

  bshape(shILeaf[0], 33);
  for(int t=0; t<=6; t++) {
    hpcpush(ddi(7 + t*14, hexf*.7) * C0);
    hpcpush(ddi(14 + t*14, hexf*.15) * C0);
    }

  bshape(shILeaf[1], 33);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*36, hexf*.8) * C0);

  bshape(shWolf, 33);
  hpcpush(hpxyz(-0.310601,0.000000,1.047126));
  hpcpush(hpxyz(-0.158251,0.009739,1.012491));
  hpcpush(hpxyz(-0.149626,0.045009,1.012133));
  hpcpush(hpxyz(-0.173168,0.066320,1.017047));
  hpcpush(hpxyz(-0.250414,0.056912,1.032447));
  hpcpush(hpxyz(-0.242122,0.064314,1.030902));
  hpcpush(hpxyz(-0.253563,0.065926,1.033751));
  hpcpush(hpxyz(-0.241104,0.075740,1.031440));
  hpcpush(hpxyz(-0.249897,0.082453,1.034044));
  hpcpush(hpxyz(-0.237118,0.081982,1.030993));
  hpcpush(hpxyz(-0.246310,0.097762,1.034517));
  hpcpush(hpxyz(-0.230366,0.088118,1.029968));
  hpcpush(hpxyz(-0.210870,0.082352,1.025304));
  hpcpush(hpxyz(-0.194727,0.083100,1.022166));
  hpcpush(hpxyz(-0.180298,0.087679,1.019899));
  hpcpush(hpxyz(-0.162032,0.087154,1.016784));
  hpcpush(hpxyz(-0.135091,0.081542,1.012373));
  hpcpush(hpxyz(-0.116083,0.068924,1.009072));
  hpcpush(hpxyz(-0.106144,0.066340,1.007803));
  hpcpush(hpxyz(-0.084057,0.063643,1.005543));
  hpcpush(hpxyz(-0.061108,0.071892,1.004441));
  hpcpush(hpxyz(-0.044295,0.077815,1.004001));
  hpcpush(hpxyz(-0.021516,0.077698,1.003245));
  hpcpush(hpxyz(0.000000,0.078872,1.003106));
  hpcpush(hpxyz(0.025203,0.099613,1.005265));
  hpcpush(hpxyz(0.040964,0.113252,1.007226));
  hpcpush(hpxyz(0.067885,0.127285,1.010351));
  hpcpush(hpxyz(0.086481,0.135203,1.012798));
  hpcpush(hpxyz(0.104129,0.144556,1.015746));
  hpcpush(hpxyz(0.097579,0.132951,1.013508));
  hpcpush(hpxyz(0.112604,0.134635,1.015286));
  hpcpush(hpxyz(0.098603,0.124167,1.012492));
  hpcpush(hpxyz(0.118341,0.115901,1.013626));
  hpcpush(hpxyz(0.094688,0.115325,1.011072));
  hpcpush(hpxyz(0.079806,0.108826,1.009065));
  hpcpush(hpxyz(0.065011,0.097516,1.006845));
  hpcpush(hpxyz(0.053964,0.082746,1.004868));
  hpcpush(hpxyz(0.049028,0.066966,1.003438));
  hpcpush(hpxyz(0.045353,0.053708,1.002468));
  hpcpush(hpxyz(0.046494,0.040534,1.001901));
  hpcpush(hpxyz(0.051260,0.033378,1.001869));
  hpcpush(hpxyz(0.059646,0.029823,1.002221));
  hpcpush(hpxyz(0.069275,0.029860,1.002841));
  hpcpush(hpxyz(0.077732,0.029897,1.003462));
  hpcpush(hpxyz(0.081360,0.028715,1.003715));
  hpcpush(hpxyz(0.092268,0.021569,1.004479));
  hpcpush(hpxyz(0.102040,0.020408,1.005400));
  hpcpush(hpxyz(0.108180,0.020434,1.006042));
  hpcpush(hpxyz(0.123078,0.030166,1.007997));
  hpcpush(hpxyz(0.100952,0.042063,1.005963));
  hpcpush(hpxyz(0.125691,0.043508,1.008807));
  hpcpush(hpxyz(0.144550,0.044944,1.011393));
  hpcpush(hpxyz(0.154674,0.041409,1.012738));
  hpcpush(hpxyz(0.177755,0.029421,1.016102));
  hpcpush(hpxyz(0.154429,0.012160,1.011927));
  hpcpush(hpxyz(0.146841,0.019417,1.010910));
  hpcpush(hpxyz(0.132964,0.013296,1.008889));
  hpcpush(hpxyz(0.143007,0.009695,1.010220));
  hpcpush(hpxyz(0.154429,0.012160,1.011927));
  hpcpush(hpxyz(0.177755,0.029421,1.016102));
  hpcpush(hpxyz(0.184262,0.025797,1.017162));
  hpcpush(hpxyz(0.185576,0.025809,1.017401));
  hpcpush(hpxyz(0.190852,0.025857,1.018378));
  hpcpush(hpxyz(0.194828,0.025895,1.019131));
  hpcpush(hpxyz(0.201483,0.024722,1.020395));
  hpcpush(hpxyz(0.205462,0.019804,1.021081));
  hpcpush(hpxyz(0.210828,0.016122,1.022110));
  hpcpush(hpxyz(0.210793,0.009920,1.022024));
  hpcpush(hpxyz(0.217604,0.013678,1.023493));
  hpcpush(hpxyz(0.208083,0.007432,1.021447));
  hpcpush(hpxyz(0.208083,-0.007432,1.021447));
  hpcpush(hpxyz(0.217604,-0.013678,1.023493));
  hpcpush(hpxyz(0.210793,-0.009920,1.022024));
  hpcpush(hpxyz(0.210828,-0.016122,1.022110));
  hpcpush(hpxyz(0.205462,-0.019804,1.021081));
  hpcpush(hpxyz(0.201483,-0.024722,1.020395));
  hpcpush(hpxyz(0.194828,-0.025895,1.019131));
  hpcpush(hpxyz(0.190852,-0.025857,1.018378));
  hpcpush(hpxyz(0.185576,-0.025809,1.017401));
  hpcpush(hpxyz(0.184262,-0.025797,1.017162));
  hpcpush(hpxyz(0.177755,-0.029421,1.016102));
  hpcpush(hpxyz(0.154429,-0.012160,1.011927));
  hpcpush(hpxyz(0.143007,-0.009695,1.010220));
  hpcpush(hpxyz(0.132964,-0.013296,1.008889));
  hpcpush(hpxyz(0.146841,-0.019417,1.010910));
  hpcpush(hpxyz(0.154429,-0.012160,1.011927));
  hpcpush(hpxyz(0.177755,-0.029421,1.016102));
  hpcpush(hpxyz(0.154674,-0.041409,1.012738));
  hpcpush(hpxyz(0.144550,-0.044944,1.011393));
  hpcpush(hpxyz(0.125691,-0.043508,1.008807));
  hpcpush(hpxyz(0.100952,-0.042063,1.005963));
  hpcpush(hpxyz(0.123078,-0.030166,1.007997));
  hpcpush(hpxyz(0.108180,-0.020434,1.006042));
  hpcpush(hpxyz(0.102040,-0.020408,1.005400));
  hpcpush(hpxyz(0.092268,-0.021569,1.004479));
  hpcpush(hpxyz(0.081360,-0.028715,1.003715));
  hpcpush(hpxyz(0.077732,-0.029897,1.003462));
  hpcpush(hpxyz(0.069275,-0.029860,1.002841));
  hpcpush(hpxyz(0.059646,-0.029823,1.002221));
  hpcpush(hpxyz(0.051260,-0.033378,1.001869));
  hpcpush(hpxyz(0.046494,-0.040534,1.001901));
  hpcpush(hpxyz(0.045353,-0.053708,1.002468));
  hpcpush(hpxyz(0.049028,-0.066966,1.003438));
  hpcpush(hpxyz(0.053964,-0.082746,1.004868));
  hpcpush(hpxyz(0.065011,-0.097516,1.006845));
  hpcpush(hpxyz(0.079806,-0.108826,1.009065));
  hpcpush(hpxyz(0.094688,-0.115325,1.011072));
  hpcpush(hpxyz(0.118341,-0.115901,1.013626));
  hpcpush(hpxyz(0.098603,-0.124167,1.012492));
  hpcpush(hpxyz(0.112604,-0.134635,1.015286));
  hpcpush(hpxyz(0.097579,-0.132951,1.013508));
  hpcpush(hpxyz(0.104129,-0.144556,1.015746));
  hpcpush(hpxyz(0.086481,-0.135203,1.012798));
  hpcpush(hpxyz(0.067885,-0.127285,1.010351));
  hpcpush(hpxyz(0.040964,-0.113252,1.007226));
  hpcpush(hpxyz(0.025203,-0.099613,1.005265));
  hpcpush(hpxyz(0.000000,-0.078872,1.003106));
  hpcpush(hpxyz(-0.021516,-0.077698,1.003245));
  hpcpush(hpxyz(-0.044295,-0.077815,1.004001));
  hpcpush(hpxyz(-0.061108,-0.071892,1.004441));
  hpcpush(hpxyz(-0.084057,-0.063643,1.005543));
  hpcpush(hpxyz(-0.106144,-0.066340,1.007803));
  hpcpush(hpxyz(-0.116083,-0.068924,1.009072));
  hpcpush(hpxyz(-0.135091,-0.081542,1.012373));
  hpcpush(hpxyz(-0.162032,-0.087154,1.016784));
  hpcpush(hpxyz(-0.180298,-0.087679,1.019899));
  hpcpush(hpxyz(-0.194727,-0.083100,1.022166));
  hpcpush(hpxyz(-0.210870,-0.082352,1.025304));
  hpcpush(hpxyz(-0.230366,-0.088118,1.029968));
  hpcpush(hpxyz(-0.246310,-0.097762,1.034517));
  hpcpush(hpxyz(-0.237118,-0.081982,1.030993));
  hpcpush(hpxyz(-0.249897,-0.082453,1.034044));
  hpcpush(hpxyz(-0.241104,-0.075740,1.031440));
  hpcpush(hpxyz(-0.253563,-0.065926,1.033751));
  hpcpush(hpxyz(-0.242122,-0.064314,1.030902));
  hpcpush(hpxyz(-0.250414,-0.056912,1.032447));
  hpcpush(hpxyz(-0.173168,-0.066320,1.017047));
  hpcpush(hpxyz(-0.149626,-0.045009,1.012133));
  hpcpush(hpxyz(-0.158251,-0.009739,1.012491));
  hpcpush(hpxyz(-0.310601,-0.000000,1.047126));
  hpcpush(hpxyz(-0.310601,0.000000,1.047126));
  
  bshape(shEagle, 33);
  hpcpush(hpxyz(-0.153132,0.000000,1.011657));
  hpcpush(hpxyz(-0.151960,0.025529,1.011802));
  hpcpush(hpxyz(-0.192201,0.028337,1.018697));
  hpcpush(hpxyz(-0.204119,0.019793,1.020812));
  hpcpush(hpxyz(-0.201539,0.029674,1.020538));
  hpcpush(hpxyz(-0.208386,0.038452,1.022205));
  hpcpush(hpxyz(-0.200321,0.038333,1.020587));
  hpcpush(hpxyz(-0.199208,0.050730,1.020910));
  hpcpush(hpxyz(-0.192349,0.039456,1.019095));
  hpcpush(hpxyz(-0.149495,0.034031,1.011685));
  hpcpush(hpxyz(-0.148542,0.057225,1.012590));
  hpcpush(hpxyz(-0.089847,0.025157,1.004343));
  hpcpush(hpxyz(-0.048881,0.038151,1.001921));
  hpcpush(hpxyz(0.002416,0.130440,1.008474));
  hpcpush(hpxyz(-0.028450,0.202864,1.020766));
  hpcpush(hpxyz(-0.044701,0.209848,1.022758));
  hpcpush(hpxyz(-0.044823,0.216644,1.024180));
  hpcpush(hpxyz(-0.039848,0.217916,1.024244));
  hpcpush(hpxyz(-0.028602,0.216381,1.023542));
  hpcpush(hpxyz(-0.018604,0.210847,1.022156));
  hpcpush(hpxyz(-0.016122,0.210828,1.022110));
  hpcpush(hpxyz(-0.014922,0.217612,1.023512));
  hpcpush(hpxyz(-0.025023,0.231467,1.026744));
  hpcpush(hpxyz(-0.035205,0.241403,1.029327));
  hpcpush(hpxyz(-0.041644,0.248604,1.031280));
  hpcpush(hpxyz(-0.041754,0.254318,1.032677));
  hpcpush(hpxyz(-0.034211,0.258480,1.033432));
  hpcpush(hpxyz(-0.021481,0.253982,1.031973));
  hpcpush(hpxyz(-0.011317,0.243946,1.029387));
  hpcpush(hpxyz(0.001251,0.232708,1.026720));
  hpcpush(hpxyz(0.008728,0.224428,1.024912));
  hpcpush(hpxyz(0.009969,0.223059,1.024624));
  hpcpush(hpxyz(0.011289,0.238321,1.028068));
  hpcpush(hpxyz(0.003802,0.261058,1.033521));
  hpcpush(hpxyz(-0.005116,0.280126,1.038507));
  hpcpush(hpxyz(-0.007740,0.296717,1.043121));
  hpcpush(hpxyz(0.001286,0.290619,1.041375));
  hpcpush(hpxyz(0.012793,0.280166,1.038584));
  hpcpush(hpxyz(0.022854,0.264095,1.034538));
  hpcpush(hpxyz(0.028906,0.241303,1.029108));
  hpcpush(hpxyz(0.031328,0.234335,1.027567));
  hpcpush(hpxyz(0.032584,0.234354,1.027611));
  hpcpush(hpxyz(0.034119,0.252735,1.032007));
  hpcpush(hpxyz(0.032108,0.286405,1.040701));
  hpcpush(hpxyz(0.030917,0.292424,1.042338));
  hpcpush(hpxyz(0.039807,0.285067,1.040600));
  hpcpush(hpxyz(0.050896,0.264659,1.035681));
  hpcpush(hpxyz(0.058073,0.246178,1.031492));
  hpcpush(hpxyz(0.063739,0.221211,1.026156));
  hpcpush(hpxyz(0.083520,0.165812,1.017089));
  hpcpush(hpxyz(0.083035,0.146532,1.014084));
  hpcpush(hpxyz(0.075285,0.129928,1.011212));
  hpcpush(hpxyz(0.066581,0.122266,1.009645));
  hpcpush(hpxyz(0.055484,0.112174,1.007800));
  hpcpush(hpxyz(0.044365,0.087531,1.004803));
  hpcpush(hpxyz(0.034631,0.065680,1.002753));
  hpcpush(hpxyz(0.034631,0.065680,1.002753));
  hpcpush(hpxyz(0.026221,0.053634,1.001781));
  hpcpush(hpxyz(0.022616,0.041661,1.001123));
  hpcpush(hpxyz(0.021411,0.033306,1.000784));
  hpcpush(hpxyz(0.022591,0.024969,1.000567));
  hpcpush(hpxyz(0.026158,0.021402,1.000571));
  hpcpush(hpxyz(0.028538,0.020215,1.000611));
  hpcpush(hpxyz(0.034494,0.017842,1.000754));
  hpcpush(hpxyz(0.040463,0.020231,1.001023));
  hpcpush(hpxyz(0.042853,0.021427,1.001147));
  hpcpush(hpxyz(0.044051,0.022621,1.001225));
  hpcpush(hpxyz(0.051232,0.023829,1.001595));
  hpcpush(hpxyz(0.058426,0.023847,1.001989));
  hpcpush(hpxyz(0.064425,0.020282,1.002278));
  hpcpush(hpxyz(0.068025,0.015514,1.002431));
  hpcpush(hpxyz(0.071634,0.011939,1.002634));
  hpcpush(hpxyz(0.089791,0.002394,1.004026));
  hpcpush(hpxyz(0.089791,-0.002394,1.004026));
  hpcpush(hpxyz(0.071634,-0.011939,1.002634));
  hpcpush(hpxyz(0.068025,-0.015514,1.002431));
  hpcpush(hpxyz(0.064425,-0.020282,1.002278));
  hpcpush(hpxyz(0.058426,-0.023847,1.001989));
  hpcpush(hpxyz(0.051232,-0.023829,1.001595));
  hpcpush(hpxyz(0.044051,-0.022621,1.001225));
  hpcpush(hpxyz(0.042853,-0.021427,1.001147));
  hpcpush(hpxyz(0.040463,-0.020231,1.001023));
  hpcpush(hpxyz(0.034494,-0.017842,1.000754));
  hpcpush(hpxyz(0.028538,-0.020215,1.000611));
  hpcpush(hpxyz(0.026158,-0.021402,1.000571));
  hpcpush(hpxyz(0.022591,-0.024969,1.000567));
  hpcpush(hpxyz(0.021411,-0.033306,1.000784));
  hpcpush(hpxyz(0.022616,-0.041661,1.001123));
  hpcpush(hpxyz(0.026221,-0.053634,1.001781));
  hpcpush(hpxyz(0.034631,-0.065680,1.002753));
  hpcpush(hpxyz(0.034631,-0.065680,1.002753));
  hpcpush(hpxyz(0.044365,-0.087531,1.004803));
  hpcpush(hpxyz(0.055484,-0.112174,1.007800));
  hpcpush(hpxyz(0.066581,-0.122266,1.009645));
  hpcpush(hpxyz(0.075285,-0.129928,1.011212));
  hpcpush(hpxyz(0.083035,-0.146532,1.014084));
  hpcpush(hpxyz(0.083520,-0.165812,1.017089));
  hpcpush(hpxyz(0.063739,-0.221211,1.026156));
  hpcpush(hpxyz(0.058073,-0.246178,1.031492));
  hpcpush(hpxyz(0.050896,-0.264659,1.035681));
  hpcpush(hpxyz(0.039807,-0.285067,1.040600));
  hpcpush(hpxyz(0.030917,-0.292424,1.042338));
  hpcpush(hpxyz(0.032108,-0.286405,1.040701));
  hpcpush(hpxyz(0.034119,-0.252735,1.032007));
  hpcpush(hpxyz(0.032584,-0.234354,1.027611));
  hpcpush(hpxyz(0.031328,-0.234335,1.027567));
  hpcpush(hpxyz(0.028906,-0.241303,1.029108));
  hpcpush(hpxyz(0.022854,-0.264095,1.034538));
  hpcpush(hpxyz(0.012793,-0.280166,1.038584));
  hpcpush(hpxyz(0.001286,-0.290619,1.041375));
  hpcpush(hpxyz(-0.007740,-0.296717,1.043121));
  hpcpush(hpxyz(-0.005116,-0.280126,1.038507));
  hpcpush(hpxyz(0.003802,-0.261058,1.033521));
  hpcpush(hpxyz(0.011289,-0.238321,1.028068));
  hpcpush(hpxyz(0.009969,-0.223059,1.024624));
  hpcpush(hpxyz(0.008728,-0.224428,1.024912));
  hpcpush(hpxyz(0.001251,-0.232708,1.026720));
  hpcpush(hpxyz(-0.011317,-0.243946,1.029387));
  hpcpush(hpxyz(-0.021481,-0.253982,1.031973));
  hpcpush(hpxyz(-0.034211,-0.258480,1.033432));
  hpcpush(hpxyz(-0.041754,-0.254318,1.032677));
  hpcpush(hpxyz(-0.041644,-0.248604,1.031280));
  hpcpush(hpxyz(-0.035205,-0.241403,1.029327));
  hpcpush(hpxyz(-0.025023,-0.231467,1.026744));
  hpcpush(hpxyz(-0.014922,-0.217612,1.023512));
  hpcpush(hpxyz(-0.016122,-0.210828,1.022110));
  hpcpush(hpxyz(-0.018604,-0.210847,1.022156));
  hpcpush(hpxyz(-0.028602,-0.216381,1.023542));
  hpcpush(hpxyz(-0.039848,-0.217916,1.024244));
  hpcpush(hpxyz(-0.044823,-0.216644,1.024180));
  hpcpush(hpxyz(-0.044701,-0.209848,1.022758));
  hpcpush(hpxyz(-0.028450,-0.202864,1.020766));
  hpcpush(hpxyz(0.002416,-0.130440,1.008474));
  hpcpush(hpxyz(-0.048881,-0.038151,1.001921));
  hpcpush(hpxyz(-0.089847,-0.025157,1.004343));
  hpcpush(hpxyz(-0.148542,-0.057225,1.012590));
  hpcpush(hpxyz(-0.149495,-0.034031,1.011685));
  hpcpush(hpxyz(-0.192349,-0.039456,1.019095));
  hpcpush(hpxyz(-0.199208,-0.050730,1.020910));
  hpcpush(hpxyz(-0.200321,-0.038333,1.020587));
  hpcpush(hpxyz(-0.208386,-0.038452,1.022205));
  hpcpush(hpxyz(-0.201539,-0.029674,1.020538));
  hpcpush(hpxyz(-0.204119,-0.019793,1.020812));
  hpcpush(hpxyz(-0.192201,-0.028337,1.018697));
  hpcpush(hpxyz(-0.151960,-0.025529,1.011802));
  hpcpush(hpxyz(-0.153132,-0.000000,1.011657));
  hpcpush(hpxyz(-0.153132,0.000000,1.011657));

  bshape(shSlime, 33);
  for(int i=0; i<=84; i++)
    hpcpush(ddi(i, crossf * (0.7 + .2 * sin(i * M_PI * 2 / 84 * 9))) * C0);

  bshape(shTentHead, 33);
  hpcpush(hpxyz(-0.153133,-0.001215,1.011658));
  hpcpush(hpxyz(-0.153144,0.008508,1.011694));
  hpcpush(hpxyz(-0.148132,0.023070,1.011175));
  hpcpush(hpxyz(-0.142123,0.052233,1.011399));
  hpcpush(hpxyz(-0.146204,0.068228,1.012932));
  hpcpush(hpxyz(-0.177590,0.085095,1.019205));
  hpcpush(hpxyz(-0.224851,0.089187,1.028840));
  hpcpush(hpxyz(-0.262047,0.120747,1.040792));
  hpcpush(hpxyz(-0.255264,0.167153,1.045514));
  hpcpush(hpxyz(-0.229967,0.174723,1.040871));
  hpcpush(hpxyz(-0.184192,0.147855,1.027515));
  hpcpush(hpxyz(-0.138732,0.121544,1.016867));
  hpcpush(hpxyz(-0.108401,0.118145,1.012773));
  hpcpush(hpxyz(-0.102300,0.123003,1.012717));
  hpcpush(hpxyz(-0.091928,0.154439,1.016023));
  hpcpush(hpxyz(-0.090798,0.199010,1.023645));
  hpcpush(hpxyz(-0.056384,0.229293,1.027499));
  hpcpush(hpxyz(-0.011331,0.246774,1.030061));
  hpcpush(hpxyz(0.024027,0.255446,1.032390));
  hpcpush(hpxyz(0.073005,0.235380,1.029919));
  hpcpush(hpxyz(0.151935,0.187094,1.028634));
  hpcpush(hpxyz(0.146867,0.164292,1.023993));
  hpcpush(hpxyz(0.094648,0.162253,1.017489));
  hpcpush(hpxyz(0.044142,0.175341,1.016215));
  hpcpush(hpxyz(-0.003641,0.148054,1.010907));
  hpcpush(hpxyz(-0.002399,0.100774,1.005068));
  hpcpush(hpxyz(0.047859,0.071788,1.003715));
  hpcpush(hpxyz(0.098991,0.082090,1.008235));
  hpcpush(hpxyz(0.151038,0.106832,1.016969));
  hpcpush(hpxyz(0.199477,0.131730,1.028175));
  hpcpush(hpxyz(0.255339,0.129594,1.040189));
  hpcpush(hpxyz(0.264313,0.081720,1.037564));
  hpcpush(hpxyz(0.226731,0.063886,1.027370));
  hpcpush(hpxyz(0.168813,0.041592,1.015001));
  hpcpush(hpxyz(0.128034,0.026573,1.008513));
  hpcpush(hpxyz(0.108135,0.002403,1.005832));
  hpcpush(hpxyz(0.108135,-0.002403,1.005832));
  hpcpush(hpxyz(0.128034,-0.026573,1.008513));
  hpcpush(hpxyz(0.168813,-0.041592,1.015001));
  hpcpush(hpxyz(0.226731,-0.063886,1.027370));
  hpcpush(hpxyz(0.264313,-0.081720,1.037564));
  hpcpush(hpxyz(0.255339,-0.129594,1.040189));
  hpcpush(hpxyz(0.199477,-0.131730,1.028175));
  hpcpush(hpxyz(0.151038,-0.106832,1.016969));
  hpcpush(hpxyz(0.098991,-0.082090,1.008235));
  hpcpush(hpxyz(0.047859,-0.071788,1.003715));
  hpcpush(hpxyz(-0.002399,-0.100774,1.005068));
  hpcpush(hpxyz(-0.003641,-0.148054,1.010907));
  hpcpush(hpxyz(0.044142,-0.175341,1.016215));
  hpcpush(hpxyz(0.094648,-0.162253,1.017489));
  hpcpush(hpxyz(0.146867,-0.164292,1.023993));
  hpcpush(hpxyz(0.151935,-0.187094,1.028634));
  hpcpush(hpxyz(0.073005,-0.235380,1.029919));
  hpcpush(hpxyz(0.024027,-0.255446,1.032390));
  hpcpush(hpxyz(-0.011331,-0.246774,1.030061));
  hpcpush(hpxyz(-0.056384,-0.229293,1.027499));
  hpcpush(hpxyz(-0.090798,-0.199010,1.023645));
  hpcpush(hpxyz(-0.091928,-0.154439,1.016023));
  hpcpush(hpxyz(-0.102300,-0.123003,1.012717));
  hpcpush(hpxyz(-0.108401,-0.118145,1.012773));
  hpcpush(hpxyz(-0.138732,-0.121544,1.016867));
  hpcpush(hpxyz(-0.184192,-0.147855,1.027515));
  hpcpush(hpxyz(-0.229967,-0.174723,1.040871));
  hpcpush(hpxyz(-0.255264,-0.167153,1.045514));
  hpcpush(hpxyz(-0.262047,-0.120747,1.040792));
  hpcpush(hpxyz(-0.224851,-0.089187,1.028840));
  hpcpush(hpxyz(-0.177590,-0.085095,1.019205));
  hpcpush(hpxyz(-0.146204,-0.068228,1.012932));
  hpcpush(hpxyz(-0.142123,-0.052233,1.011399));
  hpcpush(hpxyz(-0.148132,-0.023070,1.011175));
  hpcpush(hpxyz(-0.153144,-0.008508,1.011694));
  hpcpush(hpxyz(-0.153133,0.001215,1.011658));
  hpcpush(hpxyz(-0.153133,-0.001215,1.011658));
  
  bshape(shWormHead, 33);
  hpcpush(hpxyz(-0.212131,-0.004962,1.022264));
  hpcpush(hpxyz(-0.213579,0.021110,1.022771));
  hpcpush(hpxyz(-0.208766,0.057162,1.023157));
  hpcpush(hpxyz(-0.193784,0.094408,1.022969));
  hpcpush(hpxyz(-0.171245,0.126572,1.022421));
  hpcpush(hpxyz(-0.149103,0.156559,1.023104));
  hpcpush(hpxyz(-0.121949,0.182923,1.023881));
  hpcpush(hpxyz(-0.099674,0.199349,1.024536));
  hpcpush(hpxyz(-0.073594,0.213299,1.025140));
  hpcpush(hpxyz(-0.039892,0.220654,1.024831));
  hpcpush(hpxyz(0.000000,0.225787,1.025173));
  hpcpush(hpxyz(0.051224,0.223636,1.025981));
  hpcpush(hpxyz(0.099986,0.207472,1.026178));
  hpcpush(hpxyz(0.126101,0.189776,1.025630));
  hpcpush(hpxyz(0.144643,0.172075,1.024954));
  hpcpush(hpxyz(0.260938,0.181352,1.049275));
  hpcpush(hpxyz(0.170506,0.139392,1.023964));
  hpcpush(hpxyz(0.241779,0.141997,1.038567));
  hpcpush(hpxyz(0.188983,0.108168,1.023433));
  hpcpush(hpxyz(0.239385,0.101327,1.033234));
  hpcpush(hpxyz(0.202657,0.079571,1.023426));
  hpcpush(hpxyz(0.238058,0.069276,1.030277));
  hpcpush(hpxyz(0.208736,0.055911,1.023082));
  hpcpush(hpxyz(0.237283,0.040175,1.028551));
  hpcpush(hpxyz(0.212303,0.028555,1.022687));
  hpcpush(hpxyz(0.235548,0.015035,1.027477));
  hpcpush(hpxyz(0.216200,0.001243,1.023105));
  hpcpush(hpxyz(0.216200,-0.001243,1.023105));
  hpcpush(hpxyz(0.235548,-0.015035,1.027477));
  hpcpush(hpxyz(0.212303,-0.028555,1.022687));
  hpcpush(hpxyz(0.237283,-0.040175,1.028551));
  hpcpush(hpxyz(0.208736,-0.055911,1.023082));
  hpcpush(hpxyz(0.238058,-0.069276,1.030277));
  hpcpush(hpxyz(0.202657,-0.079571,1.023426));
  hpcpush(hpxyz(0.239385,-0.101327,1.033234));
  hpcpush(hpxyz(0.188983,-0.108168,1.023433));
  hpcpush(hpxyz(0.241779,-0.141997,1.038567));
  hpcpush(hpxyz(0.170506,-0.139392,1.023964));
  hpcpush(hpxyz(0.260938,-0.181352,1.049275));
  hpcpush(hpxyz(0.144643,-0.172075,1.024954));
  hpcpush(hpxyz(0.126101,-0.189776,1.025630));
  hpcpush(hpxyz(0.099986,-0.207472,1.026178));
  hpcpush(hpxyz(0.051224,-0.223636,1.025981));
  hpcpush(hpxyz(0.000000,-0.225787,1.025173));
  hpcpush(hpxyz(-0.039892,-0.220654,1.024831));
  hpcpush(hpxyz(-0.073594,-0.213299,1.025140));
  hpcpush(hpxyz(-0.099674,-0.199349,1.024536));
  hpcpush(hpxyz(-0.121949,-0.182923,1.023881));
  hpcpush(hpxyz(-0.149103,-0.156559,1.023104));
  hpcpush(hpxyz(-0.171245,-0.126572,1.022421));
  hpcpush(hpxyz(-0.193784,-0.094408,1.022969));
  hpcpush(hpxyz(-0.208766,-0.057162,1.023157));
  hpcpush(hpxyz(-0.213579,-0.021110,1.022771));
  hpcpush(hpxyz(-0.212131,0.004962,1.022264));
  hpcpush(hpxyz(-0.212131,-0.004962,1.022264));
  
  // bodyparts

  bshape(shPBody, 41);
  hpcpush(hpxyz(-0.127943,0.000000,1.008151));
  hpcpush(hpxyz(-0.121732,0.008437,1.007417));
  hpcpush(hpxyz(-0.120752,0.047093,1.008364));
  hpcpush(hpxyz(-0.114785,0.065246,1.008679));
  hpcpush(hpxyz(-0.096531,0.082051,1.007993));
  hpcpush(hpxyz(-0.079664,0.100183,1.008158));
  hpcpush(hpxyz(-0.087015,0.156872,1.015963));
  hpcpush(hpxyz(-0.090442,0.188317,1.021588));
  hpcpush(hpxyz(-0.085023,0.215058,1.026391));
  hpcpush(hpxyz(-0.078296,0.241201,1.031653));
  hpcpush(hpxyz(-0.070101,0.263835,1.036592));
  hpcpush(hpxyz(-0.062700,0.273833,1.038709));
  hpcpush(hpxyz(-0.053763,0.276497,1.038913));
  hpcpush(hpxyz(-0.030638,0.274461,1.037433));
  hpcpush(hpxyz(-0.015319,0.275737,1.037432));
  hpcpush(hpxyz(0.001277,0.277150,1.037696));
  hpcpush(hpxyz(0.020384,0.271369,1.036367));
  hpcpush(hpxyz(0.038101,0.262896,1.034681));
  hpcpush(hpxyz(0.045596,0.255842,1.033215));
  hpcpush(hpxyz(0.062388,0.263558,1.036029));
  hpcpush(hpxyz(0.085371,0.258660,1.036433));
  hpcpush(hpxyz(0.084235,0.228817,1.029297));
  hpcpush(hpxyz(0.071073,0.213220,1.024946));
  hpcpush(hpxyz(0.048603,0.218088,1.024658));
  hpcpush(hpxyz(0.042541,0.228972,1.026761));
  hpcpush(hpxyz(0.028749,0.228742,1.026231));
  hpcpush(hpxyz(0.011222,0.224439,1.024938));
  hpcpush(hpxyz(-0.012498,0.229969,1.026178));
  hpcpush(hpxyz(-0.026261,0.230095,1.026466));
  hpcpush(hpxyz(-0.024880,0.217700,1.023725));
  hpcpush(hpxyz(-0.022225,0.198787,1.019809));
  hpcpush(hpxyz(-0.020850,0.180288,1.016336));
  hpcpush(hpxyz(-0.021870,0.150662,1.011522));
  hpcpush(hpxyz(-0.022997,0.136774,1.009572));
  hpcpush(hpxyz(-0.004819,0.120485,1.007244));
  hpcpush(hpxyz(0.007204,0.104455,1.005466));
  hpcpush(hpxyz(0.016748,0.083741,1.003640));
  hpcpush(hpxyz(0.026225,0.054833,1.001846));
  hpcpush(hpxyz(0.033323,0.030943,1.001033));
  hpcpush(hpxyz(0.034483,0.001189,1.000595));
  hpcpush(hpxyz(0.034483,-0.001189,1.000595));
  hpcpush(hpxyz(0.034483,0.001189,1.000595));
  hpcpush(hpxyz(0.034483,-0.001189,1.000595));
  hpcpush(hpxyz(0.033323,-0.030943,1.001033));
  hpcpush(hpxyz(0.026225,-0.054833,1.001846));
  hpcpush(hpxyz(0.016748,-0.083741,1.003640));
  hpcpush(hpxyz(0.007204,-0.104455,1.005466));
  hpcpush(hpxyz(-0.004819,-0.120485,1.007244));
  hpcpush(hpxyz(-0.022997,-0.136774,1.009572));
  hpcpush(hpxyz(-0.021870,-0.150662,1.011522));
  hpcpush(hpxyz(-0.020850,-0.180288,1.016336));
  hpcpush(hpxyz(-0.022225,-0.198787,1.019809));
  hpcpush(hpxyz(-0.024880,-0.217700,1.023725));
  hpcpush(hpxyz(-0.026261,-0.230095,1.026466));
  hpcpush(hpxyz(-0.012498,-0.229969,1.026178));
  hpcpush(hpxyz(0.011222,-0.224439,1.024938));
  hpcpush(hpxyz(0.028749,-0.228742,1.026231));
  hpcpush(hpxyz(0.042541,-0.228972,1.026761));
  hpcpush(hpxyz(0.048603,-0.218088,1.024658));
  hpcpush(hpxyz(0.071073,-0.213220,1.024946));
  hpcpush(hpxyz(0.084235,-0.228817,1.029297));
  hpcpush(hpxyz(0.085371,-0.258660,1.036433));
  hpcpush(hpxyz(0.062388,-0.263558,1.036029));
  hpcpush(hpxyz(0.045596,-0.255842,1.033215));
  hpcpush(hpxyz(0.038101,-0.262896,1.034681));
  hpcpush(hpxyz(0.020384,-0.271369,1.036367));
  hpcpush(hpxyz(0.001277,-0.277150,1.037696));
  hpcpush(hpxyz(-0.015319,-0.275737,1.037432));
  hpcpush(hpxyz(-0.030638,-0.274461,1.037433));
  hpcpush(hpxyz(-0.053763,-0.276497,1.038913));
  hpcpush(hpxyz(-0.062700,-0.273833,1.038709));
  hpcpush(hpxyz(-0.070101,-0.263835,1.036592));
  hpcpush(hpxyz(-0.078296,-0.241201,1.031653));
  hpcpush(hpxyz(-0.085023,-0.215058,1.026391));
  hpcpush(hpxyz(-0.090442,-0.188317,1.021588));
  hpcpush(hpxyz(-0.087015,-0.156872,1.015963));
  hpcpush(hpxyz(-0.079664,-0.100183,1.008158));
  hpcpush(hpxyz(-0.096531,-0.082051,1.007993));
  hpcpush(hpxyz(-0.114785,-0.065246,1.008679));
  hpcpush(hpxyz(-0.120752,-0.047093,1.008364));
  hpcpush(hpxyz(-0.121732,-0.008437,1.007417));
  hpcpush(hpxyz(-0.127943,-0.000000,1.008151));
  hpcpush(hpxyz(-0.127943,0.000000,1.008151));

  bshape(shYeti, 41);
  hpcpush(hpxyz(-0.146785,0.001213,1.010716));
  hpcpush(hpxyz(-0.119261,0.012047,1.007158));
  hpcpush(hpxyz(-0.134264,0.022982,1.009235));
  hpcpush(hpxyz(-0.116850,0.026502,1.007153));
  hpcpush(hpxyz(-0.128124,0.037470,1.008871));
  hpcpush(hpxyz(-0.114489,0.040975,1.007366));
  hpcpush(hpxyz(-0.134526,0.049690,1.010231));
  hpcpush(hpxyz(-0.108483,0.056652,1.007461));
  hpcpush(hpxyz(-0.124869,0.072739,1.010388));
  hpcpush(hpxyz(-0.100038,0.069906,1.007420));
  hpcpush(hpxyz(-0.116319,0.082393,1.010108));
  hpcpush(hpxyz(-0.086776,0.085571,1.007399));
  hpcpush(hpxyz(-0.105410,0.095717,1.010086));
  hpcpush(hpxyz(-0.078461,0.101396,1.008185));
  hpcpush(hpxyz(-0.084756,0.111394,1.009749));
  hpcpush(hpxyz(-0.078850,0.123734,1.010706));
  hpcpush(hpxyz(-0.094972,0.127846,1.012603));
  hpcpush(hpxyz(-0.081624,0.138883,1.012892));
  hpcpush(hpxyz(-0.102733,0.139423,1.014886));
  hpcpush(hpxyz(-0.083221,0.154204,1.015236));
  hpcpush(hpxyz(-0.115566,0.148760,1.017588));
  hpcpush(hpxyz(-0.081064,0.167041,1.017091));
  hpcpush(hpxyz(-0.093630,0.170012,1.018661));
  hpcpush(hpxyz(-0.081530,0.184060,1.020061));
  hpcpush(hpxyz(-0.104404,0.190164,1.023261));
  hpcpush(hpxyz(-0.078114,0.195904,1.021998));
  hpcpush(hpxyz(-0.088688,0.211101,1.025880));
  hpcpush(hpxyz(-0.074815,0.211977,1.024954));
  hpcpush(hpxyz(-0.081589,0.225940,1.028448));
  hpcpush(hpxyz(-0.067678,0.226847,1.027638));
  hpcpush(hpxyz(-0.091419,0.248863,1.034548));
  hpcpush(hpxyz(-0.059306,0.244796,1.031234));
  hpcpush(hpxyz(-0.060896,0.256271,1.034110));
  hpcpush(hpxyz(-0.051997,0.257446,1.033916));
  hpcpush(hpxyz(-0.058919,0.276663,1.039237));
  hpcpush(hpxyz(-0.042072,0.270280,1.036736));
  hpcpush(hpxyz(-0.042449,0.288141,1.041550));
  hpcpush(hpxyz(-0.020355,0.268437,1.035603));
  hpcpush(hpxyz(-0.007716,0.290637,1.041407));
  hpcpush(hpxyz(0.001272,0.268323,1.035374));
  hpcpush(hpxyz(0.017995,0.289206,1.041136));
  hpcpush(hpxyz(0.021598,0.265534,1.034879));
  hpcpush(hpxyz(0.038374,0.277573,1.038518));
  hpcpush(hpxyz(0.037997,0.257112,1.033223));
  hpcpush(hpxyz(0.048725,0.280810,1.039821));
  hpcpush(hpxyz(0.050653,0.254534,1.033128));
  hpcpush(hpxyz(0.069009,0.269645,1.038013));
  hpcpush(hpxyz(0.085977,0.273328,1.040241));
  hpcpush(hpxyz(0.075050,0.258222,1.035525));
  hpcpush(hpxyz(0.097049,0.259223,1.037601));
  hpcpush(hpxyz(0.103016,0.248003,1.035431));
  hpcpush(hpxyz(0.079575,0.241251,1.031763));
  hpcpush(hpxyz(0.091636,0.222186,1.028476));
  hpcpush(hpxyz(0.070376,0.232494,1.029080));
  hpcpush(hpxyz(0.072102,0.205119,1.023363));
  hpcpush(hpxyz(0.060388,0.237778,1.029653));
  hpcpush(hpxyz(0.058115,0.195365,1.020561));
  hpcpush(hpxyz(0.045217,0.237388,1.028784));
  hpcpush(hpxyz(0.037611,0.234439,1.027802));
  hpcpush(hpxyz(0.034796,0.213746,1.023180));
  hpcpush(hpxyz(0.023715,0.225917,1.025476));
  hpcpush(hpxyz(0.027226,0.204191,1.020997));
  hpcpush(hpxyz(0.012420,0.214873,1.022900));
  hpcpush(hpxyz(0.020917,0.189485,1.018009));
  hpcpush(hpxyz(0.003700,0.197357,1.019296));
  hpcpush(hpxyz(0.009817,0.182844,1.016626));
  hpcpush(hpxyz(-0.011065,0.188106,1.017598));
  hpcpush(hpxyz(0.003663,0.168520,1.014107));
  hpcpush(hpxyz(-0.023305,0.180308,1.016393));
  hpcpush(hpxyz(-0.004879,0.164654,1.013477));
  hpcpush(hpxyz(-0.034247,0.170012,1.014927));
  hpcpush(hpxyz(-0.004861,0.153136,1.011669));
  hpcpush(hpxyz(-0.027964,0.151980,1.011870));
  hpcpush(hpxyz(-0.003639,0.146787,1.010722));
  hpcpush(hpxyz(-0.015710,0.131722,1.008760));
  hpcpush(hpxyz(0.010889,0.136717,1.009361));
  hpcpush(hpxyz(-0.003615,0.120484,1.007239));
  hpcpush(hpxyz(0.027708,0.116858,1.007186));
  hpcpush(hpxyz(0.007202,0.103229,1.005340));
  hpcpush(hpxyz(0.034761,0.089899,1.004634));
  hpcpush(hpxyz(0.015548,0.082525,1.003520));
  hpcpush(hpxyz(0.044270,0.074183,1.003725));
  hpcpush(hpxyz(0.021490,0.069245,1.002625));
  hpcpush(hpxyz(0.051357,0.054940,1.002824));
  hpcpush(hpxyz(0.027405,0.050043,1.001626));
  hpcpush(hpxyz(0.053662,0.034582,1.002036));
  hpcpush(hpxyz(0.034521,0.033331,1.001151));
  hpcpush(hpxyz(0.060848,0.029827,1.002293));
  hpcpush(hpxyz(0.041653,0.017851,1.001026));
  hpcpush(hpxyz(0.065617,0.015510,1.002271));
  hpcpush(hpxyz(0.051204,0.004763,1.001321));
  hpcpush(hpxyz(0.051204,-0.004763,1.001321));
  hpcpush(hpxyz(0.065617,-0.015510,1.002271));
  hpcpush(hpxyz(0.041653,-0.017851,1.001026));
  hpcpush(hpxyz(0.060848,-0.029827,1.002293));
  hpcpush(hpxyz(0.034521,-0.033331,1.001151));
  hpcpush(hpxyz(0.053662,-0.034582,1.002036));
  hpcpush(hpxyz(0.027405,-0.050043,1.001626));
  hpcpush(hpxyz(0.051357,-0.054940,1.002824));
  hpcpush(hpxyz(0.021490,-0.069245,1.002625));
  hpcpush(hpxyz(0.044270,-0.074183,1.003725));
  hpcpush(hpxyz(0.015548,-0.082525,1.003520));
  hpcpush(hpxyz(0.034761,-0.089899,1.004634));
  hpcpush(hpxyz(0.007202,-0.103229,1.005340));
  hpcpush(hpxyz(0.027708,-0.116858,1.007186));
  hpcpush(hpxyz(-0.003615,-0.120484,1.007239));
  hpcpush(hpxyz(0.010889,-0.136717,1.009361));
  hpcpush(hpxyz(-0.015710,-0.131722,1.008760));
  hpcpush(hpxyz(-0.003639,-0.146787,1.010722));
  hpcpush(hpxyz(-0.027964,-0.151980,1.011870));
  hpcpush(hpxyz(-0.004861,-0.153136,1.011669));
  hpcpush(hpxyz(-0.034247,-0.170012,1.014927));
  hpcpush(hpxyz(-0.004879,-0.164654,1.013477));
  hpcpush(hpxyz(-0.023305,-0.180308,1.016393));
  hpcpush(hpxyz(0.003663,-0.168520,1.014107));
  hpcpush(hpxyz(-0.011065,-0.188106,1.017598));
  hpcpush(hpxyz(0.009817,-0.182844,1.016626));
  hpcpush(hpxyz(0.003700,-0.197357,1.019296));
  hpcpush(hpxyz(0.020917,-0.189485,1.018009));
  hpcpush(hpxyz(0.012420,-0.214873,1.022900));
  hpcpush(hpxyz(0.027226,-0.204191,1.020997));
  hpcpush(hpxyz(0.023715,-0.225917,1.025476));
  hpcpush(hpxyz(0.034796,-0.213746,1.023180));
  hpcpush(hpxyz(0.037611,-0.234439,1.027802));
  hpcpush(hpxyz(0.045217,-0.237388,1.028784));
  hpcpush(hpxyz(0.058115,-0.195365,1.020561));
  hpcpush(hpxyz(0.060388,-0.237778,1.029653));
  hpcpush(hpxyz(0.072102,-0.205119,1.023363));
  hpcpush(hpxyz(0.070376,-0.232494,1.029080));
  hpcpush(hpxyz(0.091636,-0.222186,1.028476));
  hpcpush(hpxyz(0.079575,-0.241251,1.031763));
  hpcpush(hpxyz(0.103016,-0.248003,1.035431));
  hpcpush(hpxyz(0.097049,-0.259223,1.037601));
  hpcpush(hpxyz(0.075050,-0.258222,1.035525));
  hpcpush(hpxyz(0.085977,-0.273328,1.040241));
  hpcpush(hpxyz(0.069009,-0.269645,1.038013));
  hpcpush(hpxyz(0.050653,-0.254534,1.033128));
  hpcpush(hpxyz(0.048725,-0.280810,1.039821));
  hpcpush(hpxyz(0.037997,-0.257112,1.033223));
  hpcpush(hpxyz(0.038374,-0.277573,1.038518));
  hpcpush(hpxyz(0.021598,-0.265534,1.034879));
  hpcpush(hpxyz(0.017995,-0.289206,1.041136));
  hpcpush(hpxyz(0.001272,-0.268323,1.035374));
  hpcpush(hpxyz(-0.007716,-0.290637,1.041407));
  hpcpush(hpxyz(-0.020355,-0.268437,1.035603));
  hpcpush(hpxyz(-0.042449,-0.288141,1.041550));
  hpcpush(hpxyz(-0.042072,-0.270280,1.036736));
  hpcpush(hpxyz(-0.058919,-0.276663,1.039237));
  hpcpush(hpxyz(-0.051997,-0.257446,1.033916));
  hpcpush(hpxyz(-0.060896,-0.256271,1.034110));
  hpcpush(hpxyz(-0.059306,-0.244796,1.031234));
  hpcpush(hpxyz(-0.091419,-0.248863,1.034548));
  hpcpush(hpxyz(-0.067678,-0.226847,1.027638));
  hpcpush(hpxyz(-0.081589,-0.225940,1.028448));
  hpcpush(hpxyz(-0.074815,-0.211977,1.024954));
  hpcpush(hpxyz(-0.088688,-0.211101,1.025880));
  hpcpush(hpxyz(-0.078114,-0.195904,1.021998));
  hpcpush(hpxyz(-0.104404,-0.190164,1.023261));
  hpcpush(hpxyz(-0.081530,-0.184060,1.020061));
  hpcpush(hpxyz(-0.093630,-0.170012,1.018661));
  hpcpush(hpxyz(-0.081064,-0.167041,1.017091));
  hpcpush(hpxyz(-0.115566,-0.148760,1.017588));
  hpcpush(hpxyz(-0.083221,-0.154204,1.015236));
  hpcpush(hpxyz(-0.102733,-0.139423,1.014886));
  hpcpush(hpxyz(-0.081624,-0.138883,1.012892));
  hpcpush(hpxyz(-0.094972,-0.127846,1.012603));
  hpcpush(hpxyz(-0.078850,-0.123734,1.010706));
  hpcpush(hpxyz(-0.084756,-0.111394,1.009749));
  hpcpush(hpxyz(-0.078461,-0.101396,1.008185));
  hpcpush(hpxyz(-0.105410,-0.095717,1.010086));
  hpcpush(hpxyz(-0.086776,-0.085571,1.007399));
  hpcpush(hpxyz(-0.116319,-0.082393,1.010108));
  hpcpush(hpxyz(-0.100038,-0.069906,1.007420));
  hpcpush(hpxyz(-0.124869,-0.072739,1.010388));
  hpcpush(hpxyz(-0.108483,-0.056652,1.007461));
  hpcpush(hpxyz(-0.134526,-0.049690,1.010231));
  hpcpush(hpxyz(-0.114489,-0.040975,1.007366));
  hpcpush(hpxyz(-0.128124,-0.037470,1.008871));
  hpcpush(hpxyz(-0.116850,-0.026502,1.007153));
  hpcpush(hpxyz(-0.134264,-0.022982,1.009235));
  hpcpush(hpxyz(-0.119261,-0.012047,1.007158));
  hpcpush(hpxyz(-0.146785,-0.001213,1.010716));
  hpcpush(hpxyz(-0.146785,0.001213,1.010716));
  
  bshape(shPSword, 42);
  hpcpush(hpxyz(0.093822,0.244697,1.033769));
  hpcpush(hpxyz(0.105758,0.251015,1.036433));
  hpcpush(hpxyz(0.110908,0.249862,1.036693));
  hpcpush(hpxyz(0.110690,0.245554,1.035640));
  hpcpush(hpxyz(0.113376,0.247134,1.036306));
  hpcpush(hpxyz(0.117228,0.245924,1.036446));
  hpcpush(hpxyz(0.127263,0.237981,1.035775));
  hpcpush(hpxyz(0.131886,0.226997,1.033887));
  hpcpush(hpxyz(0.116494,0.231721,1.033085));
  hpcpush(hpxyz(0.106117,0.231182,1.031846));
  hpcpush(hpxyz(0.105927,0.226986,1.030894));
  hpcpush(hpxyz(0.263283,-0.174653,1.048724));
  hpcpush(hpxyz(0.086104,0.209645,1.025361));
  hpcpush(hpxyz(0.079571,0.202657,1.023426));
  hpcpush(hpxyz(0.074206,0.190462,1.020677));
  hpcpush(hpxyz(0.068951,0.179766,1.018366));
  hpcpush(hpxyz(0.065727,0.200902,1.022097));
  hpcpush(hpxyz(0.068444,0.209067,1.023911));
  hpcpush(hpxyz(0.077641,0.221653,1.027209));
  hpcpush(hpxyz(0.086737,0.227526,1.029219));
  hpcpush(hpxyz(0.086260,0.248631,1.034049));
  hpcpush(hpxyz(0.086431,0.252937,1.035107));
  hpcpush(hpxyz(0.093822,0.244697,1.033769));

  bshape(shHedgehogBlade, 42);
  hpcpush(hpxyz(0.117178,0.032617,1.007370));
  hpcpush(hpxyz(0.102699,0.066452,1.007454));
  hpcpush(hpxyz(0.056807,0.109987,1.007633));
  hpcpush(hpxyz(0.052506,0.272774,1.037864));
  hpcpush(hpxyz(0.079931,0.279758,1.041467));
  hpcpush(hpxyz(0.082589,0.170109,1.017722));
//hpcpush(hpxyz(0.173109,0.220554,1.038562));
  hpcpush(hpxyz(0.139258,0.126935,1.017598));
  hpcpush(hpxyz(0.240653,0.136967,1.037629));
  hpcpush(hpxyz(0.177567,0.067821,1.017905));
  hpcpush(hpxyz(0.273978,0.042249,1.037713));
  hpcpush(hpxyz(0.187242,-0.000000,1.017379));
  hpcpush(hpxyz(0.187242,0.000000,1.017379));
  hpcpush(hpxyz(0.273978,-0.042249,1.037713));
  hpcpush(hpxyz(0.177567,-0.067821,1.017905));
  hpcpush(hpxyz(0.240653,-0.136967,1.037629));
  hpcpush(hpxyz(0.139258,-0.126935,1.017598));
//hpcpush(hpxyz(0.173109,-0.220554,1.038562));
  hpcpush(hpxyz(0.082589,-0.170109,1.017722));
  hpcpush(hpxyz(0.079931,-0.279758,1.041467));
  hpcpush(hpxyz(0.052506,-0.272774,1.037864));
  hpcpush(hpxyz(0.056807,-0.109987,1.007633));
  hpcpush(hpxyz(0.102699,-0.066452,1.007454));
  hpcpush(hpxyz(0.117178,-0.032617,1.007370));
  hpcpush(hpxyz(0.117178,0.032617,1.007370));
  
  bshape(shHedgehogBladePlayer, 42);
  hpcpush(hpxyz(0.117178,0.032617,1.007370));
  hpcpush(hpxyz(0.102699,0.066452,1.007454));
  hpcpush(hpxyz(0.056807,0.109987,1.007633));
  hpcpush(hpxyz(0.052506,0.272774,1.037864));
  hpcpush(hpxyz(0.079931,0.279758,1.041467));
  hpcpush(hpxyz(0.082589,0.170109,1.017722));
  hpcpush(hpxyz(0.173109,0.220554,1.038562));
  hpcpush(hpxyz(0.139258,0.126935,1.017598));
  hpcpush(hpxyz(0.240653,0.136967,1.037629));
  hpcpush(hpxyz(0.177567,0.067821,1.017905));
  hpcpush(hpxyz(0.273978,0.042249,1.037713));
  hpcpush(hpxyz(0.187242,-0.000000,1.017379));
  hpcpush(hpxyz(0.187242,0.000000,1.017379));
  hpcpush(hpxyz(0.273978,-0.042249,1.037713));
  hpcpush(hpxyz(0.177567,-0.067821,1.017905));
  hpcpush(hpxyz(0.240653,-0.136967,1.037629));
  hpcpush(hpxyz(0.139258,-0.126935,1.017598));
  hpcpush(hpxyz(0.173109,-0.220554,1.038562));
  hpcpush(hpxyz(0.082589,-0.170109,1.017722));
  hpcpush(hpxyz(0.079931,-0.279758,1.041467));
  hpcpush(hpxyz(0.052506,-0.272774,1.037864));
  hpcpush(hpxyz(0.056807,-0.109987,1.007633));
  hpcpush(hpxyz(0.102699,-0.066452,1.007454));
  hpcpush(hpxyz(0.117178,-0.032617,1.007370));
  hpcpush(hpxyz(0.117178,0.032617,1.007370));
  
  bshape(shFemaleBody, 41);
  hpcpush(hpxyz(-0.091723,0.073620,1.006893));
  hpcpush(hpxyz(-0.089581,0.094424,1.008435));
  hpcpush(hpxyz(-0.082621,0.117856,1.010305));
  hpcpush(hpxyz(-0.070099,0.258731,1.035304));
  hpcpush(hpxyz(-0.058825,0.268551,1.037102));
  hpcpush(hpxyz(-0.003834,0.273475,1.036727));
  hpcpush(hpxyz(0.040760,0.263668,1.034979));
  hpcpush(hpxyz(0.077891,0.260490,1.036302));
  hpcpush(hpxyz(0.085895,0.233685,1.030527));
  hpcpush(hpxyz(0.064882,0.210867,1.024048));
  hpcpush(hpxyz(0.037427,0.217075,1.023974));
  hpcpush(hpxyz(0.011258,0.226410,1.025372));
  hpcpush(hpxyz(-0.037448,0.218444,1.024266));
  hpcpush(hpxyz(-0.026537,0.112181,1.006622));
  hpcpush(hpxyz(-0.015609,0.091250,1.004276));
  hpcpush(hpxyz(0.007196,0.086357,1.003748));
  hpcpush(hpxyz(0.019189,0.083950,1.003701));
  hpcpush(hpxyz(0.027568,0.077909,1.003409));
  hpcpush(hpxyz(0.033526,0.068249,1.002887));
  hpcpush(hpxyz(0.034689,0.059808,1.002387));
  hpcpush(hpxyz(0.037035,0.046593,1.001770));
  hpcpush(hpxyz(0.029813,0.028620,1.000854));
  hpcpush(hpxyz(0.017865,0.010719,1.000217));
  hpcpush(hpxyz(0.010716,0.003572,1.000064));
  hpcpush(hpxyz(0.010716,-0.003572,1.000064));
  hpcpush(hpxyz(0.017865,-0.010719,1.000217));
  hpcpush(hpxyz(0.029813,-0.028620,1.000854));
  hpcpush(hpxyz(0.037035,-0.046593,1.001770));
  hpcpush(hpxyz(0.034689,-0.059808,1.002387));
  hpcpush(hpxyz(0.033526,-0.068249,1.002887));
  hpcpush(hpxyz(0.027568,-0.077909,1.003409));
  hpcpush(hpxyz(0.019189,-0.083950,1.003701));
  hpcpush(hpxyz(0.007196,-0.086357,1.003748));
  hpcpush(hpxyz(-0.015609,-0.091250,1.004276));
  hpcpush(hpxyz(-0.026537,-0.112181,1.006622));
  hpcpush(hpxyz(-0.037448,-0.218444,1.024266));
  hpcpush(hpxyz(0.011258,-0.226410,1.025372));
  hpcpush(hpxyz(0.037427,-0.217075,1.023974));
  hpcpush(hpxyz(0.064882,-0.210867,1.024048));
  hpcpush(hpxyz(0.085895,-0.233685,1.030527));
  hpcpush(hpxyz(0.077891,-0.260490,1.036302));
  hpcpush(hpxyz(0.040760,-0.263668,1.034979));
  hpcpush(hpxyz(-0.003834,-0.273475,1.036727));
  hpcpush(hpxyz(-0.058825,-0.268551,1.037102));
  hpcpush(hpxyz(-0.070099,-0.258731,1.035304));
  hpcpush(hpxyz(-0.082621,-0.117856,1.010305));
  hpcpush(hpxyz(-0.089581,-0.094424,1.008435));
  hpcpush(hpxyz(-0.091723,-0.073620,1.006893));
  hpcpush(hpxyz(-0.091723,0.073620,1.006893));

  bshape(shFemaleDress, 42);
  hpcpush(hpxyz(-0.094893,0.000000,1.004492));
  hpcpush(hpxyz(-0.098831,0.051826,1.006207));
  hpcpush(hpxyz(-0.095833,0.099473,1.009494));
  hpcpush(hpxyz(-0.087712,0.125476,1.011651));
  hpcpush(hpxyz(-0.020535,0.119586,1.007334));
  hpcpush(hpxyz(-0.012011,0.093684,1.004451));
  hpcpush(hpxyz(0.001200,0.088788,1.003935));
  hpcpush(hpxyz(0.021589,0.083958,1.003751));
  hpcpush(hpxyz(0.033549,0.073088,1.003228));
  hpcpush(hpxyz(0.041871,0.056227,1.002454));
  hpcpush(hpxyz(0.039397,0.035815,1.001416));
  hpcpush(hpxyz(0.030996,0.021459,1.000710));
  hpcpush(hpxyz(0.019057,0.011911,1.000252));
  hpcpush(hpxyz(0.010716,-0.001191,1.000058));
  hpcpush(hpxyz(0.010716,0.001191,1.000058));
  hpcpush(hpxyz(0.019057,-0.011911,1.000252));
  hpcpush(hpxyz(0.030996,-0.021459,1.000710));
  hpcpush(hpxyz(0.039397,-0.035815,1.001416));
  hpcpush(hpxyz(0.041871,-0.056227,1.002454));
  hpcpush(hpxyz(0.033549,-0.073088,1.003228));
  hpcpush(hpxyz(0.021589,-0.083958,1.003751));
  hpcpush(hpxyz(0.001200,-0.088788,1.003935));
  hpcpush(hpxyz(-0.012011,-0.093684,1.004451));
  hpcpush(hpxyz(-0.020535,-0.119586,1.007334));
  hpcpush(hpxyz(-0.087712,-0.125476,1.011651));
  hpcpush(hpxyz(-0.095833,-0.099473,1.009494));
  hpcpush(hpxyz(-0.098831,-0.051826,1.006207));
  hpcpush(hpxyz(-0.094893,-0.000000,1.004492));
  hpcpush(hpxyz(-0.094893,0.000000,1.004492));

  bshape(shDemon, 43);
  drawDemon(1);

  bshape(shArmor, 43);
  hpcpush(hpxyz(-0.131705,0.010875,1.008694));
  hpcpush(hpxyz(-0.133453,0.061874,1.010761));
  hpcpush(hpxyz(-0.093134,0.099182,1.009213));
  hpcpush(hpxyz(-0.097643,0.135480,1.013849));
  hpcpush(hpxyz(-0.010956,0.158255,1.012504));
  hpcpush(hpxyz(0.016844,0.113094,1.006516));
  hpcpush(hpxyz(0.022740,0.084974,1.003861));
  hpcpush(hpxyz(-0.062459,0.086481,1.005674));
  hpcpush(hpxyz(-0.061152,0.076740,1.004803));
  hpcpush(hpxyz(0.063569,0.076763,1.004954));
  hpcpush(hpxyz(0.074360,0.065964,1.004928));
  hpcpush(hpxyz(0.083920,0.049153,1.004718));
  hpcpush(hpxyz(0.088667,0.032352,1.004444));
  hpcpush(hpxyz(0.092226,0.002395,1.004247));
  hpcpush(hpxyz(0.057196,0.007150,1.001660));
  hpcpush(hpxyz(0.054891,0.041765,1.002376));
  hpcpush(hpxyz(0.039313,0.039313,1.001544));
  hpcpush(hpxyz(0.039313,-0.039313,1.001544));
  hpcpush(hpxyz(0.054891,-0.041765,1.002376));
  hpcpush(hpxyz(0.057196,-0.007150,1.001660));
  hpcpush(hpxyz(0.092226,-0.002395,1.004247));
  hpcpush(hpxyz(0.088667,-0.032352,1.004444));
  hpcpush(hpxyz(0.083920,-0.049153,1.004718));
  hpcpush(hpxyz(0.074360,-0.065964,1.004928));
  hpcpush(hpxyz(0.063569,-0.076763,1.004954));
  hpcpush(hpxyz(-0.061152,-0.076740,1.004803));
  hpcpush(hpxyz(-0.062459,-0.086481,1.005674));
  hpcpush(hpxyz(0.022740,-0.084974,1.003861));
  hpcpush(hpxyz(0.016844,-0.113094,1.006516));
  hpcpush(hpxyz(-0.010956,-0.158255,1.012504));
  hpcpush(hpxyz(-0.097643,-0.135480,1.013849));
  hpcpush(hpxyz(-0.093134,-0.099182,1.009213));
  hpcpush(hpxyz(-0.133453,-0.061874,1.010761));
  hpcpush(hpxyz(-0.131705,-0.010875,1.008694));
  hpcpush(hpxyz(-0.131705,0.010875,1.008694));

  bshape(shHood, 43);
  hpcpush(hpxyz(-0.289108,0.001285,1.040954));
  hpcpush(hpxyz(-0.253872,0.006315,1.031742));
  hpcpush(hpxyz(-0.239738,0.013807,1.028428));
  hpcpush(hpxyz(-0.225917,0.023715,1.025476));
  hpcpush(hpxyz(-0.220610,0.037392,1.024728));
  hpcpush(hpxyz(-0.239040,0.055357,1.029662));
  hpcpush(hpxyz(-0.278480,0.068016,1.040278));
  hpcpush(hpxyz(-0.290572,0.069737,1.043693));
  hpcpush(hpxyz(-0.249897,0.082453,1.034044));
  hpcpush(hpxyz(-0.220651,0.087759,1.027808));
  hpcpush(hpxyz(-0.204652,0.097335,1.025357));
  hpcpush(hpxyz(-0.243692,0.127587,1.037142));
  hpcpush(hpxyz(-0.294677,0.136206,1.051374));
  hpcpush(hpxyz(-0.303482,0.130064,1.053099));
  hpcpush(hpxyz(-0.255520,0.152279,1.043302));
  hpcpush(hpxyz(-0.227045,0.154340,1.037001));
  hpcpush(hpxyz(-0.176957,0.161897,1.028360));
  hpcpush(hpxyz(-0.131265,0.160986,1.021346));
  hpcpush(hpxyz(-0.098249,0.157199,1.017037));
  hpcpush(hpxyz(-0.054857,0.153598,1.013214));
  hpcpush(hpxyz(-0.031595,0.149471,1.011603));
  hpcpush(hpxyz(-0.001208,0.130439,1.008472));
  hpcpush(hpxyz(0.016853,0.115564,1.006796));
  hpcpush(hpxyz(0.028853,0.106995,1.006121));
  hpcpush(hpxyz(0.043228,0.096063,1.005533));
  hpcpush(hpxyz(0.053964,0.082746,1.004868));
  hpcpush(hpxyz(0.056228,0.064603,1.003661));
  hpcpush(hpxyz(0.063348,0.049005,1.003202));
  hpcpush(hpxyz(0.054927,0.048957,1.002703));
  hpcpush(hpxyz(0.014325,0.070433,1.002580));
  hpcpush(hpxyz(-0.046699,0.077832,1.004111));
  hpcpush(hpxyz(-0.088908,0.061274,1.005813));
  hpcpush(hpxyz(-0.081520,0.052748,1.004703));
  hpcpush(hpxyz(-0.044233,0.068142,1.003294));
  hpcpush(hpxyz(0.011920,0.059602,1.001846));
  hpcpush(hpxyz(0.086263,0.037141,1.004401));
  hpcpush(hpxyz(0.086263,-0.037141,1.004401));
  hpcpush(hpxyz(0.011920,-0.059602,1.001846));
  hpcpush(hpxyz(-0.044233,-0.068142,1.003294));
  hpcpush(hpxyz(-0.081520,-0.052748,1.004703));
  hpcpush(hpxyz(-0.088908,-0.061274,1.005813));
  hpcpush(hpxyz(-0.046699,-0.077832,1.004111));
  hpcpush(hpxyz(0.014325,-0.070433,1.002580));
  hpcpush(hpxyz(0.054927,-0.048957,1.002703));
  hpcpush(hpxyz(0.063348,-0.049005,1.003202));
  hpcpush(hpxyz(0.056228,-0.064603,1.003661));
  hpcpush(hpxyz(0.053964,-0.082746,1.004868));
  hpcpush(hpxyz(0.043228,-0.096063,1.005533));
  hpcpush(hpxyz(0.028853,-0.106995,1.006121));
  hpcpush(hpxyz(0.016853,-0.115564,1.006796));
  hpcpush(hpxyz(-0.001208,-0.130439,1.008472));
  hpcpush(hpxyz(-0.031595,-0.149471,1.011603));
  hpcpush(hpxyz(-0.054857,-0.153598,1.013214));
  hpcpush(hpxyz(-0.098249,-0.157199,1.017037));
  hpcpush(hpxyz(-0.131265,-0.160986,1.021346));
  hpcpush(hpxyz(-0.176957,-0.161897,1.028360));
  hpcpush(hpxyz(-0.227045,-0.154340,1.037001));
  hpcpush(hpxyz(-0.255520,-0.152279,1.043302));
  hpcpush(hpxyz(-0.303482,-0.130064,1.053099));
  hpcpush(hpxyz(-0.294677,-0.136206,1.051374));
  hpcpush(hpxyz(-0.243692,-0.127587,1.037142));
  hpcpush(hpxyz(-0.204652,-0.097335,1.025357));
  hpcpush(hpxyz(-0.220651,-0.087759,1.027808));
  hpcpush(hpxyz(-0.249897,-0.082453,1.034044));
  hpcpush(hpxyz(-0.290572,-0.069737,1.043693));
  hpcpush(hpxyz(-0.278480,-0.068016,1.040278));
  hpcpush(hpxyz(-0.239040,-0.055357,1.029662));
  hpcpush(hpxyz(-0.220610,-0.037392,1.024728));
  hpcpush(hpxyz(-0.225917,-0.023715,1.025476));
  hpcpush(hpxyz(-0.239738,-0.013807,1.028428));
  hpcpush(hpxyz(-0.253872,-0.006315,1.031742));
  hpcpush(hpxyz(-0.289108,-0.001285,1.040954));
  hpcpush(hpxyz(-0.289108,0.001285,1.040954));

  bshape(shPHead, 43);
  hpcpush(hpxyz(0.060794,0.001192,1.001847));
  hpcpush(hpxyz(0.058426,0.023847,1.001989));
  hpcpush(hpxyz(0.050054,0.030986,1.001731));
  hpcpush(hpxyz(0.042896,0.038130,1.001646));
  hpcpush(hpxyz(0.044109,0.042917,1.001892));
  hpcpush(hpxyz(0.032180,0.050058,1.001769));
  hpcpush(hpxyz(0.017884,0.059612,1.001935));
  hpcpush(hpxyz(0.005963,0.064401,1.002089));
  hpcpush(hpxyz(-0.009546,0.068015,1.002356));
  hpcpush(hpxyz(-0.022689,0.070455,1.002736));
  hpcpush(hpxyz(-0.044247,0.070556,1.003462));
  hpcpush(hpxyz(-0.053847,0.068206,1.003769));
  hpcpush(hpxyz(-0.047819,0.065752,1.003300));
  hpcpush(hpxyz(-0.040573,0.056087,1.002393));
  hpcpush(hpxyz(-0.040563,0.053686,1.002261));
  hpcpush(hpxyz(-0.053753,0.053753,1.002885));
  hpcpush(hpxyz(-0.067016,0.056246,1.003820));
  hpcpush(hpxyz(-0.090053,0.054032,1.005499));
  hpcpush(hpxyz(-0.097365,0.051688,1.006057));
  hpcpush(hpxyz(-0.072989,0.046665,1.003746));
  hpcpush(hpxyz(-0.065710,0.040621,1.002979));
  hpcpush(hpxyz(-0.063272,0.034621,1.002598));
  hpcpush(hpxyz(-0.064461,0.031037,1.002556));
  hpcpush(hpxyz(-0.074098,0.028683,1.003152));
  hpcpush(hpxyz(-0.094756,0.031185,1.004963));
  hpcpush(hpxyz(-0.113149,0.027685,1.006762));
  hpcpush(hpxyz(-0.114376,0.026487,1.006868));
  hpcpush(hpxyz(-0.088611,0.020357,1.004125));
  hpcpush(hpxyz(-0.087387,0.017956,1.003972));
  hpcpush(hpxyz(-0.103257,0.018010,1.005478));
  hpcpush(hpxyz(-0.127970,0.014487,1.008259));
  hpcpush(hpxyz(-0.164656,0.006098,1.013484));
  hpcpush(hpxyz(-0.174996,0.001224,1.015197));
  hpcpush(hpxyz(-0.174996,-0.001224,1.015197));
  hpcpush(hpxyz(-0.164656,-0.006098,1.013484));
  hpcpush(hpxyz(-0.127970,-0.014487,1.008259));
  hpcpush(hpxyz(-0.103257,-0.018010,1.005478));
  hpcpush(hpxyz(-0.087387,-0.017956,1.003972));
  hpcpush(hpxyz(-0.088611,-0.020357,1.004125));
  hpcpush(hpxyz(-0.114376,-0.026487,1.006868));
  hpcpush(hpxyz(-0.113149,-0.027685,1.006762));
  hpcpush(hpxyz(-0.094756,-0.031185,1.004963));
  hpcpush(hpxyz(-0.074098,-0.028683,1.003152));
  hpcpush(hpxyz(-0.064461,-0.031037,1.002556));
  hpcpush(hpxyz(-0.063272,-0.034621,1.002598));
  hpcpush(hpxyz(-0.065710,-0.040621,1.002979));
  hpcpush(hpxyz(-0.072989,-0.046665,1.003746));
  hpcpush(hpxyz(-0.097365,-0.051688,1.006057));
  hpcpush(hpxyz(-0.090053,-0.054032,1.005499));
  hpcpush(hpxyz(-0.067016,-0.056246,1.003820));
  hpcpush(hpxyz(-0.053753,-0.053753,1.002885));
  hpcpush(hpxyz(-0.040563,-0.053686,1.002261));
  hpcpush(hpxyz(-0.040573,-0.056087,1.002393));
  hpcpush(hpxyz(-0.047819,-0.065752,1.003300));
  hpcpush(hpxyz(-0.053847,-0.068206,1.003769));
  hpcpush(hpxyz(-0.044247,-0.070556,1.003462));
  hpcpush(hpxyz(-0.022689,-0.070455,1.002736));
  hpcpush(hpxyz(-0.009546,-0.068015,1.002356));
  hpcpush(hpxyz(0.005963,-0.064401,1.002089));
  hpcpush(hpxyz(0.017884,-0.059612,1.001935));
  hpcpush(hpxyz(0.032180,-0.050058,1.001769));
  hpcpush(hpxyz(0.044109,-0.042917,1.001892));
  hpcpush(hpxyz(0.042896,-0.038130,1.001646));
  hpcpush(hpxyz(0.050054,-0.030986,1.001731));
  hpcpush(hpxyz(0.058426,-0.023847,1.001989));
  hpcpush(hpxyz(0.060794,-0.001192,1.001847));
  hpcpush(hpxyz(0.060794,0.001192,1.001847));

  shGolemhead = shDisk; shGolemhead.prio = 43;

  bshape(shFemaleHair, 43);
  hpcpush(hpxyz(-0.185924,0.002463,1.017140));
  hpcpush(hpxyz(-0.273576,0.019176,1.036924));
  hpcpush(hpxyz(-0.186050,0.025874,1.017489));
  hpcpush(hpxyz(-0.268509,0.057538,1.037019));
  hpcpush(hpxyz(-0.183840,0.054288,1.018206));
  hpcpush(hpxyz(-0.258480,0.097250,1.037434));
  hpcpush(hpxyz(-0.174976,0.069005,1.017535));
  hpcpush(hpxyz(-0.246145,0.133329,1.038443));
  hpcpush(hpxyz(0.029891,0.058585,1.002161));
  hpcpush(hpxyz(0.045415,0.043025,1.001955));
  hpcpush(hpxyz(0.051358,0.025082,1.001632));
  hpcpush(hpxyz(0.051358,-0.025082,1.001632));
  hpcpush(hpxyz(0.045415,-0.043025,1.001955));
  hpcpush(hpxyz(0.029891,-0.058585,1.002161));
  hpcpush(hpxyz(-0.246145,-0.133329,1.038443));
  hpcpush(hpxyz(-0.174976,-0.069005,1.017535));
  hpcpush(hpxyz(-0.258480,-0.097250,1.037434));
  hpcpush(hpxyz(-0.183840,-0.054288,1.018206));
  hpcpush(hpxyz(-0.268509,-0.057538,1.037019));
  hpcpush(hpxyz(-0.186050,-0.025874,1.017489));
  hpcpush(hpxyz(-0.273576,-0.019176,1.036924));
  hpcpush(hpxyz(-0.185924,-0.002463,1.017140));
  hpcpush(hpxyz(-0.185924,0.002463,1.017140));

  bshape(shPFace, 44);
  hpcpush(hpxyz(0.011878,-0.000000,1.000071));
  hpcpush(hpxyz(0.011880,0.010692,1.000128));
  hpcpush(hpxyz(0.014262,0.022581,1.000357));
  hpcpush(hpxyz(0.015455,0.027343,1.000493));
  hpcpush(hpxyz(0.017842,0.034494,1.000754));
  hpcpush(hpxyz(0.021425,0.041659,1.001097));
  hpcpush(hpxyz(0.025005,0.044056,1.001282));
  hpcpush(hpxyz(0.029785,0.047656,1.001578));
  hpcpush(hpxyz(0.032176,0.048860,1.001710));
  hpcpush(hpxyz(0.036964,0.051273,1.001996));
  hpcpush(hpxyz(0.044133,0.048905,1.002167));
  hpcpush(hpxyz(0.046519,0.046519,1.002162));
  hpcpush(hpxyz(0.051292,0.041749,1.002185));
  hpcpush(hpxyz(0.056061,0.034591,1.002167));
  hpcpush(hpxyz(0.060844,0.028632,1.002258));
  hpcpush(hpxyz(0.064425,0.020282,1.002278));
  hpcpush(hpxyz(0.064417,0.016701,1.002212));
  hpcpush(hpxyz(0.069220,0.009548,1.002438));
  hpcpush(hpxyz(0.075246,0.003583,1.002833));
  hpcpush(hpxyz(0.077662,0.001195,1.003012));
  hpcpush(hpxyz(0.080082,-0.000000,1.003201));
  hpcpush(hpxyz(0.080082,0.000000,1.003201));
  hpcpush(hpxyz(0.077662,-0.001195,1.003012));
  hpcpush(hpxyz(0.075246,-0.003583,1.002833));
  hpcpush(hpxyz(0.069220,-0.009548,1.002438));
  hpcpush(hpxyz(0.064417,-0.016701,1.002212));
  hpcpush(hpxyz(0.064425,-0.020282,1.002278));
  hpcpush(hpxyz(0.060844,-0.028632,1.002258));
  hpcpush(hpxyz(0.056061,-0.034591,1.002167));
  hpcpush(hpxyz(0.051292,-0.041749,1.002185));
  hpcpush(hpxyz(0.046519,-0.046519,1.002162));
  hpcpush(hpxyz(0.044133,-0.048905,1.002167));
  hpcpush(hpxyz(0.036964,-0.051273,1.001996));
  hpcpush(hpxyz(0.032176,-0.048860,1.001710));
  hpcpush(hpxyz(0.029785,-0.047656,1.001578));
  hpcpush(hpxyz(0.025005,-0.044056,1.001282));
  hpcpush(hpxyz(0.021425,-0.041659,1.001097));
  hpcpush(hpxyz(0.017842,-0.034494,1.000754));
  hpcpush(hpxyz(0.015455,-0.027343,1.000493));
  hpcpush(hpxyz(0.014262,-0.022581,1.000357));
  hpcpush(hpxyz(0.011880,-0.010692,1.000128));
  hpcpush(hpxyz(0.011878,0.000000,1.000071));
  hpcpush(hpxyz(0.011878,-0.000000,1.000071));

  bshape(shEyes, 46);
  hpcpush(hpxyz(0.070467,0.026276,1.002824));
  hpcpush(hpxyz(0.054837,0.027418,1.001878));
  hpcpush(hpxyz(0.040494,0.034539,1.001415));
  hpcpush(hpxyz(0.034561,0.047670,1.001732));
  hpcpush(hpxyz(0.032259,0.070492,1.003000));
  hpcpush(hpxyz(0.044434,0.096073,1.005587));
  hpcpush(hpxyz(0.065124,0.106127,1.007722));
  hpcpush(hpxyz(0.089628,0.109007,1.009909));
  hpcpush(hpxyz(0.110425,0.098291,1.010868));
  hpcpush(hpxyz(0.121434,0.088647,1.011239));
  hpcpush(hpxyz(0.124892,0.073965,1.010479));
  hpcpush(hpxyz(0.123474,0.064158,1.009635));
  hpcpush(hpxyz(0.117069,0.050690,1.008104));
  hpcpush(hpxyz(0.113313,0.047013,1.007497));
  hpcpush(hpxyz(0.098463,0.037224,1.005525));
  hpcpush(hpxyz(0.081366,0.029914,1.003751));
  hpcpush(hpxyz(0.070467,0.026276,1.002824));
  hpcpush(hpxyz(0.070467,-0.026276,1.002824));
  hpcpush(hpxyz(0.081366,-0.029914,1.003751));
  hpcpush(hpxyz(0.098463,-0.037224,1.005525));
  hpcpush(hpxyz(0.113313,-0.047013,1.007497));
  hpcpush(hpxyz(0.117069,-0.050690,1.008104));
  hpcpush(hpxyz(0.123474,-0.064158,1.009635));
  hpcpush(hpxyz(0.124892,-0.073965,1.010479));
  hpcpush(hpxyz(0.121434,-0.088647,1.011239));
  hpcpush(hpxyz(0.110425,-0.098291,1.010868));
  hpcpush(hpxyz(0.089628,-0.109007,1.009909));
  hpcpush(hpxyz(0.065124,-0.106127,1.007722));
  hpcpush(hpxyz(0.044434,-0.096073,1.005587));
  hpcpush(hpxyz(0.032259,-0.070492,1.003000));
  hpcpush(hpxyz(0.034561,-0.047670,1.001732));
  hpcpush(hpxyz(0.040494,-0.034539,1.001415));
  hpcpush(hpxyz(0.054837,-0.027418,1.001878));
  hpcpush(hpxyz(0.070467,-0.026276,1.002824));
  hpcpush(hpxyz(0.070467,0.026276,1.002824));
  
  bshape(shShark, 47);
  hpcpush(hpxyz(-0.254548,0.002533,1.031892));
  hpcpush(hpxyz(-0.295445,0.115041,1.049058));
  hpcpush(hpxyz(-0.214304,0.034887,1.023300));
  hpcpush(hpxyz(-0.113418,0.026545,1.006761));
  hpcpush(hpxyz(-0.034699,0.062219,1.002534));
  hpcpush(hpxyz(-0.010782,0.079071,1.003179));
  hpcpush(hpxyz(0.009598,0.087578,1.003874));
  hpcpush(hpxyz(0.037297,0.096250,1.005313));
  hpcpush(hpxyz(-0.001207,0.119535,1.007120));
  hpcpush(hpxyz(-0.032645,0.120908,1.007812));
  hpcpush(hpxyz(-0.002431,0.144615,1.010406));
  hpcpush(hpxyz(0.027983,0.145997,1.010989));
  hpcpush(hpxyz(0.058411,0.137510,1.011099));
  hpcpush(hpxyz(0.096163,0.115639,1.011247));
  hpcpush(hpxyz(0.130542,0.089062,1.012410));
  hpcpush(hpxyz(0.175135,0.075234,1.018004));
  hpcpush(hpxyz(0.227410,0.066590,1.027691));
  hpcpush(hpxyz(0.215688,0.036156,1.023635));
  hpcpush(hpxyz(0.187577,0.041958,1.018305));
  hpcpush(hpxyz(0.184662,0.017235,1.017053));
  hpcpush(hpxyz(0.208627,0.008693,1.021568));
  hpcpush(hpxyz(0.214217,0.028645,1.023088));
  hpcpush(hpxyz(0.229902,0.056533,1.027643));
  hpcpush(hpxyz(0.249359,0.045568,1.031628));
  hpcpush(hpxyz(0.259206,0.035577,1.033660));
  hpcpush(hpxyz(0.276638,0.026895,1.037908));
  hpcpush(hpxyz(0.302288,0.020758,1.044897));
  hpcpush(hpxyz(0.330687,0.013175,1.053341));
  hpcpush(hpxyz(0.348759,-0.003993,1.059079));
  hpcpush(hpxyz(0.348759,0.003993,1.059079));
  hpcpush(hpxyz(0.330687,-0.013175,1.053341));
  hpcpush(hpxyz(0.302288,-0.020758,1.044897));
  hpcpush(hpxyz(0.276638,-0.026895,1.037908));
  hpcpush(hpxyz(0.259206,-0.035577,1.033660));
  hpcpush(hpxyz(0.249359,-0.045568,1.031628));
  hpcpush(hpxyz(0.229902,-0.056533,1.027643));
  hpcpush(hpxyz(0.214217,-0.028645,1.023088));
  hpcpush(hpxyz(0.208627,-0.008693,1.021568));
  hpcpush(hpxyz(0.184662,-0.017235,1.017053));
  hpcpush(hpxyz(0.187577,-0.041958,1.018305));
  hpcpush(hpxyz(0.215688,-0.036156,1.023635));
  hpcpush(hpxyz(0.227410,-0.066590,1.027691));
  hpcpush(hpxyz(0.175135,-0.075234,1.018004));
  hpcpush(hpxyz(0.130542,-0.089062,1.012410));
  hpcpush(hpxyz(0.096163,-0.115639,1.011247));
  hpcpush(hpxyz(0.058411,-0.137510,1.011099));
  hpcpush(hpxyz(0.027983,-0.145997,1.010989));
  hpcpush(hpxyz(-0.002431,-0.144615,1.010406));
  hpcpush(hpxyz(-0.032645,-0.120908,1.007812));
  hpcpush(hpxyz(-0.001207,-0.119535,1.007120));
  hpcpush(hpxyz(0.037297,-0.096250,1.005313));
  hpcpush(hpxyz(0.009598,-0.087578,1.003874));
  hpcpush(hpxyz(-0.010782,-0.079071,1.003179));
  hpcpush(hpxyz(-0.034699,-0.062219,1.002534));
  hpcpush(hpxyz(-0.113418,-0.026545,1.006761));
  hpcpush(hpxyz(-0.214304,-0.034887,1.023300));
  hpcpush(hpxyz(-0.295445,-0.115041,1.049058));
  hpcpush(hpxyz(-0.254548,-0.002533,1.031892));
  hpcpush(hpxyz(-0.254548,0.002533,1.031892));

  bshape(shWolfBody, 33);
  hpcpush(hwolf(-0.311492,0.001303,1.047391));
  hpcpush(hwolf(-0.165080,0.009783,1.013581));
  hpcpush(hwolf(-0.195342,0.027200,1.019264));
  hpcpush(hwolf(-0.153616,0.025603,1.012054));
  hpcpush(hwolf(-0.173302,0.051622,1.016218));
  hpcpush(hwolf(-0.128440,0.037563,1.008914));
  hpcpush(hwolf(-0.152961,0.068527,1.013949));
  hpcpush(hwolf(-0.107392,0.045853,1.006795));
  hpcpush(hwolf(-0.121764,0.090105,1.011408));
  hpcpush(hwolf(-0.079263,0.050440,1.004404));
  hpcpush(hwolf(-0.084555,0.086971,1.007330));
  hpcpush(hwolf(-0.043029,0.046615,1.002010));
  hpcpush(hwolf(-0.049440,0.102497,1.006454));
  hpcpush(hwolf(-0.020293,0.047747,1.001345));
  hpcpush(hwolf(-0.012028,0.101033,1.005163));
  hpcpush(hwolf(0.009546,0.047732,1.001184));
  hpcpush(hwolf(0.027630,0.091298,1.004539));
  hpcpush(hwolf(0.043025,0.045415,1.001955));
  hpcpush(hwolf(0.071007,0.077024,1.005472));
  hpcpush(hwolf(0.071856,0.028742,1.002990));
  hpcpush(hwolf(0.113578,0.045915,1.007476));
  hpcpush(hwolf(0.090012,0.007201,1.004069));
  hpcpush(hwolf(0.090012,-0.007201,1.004069));
  hpcpush(hwolf(0.113578,-0.045915,1.007476));
  hpcpush(hwolf(0.071856,-0.028742,1.002990));
  hpcpush(hwolf(0.071007,-0.077024,1.005472));
  hpcpush(hwolf(0.043025,-0.045415,1.001955));
  hpcpush(hwolf(0.027630,-0.091298,1.004539));
  hpcpush(hwolf(0.009546,-0.047732,1.001184));
  hpcpush(hwolf(-0.012028,-0.101033,1.005163));
  hpcpush(hwolf(-0.020293,-0.047747,1.001345));
  hpcpush(hwolf(-0.049440,-0.102497,1.006454));
  hpcpush(hwolf(-0.043029,-0.046615,1.002010));
  hpcpush(hwolf(-0.084555,-0.086971,1.007330));
  hpcpush(hwolf(-0.079263,-0.050440,1.004404));
  hpcpush(hwolf(-0.121764,-0.090105,1.011408));
  hpcpush(hwolf(-0.107392,-0.045853,1.006795));
  hpcpush(hwolf(-0.152961,-0.068527,1.013949));
  hpcpush(hwolf(-0.128440,-0.037563,1.008914));
  hpcpush(hwolf(-0.173302,-0.051622,1.016218));
  hpcpush(hwolf(-0.153616,-0.025603,1.012054));
  hpcpush(hwolf(-0.195342,-0.027200,1.019264));
  hpcpush(hwolf(-0.165080,-0.009783,1.013581));
  hpcpush(hwolf(-0.311492,-0.001303,1.047391));
  hpcpush(hwolf(-0.311492,0.001303,1.047391));

  // group 2 layer 1

  bshape(shWolfHead, 34);
  hpcpush(hwolf(0.183292,0.002460,1.016662));
  hpcpush(hwolf(0.178071,0.011053,1.015791));
  hpcpush(hwolf(0.185996,0.019708,1.017341));
  hpcpush(hwolf(0.170281,0.015926,1.014519));
  hpcpush(hwolf(0.170341,0.024510,1.014700));
  hpcpush(hwolf(0.158693,0.019531,1.012702));
  hpcpush(hwolf(0.158807,0.032983,1.013068));
  hpcpush(hwolf(0.147214,0.020683,1.010989));
  hpcpush(hwolf(0.142274,0.036481,1.010729));
  hpcpush(hwolf(0.135869,0.025475,1.009510));
  hpcpush(hwolf(0.129693,0.037575,1.009075));
  hpcpush(hwolf(0.118394,0.028994,1.007401));
  hpcpush(hwolf(0.114793,0.043501,1.007507));
  hpcpush(hwolf(0.102362,0.033719,1.005791));
  hpcpush(hwolf(0.100073,0.053051,1.006394));
  hpcpush(hwolf(0.087692,0.037239,1.004528));
  hpcpush(hwolf(0.080602,0.063760,1.005267));
  hpcpush(hwolf(0.065828,0.032315,1.002685));
  hpcpush(hwolf(0.008362,0.058537,1.001747));
  hpcpush(hwolf(0.046539,0.014320,1.001185));
  hpcpush(hwolf(0.028597,0.008341,1.000444));
  hpcpush(hwolf(0.051325,-0.000000,1.001316));
  hpcpush(hwolf(0.051325,0.000000,1.001316));
  hpcpush(hwolf(0.028597,-0.008341,1.000444));
  hpcpush(hwolf(0.046539,-0.014320,1.001185));
  hpcpush(hwolf(0.008362,-0.058537,1.001747));
  hpcpush(hwolf(0.065828,-0.032315,1.002685));
  hpcpush(hwolf(0.080602,-0.063760,1.005267));
  hpcpush(hwolf(0.087692,-0.037239,1.004528));
  hpcpush(hwolf(0.100073,-0.053051,1.006394));
  hpcpush(hwolf(0.102362,-0.033719,1.005791));
  hpcpush(hwolf(0.114793,-0.043501,1.007507));
  hpcpush(hwolf(0.118394,-0.028994,1.007401));
  hpcpush(hwolf(0.129693,-0.037575,1.009075));
  hpcpush(hwolf(0.135869,-0.025475,1.009510));
  hpcpush(hwolf(0.142274,-0.036481,1.010729));
  hpcpush(hwolf(0.147214,-0.020683,1.010989));
  hpcpush(hwolf(0.158807,-0.032983,1.013068));
  hpcpush(hwolf(0.158693,-0.019531,1.012702));
  hpcpush(hwolf(0.170341,-0.024510,1.014700));
  hpcpush(hwolf(0.170281,-0.015926,1.014519));
  hpcpush(hwolf(0.185996,-0.019708,1.017341));
  hpcpush(hwolf(0.178071,-0.011053,1.015791));
  hpcpush(hwolf(0.183292,-0.002460,1.016662));
  hpcpush(hwolf(0.183292,0.002460,1.016662));

  // group 2 layer 2

  bshape(shWolfLegs, 32);
  hpcpush(hwolf(-0.139559,-0.001214,1.009692));
  hpcpush(hwolf(-0.139599,0.016995,1.009840));
  hpcpush(hwolf(-0.228445,0.053973,1.027181));
  hpcpush(hwolf(-0.209280,0.056057,1.023201));
  hpcpush(hwolf(-0.216331,0.065024,1.025196));
  hpcpush(hwolf(-0.204121,0.065966,1.022750));
  hpcpush(hwolf(-0.204581,0.081084,1.023928));
  hpcpush(hwolf(-0.194817,0.069489,1.021167));
  hpcpush(hwolf(-0.113487,0.036219,1.007071));
  hpcpush(hwolf(-0.008338,0.022632,1.000291));
  hpcpush(hwolf(0.076062,0.091757,1.007077));
  hpcpush(hwolf(0.081210,0.107875,1.009075));
  hpcpush(hwolf(0.084610,0.090653,1.007659));
  hpcpush(hwolf(0.099450,0.094598,1.009376));
  hpcpush(hwolf(0.089336,0.078471,1.007045));
  hpcpush(hwolf(0.105258,0.072592,1.008141));
  hpcpush(hwolf(0.083028,0.062572,1.005390));
  hpcpush(hwolf(0.025016,0.001191,1.000314));
  hpcpush(hwolf(0.025016,-0.001191,1.000314));
  hpcpush(hwolf(0.083028,-0.062572,1.005390));
  hpcpush(hwolf(0.105258,-0.072592,1.008141));
  hpcpush(hwolf(0.089336,-0.078471,1.007045));
  hpcpush(hwolf(0.099450,-0.094598,1.009376));
  hpcpush(hwolf(0.084610,-0.090653,1.007659));
  hpcpush(hwolf(0.081210,-0.107875,1.009075));
  hpcpush(hwolf(0.076062,-0.091757,1.007077));
  hpcpush(hwolf(-0.008338,-0.022632,1.000291));
  hpcpush(hwolf(-0.113487,-0.036219,1.007071));
  hpcpush(hwolf(-0.194817,-0.069489,1.021167));
  hpcpush(hwolf(-0.204581,-0.081084,1.023928));
  hpcpush(hwolf(-0.204121,-0.065966,1.022750));
  hpcpush(hwolf(-0.216331,-0.065024,1.025196));
  hpcpush(hwolf(-0.209280,-0.056057,1.023201));
  hpcpush(hwolf(-0.228445,-0.053973,1.027181));
  hpcpush(hwolf(-0.139599,-0.016995,1.009840));
  hpcpush(hwolf(-0.139559,0.001214,1.009692));
  hpcpush(hwolf(-0.139559,-0.001214,1.009692));

  // group 2 layer 3

  bshape(shWolfEyes, 35);
  hpcpush(hwolf(0.172835,0.002452,1.014829));
  hpcpush(hwolf(0.156071,0.002439,1.012109));
  hpcpush(hwolf(0.154815,0.012190,1.011986));
  hpcpush(hwolf(0.138338,0.016989,1.009666));
  hpcpush(hwolf(0.135790,0.008487,1.009213));
  hpcpush(hwolf(0.147155,0.006081,1.010788));
  hpcpush(hwolf(0.153518,0.004874,1.011727));
  hpcpush(hwolf(0.156070,-0.000000,1.012106));
  hpcpush(hwolf(0.156070,0.000000,1.012106));
  hpcpush(hwolf(0.153518,-0.004874,1.011727));
  hpcpush(hwolf(0.147155,-0.006081,1.010788));
  hpcpush(hwolf(0.135790,-0.008487,1.009213));
  hpcpush(hwolf(0.138338,-0.016989,1.009666));
  hpcpush(hwolf(0.154815,-0.012190,1.011986));
  hpcpush(hwolf(0.156071,-0.002439,1.012109));
  hpcpush(hwolf(0.172835,-0.002452,1.014829));
  hpcpush(hwolf(0.172835,0.002452,1.014829));
  
  bshape(shBigHepta, 11);
  for(int t=0; t<=7; t++) hpcpush(ddi(t*12, -hexf*1.5) * C0);
  
  bshape(shBigTriangle, 11);
  for(int t=0; t<=3; t++) hpcpush(ddi(t*28, -hexf*1.5) * C0);
  
  bshapeend();

  for(int i=0; i<8; i++) 
    shUser[i][0].prio = 1 + i,
    shUser[i][1].prio = 1 + i,
    shUser[i][2].prio = 50 + i;

  prehpc = qhpc;
  }

struct drawshape {
  vector<hyperpoint> list;
  bool sym;
  int rots;
  };

drawshape dsUser[8][3], *dsCur;

int dslayer;
bool floordraw;

void pushShape(const drawshape& ds) {

  for(int r=0; r<ds.rots; r++) {
    for(int i=0; i<size(ds.list); i++)
      hpcpush(spin(2*M_PI*r/ds.rots) * ds.list[i]);

    if(ds.sym) {
  
      transmatrix mirrortrans = Id; mirrortrans[1][1] = -1;
      for(int i=size(ds.list)-1; i>=0; i--)
        hpcpush(spin(2*M_PI*r/ds.rots) * mirrortrans * ds.list[i]);
      }
    }
  hpcpush(ds.list[0]);
  }

void saveImages() {
  qhpc = prehpc;
  
  for(int i=0; i<8; i++) for(int j=0; j<3; j++) {
    if(size(dsUser[i][j].list)) {
      shUser[i][j].s = qhpc;
      pushShape(dsUser[i][j]);
      shUser[i][j].e = qhpc;
      }
    else shUser[i][j].s = 0;
    }
  }

void queuepoly(const transmatrix& V, int ct, const hpcshape& h, int col) {
  /* printf("draw poly #%d-%d at:\n", h.s, h.e);
  display(V); */
  polytodraw ptd;
  ptd.V = V;
  ptd.start = h.s, ptd.end = h.e;
  ptd.col = (darkened(col >> 8) << 8) + (col & 0xFF);
  ptd.prio = h.prio;
  ptds.push_back(ptd);
  }

