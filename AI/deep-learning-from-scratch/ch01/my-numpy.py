import numpy as np

x = np.array([1.0, 2.0, 3.0])
print(x)

print("====================")

A = np.array([[1, 2], [3, 4]])
print(A)
# shape: 矩阵的形状
print(A.shape)
# dtype: 矩阵元素的数据类型
print(A.dtype)

print("====================")

A = np.array([[1, 2], [3, 4]])
B = np.array([10, 20])
print(A * B)

print("====================")

X = np.array([[51, 55], [14, 19], [0, 4]])
print(X)
X = X.flatten() # 将 X 转换为一维数组
print(X)
print(X[np.array([0, 2, 4])])

print("====================")
print(X > 15)
print(X[X>15])