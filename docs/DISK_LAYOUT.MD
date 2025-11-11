# Default Disk Layout

The default disk image is `disk.hdd`. The block size is `512` bytes. It has the following GPT partition layout:

| Partition | Start | End | Size | Type | Filesystem |
|-----------|-------|-----|------|------|------------|
| 1 | 2048 | 4096 | 2 MiB | Limine BIOS | none |
| 2 | 8192 | 131038 | 60 MiB | ESP | FAT32 |
| 3 | 133086 | 1048576 | 448 MiB | Root | FAT32 | 

Note: start and end denote blocks