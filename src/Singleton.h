/*
 * Created by Jemmy on 2018/7/4.
 *
 */
#include <pthread.h>

template<typename T>
class Singleton {
public:
    static T &instance() {
        pthread_once(&ponce_, &Singleton::init);
        return *value_;
    }
private:
    Singleton();
    ~Singleton();

    static void init() {
        value_ = new T();
    }

    static pthread_once_t ponce_;
    static T *value_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T *Singleton<T>::value_ = NULL;
