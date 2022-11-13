【分享】菜鸟都知道怎么整合大气层破解包2.0版

    我不是什么大佬大神，也是和大家一样菜鸟入门，通过大佬大神们的教程，知道破解的流程，也学会整合大气层，通过使用觉得大气层越简单越好，那么多插件软件其实用途不大，而且可以根据需要增加。
    
https://github.com/laila509/hekate_ipl

    2.0版的教程比起前面1.0版的教程，补充Sigpatch获取方法，补充Tesla插件核心版本的区别。

【第一章  大气层纯净包整合教程】

    1.整合大气层首先做纯净包，就是Atmosphere+Hekate+Sigpatch的三件套，有了它就可以破解系统，运行已安装的破解游戏。纯净包是基础包可直接用，只要加合适的引导就可适用所有破解机，不区分软破用的TX注入器或DQC注入器，不区分TX芯片还是HW芯片，软硬破通用。另外通用的还有Switch系统的离线升级包，最新SW15.0.1系统。
    
    大气层整合包=纯净包+搭配包，搭配包就是特斯拉等一众插件和相册NRO软件的组合，搭配包不能直接使用，需要覆盖到纯净包或其它大气层整合包上使用，整合包应实用为主，需要插件和软件多少可以不断的添加删减。
    
    2.最新的三件套版本是Atmosphere1.4.0，Hekate5.9.0，Sigpatch最高15.0.0/15.0.1
    
    Atmosphere，https://github.com/Atmosphere-NX/Atmosphere/releases
    
    下atmosphere-1.4.0-master的zip和fusee.bin，zip解包后是[atmosphere],[switch]和hbmenu.nro
    
    Hekate，https://github.com/CTCaer/hekate/releases
    
    下hekate_ctcaer_5.9.0.zip，zip解包后是[bootloader]和hekate_ctcaer_5.9.0.bin
    
    3.Sigpatch大气层签名补丁，允许大气层玩破解游戏，原下载https://github.com/ITotalJustice/patches/releases，
    
    已被关闭，所以可以自制或从其它地方找。
    
    自制方法是下载https://github.com/mrdude2478/IPS_Patch_Creator/releases，
    
    在电脑上打开IPS_Patch_Creator，还要有大气层的package3文件、Switch系统离线升级包以及Lockpick_rcm获取对应Switch系统的prod.key。
    
    Sigpatch分ES patch，FS patch，Loader patch和nfim_ctest，前三个和玩破解游戏有关，nfim_ctest可以跳过联网服务器认证。
    
    其中ES patch，FS patch和nfim_ctest需要switch离线升级包和对应prod.key才可以做出来，loader patch只需要大气层package3文件就可以做出来。大气层的引导分fss0和fusee，其中ES patch和nfim_ctest，两种引导一样，FS patch和Loader patch，两种引导不一样，但是只要都在TF卡上就不会出问题。
    
    做出来的sigpatch是通用所有对应的大气层整合包，所以还可以从别人发布的大气层1.4.0整合包里获取。
    
    sigpatch提取的位置是atmosphere/exefs_patches的es_patches和nfim_ctest两个文件夹，atmosphere/kip_patches的fs_patches和loader_patches两个文件夹，最后还有一个文件是bootloader/patches.ini，它是fss0引导的FS patch和Loader patch。
    
    4.三件套不分先后复制到同一目录，fusee.bin移至/bootloader/payloads/
    
    5.通用的大气层设置，适用所有大气层包，在奇乐融合大气层破解包中找
    
override_config.ini，stratosphere.ini和system_settings.ini复制/atmosphere/config/

exosphere.ini，万能前端hbmenu.nsp和sx gear的boot.dat复制根目录

把hekate_ctcaer_5.9.0.bin重命名payload.bin

把Lockpick_RCM.bin，TegraExplorer.bin等常用引导文件复制/bootloader/payloads/

emummc.txt复制/atmosphere/hosts/

如想真实破解系统也开hosts和序号保护，把emummc.txt再复制成sysmmc.txt，打开exosphere.ini把blank_prodinfo_sysmmc=0改成blank_prodinfo_sysmmc=1保存。

    6.自编的Hekate设置，所有大气层包就这差别，可参照乐享或奇想大气层包调整
    
/bootloader/hekate_ipl.ini是启动文件，设置启动全靠它

/bootloader/bootlogo.bmp是开机图，文件名和位置以hekate_ipl.ini为准

/bootloader/res/的图片是启动图标，文件名和位置以hekate_ipl.ini为准

/bootloader/ini/的启动文件也和hekate_ipl.ini相似，在launch右边的more configs出现

    7.后期升级三件套，只要在原版本上对比添加新的组件就行了
    
下Beyond Compare，左右两边分别是旧整合包和新的三件套组件，区分哪些需要更新

【第二章  大气层纯净包文件列表】
[atmosphere]，是大气层的核心
  -|-|-[config]，大气层配置文件
    -|-|-override_config.ini，非必要，大气层默认设置
    -|-|-stratosphere.ini，非必要，Hekate设置autonogc=1也可以
    -|-|-system_settings.ini，必要，开hosts和关闭金手指自动启动
  -|-|-[exefs_patches]，大气层插件
     -|-|-[es_patches]，es插件，与SW系统对应，因大气层向下兼容，所以所有SW系统都放上
        -|-|-这里一堆IPS文件，如现在总共有23个IPS文件，其中一个对应15.0.0/15.0.1系统
     -|-|-[nfim_ctest]，免联服务器插件，与SW系统对应，同样都放上
        -|-|-这里一堆IPS文件，和es插件同理
  -|-|-[fatal_errors]，非必要，大气层运行出错会生成日志
  -|-|-[flags]，非必要，可删
  -|-|-[hbl_html]，非必要，可删
  -|-|-[hosts]，host列表保护，阻止连服务器
    -|-|-default.txt，默认不用管
    -|-|-emummc.txt，这是在虚拟系统做host保护
  -|-|-[kip_patches]，大气层插件
    -|-|-[fs_patches]，fs插件，和es插件同理
        -|-|-这里一堆IPS文件，但每次更新fs插件是2个，分别是exfat和fat32
    -|-|-[loader_patches]，loader插件，与大气层版本对应，只需要一个
        -|-|-这里只需一个IPS文件，对应大气层版本，就是与当前package3对应
  -|-|-hbl.nsp，有了它才能进相册，homebrew启动器
  -|-|-package3，大气层模块，以前名叫fusee-secondary.bin
  -|-|-reboot_payload.bin，重启payload，可以在Hekate设置，也可以用Hekate重命名
  -|-|-stratosphere.romfs，大气层模块，和package3，fusee.bin构成大气层基本核心
[bootloader]，是Hekate的核心
  -|-|-[ini]，是Hekate中more configs的引导配置
    -|-|-lakka.ini，非必要，可以编辑
  -|-|-[payloads]，可以在Hekate中加载它们
    -|-|-commonproblemresolver.bin，一键关闭插件启动和删除主题，解决开机问题
    -|-|-fusee.bin，大气层原版引导模块
    -|-|-lockpick_rcm.bin，主机Keys获取工具
    -|-|-tegraexplorer.bin，主机固件获取工具
  -|-|-[res]，Hekate的图标、开机图等，可以编辑，需Hekate_ipl.ini调整
    -|-|-图标分辨率192*192，开机图720*1280，位深度都是32
    -|-|-[sys]，Hekate模块，和Hekate.bin构成Hekate基本核心
    -|-|-这里几个文件都是必要的
  -|-|-bootlogo.bmp，开机图，
  -|-|-hekate_ipl.ini，Hekate启动设置文件
  -|-|-patches.ini，fss0引导的FS和Loader插件以文本，和fusee引导不一样
  -|-|-update.bin，最新Hekate.bin的重命名，不需要更新注入器的Payload.bin
[switch]，所有相册的NRO软件，特斯拉的OVL插件都在这里
  -|-|-daybreak.nro，大气层用Daybreak离线升级系统
boot.dat，它如果是SX Gear的boot文件，TX注入器或TX芯片转大气层必要
exosphere.ini，大气层配置文件，设置虚拟系统隐藏序号保护
hbmenu.nro，相册的工具菜单
hbmenu.nsp，万能前端，相册的工具菜单有高权限
payload.bin，硬破机需要它来启动，TX转大气层也是启动它

【第三章  乐享大气层的Hekate_ipl启动设置】
    /bootloader/hekate_ipl.ini
    [config]，常用设置
autoboot=0，=0开机进Hekate主页，=1开机进第一项[ATM-AUTO]
autoboot_list=0，=0，=1是more configs启动设置
bootwait=3，开机图3秒内按音量-键返回Hekate主页
backlight=100，背光亮度=100
autohosoff=0，续航/OLED=1，防止关机重启
autonogc=1，防止游戏卡槽升级固件
updater2p=1，启动FSS0引导，就会替换reboot_payload.bin为Hekate。
    [ATM-AUTO]，自动识别真实或虚拟系统（原版FUSEE引导）
payload=bootloader/payloads/fusee.bin，大气层原版引导fusee.bin
icon=bootloader/res/fusee.bmp，启动图标
    [ATM-EMU]，虚拟系统（Hekate的Fss0引导）
emummcforce=1，启动虚拟系统
fss0=atmosphere/package3，Hekate的FSS0引导
kip1patch=nosigchk，启动sigpatch
atmosphere=1，启动大气层破解权限
logopath=bootloader/bootlogo.bmp，开机图
icon=bootloader/res/emunand.bmp，启动图标
id=cfw-emu，fss0引导的启动项ID，在fastcfwswitch可切换
    [OFW-SYS]，真实正版系统（Hekate的Fss0引导）
emummc_force_disable=1，关闭虚拟系统
fss0=atmosphere/package3，Hekate的FSS0引导
stock=1，启动正版系统引导
icon=bootloader/res/icon_switch.bmp，启动图标
id=ofw-sys，启动项ID
    [ATM-SYS]，真实破解系统（Hekate的Fss0引导）
emummc_force_disable=1，关闭虚拟系统
fss0=atmosphere/package3，Hekate的FSS0引导
kip1patch=nosigchk，启动sigpatch
atmosphere=1，启动大气层破解权限
logopath=bootloader/bootlogo.bmp，开机图
icon=bootloader/res/sysnand.bmp，启动图标
id=cfw-sys，启动项ID

【第四章  大气层搭配包整合教程】
    1.搭配包分三类，分别是特斯拉内核、大气层插件、相册NRO软件。
    2.特斯拉内核三个构成，分别是特斯拉启动器ovlloader，特斯拉菜单ovlmenu.ovl，特斯拉系统设定ovlSysmodules.ovl，这是必须的，相当于特斯拉插件的纯净版。其中ovlloader和ovlmenu.ovl是基础核心，是对应大气层的libnx开发的，其它插件都要符合这个核心，否则会出错。
    特斯拉核心目前分两种
原版核心的ovlloader和ovlmenu.ovl是在这里下载
https://github.com/WerWolv/nx-ovlloader/releases
https://github.com/WerWolv/Tesla-Menu/releases
修改版核心的ovlloader和ovlmenu.ovl是在这里下载
https://www.91tvg.com/thread-222735-1-1.html
两者的区别是
原版特斯拉核心是基于旧的大气层libnx开发，其它在Github上发布特斯拉平台的大气层插件都是符合这个核心开发，所以兼容性和稳定性好，插件更新也快，但汉化版少。
修改版核心是zdm65477730基于新的大气层libnx开发，而且做了验证，所以要想用其它的插件，需要zdm65477730进行开发适配才可以用，优点有多语言菜单。
我发的奇乐融合大气层破解包里带的特斯拉核心是zdm65477730基于旧的大气层libnx开发，可以支持github上的大气层插件。
    3.大气层的插件有很多，多个少个都行，随心添加删除。只是插件可以进驻系统后台，会占资源，容易造成系统奔溃，除了ovlloader，其它大部分的大气层插件也都有启动器，在/atmosphere/contents/，所以除特斯拉启动器，其它大气层插件的启动器不要启动，如果要启动可以去深海工具箱的后台服务启动。有需要再开，不需要就关掉，保证系统稳定。
    4.大气层插件和特斯拉挂钩的，不少插件就有ovl和nro两个版本，ovl版功能阉割，但能嵌入特斯拉菜单，nro功能全，但要进相册开启这个nro，两个版本可以择其一，如sys-clk超频插件、edizon金手指插件。
    还有的大气层插件只有ovl版本，如emiibo插件、fastcfwswitch插件。
    还有的大气层插件只有启动器，没有ovl或nro版本，但可以在deepsea工具箱启动，如sys-con手柄插件、missioncontrol蓝牙手柄插件。
    5.相册里的软件，叫它NRO软件，种类也有很多，都是一个.nro文件，多个少个都行，随心添加删除，搭配包组合就包含AtmoXL-Titel-Installer.nro游戏安装器、Checkpoint.nro存档管理器等。有些nro文件会在指定Switch/子文件夹中生成config.json，所以这些nro文件要放在Switch/指定的子文件夹中，否则会在相册出现空的文件夹。

【第五章  大气层搭配包文件列表】
[atmosphere]，搭配包A组合
  -|-|-[contents]，是插件的启动器
    -|-|-[00FF0000636C6BFF]，sys-clk超频插件启动器
    -|-|-[00FF0000636C6BFFsys-clk]，空目录名为方便找位置
    -|-|-[0000000000534C56]，SaltyNX底座手持插件启动器
    -|-|-[0000000000534C56SaltyNX]
    -|-|-[420000000007E51A]，特斯拉启动器
      -|-|-[flags]
        -|-|-boot2.flag，空文件的作用是启动，删除它就不会启动
      -|-|-exefs.nsp，是特斯拉插件启动器的文件
      -|-|-toolbox.json，能在深海工具箱里显示，切换开和关
    -|-|-[420000000007E51Atesla]
    -|-|-[690000000000000D]，sys-con手柄插件启动器
    -|-|-[690000000000000Dsys-con]
[config]，插件配置文件
    -|-|-[sys-clk]
    -|-|-[sys-con]
    -|-|-[tesla]
      -|-|-config.ini，特斯拉启动组合键
[SaltySD]，底座手持插件的组件
[switch]，是相册NRO软件的目录
    -|-|-[.overlays]，特斯拉插件的ovl文件目录
      -|-|-[lang]，特斯拉插件的语言包
      -|-|-ovlEdiZon.ovl，金手指Edizon插件
      -|-|-ovlmenu.ovl，特斯拉菜单
      -|-|-ovlSysmodules.ovl，特斯拉系统设定
      -|-|-ReverseNX-RT-ovl.ovl，底座手持插件
      -|-|-Status-Monitor-Overlay.ovl，系统监控插件
      -|-|-sys-clk-overlay.ovl，超频插件
    -|-|-[AtmoXL-Titel-Installer]
      -|-|-AtmoXL-Titel-Installer.nro，游戏安装器
    -|-|-[Checkpoint]
      -|-|-Checkpoint.nro，存档管理器
    -|-|-[DBI]
      -|-|-dbi.nro，游戏安装器
      -|-|-dbi.config，游戏安装器设置
    -|-|-[DeepSea-Toolbox]
      -|-|-DeepSea-Toolbox.nro，深海工具箱
    -|-|-[goldleaf]
      -|-|-goldleaf.nro，文件管理器
    -|-|-[jksv]
      -|-|-jksv.nro，存档管理器
    -|-|-NXThemesInstaller.nro，主题安装器

