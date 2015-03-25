// Hyperbolic Rogue
// Copyright (C) 2011-2012 Zeno Rogue, see 'hyper.cpp' for details

// cells the game is played on

int fix6(int a) { return (a+96)% 6; }

struct cell : gcell {
  char type; // 6 for hexagons, 7 for heptagons
  unsigned char spn[7];
  heptagon *master;
  cell *mov[7]; // meaning very similar to heptagon::move
  };

int cellcount = 0;

void initcell(cell *c); // from game.cpp

cell *newCell(int type, heptagon *master) {
  cell *c = new cell;
  cellcount++;
  c->type = type;
  c->master = master;
  for(int i=0; i<7; i++) c->mov[i] = NULL;
  initcell(c);
  return c;
  }

void merge(cell *c, int d, cell *c2, int d2) {
  c->mov[d] = c2;
  c->spn[d] = d2;
  c2->mov[d2] = c;
  c2->spn[d2] = d;
  }

typedef unsigned short eucoord;

cell*& euclideanAtCreate(eucoord x, eucoord y);

union heptacoder {
  heptagon *h;
  struct { eucoord x; eucoord y; } c;
  };

void decodeMaster(heptagon *h, eucoord& x, eucoord& y) {
  heptacoder u;
  u.h = h; x = u.c.x; y = u.c.y;
  }

heptagon* encodeMaster(eucoord x, eucoord y) {
  heptacoder u;
  u.c.x = x; u.c.y = y;
  return u.h;
  }

// very similar to createMove in heptagon.cpp
cell *createMov(cell *c, int d) {

  if(euclid && !c->mov[d]) {
    eucoord x, y;
    decodeMaster(c->master, x, y);
    for(int dx=-1; dx<=1; dx++)
    for(int dy=-1; dy<=1; dy++)
      euclideanAtCreate(x+dx, y+dy);
    if(!c->mov[d]) { printf("fail!\n"); }
    }
  
  if(c->mov[d]) return c->mov[d];
  else if(c->type == 7) {
    cell *n = newCell(6, c->master);

    c->mov[d] = n; n->mov[0] = c;
    c->spn[d] = 0; n->spn[0] = d;
    
    heptspin hs; hs.h = c->master; hs.spin = d;
    
    heptspin hs2 = hsstep(hsspin(hs, 3), 3);
    
    // merge(hs2.h->c7, hs2.spin, n, 2);
    
    hs2.h->c7->mov[hs2.spin] = n; n->mov[2] = hs2.h->c7;
    hs2.h->c7->spn[hs2.spin] = 2; n->spn[2] = hs2.spin;
    
    hs2 = hsstep(hsspin(hs, 4), 4);
    // merge(hs2.h->c7, hs2.spin, n, 4);
    hs2.h->c7->mov[hs2.spin] = n; n->mov[4] = hs2.h->c7;
    hs2.h->c7->spn[hs2.spin] = 4; n->spn[4] = hs2.spin;
    
    }
  
  else if(d == 5) {
    int di = fixrot(c->spn[0]+1);
    cell *c2 = createMov(c->mov[0], di);
    merge(c, 5, c2, fix6(c->mov[0]->spn[di] + 1));
    
    // c->mov[5] = c->mov[0]->mov[fixrot(c->spn[0]+1)]; 
    // c->spn[5] = fix6(c->mov[0]->spn[fixrot(c->spn[0]+1)] + 1);
    }
  
  else if(d == 1) {
    int di = fixrot(c->spn[0]-1);
    cell *c2 = createMov(c->mov[0], di);
    merge(c, 1, c2, fix6(c->mov[0]->spn[di] - 1));
    
    // c->mov[1] = c->mov[0]->mov[fixrot(c->spn[0]-1)]; 
    // c->spn[1] = fix6(c->mov[0]->spn[fixrot(c->spn[0]-1)] - 1);
    }
  
  else if(d == 3) {
    int di = fixrot(c->spn[2]-1);
    cell *c2 = createMov(c->mov[2], di);
    merge(c, 3, c2, fix6(c->mov[2]->spn[di] - 1));
    // c->mov[3] = c->mov[2]->mov[fixrot(c->spn[2]-1)];
    // c->spn[3] = fix6(c->mov[2]->spn[fixrot(c->spn[2]-1)] - 1);
    }
  return c->mov[d];
  }

// similar to heptspin from heptagon.cpp
struct cellwalker {
  cell *c;
  int spin;
  cellwalker(cell *c, int spin) : c(c), spin(spin) {}
  cellwalker() {}
  };

void cwspin(cellwalker& cw, int d) {
  cw.spin = (cw.spin+d + 42) % cw.c->type;
  }

void cwstep(cellwalker& cw) {
  createMov(cw.c, cw.spin);
  int nspin = cw.c->spn[cw.spin];
  cw.c = cw.c->mov[cw.spin];
  cw.spin = nspin;
  }

// approximate distance of c from origin
int celldist(cell *c) {
  // EUCLIDEAN
  if(euclid) return 0; // todo fix
  if(c->type == 7) return c->master->distance;
  int d1 = c->mov[0]->master->distance;
  int d2 = c->mov[2]->master->distance;
  int d3 = c->mov[4]->master->distance;
  if((d1+d2+d3) % 3 == 2) return ((d1+d2+d3) / 3);
  else return ((d1+d2+d3) / 3);
  }

void eumerge(cell* c1, cell *c2, int s1, int s2) {
  if(!c2) return;
  c1->mov[s1] = c2; c1->spn[s1] = s2;
  c2->mov[s2] = c1; c2->spn[s2] = s1;
  }       

struct euclideanSlab {
  cell* a[256][256];
  euclideanSlab() {
    for(int y=0; y<256; y++) for(int x=0; x<256; x++)
      a[y][x] = NULL;
    }
  ~euclideanSlab() {
    for(int y=0; y<256; y++) for(int x=0; x<256; x++)
      if(a[y][x]) delete a[y][x];
    }
  };

euclideanSlab* euclidean[256][256];

//  map<pair<eucoord, eucoord>, cell*> euclidean;

cell*& euclideanAt(eucoord x, eucoord y) {
  euclideanSlab*& slab(euclidean[y>>8][x>>8]);
  if(!slab) slab = new euclideanSlab;
  return slab->a[y&255][x&255];
  }

cell*& euclideanAtCreate(eucoord x, eucoord y) {
  cell*& c ( euclideanAt(x,y) );
  if(!c) {
    c = newCell(6, &origin);
    c->master = encodeMaster(x,y);
    euclideanAt(x,y) = c;
    eumerge(c, euclideanAt(x+1,y), 0, 3);
    eumerge(c, euclideanAt(x,y+1), 1, 4);
    eumerge(c, euclideanAt(x-1,y+1), 2, 5);
    eumerge(c, euclideanAt(x-1,y), 3, 0);
    eumerge(c, euclideanAt(x,y-1), 4, 1);
    eumerge(c, euclideanAt(x+1,y-1), 5, 2);
    }
  return c;
  }


// initializer (also inits origin from heptagon.cpp)
void initcells() {

  origin.s = hsOrigin;
  for(int i=0; i<7; i++) origin.move[i] = NULL;
  origin.distance = 0;
  if(euclid)
    origin.c7 = euclideanAtCreate(0,0);
  else
    origin.c7 = newCell(7, &origin);
  }

#define DEBMEM(x) // { x fflush(stdout); }

void clearcell(cell *c) {
  if(!c) return;
  DEBMEM ( printf("c%d %p\n", c->type, c); )
  for(int t=0; t<c->type; t++) if(c->mov[t]) {
    DEBMEM ( printf("mov %p [%p] S%d\n", c->mov[t], c->mov[t]->mov[c->spn[t]], c->spn[t]); )
    if(c->mov[t]->mov[c->spn[t]] != NULL &&
      c->mov[t]->mov[c->spn[t]] != c) {
        printf("cell error\n");
        exit(1);
        }
    c->mov[t]->mov[c->spn[t]] = NULL;
    }
  DEBMEM ( printf("DEL %p\n", c); )
  delete c;
  }

void clearfrom(heptagon *at) {
  DEBMEM ( printf("from %p\n", at); )
  for(int i=0; i<7; i++) if(at->move[i] && at->spin[i] == 0 && at->move[i] != &origin)
    clearfrom(at->move[i]);
  DEBMEM ( printf("at %p\n", at); )
  if(at->c7) {
    for(int i=0; i<7; i++)
      clearcell(at->c7->mov[i]);
    clearcell(at->c7);
    }
  for(int i=0; i<7; i++) if(at->move[i]) {
    DEBMEM ( printf("!mov %p [%p]\n", at->move[i], at->move[i]->move[at->spin[i]]); )
    if(at->move[i]->move[at->spin[i]] != NULL &&
      at->move[i]->move[at->spin[i]] != at) {
        printf("hept error\n");
        exit(1);
        }
    at->move[i]->move[at->spin[i]] = NULL;
    at->move[i] = NULL;
    }
  DEBMEM ( printf("!DEL %p\n", at); )
  if(at != &origin) delete at;
  }

void verifycell(cell *c) {
  int t = c->type;
  for(int i=0; i<t; i++) {
    cell *c2 = c->mov[i];
    if(c2) {
      if(t == 7) verifycell(c2);
      if(c2->mov[c->spn[i]] && c2->mov[c->spn[i]] != c) 
        printf("cell error %p %p\n", c, c2);
      }
    }
  }

void verifycells(heptagon *at) {
  for(int i=0; i<7; i++) if(at->move[i] && at->spin[i] == 0 && at->move[i] != &origin)
    verifycells(at->move[i]);
  for(int i=0; i<7; i++) if(at->move[i] && at->move[i]->move[at->spin[i]] && at->move[i]->move[at->spin[i]] != at) {
    printf("hexmix error %p %p %p\n", at, at->move[i], at->move[i]->move[at->spin[i]]);
    }
  verifycell(at->c7);
  }

bool ishept(cell *c) {
  // EUCLIDEAN
  if(euclid) {
    eucoord x, y;
    decodeMaster(c->master, x, y);
    return (short(y+2*x))%3 == 0;
    }
  else return c->type == 7;
  }

void clearMemory() {
  extern void clearGameMemory();
  clearGameMemory();
  // EUCLIDEAN
  if(euclid) { 
    for(int y=0; y<256; y++) for(int x=0; x<256; x++)
      if(euclidean[y][x]) { 
        delete euclidean[y][x];
        euclidean[y][x] = NULL;
        }
    }
  else {
    DEBMEM ( verifycells(&origin); )
    clearfrom(&origin);
    }
  DEBMEM ( printf("ok\n"); )
  }
