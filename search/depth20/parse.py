import re,sys,json
h=open('table.html',encoding='utf8').read()
print(re.search(r'Page updated[^<]*',h).group(0))
front={}
for m in re.finditer(r'<tr><td class="idx">(\d+)</td><td class="idx">(.*?)</td>',h):
    n=int(m.group(1)); front[n]=[tuple(map(int,x)) for x in re.findall(r'\((\d+), (\d+)\)',m.group(2))]
for m in re.finditer(r'<tr id="N(\d+)L(\d+)D(\d+)"><td>.*?<p class="mono">(.*?)</p>',h,re.S):
    n,L,D=map(int,m.groups()[:3])
    lay=[[tuple(map(int,p)) for p in re.findall(r'\((\d+),(\d+)\)',l)] for l in m.group(4).split('<br>') if '(' in l]
    if n>=33: open(f'nets/{n}_{L}_{D}.txt','w').write('\n'.join('['+','.join(f'({a},{b})' for a,b in l)+']' for l in lay)+'\n')
json.dump(front,open('front.json','w'))
for n in range(33,65): print(n,front[n])
