#pragma once
#include <fb/Engine/MemoryArena.h>
#include <exception>

#define OFFSET_EMPTYARRAYBEGIN CYPRESS_GW_SELECT(0x141EAAFB8, 0x14294E450, 0x143274600)

namespace fb
{
    template<typename T>
    class Array {
    private:
        T* m_elements;

        void setSize(int n) {
            *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(m_elements) - 0x4) = n;
        }

        T* allocate(int count) {
            int newSize = count * sizeof(T) + 8;
            return static_cast<T*>(MemoryArena::alloc(this, newSize));
        }

        bool allocRange(int count) {
            int prevSize = size();

            if (T* ptr = allocate(prevSize + count))
            {
                if (prevSize > 0)
                    memmove(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + 0x8), m_elements, size() * sizeof(T));

                clear();

                m_elements = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + 0x8);
                setSize(prevSize + count);

                return true;
            }
            return false;
        }

    public:

        Array() : m_elements(reinterpret_cast<T*>(OFFSET_EMPTYARRAYBEGIN)) {}

        int size() const { return *reinterpret_cast<int*>((reinterpret_cast<uintptr_t>(m_elements) - 0x4)) & 0x7FFFFFFF; }

        bool empty() { return size() == 0; }

        T& at(int index) {
            if (index < size())
                return m_elements[index];
            throw std::out_of_range("[fb::Array] Exceeded array range");
        }

        T* begin() { return m_elements; }
        T* end() { return m_elements + size(); }

        T& front() { return *m_elements; }
        T& back() { return m_elements[size() - 1]; }
        T& operator[](int index) { return m_elements[index]; }

        //initializes a new array
        void operator=(const std::initializer_list<T>&& in) {
            if (T* block = allocate(in.size()))
            {
                clear();

                m_elements = reinterpret_cast<T*>(reinterpret_cast<uint64_t>(block) + 0x8);
                memmove(m_elements, in.begin(), in.size() * sizeof(T));
                setSize(in.size());
            }
        }

        void push_back(const T& element) {
            int prevSize = size();

            if (allocRange(1))
                m_elements[prevSize] = element;
        }

        void clear() {
            if (reinterpret_cast<uint64_t>(m_elements) == OFFSET_EMPTYARRAYBEGIN) return;

            MemoryArena::free(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_elements) - 0x8));
            m_elements = reinterpret_cast<T*>(OFFSET_EMPTYARRAYBEGIN);
        }
    };
}