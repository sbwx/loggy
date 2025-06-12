/*
 * sd.h
 */
 
 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 #include <stdio.h>
 #include <string.h>
 #include <zephyr/drivers/spi.h>

 #include <zephyr/storage/disk_access.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/fs/fs.h>

 #include <zephyr/drivers/rtc.h>

 #if defined(CONFIG_FAT_FILESYSTEM_ELM)

 #include <ff.h>
 
 /*
  *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
  *  in ffconf.h
  */
 #if defined(CONFIG_DISK_DRIVER_MMC)
 #define DISK_DRIVE_NAME "SD2"
 #else
 #define DISK_DRIVE_NAME "SD"
 #endif
 
 #define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"
 
 static FATFS fat_fs;
 /* mounting info */
 static struct fs_mount_t mp = {
	 .type = FS_FATFS,
	 .fs_data = &fat_fs,
 };
 
 #elif defined(CONFIG_FILE_SYSTEM_EXT2)
 
 #include <zephyr/fs/ext2.h>
 
 #define DISK_DRIVE_NAME "SD"
 #define DISK_MOUNT_PT "/ext"
 
 static struct fs_mount_t mp = {
	 .type = FS_EXT2,
	 .flags = FS_MOUNT_FLAG_NO_FORMAT,
	 .storage_dev = (void *)DISK_DRIVE_NAME,
	 .mnt_point = "/ext",
 };
 
 #endif
 
 #if defined(CONFIG_FAT_FILESYSTEM_ELM)
 #define FS_RET_OK FR_OK
 #else
 #define FS_RET_OK 0
 #endif
 
 LOG_MODULE_REGISTER(main);

 const struct device* rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
 
 static const char *disk_mount_pt = DISK_MOUNT_PT;

 static struct fs_file_t data_filp;

 static char file_name[29];

 struct rtc_time time = {
	.tm_year = 2025 - 1900,
	.tm_mon = 03 - 1,
	.tm_mday = 27,
	.tm_hour = 16,
	.tm_min = 30,
	.tm_sec = 0,
};

 void csv_create() {
	rtc_set_time(rtc, &time);	
	rtc_get_time(rtc, &time);
	 
	 do {
		 static const char *disk_pdrv = DISK_DRIVE_NAME;
		 uint64_t memory_size_mb;
		 uint32_t block_count;
		 uint32_t block_size;
 
		 if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_CTRL_INIT, NULL) != 0) {
			 LOG_ERR("Storage init ERROR!");
			 break;
		 }

		 if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
			 LOG_ERR("Unable to get sector count");
			 break;
		 }
		 LOG_INF("Block count %u", block_count);
 
		 if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
			 LOG_ERR("Unable to get sector size");
			 break;
		 }
		 printf("Sector size %u\n", block_size);
 
		 memory_size_mb = (uint64_t)block_count * block_size;
		 printf("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));
 
		 if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
			 LOG_ERR("Storage deinit ERROR!");
			 break;
		 }
	 } while (0);

	 mp.mnt_point = disk_mount_pt;

	 int res = fs_mount(&mp);

	 while (res != FS_RET_OK) {
		printf("Error mounting disk.\n");
		res = fs_mount(&mp);
	 }

	 fs_file_t_init(&data_filp);

	 sprintf(file_name, "/SD:/%04d-%02d-%02d_%02d-%02d-%02d.csv", (time.tm_year + 1900), time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
	
	 fs_open(&data_filp, file_name, FS_O_CREATE | FS_O_WRITE);

	 fs_close(&data_filp);

		/* raw disk i/o */
 }

 /*
 Timestamp, CH1, CH2...

 
 
 */
 void write_csv(char* file_data_buffer) {
    int err = fs_open(&data_filp, file_name, FS_O_CREATE | FS_O_APPEND | FS_O_WRITE);
    fs_write(&data_filp, file_data_buffer, strlen(file_data_buffer));
    fs_close(&data_filp);
 }

 void format_csv_header() {
    write_csv("Timestamp, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8\n");
 }

 void csv_add_entry(float data[8]) {
	rtc_get_time(rtc, &time);
	char file_data_buffer[256];
	sprintf(file_data_buffer, "%04d-%02d-%02d_%02d-%02d-%02d.%03d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n", (time.tm_year + 1900), time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, (time.tm_nsec/1000000), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
	write_csv(file_data_buffer);
 }