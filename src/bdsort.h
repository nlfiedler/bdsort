//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

/*
 * Constants
 */
#define MBYTE 1048576
#define LOCHAR0 32
#define HICHAR0 255
#define MAXPHASES 13
#define MAXREPS 12
#define MKQCUT 13
#define MED9CUT 31

// TODO: enum these?
#define OFF 1
#define ADD 2
#define GROW 3
#define SORT 4
#define MERGE 5
#define READ 6
#define WRITE 7
#define TFREAD 8
#define TFWRITE 9
#define ALL 10

/*
 * Type definitions
 */
#define UINT unsigned int
#define ULL unsigned long long int

typedef char *string;

typedef struct {
	string s;
	int *p, v[30], n, lo, hi;
} alist;

typedef struct {
	int min, max;
	int nv, ls;
} series;

typedef struct {
	int ph, rep;
	clock_t cl, tm[MAXPHASES][MAXREPS];
} timer;

typedef struct {
	string *p;
	int n, d;
} run;

typedef struct {
	ULL fb, mb, n;
	int bz, sl;
	FILE *f;
	char *b0, *b1, *b, *bl, *b2;
} bfile;

typedef struct {
	ULL *sg, *ln;
	int ns, *r;
} segfile;

typedef struct snk {
	string s;
	struct snk *p;
} sink;

typedef struct {
	int ct;
	string ap, lp;
	short int ns, sn;
	sink *sp;
} bin, *bnp, **ndp;

typedef struct {
	int ct;
	string ap, lp;
} mbin, *mbnp, **mndp;

typedef struct {
	char key[10], num[10], txt[80];
} record;

typedef void (*hsort)(string *p0, int n, int d);

/*
 * Macro definitions
 */
#define MIN(x, y) ((x<y)?x:y)
#define MIN3(x, y, z) ((x<y)?MIN(x, z):MIN(y, z))
#define MAX(x, y) ((x>y)?x:y)
#define UP(X, Y) ((ULL)(X)*(ULL)(Y))
#define DP(X, Y) ((double)(X)*(double)(Y))
#define DR(X, Y) ((double)(X)/(double)(Y))
#define DPR(X, Y, z) (((double)(X)*(double)(Y))/(double)(z))
#define DOT say(".")

#define MSEC(TM) DPR(TM, 1000, CLOCKS_PER_SEC)
#define TIME(X) {clock_t T1; T1=clock()-TMR.cl; TMR.tm[TMR.ph][TMR.rep]+=T1; \
		TMR.ph=X; TMR.cl=clock();}
#define TON {TIME(OFF); TMR.tm[ALL][TMR.rep=0]-=clock();}
#define TNEXTREP {TMR.tm[ALL][TMR.rep]+=clock(); TMR.tm[ALL][++TMR.rep]-=clock();}
#define TOFF {TIME(OFF); TMR.tm[ALL][TMR.rep]+=clock();}
#define TADD TIME(ADD)
#define TGRO TIME(GROW)
#define TSRT TIME(SORT)
#define TREA TIME(READ)
#define TWRI TIME(WRITE)
#define TTFR TIME(TFREAD)
#define TTFW TIME(TFWRITE)
#define TMER TIME(MERGE)

#define NOTEMEM if (MEM>MAXMEM) MAXMEM=MEM;
#define NOTENODES if (NODES>MAXNODES) MAXNODES=NODES;
#define NOTENBINS if (NBINS>MAXNBINS) MAXNBINS=NBINS;
#define NOTEVBINS if (VBINS>MAXVBINS) MAXVBINS=VBINS;
#define NOTESINKS if (SINKS>MAXSINKS) MAXSINKS=SINKS;
#define NOTESBINS if (SBINS>MAXSBINS) MAXSBINS=SBINS;
#define NOTEFBINS if (FBINS>MAXFBINS) MAXFBINS=FBINS;
#define TALLY {NOTEMEM; NOTENODES; NOTENBINS;			\
		NOTEVBINS; NOTESBINS; NOTESINKS; NOTEFBINS;}
#define CLEARTALLIES NBYTES=NKEYS=TFBYTES=OBYTES=MEM=MAXMEM=NODES=NBINS=VBINS=BINS= \
		SINKS=SBINS=FBINS=MAXNODES=MAXNBINS=MAXVBINS=MAXSINKS=MAXSBINS=MAXFBINS=0;
#define SMALLOC(X, Y, z) {int W; W=(z); X=(Y)malloc(W); MEM+=W;}
#define SCALLOC(X, Y, z, W) {int V; V=(z)*(W); {X=(Y)calloc((z), (W)); MEM+=V;}}
#define FREE(x, y) {NOTEMEM; MEM-=(y); free(x); x=NULL;}
#define FREEVBIN(X, Y) NOTEVBINS; --VBINS; FREE(X, Y);
#define NULLCT(ND) (((int*)ND)[1])

/*
 * Global variables!
 */
int ORDERIN, WRITEFILE, NREPS;
char INDIR[100], OUTDIR[100], IFP[100], TFP[100], OFP[100], DATANAM[20], SORTNAM[20];
timer TMR;
double GNUMBS;
series *SER, SER0;

int PAD, PAD2, NODESIZE, IOSIZE, SINKSIZE, HALFCACHE, CACHESIZE, Z[20], BULV, MAXLV;
int MINMB, MAXMB, NVALS, LOGSER;
ULL NKEYS, NBYTES, TFBYTES, OBYTES, FBYTES;
int NCHARS, LOCHAR, HICHAR, DELIM;

int MAXCT, NSEGS, NSEGS0, NSEGS1, *CNT;
string PFX, *TP0, **AD;

int MEM, MAXMEM, MLIM;
int NODES, MAXNODES, NLIM1, NLIM;
int BINS, MAXBINS, BLIM;
int NBINS, MAXNBINS;
int VBINS, MAXVBINS;
int SBINS, MAXSBINS, SINKS, MAXSINKS;
int FBINS, MAXFBINS, FLIM, TFLIM;

/*
 * burst-merge sort
 */
int bmsDo(int fsz);
int bmsAddFile(int fsz, int rep);
void bmsMergeRuns(int nr, int sz);
int bmsAddKeysD(mndp rt, string b, string bl, int *cnt);
int bmsBurst(mndp nd, int c, string bp, string s, int sz);
void bmsSrt(mndp nd, bfile *bf, int phase, int delim);
mndp bmsNuNode(void);

/*
 * Say functions
 */
void cr(void);
void say(string s);
void sayln(string s);
void isay(int i);
void ullsay(ULL i);
void ullsaye(ULL i, string s);
void dsay(double dv, int n);
void dsaye(double dv, string s);
void isaye(int i, string s);
void br(void);
void brp(string s);

/*
 * Sort functions
 */
string *med3(string *p, string *b, string *c, int d);
void insSort(string *p, int n, int d);
void mkqSort(string *p, int n, int d);
void gnuDo(int fsz);

/*
 * File functions
 */
FILE *getf(string fp, string md, string lc);
bfile *tbf(bfile *bf, int bz, int sl);
bfile *nubf(string fp, string mode, int bz, int sl);
bfile *nubfin(string fp, string mode, int bz, int sl, int delim, int fsz);
bfile *nubfout(string fp, string mode, int bz, int sl);
void bfflush(bfile *bf);
void bfclose(bfile *bf);
void bfclear(bfile *bf);
int bfcmp(bfile *bf1, bfile *bf2, int delim);
string bfmoves(bfile *ibf, bfile *obf, int idl, int odl);
segfile *bfsegs(bfile *bf, int ns);
void bfkillsegs(segfile *sf);

/*
 * Burst distribution sort
 */
int bdsDo(int fsz);
int bdsAddFile(int fsz, int rep);
int bdsAddKeysD(ndp rt, string b, string bl, int *cnt);
int bdsAddKeys(ndp rt, string b, string bl);
int bdsBurst(ndp nd, int c, string bp, string s, int sz);
int bdsSrt(ndp nd);
ndp bdsAddSink(bnp bn);
ndp bdsAddTFile(bnp bn);
ndp bdsNuNode(void);
bnp bdsNuSBin(bnp *HP, string bp, string lp, string s);
bnp bdsNuFBin(bnp *HP, bnp bn);
void bdsGroBin(bnp bn, string s);
int bdsGetMem(bnp *HP, int sz);
void bdsHeapCut(bnp *HP, bnp bn);

/*
 * Main functions
 */
int main(int argc, char *argv[]);
void runSort(int dsn, int snum, int fsz);
void titles(void);
void inlist(void);
void trielist(void);
void outlist(void);
void serset(series *ser, int minMB, int maxMB, int nvals, int logser);
ULL serval(series *ser, int i);
void setArg(alist *al, string s, int *p, int lo, int def, int hi);
alist *loadArg(string *tok, int n, alist *al, alist *last);
void resetTimers(int nt, int nreps);
void sortTimer(int ph);
double medMS(int ph);
int setMaxFiles(long nfiles);
