# mkbatch.py : write SorterHunter configs (record VV tail behind fixed sub-sorter prefix) into this dir
# uses A-1's glue/split/mkcfg; prints group sizes and live-table min size
import sys,os,re,html
A1=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','A-1-playful-feynman')
sys.path.insert(0,A1)
from glue import load,layers,comps
from split import split
from mkcfg import cfg
live=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'ext_live.html'),encoding='utf8').read()
def listed(n):
    m=re.search(r'<td class="idx">%d</td><td class="idx">(.*?)</td>'%n,live)
    return re.findall(r'\((\d+), (\d+)\)',m.group(1))
for spec in sys.argv[1:]:          # n:L:d:seed
    n,L,d,seed=map(int,spec.split(':'))
    N,nw=load(n,L,d); lay=layers(N,nw)
    groups=comps(N,[p for l in lay[:4] for p in l])
    pre,tail=split(N,nw,groups)
    name=f'r{n}_{L}_g4_s{seed}'
    open(name+'.cfg','w').write(cfg(N,pre,tail,seed))
    print(name,'groups',[len(g) for g in groups],'pre',len(pre),'tail',len(tail),'listed',listed(n))
