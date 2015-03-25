#define NUMLEADER 19

#define SCORE_UNKNOWN (-1)
#define NO_SCORE_YET (-2)

int currentscore[NUMLEADER];

const char* leadernames[NUMLEADER] = {
  "Score", "Diamonds", "Gold", "Spice", "Rubies", "Elixirs",
  "Shards", "Totems", "Daisies", "Statues", "Feathers", "Sapphires",
  "Hyperstones", "Time to Win", "Turns to Win",
  "Time to 10 Hyperstones-50", "Turns to 10 Hyperstones-50", "Orbs of Yendor",
  "Fern Flowers"
  };

bool haveLeaderboard(int id);

void upload_score(int id, int v);

string achievementMessage[3];
int achievementTimer;

#ifdef STEAM
#include "hypersteam.cpp"
#else
#ifndef ANDROID
void achievement_init() {}
void achievement_close() {}
void achievement_gain(const char*, bool euclideanAchievement) {}
#endif
#endif

void achievement_collection(eItem it, int prevgold, int newgold) {
#ifdef HAVE_ACHIEVEMENTS
  if(cheater) return;
  int q = items[it];

  if(q == 1) {
    if(it == itDiamond) achievement_gain("DIAMOND1");
    if(it == itEmerald) achievement_gain("RUBY1");
    if(it == itHyperstone) achievement_gain("HYPER1");
    if(it == itGold) achievement_gain("GOLD1");
    if(it == itStatue) achievement_gain("STATUE1");
    if(it == itShard) achievement_gain("MIRROR1");
    if(it == itBone) achievement_gain("TOTEM1");
    if(it == itSpice) achievement_gain("SPICE1");
    if(it == itElixir) achievement_gain("ELIXIR1");
    if(it == itHell) achievement_gain("DAISY1");
    if(it == itFeather) achievement_gain("FEATHER1");
    if(it == itSapphire) achievement_gain("SAPPHIRE1");
    if(it == itFernFlower) achievement_gain("FERN1");
    }

  if(q == 10) {
    if(it == itDiamond) achievement_gain("DIAMOND2");
    if(it == itEmerald) achievement_gain("RUBY2");
    if(it == itHyperstone) achievement_gain("HYPER2");
    if(it == itGold) achievement_gain("GOLD2");
    if(it == itStatue) achievement_gain("STATUE2");
    if(it == itShard) achievement_gain("MIRROR2");
    if(it == itBone) achievement_gain("TOTEM2");
    if(it == itSpice) achievement_gain("SPICE2");
    if(it == itElixir) achievement_gain("ELIXIR2");
    if(it == itHell) achievement_gain("DAISY2");
    if(it == itFeather) achievement_gain("FEATHER2");
    if(it == itSapphire) achievement_gain("SAPPHIRE2");
    if(it == itFernFlower) achievement_gain("FERN2");
    }

  if(q == 25) {
    if(it == itDiamond) achievement_gain("DIAMOND3");
    if(it == itEmerald) achievement_gain("RUBY3");
    if(it == itHyperstone) achievement_gain("HYPER3");
    if(it == itGold) achievement_gain("GOLD3");
    if(it == itStatue) achievement_gain("STATUE3");
    if(it == itShard) achievement_gain("MIRROR3");
    if(it == itBone) achievement_gain("TOTEM3");
    if(it == itSpice) achievement_gain("SPICE3");
    if(it == itElixir) achievement_gain("ELIXIR3");
    if(it == itHell) achievement_gain("DAISY3");
    if(it == itFeather) achievement_gain("FEATHER3");
    if(it == itSapphire) achievement_gain("SAPPHIRE3");
    if(it == itFernFlower) achievement_gain("FERN3");
    }

  if(q == 50) {
    if(it == itDiamond) achievement_gain("DIAMOND4");
    if(it == itEmerald) achievement_gain("RUBY4");
    if(it == itHyperstone) achievement_gain("HYPER4");
    if(it == itGold) achievement_gain("GOLD4");
    if(it == itStatue) achievement_gain("STATUE4");
    if(it == itShard) achievement_gain("MIRROR4");
    if(it == itBone) achievement_gain("TOTEM4");
    if(it == itSpice) achievement_gain("SPICE4");
    if(it == itElixir) achievement_gain("ELIXIR4");
    if(it == itHell) achievement_gain("DAISY4");
    if(it == itFeather) achievement_gain("FEATHER4");
    if(it == itSapphire) achievement_gain("SAPPHIRE4");
    if(it == itFernFlower) achievement_gain("FERN4");
    }
  
  if(it == itOrbYendor)
    achievement_gain("YENDOR2");
#endif
  }

void achievement_count(const string& s, int current, int prev) {
#ifdef HAVE_ACHIEVEMENTS
  if(cheater) return;
  if(s == "GOLEM" && current >= 5)
    achievement_gain("GOLEM2");
  if(s == "GOLEM" && current >= 10)
    achievement_gain("GOLEM3");
  if(s == "STAB" && current >= 1)
    achievement_gain("STABBER1");
  if(s == "STAB" && current >= 2)
    achievement_gain("STABBER2");
  if(s == "STAB" && current >= 4)
    achievement_gain("STABBER3");
  if(s == "MIRRORKILL" && current-prev >= 1)
    achievement_gain("MIRRORKILL1");
  if(s == "MIRRORKILL" && current-prev >= 2)
    achievement_gain("MIRRORKILL2");
  if(s == "MIRRORKILL" && current-prev >= 3)
    achievement_gain("MIRRORKILL3");
  if(s == "FLASH" && current-prev >= 1)
    achievement_gain("FLASH1");
  if(s == "FLASH" && current-prev >= 5)
    achievement_gain("FLASH2");
  if(s == "FLASH" && current-prev >= 10)
    achievement_gain("FLASH3");
  if(s == "LIGHTNING" && current-prev >= 1)
    achievement_gain("LIGHTNING1");
  if(s == "LIGHTNING" && current-prev >= 5)
    achievement_gain("LIGHTNING2");
  if(s == "LIGHTNING" && current-prev >= 10)
    achievement_gain("LIGHTNING3");
  if(s == "MIRAGE" && current >= 35)
    achievement_gain("MIRAGE", true);
#endif
  }

int specific_improved = 0;
int specific_what = 0;

void improve_score(int i, eItem what) {
#ifdef HAVE_ACHIEVEMENTS
  if(items[what] && haveLeaderboard(i)) {
    if(items[what] > currentscore[i] && currentscore[i] != SCORE_UNKNOWN) {
      specific_improved++; specific_what = what;
      currentscore[i] = items[what];
      }
      
    upload_score(i, items[what]);
    }
#endif
  }

void achievement_final(bool really_final) {
#ifdef HAVE_ACHIEVEMENTS
  if(cheater) return;
  if(euclid) return;
  int total_improved = 0;
  specific_improved = 0;
  specific_what = 0;
  
  for(int i=1; i<=12; i++) improve_score(i, eItem(i));
  improve_score(17, itOrbYendor);
  improve_score(18, itFernFlower);
  
  int tg = gold();
  if(tg && haveLeaderboard(0)) {
    if(tg > currentscore[0] && currentscore[0] != SCORE_UNKNOWN) {
      if(currentscore[0] < 0) total_improved += 2;
      total_improved++; currentscore[0] = tg;
      }
    upload_score(0, tg);
    }
  
  if(total_improved >= 2) {
    addMessage(XLAT("Your total treasure has been recorded in the "LEADERFULL"."));
    addMessage(XLAT("Congratulations!"));
    }
  else if(total_improved && specific_improved >= 2)
    addMessage(XLAT("You have improved your total high score and %1 specific high scores!", its(specific_improved)));
  else if(total_improved && specific_improved)
    addMessage(XLAT("You have improved your total and '%1' high score!", iinf[specific_what].name));
  else if(total_improved)
    addMessage(XLAT("You have improved your total high score on "LEADER". Congratulations!"));
  else if(specific_improved >= 2)
    addMessage(XLAT("You have improved %1 of your specific high scores!", its(specific_improved)));
  else if(specific_improved)
    addMessage(XLAT("You have improved your '%1' high score on "LEADER"!", iinf[specific_what].name));
#endif
  }

void achievement_victory(bool hyper) {
#ifdef HAVE_ACHIEVEMENTS
  if(cheater) return;
  if(euclid) return;

  int t = savetime + time(NULL) - timerstart;
  
  int ih1 = hyper ? 15 : 13;
  int ih2 = hyper ? 16 : 14;
  
  int improved = 0;
  if(currentscore[ih1] == NO_SCORE_YET || currentscore[ih2] == NO_SCORE_YET)
    improved += 4;
    
  if(currentscore[ih1] < 0 || currentscore[ih1] > t) {
    improved++; currentscore[ih1] = t;
    }

  if(currentscore[ih2] < 0 || currentscore[ih2] > turncount) {
    improved+=2; currentscore[ih2] = turncount;
    }

  if(hyper)
    addMessage(XLAT("You have collected 10 treasures of each type."));

  if(improved) {
    if(improved >= 4) {
      if(!hyper) addMessage(XLAT("This is your first victory!"));
      addMessage(XLAT("This has been recorded in the " LEADERFULL "."));
      addMessage(XLAT("The faster you get here, the better you are!"));
      }
    else if(improved >= 3)
      addMessage(XLAT("You have improved both your real time and turn count. Congratulations!"));
    else if(improved == 1)
      addMessage(XLAT("You have used less real time than ever before. Congratulations!"));
    else if(improved == 2)
      addMessage(XLAT("You have used less turns than ever before. Congratulations!"));
    }
  
  upload_score(ih1, t);
  upload_score(ih2, turncount);
#endif
  }

void achievement_display() {
#ifdef HAVE_ACHIEVEMENTS
#ifdef STEAM
  SteamAPI_RunCallbacks();
#endif
  if(achievementTimer) {
    int col = (ticks - achievementTimer);
    if(col > 5000) { achievementTimer = 0; return; }
    if(col > 2500) col = 5000 - col;
    col /= 10; col *= 0x10101;
    displayfr(vid.xres/2, vid.yres/4, 2, vid.fsize * 2, achievementMessage[0], col & 0xFFFF00, 8);
    displayfr(vid.xres/2, vid.yres/4 + vid.fsize*2, 2, vid.fsize * 2, achievementMessage[1], col, 8);
    displayfr(vid.xres/2, vid.yres/4 + vid.fsize*4, 2, vid.fsize, achievementMessage[2], col, 8);
    }
#endif
  }

