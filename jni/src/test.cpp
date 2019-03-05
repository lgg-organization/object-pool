#include <stdlib.h>
#include <stdint.h>

#include "object_pool.hpp"

#if 0
class TestImage
{
    static constexpr int WIDTH = 1280;
    static constexpr int HEIGHT = 720;

public:
	TestImage() {
        (void)alloc_buffer(WIDTH, HEIGHT);
        return;
	}

	TestImage(int width, int height) {
        (void)alloc_buffer(width, height);
        return;
	}

	~TestImage() {
        if (nullptr != m_buffer) {
            free(m_buffer);
            m_buffer = nullptr;
        }
	}

	uint8_t* get_buffer() const {
		return m_buffer;
	}

	int get_size() const {
		return m_width * m_height * 3 / 2;
	}

    int get_width() {
        return m_width;
    }

    int get_height() {
        return m_height;
    }

	bool alloc_buffer(int width, int height)
	{
        m_width = width;
        m_height = height;
		return allocate(get_size());
	}

private:
    bool allocate(int size) {
        if (nullptr != m_buffer) {
            free(m_buffer);
            m_buffer = nullptr;
        }

        m_buffer = (uint8_t *)malloc(size);
        if (nullptr != m_buffer) {
            return true;
        }
        return false;
    }

private:
    uint8_t *m_buffer = nullptr;

    int m_width = WIDTH;
    int m_height = HEIGHT;    
};

int main(int argc, char* argv[])
{
    auto pool = ObjectPool<TestImage>::CreateObjectPoolPtr();

    bool ret = pool->create(2, 576, 320);
    printf("creat: ret= %d\n", ret);

    auto img = pool->get(1280, 720);
    if (nullptr == img->get_buffer()) {
        printf("pool object get: failed!!\n");
        return -1;
    }

    printf("img: width= %d, height= %d, size= %d\n", 
        img->get_width(), img->get_height(), img->get_size());

    // process image
    (void)memset(img->get_buffer(), 1, img->get_size());

    printf("pool: obj_count= %d\n", pool->obj_count());

    printf("main done!!\n");

    // auto release pool && img!!
    return 0;
}

#else
struct AT
{
    AT(){}
    AT(int a, int b) :m_a(a), m_b(b){}
    ~AT() {
        std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ << std::endl;
    }
    
    void Fun()
    {
        std::cout << m_a << " " << m_b << std::endl;
    }

    int m_a = 0;
    int m_b = 0;
};

struct BT
{
    BT(){}
    ~BT() {
        std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ << std::endl;
    }

    void Fun()
    {
        std::cout << "from object b " << std::endl;
    }
};

static std::shared_ptr<AT> g_at;
static std::shared_ptr<BT> g_bt;

void TestObjectPool()
{
    ObjectPool<AT>::ObjPoolSharedPtr p_pool_at = ObjectPool<AT>::CreateObjectPoolPtr();
    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ \
        << " at_cnt= "<< p_pool_at.use_count() \
        << std::endl;
    
    auto p_pool_bt = ObjectPool<BT>::CreateObjectPoolPtr();
    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ \
        << " bt_cnt= "<< p_pool_bt.use_count() \
        << std::endl;

    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ << std::endl;
    p_pool_at->create(2);
    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ 
        << " at:obj_count= " << p_pool_at->obj_count() \
        << std::endl;

    p_pool_bt->create(2);
    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ 
        << " bt:obj_count= " << p_pool_bt->obj_count() \
        << std::endl;

    p_pool_at->create<int, int>(2, 1, 1);

    {
        auto p = p_pool_at->get();
        p->m_a = 8;
        p->m_b = 8;
        p->Fun();
    }

    #if 1
    for (int i= 0; i<10; i++)
    {
        auto p = p_pool_at->get();
        p->Fun();
    }
    #endif

    auto pb = p_pool_bt->get();
    g_bt = pb;
    pb->Fun();

    auto p = p_pool_at->get();
    g_at = p;
    p->Fun();

    {
        auto p10 = p_pool_at->get();
        auto p11 = p_pool_at->get();
        auto p12 = p_pool_at->get();
        auto p13 = p_pool_at->get();
        auto p14 = p_pool_at->get();

        auto p_pool_at_2 = p_pool_at;
        auto p15 = p_pool_at->get();
        auto p16 = p_pool_at->get();
        auto p17 = p_pool_at->get();
        auto p18 = p_pool_at->get();
        auto p19 = p_pool_at->get();
        auto p20 = p_pool_at->get();
        auto p21 = p_pool_at->get();

        std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ 
            << " at:obj_count= " << p_pool_at->obj_count() \
            << std::endl;
    }

    int a = 5, b = 6;
    auto p2 = p_pool_at->get(a, b);
    p2->Fun();

    auto p3 = p_pool_at->get(3, 4);
    p3->Fun();

    {
        auto p4 = p_pool_at->get(3, 4);
        p4->Fun();
    }

    auto p4 = p_pool_at->get(7, 8);
    p4->Fun();

    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ 
        << " at:obj_count= " << p_pool_at->obj_count() \
        << std::endl;

    std::cout << "function= " << __FUNCTION__ << " &line= " << __LINE__ 
        << " bt:obj_count= " << p_pool_bt->obj_count() \
        << std::endl;

    return;
}

int main(int argc, char* argv[])
{
    TestObjectPool();
    return 0;
}
#endif

