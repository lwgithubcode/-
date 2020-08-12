#ifndef _DATA_LIST_H_
#define _DATA_LIST_H_

#include"commont.h"

template<class T>
class list
{
	public:
		list(int capacity = 20) : _capacity(capacity), _size(0)
		{
			_data = new T[_capacity + 1];
			memset(_data, 0, sizeof(T) * _capacity);
		}
		list(const list<T>& ls) : _capacity(ls._capacity), _size(ls._size)
		{
			_data = new T[_capacity + 1];
			memcpy(_data, ls._data, sizeof(T) * (_capacity + 1));
		}
		list<T>& operator=(const list<T>& ls)
		{
			if (this != &ls)
			{
				_capacity = ls._capacity;
				_size = ls._size;
				T* temp = new T[_capacity + 1];
				delete[] _data;
				_data = temp;
				memcpy(_data, ls._data, sizeof(T) * (_capacity + 1));
			}
			return *this;
		}
		~list()
		{
			free(_data);
			_data = NULL;
			_capacity = 0;
			_size = 0;
		}
	public:
		T& operator[](int index)
		{
			return _data[index];
		}
	public:
		size_t& size()
		{
			return _size;
		}
		bool fool()
		{
			return _size == _capacity;
		}
		bool empty()
		{
			return !_size;
		}
	public:
		void push_back(const T &x)
		{
			if (!fool() || _Inc())
			{
				_size++;
				_data[_size] = x;
			}
		}
		void push_front(const T &x)
		{
			if (!fool() || _Inc())
			{
				for (int i = _size; i >= 1; i--)
				{
					_data[i + 1] = _data[i];
				}
				_data[1] = x;
				_size++;
			}
		}
		void pop_back()
		{
			if (!empty())
			{
				_data[_size] = T();
				_size--;
			}
		}
		void pop_front()
		{
			if (!empty())
			{
				for (size_t i = 1; i < _size; i++)
				{
					_data[i] = _data[i + 1];
				}
				_data[_size] = T();
				_size--;
			}
		}
		const T& back()
		{
			return _data[_size];
		}
		const T& front()
		{
			return _data[1];
		}
	public:
		void reverse()
		{
			int start = 1, end = _size();
			while (start < end)
			{
				T temp = _data[start];
				_data[start] = _data[end];
				_data[end] = temp;
				start++;
				end--;
			}
		}
		void clear()
		{
			while (_size > 0)
			{
				_data[_size] = T();
				_size--;
			}
		}
		void show()
		{
			for (size_t i = 1; i <= size(); i++)
				cout << _data[i] << " ";
			cout << endl;
		}
	protected:
		bool _Inc()
		{
			T* new_base = new T[_capacity * 2 + 1];
			memset(new_base, 0, sizeof(T) * (_capacity * 2 + 1));
			memcpy(new_base, _data, sizeof(T)* (_capacity + 1));
			_capacity *= 2;
			delete[] _data;
			_data = new_base;
			return true;
		}

	private:
		T* _data;
		size_t _capacity;
		size_t _size;
};





#endif //_DATA_LIST_H_
