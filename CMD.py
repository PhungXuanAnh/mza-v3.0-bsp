
######	Setup enviroment
export ARCH=arm
export YOUR_PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
export PATH=$PATH:/media/DATA/arm_gcc/bin
export MYDROID=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
export PATH=$MYDROID/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH 
export CROSS_COMPILE=arm-eabi- 
export PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot/tools:$PATH 
export TARGET_PRODUCT=beagleboard
export OMAPES=5.x


######	x-loader
cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/x-loader
make distclean	
make ARCH=arm omap3beagle_config
make 2>&1 |tee x-loader_make.out
cp $YOUR_PATH/dm37_tools/signGP/signGP $YOUR_PATH/x-loader
./signGP ./x-load.bin
mv x-load.bin.ift MLO



######	u-boot
cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot
make distclean
make ARCH=arm omap3_beagle_config
make -j8 2>&1 |tee u-boot_make.out


######	kernel
export PATH=$PATH:/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot/tools 
cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/kernel
#make ARCH=arm distclean
#make ARCH=arm omap3_beagle_android_defconfig
make ARCH=arm uImage -j8 2>&1 |tee kernel_make.out
make ARCH=arm modules -j8 2>&1 |tee modules_make.out


### 	wilink
cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/hardware/ti/wlan/mac80211/compat_wl12xx 
export KERNEL_DIR=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/kernel/ 
export KLIB=${KERNEL_DIR} 
export KLIB_BUILD=${KERNEL_DIR} 
make clean
make ARCH=arm -j8



##### 	Android.
export ARCH=arm
export YOUR_PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
export PATH=$PATH:/media/DATA/arm_gcc/bin
export MYDROID=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
export PATH=$MYDROID/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH 
export CROSS_COMPILE=arm-eabi- 
export PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot/tools:$PATH 
export TARGET_PRODUCT=beagleboard
export OMAPES=5.x

cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0	
#make TARGET_PRODUCT=beagleboard clean
make TARGET_PRODUCT=beagleboard OMAPES=5.x -j8 2>&1 |tee android_make.out



###### 	Prepare need file
export ARCH=arm
export YOUR_PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
#	export PATH=$PATH:/home/anhpx/Toolchain/arm-2009q1/bin
export PATH=$PATH:/media/DATA/arm_gcc/bin
export MYDROID=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0
export PATH=$MYDROID/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH 
export CROSS_COMPILE=arm-eabi- 
export PATH=/media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot/tools:$PATH 
export TARGET_PRODUCT=beagleboard
export OMAPES=5.x

cd ${MYDROID}/out/target/product/beagleboard
mkdir -p system/lib/modules
cp ${MYDROID}/hardware/ti/wlan/mac80211/compat_wl12xx/compat/compat.ko system/lib/modules/
cp ${MYDROID}/hardware/ti/wlan/mac80211/compat_wl12xx/net/wireless/cfg80211.ko system/lib/modules/
cp ${MYDROID}/hardware/ti/wlan/mac80211/compat_wl12xx/net/mac80211/mac80211.ko system/lib/modules/
cp ${MYDROID}/hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx.ko system/lib/modules/
cp ${MYDROID}/hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx_sdio.ko system/lib/modules/


### Tao filesystem.
cd /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/out/target/product/beagleboard
mkdir android_rootfs
cp -r root/* android_rootfs
cp -r system android_rootfs


###### 	Create SDcard
sudo ../../../../build/tools/mktarball.sh ../../../host/linux-x86/bin/fs_get_stats android_rootfs . rootfs rootfs.tar.bz2
cd $YOUR_PATH
mkdir prebuilt_images
mkdir prebuilt_images/Boot_Images
cp kernel/arch/arm/boot/uImage prebuilt_images/Boot_Images
cp u-boot/u-boot.bin prebuilt_images/Boot_Images
cp x-loader/MLO prebuilt_images/Boot_Images
cp dm37_tools/mk-bootscr/boot.scr prebuilt_images/Boot_Images
mkdir prebuilt_images/Filesystem
cp out/target/product/beagleboard/rootfs.tar.bz2 prebuilt_images/Filesystem
cp /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/dm37_tools/mk-mmc/mkmmc-android.sh prebuilt_images
cd prebuilt_images

####### 	Make ubi.img
cd /home/anhpx/WORK/mtd-utils/
rm ubifs.img
rm ubi.img
chmod -R 755 /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/out/target/product/beagleboard/android_rootfs/
mkfs.ubifs/mkfs.ubifs -v -r /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/out/target/product/beagleboard/android_rootfs/ -m 2048 -e 126976 -c 3991 -o ubifs.img -x lzo
ubi-utils/ubinize -o ubi.img -O 2048 -m 2048 -p 128KiB ubinize.cfg


#############################################################################	Boot ENV 

######################### NAND

setenv nandboot 'echo Booting from nand ...; nand read ${loadaddr} ${boot_nand_offset} ${boot_nand_size}; bootm ${loadaddr}'
setenv bootcmd 'run nandboot'
setenv bootargs 'init=/init console=ttyO2,115200n8 noinitrd ip=off androidboot.console=ttyO2 rootwait mem=512M omap_vout.vid1_static_vrfb_alloc=y rw ubi.mtd=4,2048 rootfstype=ubifs root=ubi0:rootfs rootdelay=2 vram=8M omapfb.vram=0:8M'
saveenv

######################### MMC

setenv mmcboot 'echo Booting from mmc ...; run mmcargs; bootm ${loadaddr}'
setenv bootcmd 'mmc init;fatload mmc 0 80300000 uImage;bootm 80300000'
setenv bootargs 'console=ttyO2,115200n8 androidboot.console=ttyO2 mem=512M root=/dev/mmcblk0p2 rw rootfstype=ext3 rootdelay=1 init=/init ip=off omap_vout.vid1_static_vrfb_alloc=y omapdss.def_disp=lcd omapfb.mode=lcd:800x480MR-16 vram=8M omapfb.vram=0:8M mpurate=1000'
saveenv


################	Flash down to Nand
fastboot flash xloader /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/x-loader/MLO
fastboot flash bootloader /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/u-boot/u-boot.bin
fastboot flash boot /media/anhpx/anhpx/WORK/sources/mza-v3.0-bsp_0/kernel/arch/arm/boot/uImage




