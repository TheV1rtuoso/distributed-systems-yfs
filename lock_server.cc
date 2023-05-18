// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}


//LAB1
lock_protocol::status
lock_server::acquire(int clt, lockid_t lid, int&) {
    lock_protocol::status ret = lock_protocol::OK;
    std::unique_lock<std::mutex> lock(lock_dict_mutex);

    while (is_locked(lid)) {
        cond_unlocked.wait(lock);
    }

    auto it = lock_dict.find(lid);
    if (it == lock_dict.end()) {
        lock_dict.emplace(lid, true);
    } else {
        assert(!it->second);
        it->second = true;
    }

    return ret;
}

//LAB1
lock_protocol::status
lock_server::release(int clt, lockid_t lid, int&) {
    lock_protocol::status ret = lock_protocol::OK;
    std::unique_lock<std::mutex> lock(lock_dict_mutex);

    auto it = lock_dict.find(lid);
    if (it != lock_dict.end()) {
        it->second = false;
    } else {
        std::cout << "[Warning] Tried to release non-existing lock" << std::endl;
    }

    cond_unlocked.notify_all();
    return ret;
}

//LAB1
bool lock_server::is_locked(lockid_t lockid) {
    auto it = lock_dict.find(lockid);
    if (it == lock_dict.end()) {
        return false;
    } else {
        return it->second;
    }
}
