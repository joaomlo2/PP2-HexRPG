// Hyperbolic Rogue
// Copyright (C) 2011-2012 Zeno Rogue, see 'hyper.cpp' for details

// heptagon here refers to underlying heptagonal tesselation
// (which you can see by changing the conditions in graph.cpp)

// automaton state
enum hstate { hsOrigin, hsA, hsB, hsError };

int fixrot(int a) { return (a+98)% 7; }
int fix42(int a) { return (a+420)% 42; }

struct heptagon;

struct cell;
cell *newCell(int type, heptagon *master);

struct heptagon {
  // automaton state
  hstate s : 8;
  // we are spin[i]-th neighbor of move[i]
  unsigned char spin[7];
  // neighbors; move[0] always goes towards origin,
  // and then we go clockwise
  heptagon* move[7];
  // distance from the origin
  short distance;
  heptagon*& modmove(int i) { return move[fixrot(i)]; }
  unsigned char& gspin(int i) { return spin[fixrot(i)]; }
  cell *c7;
  };

// the automaton is used to generate each heptagon in an unique way
// (you can see the tree obtained by changing the conditions in graph.cpp)
// from the origin we can go further in any direction, and from other heptagons
// we can go in directions 3 and 4 (0 is back to origin, so 3 and 4 go forward),
// and sometimes in direction 5

hstate transition(hstate s, int dir) {
  if(s == hsOrigin) return hsA;
  if(s == hsA && dir >= 3 && dir <= 4) return hsA;
  if(s == hsA && dir == 5) return hsB;
  if(s == hsB && dir == 4) return hsB;
  if(s == hsB && dir == 3) return hsA;
  return hsError;
  }

heptagon origin;

// create a new heptagon
void buildHeptagon(heptagon *parent, int d, hstate s) {
  heptagon *h = new heptagon;
  h->s = s;
  for(int i=0; i<7; i++) h->move[i] = NULL;
  h->move[0] = parent; h->spin[0] = d;
  parent->move[d] = h; parent->spin[d] = 0;
  h->c7 = newCell(7, h);
  h->distance = parent->distance + 1;
  }

// create h->move[d] if not created yet
heptagon *createStep(heptagon *h, int d);

void addSpin(heptagon *h, int d, heptagon *from, int rot, int spin) {
  rot = fixrot(rot);
  createStep(from, rot);
  h->move[d] = from->move[rot];
  h->spin[d] = fixrot(from->spin[rot] + spin);
  h->move[d]->move[fixrot(from->spin[rot] + spin)] = h;
  h->move[d]->spin[fixrot(from->spin[rot] + spin)] = d;
  }

heptagon *createStep(heptagon *h, int d) {
  d = fixrot(d);
  if(h->move[d]) return h->move[d];
  if(h->s == hsOrigin) {
    buildHeptagon(h, d, hsA);
    }
  else if(d == 1) {
    addSpin(h, d, h->move[0], h->spin[0]-1, -1);
    }
  else if(d == 6) {
    addSpin(h, d, h->move[0], h->spin[0]+1, +1);
    }
  else if(d == 2) {
    createStep(h->move[0], h->spin[0]-1);
    addSpin(h, d, h->move[0]->modmove(h->spin[0]-1), 5 + h->move[0]->gspin(h->spin[0]-1), -1);
    }
  else if(d == 5 && h->s == hsB) {
    createStep(h->move[0], h->spin[0]+1);
    addSpin(h, d, h->move[0]->modmove(h->spin[0]+1), 2 + h->move[0]->gspin(h->spin[0]+1), +1);
    }
  else
    buildHeptagon(h, d, (d == 5 || (h->s == hsB && d == 4)) ? hsB : hsA);
  return h->move[d];
  }

// a structure used to walk on the heptagonal tesselation
// (remembers not only the heptagon, but also direction)
struct heptspin {
  heptagon *h;
  int spin;
  };

heptspin hsstep(const heptspin &hs, int spin) {
  createStep(hs.h, hs.spin);
  heptspin res;
  res.h = hs.h->move[hs.spin];
  res.spin = fixrot(hs.h->spin[hs.spin] + spin);
  return res;
  }

heptspin hsspin(const heptspin &hs, int val) {
  heptspin res;
  res.h = hs.h;
  res.spin = fixrot(hs.spin + val);
  return res;
  }

// display the coordinates of the heptagon
void backtrace(heptagon *pos) {
  if(pos == &origin) return;
  backtrace(pos->move[0]);
  printf(" %d", pos->spin[0]);
  }

void hsshow(const heptspin& t) {
  printf("ORIGIN"); backtrace(t.h); printf(" (spin %d)\n", t.spin);
  }

