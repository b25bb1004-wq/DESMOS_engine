import matplotlib.pyplot as plt

x = []
y = []

with open("points.txt", "r") as f:
    for line in f:
        a, b = map(float, line.split())
        x.append(a)
        y.append(b)

plt.figure(figsize=(10,6))
plt.plot(x, y, color='blue', linewidth=2)

plt.axhline(0)
plt.axvline(0)

plt.title("Graph of f(x)")
plt.xlabel("x")
plt.ylabel("y")

plt.grid(True)
plt.show()