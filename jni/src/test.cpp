#include <stdlib.h>
#include <stdint.h>

#include "object_pointer_pool.hpp"

class SharedBuffer : public ObjectPoolObject<SharedBuffer>
{
public:
	SharedBuffer() {
	}

	~SharedBuffer() {
        if (nullptr != m_buffer) {
            free(m_buffer);
            m_buffer = 0;
            printf("buffer is free!! size= %d\n", m_size);
        }
	}

	uint8_t* get_buffer() const {
		return m_buffer;
	}

	int get_size() const {
		return m_size;
	}

	bool alloc_buffer(int size)
	{
		return allocate(size);
	}

private:
    bool allocate(int size) {
        m_buffer = (uint8_t *)malloc(size);
        if (nullptr != m_buffer) {
            m_size = size;
            return true;
        }
        return false;
    }

private:
    uint8_t *m_buffer = nullptr;
    int m_size = 0;
};

int main(int argc, char* argv[])
{
    ObjPoolPtr<SharedBuffer> obj_pool_ptr = 
        ObjectPointerPool<SharedBuffer>::CreateObjectPoolSharedPtr();
        
    auto frame_ptr = ObjectPointerPool<SharedBuffer>::get_object(obj_pool_ptr);
    if (!frame_ptr->alloc_buffer(1024)) {
        printf("init failed!!\n");
        return -1;
    }

    (void)memset(frame_ptr->get_buffer(), 1, frame_ptr->get_size());
    printf("main done!!\n");
    return 0;
}
