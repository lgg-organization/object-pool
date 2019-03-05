#include <stdlib.h>
#include <stdint.h>

#include "object_pool.hpp"

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
