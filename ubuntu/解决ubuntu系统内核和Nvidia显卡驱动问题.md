# 解决ubuntu系统内核和Nvidia显卡驱动问题

## 问题

ubuntu使用命令`sudo apt upgrade antigravity`后，由于antigravity是用.dep安装包下

载的，**并不是ubuntu的软件包**。执行的命令实际上等价于 `sudo apt upgrade`（因为 

`antigravity` 不存在于 apt 仓库），导致系统执行了一次全面的系统升级。重启后系统卡

死。



## 可能原因

**NVIDIA 驱动 (535.274.02 → 535.288.01)** 和 **内核 (6.8.0-40 → 6.8.0-106)** 同时更新，导致

驱动模块与新内核不匹配而卡死。



## 解决方案

1. 重启电脑，在 GRUB 界面选择 **"Advanced options for Ubuntu"**

2. 选择 **"Ubuntu, with Linux 6.8.0-40-generic Recovery mode"**（旧内核），进入Recovery mode修复。如果选择**"Ubuntu, with Linux 6.8.0-40-generic Recovery mode"**（新内核）还是无法启动。

3. 正常进入系统后，用`dpkg --list | grep linux-image`查看系统内核版本有哪些。

   ```bash
   $ dpkg --list | grep linux-image
   ii  linux-image-5.15.0-173-generic                         5.15.0-173.183                          amd64        Signed kernel image generic
   ii  linux-image-6.8.0-106-generic                          6.8.0-106.106~22.04.1                   amd64        Signed kernel image generic
   ii  linux-image-6.8.0-40-generic                           6.8.0-40.40~22.04.3                     amd64        Signed kernel image generic
   ```

4. 删除5.15.0-173和6.8.0-106的内核。

   ```bash
   sudo apt remove linux-image-6.8.0-106-generic linux-image-5.15.0-173-generic 
   ```

5. 删除所有残留的内核包。

   ```bash
   # 1. 删除所有残留的内核包（包括 unsigned 版本）
   sudo apt remove --purge \
       linux-image-unsigned-5.15.0-173-generic \
       linux-image-unsigned-6.8.0-106-generic \
       linux-modules-5.15.0-173-generic \
       linux-modules-6.8.0-106-generic \
       linux-modules-extra-6.8.0-106-generic \
       linux-hwe-6.8-tools-6.8.0-106 \
       linux-tools-6.8.0-106-generic \
       linux-headers-6.8.0-106-generic \
       linux-hwe-6.8-headers-6.8.0-106
   
   # 2. 清理自动安装的残留（包括那个 580 驱动固件）
   sudo apt autoremove --purge
   
   # 3. 清理配置残留（消除 rc 状态）
   sudo dpkg --purge $(dpkg -l | grep '^rc' | awk '{print $2}')
   
   # 4. 更新 GRUB
   sudo update-grub
   ```

6. 删除已经安装的nvidia显卡驱动，重装nvidia-535

   ```bash
   # 查看安装了哪些 NVIDIA 包
   dpkg -l | grep nvidia
   
   # 查看当前加载的驱动
   lsmod | grep nvidia
   
   # 卸载所有 nvidia 相关包
   sudo apt purge nvidia*
   
   # 重装nvidia-535
   sudo apt reinstall nvidia-driver-535
   ```

7. 确认驱动模块是否编译到 **6.8.0-40** 内核：

   ```bash
   sudo dkms status | grep nvidia
   nvidia/535.288.01, 6.8.0-40-generic, x86_64: installed
   ```

8. 在软件更新器中选择：NVIDIA driver metapackage 来自 nvidia-driver-535（专有）

   ![img](https://www.kimi.com/apiv2-files/sign-obj/kimi-fs%2Ffiles%2Fblob%2F4275e495aa5c1775349e85891e0c6d84bcbc30cdd05189523dddbd459a16592f?filename=%E6%88%AA%E5%9B%BE+2026-03-14+20-16-53.png&sig=BeZOD957t9ZAGwqJKrJiiOAX0Bt6Nc52MJzR0K_OUSQ=&t=t)

9. 最后执行清理，并重启。

   ```bash
   # 1. 彻底删除所有残留内核包（包括 unsigned 版本）
   sudo apt remove --purge \
       linux-image-unsigned-5.15.0-173-generic \
       linux-image-unsigned-6.8.0-106-generic \
       linux-modules-5.15.0-173-generic \
       linux-modules-6.8.0-106-generic \
       linux-modules-extra-6.8.0-106-generic \
       linux-hwe-6.8-tools-6.8.0-106 \
       linux-tools-6.8.0-106-generic
   
   # 2. 清理所有自动残留的包（包括之前看到的 580 固件等）
   sudo apt autoremove --purge
   
   # 3. 清理 rc 状态的配置残留
   sudo dpkg --purge $(dpkg -l | grep '^rc' | awk '{print $2}') 2>/dev/null || true
   
   # 4. 更新 GRUB 和 initramfs
   sudo update-grub
   sudo update-initramfs -u -k 6.8.0-40-generic
   
   # 5. 重启测试
   sudo reboot
   ```



重启后，能正常进入ubuntu。