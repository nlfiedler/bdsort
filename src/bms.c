//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

static mndp FNODE,LNODE; static bfile *BFH,**HP; static ULL BF;

#define BULGING(TB,CT,SF) (lv>=BULV)&&(BF*(ULL)(TB+(CT<<2))>NBYTES)

//0.201: added error handling for bmsNuNode and callers
//0.202: added lv>=BULV to burst test "BULGING"
//0.203: fixed bug occurring when there was just one run

int bmsDo(int fsz) {
	int i;
	
	TON; TADD; NLIM=CACHESIZE/(NCHARS*sizeof(mbnp));
	for (i=0; i<NREPS; ++i) if (!bmsAddFile(fsz,i)) {break; TOFF; return(0);}
	TOFF; return(-1);
}

int bmsAddFile(int fsz,int rep) {
	mndp rt; bfile *ibf,*obf; segfile *sf; FILE *f; 
	int i,j,mc,rn,sz; ULL bz,mb,nb,nk,rb; string b1,b,bl,bm; 
		
	TADD; CLEARTALLIES; SMALLOC(FNODE,mbnp*,NLIM*NODESIZE); LNODE=FNODE+NLIM*NCHARS; 
	mb=serval(SER,fsz); BF=mb/HALFCACHE; NSEGS=MAX(NSEGS1,1+(int)(mb/(ULL)MLIM));
	while (Z[MAXLV]*NSEGS>HALFCACHE) --MAXLV; sz=Z[MAXLV]; bz=sz; if (NSEGS>1) {
		SCALLOC(BFH,bfile*,NSEGS,sizeof(bfile)); SCALLOC(HP,bfile**,NSEGS,sizeof(bfile*));
		for (i=0; i<NSEGS; ++i) {HP[i]=tbf(BFH+i,sz,PAD); ++FBINS;} 
	}
	ibf=nubfin(IFP,"rb",sz,PAD,0,fsz); sf=bfsegs(ibf,NSEGS); 
	f=ibf->f; mb=ibf->mb; b=b1=ibf->b1; 
	SMALLOC(CNT,int*,NCHARS*sizeof(int)); SMALLOC(PFX,string,PAD*sizeof(char)); 
	mc=MAXCT; SMALLOC(TP0,string*,mc*sizeof(string)); 
	
	for (rn=0; rn<NSEGS; ++rn) {
		rt=bmsNuNode(); if (!rt) return(0);
		memset(CNT,0,NCHARS*sizeof(int)); nk=NKEYS;
		TREA; fseeko(f,sf->sg[rn],SEEK_SET); rb=sf->ln[rn];
		while (rb>0) {
			TREA; if (feof(f)) rewind(f); rb-=nb=MIN(rb,bz); 
			bl=bm=b1+fread(b1,1,(int)nb,f); 
			TADD; while (bl>b&&*--bl!=DELIM) ; if (bl==b) break; ++bl; 
			bmsAddKeysD(rt,b,bl,CNT); b=b1; while (bl<bm) *--b=*--bm; 
		}
		LOCHAR=LOCHAR0; while (!CNT[LOCHAR]) ++LOCHAR; 
		HICHAR=HICHAR0; while (!CNT[HICHAR]) --HICHAR; TALLY; 
		if (!rep&&rn==NSEGS-1) inlist();
		
		TSRT; if (NSEGS==1) {
			bmsSrt(rt,obf=nubfout(OFP,"wb+",sz,PAD),WRITE,DELIM); 
			OBYTES=obf->fb; bfclear(obf); break;
		} 
		obf=HP[rn]; obf->n=NKEYS-nk; bmsSrt(rt,obf,TFWRITE,0); TFBYTES+=obf->fb; TMER;
		i=rn; while (i>0&&bfcmp(obf,HP[j=(i-1)/2],0)<0) {HP[i]=HP[j]; i=j;} HP[i]=obf; 
	} bfkillsegs(sf); bfclear(ibf); 
	
	if (NSEGS>1) {TMER; bmsMergeRuns(NSEGS,sz); TSRT;} 
	NOTEMEM; FREE(FNODE,NLIM*NODESIZE); NODES=0; FREE(CNT,NCHARS*sizeof(int)); 
	FREE(PFX,PAD*sizeof(char)); FREE(TP0,mc*sizeof(string)); 
	if (NSEGS>1) {
		for (rn=0; rn<NSEGS; ++rn) {bfclose(BFH+rn); --FBINS;} 
		FREE(BFH,NSEGS*sizeof(bfile)); FREE(HP,NSEGS*sizeof(bfile*)); 
	} if (!WRITEFILE) remove(OFP); TNEXTREP; return(-1);
}

void bmsMergeRuns(int nr,int sz) {
	bfile *bf,*obf; int i,j; string t;
	
	obf=nubfout(OFP,"wb+",sz*nr,PAD); bf=HP[0]; 
	while (nr>1) {
		i=0; while ((j=1+(i*2))<nr) {
			if (j+1<nr&&bfcmp(HP[j+1],HP[j],0)<0) ++j;
			if (bfcmp(bf,HP[j],0)<=0) break; HP[i]=HP[j]; i=j;
		} HP[i]=bf; bf=HP[0];
		if (!bfmoves(bf,obf,0,DELIM)) {HP[0]=(bf=HP[--nr]);}
	} 
	do t=bfmoves(bf,obf,0,DELIM); while (t);
	bfflush(obf); OBYTES=ftello(obf->f); bfclear(obf); 
} 

int bmsAddKeysD(mndp rt,string b,string bl,int *cnt) {
	mbnp bn; mndp nd,nu; string bp,lp,s,t; int c,c2,lv,sz,sz2,tb,ct,k;

	TADD; for ( ; b<bl; ++NKEYS,NBYTES+=(ULL)(b-t)) {
		nd=rt; t=b; while ((nu=(mndp)(bn=nd[c=*b++]))>FNODE&&nu<LNODE) nd=nu;
		if (c==DELIM) {if (++NULLCT(nd)==1) ++NBINS; continue;}
		if (bn==NULL) {
			TGRO; SMALLOC(s,string,PAD2); lp=s+PAD; ++VBINS; TADD; 
			cnt[c]=1; while ((c2=*b++)!=DELIM) {*s++=c2; cnt[c2]=1;} *s++=0; 
			nd[c]=bn=(mbnp)s; bn->lp=lp; bn->ct=1; continue;
		} lp=bn->lp; k=bn->ct+1; s=(string)bn;
		while ((c2=*b++)!=DELIM) {*s++=c2; cnt[c2]=1;} *s++=0; 
		if (s>lp) {
			TGRO; ct=k&0xFFFFFF; lv=k>>24; sz=Z[lv]; bp=lp-sz; tb=s-bp; 
			if (BULGING(tb,ct,BF)&&NODES<NLIM) {
				if (!bmsBurst(nd,c,bp,s,sz)) return(0); TADD; continue;
			} 
			sz2=Z[++lv]; SMALLOC(s,string,sz2+PAD); lp=s+sz2; memcpy(s,bp,tb); 
			s+=tb; FREE(bp,sz+PAD); k=ct+(lv<<24); TADD;
		} nd[c]=bn=(mbnp)s; bn->lp=lp; bn->ct=k;
	} return(-1);
}

int bmsBurst(mndp nd,int c,string bp,string s,int sz) {
	mbnp bn; mndp nu; string b,b0,bl,lp; int lv,tb,sz0,sz2,bns,ct,k;

	l1: nu=bmsNuNode(); if (!nu) return(0);
	nd[c]=(mbnp)nu; nd=nu; b=b0=bp; bl=s; sz0=sz; bns=0;
	while (b<bl) {
		c=*b++; if (c==0) {if (++NULLCT(nd)==1) ++NBINS; continue;}
		bn=nd[c]; if (bn==NULL) {
			SMALLOC(s,string,PAD2); lp=s+PAD; ++VBINS; while ((*s++=*b++)!=0) ; k=1; ++bns;
		} else {
			lp=bn->lp; k=bn->ct+1; s=(string)bn; while ((*s++=*b++)!=0) ; 
			if (s>lp) {
				ct=k&0xFFFFFF; lv=k>>24; sz=Z[lv]; bp=lp-sz;
				if (b==bl&&bns==1&&NODES<NLIM) {FREEVBIN(b0,sz0+PAD); goto l1;}
				tb=s-bp; sz2=Z[++lv]; SMALLOC(s,string,sz2+PAD); lp=s+sz2; 
				memcpy(s,bp,tb); s+=tb; FREE(bp,sz+PAD); k=ct+(lv<<24);
			}
		} nd[c]=bn=(mbnp)s; bn->lp=lp; bn->ct=k;
	} FREEVBIN(b0,sz0+PAD); return(-1);
}

void bmsSrt(mndp nd,bfile *bf,int phase,int delim) {
	int bz,c,d,sz,ct,k; string b,b1,b2,bp,s,*tp; FILE *f; mbnp bn; mndp nu;
	
	TSRT; bz=bf->bz; f=bf->f; b=b1=bf->b1; b2=bf->b2; 
	memset(CNT,0,NCHARS*sizeof(int)); c=d=0;
	while (1) {
		if (c==0) {
			if ((ct=NULLCT(nd))>0) {
				TIME(phase); --NBINS; while (ct-->0) {
					memcpy(b,PFX,d); b+=d; *b++=delim;
					if (b>=b2) {fwrite(b1,1,bz,f); memcpy(b1,b2,b-b2); b-=bz;}
				} TSRT;
			} c=LOCHAR;
		}
		for ( ; c<=HICHAR; ++c) {
			TSRT; bn=nd[c]; if (!bn) continue;  
			if ((nu=(mndp)bn)>FNODE&&nu<LNODE) goto l1;
			ct=bn->ct; sz=Z[ct>>24]; ct&=0xFFFFFF; s=bp=bn->lp-sz; 
			tp=TP0; k=ct; while (k-->0) {*tp++=s; while (*s++) ;} 
			if (ct>1) mkqSort(TP0,ct,0);
			TIME(phase); tp=TP0; while (ct-->0) {
				memcpy(b,PFX,d); b+=d; *b++=c; 
				s=*tp++; while ((*b=*s++)!=0) ++b; *b++=delim; 
				if (b>=b2) {fwrite(b1,1,bz,f); memcpy(b1,b2,b-b2); b-=bz;}
			} TSRT; FREEVBIN(bp,sz+PAD);
		} --NODES; if (d==0) break; nd=(mndp)nd[2]; c=PFX[--d]+1; continue; 
		l1: nu[2]=(mbnp)nd; nd=nu; PFX[d++]=c; c=0; 
	} fwrite(b1,1,b-b1,f); fflush(f); bf->fb=ftello(f); rewind(f); 
	b=bf->b2=b1+fread(b1,1,bz,f); while (*--b!=delim) ; bf->bl=++b; bf->b=b1;
}

mndp bmsNuNode(void) {
	mndp nd; int k=NCHARS;
	
	if (NODES==NLIM) {sayln("out of nodes"); return(NULL);}
	nd=(mndp)(FNODE+k*NODES++); while (k>0) nd[--k]=0; return(nd);
}
