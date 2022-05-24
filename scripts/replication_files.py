import sys
import random
from tqdm import tqdm
import math

op = sys.argv[1]

min_neurons = 0
max_neurons = 0
min_density = 0
max_density = 0

if op == 'density':
    min_density = float(sys.argv[2])
    max_density = float(sys.argv[3])
    min_neurons = int(sys.argv[4])
    max_neurons = min_neurons
    n = 1
    m = int(sys.argv[5])
if op == 'neurons':
    min_neurons = int(sys.argv[2])
    max_neurons = int(sys.argv[3])
    min_density = float(sys.argv[4])
    max_density = min_density
    n = int(sys.argv[5])
    m = 1

rotations = [[], [0], [1], [2], [0,1], [0,2], [1,2], [0,1,2]]

for i in tqdm(range(1, n+1), ncols=80):
    for j in tqdm(range(1, m+1), ncols=80, leave=False):
        neurons = int(min_neurons*(1-i/n) + max_neurons*(i/n))
        density = min_density*(1-j/m) + max_density*(j/m)

        d = (1./density)**(1./3.)*1000
        n1 = round(neurons**(1./3.))
        n2 = round((neurons/n1)**(1./2.))
        n3 = round((neurons/(n1*n2)))

        out = open('../tests/'+op+'/' + str(n1*n2*n3).zfill(len(str(max_neurons))) + '_' + str(density) + '.rpl', 'w')

        for a in range(n1):
            for b in range(n2):
                for c in range(n3):
                    rot = random.choice(rotations)
                    random.shuffle(rot)
                    for r in rot:
                        out.write('{} {} {} '.format(1, r, random.uniform(-math.pi, math.pi)))
                    out.write('0 0 {} 0 1 {} 0 2 {}\n'.format(d*a, d*b, d*c))
        out.close()

