【分享】大气层启动设置的互换

研究了大气层的启动设置，主要区别就是Hekate_ipl.ini的设置。我发布的乐享大气层包和奇想大气层包，两者的区别就是启动设置，所以如果你想互换也是没有任何问题，因为乐享大气层包和奇想大气层包的Atmosphere和Sigpatch都是一样的。

互换方法1，把乐享大气层纯净包覆盖到奇想大气层纯净包，反过来也一样。

互换方法2，把乐享大气层启动设置的文件夹Bootloader覆盖到奇想大气层纯净包，反过来也一样。

两种互换的结果都一样

由此可以得出结论，论坛所有的大气层整合包都可以这样换来换去，和换皮肤MOD一样，比如Alan大佬的Deepsea，刺心大佬的原版大气层，Yuanbanban大佬的增强大气层，AK大佬的AK大气层等等。

需注意的是

1.注意Hekate_ipl.ini启动设置里的文件名称，比如刺心大佬把/bootloader/payloads/里的fusee.bin改名为atmosphere.bin，也有大佬把fusee.bin改成另外的名称。

2.注意Fusee引导和Fss0引导的FS和Loader补丁位置不一样，只要都在就可以了。

3.不同的大气层整合包，除了启动顺序，还有Hosts和隐藏序号的区别，有些是只启动虚拟系统保护，真实系统不受限，其实这样就够了，但是还有些整合包是真实破解和虚拟都启用了保护。

4.整合包=纯净包+搭配包，纯净包是必要的，但搭配包不一定要很多很全，因为很多插件和软件要么重复，要么无用，留个DBI，JKSV，Checkpoint和Edizon就够了。

<img src="https://GitHub.com/laila509/hekate_ipl/blob/master/Hekate_ipl.jpg?raw=true" align="center" width="80%" />
