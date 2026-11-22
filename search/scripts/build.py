# build.py n L d Lg lo(csv) hi(csv) choice(csv) out.json : reduced/variant network, standard form
import sys,os,json
A1=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','A-1-playful-feynman')
sys.path.insert(0,A1)
from glue import load,layers,comps,D
from split import split
OPT={2:[(1,1)],3:[(3,3)],4:[(5,3)],5:[(9,5)],6:[(12,5)],7:[(16,6)],8:[(19,6)],9:[(25,7)],10:[(29,8),(31,7)],11:[(35,8)],12:[(39,9),(40,8)],13:[(45,10),(46,9)],14:[(51,10),(52,9)],15:[(56,10),(57,9)],16:[(60,10),(61,9)]}
n,L,d,Lg=map(int,sys.argv[1:5]); lo,hi,ch=[list(map(int,x.split(','))) for x in sys.argv[5:8]]
N,nw=load(n,L,d); lay=layers(N,nw)
groups=comps(N,[p for l in lay[:Lg] for p in l]); pre0,tail=split(N,nw,groups)
val=[None]*N; lab=list(range(N)); gen=[]
for k,g in enumerate(groups):
    for c in g[:lo[k]]: val[c]=0
    for c in g[len(g)-hi[k]:]: val[c]=1
for a,b in tail:
    va,vb=val[a],val[b]
    if va is None and vb is None: gen.append((lab[a],lab[b])); continue
    if va==0 or vb==1: continue
    val[a],val[b]=vb,va; lab[a],lab[b]=lab[b],lab[a]
r={}; k=0
for c in range(N):
    if val[c] is None: r[lab[c]]=k; k+=1
pre=[]
for k,g in enumerate(groups):
    gg=g[lo[k]:len(g)-hi[k]]
    if len(gg)>1:
        s,dd=OPT[len(gg)][ch[k]]
        j=json.load(open(os.path.join(D,f'Sort_{len(gg)}_{s}_{dd}.json')))
        pre+=[(gg[a],gg[b]) for a,b in j['nw']]
full=[(r[a],r[b]) for a,b in pre]+[(r[a],r[b]) for a,b in gen]
res=[];perm={}
P=lambda x: perm.get(x,x)
for x,y in full:
    a,b=P(x),P(y)
    if a>b:
        inv={v:kk for kk,v in perm.items()}; ka=inv.get(a,a); kb=inv.get(b,b)
        perm[ka],perm[kb]=b,a; a,b=b,a
    res.append((a,b))
m=len(r); last=[0]*m
for a,b in res:
    l=max(last[a],last[b])+1; last[a]=last[b]=l
json.dump({'n':m,'pre':res[:len(pre)],'tail':res[len(pre):],'src':f'{n}_{L}_{d}'},open(sys.argv[8],'w'))
print(sys.argv[8],'n',m,'size',len(res),'depth',max(last))
