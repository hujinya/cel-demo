# CEL-DEMO（CEL微服务示例）

## 1、相关文件夹说明

cel - 核心库的头文件及lib
cel-demo - http微服务demo源码
mysql - 依赖第三方mysql客户端头文件及lib（已经编译好，若要升级版本，可自行网上下载源码编辑即可）
openssl - 第三方opensll客户端头文件及lib（已经编译好，若要升级版本，可自行网上下载源码编辑即可）
注：其中mysql、openssl为已经编译好的库，若要升级，可自行网上下载源码编辑即可

## 2、测试

### (1)Windows

直接通过vs 2008以上版本打开.\cel-demo\src\cel-demo.sln项目
然后执行编译运行即可，编译后可执行文件目录在.\cel-demo\src\bin\cel-demo.exe下

### (2)Linux

进入源码目录.\cel-demo\src执行make
生成后可执行文件在在.\cel-demo\src\bin\cel-demo

## 3、测试url

https://ip:9045/demo/conf/get  - 下载配置文件
https://ip:9045/demo/allocator_dump - 打印demo内存池分配信息