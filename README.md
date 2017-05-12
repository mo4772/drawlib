# drawlib
drawlib是基于opengl的一个跨平台(windows和linux)实时视频流显示库，其很容易移植到opengl es上，从而支持ios和android平台。视频流经过解码为rgb数据后可以直接交由drawlib显示，其提供了以下功能：
<ol>
<li>在windows下，可以为显示窗口指定一个父窗口句柄，从而将显示功能嵌入应用中</li>
<li>可以分屏显示，每个子分屏播放不同的画面</li>
<li>可以在指定分屏上叠加文字，可以指定文字的大小，颜色和位置</li>
<li>可以捕获在分屏上的鼠标事件，通过回调来通知上层应用</li>
</ol>
