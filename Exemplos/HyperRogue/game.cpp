// Hyperbolic Rogue

// Copyright (C) 2011-2013 Zeno Rogue, see 'hyper.cpp' for details

#define DEB(x) // printf("%s\n", x); fflush(stdout);

#define SAGEMELT .1

time_t timerstart, savetime;
bool timerstopped;
int savecount;
int turncount;
int lastexplore;
eLand lastland;

int sagephase = 0;

bool showoff = false;
bool safety = false;
int showid = 0;

int currentLocalTreasure;

bool landvisited[16];

eLand showlist[10] = {
  laHell, laRlyeh, laAlchemist, laGraveyard, laCaves, laDesert, laIce, laJungle, laMotion, laMirror
  };

eLand firstland = laIce, euclidland = laIce;

extern void DEBT(const char *buf);

#define DEBT(x) // printf("%s\n", x);

bool eq(short a, short b) { return a==b; }

// game state
int items[ittypes], kills[motypes], explore[10], exploreland[10][landtypes], landcount[landtypes];
bool playerdead = false;  // obsolete
bool playermoved = true;  // center on the PC?
bool flipplayer = true;   // flip the player image after move, do not flip after attack
int  cheater = 0;         // did the player cheat?

#define INF  9999
#define INFD 20

vector<cell*> dcal;   // queue for cpdist
vector<cell*> pathq;  // queue for pathdist

vector<cell*> pathqm; // list of monsters to move (pathq restriced to monsters)

// monsters of specific types to move
vector<cell*> worms, ivies, ghosts, golems, mirrors, mirrors2; 

vector<cell*> temps;  // temporary changes during bfs
vector<eMonster> tempval;  // restore temps

// a bit nicer way of DFS
vector<int> reachedfrom;

// additional direction information for BFS algorithms
// it remembers from where we have got to this location
// the opposite cell will be added to the queue first,
// which helps the AI
vector<cell*> movesofgood[8];

int first7;           // the position of the first monster at distance 7 in dcal

cellwalker cwt; // player character position

bool isIcyLand(cell *c) {
  return c->land == laIce || c->land == laCocytus;
  }

void initcell(cell *c) {
  c->mpdist = INFD;   // minimum distance from the player, ever
  c->cpdist = INFD;   // current distance from the player
  c->pathdist = INFD; // current distance from the player, along paths (used by yetis)
  c->heat = 0;
  c->wall  = waNone;
  c->item  = itNone;
  c->monst = moNone;
  c->bardir = NODIR;
  c->land = laNone;
  c->tmp = -1;
  c->ligon = 0;
  lastexplore = turncount;
  }

// 0 = basic treasure, 1 = something else, 2 = power orb
int itemclass(eItem i) {
  if(i < itKey || i == itFernFlower) return 0;
  if(i == itKey || i == itOrbYendor || i == itGreenStone) return 1;
  return 2;
  }
  
int puregold() {
  return 
    items[itDiamond] + items[itGold] + items[itSpice] + items[itEmerald] + items[itElixir] +
    items[itShard] + items[itBone] + items[itStatue] + items[itHell] + items[itFeather] +
    items[itSapphire] + items[itFernFlower] +
    items[itHyperstone] + items[itOrbYendor] * 50;
  }

eItem treasureType(eLand l) {
  switch(l) {
    case laIce: return itDiamond;
    case laJungle: return itEmerald;
    case laCaves: return itGold;
    case laDesert: return itSpice;

    case laAlchemist: return itElixir;
    case laMirror: return itShard;
    case laMotion: return itFeather;

    case laGraveyard: return itBone;
    case laRlyeh: return itStatue;
    case laDryForest: return itFernFlower;    

    case laHell: return itHell;
    case laCocytus: return itSapphire;
    case laCrossroads: return itHyperstone;
    
    case laNone: return itNone;
    case laBarrier: return itNone;
    case laGameBoard: return itNone;
    }
  return itNone;
  }

eItem localTreasureType() {
  lastland = cwt.c->land;
  return treasureType(lastland);
  }

void countLocalTreasure() {
  eItem i = localTreasureType();
  currentLocalTreasure = i ? items[i] : 0;
  }

bool hellUnlocked() {
  int i = 0;
  for(int t=0; t<ittypes; t++) if(itemclass(eItem(t)) == 0 && items[t] >= 10)
    i++;
  return i >= 9;
  }

bool hyperstonesUnlocked() {
  return
    items[itDiamond] >= 10 &&
    items[itGold] >= 10 &&
    items[itSpice] >= 10 &&
    items[itEmerald] >= 10 &&
    items[itElixir] >= 10 && 
    items[itShard] >= 10 &&
    items[itBone] >= 10 &&
    items[itStatue] >= 10 &&
    items[itHell] >= 10 &&
    items[itFeather] >= 10 &&
    items[itSapphire] >= 10 &&
    items[itFernFlower] >= 10;
  }

int gold() {
  return puregold();
  }

int maxgold() {
  int mg = 0;
  for(int i=itDiamond; i<=itFeather; i++) if(items[i] > mg) mg = items[i];
  return mg;
  }

int tkills() {
  return 
    kills[moYeti] + kills[moWolf] + 
    kills[moRanger] + kills[moTroll] + kills[moGoblin] +
    kills[moWorm] + kills[moDesertman] + kills[moIvyRoot] +
    kills[moMonkey] + kills[moEagle] + kills[moSlime] + kills[moSeep] +
    kills[moRunDog] + 
    kills[moCultist] + kills[moTentacle] + kills[moPyroCultist] + 
    kills[moLesser] + kills[moGreater] + 
    kills[moZombie] + kills[moGhost] + kills[moNecromancer] +
    kills[moHedge] + kills[moFireFairy] +
    kills[moCrystalSage] + kills[moShark] + kills[moGreaterShark];
  }

bool passable(cell *w, cell *from, bool monster_passable, bool mirror_passable) {
  if(w->wall == waFloorA && from && from->wall == waFloorB && !w->item && !from->item)
    return false;
  if(w->wall == waFloorB && from && from->wall == waFloorA && !w->item && !from->item)
    return false;
  if(w->wall == waMirror || w->wall == waCloud) return mirror_passable;
  if(w->wall == waNone || w->wall == waFloorA || w->wall == waFloorB || 
    w->wall == waCavefloor || w->wall == waFrozenLake) {
    if(w->monst) return monster_passable;
    return true;
    }
  return false;
  }

bool cellUnstable(cell *c) {
  return c->land == laMotion && c->wall == waNone;
  }

bool cellUnstableOrChasm(cell *c) {
  return c->land == laMotion && (c->wall == waNone || c->wall == waChasm);
  }

bool player_passable(cell *w, cell *from, bool mon) {
  if(w->wall == waBonfire && items[itOrbWinter]) return true;
  return passable(w, from, mon, true);
  }

// eagles can go through lakes, chasms, and slime
bool eaglepassable(cell *w) {
  if(w->monst) return false;
  return
    w->wall == waNone || w->wall == waFloorA || w->wall == waFloorB || 
    w->wall == waCavefloor || w->wall == waFrozenLake || w->wall == waLake ||
    w->wall == waSulphur || w->wall == waSulphurC || w->wall == waChasm;
  }

bool isActiv(cell *c) {
  return c->wall == waThumper || c->wall == waBonfire;
  }

bool isMimic(cell *c) {
  return c->monst == moMirror || c->monst == moMirage;
  }

bool isFriendly(cell *c) {
  return c->monst == moMirror || c->monst == moMirage || c->monst == moGolem || c->monst == moGolemMoved;
  }

bool isIvy(cell *c) {
  return c->monst == moIvyRoot || c->monst == moIvyHead || c->monst == moIvyBranch || c->monst == moIvyWait ||
    c->monst == moIvyNext || c->monst == moIvyDead;
  }

bool isDemon(cell *c) {
  return c->monst == moLesser || c->monst == moLesserM || 
    c->monst == moGreater || c->monst == moGreaterM;
  }

bool isWorm(cell *c) {
  return c->monst == moWorm || c->monst == moWormtail || c->monst == moWormwait || 
    c->monst == moTentacle || c->monst == moTentacletail || c->monst == moTentaclewait ||
    c->monst == moTentacleEscaping;
  }

void useup(cell *c) {
  c->tmp--;
  if(c->tmp == 0) c->wall = waNone;
  }

bool isInactiveEnemy(cell *w) {
  if(w->monst == moWormtail || w->monst == moWormwait || w->monst == moTentacletail || w->monst == moTentaclewait)
    return true;
  if(w->monst == moLesserM || w->monst == moGreaterM)
    return true;
  if(w->monst == moIvyRoot || w->monst == moIvyWait || w->monst == moIvyNext || w->monst == moIvyDead)
    return true;
  return false;
  }

bool isActiveEnemy(cell *w, cell *killed) {
  if(w->monst == moNone || w == killed) return false;
  if(isFriendly(w)) return false;
  if(isInactiveEnemy(w)) return false;
  if(w->monst == moIvyHead || w->monst == moIvyBranch) {
    while(w != killed && w->mondir != NODIR) w = w->mov[w->mondir];
    return w != killed;
    }
  return true;
  }

bool isArmedEnemy(cell *w, cell *killed) {
  return w->monst != moCrystalSage && isActiveEnemy(w, killed);
  }

bool isKillable(cell *c) {
  return c->monst != moShadow && !isWorm(c) && c->monst != moGreater && c->monst != moGreaterM &&
    c->monst != moHedge;
  }

bool isNeighbor(cell *c1, cell *c2) {
  for(int i=0; i<c1->type; i++) if(c1->mov[i] == c2) return true;
  return false;
  }

// how many monsters are near
eMonster which;

bool mirrorkill(cell *c) {
  for(int t=0; t<c->type; t++) 
    if(c->mov[t] && isMimic(c->mov[t]) && c->mov[t]->mov[c->mov[t]->mondir] == c)
      return true;
  return false;
  }

int monstersnear(cell *c, cell *nocount = NULL, bool shielded = true) {
  int res = 0;
  if(shielded) {
    if(items[itOrbShield] > 1) return 0;
    if(items[itOrbSpeed]  && !(items[itOrbSpeed] & 1)) return 0;
    }
  for(int t=0; t<c->type; t++) 
    if(c->mov[t] && isArmedEnemy(c->mov[t], nocount) && !(nocount && mirrorkill(c->mov[t]))) {
      // do not include stabbed enemies
      if(
        (c->mov[t]->monst == moHedge || (isKillable(c->mov[t]) && items[itOrbThorns]))
        && c->mov[t]->cpdist == 1 && c != cwt.c)
        continue;
      res++, which = c->mov[t]->monst;
      }
  return res;
  }

// reduce c->mpdist to d; also generate the landscape

bool checkBarriersBack(cellwalker& bb);

#define BARLEV (ISANDROID?9:10)
// #define BARLEV 9

bool checkBarriersFront(cellwalker& bb) {
  if(bb.c->mpdist < BARLEV) return false;
  if(bb.c->bardir != NODIR) return false;
  if(bb.spin == 0) return true;

  if(1) for(int i=0; i<7; i++) {
    cellwalker bb2 = bb;
    cwspin(bb2, i); cwstep(bb2); cwspin(bb2, 4); cwstep(bb2);
    if(bb2.c->bardir != NODIR) return false;
    }

  cwstep(bb); cwspin(bb, 3); cwstep(bb); cwspin(bb, 3); cwstep(bb);
  return checkBarriersBack(bb);
  }

bool checkBarriersBack(cellwalker& bb) {
  if(bb.c->mpdist < BARLEV) return false;
  if(bb.c->bardir != NODIR) return false;
  if(bb.spin == 0 && bb.c->mpdist == INFD) return true;
  
  if(1) for(int i=0; i<7; i++) {
    cellwalker bb2 = bb;
    cwspin(bb2, i); cwstep(bb2); cwspin(bb2, 4); cwstep(bb2);
    if(bb2.c->bardir != NODIR) return false;
    }

  cwspin(bb, 3); cwstep(bb); cwspin(bb, 4); cwstep(bb); cwspin(bb, 3); 
  return checkBarriersFront(bb);
  }

void setbarrier(cell *c) {
  c->wall = waBarrier;
  c->land = laBarrier;
  }

void killIvy(cell *c) {
  if(c->monst == moIvyDead) return;
  for(int i=0; i<c->type; i++) if(c->mov[i])
    if(c->mov[i]->mondir == c->spn[i] && isIvy(c->mov[i]))
      killIvy(c->mov[i]);
  c->monst = moIvyDead;
  }

int buildIvy(cell *c, int children, int minleaf) {
  c->mondir = NODIR;
  c->monst = moIvyRoot;
  
  cell *child = NULL;

  int leaf = 0;
  int leafchild = 0;
  for(int i=0; i<c->type; i++) {
    createMov(c, i);
    if(passable(c->mov[i], c, false, false)) {
      if(children && !child) 
        child = c->mov[i], leafchild = buildIvy(c->mov[i], children-1, 5);
      else 
        c->mov[i]->monst = (leaf++) ? moIvyWait : moIvyHead,
        c->mov[i]->mondir = c->spn[i];
      }
    }
  
  leaf += leafchild;
  if(leaf < minleaf) {
    if(child) killIvy(child);
    killIvy(c);
    return 0;
    }
  else return leaf;
  }

bool isIcyWall(cell *c) {
  return c->wall == waNone || c->wall == waIcewall || c->wall == waFrozenLake || c->wall == waLake;
  }

void prespill(cell* c, eWall t, int rad) {
  // these walls block spilling completely
  if(c->wall == waIcewall || c->wall == waBarrier || c->wall == waDeadTroll ||
    c->wall == waDune || c->wall == waBonfire || c->wall == waAncientGrave ||
    c->wall == waFreshGrave || c->wall == waColumn)
    return;
  // these walls block further spilling
  if(c->wall == waCavewall || cellUnstable(c) || c->wall == waSulphur ||
    c->wall == waSulphurC || c->wall == waLake || c->wall == waChasm ||
    c->wall == waDryTree || c->wall == waWetTree || c->wall == waTemporary)
      t = waTemporary;

  if(c->wall == waSulphur) {
    // remove the center as it would not look good
    for(int i=0; i<c->type; i++) if(c->mov[i] && c->mov[i]->wall == waSulphurC)
      c->mov[i]->wall = waSulphur;
    }

  c->wall = t;
  // destroy items...
  c->item = itNone;
  // block spill
  if(t == waTemporary) return;
  // cwt.c->item = itNone;
  if(rad) for(int i=0; i<c->type; i++) if(c->mov[i])
    prespill(c->mov[i], t, rad-1);
  }

void spillfix(cell* c, eWall t, int rad) {
  if(c->wall == waTemporary) c->wall = t;
  if(rad) for(int i=0; i<c->type; i++) if(c->mov[i])
    spillfix(c->mov[i], t, rad-1);
  }

void spill(cell* c, eWall t, int rad) {
  prespill(c,t,rad); spillfix(c,t,rad);
  }

void degradeDemons() {
  addMessage(XLAT("You feel more experienced in demon fighting!"));
  int dcs = size(dcal);
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->monst == moGreaterM || c->monst == moGreater)
      achievement_gain("DEMONSLAYER");
    if(c->monst == moGreaterM) c->monst = moLesserM;
    if(c->monst == moGreater) c->monst = moLesser;
    }  
  }

void ivynext(cell *c);

void killMonster(cell *c) {
  DEB("killmonster");
  if(!c->monst) return;
  if(isWorm(c)) return;
  if(c->monst == moShadow) return;
  if(c->monst == moLesserM) c->monst = moLesser;
  if(c->monst == moGreater) c->monst = moLesser;
  if(c->monst == moGreaterM) c->monst = moLesser;
  kills[c->monst]++;
  

  if(c->monst == moTroll) {
    c->wall = cellUnstableOrChasm(c) ? waChasm : waDeadTroll;
    c->item = itNone;
    for(int i=0; i<c->type; i++) if(c->mov[i]) c->mov[i]->item = itNone;
    }
  if(c->monst == moFireFairy) {
    c->wall = cellUnstableOrChasm(c) ? waChasm : waBonfire;
    c->item = itNone, c->tmp = 50;
    }
  if(c->monst == moSlime) spill(c, c->wall, 2);
  // if(c->monst == moShark) c->heat += 1;
  // if(c->monst == moGreaterShark) c->heat += 10;
  if(c->monst == moCultist) c->heat += 3;
  if(c->monst == moPyroCultist) c->heat += 6;
  if(c->monst == moLesser) c->heat += 10;
  if(c->monst == moLesser && !(kills[c->monst] % 10))
    degradeDemons();
  if(isIvy(c)) {
    eMonster m = c->monst;
    /*if((m == moIvyBranch || m == moIvyHead) && c->mov[c->mondir]->monst == moIvyRoot)
      ivynext(c, moIvyNext); */
    killIvy(c);
    if(m == moIvyBranch || m == moIvyHead || m == moIvyNext) {
      int qty = 0;
      cell *c2 = c->mov[c->mondir];
      for(int i=0; i<c2->type; i++)
        if(c2->mov[i]->monst == moIvyWait && c2->mov[i]->mondir == c2->spn[i])
          qty++;
      if(c->mov[c->mondir]->monst == moIvyRoot || qty) {
        c->monst = moIvyNext;
        /* c->monst = moIvyHead;
        ivynext(c);
        if(c->monst == moIvyHead) c->monst = moIvyNext;
        else c->monst = moNone; */
        }
      else {
        c->mov[c->mondir]->monst = moIvyHead;
        }
      }
    }
  else c->monst = moNone;

  }

bool orbChance(cell *c, eLand usual, int chthere, int chcross) {
  if(c->land == usual) return rand() % chthere == 0;
  if(chcross && c->land == laCrossroads) {
    chcross = (chcross / 50) * (50 + items[itHyperstone]);
    return rand() % chcross == 0;
    }
  return false;
  }

void buildBarrier(cell *c) {
  if(c->wall == waBarrier) return;
  if(c->mpdist == INFD) return;

  cellwalker bb(c, c->bardir); setbarrier(bb.c);
  cwstep(bb); setbarrier(bb.c);
  cwspin(bb, 2); cwstep(bb); bb.c->land = c->barleft; cwstep(bb);
  cwspin(bb, 2); cwstep(bb); bb.c->land = c->barright; cwstep(bb);
  cwspin(bb, 2); 
  
  cwspin(bb, 3); cwstep(bb); setbarrier(bb.c);
  cwspin(bb, 3); cwstep(bb);
  
  bb.c->bardir = bb.spin;
  bb.c->barleft = c->barright;
  bb.c->barright = c->barleft;
  buildBarrier(c);
  
  for(int a=-3; a<=3; a++) if(a) {
    bb.c = c; bb.spin = c->bardir; cwspin(bb, a); cwstep(bb); 
    bb.c->land = a > 0 ? c->barright : c->barleft;
    }

  bb.c = c; bb.spin = c->bardir;
  cwspin(bb, 3); cwstep(bb); cwspin(bb, 4); bb.c->land = c->barright; cwstep(bb); cwspin(bb, 3);
  bb.c->bardir = bb.spin;
  bb.c->barleft = c->barright;
  bb.c->barright = c->barleft;
  buildBarrier(c);
  }

void chasmify(cell *c) {
  c->wall = waChasm; c->item = itNone;
  int q = 0;
  cell *c2[10];
  for(int i=0; i<c->type; i++) if(c->mov[i] && c->mov[i]->mpdist > c->mpdist && cellUnstable(c->mov[i]))
    c2[q++] = c->mov[i];
  if(q) {
    cell *c3 = c2[rand() % q];
    c3->wall = waChasmD;
    }
  }

eLand getNewLand(eLand old) {
  eLand tab[32];
  int cnt = 0;
  
  // return (rand() % 2) ? laMotion : laJungle;

  // the basic lands, always available
  tab[cnt++] = laCrossroads;
  tab[cnt++] = laIce;
  tab[cnt++] = laDesert;
  tab[cnt++] = laJungle;
  tab[cnt++] = laCaves;
  
  // the advanced lands
  if(gold() >= 30) {
    tab[cnt++] = laCrossroads;
    tab[cnt++] = laAlchemist;
    tab[cnt++] = laMirror;
    tab[cnt++] = laMotion;
    }
  
  if(gold() >= 60) {
    tab[cnt++] = laCrossroads;
    tab[cnt++] = laRlyeh;
    tab[cnt++] = laDryForest;
    }
  
  if(tkills() >= 100) {
    tab[cnt++] = laGraveyard;
    }
  
  if(hellUnlocked()) {
    tab[cnt++] = laCrossroads;
    tab[cnt++] = laHell;
    }
  
  if(items[itHell] >= 10)
    tab[cnt++] = laCocytus;
  
  eLand n = old;
  while(n == old) n = tab[rand() % cnt];
  
  return n;  
  }

bool notDippingFor(eItem i) {
  int v = items[i] - currentLocalTreasure;
  if(v <= 10) return true;
  if(v >= 20) return false;
  return v >= rand() % 10 + 10;
  }

eLand euland[65536];

eLand switchable(eLand nearland, eLand farland, eucoord c) {
  if(nearland == laCrossroads) {
    if(rand() % 4 == 0 && (short(c)%3==0))
      return laBarrier;
    return laCrossroads;
    }
  else if(nearland == laBarrier) {
    return getNewLand(farland);
    }
  else {
    if(rand() % 20 == 0 && (short(c)%3==0))
      return laBarrier;
    return nearland;
    }
  }

eLand getEuclidLand(eucoord c) {
  if(euland[c]) return euland[c];
  if(c == 0 || c == eucoord(-1) || c == 1)
    return euland[c] = laCrossroads;
  if(euland[eucoord(c-2)] && ! euland[eucoord(c-1)]) getEuclidLand(c-1);
  if(euland[eucoord(c+2)] && ! euland[eucoord(c+1)]) getEuclidLand(c+1);
  if(euland[eucoord(c-1)]) return 
    euland[c] = switchable(euland[c-1], euland[eucoord(c-2)], c);
  if(euland[eucoord(c+1)]) return 
    euland[c] = switchable(euland[c+1], euland[eucoord(c+2)], c);
  return euland[c] = laCrossroads;
  }

// This function generates all lands. Warning: it's very long!

void setdist(cell *c, int d, cell *from) {
  if(signed(c->mpdist) <= d) return;
  c->mpdist = d;
  
  if(d >= BARLEV) {
    if(!c->land) c->land = from->land;

    if(euclid) {
      c->land = euclidland;
      if(euclidland == laCrossroads) {
        eucoord x, y;
        decodeMaster(c->master, x, y);
        c->land = getEuclidLand(y+2*x);
        }
      }
    }
  
  if(d == BARLEV && !euclid) {
    
    if(c->type == 7 && rand() % 10000 < (showoff ? (cwt.c->mpdist > 7 ? 0 : 10000) : c->land == laCrossroads ? 5000 : 50) &&
      c->land != laGameBoard) {
    
      int bd = 2 + (rand() % 2) * 3;
      
      cellwalker bb(c, bd);
      cellwalker bb2 = bb;

      if(checkBarriersFront(bb) && checkBarriersBack(bb2)) {
        c->bardir = bd;
        eLand oldland = c->land;
        eLand newland = getNewLand(oldland);
        if(showoff) newland = showlist[(showid++) % 10];
        landcount[newland]++;

        if(bd == 5) c->barleft = oldland, c->barright = newland;
        else c->barleft = newland, c->barright = oldland;
        }
      }
    
    if(c->bardir != NODIR) 
      buildBarrier(c);    
    }
  
  if(d < 10) {
    explore[d]++;
    exploreland[d][c->land]++;
    
    if(d < BARLEV)
    for(int i=0; i<c->type; i++) {
      createMov(c, i);
      setdist(c->mov[i], d+1, c);
      }
    
    if(d == 9) {
    
      if(c->land == laIce) if(rand() % 100 < 5 && c->wall != waBarrier) {
        c->wall = waIcewall;
        for(int i=0; i<c->type; i++) if(rand() % 100 < 50) {
          createMov(c, i);
          setdist(c->mov[i], d+1, c);
          cell *c2 = c->mov[i];
          if(c2->wall == waBarrier || c2->land != laIce) continue;
          c2->wall = waIcewall;
          for(int j=0; j<c2->type; j++) if(rand() % 100 < 20) {
            createMov(c2, j);
            setdist(c->mov[i], d+2, c);
            cell *c3 = c2->mov[j];
            if(c3->wall == waBarrier || c3->land != laIce) continue;
            c3->wall = waIcewall;
            }
          }
        }

      if(c->land == laIce || c->land == laCocytus) if(c->wall == waIcewall && items[itDiamond] >= 5 && rand() % 200 == 1)
        c->wall = waBonfire, c->tmp = -1;;

      if(c->land == laCaves) 
        c->wall = rand() % 100 < 55 ? waCavewall : waCavefloor;
      
      if(c->land == laAlchemist) 
        c->wall = rand() % 2 ? waFloorA : waFloorB;
      
      if(c->land == laDryForest) 
        c->wall = rand() % 100 < 50 ? (rand() % 100 < 50 ? waDryTree : waWetTree) : 
          waNone;
      
      if(c->land == laGraveyard && ishept(c)) 
        c->wall = rand() % 5 ? waAncientGrave : waFreshGrave;
      
      if(c->land == laRlyeh)  {
        if(rand() % 500 < 5) {
          for(int i=0; i<c->type; i++) {
            createMov(c, i);
            setdist(c->mov[i], d+1, c);
            if(c->mov[i] && c->mov[i]->land == laRlyeh)
              c->mov[i]->wall = waColumn;
            }
          
          for(int j=0; j<2; j++) {
            int i = rand() % c->type;
            if(c->mov[i] && c->mov[i]->land == laRlyeh)
              c->mov[i]->wall = waNone;
            }
          }
        if(ishept(c) && rand() % 2) c->wall = waColumn;
        }
      
      if(c->land == laHell) {
        if(rand() % 100 < 4) {
          for(int i=0; i<c->type; i++) {
            createMov(c, i);
            setdist(c->mov[i], d+1, c);
            if(c->mov[i] && c->mov[i]->land == laHell)
              if(c->mov[i]->wall != waSulphurC)
                c->mov[i]->wall = waSulphur;
            }
          
          c->wall = waSulphurC;
          }
        }
      
      if(c->land == laCocytus)  {
        if(c->wall == waNone) c->wall = waFrozenLake;
        if(rand() % 100 < 5) {
          for(int i=0; i<c->type; i++) {
            createMov(c, i);
            setdist(c->mov[i], d+1, c);
            if(c->mov[i] && c->mov[i]->land == laCocytus)
              c->mov[i]->wall = waLake;
            }
          
          c->wall = waLake;

          if(rand() % 500 < 100 + 2 * (items[itSapphire]))
            c->monst = moShark;
          }
        }
      
      if(c->land == laDesert) {
        if(rand() % 100 < 5) {
          for(int i=0; i<c->type; i++) {
            createMov(c, i);
            setdist(c->mov[i], d+1, c);
            if(c->mov[i] && c->mov[i]->land == laDesert)
              c->mov[i]->wall = waDune;
            }
          
          for(int j=0; j<2; j++) {
            int i = rand() % c->type;
            if(c->mov[i] && c->mov[i]->land == laDesert)
              c->mov[i]->wall = waNone;
            }
          }

        if(rand() % 300 == 1 && items[itSpice] >= 5) 
          c->wall = waThumper, c->tmp = -1;
        }
      }
    
    if(d == 7 && c->wall == waCavewall && rand() % 5000 < items[itGold])
      c->monst = moSeep;
    
    // repair the buggy walls flowing in from another land, like ice walls flowing into the Caves
    if(d == 7 && c->land == laCaves && c->wall != waCavewall && c->wall != waCavefloor)
      c->wall = waCavefloor;
    
    if(d == 7 && c->land == laCocytus && c->wall != waFrozenLake && c->wall != waLake && c->wall != waIcewall)
      c->wall = waFrozenLake;
    
    if(d == 7 && c->land == laAlchemist && c->wall != waFloorA && c->wall != waFloorB)
      c->wall = waFloorA;
    
    if(d == 7 && c->wall == waIcewall && c->land != laIce && c->land != laCocytus)
      c->wall = waNone;
    
    if(d == 7 && c->wall == waChasmD) {
      chasmify(c);
      }
    
    if(d == 7 && passable(c, NULL, false, false) && !safety) {
      int hard = items[itOrbYendor] * 5;
      
      if(c->land == laBarrier) c->wall = waBarrier;
      if(c->land == laIce) {
        if(rand() % 5000 < 100 + 2 * (kills[moYeti] + kills[moWolf]) && notDippingFor(itDiamond))
          c->item = itDiamond;
        if(rand() % 8000 < 2 * (items[itDiamond] + hard))
          c->monst = rand() % 2 ? moYeti : moWolf;
        }
      if(c->land == laCaves) {
        if(rand() % 5000 < 100 + 2 * (kills[moTroll] + kills[moGoblin]) && notDippingFor(itGold))
          c->item = itGold;
        if(rand() % 8000 < 10 + 2 * (items[itGold] + hard))
          c->monst = rand() % 2 ? moTroll : moGoblin;
        }
      if(c->land == laDesert) {
        if(rand() % 5000 < 100 + 2 * (kills[moWorm] + kills[moDesertman]) && notDippingFor(itSpice))
          c->item = itSpice;
        if(rand() % 8000 < 10 + 2 * (items[itSpice] + hard))
          c->monst = rand() % 2 ? moWorm : moDesertman,
          c->mondir = NODIR;
        }
      if(c->land == laJungle) {
        if(rand() % 5000 < 25 + 2 * (kills[moIvyRoot] + kills[moMonkey]) && notDippingFor(itEmerald))
          c->item = itEmerald;
        if(rand() % 15000 < 5 + 1 * (items[itEmerald] + hard))
          c->monst = moMonkey;
        else if(rand() % 80000 < 5 + items[itEmerald] + hard)
          c->monst = moEagle;
        else if(ishept(c) && rand() % 4000 < 300 + items[itEmerald]) {
          bool hard = rand() % 100 < 25;
          if(hard ? buildIvy(c, 1, 9) : buildIvy(c, 0, c->type))
            c->item = itEmerald;
          }
        }
      if(c->land == laAlchemist) {
        if(rand() % 5000 < 25 + min(kills[moSlime], 200) && notDippingFor(itElixir))
          c->item = itElixir;
        else if(rand() % 3500 < 10 + items[itElixir] + hard)
          c->monst = moSlime;
        }
      if(c->land == laCrossroads) {
        if(c->type == 6 && rand() % 8000 < 120 && items[itShard] >= 10)
          c->wall = rand() % 2 ? waMirror : waCloud;
        else if(hyperstonesUnlocked() && rand() % 5000 < tkills() && notDippingFor(itHyperstone))
          c->item = itHyperstone;
        else if(rand() % 4000 < items[itHyperstone]) {
          // only interesting monsters here!
          static eMonster m[11] = {
            moWorm, moTroll, moEagle,
            moLesser, moGreater, moPyroCultist, moGhost,
            moFireFairy, moIvyRoot, moTentacle, moHedge
            };
          eMonster cm = m[rand() % 11];
          if(cm == moIvyRoot) buildIvy(c, 0, c->type);
          else c->monst = cm;
          if(cm == moWorm || cm == moTentacle)
            c->mondir = NODIR;
          }
        }
      if(c->land == laMirror) {
        if(c->type == 6 && rand() % 5000 < 120 && notDippingFor(itShard))
          c->wall = rand() % 2 ? waMirror : waCloud;
        if(rand() % 12000 < 8 + items[itShard] + hard)
          c->monst = moRanger;
        else if(rand() % 60000 < 8 + items[itShard] + hard)
          c->monst = moEagle;
        }
      if(c->land == laGraveyard) {
        if(rand() % 5000 < 30 + 2 * (kills[moZombie] + kills[moGhost] + kills[moNecromancer]) && notDippingFor(itBone))
          c->item = itBone;
        if(rand() % 20000 < 10 + items[itBone] + hard + (kills[moZombie] + kills[moGhost] + kills[moNecromancer])/60) {
          eMonster grm[6] = { moZombie, moZombie, moZombie, moGhost, moGhost, moNecromancer};
          c->monst = grm[rand() % 6];
          }
        }
      if(c->land == laRlyeh) {
        if(rand() % 5000 < 30 + 2 * (kills[moCultist] + kills[moTentacle] + kills[moPyroCultist]) && notDippingFor(itStatue))
          c->item = itStatue;
        if(rand() % 8000 < 5 + items[itStatue] + hard)
          c->monst = moTentacle, c->item = itStatue, c->mondir = NODIR;
        else if(rand() % 12000 < 5 + items[itStatue] + hard)
          c->monst = rand() % 3 ? moCultist : moPyroCultist;
        else if(rand() % 8000 < 5 + items[itStatue] + hard && c->type == 6) {
          for(int t=0; t<c->type; t++) {
            if(c->mov[t] && c->mov[t]->monst == moNone)
              c->mov[t]->wall = ishept(c->mov[t]) ? waColumn : waNone;
            if(c->mov[t]->wall == waColumn)
              c->mov[t]->item = itNone;
            }
          if(buildIvy(c, 0, 3)) c->item = itStatue;
          }
        }
      if(c->land == laDryForest) {
        if(rand() % 5000 < 100 + 2 * (kills[moFireFairy]*2 + kills[moHedge]) && notDippingFor(itFernFlower))
          c->item = itFernFlower;
        if(rand() % 4000 < 40 + items[itFernFlower] + hard)
          c->monst = moHedge;
        else if(rand() % 8000 < 2 * items[itFernFlower] + hard)
          c->monst = moFireFairy;
        }
      if(c->land == laHell) {
        if(rand() % 1500 < 30 + (kills[moCultist] + kills[moTentacle]) && notDippingFor(itHell))
          c->item = itHell;
        if(rand() % 8000 < 40 + items[itHell] + hard)
          c->monst = moLesser;
        else if(rand() % 24000 < 40 + items[itHell] + hard)
          c->monst = moGreater;
        }
      if(c->land == laCocytus) {
        if(rand() % 5000 < 100 + 2 * (kills[moShark] + kills[moGreaterShark] + kills[moCrystalSage]) && notDippingFor(itSapphire))
          c->item = itSapphire;
        if(rand() % 5000 < 2 * (items[itSapphire] + hard)) {
          eMonster ms[3] = { moYeti, moGreaterShark, moCrystalSage };
          c->monst = ms[rand() % 3];
          if(c->monst == moGreaterShark) c->wall = waLake;
          }
        }
      if(c->land == laMotion) {
        if(rand() % 1500 < 30 + (kills[moRunDog]) && notDippingFor(itFeather))
          c->item = itFeather;
        if(rand() % 20000 < 25 + items[itFeather] + hard) {
          c->monst = moRunDog;
          chasmify(c);
          }
        }
      if(!c->item && c->wall != waCloud && c->wall != waMirror) {
        if(orbChance(c, laJungle, 1200, 500) && items[itEmerald] >= 10)
          c->item = itOrbLightning;
        if(orbChance(c, laIce, 2000, 500) && items[itDiamond] >= 10)
          c->item = itOrbFlash;
        if(orbChance(c, laCaves, 1800, 2000) && items[itGold] >= 10)
          c->item = itOrbLife;
        if(orbChance(c, laAlchemist, 500, 800) && items[itElixir] >= 10)
          c->item = itOrbSpeed;
        if(orbChance(c, laGraveyard, 200, 200) && items[itBone] >= 10)
          c->item = itGreenStone;
        if(orbChance(c, laDesert, 2500, 600) && items[itSpice] >= 10)
          c->item = itOrbShield;
        if(orbChance(c, laHell, 2000, 1000) && items[itHell] >= 10)
          c->item = itOrbYendor;
        if(orbChance(c, laRlyeh, 1500, 1000) && items[itStatue] >= 10)
          c->item = itOrbTeleport;
        if(orbChance(c, laMotion, 2000, 1000) && items[itFeather] >= 10) {
          c->item = itOrbSafety;
          }

        if(orbChance(c, laIce, 1500, 0) && items[itDiamond] >= 10)
          c->item = itOrbWinter;
        if(orbChance(c, laDryForest, 2500, 0) && items[itFernFlower] >= 10)
          c->item = itOrbWinter;
        if(orbChance(c, laCocytus, 1500, 1500) && items[itSapphire] >= 10)
          c->item = itOrbWinter;
        if(orbChance(c, laCaves, 1200, 0) && items[itGold] >= 10)
          c->item = itOrbDigging;
        if(orbChance(c, laDryForest, 500, 2000) && items[itFernFlower] >= 10)
          c->item = itOrbThorns;
        }
      }

    }
  }

// find worms and ivies
void settemp(cell *c) {
  temps.push_back(c); tempval.push_back(c->monst); c->monst = moNone;
  }

void findWormIvy(cell *c) {
  while(true) {
    if(c->monst == moWorm || c->monst == moTentacle || c->monst == moWormwait || c->monst == moTentaclewait ||
      c->monst == moTentacleEscaping) {
      worms.push_back(c); settemp(c);
      break;
      }
    else if(c->monst == moWormtail) {
      bool bug = true;
      for(int i=0; i<c->type; i++) {
        cell* c2 = c->mov[i];
        if(c2 && isWorm(c2) && c2->mov[c2->mondir] == c) {
          settemp(c);
          c = c2;
          bug = false;
          }
        }
      if(bug) break;
      }
    else if(c->monst == moIvyWait) {
      cell* c2 = c->mov[c->mondir];
      settemp(c); c=c2;
      }
    else if(c->monst == moIvyHead) {
      ivies.push_back(c); settemp(c);
      break;
      }
    else if(c->monst == moIvyBranch || c->monst == moIvyRoot) {
      bool bug = true;
      for(int i=0; i<c->type; i++) {
        cell* c2 = c->mov[i];
        if(c2 && (c2->monst == moIvyHead || c2->monst == moIvyBranch) && c2->mov[c2->mondir] == c) {
          settemp(c);
          c = c2;
          bug = false;
          }
        }
      if(bug) break;
      }
    else break;
    }
  }

// calculate cpdist and pathdist
void bfs() {
  
  int dcs = size(dcal);
  for(int i=0; i<dcs; i++) dcal[i]->cpdist = INFD;
  worms.clear(); ivies.clear(); ghosts.clear(); golems.clear(); mirrors.clear();
  temps.clear(); tempval.clear();
  
  dcal.clear(); dcal.push_back(cwt.c);
  reachedfrom.clear(); reachedfrom.push_back(rand() % cwt.c->type);
  
  cwt.c->cpdist = 0;
  int qb = 0;
  while(true) {
    int i, fd = reachedfrom[qb] + 3;
    cell *c = dcal[qb++];
    int d = c->cpdist;
    if(d == 7) { first7 = qb; break; }
    for(int j=0; j<c->type; j++) if(i = (fd+j) % c->type, c->mov[i]) {
      // printf("i=%d cd=%d\n", i, c->mov[i]->cpdist);
      cell *c2 = c->mov[i];
      if(c2 && signed(c2->cpdist) > d+1) {
        c2->cpdist = d+1;
        
        // remove treasures
        if(c2->item && c2->cpdist == 7 && itemclass(c2->item) == 0 &&
          items[c2->item] >= 20 + currentLocalTreasure)
            c2->item = itNone;
        
        c2->ligon = 0;
        dcal.push_back(c2);
        reachedfrom.push_back(c->spn[i]);
        if(c2->monst) {
          if(isWorm(c2) || isIvy(c2)) findWormIvy(c2);
          else if(isMimic(c2)) mirrors.push_back(c2);
          else if(c2->monst == moGhost || c2->monst == moGreaterShark) ghosts.push_back(c2);
          else if(c2->monst == moGolem) golems.push_back(c2);
          }
        if(c2->wall == waThumper && c2->tmp > 0) {
          useup(c2);
          c2->pathdist = 1; pathq.push_back(c2);
          }
        }
      }
    }

  int qtemp = size(temps);
  for(int i=0; i<qtemp; i++) temps[i]->monst = tempval[i];
  
  int pqs = size(pathq);
  for(int i=0; i<pqs; i++) pathq[i]->pathdist = INFD;
  pathq.clear();  pathq.push_back(cwt.c);
  pathqm.clear();  
  cwt.c->pathdist = 0;
  reachedfrom.clear(); reachedfrom.push_back(rand() % cwt.c->type);

  qb = 0;
  for(qb=0; qb < size(pathq); qb++) {
    int fd = reachedfrom[qb] + 3;
    cell *c = pathq[qb];
    eucoord x, y;
    decodeMaster(c->master, x, y);
    if(c->monst) { 
      pathqm.push_back(c); 
      continue; // no paths going through monsters
      }
    if(c->cpdist > 7) continue;
    int d = c->pathdist;
    for(int j=0; j<c->type; j++) {
      int i = (fd+j) % c->type; 
      // printf("i=%d cd=%d\n", i, c->mov[i]->cpdist);
      if(c->mov[i] && c->mov[i]->pathdist == INFD && 
        (c->mov[i]->monst || passable(c->mov[i], d==0?NULL:c, true, false))) {
        c->mov[i]->pathdist = d+1;
        pathq.push_back(c->mov[i]); reachedfrom.push_back(c->spn[i]);
        }
      }
    }
  }

#define SCSIZE 65
#define SCSIZE2 13

struct score {
  string ver;
  int tab[SCSIZE];
  int tab2[SCSIZE2];
  };

extern void cleargraphmemory();

// initialize the game
void initgame() {
  cwt.c = origin.c7; cwt.spin = 0;
  cwt.c->land = euclid ? euclidland : firstland;
  createMov(cwt.c, 0);

  for(int i=0; i<65536; i++) euland[i] = laNone;
  
  // extern int sightrange; sightrange = 9;
  // cwt.c->land = laHell; items[itHell] = 10;
  for(int i=9; i>=0; i--) {
    setdist(cwt.c, i, NULL);
    verifycells(&origin);
    }
  if(cwt.c->land == laCocytus)
    cwt.c->wall = waFrozenLake;
  else if(cwt.c->land == laAlchemist || cwt.c->land == laGameBoard)
    ;
  else if(cwt.c->land == laCaves)
    cwt.c->wall = waCavefloor;
  else
    cwt.c->wall = waNone;
  cwt.c->item = itNone;
  cwt.c->monst = moNone;

  cleargraphmemory();
  
  if(!safety) {
    timerstart = time(NULL); turncount = 0; sagephase = 0;
    timerstopped = false;
    savecount = 0; savetime = 0;
    cheater = 0;
    if(firstland != laIce) cheater++;
    }
  else safety = false;
  
  // items[itGreenStone] = 100;
  // items[itOrbTeleport] = 100;
  
  /*
  items[itGold] = 20;
  items[itDiamond] = 20;
  items[itSpice] = 20;
  items[itEmerald] = 20;
  items[itElixir] = 20;
  */
  
  /*
  items[itOrbShield]    = 100;
  items[itOrbSpeed]     = 100;
  items[itOrbWinter]    = 100;
  items[itOrbLightning] = 100;
  */
  
  // items[itOrbLightning]    = 100;
  // items[itEmerald]      = 100;
  
  // items[itOrbWinter]    = 1000;
  
  bfs();
  }

void moveNormal(cell *c) {
  bool repeat = true;
  eMonster m = c->monst;
  int nc = 0;
  
  again:

  for(int j=0; j<c->type; j++) 
    if(c->mov[j] && isFriendly(c->mov[j]) && repeat) {
      // XLATC enemy destroys a friend
      addMessage(XLAT("The %1 destroys the %2!", m, c->mov[j]->monst));
      c->mov[j]->monst = moNone;
      return;
      }

  cell *posdest[10];
  
  for(int j=0; j<c->type; j++)
    if(c->mov[j] && c->mov[j]->pathdist < c->pathdist && passable(c->mov[j], c, false, false)) 
      // crystal sages can't stand out of Cocytus
      if(m == moCrystalSage ? (c->mov[j]->land == laCocytus && c->mov[j]->heat <= SAGEMELT) : true)
//    if(m == moNecromancer ? c->mov[j]->land == laGraveyard : true)
//    it is more fun when demons step into the Land of Eternal Motion, IMO
//    if((m == moLesser || m == moGreater) ? c->mov[j]->land != laMotion : true)
      if(c->mov[j]->cpdist > 0)
        posdest[nc++] = c->mov[j];
      
/*
  // Eagles can fly through chasms/slime if needed
  if(m == moEagle && nc == 0) {
    printf("no eagle no fly\n");
    for(int j=0; j<c->type; j++) printf("%d from %d\n", c->mov[j]->cpdist, c->cpdist);
    for(int j=0; j<c->type; j++)
      if(c->mov[j] && c->mov[j]->cpdist < c->cpdist && 
        !c->mov[j]->monst && 
        (c->mov[j]->land == laMotion || c->mov[j]->land == laAlchemist) && c->wall != waDeadTroll)
        if(c->mov[j]->cpdist > 0)
          posdest[nc++] = c->mov[j];
    printf("nc=%d\n", nc);
    } */

  if(!nc) return;  
  nc = rand() % nc;
  cell *c2 = posdest[nc];
  c2->monst = m, c->monst = moNone;
  if(m == moEagle && repeat && c->pathdist > 1) { repeat = false; c = c2; goto again; }
  }

void explodeAround(cell *c) {
  for(int j=0; j<c->type; j++) {
    cell* c2 = c->mov[j];
    if(c2) {
      c2->heat += 0.5;
      if((c2->wall == waDune || c2->wall == waIcewall ||
        c2->wall == waAncientGrave || c2->wall == waFreshGrave || 
        c2->wall == waColumn || c2->wall == waThumper || c2->wall == waBonfire ||
        c2->wall == waDryTree || c2->wall == waWetTree))
        c2->wall = waNone;
      if(c2->wall == waCavewall || c2->wall == waDeadTroll) c2->wall = waCavefloor;
      }
    }
  }

void moveWorm(cell *c) {

  if(c->monst == moWormwait) { c->monst = moWorm; return; }
  else if(c->monst == moTentaclewait) { c->monst = moTentacle; return; }
  else if(c->monst == moTentacleEscaping) {    
    // explodeAround(c);
    c->monst = moNone;
    if(c->mondir != NODIR) c->mov[c->mondir]->monst = moTentacleEscaping;
    return;
    }
  else if(c->monst != moWorm && c->monst != moTentacle) return;

  int ncg = 0, ncb = 0;
  cell *gmov[7], *bmov[7];
  
  int id = c->monst - moWorm;
  
  for(int j=0; j<c->type; j++) {
    if(c->mov[j] && isFriendly(c->mov[j])) {
      addMessage(XLAT("The %1 eats the %2!", c->monst, c->mov[j]->monst));
      ncg = 1; gmov[0] = c->mov[j];
      break;
      }
    if(c->mov[j] && passable(c->mov[j], c, false, false) && !cellUnstable(c->mov[j]) && c->mov[j] != cwt.c) {
      if(c->mov[j]->pathdist < c->pathdist) gmov[ncg++] = c->mov[j]; else bmov[ncb++] = c->mov[j];
      }
    }

  if(ncg == 0 && ncb == 0) {
    int spices = 0;
    if(id) {
      addMessage(XLAT("Cthulhu withdraws his tentacle!"));
      kills[moTentacle]++;
      c->monst = moTentacleEscaping;
      moveWorm(c);
      }
    else {
      addMessage(XLAT("The sandworm explodes in a cloud of Spice!"));
      kills[moWorm]++;
      spices = 3;
      }
    while(c->monst == moWorm || c->monst == moWormtail || c->monst == moTentacle || c->monst == moTentacletail) {
      if(id) explodeAround(c);
      if(spices > 0 && c->land == laDesert) c->item = itSpice, spices--;
      c->monst = moNone;
      if(c->mondir != NODIR) c = c->mov[c->mondir];
      }
    return;
    }
  
  cell* goal;
  if(ncg) goal = gmov[rand() % ncg];
  else goal = bmov[rand() % ncb];
  
  for(int j=0; j<c->type; j++) if(c->mov[j] == goal) {
    goal->monst = eMonster(moWormwait + id);
      
    c->monst = eMonster(moWormtail + id);
    goal->mondir = c->spn[j];
    
    if(id) break;
    
    cell *c2 = c, *c3 = c2;
    for(int a=0; a<15; a++)
      if(c2->monst == moWormtail) {
        if(c2->mondir == NODIR) return;
        c3 = c2, c2 = c3->mov[c2->mondir];
        }
    
    if(c2->monst == moWormtail) c2->monst = moNone, c3->mondir = NODIR;
    }

  }

void ivynext(cell *c) {
  cellwalker cw(c, c->mondir);
  cw.c->monst = moIvyWait;
  bool findleaf = false;
  while(true) {
    cwspin(cw, 1);
    if(cw.spin == signed(cw.c->mondir)) {
      if(findleaf) { 
        cw.c->monst = moIvyHead; break;
        }
      cw.c->monst = moIvyWait;
      cwstep(cw);
      continue;
      }
    cwstep(cw);
    if(cw.c->monst == moIvyWait && signed(cw.c->mondir) == cw.spin) {
      cw.c->monst = moIvyBranch;
      findleaf = true; continue;
      }
    cwstep(cw);
    }
  }

void moveivy() {
  for(int i=0; i<size(ivies); i++) {
    cell *c = ivies[i];
    cell *co = c;
    if(c->monst != moIvyHead) continue;
    ivynext(c);

    cell *mto = NULL;
    int pd = c->pathdist;
    int sp = 0;
    
    while(c->monst != moIvyRoot) {
      for(int j=0; j<c->type; j++) {
        if(c->mov[j] && isFriendly(c->mov[j])) {
          addMessage(XLAT("The ivy destroys the %1!", c->mov[j]->monst));
          c->mov[j]->monst = moNone;
          continue;
          }
        if(c->mov[j] && signed(c->mov[j]->pathdist) < pd && passable(c->mov[j], c, false, false))
          mto = c->mov[j], pd = mto->pathdist, sp = c->spn[j];
        }
      c = c->mov[c->mondir];
      }

    if(mto && mto->cpdist) {        
      mto->monst = moIvyWait, mto->mondir = sp;
      // if this is the only branch, we want to move the head immediately to mto instead
      if(mto->mov[mto->mondir]->monst == moIvyHead) {
        mto->monst = moIvyHead; co->monst = moIvyBranch;
        }
      }
    else if(co->mov[co->mondir]->monst != moIvyRoot) {
      // shrink useless branches, but do not remove them completely (at the root)
      if(co->monst == moIvyHead) co->mov[co->mondir]->monst = moIvyHead;
      co->monst = moNone;
      }
    }
  }

// move slimes, and also seeps

int sval = 1;
vector<cell*> slimedfs;

void slimevisit(cell *c, cell *from, int u) {
  if(!c) return;
  if(eq(c->tmp, sval)) return;
  if(c->land != laAlchemist && c->wall != waCavewall && c->wall != waLake &&
    c->wall != waFloorA && c->wall != waFloorB) return;
  if(c->wall != from->wall && from != cwt.c) return;
  if(c->item) return;
  if(c->wall == waThumper || c->wall == waBonfire) return;
  c->tmp = sval;
  if(size(slimedfs) < 1000) slimedfs.push_back(c), reachedfrom.push_back(u);
  if(!isWorm(c) && !isIvy(c))
    for(int i=0; i<c->type; i++) if(c->mov[i] == from)
      c->mondir = i;
  if(c->monst == moSlime || c->monst == moSeep || c->monst == moShark) {

    for(int j=0; j<c->type; j++) 
      if(c->mov[j] && isFriendly(c->mov[j])) {
        // XLATC slime/seep/shark
        addMessage(XLAT("The %1 eats the %2!", c->monst, c->mov[j]->monst));
        c->mov[j]->monst = moNone;
        return;
        }
    
    if(from->cpdist == 0 || from->monst) return;

    from->monst = c->monst, c->monst = moNone;
    }
  }

void moveslimes() {
  sval++;
  slimedfs.clear(); slimedfs.push_back(cwt.c);
  reachedfrom.clear(); reachedfrom.push_back(rand() % cwt.c->type);
  for(int i=0; i<size(slimedfs); i++) {
    cell *c = slimedfs[i];
    int j = reachedfrom[i];
    for(int t=0; t<c->type; t++) {
      int u = (j+t) % c->type;
      slimevisit(c->mov[u], c, c->spn[u]);
      }
    }
  }

// move eagles

vector<cell*> eagledfs;

void eaglevisit(cell *c, cell *from, int id) {
  if(!c) return;
  if(eq(c->tmp, sval)) return;
  if(c->wall == waThumper || c->wall == waBonfire) return;
  c->tmp = sval;
  if(c->monst == moEagle) {
    if(id == 1) for(int j=0; j<c->type; j++) 
      if(c->mov[j] && isFriendly(c->mov[j])) {
        // XLATC eagle
        addMessage(XLAT("The %1 claws the %2!", c->monst, c->mov[j]->monst));
        c->mov[j]->monst = moNone;
        return;
        }
    
    if(from->cpdist == 0 || from->monst) return;

    from->monst = c->monst, c->monst = moNone;
    }
  if(!eaglepassable(c)) return;
  if(size(eagledfs) < 1000) eagledfs.push_back(c);
  }

void moveeagles(int id) {
  sval++;
  eagledfs.clear();
  eagledfs.push_back(cwt.c);
  for(int i=0; i<size(eagledfs); i++) {
    cell *c = eagledfs[i];
    for(int t=0; t<c->type; t++)
      eaglevisit(c->mov[t], c, id);
    }
  }

#define SHSIZE 16

cell *shpos[SHSIZE];
int cshpos = 0;

void clearshadow() {
  for(int i=0; i<SHSIZE; i++) shpos[i] = NULL;
  }

void moveshadow() {
  if(shpos[cshpos] && shpos[cshpos]->monst == moShadow)
    shpos[cshpos]->monst = moNone;
  shpos[cshpos] = cwt.c;
  cshpos = (cshpos+1) % SHSIZE;
  if(shpos[cshpos] && shpos[cshpos]->monst == moNone && shpos[cshpos]->cpdist && shpos[cshpos]->land == laGraveyard)
    shpos[cshpos]->monst = moShadow;
  }

void moveghosts() {

  for(int d=0; d<8; d++) movesofgood[d].clear();  

  for(int i=0; i<size(ghosts); i++) {
    cell *c = ghosts[i];
    if((c->monst == moGhost || c->monst == moGreaterShark) && c->cpdist > 1) {
      int goodmoves = 0;

      for(int k=0; k<c->type; k++) if(c->mov[k] && !c->mov[k]->monst && c->mov[k]->cpdist < c->cpdist)
        if(c->monst == moGhost || c->mov[k]->wall == waLake)
          goodmoves++;
      
      movesofgood[goodmoves].push_back(c);
      }
    }
  
  for(int d=0; d<8; d++) for(int i=0; i<size(movesofgood[d]); i++) {
    cell *c = movesofgood[d][i];
    
    if((c->monst == moGhost || c->monst == moGreaterShark) && c->cpdist > 1) {
      
      cell *mpos[7];

      for(int j=0; j<c->type; j++) 
        if(c->mov[j] && isFriendly(c->mov[j])) {
          // XLATC ghost/greater shark
          addMessage(XLAT("The %1 scares the %2!", c->monst, c->mov[j]->monst));
          c->mov[j]->monst = moNone;
          return;
          }
    
      int qmpos = 0;
      for(int k=0; k<c->type; k++) if(c->mov[k] && !c->mov[k]->monst && c->mov[k]->cpdist < c->cpdist)
        if(c->monst == moGhost || c->mov[k]->wall == waLake)
          mpos[qmpos++] = c->mov[k];
      if(!qmpos) continue;
      cell *c2 = mpos[rand() % qmpos];
      c2->monst = c->monst; c->monst = moNone;
      }
    }
  }

int lastdouble = -3;

void stabbingAttack(cell *mf, cell *mt, eMonster who = moNone) {
  int numsh = 0;
  for(int t=0; t<mf->type; t++) {
    cell *c = mf->mov[t];
    if(c->monst == moHedge || (!who && items[itOrbThorns] && c->monst && isKillable(c))) {
      for(int u=0; u<c->type; u++) {
        if(c->mov[u] == mt) {
          if(who)
            addMessage(XLAT("%1 stabs the %2.", who, c->monst));
          else
            addMessage(XLAT("You stab the %1.", c->monst));
          int k = tkills();
          killMonster(c);
          if(tkills() > k) numsh++;
          }
        }
      }
    }
  if(numsh) achievement_count("STAB", numsh, 0);
  if(numsh == 2) {
    if(lastdouble == turncount-1) achievement_count("STAB", 4, 0);
    lastdouble = turncount;
    }
  }

void movegolems() {
  int qg = 0;
  for(int i=0; i<size(golems); i++) {
    cell *c = golems[i];
    if(c->monst == moGolem) {
      qg++;
      int bestv = 100, bq = 0, bdirs[7];
      for(int k=0; k<c->type; k++) if(c->mov[k]) {
        int val;
        if(c->mov[k] == cwt.c) val = 0;
        else if(isActiveEnemy(c->mov[k], NULL) && isKillable(c->mov[k]))
          val = 12000;
        else if(isInactiveEnemy(c->mov[k]) && isKillable(c->mov[k]))
          val = 10000;
        else if(isIvy(c->mov[k])) val = 8000;
        else if(monstersnear(c->mov[k], NULL, false)) val = 0;
        else if(passable(c->mov[k], c, false, false)) val = 4000;
        else val = 0;
        val -= c->mov[k]->pathdist;
        if(val > bestv) bestv = val, bq = 0;
        if(val == bestv) bdirs[bq++] = k;
        }
      if(bestv <= 100) continue;
      int dir = bdirs[rand() % bq];
      cell *c2 = c->mov[dir];
      if(c2->monst) {
        addMessage(XLAT("The %1 destroys the %2!", c->monst, c2->monst));
        killMonster(c2);
        }
      else {
        stabbingAttack(c, c2, moGolem);
        c2->monst = moGolemMoved;
        c->monst = moNone;
        }
      }
    }
  achievement_count("GOLEM", qg, 0);
  }

bool wchance(int a, int of) {
  of *= 10; 
  a += items[itOrbYendor] * 5 + 1;
  if(cwt.c->land == laCrossroads) a+= items[itHyperstone] * 10;
  for(int i=0; i<ittypes; i++) if(itemclass(eItem(i)) == 0)
    a = max(a, (items[i]-10) / 10);
  return rand() % (a+of) < a;
  }

void wandering() {
  int t = turncount - lastexplore;
  int seepcount = 0;
  if(t > 40) seepcount = (t-40 + rand() % 20) / 20;
  int ghostcount = 0;
  if(t > 80) ghostcount = (t-80 + rand() % 20) / 20;
  
  while(first7 < size(dcal)) {
    int i = first7 + rand() % (size(dcal) - first7);
    cell *c = dcal[i];
    
    // wandering seeps & ghosts
    if(seepcount && c->wall == waCavewall && !c->monst) {
      c->monst = moSeep;
      seepcount--;
      continue;
      }
    
    if(ghostcount && !c->monst && cwt.c->type != laCaves) {
      c->monst = moGhost;
      ghostcount--;
      continue;
      }
    
    if(c->monst || c->pathdist == INFD) break;

    else if(c->land == laIce && wchance(items[itDiamond], 10))
      c->monst = rand() % 2 ? moWolf : moYeti;

    else if(c->land == laDesert && wchance(items[itSpice], 10))
      c->monst = rand() % 10 ? moDesertman : moWorm;

    else if(c->land == laCaves && wchance(items[itGold], 5))
      c->monst = rand() % 3 ? moTroll : moGoblin;

    else if(c->land == laJungle && wchance(items[itEmerald], 40))
      c->monst = rand() % 10 ? moMonkey : moEagle;

    else if(c->land == laMirror && wchance(items[itShard], 15))
      c->monst = rand() % 10 ? moRanger : moEagle;

    else if(c->land == laHell && wchance(items[itHell], 20))
      c->monst = rand() % 3 ? moLesser : moGreater;

    else if(c->land == laRlyeh && wchance(items[itStatue], 15))
      c->monst = rand() % 3 ? moPyroCultist : moCultist;

    else if(c->land == laGraveyard && wchance(items[itBone], 15))
      c->monst = rand() % 5 ? moGhost : moNecromancer;
      
    else if(c->land == laDryForest && wchance(items[itFernFlower], 5))
      c->monst = rand() % 5 ? moHedge : moFireFairy;
      
    else if(c->land == laCocytus && wchance(items[itSapphire], 45))
      c->monst = moCrystalSage;
      
    else if(c->land == laAlchemist && wchance(items[itElixir], 3) && eq(c->tmp, sval) && c->item == itNone)
      c->monst = moSlime;
    
    else if(c->land == laCrossroads && items[itHyperstone] && wchance(items[itHyperstone], 20)) {
      // only interesting monsters here!
      static eMonster m[9] = {
        moWorm, moTroll, moEagle,
        moLesser, moGreater, moPyroCultist, moGhost,
        moFireFairy, moHedge
        };
      c->monst = m[rand() % 9];
      }
    
    else break;
      
    if(c->monst == moWorm) c->mondir = NODIR;
      
    // laMotion -> no respawn!
    }
  }

void sageheat(cell *c, double v) {
  c->heat += v;
  if(c->wall == waFrozenLake && c->heat > .6) c->wall = waLake;
  }

bool normalMover(eMonster m) {
  return
    m == moYeti || m == moRanger || m == moGoblin || m == moTroll || m == moDesertman || m == moMonkey ||
    m == moZombie || m == moNecromancer || m == moCultist || m == moLesser || m == moGreater ||
    m == moRunDog || m == moPyroCultist || m == moFireFairy || m == moCrystalSage ||
    m == moHedge;
  }

void movemonsters() {

  bool sagefresh = true;
  turncount++;
  DEBT("golems");
  movegolems();
  DEBT("ghosts");
  moveghosts();
  
  DEBT("normal");
  
  for(int d=0; d<8; d++) movesofgood[d].clear();
  
  for(int i=0; i<size(pathqm); i++) {
    cell *c = pathqm[i];
    eMonster m = c->monst;
    
    if(isActiveEnemy(c, NULL)) {
    
      if(c->pathdist == 1 && c->monst != moGhost) {
        // c->iswall = true; c->ismon = false;
        if(items[itOrbShield] || c->monst == moCrystalSage) continue;

        addMessage(XLAT("The %1 is confused!", m));
        // playerdead = true;
        break;
        }
      
      if(c->monst == moNecromancer) {
        int gravenum = 0, zombienum = 0;
        cell *gtab[8], *ztab[8];
        for(int j=0; j<c->type; j++) if(c->mov[j]) {
          if(c->mov[j]->wall == waFreshGrave) gtab[gravenum++] = c->mov[j];
          if(passable(c->mov[j], c, false, false) && c->mov[j]->pathdist < c->pathdist)
            ztab[zombienum++] = c->mov[j];
          }
        if(gravenum && zombienum) {
          cell *gr = gtab[rand() % gravenum];
          gr->wall = waAncientGrave;
          gr->monst = moGhost;
          ztab[rand() % zombienum]->monst = moZombie;
          addMessage(XLAT("The %1 raises some undead!", c->monst));
          continue;
          }
        }
      
      if(c->monst == moWolf) {
        int bhd = NODIR;
        ld besth = c->heat;
        for(int j=0; j<c->type; j++) if(c->mov[j]->heat > besth && passable(c->mov[j], c, false, false))
          besth = c->mov[j]->heat, bhd = j;
        if(bhd != NODIR) {
          // printf("wolf moved from %Lf (%p) to %Lf (%p)\n", c->heat, c, besth, c->mov[bhd]);
          c->mov[bhd]->monst = moWolfMoved, c->monst = moNone;
          }
        }
      
      else if(c->monst == moPyroCultist && c->cpdist <= 4 && cwt.c->wall == waNone && !cellUnstable(cwt.c)) {
        addMessage(XLAT("The %1 throws fire at you!", c->monst));
        cwt.c->wall = waBonfire;
        cwt.c->tmp = 20;
        c->monst = moCultist;
        }

      else if(c->monst == moCrystalSage && c->cpdist <= 4 && isIcyLand(cwt.c)) {
        // only one sage attacks
        if(sagefresh) {
          sagefresh = false;
          if(sagephase == 0) {
            addMessage(XLAT("The %1 shows you two fingers.", c->monst));
            addMessage(XLAT("You wonder what does it mean?"));
            }
          else if(sagephase == 1) {
            addMessage(XLAT("The %1 shows you a finger.", c->monst));
            addMessage(XLAT("You think about possible meanings."));
            }
          else {
            addMessage(XLAT("The %1 moves his finger downwards.", c->monst));
            addMessage(XLAT("Your brain is steaming."));
            }
          sagephase++;
          sageheat(cwt.c, .0);
          for(int i=0; i<cwt.c->type; i++)
            sageheat(cwt.c->mov[i], .3);
          }
        }
      
      else if(normalMover(m)) {
        int goodmoves = 0;
        for(int t=0; t<c->type; t++) if(c->mov[t] && c->mov[t]->pathdist < c->pathdist)
          goodmoves++;
        movesofgood[goodmoves].push_back(c);
        }
      }
    }

  for(int d=0; d<8; d++) for(int i=0; i<size(movesofgood[d]); i++) {
    cell *c = movesofgood[d][i];
    if(normalMover(c->monst))
      moveNormal(c);
    }

  if(sagefresh) sagephase = 0;
  
  int dcs = size(dcal);

  DEBT("worm");
  int wrm = size(worms);
  for(int i=0; i<wrm; i++) {
    moveWorm(worms[i]);
    }

  DEBT("ivy");
  moveivy();
  DEBT("slimes");
  moveslimes();
  DEBT("eagles");
  moveeagles(1);
  moveeagles(2);
  
  DEBT("fresh");
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    
    if(c->monst == moWolfMoved) c->monst = moWolf;
    if(c->monst == moIvyNext) {
      c->monst = moIvyHead; ivynext(c);
      }
    if(c->monst == moIvyDead) c->monst = moNone;
    if(c->monst == moGolemMoved) c->monst = moGolem;
    if(c->monst == moLesser) c->monst = moLesserM;
    else if(c->monst == moLesserM) c->monst = moLesser;
    if(c->monst == moGreater) c->monst = moGreaterM;
    else if(c->monst == moGreaterM) c->monst = moGreater;
    
    if(c->wall == waChasm) {
      c->item = itNone;
      if(c->monst && c->monst != moGhost && c->monst != moEagle) {
        if(c->monst != moRunDog) achievement_gain("FALLDEATH1");
        killMonster(c);
        }
      }

    if(c->wall == waLake) {
      c->item = itNone;
      if(c->monst == moLesser || c->monst == moLesserM || c->monst == moGreater || c->monst == moGreaterM)
        c->monst = moGreaterShark;
      if(c->monst && c->monst != moShark && c->monst != moGreaterShark &&
        c->monst != moGhost && c->monst != moEagle) killMonster(c);
      }

    if(c->monst && cellUnstable(c) && c->monst != moGhost && c->monst != moEagle) {
      c->wall = waChasm;
      }
    }

  DEBT("shadow");
  moveshadow();
  
  DEBT("wandering");
  wandering();
  }

// move heat
void heat() {
  double rate = items[itOrbSpeed] ? .5 : 1;
  int oldmelt = kills[0];

  /* if(cwt.c->heat > .5)  cwt.c->heat += .3;
  if(cwt.c->heat > 1.)  cwt.c->heat += .3;
  if(cwt.c->heat > 1.4) cwt.c->heat += .5; */
  if(isIcyLand(cwt.c))
    cwt.c->heat += (items[itOrbWinter] ? -1.2 : 1.2) * rate;
  
  int dcs = size(dcal);
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->wall == waBonfire && c->tmp > 0) useup(c);
    if(!isIcyLand(c)) continue;
    if(c->monst == moRanger) c->heat += 3 * rate;
    if(c->monst == moDesertman) c->heat += 4 * rate;
    if(c->monst == moMonkey) c->heat += rate;
    if(c->wall == waDeadTroll) c->heat -= 2 * rate;
    if(c->monst == moLesser || c->monst == moLesserM || c->monst == moGreater || c->monst == moGreaterM)
      c->heat += (c->land == laCocytus ? 1.5 : 10) * rate;
    if(c->monst == moGreaterShark)
      c->heat += 2 * rate;
    if(c->monst == moCultist) c->heat += 3 * rate;
    if(c->monst == moPyroCultist) c->heat += 6 * rate;
    if(c->wall == waBonfire && c->tmp > 0) c->heat += 4 * rate;
    
    ld hmod = 0;
    
    for(int j=0; j<c->type; j++) if(c->mov[j]) {
      if(!isIcyLand(c->mov[j])) {
        // make sure that we can still enter Cocytus,
        // it won't heat up right away even without Orb of Winter or Orb of Speed
        if(c->mov[j] == cwt.c && (c->land == laIce || items[itOrbWinter])) 
          hmod += items[itOrbWinter] ? -1.2 : 1.2;
        continue;
        }
      ld hdiff = c->mov[j]->heat - c->heat;
      hdiff /= 10;
      if(c->mov[j]->cpdist <= 8)
        c->mov[j]->heat -= hdiff;
      else
        hdiff = -c->heat / 250;
      hmod += hdiff;
      }
    
    c->heat += hmod * rate;
    if(c->monst == moCrystalSage && c->heat >= SAGEMELT) {
      addMessage(XLAT("The %1 melts away!", c->monst));
      killMonster(c);
      }
    }
  
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->wall == waIcewall && c->heat > .4) c->wall = waNone, kills[0]++;
    if(c->wall == waFrozenLake && c->heat > .6) c->wall = waLake, kills[0]++;
    if(c->wall == waLake && c->heat < -.4 && c->monst != moGreaterShark) {
      c->wall = waFrozenLake;
      if(c->monst == moShark) {
        addMessage(XLAT("The %1 is frozen!", c->monst));
        killMonster(c);
        }
      }
    }

  if(kills[0] != oldmelt) bfs();
  }

void livecaves() {
  int dcs = size(dcal);
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->wall != waCavefloor && c->wall != waCavewall) continue;
    
    // if(c->wall == waThumper || c->wall == waBonfire) continue;
    c->tmp = 0;
    if(c->item || c->monst || c->cpdist == 0) continue;
    for(int j=0; j<c->type; j++) if(c->mov[j]) {
      if(c->mov[j]->wall == waCavefloor) c->tmp++;
      else if(c->mov[j]->wall == waCavewall) c->tmp--;
      else if(c->mov[j]->wall == waDeadTroll) c->tmp -= 5;
      else if(c->mov[j]->wall != waBarrier) c->tmp += 5;      
      if(c->mov[j]->cpdist == 0 && items[itOrbDigging]) c->tmp+=100;
      if(c->mov[j]->wall == waThumper && c->mov[j]->tmp > 0) c->tmp+=100;
      if(c->mov[j]->wall == waBonfire) c->tmp+=100;
      if(c->mov[j]->item) c->tmp+=2;
      if(c->mov[j]->monst == moZombie) c->tmp += 10;
      if(c->mov[j]->monst == moGhost) c->tmp += 10;
      if(c->mov[j]->monst == moNecromancer) c->tmp += 10;
      if(c->mov[j]->monst == moWormtail) c->tmp++;
      if(c->mov[j]->monst == moTentacletail) c->tmp-=2;
      if(isIvy(c->mov[j])) c->tmp--;
      if(isDemon(c->mov[j])) c->tmp-=3;
      // if(c->mov[j]->monst) c->tmp++;
      // if(c->mov[j]->monst == moTroll) c->tmp -= 3;
      }
    }

  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->wall != waCavefloor && c->wall != waCavewall) continue;
//  if(c->land != laCaves) continue;
//  if(c->wall == waThumper || c->wall == waBonfire) continue;
    
    if(c->tmp > 0) c->wall = waCavefloor;
    if(c->tmp < 0) c->wall = waCavewall;
    }
  
  }

void dryforest() {
  int dcs = size(dcal);
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->land != laDryForest) continue;
    if(c->wall == waThumper || c->wall == waBonfire) continue;
    
    for(int j=0; j<c->type; j++) if(c->mov[j]) {
      if(c->mov[j]->wall == waBonfire) c->heat++;
      }
    }

  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->land != laDryForest) continue;
    if(c->wall == waNone && c->heat >= 10) {
      if(c->item) addMessage(XLAT("The %1 burns!", c->item)), c->item = itNone;
      if(c->monst && c->monst != moGhost) 
        addMessage(XLAT("The %1 burns!", c->monst)), c->monst = moNone;
      c->wall = waBonfire, c->tmp = 50;
      }
    if((c->wall == waDryTree || c->wall == waWetTree) && c->heat >= 1)
      c->wall = waBonfire, c->tmp = 50;
    }

/*
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    c->tmp = 0;
    c->heat = 0;
    if(c->cpdist > 8) break;
    if(c->land != laDryForest) continue;
    
    for(int j=0; j<c->type; j++) if(c->mov[j]) {
      if(c->mov[j]->wall == waWetTree)
        c->tmp++;
      if(c->mov[j]->wall == waDryTree)
        c->heat++;
      }
    }

  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    int a = c->type - c->tmp - int(c->heat);
    if(c->tmp > a && c->tmp > c->heat)
      c->wall = waWetTree;
    else if(c->heat > a && c->heat > c->tmp)
      c->wall = waDryTree;
    else if(a > c->heat && a > c->tmp)
      c->wall = waNone;
    } */
  }

// mirror management

void createMirrors(cell *c, int dir, eMonster type) {
  cellwalker C(c, dir);
  
  if(type == moMirror) type = moMirage;
  else type = moMirror;

  for(int i=0; i<6; i++) {
    cwstep(C);
    if(C.c->type == 6) {
      cwspin(C, i);
      if(C.c->monst == moNone && C.c->wall == waNone && C.c != cwt.c) {
        C.c->monst = type;
        C.c->mondir = C.spin;
        }
      cwspin(C, -i);
      }
    cwstep(C);
    cwspin(C, 1);
    }
  }

void createMirages(cell *c, int dir, eMonster type) {
  cellwalker C(c, dir);
  for(int i=0; i<6; i++) {
    cwstep(C);
    if(C.c->type == 6) {
      cwspin(C, 2);
      cwstep(C);
      cwspin(C, 4-i);
      if(C.c->monst == moNone && C.c->wall == waNone && C.c != cwt.c) {
        C.c->monst = type;
        C.c->mondir = C.spin;
        }
      cwspin(C, 6-4+i);
      cwstep(C);
      cwspin(C, 2);
      cwstep(C);
      cwspin(C, 2-i);
      if(C.c->monst == moNone && C.c->wall == waNone && C.c != cwt.c) {
        C.c->monst = type;
        C.c->mondir = C.spin;
        }
      cwspin(C, 6-2+i);
      cwstep(C);
      cwspin(C, 2);
      }
    cwstep(C);
    cwspin(C, 1);
    }
  }

void spinmirrors(int d) {

  for(int i=0; i<size(mirrors); i++) {
    cell *c = mirrors[i];
    if(c->monst == moMirror) 
      mirrors[i]->mondir = (mirrors[i]->mondir - d + 42) % mirrors[i]->type;
    if(c->monst == moMirage)
      mirrors[i]->mondir = (mirrors[i]->mondir + d + 42) % mirrors[i]->type;
    }

  }

void gomirrors(bool go) {
  int tk = tkills();
  int nummirage = 0;
  mirrors2.clear();
  for(int i=0; i<size(mirrors); i++) {
    cell *c = mirrors[i];
    eMonster m = c->monst;
    if(m == moMirror || m == moMirage) {
      if(m == moMirage) nummirage++;
      cell *c2 = c->mov[c->mondir];
      if(c2 && c2->monst != moNone && !isMimic(c2) && isKillable(c2)) {
        addMessage(XLAT("The %1 destroys the %2!", m, c2->monst));
        killMonster(c2);
        }
      if(!go) continue;
      c->monst = moNone;
      if(!c2) continue;
      stabbingAttack(c, c2, m);
      if(!passable(c2, c, true, true))  continue;
      if(isWorm(c2)) continue;
      if(c2->monst == moGreater) {
        c2->monst = moLesser; continue;
        }
      if(c2->monst == moGreaterM) {
        c2->monst = moLesserM; continue;
        }
      if(c2 == cwt.c) {
        addMessage(XLAT("You join the %1.", m));
        continue;
        }
      if(isMimic(c2)) {
        addMessage(XLAT("Two of your images crash and disappear!"));
        c2->monst = moNone;
        continue;
        }
      if(isIvy(c2)) killIvy(c2);
      c2->monst = m;
      c2->mondir = c->spn[c->mondir];
      if(c2->wall == waMirror || c2->wall == waCloud)
        mirrors2.push_back(c2);
      }
    }
  for(int i=0; i<size(mirrors2); i++) {
    cell *c = mirrors2[i];
    eMonster m = c->monst;
    if(c->wall == waMirror) {
      addMessage(XLAT("The %1 breaks the mirror!", m));
      createMirrors(c, c->mondir, m);
      c->wall = waNone;
      }
    if(c->wall == waCloud) {
      addMessage(XLAT("The %1 disperses the cloud!", m));
      createMirages(c, c->mondir, m);
      c->wall = waNone;
      }
    }
  achievement_count("MIRRORKILL", tkills(), tk);
  achievement_count("MIRAGE", nummirage, 0);
  }

void reduceOrbPowers() {
  if(items[itOrbLightning]) items[itOrbLightning]--;
  if(items[itOrbSpeed]) items[itOrbSpeed]--;
  if(items[itOrbFlash]) items[itOrbFlash]--;
  if(items[itOrbShield]) items[itOrbShield]--;
  if(items[itOrbWinter]) items[itOrbWinter]--;
  if(items[itOrbDigging]) items[itOrbDigging]--;
  if(items[itOrbTeleport]) items[itOrbTeleport]--;
  if(items[itOrbSafety]) items[itOrbSafety]--;
  if(items[itOrbThorns]) items[itOrbThorns]--;
  }

void flashAlchemist(cell *c) {
  if(c->wall == waFloorA || c->wall == waFloorB) {
    if(cwt.c->wall == waFloorA || cwt.c->wall == waFloorB)
      c->wall = cwt.c->wall;
    else
      c->wall = eWall(c->wall ^ waFloorB ^ waFloorA);
    }
  }

void activateFlash() {
  int tk = tkills();
  extern void drawFlash();
  drawFlash();
  addMessage(XLAT("You activate the Flash spell!"));
  items[itOrbFlash] = 0;
  for(int i=0; i<size(dcal); i++) {
    cell *c = dcal[i];
    if(c->cpdist > 2) break;
    flashAlchemist(c);
    killMonster(c);
    c->heat += 2;
    if(c->wall == waCavewall)  c->wall = waCavefloor;
    if(c->wall == waDeadTroll) c->wall = waCavefloor;
    if(c->wall == waMirror)    c->wall = waNone;
    if(c->wall == waCloud)     c->wall = waNone;
    if(c->wall == waDune)      c->wall = waNone;
    if(c->wall == waAncientGrave) c->wall = waNone;
    if(c->wall == waFreshGrave) c->wall = waNone;
    if(c->wall == waColumn)    c->wall = waNone;
    if(c->wall == waDryTree || c->wall == waWetTree)    c->wall = waNone;
    if(isActiv(c))             c->tmp = 77;
    }
  achievement_count("FLASH", tkills(), tk);
  }

bool barrierAt(cellwalker& c, int d) {
  if(d >= 7) return true;
  if(d <= -7) return true;
  d = c.spin + d + 42;
  d%=c.c->type;
  if(!c.c->mov[d]) return true;
  if(c.c->mov[d]->wall == waBarrier) return true;
  return false;
  }

void activateLightning() {
  int tk = tkills();
  extern void drawLightning();
  drawLightning();
  addMessage(XLAT("You activate the Lightning spell!"));
  items[itOrbLightning] = 0;
  for(int i=0; i<cwt.c->type; i++) {
    cellwalker lig(cwt.c, i);
    int bnc = 0;
    while(true) {
      // printf("at: %p i=%d d=%d\n", lig.c, i, lig.spin);
      if(lig.c->mov[lig.spin] == 0) break;
      cwstep(lig);
      
      cell *c = lig.c;

      flashAlchemist(c);
      killMonster(c);
      c->heat += 2;
      c->ligon = 1;
      
      bool brk = false, spin = false;
      if(c->wall == waCavewall)  c->wall = waCavefloor, brk = true;
      if(c->wall == waDeadTroll) c->wall = waCavefloor, brk = true;
      if(c->wall == waMirror)    c->wall = waNone;
      if(c->wall == waCloud)     c->wall = waNone;
      if(c->wall == waDune)      c->wall = waNone, brk = true;
      if(c->wall == waIcewall)   c->wall = waNone, brk = true;
      if(c->wall == waAncientGrave) c->wall = waNone, spin = true;
      if(c->wall == waFreshGrave) c->wall = waNone, spin = true;
      if(c->wall == waColumn)    c->wall = waNone, spin = true;
      if(c->wall == waDryTree || c->wall == waWetTree)    c->wall = waNone;
      if(c == cwt.c)             brk = true;
      if(isActiv(c))             c->tmp = 77;
      if(spin) cwspin(lig, rand() % lig.c->type);
      
      if(brk) break;
      
      if(c->wall == waBarrier)   {
        int left = -1;
        int right = 1;
        while(barrierAt(lig, left)) left--;
        while(barrierAt(lig, right)) right++;
        cwspin(lig, -(right + left));
        bnc++; if(bnc > 10) break;
        }
      else {
        cwspin(lig, 3);
        if(c->type == 7) cwspin(lig, rand() % 2);
        }
      }
    }
  achievement_count("LIGHTNING", tkills(), tk);
  }

// move the PC in direction d (or stay in place for d == -1)

bool canmove = true;

bool checkNeedMove(bool checkonly) {
  if(cwt.c->wall == waLake) {
    if(checkonly) return true;
    addMessage(XLAT("Ice below you is melting! RUN!"));
    }
  else if(cwt.c->wall == waBonfire && items[itOrbShield] < 2 && !items[itOrbWinter]) {
    if(checkonly) return true;
    addMessage(XLAT("This spot will be burning soon! RUN!"));
    }
  else if(cwt.c->wall == waChasm) {
    if(checkonly) return true;
    addMessage(XLAT("The floor has collapsed! RUN!"));
    }
  else return false;
  return true;
  }

#define YDIST 101

struct yendorinfo {
  cell *path[YDIST];
  bool found;
  };

vector<yendorinfo> yi;

int yii = 0;

bool checkYendor(cell *yendor, bool checkonly) {
  int byi = size(yi);
  for(int i=0; i<size(yi); i++) if(yi[i].path[0] == yendor) byi = i;
  if(byi < size(yi) && yi[byi].found) return true;
  if(checkonly) return false;
  if(byi == size(yi)) {
    yendorinfo nyi;
    nyi.path[0] = yendor;

    cellwalker lig(yendor, rand() % yendor->type);

    cell *prev = yendor;
    
    for(int i=0; i<YDIST-1; i++) {
      nyi.path[i] = lig.c;
      
      prev = lig.c;
      cwstep(lig);
      cwspin(lig, 3);
      if(lig.c->type == 7) cwspin(lig, rand() % 2);
            
      setdist(lig.c, 10, prev);
      setdist(lig.c, 9, prev);
      }

    nyi.path[YDIST-1] = lig.c;
    nyi.found = false;

    cell *key = lig.c;

    for(int b=10; b>=7; b--) setdist(key, b, prev);
    
    for(int i=-1; i<key->type; i++) {
      cell *c2 = i >= 0 ? key->mov[i] : key;
      c2->monst = moNone; c2->item = itNone;
      if(!passable(c2, NULL, true, true)) {
        if(c2->wall == waCavewall) c2->wall = waCavefloor;
        else if(c2->wall == waLake) c2->wall = waFrozenLake;
        else c2->wall = waNone;
        }
      }
    key->item = itKey;

    yi.push_back(nyi);
    }
  yii = byi;
  addMessage(XLAT("You need to find the right Key to unlock this Orb of Yendor!"));
  achievement_gain("YENDOR1");
  return false;
  }

int countMyGolems() {
  int g=0, dcs = size(dcal);
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->monst == moGolem) g++;
    }
  return g;
  }

void restoreGolems(int qty) {
  int dcs = size(dcal);
  for(int i=1; qty && i<dcs; i++) {
    cell *c = dcal[i];
    if(passable(c, NULL, false, false))
      c->monst = moGolem, qty--;
    }
  }

void activateSafety(eLand l) {
  extern void drawSafety();
  int g = countMyGolems();
  drawSafety();
  addMessage(XLAT("You fall into a wormhole!"));
  eLand f = firstland;
  firstland = l;
  for(int i=0; i<65536; i++) euland[i] = laNone;
  euland[0] = euland[1] = firstland;
  safety = true;
  clearMemory();
  initcells();
  initgame();
  firstland = f;
  safety = false;
  restoreGolems(g); 
  extern void restartGraph();
  restartGraph();  
  }

bool hasSafeOrb(cell *c) {
  return 
    c->item == itOrbSafety ||
    c->item == itOrbShield ||
    c->item == itOrbYendor;
  }


bool movepcto(int d, bool checkonly = false);

void checkmove() {
  canmove = false;
  if(movepcto(-1, true)) canmove = true;
  for(int i=0; i<cwt.c->type; i++) 
    if(movepcto(1, true)) canmove = true;
  if(!canmove)
    achievement_final(true);
  }

// move the PC. Warning: a very long function! todo: refactor

bool movepcto(int d, bool checkonly) {
  if(checkonly && items[itOrbTeleport]) return true;
  if(!checkonly) flipplayer = false;
  if(!checkonly) DEB("movepc");
  if(d >= 0) {
    cwspin(cwt, d);
    spinmirrors(d);
    d = cwt.spin;
    }
  playermoved = true;
  if(d >= 0) {
    cell *c2 = cwt.c->mov[d];
    
    if(!player_passable(c2, cwt.c, false) && items[itOrbFlash]) {
      if(checkonly) return true;
      activateFlash();
      checkmove();
      return true;
      }

    if(!player_passable(c2, cwt.c, false) && items[itOrbLightning]) {
      if(checkonly) return true;
      activateLightning();
      checkmove();
      return true;
      }

    if(isActiv(c2) && c2->tmp == -1) {
      if(checkonly) return true;
      addMessage(XLAT("You activate the %1.", c2->wall));
      c2->tmp = 100;
      checkmove();
      return true;
      }

    if(c2->wall == waThumper && !monstersnear(c2)) {
      cellwalker push = cwt;
      cwstep(push);
      cwspin(push, 3);
      cwstep(push);
      if((!passable(push.c, c2, false, true) || !passable(push.c, cwt.c, false, true) || push.c->item) && c2->type == 7) {
        cwstep(push);
        cwspin(push, 1);
        cwstep(push);
        }
      if(!passable(push.c, c2, false, true) || !passable(push.c, cwt.c, false, true) || push.c->item) {
        if(checkonly) return false;
        addMessage(XLAT("No room to push the %1.", c2->wall));
        return false;
        }
      if(checkonly) return true;
      addMessage(XLAT("You push the %1.", c2->wall));
      push.c->tmp = c2->tmp;
      if(c2->land == laAlchemist)
        c2->wall = (cwt.c->wall == waFloorB || cwt.c->wall == waFloorA) ? cwt.c->wall : push.c->wall;
      else c2->wall = waNone;
      push.c->wall = waThumper;
      }

    if(c2->wall == waDryTree && !monstersnear(cwt.c)) {
      if(checkonly) return true;
      addMessage(XLAT("You start cutting down the tree."));
      c2->wall = waWetTree;
      }
    else if(c2->wall == waWetTree && !monstersnear(cwt.c)) {
      if(checkonly) return true;
      addMessage(XLAT("You cut down the tree."));
      c2->wall = waNone;
      }
    else if(c2->monst && !isFriendly(c2)) {
      if(c2->monst == moWorm || c2->monst == moWormtail || c2->monst == moWormwait) {
        if(checkonly) return false;
        addMessage(XLAT("You cannot attack Sandworms directly!"));
        return false;
        }
      
      if(c2->monst == moTentacle || c2->monst == moTentacletail || c2->monst == moTentaclewait) {
        if(checkonly) return false;
        addMessage(XLAT("You cannot attack Tentacles directly!"));
        return false;
        }
      
      if(c2->monst == moHedge && !items[itOrbThorns]) {
        if(checkonly) return false;
        addMessage(XLAT("You cannot attack the %1 directly!", c2->monst));
        addMessage(XLAT("Stab them by walking around them."));
        return false;
        }
      
      if(c2->monst == moShadow) {
        if(checkonly) return false;
        addMessage(XLAT("You cannot defeat the Shadow!"));
        return false;
        }
      
      if(c2->monst == moGreater || c2->monst == moGreaterM) {
        if(checkonly) return false;
        addMessage(XLAT("You cannot defeat the Greater Demon yet!"));
        return false;
        }
      
      if(monstersnear(cwt.c, c2)) {
        if(checkonly) return false;
        addMessage(XLAT("You would be killed by the %1!", which));
        return false;
        }

      if(checkNeedMove(checkonly))
        return false;
      
      if(checkonly) return true;
      addMessage(XLAT("You kill the %1.", c2->monst));

      int mt = c2->monst;
      int tk = tkills();
      killMonster(c2);
      
      int ntk = tkills();

      if(tk == 0 && ntk > 0)
        addMessage(XLAT("That was easy, but groups could be dangerous."));
        
      if(tk < 10 && ntk >= 10)
        addMessage(XLAT("Good to know that your fighting skills serve you well in this strange world."));

      if(tk < 50 && ntk >= 50)
        addMessage(XLAT("You wonder where all these monsters go, after their death..."));

      if(tk < 100 && ntk >= 100)
        addMessage(XLAT("You feel that the souls of slain enemies pull you to the Graveyard..."));
        
      if(mt == moIvyRoot && ntk>tk)
        achievement_gain("IVYSLAYER");
      
      gomirrors(0);
      }
    else if(!player_passable(c2, cwt.c, true)) {
      if(checkonly) return false;
      if(c2->wall == waFloorA || c2->wall == waFloorB)
        addMessage(XLAT("Wrong color!"));
      else
        addMessage(XLAT("You cannot move through the %1!", c2->wall));
      return false;
      }
    else if(c2->land == laGameBoard) {
      // do not pick up!
      if(checkonly) return true;
      flipplayer = true;
      cwstep(cwt);      
      setdist(cwt.c, 0, NULL);
      bfs();
      checkmove();
      return true;
      }
    else {
      if(c2->item == itOrbYendor && !checkYendor(c2, checkonly)) {
        return false;
        }
      if(!hasSafeOrb(c2) && monstersnear(c2)) {
        if(checkonly) return false;
        addMessage(XLAT("The %1 would kill you there!", which));
        return false;
        }
      if(checkonly) return true;
      flipplayer = true;
      if(c2->item && c2->land == laAlchemist) c2->wall = cwt.c->wall;
      
      int pg = gold();

      if(c2->item) {
        string s0 = "";
        if(0) ;
        if(gold() == 0)
          addMessage(XLAT("Wow! %1! This trip should be worth it!", c2->item));
        else if(gold() == 1)
          addMessage(XLAT("For now, collect as much treasure as possible..."));
        else if(gold() == 2)
          addMessage(XLAT("Prove yourself here, then find new lands, with new quests..."));
        else if(!items[c2->item] && itemclass(c2->item) == 0)
          addMessage(XLAT("You collect your first %1!", c2->item));
        else if(c2->item == itKey)
          addMessage(XLAT("You have found the Key! Now unlock this Orb of Yendor!"));
        else if(c2->item == itGreenStone && !items[itGreenStone])
          addMessage(XLAT("This orb is dead..."));
        else if(c2->item == itGreenStone)
          addMessage(XLAT("Another Dead Orb."));
        else if(itemclass(c2->item) != 0)
          addMessage(XLAT("You have found the %1!", c2->item));
        else if(items[c2->item] == 4 && maxgold() == 4) {
          addMessage(XLAT("You feel that the %2 becomes more dangerous.", c2->item, c2->land));
          addMessage(XLAT("With each %1 you collect...", c2->item, c2->land));
          }
        else if(items[c2->item] == 9 && maxgold() == 9)
          addMessage(XLAT("Are there any magical orbs in the %1?...", c2->land));
        else if(items[c2->item] == 10 && maxgold() == 10) {
          addMessage(XLAT("You feel that the %1 slowly becomes dangerous...", c2->land));
          addMessage(XLAT("Better find some other place."));
          }
        else if(c2->item == itSpice && items[itSpice] == 7)
          addMessage(XLAT("You have a vision of the future, fighting demons in Hell..."));
        else if(c2->item == itElixir && items[itElixir] == 4)
          addMessage(XLAT("With this Elixir, your life should be long and prosperous..."));
        else if(c2->item == itBone && items[itBone] == 6)
          addMessage(XLAT("The Necromancer's Totem contains hellish incantations..."));
        else if(c2->item == itStatue && items[itStatue] == 6)
          addMessage(XLAT("The inscriptions on the Statue of Cthulhu point you toward your destiny..."));
        else if(c2->item == itDiamond && items[itDiamond] == 8)
          addMessage(XLAT("Still, even greater treasures lie ahead..."));
        else {
          string t = XLAT("You collect the %1.", c2->item);
          addMessage(t);
          }
        }
      
      if(c2->item == itOrbSpeed) {
        items[c2->item] += 31;
        if(items[c2->item] > 67) items[c2->item] = 67;
        }
      else if(c2->item == itOrbLife) {
        cwt.c->monst = moGolem;
        }
      else if(c2->item == itOrbSafety) {
        items[c2->item] += 7;
        activateSafety(c2->land);
        return true;
        }
      else if(c2->item == itOrbLightning) {
        items[c2->item] += 78;
        if(items[c2->item] > 777) items[c2->item] = 777;
        }
      else if(c2->item == itOrbThorns) {
        items[c2->item] += 78;
        if(items[c2->item] > 151) items[c2->item] = 151;
        }
      else if(c2->item == itOrbFlash) {
        items[c2->item] += 78;
        if(items[c2->item] > 777) items[c2->item] = 777;
        }
      else if(c2->item == itOrbShield) {
        items[c2->item] += 16;
        if(items[c2->item] > 77) items[c2->item] = 77;
        }
      else if(c2->item == itOrbWinter) {
        items[c2->item] += 31;
        if(items[c2->item] > 77) items[c2->item] = 77;
        }
      else if(c2->item == itOrbDigging) {
        items[c2->item] += 78;
        if(items[c2->item] > 101) items[c2->item] = 101;
        }
      else if(c2->item == itOrbTeleport) {
        items[c2->item] += 78;
        if(items[c2->item] > 201) items[c2->item] = 201;
        }
      else if(c2->item == itOrbYendor) {
        items[itOrbSpeed] += 31;
        items[itOrbLightning] += 78;
        items[itOrbFlash] += 78;
        items[itOrbShield] += 31;
        items[itOrbWinter] += 151;
        items[itOrbDigging] += 151;
        items[itOrbTeleport] += 151;
        items[itOrbYendor]++;
        items[itKey]--;
        addMessage(XLAT("CONGRATULATIONS!"));
        achievement_collection(itOrbYendor, pg, gold());
        achievement_victory(false);
        }
      else if(c2->item == itKey) {
        for(int i=0; i<size(yi); i++) if(yi[i].path[YDIST-1] == c2)
          yi[i].found = true;
        items[itKey]++;
        }
      else {
        bool lhu = hellUnlocked();
        if(c2->item) items[c2->item]++;
        int g2 = gold();
        
        if(c2->item == itHyperstone && items[itHyperstone] == 10)
          achievement_victory(true);
        
        achievement_collection(c2->item, pg, g2);

        if(pg < 15 && g2 >= 15) 
          addMessage(XLAT("Collect treasure to access more different lands..."));
        if(pg < 30 && g2 >= 30)
          addMessage(XLAT("You feel that you have enough treasure to access new lands!"));
        if(pg < 45 && g2 >= 45)
          addMessage(XLAT("Collect more treasures, there are still more lands waiting..."));
        if(pg < 60 && g2 >= 60)
          addMessage(XLAT("You feel that the stars are right, and you can access R'Lyeh!"));
        if(pg < 75 && g2 >= 75)
          addMessage(XLAT("Kill monsters and collect treasures, and you may get access to Hell..."));
        if(pg < 90 && g2 >= 90) 
          addMessage(XLAT("To access Hell, collect 10 treasures each of 9 kinds..."));
        if(hellUnlocked() && !lhu) {
          addMessage(XLAT("Abandon all hope, the gates of Hell are opened!"));
          addMessage(XLAT("And the Orbs of Yendor await!"));
          }
        }
      c2->item = itNone;
      
      if(items[itOrbWinter] && isIcyLand(cwt.c)) {
        if(cwt.c->wall == waNone) 
          cwt.c->wall = waIcewall;
        }
      
      if(c2->monst == moGolem || c2->monst == moGolemMoved) {
        addMessage(XLAT("You switch places with the %1.", c2->monst));
        cwt.c->monst = c2->monst;
        c2->monst = moNone;
        }
      else if(c2->monst) {
        addMessage(XLAT("You rejoin the %1.", c2->monst));
        killMonster(c2);
        }

      stabbingAttack(cwt.c, c2);
      cwstep(cwt);
      
      gomirrors(1);

      if(c2->wall == waMirror) {
        addMessage(XLAT("The mirror shatters!"));
        if(c2->land == laMirror) {
          int g = gold();
          items[itShard]++;
          achievement_collection(itShard, g+1, g);
          }
        c2->wall = waNone;
        createMirrors(cwt.c, cwt.spin, moMirage);
        }

      if(c2->wall == waCloud) {
        addMessage(XLAT("The cloud turns into a bunch of images!"));
        if(c2->land == laMirror) {
          int g = gold();
          items[itShard]++;
          achievement_collection(itShard, g+1, g);
          }
        c2->wall = waNone;
        createMirages(cwt.c, cwt.spin, moMirage);
        }
      
      if(cellUnstable(c2))
        c2->wall = waChasm;

      countLocalTreasure();
      landvisited[cwt.c->land] = true;
      setdist(cwt.c, 0, NULL);
      }
    }
  else {
    if(checkNeedMove(checkonly))
      return false;
    if(monstersnear(cwt.c)) {
      if(checkonly) return false;
      addMessage(XLAT("The %1 would get you!", which));
      return false;
      }
    if(checkonly) return true;
    if(d == -2 && items[itGreenStone] && cwt.c->item == itNone) {
      items[itGreenStone]--;
      if(false) {
        cwt.c->item = itNone;
        spill(cwt.c, eWall(cwt.c->wall ^ waFloorA ^ waFloorB), 3);
        addMessage(XLAT("The slime reacts with the %1!", itGreenStone));
        }
      else {
        cwt.c->item = itGreenStone;
        addMessage(XLAT("You drop the %1.", itGreenStone));
        }
      }
    else if(d == -2) {
      if(gold() >= 300)
        addMessage(XLAT("You feel great, like a true treasure hunter."));
      else if(gold() >= 200)
        addMessage(XLAT("Your eyes shine like gems."));
      else if(gold() >= 100)
        addMessage(XLAT("Your eyes shine as you glance at your precious treasures."));
      else if(gold() >= 50)
        addMessage(XLAT("You glance at your great treasures."));
      else if(gold() >= 10)
        addMessage(XLAT("You glance at your precious treasures."));
      else if(gold() > 0)
        addMessage(XLAT("You glance at your precious treasure."));
      else
        addMessage(XLAT("Your inventory is empty."));
      }
    }
  DEBT("bfs");
  bfs();
  DEBT("heat");
  heat();
  DEBT("rop");
  int phase1 = (1 & items[itOrbSpeed]);
  reduceOrbPowers();
  DEBT("mmo");
  int phase2 = (1 & items[itOrbSpeed]);
  if(!phase2) movemonsters();
  DEBT("lc");
  if(!phase1) livecaves();
  if(!phase1) dryforest();
  DEBT("check");
  checkmove();
  DEBT("done");
  return true;
  }

void teleportpc(cell *dest) {
  if(dest->monst) 
    addMessage(XLAT("Cannot teleport on a monster!"));
  else if(dest->item)
    addMessage(XLAT("Cannot teleport on an item!"));
  else if(!passable(dest, NULL, false, true))
    addMessage(XLAT("Cannot teleport here!"));
  else if(dest->cpdist > 7)
    addMessage(XLAT("You cannot teleport that far away!"));
  else {
    cwt.c = dest; cwt.spin = rand() % dest->type; flipplayer = !!(rand() % 2);
    items[itOrbTeleport] = 0;
  
    addMessage(XLAT("You teleport to a new location!"));

    for(int i=9; i>=0; i--)
      setdist(cwt.c, i, NULL);

    bfs();
  
    canmove = false;
    if(movepcto(-1, true)) canmove = true;
    for(int i=0; i<cwt.c->type; i++) 
      if(movepcto(1, true)) canmove = true;

    }
  }

void saveStats() {
  if(euclid) return;
#ifndef ANDROID
  FILE *f = fopen(scorefile, "at");
  if(!f) {
    printf("Could not open the score file '%s'!\n", scorefile);
    addMessage(XLAT("Could not open the score file: ", scorefile));
    return;
    }

  if(showoff) return;
  
  time_t timer;
  timer = time(NULL);
  char sbuf[128]; strftime(sbuf, 128, "%c", localtime(&timerstart));
  char buf[128]; strftime(buf, 128, "%c", localtime(&timer));
  
  fprintf(f, "HyperRogue: game statistics (version "VER")\n");
  if(cheater)
    fprintf(f, "CHEATER! (cheated %d times)\n", cheater);
  if(true) {

    items[0] = items[itFernFlower];
    kills[moWormwait] = kills[moCrystalSage];
    kills[moWormtail] = kills[moFireFairy];
    kills[moTentaclewait] = items[itOrbThorns];

    fprintf(f, VER " %d %d %d %d %d %d", int(timerstart), int(timer), gold(), tkills(), turncount, cellcount);
    for(int i=0; i<itOrbLightning; i++) fprintf(f, " %d", items[i]);
    for(int i=0; i<43; i++) fprintf(f," %d", kills[i]);
    fprintf(f," %d %d %d %d", int(savetime + timer - timerstart), savecount + 1, cheater, int(cwt.c->land));
    items[itOrbLife] = countMyGolems(); 
    // printf("%d golems saved\n", items[itOrbLife]);
    for(int i=itOrbLightning; i<25; i++) fprintf(f, " %d", items[i]);
//  fprintf(f, " %d", items[itOrbThorns]);
//  fprintf(f, " %d", items[itFernFlower]);
//  for(int i=43; i<47; i++) fprintf(f, " %d", kills[i]);

    items[0] = 0;
    kills[moWormtail] = 0;
    kills[moWormwait] = 0;
    kills[moTentaclewait] = 0;

    fprintf(f, "\n");
    }
  fprintf(f, "Played on: %s - %s (%d turns)\n", sbuf, buf, turncount);
  fprintf(f, "Total wealth: %d\n", gold());
  fprintf(f, "Total enemies killed: %d\n", tkills());
  fprintf(f, "cells generated: %d\n", cellcount);
  fprintf(f, "Number of cells explored, by distance from the player:\n"); 
  for(int i=0; i<10; i++) fprintf(f, " %d", explore[i]); fprintf(f, "\n");
/*for(int j=0; j<landtypes; j++) {
    bool haveland = false;
    for(int i=0; i<10; i++) 
      if(exploreland[i][j]) 
        haveland = true;
    if(haveland)
      for(int i=0; i<10; i++) 
        fprintf(f, " %d", exploreland[i][j]);
    fprintf(f, " %s\n", linf[j].name);
    } */
  if(kills[0]) fprintf(f, "walls melted: %d\n", kills[0]);
  fprintf(f, "heptagons travelled: %d\n", celldist(cwt.c));
  
  fprintf(f, "\n");

  for(int i=0; i<ittypes; i++) if(items[i])  
    fprintf(f, "%4dx %s\n", items[i], iinf[i].name);
    
  fprintf(f, "\n");
  
  for(int i=1; i<motypes; i++) if(kills[i])  
    fprintf(f, "%4dx %s <%d>\n", kills[i], minf[i].name, i);
  
  fprintf(f, "\n\n\n");
  
  printf("Game statistics saved to %s\n", scorefile);
  addMessage(XLAT("Game statistics saved to %1", scorefile));
  fclose(f);
#endif
  }

bool havesave = true;

#ifndef ANDROID
// load the save
void loadsave() {
  printf("Trying to load a save.\n");
  FILE *f = fopen(scorefile, "rt");
  havesave = f;
  if(!f) return;
  score sc;
  bool ok = false;
  while(!feof(f)) {
    char buf[120];
    if(fgets(buf, 120, f) == NULL) break;
    if(buf[0] == 'H' && buf[1] == 'y') {
      if(fscanf(f, "%s", buf) <= 0) break; sc.ver = buf;
      if(sc.ver < "4.4" || sc.ver == "CHEATER!") continue;
      ok = true;
      for(int i=0; i<SCSIZE; i++) {
        if(fscanf(f, "%d", &sc.tab[i]) <= 0) ok = false;
        }
      for(int i=0; i<SCSIZE2; i++) {
        if(fscanf(f, "%d", &sc.tab2[i]) <= 0) ok = false;
        }
      /* if(sc.ver >= "4.9")
      for(int i=0; i<SCSIZE3; i++) {
        if(fscanf(f, "%d", &sc.tab3[i]) <= 0) ok = false;
        } */
      }
    }
  fclose(f);
  if(ok && sc.tab2[4 + itOrbSafety - itOrbLightning]) {
    for(int i=0; i<itOrbLightning; i++)
      items[i] = sc.tab[6+i];
    for(int i=0; i<43; i++)
      kills[i] = sc.tab[6+i+itOrbLightning];
    for(int i=itOrbLightning; i<25; i++)
      items[i] = sc.tab2[4+i-itOrbLightning];
    savetime = sc.tab2[0]; savecount = sc.tab2[1]; cheater = sc.tab2[2];
    firstland = eLand(sc.tab2[3]); timerstart = time(NULL);
    turncount = sc.tab[4]; cellcount = sc.tab[5];

    items[itFernFlower] = items[0];
    kills[moCrystalSage] = kills[moWormwait];
    kills[moFireFairy] = kills[moWormtail];
    items[itOrbThorns] = kills[moTentaclewait];
    items[0] = 0;
    kills[moWormtail] = 0;
    kills[moWormwait] = 0;
    kills[moTentaclewait] = 0;

    safety = true;
    addMessage(XLAT("Game loaded."));
    }
  }
#endif

void restartGame() {
  DEB("savestats");
  achievement_final(true);
  saveStats();
  DEB("clear");
  for(int i=0; i<ittypes; i++) items[i] = 0;
  for(int i=0; i<motypes; i++) kills[i] = 0;
  for(int i=0; i<10; i++) explore[i] = 0;
  // items[itGreenStone] = 100;
  cellcount = 0;
  DEB("clearmem");
  clearMemory();
  DEB("initc");
  initcells();
  DEB("initg");
  initgame();
  canmove = true;
  DEB("restg");
  extern void restartGraph();
  restartGraph();
  extern void resetmusic();
  resetmusic();
  }

void restartGameSwitchEuclid() {
  DEB("savestats");
  achievement_final(true);
  saveStats();
  DEB("clear");
  for(int i=0; i<ittypes; i++) items[i] = 0;
  for(int i=0; i<motypes; i++) kills[i] = 0;
  for(int i=0; i<10; i++) explore[i] = 0;
  // items[itGreenStone] = 100;
  cellcount = 0;
  DEB("clearmem");
  clearMemory();
  euclid = !euclid;
  DEB("initc");
  initcells();
  DEB("initg");
  initgame();
  canmove = true;
  DEB("restg");
  extern void restartGraph();
  restartGraph();
  extern void resetmusic();
  resetmusic();
  }

void clearGameMemory() {
  pathq.clear();
  dcal.clear();
  yii = 0; yi.clear();
  clearshadow();
  }

#ifndef ANDROID
void applyGameBoard(char u, int sym, cell *c) {
  if(!c) c = cwt.c;
  if(sym == SDLK_RETURN) {
    items[c->item]++;
    c->item = itNone;
    }
  if(u == ' ')
    c->item = itNone;

  extern void movepckeydir(int);
  if(sym == 'd' || sym == SDLK_KP6) movepckeydir(0);
  if(              sym == SDLK_KP3) movepckeydir(1);
  if(sym == 'x' || sym == SDLK_KP2) movepckeydir(2);
  if(              sym == SDLK_KP1) movepckeydir(3);
  if(sym == 'a' || sym == SDLK_KP4) movepckeydir(4);
  if(              sym == SDLK_KP7) movepckeydir(5);
  if(sym == 'w' || sym == SDLK_KP8) movepckeydir(6);
  if(              sym == SDLK_KP9) movepckeydir(7);

  if(u == 'g')
    c->item = itGreenStone;
  if(u == 'r')
    c->item = itOrbSpeed;
  if(u == 't')
    c->item = itOrbFlash;
  if(u == 'o')
    c->item = itOrbShield;
  if(u == 'i')
    c->item = itOrbTeleport;
  if(u == 'p')
    c->item = itOrbLightning;
  if(u == 'h')
    c->item = itDiamond;
  if(u == 'j')
    c->item = itHell;
  if(u == 'k')
    c->item = itFernFlower;
  if(u == 'l')
    c->item = itSapphire;
  if(u == '0')
    c->wall = waNone;
  if(u == '1')
    c->wall = waFloorA;
  if(u == '2')
    c->wall = waFloorB;
  if(u == '3')
    c->wall = waFrozenLake;
  if(u == '4')
    c->wall = waCavefloor;
  if(u == '5')
    c->wall = waLake;
  if(u == '6')
    c->wall = waCavewall;
  if(u == '7')
    c->wall = waIcewall;
  if(u == '8')
    c->wall = waDryTree;
  if(u == '9')
    c->wall = waColumn;
  }
#endif

eItem randomOrb() {
  eItem i = eItem(rand() % ittypes);
  if(itemclass(i) == 2) return i;
  else return randomOrb();
  }

eItem randomTreasure() {
  eItem i = eItem(rand() % ittypes);
  if(itemclass(i) == 0) return i;
  else return randomOrb();
  }

void applyCheat(char u, cell *c = NULL) {
  if(u == 'M' && cwt.c->type == 6) {
    addMessage(XLAT("You summon some Mirages!"));
    cheater++;
    createMirrors(cwt.c, cwt.spin, moMirage),
    createMirages(cwt.c, cwt.spin, moMirage);
    }
  if(u == 'G') {
    addMessage(XLAT("You summon a golem!"));
    cheater++;
    int i = cwt.spin;
    if(passable(cwt.c->mov[i], NULL, false, false)) 
      cwt.c->mov[i]->monst = moGolem;
    }
  if(u == 'L') {
    firstland = eLand(firstland+1);
    if(firstland == landtypes) firstland = eLand(2);
    euclidland = firstland;
    cheater++; addMessage(XLAT("You will now start your games in %1", firstland));
    }
  if(u == 'C') {
    cheater++; 
    activateSafety(laCrossroads);
    addMessage(XLAT("Activated the Hyperstone Quest!"));
    for(int i=0; i<itHyperstone; i++) items[i] = 10;
    items[itFernFlower] = 10;
    items[itHyperstone] = 0;
    items[itOrbShield] = 10;
    kills[moYeti] = 20;
    kills[moDesertman] = 20;
    kills[moRunDog] = 20;
    kills[moZombie] = 20;
    kills[moMonkey] = 20;
    kills[moCultist] = 20;
    kills[moTroll] = 20;
    }
  if(u == 'P') {
    for(int i=0; i<ittypes; i++) if(itemclass(eItem(i)) == 2) items[i] = 0;
    cheater++; addMessage(XLAT("Orb power depleted!"));
    }
  if(u == 'O') {
    cheater++; addMessage(XLAT("Orbs summoned!"));
    for(int i=0; i<cwt.c->type; i++) 
      if(passable(cwt.c->mov[i], NULL, false, false))
        cwt.c->mov[i]->item = randomOrb();
    }
  if(u == 'F') {
    items[itOrbFlash] += 4;
    items[itOrbTeleport] += 4;
    items[itOrbLightning] += 4;
    items[itOrbSpeed] += 4;
    items[itOrbShield] += 4;
    cheater++; addMessage(XLAT("Orb power gained!"));
    }
  if(u == 'D') {
    items[itGreenStone] += 10;
    cheater++; addMessage(XLAT("Dead orbs gained!"));
    }
  if(u == 'Y') {
    items[itOrbYendor] ++;
    cheater++; addMessage(XLAT("Orb of Yendor gained!"));
    }
  if(u == 'T') {
    items[randomTreasure()] += 10;
    cheater++; addMessage(XLAT("Treasure gained!"));
    }
  if(u == 'T'-64) {
    items[randomTreasure()] += 100;
    cheater++; addMessage(XLAT("Lots of treasure gained!"));
    }
  if(u == 'W') {
    addMessage(XLAT("You summon a sandworm!"));
    cheater++;
    int i = cwt.spin;
    if(passable(cwt.c->mov[i], NULL, false, false))
      cwt.c->mov[i]->monst = moWorm,
      cwt.c->mov[i]->mondir = NODIR;
    }
  if(u == 'I') {
    addMessage(XLAT("You summon an Ivy!"));
    cheater++;
    int i = cwt.spin;
    int j = cwt.c->spn[i];
    cell* c = cwt.c->mov[i]->mov[(j+3)%cwt.c->mov[i]->type];
    if(passable(c, NULL, false, false)) buildIvy(c, 0, 1);
    }
  if(u == 'E') {
    addMessage(XLAT("You summon a monster!"));
    cheater++;
    int i = cwt.spin;
    if(cwt.c->mov[i]->wall == waChasm)
      cwt.c->mov[i]->wall = waNone;
    if(passable(cwt.c->mov[i], NULL, true, false)) {
      eMonster mo[5] = { moEagle, moPyroCultist, moGhost, moCultist, moTroll };
      cwt.c->mov[i]->monst = mo[rand() % 5];
      }
    }
  if(u == 'H') {
    addMessage(XLAT("You summon some Thumpers!"));
    cheater++;
    for(int i=0; i<cwt.c->type; i++) 
      if(passable(cwt.c->mov[i], NULL, false, false))
        cwt.c->mov[i]->wall = waThumper, cwt.c->mov[i]->tmp = -1;
    }
  if(u == 'B') {
    addMessage(XLAT("You summon a bonfire!"));
    cheater++;
    int i = cwt.spin;
    if(passable(cwt.c->mov[i], NULL, false, false)) 
      cwt.c->mov[i]->wall = waBonfire, cwt.c->mov[i]->tmp = -1;
    }
  if(u == 'Z') {
    cwt.spin++; flipplayer = false;
    cwt.spin %= cwt.c->type;
    }
  if(u == 'J') {
    for(int i=1; i<ittypes; i++) if(itemclass(eItem(i)) == 0) items[i] = 0;
    cheater++; addMessage(XLAT("Treasure lost!"));
    }
  if(u == 'K') {
    for(int i=0; i<motypes; i++) kills[i] += 10;
    cheater++; addMessage(XLAT("Kills gained!"));
    }
  if(u == 'S') {
    activateSafety(cwt.c->land);
    items[itOrbSafety] += 3;
    cheater++; addMessage(XLAT("Activated Orb of Safety!"));
    }
  if(u == 'U') {
    activateSafety(firstland);
    cheater++; addMessage(XLAT("Teleported to %1!", firstland));
    }
  }

