# blackness

这是一个史瓦西度规下的光线追踪程序，主要是为了史瓦西黑洞可视化

![No Accretion Disk](images/blackhole.png)

![Chessboard background](images/motion.gif)

## 下载
[码云]

[github]

## 天体物理模拟

我的数学不好，物理也不好。我就是觉得天体物理很有意思，就写了这个黑洞外部的光线追踪程序。

程序的原理很简单，就是对史瓦西黑洞（不旋转，不带电荷）的Null测地线进行数值积分，就可以得到光线的路径。

笔记和prototype在另一个[仓库]里。

我尝试用GPU加速了，但是更慢了。可能是因为GPU频率太低，双精度浮点的速度也很低。

## Usage

程序有图形界面也有命令行接口（不完整）

![Interface](images/interface.jpg)

图形界面的右侧参数如下，

- 有一个图像模式和一个视频模式
- Blackhole 和 Accretion Disk 这两个复选框是用于选择是否要渲染黑洞和吸积盘。
- Skybox 路径是天空盒纹理的文件夹路径。默认是一个棋盘样子的天空盒。可以用其他的天空盒，只要文件夹里面的文件名称里有 "front", "up", "top", "bottom", "right", "left" 这些字样。
- 吸积盘纹理是单张图片，可以是一维也可以是二维纹理。
- 吸积盘半径(disk radius)就是吸积盘圆环的内径和外径。
- Width 和 Height 是渲染图片的分辨率。暂时还不支持长方形的渲染。
- samples 设置是采样数，分辨率高的话就不用这个来抗锯齿了，渲染速度跟这个设置成正比。设太高了会很慢。
- Threads 是渲染线程数，设置成电脑核心数就好了。如果设置很高，电脑可能会卡住。
- Camera position 和 Camera Lookat 是相机的三维笛卡尔坐标和观察点。黑洞永远放置在原点(0,0,0)
- Bloom效果要在渲染完成后使用，有bug
- 渲染完成后就可以点左上角的保存来保存图片

## 视频合成

程序可以生成动态的视频，通过合成多张图片

Camera position file 用来设置相机的位置信息

position file的格式如下：

- 每一行是视频的一帧
- 每一行有三个坐标：相机坐标，相机前方方向向量，相机上方向量

视频的输出位置暂时是硬编码的，会在渲染完成之后保存MP4在程序目录下。

[github]:https://github.com/evopen/blackness/releases
[码云]:https://gitee.com/evopen/blackness/releases
[仓库]:https://github.com/evopen/gr