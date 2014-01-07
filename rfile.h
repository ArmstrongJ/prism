#ifndef _RFILE_HDR_
#define _RFILE_HDR_

int get_file_hash(const char *filename, char *hash);

int compress_file(const char *filename, int id, int revision);

int decompress_file(const char *destination, int id, int revision);

int diff_file(const char *working, int id, int revision);

#endif /* _RFILE_HDR_ */
