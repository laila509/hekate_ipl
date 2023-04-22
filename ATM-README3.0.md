【分享】菜鸟都知道怎么整合大气层破解包3.0版-2023.4.22更新

我不是什么大佬大神，也是和大家一样菜鸟入门，通过大佬大神们的教程，知道破解的流程，也学会整合大气层，通过使用觉得大气层越简单越好，那么多插件软件其实用途不大，而且可以根据需要增加。

https://github.com/laila509/hekate\_ipl

https://codeberg.org/laila509/Switch-SDsetup

3\.0版的教程比起前面2.0版的教程，补充Tesla纯净包制作方法和常用NRO软件的下载源。

【第一章  大气层纯净包整合教程】

一、整合大气层首先做纯净包，就是Atmosphere+Hekate+Sigpatch的三件套，有了它就可以破解系统，运行已安装的破解游戏。大气层破解包和SW系统离线升级包这两个文件包在所有Switch型号都是通用的，不分国行非国行，不分续航非续航，不分oled和lite。

大气层破解包的根目录有sx gear的boot.dat和Hekate重命名的payload.bin两个引导文件，软破用的TX注入器和硬破用的TX芯片将以Hekate开机。

大气层整合包=纯净包+搭配包，搭配包就是特斯拉等一众插件和相册NRO软件的组合，搭配包不能直接使用，需要覆盖到纯净包或其它大气层整合包上使用，整合包应实用为主，需要插件和软件多少可以不断的添加删减。

2\.最新的三件套版本是Atmosphere1.5.2，Hekate6.0.3，Sigpatch最高到SW16.0.2系统。

Atmosphere，https://github.com/Atmosphere-NX/Atmosphere/releases

下atmosphere-1.5.2-master的zip和fusee.bin，zip解包后是[atmosphere],[switch]和hbmenu.nro

Hekate，https://github.com/CTCaer/hekate/releases

下hekate\_ctcaer\_6.0.3.zip，zip解包后是[bootloader]和hekate\_ctcaer\_6.0.3.bin

3\.Sigpatch大气层签名补丁，允许大气层玩破解游戏，原下载https://github.com/ITotalJustice/patches/releases，已被关闭，所以可以自制或从其它地方找，有这源https://github.com/fraxalotl/SwitchScript

自制方法是下载https://github.com/mrdude2478/IPS\_Patch\_Creator/releases，在电脑上打开IPS\_Patch\_Creator，还要有大气层的package3文件、Switch系统离线升级包以及Lockpick\_rcm获取对应Switch系统的prod.key。

Sigpatch分ES patch，FS patch，Loader patch和nfim\_ctest，前三个和玩破解游戏有关，nfim\_ctest可以跳过联网服务器认证。其中ES patch，FS patch和nfim\_ctest需要switch离线升级包和对应prod.key才可以做出来，loader patch只需要大气层package3文件就可以做出来。大气层的引导分fss0和fusee，其中ES patch和nfim\_ctest，两种引导一样，FS patch和Loader patch，两种引导不一样，但是只要都在TF卡上就不会出问题。

做出来的sigpatch是通用所有对应的大气层整合包，所以还可以从别人发布的大气层整合包里获取。

sigpatch提取的位置是atmosphere/exefs\_patches的es\_patches和nfim\_ctest两个文件夹，atmosphere/kip\_patches的fs\_patches和loader\_patches两个文件夹，最后还有一个文件是bootloader/patches.ini，它是fss0引导的FS patch和Loader patch。

4\.三件套不分先后复制到同一目录，fusee.bin移至/bootloader/payloads/

5\.通用的大气层设置，适用所有大气层包，在奇乐融合大气层破解包中找

override\_config.ini，stratosphere.ini和system\_settings.ini复制/atmosphere/config/

exosphere.ini，万能前端hbmenu.nsp和sx gear的boot.dat复制根目录

把hekate\_ctcaer.bin重命名payload.bin

把Lockpick\_RCM.bin，TegraExplorer.bin等常用引导文件复制/bootloader/payloads/

emummc.txt复制/atmosphere/hosts/

如想真实破解系统也开hosts和序号保护，把emummc.txt再复制成sysmmc.txt，打开exosphere.ini把blank\_prodinfo\_sysmmc=0改成blank\_prodinfo\_sysmmc=1保存。

6\.自编的Hekate设置，所有大气层包就这差别，可参照奇乐融合大气层包调整

/bootloader/hekate\_ipl.ini是启动文件，设置启动全靠它

/bootloader/bootlogo.bmp是开机图，文件名和位置以hekate\_ipl.ini为准

/bootloader/res/的图片是启动图标，文件名和位置以hekate\_ipl.ini为准

/bootloader/ini/的启动文件也和hekate\_ipl.ini相似，在launch右边的more configs出现

7\.后期升级三件套，只要在原版本上对比添加新的组件就行了，莱莱推荐AK478BB大佬发布的可覆盖的大气层三件套升级文件

，直接覆盖就可以完成大气层三件套的更新，不会改变你当前的大气层/Hekate的各种设定参数。

https://github.com/AK478BB/Sigpatches/releases

推荐下载更新大气层和特斯拉的利器：Beyond Compare文件对比工具，左右两边分别是旧整合包和新的三件套组件，区分哪些需要更新，超级好用。

【第二章  大气层纯净包文件列表】

[atmosphere]，是大气层的核心

-|-|-[config]，大气层配置文件

-|-|-override\_config.ini，非必要，大气层默认设置

-|-|-stratosphere.ini，非必要，Hekate设置autonogc=1也可以

-|-|-system\_settings.ini，必要，开hosts和关闭金手指自动启动

-|-|-[exefs\_patches]，大气层插件

-|-|-[es\_patches]，es插件，与SW系统对应，因大气层向下兼容，所以所有SW系统都放上

-|-|-这里一堆IPS文件，其中一个对应最新的16.0.0系统（通用16.0.1和16.0.2系统）

-|-|-[nfim\_ctest]，免联服务器插件，与SW系统对应，同样都放上

-|-|-这里一堆IPS文件，和es插件同理，其中一个对应最新的16.0.0系统（通用16.0.1和16.0.2系统）

-|-|-[fatal\_errors]，非必要，大气层运行出错会生成日志

-|-|-[flags]，非必要，可删

-|-|-[hbl\_html]，非必要，可删

-|-|-[hosts]，host列表保护，阻止连服务器

-|-|-default.txt，默认不用管

-|-|-emummc.txt，这是在虚拟系统做host保护

-|-|-[kip\_patches]，大气层插件

-|-|-[fs\_patches]，fs插件，和es插件同理，但分fat32和exfat两个

-|-|-这里一堆IPS文件，其中两个对应最新的16.0.0系统（通用16.0.1和16.0.2系统）

-|-|-[loader\_patches]，loader插件，与大气层版本对应，只需要一个

-|-|-这里只需一个IPS文件，对应大气层版本，就是与当前package3对应

-|-|-hbl.nsp，有了它才能进相册，homebrew启动器

-|-|-package3，大气层模块，以前名叫fusee-secondary.bin

-|-|-reboot\_payload.bin，重启payload，可以在Hekate设置，也可以用Hekate重命名

-|-|-stratosphere.romfs，大气层模块，和package3，fusee.bin构成大气层基本核心

[bootloader]，是Hekate的核心

-|-|-[ini]，是Hekate中more configs的引导配置

-|-|-lakka.ini，非必要，可以编辑

-|-|-[payloads]，可以在Hekate中加载它们

-|-|-commonproblemresolver.bin，一键关闭插件启动和删除主题，解决开机问题

-|-|-fusee.bin，大气层原版引导模块

-|-|-lockpick\_rcm.bin，主机Keys获取工具

-|-|-tegraexplorer.bin，主机固件获取工具

-|-|-hwflytoolbox.bin，国产芯片工具箱，升级sdloader

-|-|-[res]，Hekate的图标、开机图等，可以编辑，需Hekate\_ipl.ini调整

-|-|-图标分辨率192\*192，开机图720\*1280，位深度都是32

-|-|-[sys]，Hekate模块，和Hekate.bin构成Hekate基本核心

-|-|-这里几个文件都是必要的

-|-|-bootlogo.bmp，开机图，

-|-|-hekate\_ipl.ini，Hekate启动设置文件

-|-|-patches.ini，fss0引导的FS和Loader插件以文本，和fusee引导不一样

-|-|-update.bin，最新Hekate.bin的重命名，不需要更新注入器的Payload.bin

[switch]，所有相册的NRO软件，特斯拉的OVL插件都在这里

-|-|-daybreak.nro，大气层用Daybreak离线升级系统

boot.dat，它如果是SX Gear的boot文件，TX注入器或TX芯片转大气层必要

exosphere.ini，大气层配置文件，设置虚拟系统隐藏序号保护

hbmenu.nro，相册的工具菜单

hbmenu.nsp，万能前端，相册的工具菜单有高权限

payload.bin，硬破机需要它来启动，TX转大气层也是启动它

【第三章  乐享大气层的Hekate\_ipl启动设置】

/bootloader/hekate\_ipl.ini

[config]，常用设置

autoboot=0，=0开机进Hekate主页，=1开机进第一项[ATM-AUTO]

autoboot\_list=0，=0，=1是more configs启动设置

bootwait=3，开机图3秒内按音量-键返回Hekate主页

backlight=100，背光亮度=100

autohosoff=1，续航/OLED/Lite设置=1，防止关机重启

autonogc=1，防止游戏卡槽升级固件

updater2p=1，启动FSS0引导，就会替换reboot\_payload.bin为Hekate。

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

emummc\_force\_disable=1，关闭虚拟系统

fss0=atmosphere/package3，Hekate的FSS0引导

stock=1，启动正版系统引导

icon=bootloader/res/icon\_switch.bmp，启动图标

id=ofw-sys，启动项ID

[ATM-SYS]，真实破解系统（Hekate的Fss0引导）

emummc\_force\_disable=1，关闭虚拟系统

fss0=atmosphere/package3，Hekate的FSS0引导

kip1patch=nosigchk，启动sigpatch

atmosphere=1，启动大气层破解权限

logopath=bootloader/bootlogo.bmp，开机图

icon=bootloader/res/sysnand.bmp，启动图标

id=cfw-sys，启动项ID

【第四章  大气层搭配包整合教程】

1. 搭配包分三类，分别是特斯拉内核、大气层插件、相册NRO软件。
1. 特斯拉内核三个构成，分别是特斯拉启动器ovlloader，特斯拉菜单ovlmenu.ovl，特斯拉系统设定ovlSysmodules.ovl，这是必须的，相当于特斯拉插件的纯净版。其中ovlloader和ovlmenu.ovl是基础核心，是对应大气层的libnx开发的，其它插件都要符合这个核心，否则会出错。

特斯拉核心目前分两种

原版核心的ovlloader和ovlmenu.ovl是在这里下载

https://github.com/WerWolv/nx-ovlloader/releases

https://github.com/WerWolv/Tesla-Menu/releases

修改版核心的ovlloader和ovlmenu.ovl是在这里下载

https://www.91tvg.com/thread-222735-1-1.html

两者的区别是

原版特斯拉核心是只能使用英文版的插件，外国人开发的。

修改版核心是zdm65477730开发，优化和稳定，且有多国语言包，

我发的奇乐融合大气层破解包里带的特斯拉核心是Z大发布的，推荐更新。

1. 大气层的插件有很多，多个少个都行，随心添加删除。只是插件可以进驻系统后台，会占资源，容易造成系统奔溃，除了ovlloader，其它大部分的大气层插件也都有启动器，在/atmosphere/contents/，所以除特斯拉启动器，其它大气层插件的启动器不要启动，如果要启动可以去深海工具箱的后台服务启动。有需要再开，不需要就关掉，保证系统稳定。
1. 大气层插件和特斯拉挂钩的，不少插件就有ovl和nro两个版本，ovl版功能阉割，但能嵌入特斯拉菜单，nro功能全，但要进相册开启这个nro，两个版本可以择其一，如sys-clk超频插件、edizon金手指插件。

还有的只有启动器和ovl插件，如emuiibo插件，所有游戏的amiibo模拟文件路径tf：emuiibo/amiibo/存放

还有的只有启动器，没有ovl插件或nro软件，但可以在deepsea工具箱后台管理它们的启动，如sys-con手柄插件、missioncontrol蓝牙手柄插件。

还有的只有ovl插件，没有启动器和nro软件，如fastcfwswitch插件，

1. 相册里的软件，叫它NRO软件，种类也有很多，都是一个.nro文件，多个少个都行，随心添加删除，NRO软件都在TF：switch/，替换最新的.NRO文件就完成，搭配包组合就包含AtmoXL-Titel-Installer.nro游戏安装器、Checkpoint.nro存档管理器等。

有些nro文件会在指定Switch/子文件夹中生成config.json，所以这些nro文件要放在Switch/指定的子文件夹中，否则会在相册出现空的文件夹。

有些nro文件会自动生成config文件，比如dbi.nro会自动生成设置文件dbi.config，不想改原设置选项，就只需要覆盖新的dbi.nro，不要覆盖新的dbi.confi

【第五章 整合纯净版Tesla教程】

Z大自从做了终极版Tesla，现在已经比原版还要好用，底座模式不死机，所有插件都兼容16.0和以上系统。莱莱强烈推荐。

因为不是每个人都需要全部的特斯拉插件，所以莱莱教大家怎么整合纯净版特斯拉包，有这个纯净包，小伙伴们可以在这基础上随意添加其它任何的ovl插件和启动器。

管理Tesla插件的确没有管理相册NRO软件那样直接替换一个nro那么简单，但是莱莱折腾Tesla以后并不觉得有什么门槛，都是很基础的折腾方法。

1. 纯净Tesla的组件和原版下载地址

想玩Tesla插件，必定要有下面三个组件，其它的所有ovl插件都要基于这三个组件才能工作。

（1）nx-ovlloader，Tesla启动器

https://github.com/WerWolv/nx-ovlloader/releases

（2）ovlmenu，Tesla菜单

https://github.com/WerWolv/Tesla-Menu/releases

（3）ovlSysmodules，Tesla系统管理

https://github.com/WerWolv/ovl-sysmodules/releases

因为Z大佬基于这些插件做了优化编译

https://www.tekqart.com/thread-222735-1-1.html

https://github.com/zdm65477730

但是Z大终结版Tesla的各种插件必须基于Z大的特斯拉核心，但Z大的特斯拉核心兼容其他人发布的各种原版ovl插件。

1. 纯净Tesla的组件文件名和路径

（1）nx-ovlloader，Tesla启动器

路径在tf：atmosphere/contents/420000000007E51A/，其中

tf：atmosphere/contents/420000000007E51A/exefs.nsp，是Tesla启动器的核心文件

tf：atmosphere/contents/420000000007E51A/flags/boot2.flag，有boot2.flag文件=进系统自动启动

tf：atmospre/contents/420000000007E51A/toolbox.json，支持Deepsea工具箱开关插件的自动启动

（2）ovlmenu，Tesla菜单

有多个文件夹和文件，其中

tf：switch/.overlays/ovlmenu.ovl，是Tesla菜单的文件

tf：switch/.overlays/lang/TeslaMenu/，是Tesla菜单的多国语言包

tf：config/Tesla/config.ini，设定Tesla菜单的快捷键，没有就是默认的L+DDOWN+RS三键

tf：config/Tesla-Menu/sort.cfg，设定ovl插件上下排序，Z大编译的独特功能，和原版排序方法不一样

（3）ovlSysmodules，Tesla系统管理

有多个文件夹和文件，其中

tf：switch/.overlays/ovlsysmodules.ovl，Tesla系统管理的文件

tf：switch/.overlays/lang/Sysmodules/，是Tesla系统管理的多国语言包

tf：config/ovl-sysmodules/config.ini，是Tesla系统管理菜单里功能的显示和隐藏，建议显示

1. 其它常用的ovl插件和路径

莱莱整合和搬运了Z大佬的终极版特斯拉，这里先教小伙伴们了解常用的ovl插件

（1）ovledizon，金手指插件

tf：switch/.overlays/edizon.ovl，是金手指插件ovl版本的文件，金手指码放在atmosphere/contents/

tf：switch/.overlays/lang/edizon/，是金手指软件ovl的多国语言包

tf：switch/EdiZon/EdiZon.nro，是金手指插件nro版本

（2）sys-clk-overlay，超频插件

tf：atmosphere/contents/00FF0000636C6BFF/，是超频插件的启动器，和Tesla启动器工作原理一样

其中00FF0000636C6BFF/exefs.nsp是超频插件启动器的核心文件

tf：config/sys-clk/config.ini，是超频插件设定文件，可删，有超频的ovl或nro都可以管理设定

tf：switch/.overlays/sys-clk.ovl，是超频插件管理器ovl版本

tf：switch/.overlays/lang/sys-clk/，是超频插件管理器ovl版本的多国语言包

tf：switch/sys-clk-manager.nro，是超频插件管理器nro版本

（3）ReverseNX-RT，手持底座切换插件

手持底座切换插件还包含了saltynx启动器和NX-FPS.elf两个组件

Status-Monitor-Overlay，系统监视器插件

这两个插件原版都由一位作者发布，而且系统监视器的实时FPS功能也要基于saltynx启动器才能工作

tf：atmosphere/contents/0000000000534C56/，是saltynx启动器，和Tesla启动器工作原理一样

其中0000000000534C56/exefs.nsp是saltynx启动器的核心文件

tf：switch/.overlays/ReverseNX-RT.ovl，是手持底座切换插件的文件，可以设定掌机下屏显主机分辨率

tf：switch/.overlays/lang/ReverseNX-RT/，是手持底座切换插件的多国语言包

tf：switch/.overlays/StatusMonitor.ovl，是系统监视器插件的文件

tf：switch/.overlays/lang/StatusMonitor/，是系统监视器插件的多国语言包

tf：SaltySD/，是手持底座切换插件的设定文件，其中

tf：SaltySD/plugins/NX-FPS.elf，是系统监视器的实时FPS功能的文件

tf：SaltySD/exceptions.txt，是因为部分游戏不支持手持底座切换容易出错，可编辑的免启动名单

（4）missioncontrol，支持蓝牙无线连接第三方手柄插件，这个插件只有启动器，无其它文件

tf：atmosphere/contents/010000000000bd00/，是missioncontrol启动器，和Tesla启动器工作原理一样

其中010000000000bd00/exefs.nsp是missioncontrol启动器的核心文件

tf：atmosphere/exefs\_patches/bluetooth\_patches，是missioncontrol适配SW系统的蓝牙补丁

tf：atmosphere/exefs\_patches/btm\_patches，新加的蓝牙补丁，适配15.0.0以后的系统

tf：config/MissionControl，missioncontrol设定文件，默认或删除都可以

（5）DeepSeaToolbox.nro，深海工具箱和Hekate，Kosmos工具箱一样，有管理启动器是否启动的功能

当删除420000000007E51A/flags/boot2.flag文件致Tesla菜单不能启动的时候，可以在这里开启且不用重启

这里还可以管理其它的启动器是否启动，这个和ovlSysmodules也提供一样的管理功能。

在后台管理各种启动器ON或者OFF后，要先按一次B键，再按一次Home键保存返回主界面，然后重启Switch。

【第六章 常用相册NRO软件】

（0）reboot\_to\_payload.nro，一键关机重启工具，支持Switch全机型，在App name前加00，自动排到相册第一个。

源文件Safe\_Reboot\_Shutdown.nro，改名reboot\_to\_payload.nro，比大气层reboot\_to\_payload.nro好

https://github.com/dezem/Safe\_Reboot\_Shutdown/releases

（1）Switch\_90DNS\_tester.nro，联网检测是否屏蔽任天堂服务器

https://github.com/meganukebmp/Switch\_90DNS\_tester/releases

（2）aio-switch-updater，大气层破解升级工具，联网，前端进

https://github.com/HamletDuFromage/aio-switch-updater/releases

（3）AtmoXL-Titel-Installer，游戏安装工具，前端进

https://github.com/dezem/AtmoXL-Titel-Installer/releases

（4）Firmware-Dumper.nro，系统固件提取工具

https://github.com/mrdude2478/Switch-Firmware-Dumper/releases

（5）Calculator\_NX，简易计算器

https://github.com/EmreTech/Calculator\_NX/releases

（6）Checkpoint，游戏存档管理工具

https://github.com/FlagBrew/Checkpoint/releases

（7）DBI，游戏安装，存档管理和文件传输工具

https://github.com/rashevskyv/dbi/releases

（8）DeepSea-Toolbox，深海工具箱，插件管理，实际是Hekate-toolbox--v4.0.2，支持Mariko型主机重启

https://github.com/WerWolv/Hekate-Toolbox/releases

（9）Edizon-SE，金手指游戏修改工具

https://github.com/tomvita/EdiZon-SE/releases

（10）ftpd，FTP传输工具

https://github.com/mtheall/ftpd/releases

（11）Goldleaf，文件管理工具

https://github.com/XorTroll/Goldleaf/releases

（12）Haku33，系统洗白工具

https://github.com/StarDustCFW/Haku33/releases

（13）HB appstore，破解软件商店，联网，前端进

https://github.com/fortheusers/hb-appstore/releases

https://apps.fortheusers.org/switch

（14）JKSV，游戏存档管理工具

https://github.com/J-D-K/JKSV/releases

（15）linkalho，模拟关联任天堂账号工具

https://github.com/rdmrocha/linkalho/releases

（16）N-Xplorer.nro，文件管理工具，有文本编辑功能

https://github.com/CompSciOrBust/N-Xplorer/releases

（17）NX-Activity-Log，游戏游玩时间记录工具

https://github.com/tallbl0nde/NX-Activity-Log/releases

（18）NX-Shell，文件管理工具

https://github.com/joel16/NX-Shell/releases

（19）nxdumptool，游戏的提取分享工具，前端进

https://github.com/DarkMatterCore/nxdumptool/releases

（20）NXThemesInstaller，主题安装工具和主题资源

https://github.com/exelix11/SwitchThemeInjector/releases

https://themezer.net/packs

15\.0/16.0以后的系统需要systemPatches补丁

https://github.com/exelix11/theme-patches

（21）pplay，影音播放工具

https://github.com/Cpasjuste/pplay/releases

（22）switchtime，联网校准时钟，90dns的破解系统也可用

https://github.com/3096/switch-time/releases

（23）Tencent-switcher-GUI，国行和非国行系统切换工具

https://github.com/CaiMiao/Tencent-switcher-GUI/releases

（24）Tinfoil，黑商店，文件管理工具，自动安装前端

http://tinfoil.io/Download#download

（25）wiliwili，哔哩哔哩播放器，联网，自选安装前端

https://github.com/xfangfang/wiliwili/releases
