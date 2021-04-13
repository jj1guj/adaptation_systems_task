from matplotlib import pyplot as plt
with open("test.out",mode="r") as f:
    L=f.readlines()

L=[float(i.replace("\n","")) for i in L]
X=[i for i in range(len(L))]
plt.plot(X,L)
plt.show()
