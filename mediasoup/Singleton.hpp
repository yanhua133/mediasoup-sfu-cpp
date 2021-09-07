

#pragma once

namespace mediasoup
{

template <class T>
class Singleton
{
public:
    static T& GetInstance() {
        // http://www.nuonsoft.com/blog/2017/08/10/implementing-a-thread-safe-singleton-with-c11-using-magic-statics/
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;

private:    
    Singleton(Singleton const&) = delete;
    Singleton& operator=(Singleton const&) = delete;
};

}
