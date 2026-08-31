import numpy as np
import matplotlib.pyplot as plt

def step_function(x):
    return np.array(x > 0, dtype=np.int64)

x = np.arange(-5.0, 5.0, 0.1)
y = step_function(x)
plt.plot(x, y)
plt.ylim(-0.1, 1.1)
plt.show()


def sigmoid(x):
    return 1 / (1 + np.exp(-x))

x = np.arange(-5.0, 5.0, 0.1)
y = sigmoid(x)
plt.plot(x, y)
plt.ylim(-0.1, 1.1)
plt.show()


def relu(x):
    return np.maximum(0, x)
x = np.arange(-5.0, 5.0, 0.1)
y = relu(x)
plt.plot(x, y)
plt.ylim(-1, 5.1)
plt.show()


def old_softmax(x):
    exp_x = np.exp(x)
    sum_exp_x = np.sum(exp_x)
    y = exp_x / sum_exp_x
    return y

def softmax(x):
    c = np.max(x)
    exp_x = np.exp(x - c)
    sum_exp_x = np.sum(exp_x)
    y = exp_x / sum_exp_x
    return y

x = np.array([0.3, 2.9, 4.0])
print(old_softmax(x)) # [ 0.01821127 0.24519181 0.73659691]

a = np.array([1010, 1000, 990])
y = old_softmax(a)
print(y) # [ nan, nan, nan]

c = np.max(a) # 1010
p = a - c # [0, -10, -20]
q = old_softmax(p)
print(q) # [9.99954600e-01 4.53978686e-05 2.06106005e-09]
print(np.sum(q)) # 1.0