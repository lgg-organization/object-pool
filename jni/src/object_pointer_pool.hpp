#pragma once
#include <string>
#include <memory>
#include <list>
#include <mutex>

#define LOG_E printf

template<typename T> class ObjectPointerPool;

template<typename T>
using ObjPoolPtr = std::shared_ptr<ObjectPointerPool<T>>;


template<typename T>
class ObjectPoolObject
{
    friend ObjectPointerPool<T>;

private:
    // for debug
    int pool_use_count() const
    {
        return m_pool_ptr.use_count();
    }

protected:
    ObjectPoolObject() {
        release_pool_ptr();
    }

    virtual ~ObjectPoolObject() {
        release_pool_ptr();
    }
    
private:
    void set_pool_ptr(ObjPoolPtr<T> &pool_ptr)
    {
        m_pool_ptr = pool_ptr;
    }

    void release_pool_ptr()
    {
        m_pool_ptr.reset();
    }
    
private:
    ObjPoolPtr<T> m_pool_ptr;
};


template<typename T>
class ObjectPointerPool
{
    friend ObjectPoolObject<T>;
    
public:
    static ObjPoolPtr<T> CreateObjectPoolSharedPtr()
    {
        return std::make_shared<ObjectPointerPool<T>>();
    }

    bool init_pool(int size)
    {
        for (int i=0; i<size; i++)
        {
            auto ptr = new T();
            m_pool.emplace_back(ptr);
        }
        return true;
    }

    static std::shared_ptr<T> get_object(ObjPoolPtr<T> &pool_ptr)
    {
        std::lock_guard<std::mutex> lock(pool_ptr->m_pool_mutex);

        if (pool_ptr->m_pool.empty()) {
            LOG_E("warning: new object !!\n");
            return std::shared_ptr<T>(create_object(pool_ptr), [](T* p){
                release_ptr(p);
            });
        }

        T* p = pool_ptr->m_pool.front();
        p->set_pool_ptr(pool_ptr);
        std::shared_ptr<T> ptr(p, [](T* p){
            release_ptr(p);
        });

        pool_ptr->m_pool.pop_front();
        return ptr;
    }

public:
    ObjectPointerPool()
    {
        static_assert(std::is_base_of<ObjectPoolObject<T>, T>::value, "type error!!");
        return;
    }

    ~ObjectPointerPool() {
        LOG_E("~ObjectPointerPool!!\n");
        for (auto i: m_pool)
        {
            free_object(i);
        }
    }

private:
    static T* create_object(ObjPoolPtr<T> &pool_ptr)
    {
        T* t_ptr = new T();
        t_ptr->set_pool_ptr(pool_ptr);
        return t_ptr;
    }

    static void free_object(T* p)
    {
        delete p;
    }

    static void release_ptr(T* p)
    {
        if (nullptr == p) {
            LOG_E("impossible: null pointer !!\n");
            return;
        }

        LOG_E("release_ptr: pool_use_count()= %d\n", p->pool_use_count());

        // ·ÀÖ¹ÄÚ´æÊÍ·Å³åÍ»
        if (p->pool_use_count() >= 2)
        {
            std::lock_guard<std::mutex> lock(p->m_pool_ptr->m_pool_mutex);
            p->m_pool_ptr->m_pool.emplace_front(p);
            p->release_pool_ptr();
        }
        else
        {
            p->release_pool_ptr();
            free_object(p);
        }
        return;
    }

private:
    std::list<T*> m_pool;
    std::mutex m_pool_mutex;
};

