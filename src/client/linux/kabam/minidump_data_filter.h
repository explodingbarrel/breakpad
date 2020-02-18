
#ifndef KABAM_CLIENT_LINUX_MINIDUMP_DATA_FILTER_H__
#define KABAM_CLIENT_LINUX_MINIDUMP_DATA_FILTER_H__

// The filter flags are stored in a uint32_t
// So take care with the amount of flags you add!
enum MinidumpDataFilter
{
	MDF_NONE = 0,
	MDF_LINUX_DSO_DEBUG = 1 << 0,
	MDF_LINUX_MAPS = 1 << 1,
	MDF_LINUX_AUXV = 1 << 2,
	MDF_LINUX_ENVIRON = 1 << 3,
	MDF_LINUX_CMD_LINE = 1 << 4,
	MDF_LINUX_LSB_RELEASE = 1 << 5,
	MDF_LINUX_PROC_STATUS = 1 << 6,
	MDF_LINUX_CPU_INFO = 1 << 7
};

#endif
