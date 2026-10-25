#pragma once
#include "types.hpp"


template<class ValueType>
class [[nodiscard]] Own
{
    ValueType* m_data;
    int (*m_cleaner)(ValueType*);
    bool m_destructor_dont_release = false;

public:
    using ValueT = ValueType;

    Own(const Own&) = delete;
    Own& operator= (const Own&) = delete;
    ~Own() { if (!m_destructor_dont_release) this->release(m_cleaner); }

    Own (ValueT* ptr, int (*invokable)(ValueT*))
    {
        m_data = ptr;
        m_cleaner = invokable;
    }

    template<class Invokable>
    void release(Invokable invokable) {
        m_destructor_dont_release = true;
        invokable(m_data);
    }

    const ValueT* data() const {
        return m_data;
    }

    // overload
    ValueT* data() {
        return m_data;
    }
};
