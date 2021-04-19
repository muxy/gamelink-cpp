#pragma once
#ifndef MUXY_TEST_CONSTRAINED_STRING_H
#define MUXY_TEST_CONSTRAINED_STRING_H
#include <string>

class ConstrainedString
{
public:
    inline ConstrainedString(const char* v)
        :data(v)
    {}

    inline ConstrainedString()
    {}
    
    inline ConstrainedString(const ConstrainedString& other)
        :data(other.data)
    {}

    inline ConstrainedString& operator=(const ConstrainedString& other)
    {
        data = other.data;
        return *this;
    }

    inline bool operator==(const ConstrainedString& other) const
    {
        return data == other.data;
    }

    inline uint32_t size() const
    {
        return data.size();
    }

    inline const char * c_str() const
    {
        return data.c_str();
    }
private:
    std::string data;
};

#endif