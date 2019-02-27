#pragma once

#include <iostream>
#include <memory>

#include <string>
#include <list>
#include <mutex>

#define OBJ_POOL_DEBUG 0 // 1: enable debug. 0: disable

const int MaxObjectNum = 10;

template<typename T>
class ObjectPool
{
public:
    using ObjPoolSharedPtr = std::shared_ptr<ObjectPool<T>>;
    using ObjPoolWeakPtr = std::weak_ptr<ObjectPool<T>>;

public:
    static ObjPoolSharedPtr CreateObjectPoolPtr()
    {
        ObjectPool<T> *pt = new ObjectPool<T>();
        return ObjPoolSharedPtr(pt);
    }

    //默认创建多少个对象
    template<typename... Args>
    bool create(int num, Args... args) {
        if (num <= 0 || num > MaxObjectNum) {
            std::cout << "function= " << __FUNCTION__ \
                << " &line= " << __LINE__<< " num= " << num \
                << std::endl;
            return false;
        }

        std::lock_guard<std::mutex> lock(m_pool_mutex);
        auto size = m_pool.size();
        if (size > 0) {
            std::cout << "function= " << __FUNCTION__ \
                << " &line= " << __LINE__ \
                << " num= " << num << " size= " << size \
                << std::endl;
        }

        num -= size;
        for (int i=0; i<num; i++) {
            m_pool.emplace_back(new T(args...));
        }

        #if OBJ_POOL_DEBUG
        std::cout << "function= " << __FUNCTION__ \
            << " &line= " << __LINE__ << std::endl;
        #endif

        m_total_count += num;
        return true;
    }

    template<typename... Args>
    static std::shared_ptr<T> get(ObjPoolSharedPtr pool, Args... args) {
        ObjPoolWeakPtr weak_pool = pool;

        std::lock_guard<std::mutex> lock(pool->m_pool_mutex);
        if (pool->m_pool.empty()) {
            #if OBJ_POOL_DEBUG
            std::cout << "function= " << __FUNCTION__ \
                << " &line= " << __LINE__ << " new object!" \
                << std::endl;
            #endif

            pool->m_total_count++;
            return createSharedPtr<Args...>(weak_pool, args...);
        }else {
            T* p = pool->m_pool.back();
            pool->m_pool.pop_back();
            return createSharedPtr(weak_pool, p);
        }
    }

    int obj_count() {
        return m_total_count;
    }

private:
    ObjectPool() {}
public:
    ~ObjectPool() {
        #if OBJ_POOL_DEBUG
        std::cout << "~ObjectPool: function= " << __FUNCTION__ \
            << " &line= " << __LINE__ << " size= " << m_pool.size() \
            << std::endl;
        #endif

        std::lock_guard<std::mutex> lock(m_pool_mutex);
        for(auto it : m_pool) {
            #if OBJ_POOL_DEBUG
            std::cout << "~ObjectPool: function= " << __FUNCTION__ \
                << " &line= " << __LINE__ << " it= " << it \
                << std::endl;
            #endif
            delete it;
        }
    }

private:
    static std::shared_ptr<T> createSharedPtr(ObjPoolWeakPtr pool, T* ptr)
    {
        #if OBJ_POOL_DEBUG
        std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ \
            << " ptr= " << ptr \
            << std::endl;
        #endif
        return std::shared_ptr<T>(ptr, [pool](T* p){
            free_object(pool, p);
        });
    }

    template<typename... Args>
    static std::shared_ptr<T> createSharedPtr(ObjPoolWeakPtr pool, Args... args)
    {
        T* ptr = new T(args...);
        return createSharedPtr(pool, ptr);
    }

    static void free_object(ObjPoolWeakPtr pool, T* ptr) {
        if (pool.expired()) {
            #if OBJ_POOL_DEBUG
            std::cout << "function= " << __FUNCTION__ \
                << " &line= " << __LINE__ << " expired!" \
                << " ptr= " << ptr \
                << std::endl;
            #endif

            delete ptr;
            return;
        }

        ObjPoolSharedPtr shared_pool = pool.lock();
        std::lock_guard<std::mutex> lock(shared_pool->m_pool_mutex);
        #if OBJ_POOL_DEBUG
        std::cout << "function= " << __FUNCTION__ \
            << " &ptr= " << ptr \
            << " cnt= " << shared_pool.use_count() \
            << std::endl;
        #endif

        if (shared_pool->m_pool.size() < MaxObjectNum) {
            shared_pool->m_pool.emplace_back(ptr);
        }else {
            #if OBJ_POOL_DEBUG
            std::cout << "function= " << __FUNCTION__ \
                << " &ptr= " << ptr \
                << " full: size= " << shared_pool->m_pool.size() \
                << std::endl;
            #endif

            shared_pool->m_total_count--;
            delete ptr;
        }
        return;
    }

private:
    std::mutex m_pool_mutex;
    std::list<T*> m_pool;
    int m_total_count = 0;
};

