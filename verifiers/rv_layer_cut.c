/* Independent reviewer verifier (C-14). Usage: rv n file L
   Parses all (i,j) pairs from file in order. Checks 0<=i<j<n, counts size,
   computes ASAP depth. Then: the first L ASAP layers form the prefix P.
   Channels are grouped by union-find over P's comparators. Each group's exact
   output set is computed by running P restricted to the group on all 2^k inputs.
   The full product of the group output sets (untouched channels: {0,1}) is fed
   through the remaining comparators (in original order, minus those in P) with
   64-way bit parallelism; every output must be sorted.
   Validity: a comparator in ASAP layer <= L only depends on comparators in
   earlier layers <= L on its channels, so P is closed under predecessors:
   every non-P comparator that shares a channel with a P comparator comes
   later in the original sequence; hence running P first then the rest is the
   same network (comparators on disjoint channels commute). We check this
   closure explicitly anyway. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXC 4096
int n, m, A[MAXC], B[MAXC], lay[MAXC];
int par[64];
int fnd(int x){ while(par[x]!=x) x=par[x]=par[par[x]]; return x; }
int main(int argc, char**argv){
  n=atoi(argv[1]); int L=atoi(argv[3]);
  FILE*f=fopen(argv[2],"r"); if(!f){puts("nofile");return 2;}
  int c, st=0, num=0, have=0, a=0;
  while((c=fgetc(f))!=EOF){
    if(c>='0'&&c<='9'){ num=num*10+(c-'0'); have=1; }
    else { if(have){ if(st==0){a=num;st=1;} else {A[m]=a;B[m]=num;m++;st=0;} } num=0; have=0; }
  }
  fclose(f);
  for(int k=0;k<m;k++) if(!(0<=A[k]&&A[k]<B[k]&&B[k]<n)){printf("BAD CE %d (%d,%d)\n",k,A[k],B[k]);return 1;}
  int last[64]={0}, depth=0;
  for(int k=0;k<m;k++){ int d=(last[A[k]]>last[B[k]]?last[A[k]]:last[B[k]])+1; lay[k]=d; last[A[k]]=last[B[k]]=d; if(d>depth)depth=d; }
  printf("n=%d size=%d depth=%d\n",n,m,depth);
  /* closure check */
  int seenRest[64]={0};
  for(int k=0;k<m;k++){ if(lay[k]>L){seenRest[A[k]]=seenRest[B[k]]=1;} else if(seenRest[A[k]]||seenRest[B[k]]){puts("closure violated");return 1;} }
  for(int i=0;i<n;i++) par[i]=i;
  int np=0; for(int k=0;k<m;k++) if(lay[k]<=L){ np++; par[fnd(A[k])]=fnd(B[k]); }
  /* groups */
  int gid[64], ng=0, gch[64][64], gk[64]; int rootg[64]; for(int i=0;i<64;i++) rootg[i]=-1;
  for(int i=0;i<n;i++){ int r=fnd(i); if(rootg[r]<0){rootg[r]=ng; gk[ng]=0; ng++;} gid[i]=rootg[r]; gch[gid[i]][gk[gid[i]]++]=i; }
  uint32_t *gset[64]; int gsz[64]; double prod=1;
  for(int g=0;g<ng;g++){
    int k=gk[g]; if(k>24){printf("group too big %d\n",k);return 1;}
    char *seen=calloc((size_t)1<<k,1); int cnt=0;
    int loc[64]; for(int t=0;t<k;t++) loc[gch[g][t]]=t;
    for(uint32_t x=0;x<((uint32_t)1<<k);x++){
      uint32_t v=x;
      for(int q=0;q<m;q++) if(lay[q]<=L && gid[A[q]]==g){
        int p=loc[A[q]], r=loc[B[q]]; int bp=(v>>p)&1, br=(v>>r)&1;
        if(bp&&!br){ v&=~(1u<<p); v|=(1u<<r); }
      }
      if(!seen[v]){seen[v]=1;cnt++;}
    }
    gset[g]=malloc(sizeof(uint32_t)*cnt); gsz[g]=0;
    for(uint32_t v=0;v<((uint32_t)1<<k);v++) if(seen[v]) gset[g][gsz[g]++]=v;
    free(seen); prod*=gsz[g];
    printf(" group %d: %d ch, %d outputs\n",g,k,gsz[g]);
  }
  printf("prefix CEs=%d groups=%d product=%.0f\n",np,ng,prod);
  if(prod>4e10){puts("product too big");return 1;}
  /* enumerate product via mixed-radix counter, batching 64 vectors */
  int idx[64]={0}; uint64_t w[64]; int lane=0; long long tested=0, fails=0; int done=0;
  memset(w,0,sizeof w);
  while(!done){
    for(int g=0;g<ng;g++){ uint32_t v=gset[g][idx[g]]; for(int t=0;t<gk[g];t++) if((v>>t)&1) w[gch[g][t]]|=(1ULL<<lane); }
    lane++;
    int g=0; while(g<ng){ idx[g]++; if(idx[g]<gsz[g]) break; idx[g]=0; g++; } if(g==ng) done=1;
    if(lane==64||done){
      uint64_t mask = lane==64? ~0ULL : ((1ULL<<lane)-1);
      for(int q=0;q<m;q++) if(lay[q]>L){ uint64_t x=w[A[q]], y=w[B[q]]; w[A[q]]=x&y; w[B[q]]=x|y; }
      uint64_t bad=0; for(int i=0;i+1<n;i++) bad|= w[i] & ~w[i+1];
      bad&=mask; if(bad){ fails+=__builtin_popcountll(bad); }
      tested+=lane; lane=0; memset(w,0,sizeof w);
    }
  }
  printf("tested=%lld unsorted=%lld => %s\n",tested,fails,fails?"FAIL":"PASS");
  return fails?1:0;
}
