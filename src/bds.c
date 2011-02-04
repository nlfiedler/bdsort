//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

//	bdsNuFBin dynamically resets FLIM if needed
//	bdsDo now defines BLIM before NLIM

static ndp FNODE,LNODE,*RN; 
static bnp FBIN,LBIN,*HP,*RB; 
static ULL NB;

int bdsDo(int fsz) {
	int i;

	TON; TADD; 
	
	BLIM=MLIM/(SINKSIZE+sizeof(bin)+sizeof(sink)); 
	NLIM=(CACHESIZE-BLIM*sizeof(bin))/NODESIZE; 
	TFLIM=getdtablesize(); //TFLIM=FLIM; 
	
	i=0; while (i<NREPS && bdsAddFile(fsz,i)) ++i;
	
	TOFF;
	
	if (i==NREPS) return(-1); else return(0);
}

int bdsAddFile(int fsz,int rep) {
	ndp rt; bfile *ibf; segfile *sf; FILE *f; 
	int i,j; ULL bz,mb,nb,sb; string b1,b,bl,bm; 
	
	TADD;
	CLEARTALLIES; 
	
	bz=(ULL)IOSIZE; 
	NSEGS=NSEGS0; 
	
	SMALLOC(FNODE,bnp*,NLIM*NODESIZE); 
	SMALLOC(RN,bnp**,NLIM*sizeof(bnp*)); 
	LNODE=FNODE; for (i=0; i<NLIM; ++i) {RN[i]=LNODE; LNODE+=NCHARS;}
	
	SMALLOC(FBIN,bnp,BLIM*sizeof(bin)); 
	SMALLOC(RB,bnp*,2*BLIM*sizeof(bnp)); 
	HP=RB+BLIM; 
	LBIN=FBIN; for (i=0; i<BLIM; ++i) RB[i]=LBIN++; 
	
	rt=bdsNuNode(); if (!rt) return(0);
	
	ibf=nubfin(IFP,"rb",IOSIZE,PAD,0,fsz); 
	sf=bfsegs(ibf,NSEGS);
	f=ibf->f; 
	mb=ibf->mb; 
	b=b1=ibf->b1; 
	
	SMALLOC(CNT,int*,NCHARS*sizeof(int)); 
	
	for (i=0; i<NSEGS; ++i) {
		TREA; 
		fseeko(f,sf->sg[j=sf->r[i]],SEEK_SET); 
		sb=sf->ln[j];
		while (sb>0) {
			TREA; 
			if (feof(f)) rewind(f); 
			sb-=nb=MIN(sb,bz); 
			bl=bm=b1+fread(b1,1,(int)nb,f); 
			TADD; 
			while (bl>b&&*--bl!=DELIM) ; 
			if (bl==b) break; 
			++bl; 
			if (!bdsAddKeysD(rt,b,bl,CNT)) {return(0);}
			b=b1; 
			while (bl<bm) *--b=*--bm;
		}
	}
	LOCHAR=LOCHAR0; while (!CNT[LOCHAR]) ++LOCHAR; 
	HICHAR=HICHAR0; while (!CNT[HICHAR]) --HICHAR; 
	TALLY; 
	bfkillsegs(sf); 
	bfclear(ibf); 
	if (!rep) inlist(); 
	
	TSRT; 
	bdsSrt(rt); 
	NOTEMEM; 
	
	FREE(FNODE,NLIM*NODESIZE); 
	FREE(RN,NLIM*sizeof(bnp*)); 
	FREE(FBIN,BLIM*sizeof(bin)); 
	FREE(RB,2*BLIM*sizeof(bnp)); 
	FREE(CNT,NCHARS*sizeof(int)); 
	if (!WRITEFILE) remove(OFP); 
	TNEXTREP; 
	return(-1);
}

int bdsAddKeysD(ndp rt,string b,string bl,int *cnt) {
	bnp bn; ndp nd,nu; string bp,lp,s,t; int c,c2; int ct,k,lv,sz,sz2,tb; 

	TADD; if (!bdsGetMem(HP,MBYTE)) {return(0);}
	for ( ; b<bl; ++NKEYS,NBYTES+=(ULL)(b-t)) {
		nd=rt; t=b; while ((nu=(ndp)(bn=nd[c=*b++]))>FNODE&&nu<LNODE) nd=nu;
		if (c==DELIM) {if (++NULLCT(nd)==1) ++NBINS; continue;}
		if (bn==NULL) {
			TGRO; SMALLOC(s,string,PAD2); lp=s+PAD; ++VBINS; TADD; 
			cnt[c]=1; while ((c2=*b++)!=DELIM) {*s++=c2; cnt[c2]=1;} *s++=0; 
			nd[c]=bn=(bnp)s; bn->lp=lp; bn->ct=1; continue;
		}
		if (bn>=FBIN&&bn<LBIN) {
			s=bn->ap; ++bn->ct; while ((c2=*b++)!=DELIM) {*s++=c2; cnt[c2]=1;} *s++=0; 
			if (s>bn->lp) {TGRO; bdsGroBin(bn,s); TADD;} else bn->ap=s; continue;
		}
		lp=bn->lp; k=bn->ct+1; s=(string)bn;
		while ((c2=*b++)!=DELIM) {*s++=c2; cnt[c2]=1;} *s++=0; 
		if (s>lp) {
			TGRO; ct=k&0xFFFFFF; lv=k>>24; sz=Z[lv]; bp=lp-sz; tb=s-bp; 
			if ((lv==MAXLV)&&(UP(tb,TFLIM)>NBYTES)&&
				(NODES<NLIM)&&(VBINS+SBINS+FBINS<TFLIM)) {
					if (!bdsBurst(nd,c,bp,s,sz)) {return(0);}
					TADD; continue;
				}
			if (sz==IOSIZE) {
				bn=bdsNuSBin(HP,bp,lp,s); if (!bn) {return(0);}
				bn->ct=k; nd[c]=bn; TADD; continue;
			}
			sz2=Z[++lv]; SMALLOC(s,string,sz2+PAD); lp=s+sz2; memcpy(s,bp,tb); 
			s+=tb; FREE(bp,sz+PAD); k=ct+(lv<<24); TADD;
		} nd[c]=bn=(bnp)s; bn->lp=lp; bn->ct=k;
	} return(-1);  
}

int bdsAddKeys(ndp rt,string b,string bl) {
	bnp bn; ndp nd,nu; string s,t,bp,lp; int c; int ct,k,lv,sz,sz2,tb; 

	TADD; if (!bdsGetMem(HP,MBYTE)) {return(0);}
	for ( ; b<bl; NB+=(ULL)(b-t)) {
		nd=rt; t=b; while ((nu=(ndp)(bn=nd[c=*b++]))>FNODE&&nu<LNODE) nd=nu;
		if (c==0) {if (++NULLCT(nd)==1) ++NBINS; continue;}
		if (bn==NULL) {
			TGRO; SMALLOC(s,string,PAD2); lp=s+PAD; ++VBINS; TADD; 
			while ((*s++=*b++)!=0) ; nd[c]=bn=(bnp)s; bn->lp=lp; bn->ct=1; continue;
		}
		if (bn>=FBIN&&bn<LBIN) {
			s=bn->ap; ++bn->ct; while ((*s++=*b++)!=0) ;
			if (s>bn->lp) {TGRO; bdsGroBin(bn,s); TADD;} else bn->ap=s; continue;
		}
		lp=bn->lp; k=bn->ct+1; s=(string)bn; while ((*s++=*b++)!=0) ;
		if (s>lp) {
			TGRO; ct=k&0xFFFFFF; lv=k>>24; sz=Z[lv]; bp=lp-sz; tb=s-bp;
			if ((lv==BULV)&&(UP(tb,TFLIM)>NB)&&
				(NODES<NLIM)&&(VBINS+SBINS+FBINS<2*TFLIM)) {
					if (!bdsBurst(nd,c,bp,s,sz)) {return(0);}
					TADD; continue;
				}
			if (sz==IOSIZE) {
				bn=bdsNuSBin(HP,bp,lp,s); if (!bn) {return(0);}
				bn->ct=k; nd[c]=bn; TADD; continue;
			}
			sz2=Z[++lv]; SMALLOC(s,string,sz2+PAD); lp=s+sz2; memcpy(s,bp,tb); 
			s+=tb; FREE(bp,sz+PAD); k=ct+(lv<<24); TADD;
		} nd[c]=bn=(bnp)s; bn->lp=lp; bn->ct=k;
	} return(-1);
}

int bdsBurst(ndp nd,int c,string bp,string s,int sz) {
	bnp bn; ndp nu; string b,b0,bl,lp; int bns,ct,k,lv,sz0,sz2,tb;
	
	l1: nu=bdsNuNode(); if (!nu) return(0);
	nd[c]=(bnp)nu; nd=nu; b=b0=bp; bl=s; sz0=sz; bns=0; 
	while (b<bl) {
		c=*b++; if (c==0) {if (++NULLCT(nd)==1) ++NBINS; continue;}
		bn=nd[c]; if (bn==NULL) {
			SMALLOC(s,string,PAD2); lp=s+PAD; ++VBINS;
			while ((*s++=*b++)!=0) ; k=1; ++bns;
		} else {
			lp=bn->lp; k=bn->ct+1; s=(string)bn; while ((*s++=*b++)!=0) ; 
			if (s>lp) {
				ct=k&0xFFFFFF; lv=k>>24; sz=Z[lv]; bp=lp-sz;
				if (b==bl&&bns==1&&NODES<NLIM) {FREEVBIN(b0,sz0+PAD); goto l1;}
				tb=s-bp; sz2=Z[++lv]; SMALLOC(s,string,sz2+PAD); lp=s+sz2; 
				memcpy(s,bp,tb); s+=tb; FREE(bp,sz+PAD); k=ct+(lv<<24);
			}
		} nd[c]=bn=(bnp)s; bn->lp=lp; bn->ct=k;
	} FREEVBIN(b0,sz0+PAD); return(-1);
}

int bdsSrt(ndp nd) {
	int bz,c,d,k,sz,ct,mc; string b,b1,b2,bp,s,*tp; FILE *f; bnp bn; ndp nu; 
	bfile *bf;
	
	TSRT; SMALLOC(PFX,string,PAD*sizeof(char)); memset(CNT,0,NCHARS*sizeof(int)); 
	mc=MAXCT; SMALLOC(TP0,string*,mc*sizeof(string)); c=d=0;
	bf=nubfout(OFP,"wb+",IOSIZE,PAD); f=bf->f; b=b1=bf->b1; b2=b1+(bz=bf->bz); TALLY;
	while (1) {
		if (c==0) {
			if ((ct=NULLCT(nd))>0) {
				TWRI; --NBINS; while (ct-->0) {
					memcpy(b,PFX,d); b+=d; *b++=DELIM;
					if (b>=b2) {fwrite(b1,1,bz,f); memcpy(b1,b2,b-b2); b-=bz;}
				}
			} c=LOCHAR;
		}
		for ( ; c<=HICHAR; ++c) {
			TSRT; bn=nd[c]; if (!bn) continue; 
			if ((nu=(ndp)bn)>FNODE&&nu<LNODE) goto l1;
			if (bn>=FBIN&&bn<LBIN) {
				nu=bn->sn<0?bdsAddTFile(bn):bdsAddSink(bn); 
				if (!nu) {return(0);} goto l1;
			}
			ct=bn->ct; sz=Z[ct>>24]; ct&=0xFFFFFF; s=bp=bn->lp-sz; 
			if (ct>MAXCT) {
				TADD; nu=bdsNuNode(); if (!nu) {return(0);} 
				if (!bdsAddKeys(nu,bp,(string)bn)) {return(0);} 
				TSRT; FREEVBIN(bp,sz+PAD); goto l1;
			}
			tp=TP0; k=ct; while (k-->0) {*tp++=s; while (*s++) ;} 
			if (ct>1) mkqSort(TP0,ct,0);
			TWRI; tp=TP0; while (ct-->0) {
				memcpy(b,PFX,d); b+=d; *b++=c; 
				s=*tp++; while ((*b=*s++)!=0) ++b; *b++=DELIM; 
				if (b>=b2) {fwrite(b1,1,bz,f); memcpy(b1,b2,b-b2); b-=bz;}
			} TSRT; FREEVBIN(bp,sz+PAD);
		} RN[--NODES]=nd; if (d==0) break; nd=(ndp)nd[2]; c=PFX[--d]+1; continue; 
		l1: nu[2]=(bnp)nd; nd=nu; PFX[d++]=c; c=0;
	} bf->b=b; bfflush(bf); OBYTES=ftello(f); NOTEMEM; bfclear(bf); 
	FREE(PFX,PAD*sizeof(char)); FREE(TP0,mc*sizeof(string)); return(-1);
}

ndp bdsAddSink(bnp bn) {
	ndp rt; string s0,s,b,bl,bm,bp; sink *sp,*nu;
	
	TADD; NB=0; rt=bdsNuNode(); if (!rt) return(NULL);
	SMALLOC(s0,string,PAD*sizeof(char)); s=s0; 
	NOTESINKS; NOTESBINS; bdsHeapCut(HP,bn); nu=bn->sp->p; bn->sp->p=0; 
	while (nu) {
		sp=nu; nu=sp->p; b=sp->s; bl=bm=b+IOSIZE; while (*--bl) ; ++bl; 
		if (s>s0) {while ((*s++=*b++)!=0) ; if (!bdsAddKeys(rt,s0,s)) return(NULL); s=s0;}
		if (b<bl) {if (!bdsAddKeys(rt,b,bl)) return(NULL);} while (bl<bm) *s++=*bl++; 
		FREE(sp->s,SINKSIZE); FREE(sp,sizeof(sink)); --SINKS; 
	} 
	b=bp=bn->lp-IOSIZE; bl=bn->ap; if (s>s0) 
		{while (b<bl&&(*s++=*b++)!=0) ; if (!bdsAddKeys(rt,s0,s)) return(NULL);}
	if (b<bl) {if (!bdsAddKeys(rt,b,bl)) return(NULL);}
	NOTEMEM; FREE(s0,PAD*sizeof(char)); FREE(bp,SINKSIZE); RB[--BINS]=bn; return(rt);
}

ndp bdsAddTFile(bnp bn) {
	ndp rt; string bp,b0,b,bl,bm; FILE *f; int nr;
	
	TTFW; NB=0; rt=bdsNuNode(); if (!rt) return(NULL);
	f=(FILE*)bn->sp; bp=bn->lp-IOSIZE; 
	fwrite(bp,1,bn->ap-bp,f); fflush(f); 
	TTFR; rewind(f); b=b0=bp+PAD; setbuffer(f,b0,IOSIZE); 
	while ((nr=fread(b0,1,IOSIZE,f))>0) {
		bl=bm=b0+nr; while (*--bl) ; ++bl; 
		TADD; if (b<bl) {if (!bdsAddKeys(rt,b,bl)) return(NULL);} 
		TTFR; b=b0; while (bm>bl) *--b=*--bm; 
	} NOTEMEM; NOTEFBINS; --FBINS; fclose(f); MEM-=sizeof(FILE); 
	FREE(bp,SINKSIZE); RB[--BINS]=bn; return(rt);
}

ndp bdsNuNode(void) {
	ndp nd; 
	
	if (NODES==NLIM) {sayln("out of nodes"); return(NULL);} 
	else {nd=RN[NODES++]; memset(nd,0,NODESIZE);} return(nd);
}

bnp bdsNuSBin(bnp *hp,string bp,string lp,string s) {
	bnp bn; int n; sink *sp;
	
	if (BINS==BLIM) {sayln("out of bins"); return(NULL);} 
	else {bn=RB[BINS++]; memset(bn,0,sizeof(bin));} NOTEVBINS; --VBINS; 
	hp[bn->sn=SBINS++]=bn; SMALLOC(sp,sink*,sizeof(sink)); ++SINKS; 
	sp->p=sp; sp->s=bp; bn->sp=sp; bn->ns=1; SMALLOC(bp,string,SINKSIZE); 
	bn->lp=bp+IOSIZE; memcpy(bp,lp,n=s-lp); bn->ap=bp+n; return(bn);
}

bnp bdsNuFBin(bnp *hp,bnp bn) {
	FILE *f; sink *sp,*nu; int ph; string bp;
	
	ph=TMR.ph; TTFW; 
	if ((f=tmpfile())==NULL) {setMaxFiles(FLIM+=8); f=tmpfile(); DOT;}
	MEM+=sizeof(FILE); 
	nu=bn->sp->p; bn->sp->p=0; NOTESBINS; NOTESINKS; 
	while (nu) {
		sp=nu; nu=sp->p; fwrite(sp->s,1,IOSIZE,f); TFBYTES+=IOSIZE; 
		FREE(sp->s,SINKSIZE*sizeof(char)); FREE(sp,sizeof(sink)); --SINKS;
	} bp=bn->lp-IOSIZE; fwrite(bp,1,bn->ap-bp,f); fflush(f); 
	bdsHeapCut(hp,bn); bn->sp=(sink*)f; bn->sn=-1; 
	bn->ap=bp; setbuffer(f,bp,IOSIZE); ++FBINS; TIME(ph); return(bn);
}

void bdsGroBin(bnp bn,string s) {
	int i,j; bnp p; sink *sp; string bp,lp;
	
	if (bn->sn>=0) {
		SMALLOC(sp,sink*,sizeof(sink)); ++SINKS; ++bn->ns; sp->p=bn->sp->p; 
		bn->sp->p=sp; lp=bn->lp; sp->s=bp=lp-IOSIZE; bn->sp=sp; 
		SMALLOC(bp,string,SINKSIZE*sizeof(char)); bn->lp=bp+IOSIZE; 
		while (lp<s) *bp++=*lp++; bn->ap=bp; i=bn->sn; 
		while (i>0) {
			p=HP[j=(i-1)>>1]; if (p->ns>=bn->ns) break; 
			p->sn=i; HP[i]=p; i=j;
		} bn->sn=i; HP[i]=bn; 
	} else {
		TTFW; bp=bn->lp-IOSIZE; fwrite(bp,1,IOSIZE,(FILE*)bn->sp); TFBYTES+=IOSIZE; 
		TGRO; memcpy(bp,bn->lp,s-bn->lp); bn->ap=s-IOSIZE; ++bn->ns; 
	}
}

int bdsGetMem(bnp *hp,int sz) {
	while (MEM+sz>MLIM) if (bdsNuFBin(hp,hp[0])==0) return(0); return(-1);
}

void bdsHeapCut(bnp *hp,bnp bn) {
	bnp bn2; int l,r;
	
	l=bn->sn; bn2=hp[--SBINS]; while ((r=(l<<1)+1)<SBINS) {
		if (r+1<SBINS) {if (hp[r+1]->ns>hp[r]->ns) ++r;}
		if (bn2->ns>=hp[r]->ns) break; hp[r]->sn=l; hp[l]=hp[r]; l=r;
	} bn2->sn=l; hp[l]=bn2; 
}
