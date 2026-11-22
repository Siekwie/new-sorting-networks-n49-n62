# extract.py log n out : last network in a SorterHunter log -> ASAP-layered table format
import re,sys
t=open(sys.argv[1]).read(); n=int(sys.argv[2])
s=re.findall(r"'nw':\[(.*?)\]\}",t)[-1]
nw=[tuple(map(int,p)) for p in re.findall(r'\((\d+),(\d+)\)',s)]
last=[0]*n; lay=[]
for a,b in nw:
    l=max(last[a],last[b])+1; last[a]=last[b]=l
    while len(lay)<l: lay.append([])
    lay[l-1].append((a,b))
open(sys.argv[3],'w').write('\n'.join('['+','.join('(%d,%d)'%p for p in l)+']' for l in lay)+'\n')
print(len(nw),len(lay))
