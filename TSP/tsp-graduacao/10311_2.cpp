#include <cstdio>
#include <cmath>
#include <cstdlib>

#define MASK0 0x80
#define MASK1 0x40
#define MASK2 0x20
#define MASK3 0x10
#define MASK4 0x08
#define MASK5 0x04
#define MASK6 0x02
#define MASK7 0x01

#define MAX 100000001
#define MAXT 12500001
#define NPRIM 6000000
#define INF 100000000

char nisp[MAXT]; 
int prim[NPRIM],nprim;

void crivo(){
	int lim = (int)sqrt(MAX);
	nprim = 1;
	prim[0] = 1;
	for(int i=2;i<=lim;i++){
		int isp,ind=i/8;
		switch(i%8){
			case 0: isp = !(nisp[ind] & MASK0); break;	
			case 1: isp = !(nisp[ind] & MASK1); break;
			case 2: isp = !(nisp[ind] & MASK2); break;
			case 3: isp = !(nisp[ind] & MASK3); break;
			case 4: isp = !(nisp[ind] & MASK4); break;
			case 5: isp = !(nisp[ind] & MASK5); break;
			case 6: isp = !(nisp[ind] & MASK6); break;	
			case 7: isp = !(nisp[ind] & MASK7); break;
		} 
		if(isp){
			prim[nprim++] = i;
			for(int j=i+i;j<MAX;j+=i){
				ind = j/8;
				switch(j%8){
					case 0: nisp[ind] |= MASK0; break;	
					case 1: nisp[ind] |= MASK1; break;
					case 2: nisp[ind] |= MASK2; break;
					case 3: nisp[ind] |= MASK3; break;
					case 4: nisp[ind] |= MASK4; break;
					case 5: nisp[ind] |= MASK5; break;
					case 6: nisp[ind] |= MASK6; break;	
					case 7: nisp[ind] |= MASK7; break;
				}
			}
		}
	}
	for(int i=lim+1;i<MAX;i++){
		int isp,ind=i/8;
		switch(i%8){
			case 0: isp = !(nisp[ind] & MASK0); break;	
			case 1: isp = !(nisp[ind] & MASK1); break;
			case 2: isp = !(nisp[ind] & MASK2); break;
			case 3: isp = !(nisp[ind] & MASK3); break;
			case 4: isp = !(nisp[ind] & MASK4); break;
			case 5: isp = !(nisp[ind] & MASK5); break;
			case 6: isp = !(nisp[ind] & MASK6); break;	
			case 7: isp = !(nisp[ind] & MASK7); break;
		} 
		if(isp)
			prim[nprim++] = i;
	}
}

int main(){

	crivo();

	int num;

	while(scanf("%d",&num)!=EOF){
		int resp=-1,mindiff=INF;
		for(int i=0;num>prim[i];i++){
			int diff = num-prim[i],ind,isp,diff2;
			diff2 = abs(diff-prim[i]);
			if(diff2 < mindiff && diff2){
				ind = diff/8;
				switch(diff%8){
					case 0: isp = !(nisp[ind] & MASK0); break;	
					case 1: isp = !(nisp[ind] & MASK1); break;
					case 2: isp = !(nisp[ind] & MASK2); break;
					case 3: isp = !(nisp[ind] & MASK3); break;
					case 4: isp = !(nisp[ind] & MASK4); break;
					case 5: isp = !(nisp[ind] & MASK5); break;
					case 6: isp = !(nisp[ind] & MASK6); break;	
					case 7: isp = !(nisp[ind] & MASK7); break;
				}
				if(isp)
					resp = i;
				mindiff = diff2;
			}
		}
		if(resp!=-1){
			int diff = num-prim[resp];
			int p1 = (prim[resp]<diff)? prim[resp]: diff;	
			int p2 = (prim[resp]>diff)? prim[resp]: diff;
			printf("%d is the sum of %d and %d.\n",num,p1,p2);
		}
		else
			printf("%d is not the sum of two primes!\n",num);
	}

	return 0;
}


