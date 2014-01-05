#ifndef _DB_HDRS_
#define _DB_HDRS_

struct db_file {
    int id;
    int revision;
    char hash[33];
};

int initdb();

int get_revision();

int get_revision();

int get_file_count();

int increment_revision();

int add_file(const char *filename);

int get_db_fileinfo(const char *filename, struct db_file *info);

int disp_queue();

int commit_queue(int revision);

int revert(const char *filename, int revision);

#endif /* _DB_HDRS_ */
