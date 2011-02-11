//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include "bdsort.h"

string dst[]={"rec","","ran7","gen15","rec","wpnd","art4","art2","art5",""},
	ift[]={"dat","srt","rev"};

int main(int argc,char *argv[]) {
	int i,ntoks,nvar,set,sz,lv;
	alist al[30],*alp=al,*va[3]; string t,tok[80];

	setMaxFiles(FLIM=256);

	setArg(alp++,"lo",&MINMB,0,1,0);
	setArg(alp++,"hi",&MAXMB,0,16384,0);
	setArg(alp++,"sc",&NVALS,0,8,0);
	setArg(alp++,"ls",&LOGSER,0,1,0);
	setArg(alp++,"nr",&NREPS,1,1,100);
	setArg(alp++,"oi",&ORDERIN,0,0,2);
	setArg(alp++,"wf",&WRITEFILE,0,0,1);
	setArg(alp++,"cs",&CACHESIZE,1<<12,1<<20,0);
	setArg(alp++,"ns",&NCHARS,1,HICHAR0+1,0);
	setArg(alp++,"lk",&PAD,32,512,0);
	setArg(alp++,"xs",&IOSIZE,1<<12,1<<19,0);
	setArg(alp++,"bl",&BULV,0,3,0);
	setArg(alp++,"m0",&MLIM,0,320000000,0);
	setArg(alp++,"ns0",&NSEGS0,1,8,0);
	setArg(alp++,"ns1",&NSEGS1,1,1,0);

	for (nvar=0,i=1; i<argc; ++i) {
		say(argv[i]); ntoks=0; tok[ntoks++]=strtok(argv[i],",=");
		while ((t=strtok(NULL,","))!=NULL) tok[ntoks++]=t;
		if (strcmp(tok[0],"id")==0) strcpy(INDIR,tok[1]);
		else if (strcmp(tok[0],"od")==0) strcpy(OUTDIR,tok[1]);
		else {
			if ((va[nvar]=loadArg(tok,ntoks,al,alp))==NULL) ;
			else if (va[nvar]->n>1) ++nvar;
		}
	}

	strcpy(INDIR,"LACIE/input/"); strcpy(OUTDIR,"LACIE/output/");

	for (sz=PAD,lv=0; sz<IOSIZE; sz<<=1) Z[lv++]=sz*sizeof(char);
	MAXLV=lv; Z[MAXLV]=IOSIZE; PAD2=PAD<<1; SINKSIZE=IOSIZE+PAD;
	HALFCACHE=CACHESIZE>>1; MAXCT=HALFCACHE/sizeof(string);
	NODESIZE=NCHARS*sizeof(bnp); GNUMBS=0.0;
	serset(SER=&SER0,MINMB,MAXMB,NVALS,LOGSER);

	titles();

	for (i=0; i<NVALS; ++i) {
		for (set=0; (dst[set])!=""; ++set) {
			//runSort(set,0,i); cr();
			//runSort(set,1,i); cr();
			runSort(set,2,i); cr();
		}
	}
	cr(); isaye(FLIM,"FLIM"); cr();
	return(0);
}

void runSort(int dsn,int snum,int fsz) {
	resetTimers(MAXPHASES,MAXREPS);
	switch(snum) {
		case 0: strcpy(SORTNAM,"gnu"); break;
		case 1: strcpy(SORTNAM,"bmsQ"); break;
		case 2: strcpy(SORTNAM,"bdsQ"); break;
		default: {brp("?? sort"); return;}
	}

	strcpy(DATANAM,dst[dsn]); strcat(DATANAM,"."); strcat(DATANAM,ift[ORDERIN]);

	strcpy(IFP,"/volumes/"); strcat(IFP,INDIR); strcat(IFP,DATANAM);

	strcpy(TFP,"/volumes/"); strcat(TFP,OUTDIR);
	strcat(TFP,dst[dsn]); strcat(TFP,".tmg");

	strcpy(OFP,"/volumes/"); strcat(OFP,OUTDIR);
	strcat(OFP,dst[dsn]); strcat(OFP,SORTNAM); strcat(OFP,".std");

	switch(snum) {
		case 0: gnuDo(fsz); break;
		case 1: bmsDo(fsz); break;
		case 2: bdsDo(fsz); break;
		default: {brp("?? sort"); return;}
	}
	outlist();
}

void titles(void) {
	say("\ndset sort n nsegs . bl1 bl2 iosz tfl . nkeys nbytes tfbytes obytes alloc . ");
	say("nT nN nP nB nS nF . add gro srt mer . rea tfw tfr wri . tot mbs"); cr();
}

void inlist(void) {
	say(DATANAM); say(SORTNAM); isay(NREPS); isay(NSEGS); DOT;
	isay(BULV); isay(MAXLV); isay(IOSIZE); isay(TFLIM); DOT;
}

void trielist(void) {
	isay(MAXNODES); isay(MAXNBINS); isay(MAXVBINS);
	isay(MAXSBINS); isay(MAXSINKS); isay(MAXFBINS); DOT;
}

void outlist(void) {
	int tm,iTime,xTime; double xMBS;

	ullsay(NKEYS); ullsay(NBYTES); ullsay(TFBYTES); ullsay(OBYTES); isay(MAXMEM); DOT;
	trielist(); iTime=xTime=0;
	isay(tm=medMS(ADD)); iTime+=tm;
	isay(tm=medMS(GROW)); iTime+=tm;
	isay(tm=medMS(SORT)); iTime+=tm;
	isay(tm=medMS(MERGE)); iTime+=tm; DOT;
	isay(tm=medMS(READ)); xTime+=tm;
	isay(tm=medMS(TFWRITE)); xTime+=tm;
	isay(tm=medMS(TFREAD)); xTime+=tm;
	isay(tm=medMS(WRITE)); xTime+=tm; DOT;
	isay(tm=medMS(ALL));
	xTime+=iTime; if (GNUMBS>0.0) {dsay(GNUMBS,2); GNUMBS=0.0;}
	else {xMBS=NBYTES; xMBS/=xTime; xMBS/=1000; dsay(xMBS,2);}
	if (MEM) isaye(MEM,"\nMEM leak");
	if (NODES) isaye(NODES,"\nNODE leak");
	if (NBINS) isaye(NBINS,"\nNBIN leak");
	if (VBINS) isaye(VBINS,"\nVBIN leak");
	if (SBINS) isaye(SBINS,"\nSBIN leak");
	if (SINKS) isaye(SINKS,"\nSINK leak");
	if (FBINS) isaye(FBINS,"\nFBIN leak");
	if (OBYTES<NBYTES) ullsaye(NBYTES-OBYTES,"\nNBYTES-OBYTES");
	if (OBYTES>NBYTES) ullsaye(OBYTES-NBYTES,"\nOBYTES-NBYTES");
}

void serset(series *ser,int minMB,int maxMB,int nvals,int logser) {
	ser->min=minMB; ser->max=maxMB; ser->ls=logser; ser->nv=nvals;
}

ULL serval(series *ser,int i) {
	ULL min,max,inc,val; double dval,dinc; int nv;

	min=ser->min; min<<=20; max=ser->max; max<<=20; nv=ser->nv-1;
	if (nv==0) return(min); if (i==nv) return(max);
	if (ser->ls) {
		dval=log2((double)min); dinc=(log2((double)max)-dval)/(double)nv;
		val=exp2(dval+dinc*i);
	} else {inc=(max-min)/(ULL)nv; val=min+inc*(ULL)i;} return(val);
}

void setArg(alist *al,string s,int *p,int lo,int def,int hi) {
	al->s=s; al->p=p; al->n=1; *p=al->v[0]=def; al->lo=lo; al->hi=hi;
}

alist *loadArg(string *tok,int n,alist *al,alist *last) {
	int i,k; int j;

	while (strcmp(al->s,tok[0])) if (++al==last) return(NULL); al->n=n-1;
	for (i=1; i<n; ++i) {
		sscanf(tok[i],"%d",&j); k=(int)j;
		if (k<al->lo) {k=al->lo; sayln("value too low; using minimum.");}
		else if (al->hi>0&&k>al->hi) {k=al->hi; sayln("value too high; using maximum.");}
		al->v[i-1]=k;
	} *(al->p)=al->v[0]; return(al);
}

void resetTimers(int nt,int nreps) {
	int i,j;

	for (i=0; i<nt; ++i) for (j=0; j<nreps; ++j) TMR.tm[i][j]=0;
	TMR.cl=TMR.ph=TMR.rep=0;
}

void sortTimer(int ph) {
	int l,rep; double lt,rt;

	for (rep=1; rep<TMR.rep; ++rep) {
		rt=TMR.tm[ph][rep];
		for (l=rep; l>0; --l) {
			lt=TMR.tm[ph][l-1]; if (rt<lt) TMR.tm[ph][l]=lt; else break;
		} TMR.tm[ph][l]=rt;
	}
}

double medMS(int ph) {
	int m,n; double dv;

	if ((n=TMR.rep)>2) sortTimer(ph); m=n/2; dv=TMR.tm[ph][m];
	if (n%2==0) {dv+=TMR.tm[ph][m+1]; dv/=2;} return(MSEC(dv));
}

int setMaxFiles(long nfiles) {
    struct rlimit lim; int flim,rv;

    if (nfiles<getdtablesize()) return(0);
    getrlimit(RLIMIT_NOFILE,&lim);
    lim.rlim_cur=(rlim_t)nfiles;
    rv=setrlimit(RLIMIT_NOFILE,&lim);
    if ((flim=getdtablesize())!=nfiles) {
      isaye(nfiles,"nfiles requested"); isaye(flim,"nfiles set");
      isaye(rv,"setrlimit rv"); isaye(errno,"errno"); br();
    } return(rv?errno:0);
}

/*
void sayLimits(void) {
   struct rlimit lim;

   getrlimit(RLIMIT_CPU,&lim);
   isaye(lim.rlim_cur,"\ncur CPU"); isaye(lim.rlim_max,"max CPU");

   getrlimit(RLIMIT_CORE,&lim);
   isaye(lim.rlim_cur,"\ncur CORE"); isaye(lim.rlim_max,"max CORE");
   getrlimit(RLIMIT_FSIZE,&lim);
   isaye(lim.rlim_cur,"\ncur FSIZE"); isaye(lim.rlim_max,"max FSIZE");
   getrlimit(RLIMIT_MEMLOCK,&lim);
   isaye(lim.rlim_cur,"\ncur MEMLOCK"); isaye(lim.rlim_max,"max MEMLOCK");
   getrlimit(RLIMIT_RSS,&lim);
   isaye(lim.rlim_cur,"\ncur RSS"); isaye(lim.rlim_max,"max RSS");

   getrlimit(RLIMIT_DATA,&lim);
   isaye(lim.rlim_cur,"\ncur DATA"); isaye(lim.rlim_max,"max DATA");
   getrlimit(RLIMIT_STACK,&lim);
   isaye(lim.rlim_cur,"\ncur STACK"); isaye(lim.rlim_max,"max STACK");

   getrlimit(RLIMIT_NOFILE,&lim);
   isaye(lim.rlim_cur,"\ncur NOFILE"); isaye(lim.rlim_max,"max NOFILE");
   getrlimit(RLIMIT_NPROC,&lim);
   isaye(lim.rlim_cur,"\ncur NPROC"); isaye(lim.rlim_max,"max NPROC"); br();
}
*/
