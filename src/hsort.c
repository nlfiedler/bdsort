//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#define P2C(x,y) (*(*(x)+y))
#define SWAP(x,y,z) {z=x; x=y; y=z;}
#define VSWAP(u,v,w,x,y,z) {u=x; v=y; w=*v; *v++=*u; while (--z) {*u++=*v; *v++=*u;} *u=w;}
#define POP(R,P,N,D) {--R; P=R->p; N=R->n; D=R->d;}
#define PUSH(R,P,N,D) {R->p=P; R->n=N; R->d=D; ++R;}

string *med3(string *a,string *b,string *c,int d) {
	int i,j,k; if ((i=P2C(a,d))==(j=P2C(b,d))) return a; 
	if ((k=P2C(c,d))==i||k==j) return c; 
	return i<j?(j<k?b:(i<k?c:a)):(j>k?b:(i<k?a:c)); 
}

void insSort(string *p,int n,int d0) {
	int d,r,l,rc,lc; string ls,rs;

	for (r=1; r<n; ++r) {
		rs=p[r]; for (l=r; l>0; --l) {
			d=d0; ls=p[l-1]; do {rc=rs[d]; lc=ls[d++];} while ((lc==rc)&&rc!=0);
			if (rc<lc||(rc==0&&rs<ls)) p[l]=ls; else break;
		} p[l]=rs; 
	}
}

void mkqSort(string *p,int n,int d) {
	int k,nlo,neq,nhi,piv; string *ol,*il,*ir,*or,*l,*m,*r,t; run st[700],*sp; 
	
	if (n<MKQCUT) {insSort(p,n,d); return;}
	sp=st; PUSH(sp,p,n,d); while (sp>st) {
		POP(sp,p,n,d); l=p; m=p+(n>>1); r=p+n-1; if (n>MED9CUT) 
			{k=n>>3; l=med3(l,l+k,l+k*2,d); m=med3(m-k,m,m+k,d); r=med3(r-k*2,r-k,r,d);}
		m=med3(l,m,r,d); SWAP(*p,*m,t); piv=P2C(p,d); r=p+n; il=p+1; ir=r-1;
		while (il<=ir && P2C(il,d)==piv) ++il; ol=il; 
		while (il<=ir && P2C(ir,d)==piv) --ir; or=ir;
		for (;;) {
			while (il<=ir && (k=P2C(il,d)-piv)<=0) {if (!k) {SWAP(*ol,*il,t); ++ol;} ++il;}
			while (il<=ir && (k=P2C(ir,d)-piv)>=0) {if (!k) {SWAP(*ir,*or,t); --or;} --ir;}
			if (il>ir) break; SWAP(*il,*ir,t); ++il; --ir;
		} 
		nlo=il-ol; nhi=or-ir; neq=n-(nlo+nhi); 
		k=ol-p; if (k>nlo) k=nlo; if (k>0) VSWAP(l,m,t,p,il-k,k); 
		k=r-or-1; if (k>nhi) k=nhi; if (k>0) VSWAP(l,m,t,il,r-k,k); 
		if (neq>1&&piv!=0) 
			{if (neq<MKQCUT) insSort(p+nlo,neq,d+1); else PUSH(sp,p+nlo,neq,d+1);}
		if (nlo>1) {if (nlo<MKQCUT) insSort(p,nlo,d); else PUSH(sp,p,nlo,d);}
		if (nhi>1) {if (nhi<MKQCUT) insSort(r-nhi,nhi,d); else PUSH(sp,r-nhi,nhi,d);}
	}
}

void gnuDo(int fsz) {
	static char s[200]; static time_t t,t0; FILE *inf,*outf; char b[MBYTE]; ULL mb,k;
	double dt; int n;
	
	inf=getf(IFP,"rb","gnuDo"); outf=getf(TFP,"wb","gnuDo"); 
	
	mb=serval(SER,fsz); 
	k=mb/=(ULL)MBYTE; while (k--) {fread(b,1,MBYTE,inf); fwrite(b,1,MBYTE,outf);}
	fclose(inf); fclose(outf); sprintf(s,"sort %s -t 10 -o %s",TFP,OFP); time(&t0); 
	n=0; do {system(s); /*remove(TFP);*/ time(&t); ++n; dt=difftime(t,t0);}
	while (dt<10.0); GNUMBS=DPR(mb,n,dt); inlist(); 
}
