// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h


//LAB1
#include <string>
#include "lock_protocol.h"
#include "lock_client.h"
#include <unordered_map>
#include "rpc.h"
#include <unordered_map>
#include <mutex>
#include <condition_variable>



typedef unsigned long long lockid_t;

class lock_server {
private:
    bool is_locked(lockid_t lockid);

    std::unordered_map<lock_protocol::lockid_t, bool> lock_dict;
    std::mutex lock_dict_mutex;
    std::condition_variable cond_unlocked;
    int nacquire;

public:
    lock_server();
    ~lock_server() {};
    lock_protocol::status stat(int clt, lockid_t lid, int &);
    lock_protocol::status release(int clt, lockid_t lid, int &);
    lock_protocol::status acquire(int clt, lockid_t lid, int &);
};

#endif








