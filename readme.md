[toc]

# KNX

### 1.KNXnet/IP

#### 1、介绍

​		提供一个通用的独立于主机协议框架。

范围：

1、为KNXnet/IP通讯定义在非KNX注意协议网络上发送的数据包

2、KNXnet/IP服务的发现与自我描述

3、配置、建立和维护在KNXnet/IP客户端与服务器之间的通信通道

#### 2、头部信息

![1711963483732](readme.assets/1711963483732.png)

#### 3、端点

​      1、HPAI:host protocol address information 包括IP地址和端口号。

​      2、实体3个功能：

​            1、每个KNXnet/IP设备仅应支持一个设备相关的，双向的和用户发现的无连接端点，如果主机协议需要发现服务。

​            2、应支持至少一个用于控制的双向的和无连接的端点，

​            3、至少一个双向的和面向连接的端点作为服务用于数据收发。

​     3、控制端点能告知KNXnet/IP中的一个实体，该实体能够提供至少一种KNXnet/IP服务类型。

​      4、每个KNX子网的控制端点不同。KNXnet/IP客户端认为一个独立的实体由控制端点表示，无聊他在1个或2个单独的KNXnet/IP服务中实现。

#### 4、发现与自我描述

​				1、SEARCH_REQUEST数据包通过使用发现端点的**多播发出**，包含**发现断点的HPAI**,**发现断点的HPAI**包含一个单播IP地址，以接收直接来自点对点模式下不同KNXnet/IP的应答。通常HPAI应包含KNXnet/IP系统设置多播地址以确保哦来自不同子网上的KNXnet/ip服务器上的数据包。

​            2、在SEARCH_TIMEOUT时间段内收应答。

​				3、响应包含用于后续通信的控制端点HPAI。包括后续的子网。

​            4、KNXnet/IP服务端支持DESCRIPTION-REQUEST 单播方式 包含协议版本 自身能力 状态信息 knx连接名字（可选）

#### 5、通信



**2． 功能不同**

Typedef用来定义类型的别名，这些类型不只包含内部类型（int，char等），还包括自定义类型（如struct），可以起到使类型易于记忆的功能。

如： typedef int (*PF) (const char *, const char *);

定义一个指向函数的指针的数据类型PF，其中函数返回值为int，参数为const char *。

typedef 有另外一个重要的用途，那就是定义机器无关的类型，例如，你可以定义一个叫 REAL 的浮点类型，在目标机器上它可以i获得最高的精度：typedef long double REAL;
在不支持 long double 的机器上，该 typedef 看起来会是下面这样：typedef double REAL;
并且，在连 double 都不支持的机器上，该 typedef 看起来会是这样：typedef float REAL;

\#define不只是可以为类型取别名，还可以定义常量、变量、编译开关等。

**3． 作用域不同**

\#define没有作用域的限制，只要是之前预定义过的宏，在以后的程序中都可以使用。

而typedef有自己的作用域。

void  fun() 
 { 
   \#define  A  int 
 } 

 void  gun() 
 { 
    //在这里也可以使用A，因为宏替换没有作用域， 
    //但如果上面用的是typedef，那这里就不能用A  ，不过一般不在函数内使用typedef
 }

**4． 对指针的操作**

二者修饰指针类型时，作用不同。

Typedef int * pint；

\#define PINT int *

Const pint p；//p不可更改，p指向的内容可以更改，相当于 int * const p;

Const PINT p；//p可以更改，p指向的内容不能更改，相当于 const int *p；或 int const *p；
pint s1, s2; //s1和s2都是int型指针
PINT s3, s4; //相当于int * s3，s4；只有一个是指针。

### 3.安卓app没有图标

```
<activity android:name=".MainActivity">
    <intent-filter>
        <data android:scheme="wxde916e84088a2b98" />
        <action android:name="android.intent.action.MAIN" />

        <category android:name="android.intent.category.LAUNCHER" />

    </intent-filter>
</activity>
```

###  4.安卓开机启动服务

1.增加MyReceiver监听BroadcastReceiver 跳到 class MyService extends Service

```
public void onReceive(Context context, Intent intent) {
    // TODO: This method is called when the BroadcastReceiver is receiving
    // an Intent broadcast.
    //throw new UnsupportedOperationException("Not yet implemented");
    Log.d(TAG, "onReceive: "+intent.getAction());
    if(Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())){
        Intent thisIntent = new Intent(context, MyService.class);
        thisIntent.setAction("android.intent.action.MAIN");
        thisIntent.addCategory("android.intent.category.LAUNCHER");
        thisIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startService(thisIntent);
    }
}
```

 2.增加配置

```
<receiver
    android:name=".MyReceiver"
    android:enabled="true"
    android:exported="true">
    <intent-filter>
        <!--.接收启动完成的广播-->
        <action android:name="android.intent.action.BOOT_COMPLETED"/>
    </intent-filter>
</receiver>
<service android:name=".MyService"/>
```

### 5.JNI 函数名加1

如果你不想函数名中有1你就不要让函数名中用'_',因为jni吧C函数名映射成java方法名的时候是com_mypackage_myclass_mymethod()：com.mypackage.myclass.mymethod();也就是说jni是依靠"_"来间隔包、类、方法的，但是如果你的方法名中有'_'字符的话，jni必须能够区分方法名中的'"_"字符还是间隔符，所以用加一个1后缀的方式来区分。

### 6.h264协议帧头数据解析

**00 00 00 01 67   (SPS)**：SPS即Sequence Paramater Set，又称作序列参数集。SPS中保存了一组编码视频序列(Coded video sequence)的全局参数。所谓的编码视频序列即原始视频的一帧一帧的像素数据经过编码之后的结构组成的序列。而每一帧的编码后数据所依赖的参数保存于图像参数集中。一般情况SPS和PPS的NAL Unit通常位于整个码流的起始位置。但在某些特殊情况下，在码流中间也可能出现这两种结构，主要原因可能为：

1：解码器需要在码流中间开始解码；

2：编码器在编码的过程中改变了码流的参数（如图像分辨率等）；

**00 00 00 01 68   (PPS)**：除了序列参数集SPS之外，H.264中另一重要的参数集合为图像参数集Picture Paramater Set(PPS)。通常情况下，PPS类似于SPS，在H.264的裸码流中单独保存在一个NAL Unit中，只是PPS NAL Unit的nal_unit_type值为8；而在封装格式中，PPS通常与SPS一起，保存在视频文件的文件头中。

**00 00 00 01 65   ( IDR 帧)** ： I帧表示关键帧，你可以理解为这一帧画面的完整保留；解码时只需要本帧数据就可以完成（因为包含完整画面）

**00 00 00 01 61   (P帧)** ：P帧表示的是这一帧跟之前的一个关键帧（或P帧）的差别，解码时需要用之前缓存的画面叠加上本帧定义的差别，生成最终画面。（也就是差别帧，P帧没有完整画面数据，只有与前一帧的画面差别的数据）

二、start code

start code有两种，四个字节的“00 00 00 01”和三个字节的“00 00 01”都是。

三：帧格式

H264帧由NALU头和NALU主体组成。

NALU头由一个字节组成,它的语法如下:

+---------------+

|0|1|2|3|4|5|6|7|

+-+-+-+-+-+-+-+-+

|F|NRI| Type  |

+---------------+

F: 1个比特.

forbidden_zero_bit. 在 H.264 规范中规定了这一位必须为 0.

NRI: 2个比特.

nal_ref_idc. 取00~11,似乎指示这个NALU的重要性,如00的NALU解码器可以丢弃它而不影响图像的回放,0～3，取值越大，表示当前NAL越重要，需要优先受到保护。如果当前NAL是属于参考帧的片，或是序列参数集，或是图像参数集这些重要的单位时，本句法元素必需大于0。

Type: 5个比特.

nal_unit_type. 这个NALU单元的类型,1～12由H.264使用，24～31由H.264以外的应用使用,简述如下:

0   没有定义

1-23 NAL单元 单个 NAL 单元包

1   不分区，非IDR图像的片

2   片分区A

3   片分区B

4   片分区C

5   IDR图像中的片

6   补充增强信息单元（SEI）

7   SPS

8   PPS

9   序列结束

10  序列结束

11  码流借宿

12  填充

13-23 保留

24  STAP-A  单一时间的组合包

25  STAP-B  单一时间的组合包

26  MTAP16  多个时间的组合包

27  MTAP24  多个时间的组合包

28  FU-A   分片的单元

29  FU-B   分片的单元

30-31 没有定义

### 7.OpenGL

OpenGL自身是一个巨大的状态机(State Machine)：一系列的变量描述OpenGL此刻应当如何运行。OpenGL的状态通常被称为OpenGL上下文(Context)。我们通常使用如下途径去更改OpenGL状态：设置选项，操作缓冲。最后，我们使用当前OpenGL上下文来渲染。

1.$\textcolor{red}{VAO}$:vertex array object

2.$\textcolor{red}{VBO}$:vertex buffer object

3.$\textcolor{red}{EBO}$:element buffer object   $\textcolor{red}{IBO}$:index buffer object

**图形渲染管线**：graphic pipeline 作用1.3D坐标->2D坐标   2.2D坐标-> 有颜色的像素

**着色器**（shader）并行运行在GPU上，达成图形渲染管线的效果。==》由OpenGL着色器语言（OpenGL shading language）(**GLSL**)编写



