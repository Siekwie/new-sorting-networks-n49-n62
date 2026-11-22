# mkseed.py m seed [esc] [maxmut] [symmetric]-> cfg from seed_m.json
import sys,json,os
sys.path.insert(0,os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','A-1-playful-feynman'))
from mkcfg import cfg
m,seed=int(sys.argv[1]),int(sys.argv[2]); esc=int(sys.argv[3]) if len(sys.argv)>3 else 500; mm=int(sys.argv[4]) if len(sys.argv)>4 else 2
d=json.load(open(f'seed_{m}.json'))
name=f'x{m}_s{seed}_e{esc}_m{mm}'
open(name+'.cfg','w').write(cfg(m,[tuple(x) for x in d['pre']],[tuple(x) for x in d['tail']],seed,esc,mm))
print(name)
