// --- help ---

#define MC
#define M

const char *wormdes = 
MC"These huge monsters normally live below the sand, but your movements have "
MC"disturbed them. They are too big to be slain with your "
MC"weapons, but you can defeat them by making them unable to move. "
M "This also produces some Spice. They move two times slower than you.";

const char *cocytushelp =
M  "This frozen lake is a hellish version of the Icy Land. Now, your body heat melts the floor, not the walls.";

const char *tentdes = 
MC"The tentacles of Cthulhu are like sandworms, but longer. "
M "They also withdraw one cell at a time, instead of exploding instantly.";

const char *gameboardhelp = 
MC"Ever wondered how some boardgame would look on the hyperbolic plane? "
MC"I wondered about Go, so I have created this feature. Now you can try yourself!\n"
MC"Enter = pick up an item (and score), space = clear an item\n"
M "Other keys place orbs and terrain features of various kinds\n";

const char *ivydes = 
MC"A huge plant growing in the Jungle. Each Ivy has many branches, "
MC"and one branch grows per each of your moves. Branches grow in a clockwise "
M "order. The root itself is vulnerable.";

const char *slimehelp =
MC"The Alchemists produce magical potions from pools of blue and red slime. You "
MC"can go through these pools, but you cannot move from a blue pool to a red "
MC"pool, or vice versa. Pools containing items count as colorless, and "
MC"they change color to the PC's previous color when the item is picked up. "
MC"Slime beasts also have to keep to their own color, "
MC"but when they are killed, they explode, destroying items and changing "
M "the color of the slime and slime beasts around them.";

const char *gdemonhelp =
MC  "These creatures are slow, but very powerful... more powerful than you. "
MC  "You need some more experience in demon fighting before you will be able to defeat them. "
MC  "Even then, you will be able to slay this one, but more powerful demons will come...\n\n"
MC  "Each 10 lesser demons you kill, you become powerful enough to kill all the greater "
M   "demons on the screen, effectively turning them into lesser demons.";    

const char *ldemonhelp =
M   "These creatures are slow, but they often appear in large numbers.";

const char *trollhelp =
MC"A big monster from the Living Caves. A dead Troll will be reunited "
M "with the rocks, causing some walls to grow around its body.";

const char *barrierhelp = 
M "Huge, impassable walls which separate various lands.";

const char *cavehelp =
MC"This cave contains walls which are somehow living. After each turn, each cell "
MC"counts the number of living wall and living floor cells around it, and if it is "
MC"currently of a different type than the majority of cells around it, it switches. "
MC"Items count as three floor cells, and dead Trolls count as five wall cells. "
M "Some foreign monsters also count as floor or wall cells.\n";

const char *foresthelp =
MC"This forest is quite dry. Beware the bushfires!\n"
MC"Trees catch fire on the next turn. The temperature of the grass cells "
MC"rises once per turn for each fire nearby, and becomes fire itself "
MC"when its temperature has risen 10 times.\n"
M "You can also cut down the trees. Big trees take two turns to cut down.";

// --- monsters ---

const int motypes = 45;

struct monstertype {
  char  glyph;
  int   color;
  const char *name;
  const char *help;
  };

monstertype minf[motypes] = {
  { 0,   0,        "none"       , NULL},
  { 'Y', 0x4040FF, "Yeti"       ,
M   "A big and quite intelligent monster living in the Icy Land."
  },
  { 'W', 0xD08040, "Icewolf"    ,
MC  "A nasty predator from the Icy Land. Contrary to other monsters, "
M   "it tracks its prey by their heat."
  },
  { 'W', 0xD08040, "Icewolf"    , ""},
  { 'R', 0xFF8000, "Ranger"     ,
MC  "Rangers take care of the magic mirrors in the Land of Mirrors. "
MC  "They know that rogues like to break these mirrors... so "
M   "they will attack you!"
    },
  { 'T', 0xD0D0D0, "Rock Troll", trollhelp},
  { 'G', 0x20D020, "Goblin",     
MC  "A nasty creature native to the Living Caves. They don't like you "
M   "for some reason."
    },
  { 'S', 0xE0E040, "Sand Worm", wormdes },
  { 's', 0x808000, "Sand Worm Tail", wormdes },
  { 'S', 0x808000, "Sand Worm W", wormdes },
  { '@', 0xC00040, "Hedgehog Warrior", 
MC  "These warriors of the Forest wield exotic weapons called hedgehog blades. "
MC  "These blades protect them from a frontal attack, but they still can be 'stabbed' "
M   "easily by moving from one place next to them to another."
    },
  { 'M', 0x505050, "Desert Man",
M   "A tribe of men native to the Desert. They have even tamed the huge Sandworms, who won't attack them."},
  { 'C', 0x00FFFF, "Ivy Root", ivydes},
  { 'C', 0xFFFF00, "Active Ivy", ivydes},
  { 'C', 0x40FF00, "Ivy Branch", ivydes},
  { 'C', 0x006030, "Dormant Ivy", ivydes},
  { 'C', 0x800000, "Ivy N", ivydes},
  { 'C', 0x800000, "Ivy D", ivydes},
  { 'M', 0x804000, "Giant Ape",
M   "This giant ape thinks that you are an enemy."},
  { 'B', 0x909000, "Slime Beast", slimehelp},
  { '@', 0x8080FF, "Mirror Image",
M   "A magical being which copies your movements."
    },
  { '@', 0xFF8080, "Mirage",
MC  "A magical being which copies your movements. "
M   "You feel that it would be much more useful in an Euclidean space."
    },
  { '@', 0x509050, "Golem",
M   "You can summon these friendly constructs with a magical process."
    },
  { '@', 0x509050, "Golem",
M   "You can summon these friendly constructs with a magical process."
    },
  { 'E', 0xD09050, "Eagle",
M   "A majestic bird, who is able to fly very fast."
    },
  { 'S', 0xFF8080, "Seep",
M   "A monster who is able to live inside the living cave wall."
    },
  { 'Z', 0x804000, "Zombie",
M   "A typical Graveyard monster."
    },
  { 'G', 0xFFFFFF, "Ghost",
M   "A typical monster from the Graveyard, who moves through walls."
    },
  { 'N', 0x404040, "Necromancer",
M   "Necromancers can raise ghosts and zombies from fresh graves."
    },
  { 'S', 0x404040, "Shadow",
M   "A creepy monster who follows you everywhere in the Graveyard."
    },
  { 'T', 0x40E040, "Tentacle", tentdes },
  { 't', 0x008000, "Tentacle Tail", tentdes },
  { 'T', 0x008000, "Tentacle W", tentdes },
  { 'z', 0xC00000, "Tentacle (withdrawing)", tentdes },
  { 'P', 0xFF8000, "Cultist", 
M   "People worshipping Cthulhu. They are very dangerous." 
    },
  { 'P', 0xFFFF00, "Fire Cultist", 
MC  "People worshipping Cthulhu. This one is especially dangerous, "
M   "as he is armed with a weapon which launches fire from afar."
    },
  { 'D', 0xFF0000, "Greater Demon", gdemonhelp},
  { 'D', 0x800000, "Greater Demon", gdemonhelp},
  { 'd', 0xFF2020, "Lesser Demon", ldemonhelp},
  { 'd', 0x802020, "Lesser Demon", ldemonhelp},
  { 'S', 0x2070C0, "Ice Shark", 
M   "This dangerous predator has killed many people, and has been sent to Cocytus."
    },
  { 'W', 0xFFFFFF, "Running Dog", 
MC  "This white dog is able to run all the time. It is the only creature "
M   "able to survive and breed in the Land of Eternal Motion."
    },
  { 'S', 0xC00040, "Demon Shark", 
MC  "Demons of Hell do not drown when they fall into the lake in Cocytus. "
M   "They turn into demonic sharks, enveloped in a cloud of steam."
    },
  { 'S', 0xC00040, "Fire Fairy", 
MC  "These fairies would rather burn the forest, than let you get some Fern Flowers. "
MC  "The forest is infinite, after all...\n\n"
M   "Fire Fairies transform into fires when they die."
    },
  { 'C', 0x4000C0, "Crystal Sage", 
MC  "This being radiates an aura of wisdom. "
MC  "It is made of a beautiful crystal, you would love to take it home. "
MC  "But how is it going to defend itself? Better not to think of it, "
MC  "thinking causes your brain to go hot...\n\n"
M   "Crystal Sages melt at -30 °C, and they can rise the temperature around you from afar."
    }
  };

enum eMonster { 
  moNone, 
  moYeti, moWolf, moWolfMoved, 
  moRanger, 
  moTroll, moGoblin, 
  moWorm, moWormtail, moWormwait, moHedge, 
  moDesertman, 
  moIvyRoot, moIvyHead, moIvyBranch, moIvyWait, moIvyNext, moIvyDead,
  moMonkey,
  moSlime,
  moMirror, moMirage, moGolem, moGolemMoved,
  moEagle, moSeep,
  moZombie, moGhost, moNecromancer, moShadow,
  moTentacle, moTentacletail, moTentaclewait, moTentacleEscaping, 
  moCultist, moPyroCultist,
  moGreater, moGreaterM, moLesser, moLesserM,
  moShark, moRunDog, moGreaterShark, moFireFairy,
  moCrystalSage
  };

// --- items ----

const int ittypes = 27;

struct itemtype {
  char  glyph;
  int   color;
  const char *name;
  const char *help;
  };

itemtype iinf[ittypes] = {
  { 0,   0,        "none",       NULL},
  { '*', 0xFFFFFF, "Ice Diamond", 
M   "Cold white gems, found in the Icy Land."
    },
  { '$', 0xFFD700, "Gold",
MC  "An expensive metal from the Living Caves. For some reason "
M   "gold prevents the living walls from growing close to it."
    },
  { ';', 0xFF4000, "Spice",      
MC  "A rare and expensive substance found in the Desert. "
M   "It is believed to extend life and raise special psychic powers."
    },
  { '*', 0xC00000, "Ruby",
M   "A beautiful gem from the Jungle."
    },
  { '!', 0xFFFF00, "Elixir of Life",
MC  "A wonderful beverage, apparently obtained by mixing red and blue slime. You definitely feel more "
M   "healthy after drinking it, but you still fell that one hit of a monster is enough to kill you."},
  { '%', 0xFF00FF, "Shard",
MC  "A piece of a magic mirror, or a mirage cloud, that can be used for magical purposes. Only mirrors and clouds "
M   "in the Land of Mirrors leave these."},
  { '/', 0xFF8000, "Necromancer's Totem",
M   "These sinister totems contain valuable gems."},
  { '%', 0x00D000, "Demon Daisy",
M   "These star-shaped flowers native to Hell are a valuable alchemical component."},
  { '/', 0x00FF00, "Statue of Cthulhu",
M   "This statue is made of materials which cannot be found in your world."},
  { '*', 0xFF8000, "Phoenix Feather", 
M   "One of few things that does not cause the floor in the Land of Eternal Motion to collapse. Obviously they are quite valuable."
    },
  { '*', 0x8080FF, "Ice Sapphire", 
M   "Cold blue gems, found in the Cocytus."
    },
  { '*', 0xEEFF20, "Hyperstone", 
M   "These bright yellow gems can be found only by those who have mastered the Crossroads."
    },
  { '[', 0x8080FF, "Key", 
MC  "That's all you need to unlock the Orb of Yendor! Well... as long as you are able to return to the Orb that this key unlocks...\n\n"
M   "Each key unlocks only the Orb of Yendor which led you to it."
    },
  { 'o', 0x306030, "Dead Orb",
MC  "These orbs can be found in the Graveyard. You think that they were once powerful magical orbs, but by now, their "
M   "power is long gone. No way to use them, you could as well simply drop them...\n\n"
    },
  { 'o', 0xFF20FF, "Orb of Yendor",
MC  "This wonderful Orb can only be collected by those who have truly mastered this hyperbolic universe, "
MC  "as you need the right key to unlock it. Luckily, your psychic abilities will let you know "
M   "where the key is after you touch the Orb." },
  { 'o', 0xFFFF00, "Orb of Storms",
M   "This orb can be used to invoke the lightning spell, which causes lightning bolts to shoot from you in all directions."},
  { 'o', 0xFFFFFF, "Orb of Flash",
M   "This orb can be used to invoke a flash spell, which destroys almost everything in radius of 2."},
  { 'o', 0x8080FF, "Orb of Winter",
M   "This orb can be used to invoke a wall of ice. It also protects you from fires."},
  { 'o', 0xFF6060, "Orb of Speed",
M   "This orb can be used to move faster for some time."},
  { 'o', 0x90A090, "Orb of Life",
M   "This orb can be used to summon friendly golems. It is used instantly when you pick it up."},
  { 'o', 0x60D760, "Orb of Shielding",
M   "This orb can be protect you from damage."},
  { 'o', 0x606060, "Orb of Digging",
M   "This orb lets you go through the living wall. Only works in the Living Caves."},
  { 'o', 0x20FFFF, "Orb of Teleport",
MC  "This orb lets you instantly move to another location on the map. Just click a location which "
M   "is not next to you to teleport there. "
    },
  { 'o', 0xA0FF40, "Orb of Safety",
MC  "This orb lets you instantly move to a safe faraway location. Knowing the nature of this strange world, you doubt "
MC  "that you will ever find the way back...\n\n"
MC  "Your game will be saved if you quit the game while the Orb of Safety is still powered.\n\n"
M   "Technical note: as it is virtually impossible to return, this Orb recycles memory used for the world so far (even if you do not use it to save the game). "
    },
  { 'o', 0x40C000, "Orb of Thorns",
M   "This orb allows attacking Hedgehog Warriors directly, as well as stabbing other monsters.\n"
    },
  { '%', 0x0000FF, "Fern Flower",
M   "This flower brings fortune to the person who finds it.\n"
    }
  };

enum eItem { itNone, itDiamond, itGold, itSpice, itEmerald, itElixir, itShard, itBone, itHell, itStatue,
  itFeather, itSapphire, itHyperstone, itKey,
  itGreenStone, itOrbYendor,
  itOrbLightning, itOrbFlash, itOrbWinter, itOrbSpeed, itOrbLife, itOrbShield, itOrbDigging,
  itOrbTeleport, itOrbSafety,
  itOrbThorns, itFernFlower
  };

// --- wall types ---

const int walltypes = 25;

struct walltype {
  char  glyph;
  int   color;
  const char *name;
  const char *help;
  };

const char *lakeDesc = "Hell has these lakes everywhere... They are shaped like evil stars, and filled with burning sulphur.";

walltype winf[walltypes] = {
  { '.', 0xFF00FF, "none",        NULL},
  { '#', 0x8080FF, "ice wall",    
M   "Ice Walls melt after some time has passed."
    },
  { '#', 0xC06000, "great wall",  barrierhelp},
  { '+', 0x900030, "red slime",  slimehelp },
  { '+', 0x300090, "blue slime",   slimehelp },
  { '#', 0xA0D0A0, "living wall", cavehelp},
  { '.', 0x306060, "living floor",cavehelp},
  { '#', 0xD03030, "dead troll",  trollhelp},
  { '#', 0xCDA98F, "sand dune",   
M "A natural terrain feature of the Desert."
  },
  { '%', 0xC0C0FF, "Magic Mirror",
M "You can go inside the Magic Mirror, and produce some mirror images to help you."
  },
  { '%', 0xFFC0C0, "Cloud of Mirage",
MC "Tiny droplets of magical water. You see images of yourself inside them. "
M  "Go inside the cloud, to make these images help you."},
  { '^', 0x8D694F, "Thumper",
M    "A device that attracts sandworms and other enemies. You need to activate it."},
  { '^', 0x804000, "Bonfire",
M   "A heap of wood that can be used to start a fire. Everything is already here, you just need to touch it to fire it."
    },
  { '+', 0xC0C0C0, "ancient grave", 
M "An ancient grave." 
    },
  { '+', 0xD0D080, "fresh grave",
M "A fresh grave. Necromancers like those." 
    },
  { '#', 0x00FFFF, "column",
M "A piece of architecture typical to R'Lyeh." 
   },
  { '=', 0xFFFF00, "lake of sulphur", lakeDesc },
  { '=', 0xFFFF00, "lake of sulphur", lakeDesc },
  { '=', 0x000080, "lake", 
M "An impassable lake in Cocytus."
    },
  { '_', 0x000080, "frozen lake", cocytushelp },
  { '>', 0x101010, "chasm",
M "It was a floor... until something walked on it." 
    },
  { '>', 0x101010, "chasmD",
M "It was a floor... until something walked on it." 
    },
  { '#', 0x60C000, "big tree", foresthelp},
  { '#', 0x00C000, "tree", foresthelp}
  };

enum eWall { waNone, waIcewall, waBarrier, waFloorA, waFloorB, waCavewall, waCavefloor, waDeadTroll, waDune,
  waMirror, waCloud, waThumper, waBonfire, waAncientGrave, waFreshGrave, waColumn, waSulphurC, waSulphur,
  waLake, waFrozenLake, waChasm, waChasmD, waDryTree, waWetTree, waTemporary};

// --- land types ---

const int numLands = 13;
const int landtypes = numLands + 3;

struct landtype {
  int color;
  const char *name;
  const char *help;
  };

landtype linf[landtypes] = {
  { 0xFF00FF, "???"          , ""},
  { 0xC06000, "Great Wall"   , ""},
  { 0xFF0000, "Crossroads"   , 
MC  "This land is a quick gateway to other lands. It is very easy to find other lands "
MC  "from the Crossroads. Which means that you find monsters from most other lands here!\n\n"
MC  "As long as you have found enough treasure in their native lands, you can "
MC  "find magical items in the Crossroads. Mirror Land brings mirrors and clouds, "
MC  "and other land types bring magical orbs.\n\n"
MC  "A special treasure, Hyperstone, can be found on the Crossroads, but only "
MC  "after you have found 10 of every other treasure."
    },
  { 0xCDA98F, "Desert",
M   "A hot land, full of sand dunes, mysterious Spice, and huge and dangerous sand worms."
    },
  { 0x8080FF, "Icy Land",
MC  "A very cold land, full of ice walls. Your mere presence will cause these ice walls to "
M   "melt, even if you don't want it."
    },
  { 0x306060, "Living Cave", cavehelp},
  { 0x00C000, "Jungle",
M   "A land filled with huge ivy plants and dangerous animals."
    },
  { 0x900090, "Alchemist Lab", slimehelp},
  { 0x704070, "Mirror Land",
M   "A strange land which contains mirrors and mirages, protected by Mirror Rangers."},
  { 0x404070, "Graveyard",
MC  "All the monsters you kill are carried to this strange land, and buried. "
M   "Careless Rogues are also carried here..."
    },
  { 0x00FF00, "R'Lyeh",
M   "An ancient sunken city which can be reached only when the stars are right."
    },
  { 0xC00000, "Hell",
M   "A land filled with demons and molten sulphur. Abandon all hope ye who enter here!"
    },
  { 0x00FF00, "Cocytus",
    cocytushelp
    },
  { 0xFFFF00, "Land of Eternal Motion",
MC  "A land where you cannot stop, because every piece of floor is extremely unstable. Only monsters who "
MC  "can run forever are able to survive there, and only phoenix feathers are so light that they do not disturb "
M   "the floor.\n"
    },
  { 0x008000, "Dry Forest", foresthelp},
  { 0xE08020, "Game Board", gameboardhelp}
  };

enum eLand { laNone, laBarrier, laCrossroads, laDesert, laIce, laCaves, laJungle, laAlchemist, laMirror, laGraveyard,
  laRlyeh, laHell, laCocytus, laMotion, laDryForest, laGameBoard };

// cell information for the game

struct gcell {
  eWall wall : 5;
  eItem item : 5;
  eLand land : 4;
  eMonster monst : 6;
  eLand barleft : 4, barright : 4;
  unsigned cpdist : 5, mpdist : 5, pathdist : 10;
  unsigned bardir : 3;
  unsigned mondir : 3;
  unsigned ligon : 1;
  short tmp;
  float heat;
  };

#define NODIR 7
