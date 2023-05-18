#include "yfs_client.h"
#include "extent_client.h"
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

yfs_client::dirent::dirent() {}

yfs_client::dirent::dirent(std::string name, unsigned long long inum)
        : name(name), inum(inum) {}

std::istream &operator>>(std::istream &is,
                         yfs_client::dirent_list &dirent_list) {
    size_t cnt = 0;
    assert(is.rdbuf()->in_avail() != 0);
    is >> cnt;
    std::string name;
    yfs_client::inum inum;
    for (size_t i = 0; i < cnt; i++) {
        is >> name >> inum;
        dirent_list.emplace_back(name, inum);
    }
    return is;
}

std::ostream &operator<<(std::ostream &os,
                         yfs_client::dirent_list &dirent_list) {
    os << dirent_list.size();
    for (auto &dirent : dirent_list)
        os << " " << dirent.name << " " << dirent.inum;
    return os;
}


yfs_client::yfs_client(std::string extent_dst, std::string lock_dst) {
    ec = new extent_client(extent_dst);
}

yfs_client::inum yfs_client::n2i(std::string n) {
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string yfs_client::filename(inum inum) {
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool yfs_client::isfile(inum inum) {
    return inum & 0x80000000;
}

bool yfs_client::isdir(inum inum) {
    return !isfile(inum);
}

yfs_client::inum yfs_client::generate_inum(int is_dir) {
    inum inum = distrib(gen);
    if (is_dir)
        inum &= 0x7FFFFFFF;
    else
        inum |= 0x80000000;
    return inum;
}
int yfs_client::getfile(inum inum, fileinfo &fin) {
    int r = OK;
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;

    release:
    return r;
}

int yfs_client::getdir(inum inum, dirinfo &din) {
    int r = OK;
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

    release:
    return r;
}

int yfs_client::create(inum parent, const char *name, int is_dir, inum &inum) {
    if (!isdir(parent)) return IOERR;  // Check if parent is directory
    std::string parent_content;
    if (ec->get(parent, parent_content) != extent_protocol::OK) return IOERR;  // Get parent directory content
    std::istringstream parent_stream(parent_content);
    std::string file_name, file_inum;
    while (parent_stream >> file_name >> file_inum) {
        if (file_name == name) return IOERR;  // Check if name already exists
    }

    // Create new file or directory
    inum = generate_inum(is_dir);
    std::string content;
    if (is_dir) content = "0";  // Directories start with an entry count of 0
    if (ec->put(inum, content) != extent_protocol::OK) return IOERR;

    // Add new file or directory to parent directory
    parent_content += " " + std::string(name) + " " + filename(inum);
    if (ec->put(parent, parent_content) != extent_protocol::OK) return IOERR;

    return OK;
}

int yfs_client::lookup(inum parent, const char *name, inum &inum) {
    if (!isdir(parent)) return IOERR;  // Check if parent is directory
    std::string parent_content;
    if (ec->get(parent, parent_content) != extent_protocol::OK) return IOERR;  // Get parent directory content
    std::istringstream parent_stream(parent_content);
    std::string file_name, file_inum;
    while (parent_stream >> file_name >> file_inum) {
        if (file_name == name) {
            inum = n2i(file_inum);
            return OK;
        }
    }
    return IOERR;
}

int yfs_client::readdir(inum dir, dirent_list &list) {
    if (!isdir(dir)) return IOERR;  // Check if dir is directory
    std::string dir_content;
    if (ec->get(dir, dir_content) != extent_protocol::OK) return IOERR;  // Get directory content
    std::istringstream dir_stream(dir_content);
    std::string file_name, file_inum;
    while (dir_stream >> file_name >> file_inum) {
        dirent entry;
        entry.name = file_name;
        entry.inum = n2i(file_inum);
        list.push_back(entry);
    }
    return OK;
}
