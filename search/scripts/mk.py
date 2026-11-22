# mk.py n:L:d:Lg:seed[:esc[:maxmut]] ... -> writes r{n}_{L}_{d}_L{Lg}_s{seed}_e{esc}.cfg
import sys,os
A1=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','A-1-playful-feynman')
sys.path.insert(0,A1)
from glue import load,layers,comps
from split import split
from mkcfg import cfg
for spec in sys.argv[1:]:
    v=list(map(int,spec.split(':')))
    n,L,d,Lg,seed=v[:5]; esc=v[5] if len(v)>5 else 500; mm=v[6] if len(v)>6 else 2
    N,nw=load(n,L,d); lay=layers(N,nw)
    groups=comps(N,[p for l in lay[:Lg] for p in l])
    pre,tail=split(N,nw,groups)
    name=f'r{n}_{L}_{d}_L{Lg}_s{seed}_e{esc}_m{mm}'
    open(name+'.cfg','w').write(cfg(N,pre,tail,seed,esc,mm))
    print(name,[len(g) for g in groups],len(pre),len(tail))
