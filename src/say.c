//
// Copyright 2009 David B. Ring. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

void say(string s) {printf("%s ",s); fflush(stdout);}
void sayln(string s) {printf("%s\n",s); fflush(stdout);}

void cr(void) {printf("\n"); fflush(stdout);}
void br(void) {while (!getchar()) ;}
void brp(string s) {say(s); br();}

void isay(int i) {printf("%d ",i); fflush(stdout);}
void isaye(int i,string s) {printf("%s=%d ",s,i); fflush(stdout);}

void ullsay(ULL i) {printf("%llu ",i); fflush(stdout);}
void ullsaye(ULL i,string s) {printf("%s=%llu ",s,i); fflush(stdout);}

void dsay(double dv,int n) {
	switch (n) {
		case 0: printf("%.0f ",dv); break;
		case 1: printf("%.1f ",dv); break; 
		case 2: printf("%.2f ",dv); break; 
		case 3: printf("%.3f ",dv); break; 
		case 4: 
		default: printf("%.4f ",dv); break; 
	} fflush(stdout);
}
void dsaye(double dv,string s) {printf("%s=%.4f ",s,dv); fflush(stdout);}
