#pragma once

#include <unistd.h>
#include <sys/syscall.h>

// 获取每个线程的tid
namespace CurrentThread
{
    extern __thread int t_cachedTid; // __thread修饰的变量每一个线程有一份独立实体

    void cacheTid(); // 系统调用开销大，所以需存tid

    inline int tid()
    {
        // 线程id缓存未使用过
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}