# Replace group sub-sorters at the front of listed networks with other known sorters (other size/depth), report new pairs.
import re,json,glob,os,itertools
front={int(k):v for k,v in json.load(open('front.json')).items()}
def readnet(f): return [tuple(map(int,p)) for p in re.findall(r'\((\d+),(\d+)\)',open(f).read())]
def depth_of(n,nw,start=None):
    t=list(start) if start else [0]*n
    for a,b in nw: l=max(t[a],t[b])+1; t[a]=t[b]=l
    return max(t)
sorters={}
for f in glob.glob('Sorters/Sort_*.json'):
    j=json.load(open(f)); sorters.setdefault(j['N'],[]).append((len(j['nw']),depth_of(j['N'],j['nw']),[tuple(p) for p in j['nw']]))
for k in sorters: sorters[k].sort()
def comps(n,pairs):
    p=list(range(n))
    def f(x):
        while p[x]!=x: p[x]=p[p[x]]; x=p[x]
        return x
    for a,b in pairs: p[f(a)]=f(b)
    g={}
    for i in range(n): g.setdefault(f(i),[]).append(i)
    return sorted(g.values())
_c={}
def is_sorter(k,nw):
    key=(k,tuple(nw))
    if key in _c: return _c[key]
    _c[key]=is_sorter0(k,nw); return _c[key]
def is_sorter0(k,nw):
    if k>20: return None
    import numpy as np
    x=np.arange(1<<k,dtype=np.int64)
    v=[(x>>i)&1 for i in range(k)]
    for a,b in nw: v[a],v[b]=np.minimum(v[a],v[b]),np.maximum(v[a],v[b])
    return all((v[i]<=v[i+1]).all() for i in range(k-1))
def dominated(n,s,d): return any(S<=s and D<=d for S,D in front[n])
found=[]
for f in sorted(glob.glob('nets/*.txt')):
    n,L,D=map(int,os.path.basename(f)[:-4].split('_'))
    if n<49: continue
    nw=readnet(f)
    # layers
    t=[0]*n; lay={}
    for a,b in nw: l=max(t[a],t[b])+1; t[a]=t[b]=l; lay.setdefault(l,[]).append((a,b))
    seen=set()
    for Lg in range(1,D+1):
        groups=comps(n,[p for l in range(1,Lg+1) for p in lay[l]])
        if len(groups)==1: break
        gid={c:k for k,g in enumerate(groups) for c in g}
        dirty=[False]*n; pre=[];tail=[]
        for a,b in nw:
            if not dirty[a] and not dirty[b] and gid[a]==gid[b]: pre.append((a,b))
            else: dirty[a]=dirty[b]=True; tail.append((a,b))
        key=tuple(map(tuple,groups))
        if key in seen: continue
        seen.add(key)
        gpre=[[(g.index(a),g.index(b)) for a,b in pre if gid[a]==k] for k,g in enumerate(groups)]
        ok=[len(g)==1 or is_sorter(len(g),gp) for g,gp in zip(groups,gpre)]
        if not all(x is True for x in ok): continue
        # options per group
        opts=[]
        for g,gp in zip(groups,gpre):
            k=len(g); o=[(len(gp),depth_of(k,gp),gp)]
            for s,d,snw in sorters.get(k,[]):
                if all((s,d)!=(x[0],x[1]) for x in o): o.append((s,d,snw))
            o=[x for x in o if not any(y[0]<=x[0] and y[1]<=x[1] and (y[0],y[1])!=(x[0],x[1]) for y in o)] or o
            opts.append(o)
        best={}
        for combo in itertools.product(*opts):
            newpre=[(g[a],g[b]) for g,(s,d,snw) in zip(groups,combo) for a,b in snw]
            tot=newpre+tail; s=len(tot); d=depth_of(n,tot)
            if d not in best or s<best[d][0]: best[d]=(s,combo)
        for d,(s,combo) in sorted(best.items()):
            tag='NEW' if not dominated(n,s,d) else ''
            if tag:
                found.append((n,s,d,f,Lg))
                newpre=[(g[a],g[b]) for g,(ss,dd,snw) in zip(groups,combo) for a,b in snw]
                tot=newpre+tail; t2=[0]*n; ly={}
                for a,b in tot: l=max(t2[a],t2[b])+1; t2[a]=t2[b]=l; ly.setdefault(l,[]).append((a,b))
                os.makedirs('new',exist_ok=True)
                open(f'new/n{n}_{s}_{d}.txt','w').write(''.join('['+','.join(f'({a},{b})' for a,b in ly[l])+']\n' for l in sorted(ly)))
                print('  groups',[(len(g),ss,dd) for g,(ss,dd,snw) in zip(groups,combo)])
            print(n,L,D,'Lg',Lg,'groups',[len(g) for g in groups],'->',(s,d),tag,flush=True)
print('FOUND',found)
