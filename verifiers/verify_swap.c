/* verify.c n file [cap]: checks pairs, size, ASAP depth; proves sorting exactly.
   n<=36 and no cap given: exhaustive over 2^n. Otherwise: prefix of independent groups (each <=cap channels,
   comparators movable to front), exact output set of each group by exhaustion, tail run on the full product. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef uint64_t u64;
int N, M, A[4096], B[4096];
int par[64]; int fnd(int x){while(par[x]!=x)x=par[x]=par[par[x]];return x;}
int main(int argc,char**argv){
  N=atoi(argv[1]); FILE*f=fopen(argv[2],"r"); if(!f){printf("cannot open\n");return 2;}
  int cap = argc>3? atoi(argv[3]) : 0;
  int c,a,b; M=0;
  while((c=fgetc(f))!=EOF){ if(c=='('){ if(fscanf(f,"%d,%d)",&a,&b)!=2){printf("parse error\n");return 2;}
      if(!(0<=a && a<b && b<N)){printf("BAD PAIR (%d,%d)\n",a,b);return 1;} A[M]=a;B[M]=b;M++; } }
  int t[64]={0},D=0; for(int k=0;k<M;k++){int l=(t[A[k]]>t[B[k]]?t[A[k]]:t[B[k]])+1;t[A[k]]=t[B[k]]=l;if(l>D)D=l;}
  printf("n=%d size=%d depth=%d\n",N,M,D);
  if(cap==0 && N<=36){ /* exhaustive bit-parallel: low 6 bits enumerated within word */
    u64 pat[6]={0xAAAAAAAAAAAAAAAAull,0xCCCCCCCCCCCCCCCCull,0xF0F0F0F0F0F0F0F0ull,0xFF00FF00FF00FF00ull,0xFFFF0000FFFF0000ull,0xFFFFFFFF00000000ull};
    u64 tot = N>6? (1ull<<(N-6)):1; u64 bad=0;
    for(u64 w=0;w<tot;w++){ u64 v[64]; for(int i=0;i<N;i++) v[i]= i<6? pat[i] : (((w>>(i-6))&1)? ~0ull:0);
      for(int k=0;k<M;k++){u64 x=v[A[k]],y=v[B[k]];v[A[k]]=x&y;v[B[k]]=x|y;}
      for(int i=0;i+1<N;i++) bad|= v[i]&~v[i+1]; }
    printf(bad? "FAIL\n":"PASS exhaustive 2^%d\n",N); return bad?1:0; }
  if(cap==0) cap=20;
  for(int i=0;i<N;i++)par[i]=i; int sz[64]; for(int i=0;i<N;i++)sz[i]=1;
  int dirty[64]={0}; char inpre[4096]={0};
  for(int k=0;k<M;k++){ a=A[k];b=B[k];
    if(!dirty[a]&&!dirty[b]){ int ra=fnd(a),rb=fnd(b); if(ra==rb || sz[ra]+sz[rb]<=cap){ if(ra!=rb){par[ra]=rb;sz[rb]+=sz[ra];} inpre[k]=1; continue; } }
    dirty[a]=dirty[b]=1; }
  /* groups */
  int G=0, gof[64], gch[64][64], gn[64]; int rootg[64]; for(int i=0;i<64;i++)rootg[i]=-1;
  for(int i=0;i<N;i++){int r=fnd(i); if(rootg[r]<0){rootg[r]=G;gn[G]=0;G++;} int g=rootg[r]; gof[i]=g; gch[g][gn[g]++]=i;}
  /* output sets per group */
  static u64 *outs[64]; int no[64]; double prod=1;
  for(int g=0;g<G;g++){ int k=gn[g]; int pos[64]; for(int j=0;j<k;j++)pos[gch[g][j]]=j;
    char *seen=calloc((size_t)1<<k,1); no[g]=0; outs[g]=malloc(sizeof(u64)*((size_t)1<<k));
    for(u64 x=0;x<(1ull<<k);x++){ u64 y=x; for(int q=0;q<M;q++) if(inpre[q] && gof[A[q]]==g){int p=pos[A[q]],r=pos[B[q]];
          int bp=(y>>p)&1, br=(y>>r)&1; if(bp>br){y^=(1ull<<p)|(1ull<<r);} }
      if(!seen[y]){seen[y]=1; outs[g][no[g]++]=y;} }
    free(seen); prod*=no[g]; }
  int npre=0; for(int k=0;k<M;k++)npre+=inpre[k];
  printf("prefix %d CEs, %d groups:",npre,G); for(int g=0;g<G;g++)printf(" %d(%d outs)",gn[g],no[g]); printf("  product=%.0f\n",prod);
  if(prod>5e9){printf("product too large\n");return 2;}
  /* enumerate product, 64 at a time */
  int idx[64]={0}; u64 bad=0, cnt=0; int done=0;
  while(!done){ u64 v[64]; memset(v,0,sizeof v); int lanes=0;
    for(;lanes<64 && !done;lanes++){
      for(int g=0;g<G;g++){u64 y=outs[g][idx[g]]; for(int j=0;j<gn[g];j++) if((y>>j)&1) v[gch[g][j]]|=1ull<<lanes;}
      cnt++; int g=0; while(g<G){ if(++idx[g]<no[g])break; idx[g]=0; g++; } if(g==G)done=1; }
    u64 mask= lanes==64? ~0ull : ((1ull<<lanes)-1);
    for(int k=0;k<M;k++) if(!inpre[k]){u64 x=v[A[k]],y=v[B[k]];v[A[k]]=x&y;v[B[k]]=x|y;}
    for(int i=0;i+1<N;i++) bad|= (v[i]&~v[i+1])&mask; }
  printf("checked %llu vectors: %s\n",(unsigned long long)cnt, bad?"FAIL":"PASS");
  return bad?1:0; }
