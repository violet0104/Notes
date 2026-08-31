import numpy as np

def AND(x1, x2):
    x = np.array([x1, x2])
    w = np.array([0.5, 0.5])
    b = -0.7
    tmp = np.sum(x*w) + b
    if (tmp <= 0):
        return 0
    else:
        return 1

print(AND(0, 0)) # 0
print(AND(0, 1)) # 0
print(AND(1, 0)) # 0
print(AND(1, 1)) # 1

print("=======================")

def NAND(x1, x2):
    x = np.array([x1, x2])
    w = np.array([-0.5, -0.5])
    b = 0.7
    tmp = np.sum(x*w) + b
    if (tmp <= 0):
        return 0
    else:
        return 1

print(NAND(0, 0)) # 1
print(NAND(0, 1)) # 1
print(NAND(1, 0)) # 1
print(NAND(1, 1)) # 0

print("=======================")

def OR(x1, x2):
    x = np.array([x1, x2])
    w = np.array([0.5, 0.5])
    b = -0.2
    tmp = np.sum(x*w) + b
    if (tmp <= 0):
        return 0
    else:
        return 1

print(OR(0, 0)) # 0
print(OR(0, 1)) # 1
print(OR(1, 0)) # 1
print(OR(1, 1)) # 1

print("=======================")

print("=======================")

def XOR(x1, x2):
    x = np.array([x1, x2])
    w = np.array([0.5, 0.5])
    b = -0.2
    tmp = np.sum(x*w) + b
    if (tmp <= 0):
        return 0
    else:
        return 1

print(XOR(0, 0)) # 0
print(XOR(0, 1)) # 1
print(XOR(1, 0)) # 1
print(XOR(1, 1)) # 1

print("=======================")

x = np.array([0, 1])        # 输入
w = np.array([0.5, 0.5])    # 权重
b = -0.7                    # 偏置
print(x*w)
print(np.sum(w*x))
print(np.sum(w*x) + b)


xor = a ^ b = 