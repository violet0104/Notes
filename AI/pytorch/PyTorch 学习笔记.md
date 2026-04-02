# PyTorch 学习笔记



## Tensors（张量）

Tensors（张量）是一种与数组和矩阵非常相似的专用数据结构。在 PyTorch 中，我们使用tensors编码模型的输入和输出，以及模型参数。

tensors 类似于 NumPy 的 ndarray，不同之处在于 tensors 可以在 GPU 或其他硬件加速器上运行。事实上，tensors和 NumPy 数组通常可以共享相同的底层内存，从而无需复制数据。

```python
import torch
import numpy as np
```



### 初始化tensor

**1、直接从数据生成**

```python
data = [[1, 2],[3, 4]]
x_data = torch.tensor(data)
```

**2、使用numpy数组创建**

```python
np_array = np.array(data)
x_np = torch.from_numpy(np_array)
```

**3、使用另一个tensor**

```python
x_ones = torch.ones_like(x_data) # 保留参数tensor的属性
print(f"Ones Tensor: \n {x_ones} \n")

x_rand = torch.rand_like(x_data, dtype=torch.float) # 覆盖参数tensor的数据类型
print(f"Random Tensor: \n {x_rand} \n")
```

输出：

```
Ones Tensor:
 tensor([[1, 1],
        [1, 1]])

Random Tensor:
 tensor([[0.8042, 0.9421],
        [0.7325, 0.3900]])
```

**4、使用随机值或恒定值**

```python
shape = (2,3,)	# 两行三列
rand_tensor = torch.rand(shape)
ones_tensor = torch.ones(shape)
zeros_tensor = torch.zeros(shape)

print(f"Random Tensor: \n {rand_tensor} \n")
print(f"Ones Tensor: \n {ones_tensor} \n")
print(f"Zeros Tensor: \n {zeros_tensor}")
```

输出：

```
Random Tensor:
 tensor([[0.2620, 0.7699, 0.9328],
        [0.6073, 0.3423, 0.4578]])

Ones Tensor:
 tensor([[1., 1., 1.],
        [1., 1., 1.]])

Zeros Tensor:
 tensor([[0., 0., 0.],
        [0., 0., 0.]])
```



### tensor的属性

- shape 形状
- dtype 数据类型
- device 存储设备

```python
tensor = torch.rand(3,4)

print(f"Shape of tensor: {tensor.shape}")
print(f"Datatype of tensor: {tensor.dtype}")
print(f"Device tensor is stored on: {tensor.device}")
```

输出：

```
Shape of tensor: torch.Size([3, 4])
Datatype of tensor: torch.float32
Device tensor is stored on: cpu
```

检查电脑有没有可用的加速硬件，如果有，就调用`.to`方法把tensor移动到加速器上。

```python
# We move our tensor to the current accelerator if available
if torch.accelerator.is_available():
    tensor = tensor.to(torch.accelerator.current_accelerator())
```



### tensor的运算

将所有 tensor 的值聚合为一个值，你可以用 `item（）` 将其转换为 Python 的数值：

```python
agg = tensor.sum()
print(agg)
agg_item = agg.item()
print(agg_item, type(agg_item))
```

输出：

```
tensor(12.)
12.0 <class 'float'>
```



**原地操作：**将结果存储到操作数中的操作称为原地操作。它们用 `_` 后缀表示。例如：`x.copy_（y）`，`x.t_（）`，将改变 `x`。

```python
print(f"{tensor} \n")
tensor.add_(5)
print(tensor)
```

输出：

```
tensor([[1., 0., 1., 1.],
        [1., 0., 1., 1.],
        [1., 0., 1., 1.],
        [1., 0., 1., 1.]])

tensor([[6., 5., 6., 6.],
        [6., 5., 6., 6.],
        [6., 5., 6., 6.],
        [6., 5., 6., 6.]])
```



### tensor 到 NumPy 数组

```python
t = torch.ones(5)
print(f"t: {t}")
n = t.numpy()
print(f"n: {n}")
```

输出：

```
t: tensor([1., 1., 1., 1., 1.])
n: [1. 1. 1. 1. 1.]
```

tensor 的变化会反映在 NumPy 数组中（同理：NumPy 阵列的变化会反映在 tensor 上）。

```python
t.add_(1)
print(f"t: {t}")
print(f"n: {n}")
```

输出：

```python
t: tensor([2., 2., 2., 2., 2.])
n: [2. 2. 2. 2. 2.]
```



# Datasets & DataLoaders

处理数据样本的代码可能变得混乱且难以维护；理想情况下，我们希望数据集代码与模型训练代码解耦，以提高可读性和模块化性。PyTorch 提供了两个数据原语：`torch.utils.data.DataLoader` 和 `torch.utils.data.Dataset` 这样你既可以使用预加载的数据集，也能使用你自己的数据。 `数据集`存储样本及其对应标签，`DataLoader` 则将迭代数据包裹在`数据集`上，方便访问样本。

PyTorch 域库提供了若干预加载的数据集（如 FashionMNIST），这些数据集都是 `torch.utils.data.Dataset`的子类，并实现了针对特定数据的具体函数。它们可以用来原型设计和基准测试你的模型。你可以在这里找到它们：[ 图像数据集 ](https://pytorch.org/vision/stable/datasets.html)， [文本数据集 ](https://pytorch.org/text/stable/datasets.html)，以及 [音频数据集](https://pytorch.org/audio/stable/datasets.html)



## Loading a Dataset （加载数据集）

这里有一个示例，展示了如何加载 TorchVision 中的 [Fashion-MNIST](https://research.zalando.com/project/fashion_mnist/fashion_mnist/) 数据集。Fashion-MNIST 是一个包含 Zalando 文章图片的数据集，包含 60,000 个训练示例和 10,000 个测试示例。每个示例包含一张 28×28 灰度图像及 10 个类别之一的相关标签。

我们加载 [FashionMNIST 数据集 ](https://pytorch.org/vision/stable/datasets.html#fashion-mnist)，包含以下参数：

- `root` 存储训练/测试数据的路径
- `train` 指定训练或测试数据集，true表示训练；false表示测试
- `download=True` 如果 root 路径中没有数据，则从互联网下载数据
- `transform` and `target_transform`  指定特征和标签变换

`ToTensor()`用于把图片转换成 tensor，原理是：比如FashionMNIST 是 28×28 灰度图，共784个像素点，实际存储形式是二维数字矩阵 (28, 28)。`ToTensor()`先把原始PIL图片转换成numpy数组；再对所有数字除以255，得到的矩阵数值均在0.0 ~ 1.0；最后重排维度，原始矩阵维度：`H W C` → 高度、宽度、通道数，灰度图：`(28, 28, 1)`，底层操作：交换维度 → `C H W` → 通道数、高度、宽度结果：灰度图最终张量形状：`(1, 28, 28)`。（注：通道数C = 1表示黑白，C = 3表示彩色）。

```python
import torch
from torch.utils.data import Dataset
from torchvision import datasets
from torchvision.transforms import ToTensor
import matplotlib.pyplot as plt

# 训练集
training_data = datasets.FashionMNIST(
    root="data",
    train=True,
    download=True,
    transform=ToTensor()
)

# 测试集
test_data = datasets.FashionMNIST(
    root="data",
    train=False,
    download=True,
    transform=ToTensor()
)
```



## 数据集的迭代与可视化

我们可以手动索引数据集，就像列表一样：`training_data[index]。` 我们使用 `matplotlib` 来可视化训练数据中的部分样本。

```python
labels_map = {
    0: "T-Shirt",
    1: "Trouser",
    2: "Pullover",
    3: "Dress",
    4: "Coat",
    5: "Sandal",
    6: "Shirt",
    7: "Sneaker",
    8: "Bag",
    9: "Ankle Boot",
}
figure = plt.figure(figsize=(8, 8))
cols, rows = 3, 3
for i in range(1, cols * rows + 1):
    sample_idx = torch.randint(len(training_data), size=(1,)).item()
    img, label = training_data[sample_idx]
    figure.add_subplot(rows, cols, i)
    plt.title(labels_map[label])
    plt.axis("off")
    plt.imshow(img.squeeze(), cmap="gray")
plt.show()
```

`torch.randint(low, high, size)` 生成随机整数张量

- **左闭右开**：生成的整数 ≥ `low`，且 < `high`
- `low`可省略，默认 = 0
- `size`：张量的形状（比如 `shape=(2,3)` ）

输出：

![image-20260402100234877](/home/zdc/Notes/AI/pytorch/PyTorch 学习笔记.assets/image-20260402100234877.png)



## DataLoaders 训练

`Dataset`检索我们数据集的特征，并逐个标注样本。在训练模型时，我们通常希望通过“小批量”传递样本，每个时间点重新洗牌数据以减少模型过拟合，并利用 Python 的`multiprocessing`（多进程）加速数据检索。

`DataLoader` 是一个迭代程序，用一个简单的 API 为我们抽象了这些复杂性。

训练模型时，我们会迭代这个加载器，它会批量吐出数据：每次吐出 2 个张量：

- 图片张量（比如：64 张图）
- 标签张量（同理：64 个对应标签）

```python
from torch.utils.data import DataLoader

train_dataloader = DataLoader(training_data, batch_size=64, shuffle=True)
test_dataloader = DataLoader(test_data, batch_size=64, shuffle=True)
```

- `batch_size`：一次喂给模型的样本数（常用 32/64/128）
- `shuffle=True`：打乱数据，防止过拟合



### 遍历 DataLoader

我们已经将该数据集加载到 `DataLoader` 中，并可以根据需要遍历数据集。以下每次迭代返回一批 `train_features` 和 `train_labels`（分别包含 `batch_size=64` 个特征和标签）。由于我们指定了 `shuffle=True`，遍历所有批次后，数据会被洗牌。

```python
# Display image and label.
train_features, train_labels = next(iter(train_dataloader))
print(f"Feature batch shape: {train_features.size()}")
print(f"Labels batch shape: {train_labels.size()}")
img = train_features[0].squeeze() # 去掉维度为 1 的通道
label = train_labels[0]
plt.imshow(img, cmap="gray")
plt.show()
print(f"Label: {label}")
```



输出：

![image-20260402101614586](/home/zdc/Notes/AI/pytorch/PyTorch 学习笔记.assets/image-20260402101614586.png)



# Transforms（变换）