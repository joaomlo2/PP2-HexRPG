// #define CHECKTRANS

#define GEN_M 0
#define GEN_F 1
#define GEN_N 2
#define GEN_O 3

struct stringpar {
  string v;
  stringpar(string s) : v(s) { }
  stringpar(const char* s) : v(s) { }
  stringpar(eMonster m) { v= minf[m].name; }
  stringpar(eLand l) { v= linf[l].name; }
  stringpar(eWall w) { v= winf[w].name; }
  stringpar(eItem i) { v= iinf[i].name; }  
  };

void rep(string& pattern, string what, string to) {
  size_t at = pattern.find(what);
  if(at != string::npos)
    pattern = pattern.replace(at, what.size(), to);
  }

#define NOMAPS
#ifdef NOMAPS
template<class T> struct dictionary {
  vector<pair<string, T> > v;
  bool sorted;
  dictionary() { sorted = true; }
  void add(const string& s, const T& val) { 
    sorted = false;
    v.push_back(make_pair(s,val)); 
    }
  void sortme() {
    sort(v.begin(), v.end());
    sorted = true;
    }
  int seek(const string& s) {
    if(!sorted) sortme();
    int a = 0, b = size(v);
    while(a != b) {
      int m = (a+b)/2;
      if(v[m].first < s) a = m+1;
      else b = m;
      }
    return a;
    }
  int count(const string& s) {
    int i = seek(s);
    return (i < size(v) && v[i].first == s);
    }
  T& operator [] (const string& s) {
    return v[seek(s)].second;
    }
  void clear() { v.clear(); }
  };
#else
#include <map>
template<class T> struct dictionary {
  map<string, T> m;
  void add(const string& s, const T& val) { m[s] = val; }
  T& operator [] (const string& s) { return m[s]; }
  int count(const string& s) { return m.count(s); }
  void clear() { m.clear(); }
  };
#endif

dictionary<string> pl, tr, cz;

struct noun {
  int genus;
  string nom, nomp, acc, abl;
  };

bool operator < (noun x, noun y) { return x.nom < y.nom; }

dictionary<noun> nounspl, nounstr, nounscz;

template<class T> void comparedict(dictionary<T>& d1, dictionary<T>& d2) {
  for(int i=0; i<size(d1.v); i++)
    if(d2.count(d1.v[i].first) == 0)
      printf("Missing: %s\n", d1.v[i].first.c_str());
  }

string choose3(int g, string a, string b, string c) {
  if(g == GEN_M || g == GEN_O) return a;
  if(g == GEN_F) return b;
  if(g == GEN_N) return c;
  return "unknown genus";
  }

string choose4(int g, string a, string b, string c, string d) {
  if(g == GEN_M) return a;
  if(g == GEN_F) return b;
  if(g == GEN_N) return c;
  if(g == GEN_O) return d;
  return "unknown genus";
  }

int playergender();
int lang();

void basicrep(string& x) { 
  // translate to Polish
  if(lang() == 1 && pl.count(x)) {
    x = pl[x];
    rep(x, "%łeś0", choose3(playergender(), "łeś", "łaś", "łoś"));
    rep(x, "%ąłeś0", choose3(playergender(), "ąłeś", "ęłaś", "ęłoś"));
    rep(x, "%ógł0", choose3(playergender(), "ógł", "ogła", "ogło"));
    }

  // translate to Turkish
  else if(lang() == 2 && tr.count(x)) {
    x = tr[x];
    }

  else if(lang() == 3 && cz.count(x)) {
    x = cz[x];
    rep(x, "%l0", choose3(playergender(), "l", "la", "lo"));
    rep(x, "%d0", choose3(playergender(), "", "a", "o"));
    }
  }

void parrep(string& x, string w, stringpar p) {
  if(lang() == 0) {
    rep(x,"%"+w,p.v);
    return;
    }
  if(lang() == 1) {
    if(nounspl.count(p.v)) {
      noun& n(nounspl[p.v]);
      rep(x, "%"+w, n.nom);
      rep(x, "%P"+w, n.nomp);
      rep(x, "%a"+w, n.acc);
      rep(x, "%abl"+w, n.abl);
      rep(x, "%ł"+w, choose3(n.genus, "ł", "ła", "ło"));
      rep(x, "%ął"+w, choose3(n.genus, "ął", "ęła", "ęło"));
      rep(x, "%ya"+w, choose3(n.genus, "y", "a", "e"));
      rep(x, "%yą"+w, choose4(n.genus, "ego", "ą", "e", "y"));
      rep(x, "%oa"+w, choose3(n.genus, "", "a", "o"));
      rep(x, "%ymą"+w, choose3(n.genus, "ym", "ą", "ym"));
      }
    else {
      rep(x,"%"+w,p.v);
      rep(x, "%P"+w, p.v);
      rep(x, "%a"+w, p.v);
      rep(x, "%abl"+w, p.v);
      rep(x, "%ł"+w, choose3(0, "ł", "ła", "ło"));
      }
    }
  if(lang() == 2) {
    if(nounstr.count(p.v)) {
      noun& n(nounstr[p.v]);
      rep(x, "%"+w, n.nom);
      rep(x, "%P"+w, n.nomp);
      rep(x, "%a"+w, n.acc);
      rep(x, "%abl"+w, n.abl);
      }
    else {
      rep(x,"%"+w,p.v);
      rep(x, "%P"+w, p.v);
      rep(x, "%a"+w, p.v);
      rep(x, "%abl"+w, p.v);
      }
    }
  if(lang() == 3) {
    if(nounscz.count(p.v)) {
      noun& n(nounscz[p.v]);
      rep(x, "%"+w, n.nom);
      rep(x, "%P"+w, n.nomp);
      rep(x, "%a"+w, n.acc);
      rep(x, "%abl"+w, n.abl);
      rep(x, "%ý"+w, choose3(n.genus, "ý", "á", "é"));
      rep(x, "%l"+w, choose3(n.genus, "l", "la", "lo"));
      rep(x, "%ůj"+w, choose4(n.genus, "ého", "ou", "é", "ůj"));
      rep(x, "%ým"+w, choose3(n.genus, "ým", "ou", "ým"));
      if(p.v == "Mirror Image")
        rep(x, "%s"+w, "se");
      if(p.v == "Mirage")
        rep(x, "%s"+w, "s");
      }
    else {
      rep(x,"%"+w,p.v);
      rep(x, "%P"+w, p.v);
      rep(x, "%a"+w, p.v);
      rep(x, "%abl"+w, p.v);
//    rep(x, "%ł"+w, choose3(0, "ł", "ła", "ło"));
      }
    }
  }

void postrep(string& s) {
  }

string XLAT(string x) { 
  basicrep(x);
  postrep(x);
  return x;
  }
string XLAT(string x, stringpar p1) { 
  basicrep(x);
  parrep(x,"1",p1.v);
  postrep(x);
  return x;
  }
string XLAT(string x, stringpar p1, stringpar p2) { 
  basicrep(x);
  parrep(x,"1",p1.v);
  parrep(x,"2",p2.v);
  postrep(x);
  return x;
  }

string XLATN(string x) { 
  if(lang() == 0) return x;
  if(lang() == 1) {
    if(!nounspl.count(x)) return x;
    return nounspl[x].nomp;
    }
  if(lang() == 2) {
    if(!nounstr.count(x)) return x;
    return nounstr[x].nomp;
    }
  if(lang() == 3) {
    if(!nounscz.count(x)) return x;
    return nounscz[x].nomp;
    }
  return x;
  }

string XLAT1(string x) { 
  if(lang() == 0) return x;
  if(lang() == 1) {
    if(!nounspl.count(x)) return x;
    return nounspl[x].nom;
    }
  if(lang() == 2) {
    if(!nounstr.count(x)) return x;
    return nounstr[x].nomp;
    }
  if(lang() == 3) {
    if(!nounscz.count(x)) return x;
    return nounscz[x].nomp;
    }
  return x;
  }

#ifdef CHECKTRANS
#include <set>

void addutftoset(set<string>& s, string& w) {
  int i = 0;
//printf("%s\n", w.c_str());
  while(i < size(w)) {
  
    if(w[i] < 0) {
      string z = w.substr(i, 2);
//    printf("Insert: %s [%02x%02x]\n", z.c_str(), w[i], w[i+1]);
      s.insert(w.substr(i, 2));
      i += 2;
      }
    else {
      s.insert(w.substr(i, 1));
      i++;
      }
    }
  }

void addutftoset(set<string>& s, noun& w) {
  addutftoset(s, w.nom);
  addutftoset(s, w.nomp);
  addutftoset(s, w.acc);
  addutftoset(s, w.abl);
  }

template<class T>
void addutftoset(set<string>& s, dictionary<T>& w) {
  for(int i=0; i<size(w.v); i++)
    addutftoset(s, w.v[i].second);
  }

set<string> allchars;

void printletters(dictionary<string>& la, dictionary<noun>& nounla, const char *lang) {
  set<string> s;
  addutftoset(s, la);
  addutftoset(s, nounla);
  addutftoset(allchars, la);
  addutftoset(allchars, nounla);
  printf("%s:", lang);
  for(set<string>::iterator it = s.begin(); it != s.end(); it++)
    printf(" \"%s\",", it->c_str());
  printf("\n");
  }
#endif

void initlanguage() {

  pl.clear(); nounspl.clear();
  cz.clear(); nounscz.clear();
  tr.clear(); nounstr.clear();

#define S(a,b) pl.add(a,b);
#define N(a,b,c,d,e,f) \
  {noun n; n.genus = b; n.nom = c; n.nomp = d; n.acc = e; n.abl = f; nounspl.add(a,n);}
#include "language-pl.cpp"
#undef N
#undef S

#define S(a,b) tr.add(a,b);
#define N5(a,b,c,d,e) \
  {noun n; n.genus = b; n.nom = c; n.nomp = d; n.acc = e; n.abl = e; nounstr.add(a,n);}
#define N(a,b,c,d,e,f) \
  {noun n; n.genus = b; n.nom = c; n.nomp = d; n.acc = e; n.abl = f; nounstr.add(a,n);}
#include "language-tr.cpp"
#undef N
#undef S

#define S(a,b) cz.add(a,b);
#define N(a,b,c,d,e,f) \
  {noun n; n.genus = b; n.nom = c; n.nomp = d; n.acc = e; n.abl = f; nounscz.add(a,n);}
#include "language-cz.cpp"
#undef N
#undef S

#ifdef CHECKTRANS
  // verify
  comparedict(pl, cz);
  comparedict(cz, pl);

  comparedict(nounspl, nounscz);
  comparedict(nounscz, nounspl);
  
  printletters(pl, nounspl, "Polish");
  printletters(tr, nounstr, "Turkish");
  printletters(cz, nounscz, "Czech");

  int c =0;
  printf("ALL:");
  for(set<string>::iterator it = allchars.begin(); it != allchars.end(); it++) {
    printf(" \"%s\",", it->c_str());
    if(size(*it) == 2) c++;
    }
  printf("\n");
  printf("c = %d\n", c);
  
  exit(0);
#endif
  }

