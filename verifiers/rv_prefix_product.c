// Independent reviewer verifier. usage: rv file n cap [skip_index]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXC 4000
int A[MAXC],B[MAXC],m=0,n;
int par[128]; int fnd(int x){return par[x]==x?x:(par[x]=fnd(par[x]));}
int main(int argc,char**argv){
  FILE*f=fopen(argv[1],"r"); n=atoi(argv[2]); int cap=atoi(argv[3]); int skip=argc>4?atoi(argv[4]):-1;
  int c,st=0,num=0,have=0,vals[2],k=0;
  while((c=fgetc(f))!=EOF){ // parse "(a,b)" tokens
    if(c=='('){k=0;have=0;num=0;}
    else if(c>='0'&&c<='9'){num=num*10+(c-'0');have=1;}
    else if(c==','&&have&&k==0){vals[0]=num;k=1;num=0;have=0;}
    else if(c==')'&&have&&k==1){vals[1]=num; A[m]=vals[0];B[m]=vals[1];m++;k=0;num=0;have=0;}
  }
  fclose(f);
  if(skip>=0){for(int i=skip;i<m-1;i++){A[i]=A[i+1];B[i]=B[i+1];} m--;}
  for(int i=0;i<m;i++) if(!(0<=A[i]&&A[i]<B[i]&&B[i]<n)){printf("BAD index %d (%d,%d)\n",i,A[i],B[i]);return 1;}
  int lv[128]={0},depth=0; for(int i=0;i<m;i++){int l=(lv[A[i]]>lv[B[i]]?lv[A[i]]:lv[B[i]])+1; lv[A[i]]=lv[B[i]]=l; if(l>depth)depth=l;}
  printf("size=%d depth=%d\n",m,depth);
  // prefix: comparator i in prefix iff both channels untouched by non-prefix comps and union size<=cap
  static int inpre[MAXC]; int touched[128]={0},gsz[128];
  for(int i=0;i<n;i++){par[i]=i;gsz[i]=1;}
  for(int i=0;i<m;i++){int a=A[i],b=B[i]; inpre[i]=0;
    if(!touched[a]&&!touched[b]){int ra=fnd(a),rb=fnd(b); if(ra==rb){inpre[i]=1;} else if(gsz[ra]+gsz[rb]<=cap){par[ra]=rb;gsz[rb]+=gsz[ra];inpre[i]=1;}}
    if(!inpre[i]){touched[a]=touched[b]=1;}}
  // groups
  int gid[128],ng=0,groot[128],gch[128][64],gn[128];
  for(int i=0;i<n;i++)gid[i]=-1;
  for(int i=0;i<n;i++){int r=fnd(i);int g=-1;for(int j=0;j<ng;j++)if(groot[j]==r)g=j; if(g<0){g=ng++;groot[g]=r;gn[g]=0;} gid[i]=g; gch[g][gn[g]++]=i;}
  // per group: exact output set
  uint64_t *outs[128]; int nout[128]; long double prod=1; int npre=0;
  for(int i=0;i<m;i++)npre+=inpre[i];
  for(int g=0;g<ng;g++){int s=gn[g]; uint64_t N=1ULL<<s; char*seen=calloc(N,1); nout[g]=0; outs[g]=malloc(sizeof(uint64_t)*(N<100000?N:100000));
    int loc[128]; for(int j=0;j<s;j++)loc[gch[g][j]]=j;
    for(uint64_t x=0;x<N;x++){uint64_t v=x; for(int i=0;i<m;i++) if(inpre[i]&&gid[A[i]]==g){int p=loc[A[i]],q=loc[B[i]]; uint64_t bp=(v>>p)&1,bq=(v>>q)&1; if(bp>bq){v^=(1ULL<<p)|(1ULL<<q);} }
      if(!seen[v]){seen[v]=1; if(nout[g]>=100000){printf("too many outs\n");return 1;} outs[g][nout[g]++]=v;}}
    free(seen); prod*=nout[g];}
  printf("prefix CEs=%d groups=%d product=%.0Lf\n",npre,ng,prod);
  for(int g=0;g<ng;g++)printf(" group %d: %d ch, %d outs\n",g,gn[g],nout[g]);
  if(prod>5e9){printf("product too big\n");return 1;}
  // enumerate product, 64 per word
  int idx[128]={0}; long long total=(long long)prod,done=0,bad=0;
  uint64_t W[128];
  while(done<total){ memset(W,0,sizeof W); int cnt=0;
    while(cnt<64&&done<total){
      for(int g=0;g<ng;g++){uint64_t v=outs[g][idx[g]]; for(int j=0;j<gn[g];j++) if((v>>j)&1) W[gch[g][j]]|=1ULL<<cnt;}
      cnt++;done++; for(int g=0;g<ng;g++){if(++idx[g]<nout[g])break; idx[g]=0;} }
    for(int i=0;i<m;i++) if(!inpre[i]){uint64_t x=W[A[i]],y=W[B[i]]; W[A[i]]=x&y; W[B[i]]=x|y;}
    uint64_t mask=(cnt==64)?~0ULL:((1ULL<<cnt)-1), u=0;
    for(int c2=0;c2<n-1;c2++) u|=W[c2]&~W[c2+1]; // 1 above 0 => unsorted
    bad+=__builtin_popcountll(u&mask);
  }
  printf("checked=%lld unsorted=%lld => %s\n",done,bad,bad?"FAIL":"SORTS");
  return bad?2:0;}
