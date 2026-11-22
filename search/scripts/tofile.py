# tofile.py seed.json out.txt : ASAP layers in table format
import sys,json
d=json.load(open(sys.argv[1])); n=d['n']; nw=d['pre']+d['tail']; last=[0]*n; lay=[]
for a,b in nw:
    l=max(last[a],last[b])+1; last[a]=last[b]=l
    while len(lay)<l: lay.append([])
    lay[l-1].append((a,b))
open(sys.argv[2],'w').write('\n'.join('['+','.join(f'({a},{b})' for a,b in L)+']' for L in lay)+'\n')
print(n,len(nw),len(lay))
