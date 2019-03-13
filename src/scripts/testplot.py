# This import registers the 3D projection, but is otherwise unused.
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import

import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

file = open('../src/scripts/data.txt', 'r')
X = []
Y = []
Z = []
for line in file.readlines():
    data = [float(x) for x in line.split(' ') if x]
    X.append(data[0])
    Y.append(data[1])
    Z.append(data[2])
    
# Plot the surface.
ax.scatter(X, Y, Z, c='r', marker='o')

ax.set_xlabel('X Label')
ax.set_ylabel('Y Label')
ax.set_zlabel('Z Label')

plt.show()
