//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "bdsort.h"

FILE *getf(string fp,string md,string lc)
	{FILE *f=fopen(fp,md); if (!f) {perror(lc); brp(fp);} return(f);}

bfile *tbf(bfile *bf,int bz,int sl) {
	FILE *f;

	if ((f=tmpfile())==NULL) {perror("tbf"); return(NULL);}
	SCALLOC(bf->b0,string,1,(sl+bz+sl)*sizeof(char)); bf->b=bf->b1=bf->b0+(bf->sl=sl);
	bf->bl=bf->b2=bf->b1+(bf->bz=bz); setvbuf(bf->f=f,bf->b1,_IOFBF,bz); return(bf);
}

bfile *nubf(string fp,string mode,int bz,int sl) {
	FILE *f; bfile *bf;

	f=getf(fp,mode,"nubf"); if (!f) return(NULL); SCALLOC(bf,bfile*,1,sizeof(bfile));
	SCALLOC(bf->b0,string,1,(sl+bz+sl)*sizeof(char)); bf->b=bf->b1=bf->b0+(bf->sl=sl);
	bf->bl=bf->b2=bf->b1+(bf->bz=bz); setvbuf(bf->f=f,bf->b1,_IOFBF,bz); return(bf);
}

bfile *nubfin(string fp,string mode,int bz,int sl,int delim,int fsz) {
	bfile *bf; int c; FILE *f; string b,bl; ULL fb;

	bf=nubf(fp,mode,bz,sl); bf->mb=serval(SER,fsz); f=bf->f; b=bf->b1;
	fseek(f,-sl,SEEK_END); fb=ftello(f); bl=b+fread(b,1,sl,f);
	if (delim) while ((c=*--bl)!=delim) ; else while ((c=*--bl)!=10&&c!=13) ;
	++bl; DELIM=c; bf->fb=fb+(bl-b); return(bf);
}

bfile *nubfout(string fp,string mode,int bz,int sl)
	{remove(fp); return(nubf(fp,mode,bz,sl));}

void bfflush(bfile *bf) {fwrite(bf->b1,1,bf->b-bf->b1,bf->f); fflush(bf->f);}

void bfclose(bfile *bf) {fclose(bf->f); FREE(bf->b0,bf->bz+bf->sl*2);}

void bfclear(bfile *bf) {bfclose(bf); FREE(bf,sizeof(bfile));}

int bfcmp(bfile *bf1,bfile *bf2,int delim) {
	string s,t; int n,sl;

	s=bf1->b;								//first key
	t=bf2->b;								//second key
	sl=bf1->sl;								//maximum key length
	n=0;										//byte depth in keys
	while (s[n]==t[n]						//while keys are equal
		&&s[n]!=delim						//and the current char is not the delimiter
		&&n<sl)								//and the string length is below maximum
			++n;								//go to the next byte
	return(s[n]-t[n]);					//return the difference at the final byte depth
}

string bfmoves(bfile *ibf,bfile *obf,int idl,int odl) {
	string b,b1,bl,b2,s,s2,t;

	b=ibf->b;								//first unused key in ibf
	s=obf->b;								//first empty byte in obf
	while ((*s=*b++)!=idl) ++s;		//copy key
	*s++=odl;								//add appropriate delimiter
	if (s>=obf->b2) {						//main buffer is full
		t=s;									//save position of first empty byte
		s=obf->b1;							//set s to start of main buffer
		s2=obf->b2;							//set s2 to start of overflow buffer
		fwrite(s,1,obf->bz,obf->f);	//append main buffer to file
		while (s2<t) *s++=*s2++;		//move overflow to start of main buffer
	}
	obf->b=s;								//save position of next free byte

	if (--ibf->n==0) return(NULL);	//if all keys have been used, return NULL
	if (b==ibf->bl) {						//if we've used the last full key in the buffer
		b=b1=ibf->b1;						//set b and b1 to start of main buffer
		b2=ibf->b2;							//set b2 to start of overflow
		bl=ibf->bl;							//set bl to byte after last eol in main buffer
		while (bl<b2) *--b=*--b2;		//copy any incomplete key BEFORE main buffer
		bl=ibf->b2=b1+fread(b1,1,ibf->bz,ibf->f);
												//read at most bz bytes into main buffer; reset
												//bl & b2 to base of main buffer + actual bytes read
		while (*--bl!=idl) ;				//scan back to find last eol in main buffer
		ibf->bl=++bl;						//and set bl to the next byte
	}
	return(ibf->b=b);						//save and return position of next key
}

segfile *bfsegs(bfile *bf,int ns) {
	int i,j,k,sl; FILE *f; string b; ULL step,fb,mb,pos,n; segfile *sf;

	SCALLOC(sf,segfile*,1,sizeof(segfile)); SCALLOC(sf->sg,ULL*,ns+1,sizeof(ULL));
	SCALLOC(sf->ln,ULL*,ns,sizeof(ULL)); SCALLOC(sf->r,int*,ns,sizeof(int));
	f=bf->f; b=bf->b1; fb=bf->fb; mb=bf->mb; sl=bf->sl; sf->ns=ns;
	step=(mb/(ULL)ns); pos=0; sf->sg[ns]=mb;
	for (i=0; i<ns; ++i) {sf->r[i]=i; sf->sg[i]=pos; pos+=step;}
	for (i=1; i<=ns; ++i) {
		pos=sf->sg[i]%fb; n=(ULL)sl; if (n>pos) n=pos; pos-=n; fseeko(f,pos,SEEK_SET);
		n=fread(b,1,n,f); while (b[--n]!=DELIM) if (n==0) brp("err"); sf->sg[i]+=n+1;
	}
	for (i=0; i<ns; ++i) {
		sf->ln[i]=sf->sg[i+1]-(pos=sf->sg[i]); sf->sg[i]=pos%fb;
		k=sf->r[i]; sf->r[i]=sf->r[j=random()%ns]; sf->r[j]=k;
	}
	rewind(f); return(sf);
}

void bfkillsegs(segfile *sf) {
	int ns;

	ns=sf->ns; FREE(sf->sg,(ns+1)*sizeof(ULL)); FREE(sf->ln,ns*sizeof(ULL));
	FREE(sf->r,ns*sizeof(int)); FREE(sf,sizeof(segfile));
}
