#pragma once

#include <iostream>
#include <memory>

#include <string>
#include <list>
#include <mutex>

#define OBJ_POOL_DEBUG 0 // 1: enable debug. 0: disable

const int MaxObjectNum = 10;

template<typename T>
class ObjectPool: public std::enable_shared_from_this<ObjectPool<T>>
{
public:
    using ObjPoolSharedPtr = std::shared_ptr<ObjectPool<T>>;

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
    std::shared_ptr<T> get(Args... args) {

        ObjPoolSharedPtr pool = this->shared_from_this();
        std::lock_guard<std::mutex> lock(m_pool_mutex);
        if (m_pool.empty()) {
            #if OBJ_POOL_DEBUG
            std::cout << "function= " << __FUNCTION__ \
                << " &line= " << __LINE__ << " new object!" \
                << std::endl;
            #endif

            m_total_count++;
            return createSharedPtr<Args...>(pool, args...);
        }else {
            T* p = m_pool.back();
            m_pool.pop_back();
            return makeSharedPtr(pool, p);
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
    static std::shared_ptr<T> makeSharedPtr(ObjPoolSharedPtr pool, T* ptr)
    {
        #if OBJ_POOL_DEBUG
        std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ \
            << " ptr= " << ptr \
            << std::endl;
        #endif
        return std::shared_ptr<T>(ptr, [pool](T* p){
            pool->free_object(p);
        });
    }

    template<typename... Args>
    static std::shared_ptr<T> createSharedPtr(ObjPoolSharedPtr pool, Args... args)
    {
        T* ptr = new T(args...);
        return makeSharedPtr(pool, ptr);
    }

    void free_object(T* ptr) {

        std::lock_guard<std::mutex> lock(m_pool_mutex);
        #if OBJ_POOL_DEBUG
        std::cout << "function= " << __FUNCTION__ \
            << " &ptr= " << ptr \
            << std::endl;
        #endif

        if (m_pool.size() < MaxObjectNum) {
            m_pool.emplace_back(ptr);
        }else {
            #if OBJ_POOL_DEBUG
            std::cout << "function= " << __FUNCTION__ \
                << " &ptr= " << ptr \
                << " full: size= " << m_pool.size() \
                << std::endl;
            #endif

            m_total_count--;
            delete ptr;
        }
        return;
    }

private:
    std::mutex m_pool_mutex;
    std::list<T*> m_pool;
    int m_total_count = 0;
};
