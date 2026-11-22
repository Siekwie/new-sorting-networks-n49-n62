/* vf.c: verify a table-format network file. usage: vf file.txt [cap]
   checks 0<=i<j<n, size, ASAP depth; exact sort proof: greedy movable prefix of comparators
   whose channels lie in disjoint groups (union-find, group size <= cap), each group's output set
   computed exhaustively, rest run on the full Cartesian product (bit-sliced 64 vectors/word). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXC 4096
int A[MAXC],B[MAXC],nc=0,n=0;
int par[128],gsz[128];
int fnd(int x){while(par[x]!=x)x=par[x]=par[par[x]];return x;}
int main(int argc,char**argv){
  FILE*f=fopen(argv[1],"r"); int cap=argc>2?atoi(argv[2]):16; char line[1<<20]; int layers=0;
  while(fgets(line,sizeof line,f)){char*p=line; int any=0;
    while((p=strchr(p,'('))){int a,b; if(sscanf(p,"(%d,%d)",&a,&b)==2){A[nc]=a;B[nc]=b;nc++;any=1; if(b+1>n)n=b+1; if(a+1>n)n=a+1;} p++;}
    if(any)layers++;}
  for(int i=0;i<nc;i++) if(!(0<=A[i]&&A[i]<B[i])){printf("BAD comparator %d (%d,%d)\n",i,A[i],B[i]);return 1;}
  int last[128]={0},depth=0;
  for(int i=0;i<nc;i++){int l=(last[A[i]]>last[B[i]]?last[A[i]]:last[B[i]])+1;last[A[i]]=last[B[i]]=l;if(l>depth)depth=l;}
  printf("n=%d size=%d depth=%d (lines=%d)\n",n,nc,depth,layers);
  /* prefix */
  int dirty[128]={0}, inpre[MAXC]={0};
  for(int i=0;i<n;i++){par[i]=i;gsz[i]=1;}
  for(int i=0;i<nc;i++){int a=A[i],b=B[i];
    if(!dirty[a]&&!dirty[b]){int ra=fnd(a),rb=fnd(b);
      if(ra==rb||gsz[ra]+gsz[rb]<=cap){ if(ra!=rb){par[ra]=rb;gsz[rb]+=gsz[ra];} inpre[i]=1; continue;}}
    dirty[a]=dirty[b]=1;}
  /* groups */
  int gid[128],ng=0,gl[128][32],gn[128]; int root2g[128]; for(int i=0;i<n;i++)root2g[i]=-1;
  for(int i=0;i<n;i++){int r=fnd(i); if(root2g[r]<0){root2g[r]=ng;gn[ng]=0;ng++;} gid[i]=root2g[r]; gl[gid[i]][gn[gid[i]]++]=i;}
  /* exhaustive outputs per group: store as bitmask over n channels (uint64 pair since n<=128) */
  typedef struct{uint64_t w[2];} V;
  V*outs[128]; int no[128]; double prod=1;
  for(int g=0;g<ng;g++){int k=gn[g]; int pos[128]; for(int j=0;j<k;j++)pos[gl[g][j]]=j;
    char*seen=calloc(1u<<k,1); outs[g]=malloc(sizeof(V)*(k+2>0?(1u<<k):1)); no[g]=0;
    for(uint32_t x=0;x<(1u<<k);x++){uint32_t v=x;
      for(int i=0;i<nc;i++) if(inpre[i]&&gid[A[i]]==g){int pa=pos[A[i]],pb=pos[B[i]];
        uint32_t ba=(v>>pa)&1,bb=(v>>pb)&1; if(ba>bb){v^=(1u<<pa)|(1u<<pb);} }
      if(!seen[v]){seen[v]=1; V o={{0,0}}; for(int j=0;j<k;j++) if((v>>j)&1){int c=gl[g][j]; o.w[c>>6]|=1ull<<(c&63);} outs[g][no[g]++]=o;}}
    free(seen); prod*=no[g];}
  int npre=0; for(int i=0;i<nc;i++)npre+=inpre[i];
  printf("prefix=%d CEs, %d groups:",npre,ng); for(int g=0;g<ng;g++)printf(" %d(%d)",gn[g],no[g]); printf("  product=%.0f\n",prod);
  if(prod>4e9){printf("product too large\n");return 2;}
  long long total=(long long)prod, bad=0; int idx[128]={0};
  uint64_t ch[128];
  for(long long base=0;base<total;base+=64){
    int cnt=(total-base<64)?(int)(total-base):64;
    memset(ch,0,sizeof ch);
    for(int t=0;t<cnt;t++){ for(int g=0;g<ng;g++){V o=outs[g][idx[g]]; for(int j=0;j<gn[g];j++){int c=gl[g][j]; if((o.w[c>>6]>>(c&63))&1) ch[c]|=1ull<<t;}}
      for(int g=0;g<ng;g++){ if(++idx[g]<no[g])break; idx[g]=0; } }
    for(int i=0;i<nc;i++) if(!inpre[i]){uint64_t x=ch[A[i]],y=ch[B[i]]; ch[A[i]]=x&y; ch[B[i]]=x|y;}
    uint64_t m=0; for(int c=0;c+1<n;c++) m|=ch[c]&~ch[c+1];
    if(cnt<64) m&=(1ull<<cnt)-1;
    bad+=__builtin_popcountll(m);
  }
  printf("%s: %lld vectors, %lld unsorted -> %s\n",argv[1],total,bad,bad?"FAIL":"SORTS");
  return bad?1:0;}
