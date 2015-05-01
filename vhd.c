// vhd.c: Check for a Virtual Hard Disk file
#define _GNU_SOURCE
#include "config.h"
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dat.h"
#include "fns.h"

struct _DiskGeometry
{
	uint16_t cylinder;
	uint8_t heads;
	uint8_t sectors;
} __attribute__ ((packed));
typedef struct _DiskGeometry DiskGeometry;

struct _HardDiskFooter
{
    char cookie[8];
    uint32_t features;
    uint32_t fileFormatVersion;
    uint64_t dataOffset;
    uint32_t timeStamp;
    uint32_t creatorApplication;
    uint32_t creatorVersion;
    uint32_t creatorHostOS;
    uint64_t originalSize;
    uint64_t currentSize;
    DiskGeometry diskGeometry;
    uint32_t diskType;
    uint32_t checksum;
    uint64_t uniqueId[2];
    uint8_t savedState;
    uint8_t reserved[427];
} __attribute__ ((packed));
typedef struct _HardDiskFooter HardDiskFooter;

static uint32_t calc_checksum(void *data, size_t size)
{
	size_t i;
	uint32_t chksum;

	for (chksum = 0, i = 0; i < size; i++)
		chksum += ((uint8_t*)data)[i];
	return ~chksum;
}

int
autodetect_vhdfile(int fd, vlong *size)
{
	HardDiskFooter hdf;
	struct stat st;
	int32_t chksum;

	if (-1 == fstat(fd, &st)) {
		perror("fstat");
		exit(1);
	}
	if (!S_ISREG(st.st_mode)) return 0; // Not a regular file
	if (-1 == lseek64(fd, -sizeof(hdf), SEEK_END)) {
		perror("lseek64");
		exit(1);
	}
	if (sizeof(hdf) != read(fd, &hdf, sizeof(hdf))) {
		perror("read");
		exit(1);
	}
	if (-1 == lseek64(fd, 0, SEEK_SET)) {
		perror("lseek64");
		exit(1);
	}
	if (strncmp((const char*) &hdf.cookie, "conectix", 8)) return 0; // Cookie not found
	if (be32toh(hdf.fileFormatVersion) != 0x00010000) return -1; // Wrong file format
	if (be32toh(hdf.diskType) != 2) return -2; // Not a fixed disk
	chksum = be32toh(hdf.checksum);
	hdf.checksum = 0;
	if (chksum != calc_checksum(&hdf, sizeof(hdf))) return -3; // Wrong checksum
	if (be64toh(hdf.currentSize) > st.st_size-sizeof(hdf)) return -4; // Wrong size
	*size = (vlong) be64toh(hdf.currentSize);
	return 1;
}
