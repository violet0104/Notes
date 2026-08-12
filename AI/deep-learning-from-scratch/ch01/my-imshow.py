import matplotlib.pyplot as plt
from matplotlib.image import imread

img = imread('./violet.jpeg') # 读入图像（设定合适的路径！
plt.imshow(img)
plt.show()