#pragma once
#ifndef MUXY_TEST_CONSTRAINED_STRING_H
#define MUXY_TEST_CONSTRAINED_STRING_H
#include <mutex>
#include <string>

class ConstrainedString
{
public:
	inline ConstrainedString() {}

	inline ConstrainedString(const ConstrainedString& other)
		: _data(other._data)
	{
	}

	inline ConstrainedString& operator=(const ConstrainedString& other)
	{
		_data = other._data;
		return *this;
	}

	inline ConstrainedString(ConstrainedString&& other)
		: _data(other._data)
	{
	}

	ConstrainedString& operator=(ConstrainedString&& other)
	{
		_data = other._data;
		return *this;
	}

	inline ConstrainedString(const char* v)
		: _data(v)
	{
	}

	inline ConstrainedString(const char* v, size_t len)
		: _data(std::string(v, len))
	{
	}

	inline bool operator==(const ConstrainedString& other) const
	{
		return _data == other._data;
	}

	inline uint32_t size() const
	{
		return _data.size();
	}

	inline const char* c_str() const
	{
		return _data.c_str();
	}

private:
	std::string _data;
};

class ConstrainedLock
{
public:
	inline ConstrainedLock() {}

	// Uncopyable, nonmovable.
	ConstrainedLock(const ConstrainedLock&) = delete;
	ConstrainedLock& operator=(const ConstrainedLock&&) = delete;

	ConstrainedLock(ConstrainedLock&&) = delete;
	ConstrainedLock& operator=(ConstrainedLock&&) = delete;

	inline void lock()
	{
		_lock.lock();
	}

	inline void unlock()
	{
		_lock.unlock();
	}

	inline bool try_lock()
	{
		return _lock.try_lock();
	}

private:
	std::mutex _lock;
};

#endif
