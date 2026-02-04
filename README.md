<img width="816" height="608" alt="image" src="https://github.com/user-attachments/assets/822742dc-5cb9-4e07-893a-c7a2ad570889" />

# 简介
MCOM串口工具是一款现代化、非常易用的串口调试工具。使用QT开发，**轻量、便携、高效**。

# 功能介绍
目前该软件为V1.0版本，只支持串口模式，后续将加入Modbus模式、烧录模式。

 - 支持基本的串口选择，以及参数设置
 - 支持时间戳显示，支持收发提示
 - 支持HEX模式显示、发送
 - 支持定时发送
 - 支持超时分包自定义
 - 支持丰富的校验码算法，灵活的计算、插入方式
 - 支持收发计数，全局提示
 - 支持拖拽，磁铁吸附
 - 支持固定窗口到顶部
 - 支持Windows11的mica透明效果
 - 支持亮色、暗色两种模式
 - 支持7中主题色设置

# 下载安装

[MCOM Release](https://github.com/Fc-404/mcom/releases/tag/V1.0) 页面提供了多个版本：
> 强烈建议使用mcom_noupx.7z，在体验上非常好，如果没有7z解压工具，使用mcom_noupx.exe自解压程序。
- mcom_noupx.7z 便携版，解压即用，未使用upx压缩，启动最快，占用磁盘空间略大（≈35M），运行时占用内存较小（≈10M）
- mcom_noupx.exe 自解压程序，无需额外7z解压工具，未使用upx压缩，启动最快，占用磁盘空间略大（≈35M），运行时占用内存较小（≈10M）
- mcom_portable_win_v1.0.zip 便携版，解压即用，使用upx压缩过，启动略慢，占用磁盘空间较小（≈15M），运行时占用内存略大（≈45M）
- mcom_single_win_v1.0.exe 单程序版，双击即用，使用upx压缩过，启动略慢，占用磁盘空间较小（≈15M），运行时占用内存略大（≈45M）


# 配置设置
````ini
[Theme]
onblur=true 					#mica透明
ondark=false 					#暗色模式 
themeColor=purple 				#主题色

[TextFlow]
_background="rgba(0, 0, 0, 20)" #数据流背景
_color=#ffffff 					#数据流颜色
_font-size=14px 				#数据流尺寸
border=none 					#数据流边框
maxcount=10000 					#数据流最大行数
ontimestamp=true 				#是否显示时间戳
onutf-8=true 					#是否以UTF-8解码
onhex=false 					#是否以HEX显示
redraw=false 					#更新配置后是否重绘数据流

[ComCtl]
txbuf=@ByteArray(Hello World!) 	#发送缓冲
timeout=10 						#超时时间
autosend=1000 					#自动发送间隔
onhexsend=true 					#是否十六进制发送
onverify=false 					#是否加入校验
onvbig=false 					#校验是否大端对齐
vcrci=-1 						#校验插入位置，-1为倒数第一位
vcrcs=0 						#校验计算起始位置
vcrce=-1 						#校验计算结束位置
vcrct=CRC16_MODBUS 				#校验算法
````
该配置一般在程序目录里，名为`mcom.ini`，如果程序目录没有（单程序版本），则会在`用户目录/.xazh`创建`mcom.ini`
