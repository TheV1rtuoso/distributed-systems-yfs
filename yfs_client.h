#ifndef yfs_client_h
#define yfs_client_h
#include <string>
#include "extent_client.h"
#include <list>
#include <random>
#include <vector>

class yfs_client {
    std::random_device rd;
    std::default_random_engine gen;
    std::uniform_int_distribution<unsigned long long> distrib;

public:
    extent_client *ec;
    typedef unsigned long long inum;
    enum xxstatus { OK, RPCERR, NOENT, IOERR, FBIG };
    typedef int status;

    struct fileinfo {
        unsigned long long size;
        unsigned long atime;
        unsigned long mtime;
        unsigned long ctime;
    };
    struct dirinfo {
        unsigned long atime;
        unsigned long mtime;
        unsigned long ctime;
    };
    struct dirent {
        std::string name;
        unsigned long long inum;
        dirent();
        dirent(std::string name, unsigned long long inum);
    };
    typedef std::list<dirent> dirent_list;
    friend std::istream &operator>>(std::istream &is,
                                    yfs_client::dirent_list &dirent_list);
    friend std::ostream &operator<<(std::ostream &os,
                                    yfs_client::dirent_list &dirent_list);

private:
    static std::string filename(inum);
    static inum n2i(std::string);

public:
    yfs_client(std::string, std::string);

    bool isfile(inum);
    bool isdir(inum);
    inum ilookup(inum di, std::string name);

    inum generate_inum(int is_dir);

    int getfile(inum, fileinfo &);
    int getdir(inum, dirinfo &);

    int create(inum parent, const char *name, int is_dir, inum &inum);
    int lookup(inum parent, const char *name, inum &inum);
    int readdir(inum parent, dirent_list &dirents);
};

#endif
