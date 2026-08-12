import numpy as np
import matplotlib.pyplot as plt
from matplotlib.image import imread

# 生成数据
x = np.arange(0, 6, 0.1) # 以 0.1 为单位，生成 0 到 6 的数据
y1 = np.sin(x)
y2 = np.cos(x)

# 绘制图形
plt.plot(x, y1, label = "sin")
plt.plot(x, y2, linestyle = "--", label = "cos") # 用虚线绘制
plt.xlabel("x") # x 轴标签
plt.ylabel("y") # y 轴标签
plt.title('sin & cos') # 标题
plt.legend()
plt.show()