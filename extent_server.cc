// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>

extent_server::extent_server() {
    pthread_mutex_init(&m, NULL);
    int r;
    put(1, "", r);
}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &) {
    ScopedLock ml(&m);
    extent_t ext;
    unsigned int t = time(nullptr);

    ext.attr = {buf.size(), t, t, t};
    ext.ext = std::move(buf);

    exts[id] = std::move(ext);

    return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf) {
    ScopedLock ml(&m);
    auto it = exts.find(id);
    if (it == exts.end()) {
        return extent_protocol::IOERR;
    }

    it->second.attr.atime = time(nullptr);
    buf = it->second.ext;

    return extent_protocol::OK;
}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a) {
    ScopedLock ml(&m);
    auto it = exts.find(id);
    if (it == exts.end()) {
        return extent_protocol::IOERR;
    }

    a = it->second.attr;

    return extent_protocol::OK;
}

int extent_server::remove(extent_protocol::extentid_t id, int &) {
    ScopedLock ml(&m);
    auto it = exts.find(id);
    if (it == exts.end()) {
        return extent_protocol::IOERR;
    }

    exts.erase(it);

    return extent_protocol::OK;
}
