#pragma once

template <class T>
class Singleton
{
public:
	static T& Get();
	static inline bool IsValid() { return (s_Instance != nullptr); }

protected:
	Singleton();
	~Singleton();

private:
	static T* s_Instance;
};

template < class T >
T * Singleton< T >::s_Instance = nullptr;

template < class T >
Singleton< T >::Singleton()
{
	s_Instance = static_cast<T*>(this);
}

template < class T >
Singleton< T >::~Singleton()
{
	s_Instance = nullptr;
}

template < class T >
T & Singleton< T >::Get()
{
	return *s_Instance;
}
