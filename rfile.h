#ifndef _RFILE_HDR_
#define _RFILE_HDR_

int get_file_hash(const char *filename, char *hash);

int compress_file(const char *filename, int id, int revision);

#endif /* _RFILE_HDR_ */
