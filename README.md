# drawlib
基于opengl的视频播放库，已在windows和centos7.2上测试通过，提供如下功能:
<ol>
<li>1.在windows下，可以为播放窗口指定一个父窗口句柄，从而将显示功能嵌入应用中</li>
<li>2.可以分屏显示，每个子分屏播放不同的画面</li>
<li>3.可以在指定分屏上叠加文字，可以指定文字的大小，颜色和位置</li>
<li>4.可以捕获在分屏上的鼠标事件，通过回调来通知上层应用</li>
<li>5.drawlib支持的显示数据为rgb，传入图像数据前需将数据转化为rgb格式</li>
</ol>
