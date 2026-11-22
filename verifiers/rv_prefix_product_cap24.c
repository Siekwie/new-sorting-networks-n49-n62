/* Independent reviewer verifier. Usage: rv file n */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXC 4000
int A[MAXC],B[MAXC],m=0,n;
int par[64],gsz[64];
int fnd(int x){while(par[x]!=x)x=par[x]=par[par[x]];return x;}
int main(int argc,char**argv){
  n=atoi(argv[2]); FILE*f=fopen(argv[1],"r"); int c,state=0,v=0,cur[2],k=0,neg=0;
  int lines=0;
  while((c=fgetc(f))!=EOF){
    if(c=='\n')lines++;
    if(c=='-')neg=1;
    if(c>='0'&&c<='9'){v=v*10+(c-'0');state=1;}
    else if(state){cur[k++]=neg?-v:v;v=0;state=0;neg=0;if(k==2){A[m]=cur[0];B[m]=cur[1];m++;k=0;}}
  }
  if(k){printf("odd number of ints\n");return 1;}
  for(int i=0;i<m;i++) if(!(0<=A[i]&&A[i]<B[i]&&B[i]<n)){printf("BAD comparator %d (%d,%d)\n",i,A[i],B[i]);return 1;}
  /* ASAP depth */
  int lay[64]={0},d=0;
  for(int i=0;i<m;i++){int L=(lay[A[i]]>lay[B[i]]?lay[A[i]]:lay[B[i]])+1;lay[A[i]]=lay[B[i]]=L;if(L>d)d=L;}
  printf("n=%d size=%d depth=%d (file lines=%d)\n",n,m,d,lines);
  /* channel usage */
  /* prefix: greedy */
  int fr[64],inpre[MAXC]={0};
  for(int i=0;i<n;i++){fr[i]=1;par[i]=i;gsz[i]=1;}
  for(int i=0;i<m;i++){int a=A[i],b=B[i];
    if(fr[a]&&fr[b]){int ra=fnd(a),rb=fnd(b);
      if(ra==rb||gsz[ra]+gsz[rb]<=24){ if(ra!=rb){par[ra]=rb;gsz[rb]+=gsz[ra];} inpre[i]=1; continue;}}
    fr[a]=fr[b]=0;}
  /* groups */
  int gid[64],ng=0,root2g[64];for(int i=0;i<64;i++)root2g[i]=-1;
  int gch[64][64],gn[64];
  for(int i=0;i<n;i++){int r=fnd(i);if(root2g[r]<0){root2g[r]=ng;gn[ng]=0;ng++;}gid[i]=root2g[r];gch[gid[i]][gn[gid[i]]++]=i;}
  int npre=0;for(int i=0;i<m;i++)npre+=inpre[i];
  printf("prefix comparators=%d groups=%d:",npre,ng);
  /* per-group output set, as uint64 masks over full channel space */
  uint64_t *gs[64];int gcount[64];double prod=1;
  for(int g=0;g<ng;g++){int s=gn[g];uint32_t N=1u<<s;
    char*seen=calloc(N,1);
    int loc[64];for(int i=0;i<s;i++)loc[gch[g][i]]=i;
    for(uint32_t x=0;x<N;x++){uint32_t y=x;
      for(int i=0;i<m;i++)if(inpre[i]&&gid[A[i]]==g){int a=loc[A[i]],b=loc[B[i]];
        int ba=(y>>a)&1,bb=(y>>b)&1; if(ba>bb){y^=(1u<<a)|(1u<<b);} }
      seen[y]=1;}
    int cnt=0;for(uint32_t y=0;y<N;y++)cnt+=seen[y];
    gs[g]=malloc(sizeof(uint64_t)*cnt);cnt=0;
    for(uint32_t y=0;y<N;y++)if(seen[y]){uint64_t M=0;for(int i=0;i<s;i++)if((y>>i)&1)M|=1ull<<gch[g][i];gs[g][cnt++]=M;}
    gcount[g]=cnt;prod*=cnt;free(seen);
    printf(" [%d ch -> %d outs]",s,cnt);}
  printf("\nproduct=%.0f\n",prod);
  if(prod>5e8){printf("product too large\n");return 1;}
  /* enumerate product */
  int idx[64]={0};long long tested=0,bad=0;
  while(1){uint64_t x=0;for(int g=0;g<ng;g++)x|=gs[g][idx[g]];
    for(int i=0;i<m;i++)if(!inpre[i]){int a=A[i],b=B[i];if(((x>>a)&1)&&!((x>>b)&1))x^=(1ull<<a)|(1ull<<b);}
    /* sorted: ones occupy top channels => x == mask of top popcount */
    int p=__builtin_popcountll(x); uint64_t want= p? (((p==64)?~0ull:((1ull<<p)-1))<<(n-p)) : 0;
    if(x!=want){bad++; if(bad<3)printf("UNSORTED output %llx\n",(unsigned long long)x);}
    tested++;
    int g=0;while(g<ng){idx[g]++;if(idx[g]<gcount[g])break;idx[g]=0;g++;}
    if(g==ng)break;}
  printf("tested=%lld bad=%lld => %s\n",tested,bad,bad?"FAIL":"SORTS");
  return bad?2:0;}
