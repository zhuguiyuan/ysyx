# "一生一芯"工程项目

这是"一生一芯"的工程项目. 通过运行
```bash
bash init.sh subproject-name
```
进行初始化, 具体请参考[实验讲义][lecture note].

[lecture note]: https://ysyx.oscc.cc/docs/

要在新的设备上部署，添加下面的环境变量
```bash
export NEMU_HOME=/home/zhugy/Desktop/YSYX/nemu
export AM_HOME=/home/zhugy/Desktop/YSYX/abstract-machine
export PATH="/usr/lib/ccache:$PATH"
```
并（根据 `init.sh`）克隆以下仓库
```bash
git clone --depth 1 -b ics2021 git@github.com:NJU-ProjectN/fceux-am.git fceux-am
git clone --depth 1 -b ics2026 git@github.com:NJU-ProjectN/am-kernels.git am-kernels
```
即可继续开发.
