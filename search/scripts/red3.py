# red3.py maxrem nmin nmax: Pareto (size,depth) scan of extreme-element reductions of multi-group record networks
import sys,os,itertools,json,glob,re
A1=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','A-1-playful-feynman')
sys.path.insert(0,A1)
from glue import load,layers,comps,D
from split import split
OPT={1:[(0,0)],2:[(1,1)],3:[(3,3)],4:[(5,3)],5:[(9,5)],6:[(12,5)],7:[(16,6)],8:[(19,6)],9:[(25,7)],10:[(29,8),(31,7)],11:[(35,8)],12:[(39,9),(40,8)],13:[(45,10),(46,9)],14:[(51,10),(52,9)],15:[(56,10),(57,9)],16:[(60,10),(61,9)]}
live=open('ext_now.html',encoding='utf8').read()
REC={}
for n in range(17,65):
    m=re.search(r'<td class="idx">%d</td><td class="idx">(.*?)</td>'%n,live)
    if m: REC[n]=[(int(a),int(b)) for a,b in re.findall(r'\((\d+), (\d+)\)',m.group(1))]
def beats(m,s,d): return all(not(a<=s and b<=d) for a,b in REC.get(m,[]))
maxrem,nmin,nmax=map(int,sys.argv[1:4])
srcs=[]
for f in sorted(glob.glob(os.path.join(D,'Sort_*.json'))):
    n,L,d=map(int,os.path.basename(f)[5:-5].split('_'))
    if n<nmin or n>nmax+maxrem or n<20: continue
    N,nw=load(n,L,d); lay=layers(N,nw)
    for Lg in range(3,9):
        groups=comps(N,[p for l in lay[:Lg] for p in l])
        if len(groups)<2: break
        if max(len(g) for g in groups)>16 or min(len(g) for g in groups)<2: continue
        pre,tail=split(N,nw,groups)
        if len(pre)<sum(OPT[len(g)][0][0] for g in groups): continue
        srcs.append((f'{n}_{L}_{d}_L{Lg}',N,tail,groups)); break
print('sources',len(srcs),file=sys.stderr)
def reduce(n,tail,zeros,ones):
    val=[None]*n
    for c in zeros: val[c]=0
    for c in ones: val[c]=1
    out=[]
    for a,b in tail:
        va,vb=val[a],val[b]
        if va is None and vb is None: out.append((a,b)); continue
        if va==0 or vb==1: continue
        val[a],val[b]=vb,va
    return out
front={}
for name,n,tail,groups in srcs:
    G=len(groups); gid={c:k for k,g in enumerate(groups) for c in g}
    rng=[range(0,min(len(g)-1,maxrem)+1) for g in groups]
    for lo in itertools.product(*rng):
        if sum(lo)>maxrem: continue
        for hi in itertools.product(*rng):
            r=sum(lo)+sum(hi)
            if r>maxrem: continue
            m=n-r
            if m<nmin or m>nmax: continue
            sizes=[len(groups[k])-lo[k]-hi[k] for k in range(G)]
            if min(sizes)<1: continue
            zeros=[c for k,g in enumerate(groups) for c in g[:lo[k]]]
            ones=[c for k,g in enumerate(groups) for c in g[len(g)-hi[k]:]]
            out=reduce(n,tail,zeros,ones)
            for choice in itertools.product(*[range(len(OPT[s])) for s in sizes]):
                ps=sum(OPT[s][c][0] for s,c in zip(sizes,choice))
                last=[OPT[sizes[gid[c]]][choice[gid[c]]][1] for c in range(n)]
                for a,b in out:
                    l=max(last[a],last[b])+1; last[a]=last[b]=l
                tot=ps+len(out); dep=max(last)
                F=front.setdefault(m,{})
                if dep not in F or tot<F[dep][0]: F[dep]=(tot,name,sizes,lo,hi,choice)
for m in sorted(front):
    F=front[m]; pts=sorted(F.items())
    for dep,(tot,name,sizes,lo,hi,choice) in pts:
        if any(F[d2][0]<=tot for d2 in F if d2<dep): continue
        flag='BEATS' if beats(m,tot,dep) else ''
        print(m,'rec',REC.get(m),'cand',(tot,dep),name,sizes,lo,hi,choice,flag,flush=True)
