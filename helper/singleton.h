#ifndef SINGLETON_H
#define SINGLETON_H

#include <QMutex>

// 模板类Singleton，用于创建单例对象
template <typename T>
class Singleton
{
public:
    // 获取单例对象的静态方法
    static T* getInstance();

private:
    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY_MOVE(Singleton)
};

// 实现单例模式的静态方法
template <typename T>
T* Singleton<T>::getInstance()
{
    static QMutex mutex; // 静态互斥锁，用于线程同步
    QMutexLocker locker(&mutex); // 自动管理互斥锁
    static T* instance = nullptr; // 单例对象指针
    if (instance == nullptr)
    {
        instance = new T(); // 如果实例不存在，则创建一个新的实例
    }
    return instance; // 返回单例对象指针
}

// 宏定义，用于简化单例类的创建
#define Q_SINGLETON_CREATE(Class)               \
private:                                        \
    friend class Singleton<Class>;              \
                                                \
    public:                                         \
    static Class* getInstance()                 \
{                                           \
        return Singleton<Class>::getInstance(); \
}

// 宏定义，用于隐藏构造函数，防止外部创建对象
#define Q_HIDE_CONSTRUCTOR(Class)       \
private:                                \
    Class() = default;                  \
    Class(const Class& other) = delete; \
    Q_DISABLE_COPY_MOVE(Class);

#endif // SINGLETON_H
