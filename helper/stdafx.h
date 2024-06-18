#ifndef STDAFX_H
#define STDAFX_H

#include <qglobal.h>

// 导出/导入宏定义
#ifdef ELAWIDGETTOOLS_LIBRARY
#define ELA_EXPORT Q_DECL_EXPORT
#else
#define ELA_EXPORT Q_DECL_IMPORT
#endif

// 简单属性宏
// 自动生成 setValue、value 方法和 valueChanged 信号
#define Q_PROPERTY_CREATE(TYPE, M)                          \
Q_PROPERTY(TYPE p##M MEMBER _p##M NOTIFY p##M##Changed) \
    public:                                                     \
    Q_SIGNAL void p##M##Changed();                          \
    void set##M(TYPE M)                                     \
{                                                       \
        if (_p##M != M) {                                   \
            _p##M = M;                                      \
            Q_EMIT p##M##Changed();                         \
    }                                                   \
}                                                       \
    TYPE get##M() const                                     \
{                                                       \
        return _p##M;                                       \
}                                                       \
    private:                                                    \
    TYPE _p##M;

// Q_D Q_Q模式普通属性宏
// 用于在类的头文件中声明属性。你需要在类的实现文件中使用 Q_PROPERTY_CREATE_Q_CPP 宏来实现 setter 和 getter
#define Q_PROPERTY_CREATE_Q_H(TYPE, M)                                  \
Q_PROPERTY(TYPE p##M READ get##M WRITE set##M NOTIFY p##M##Changed) \
    public:                                                                 \
    Q_SIGNAL void p##M##Changed();                                      \
    void set##M(TYPE M);                                                \
    TYPE get##M() const;

// Q_D Q_Q模式指针变量宏
// 用于在类的头文件中声明指针类型的私有变量。同样，你需要在类的实现文件中使用 Q_PRIVATE_CREATE_Q_CPP 宏来实现 setter 和 getter
#define Q_PRIVATE_CREATE_Q_H(TYPE, M) \
public:                               \
    Q_SIGNAL void p##M##Changed();    \
    void set##M(TYPE M);              \
    TYPE get##M() const;

// 属性实现宏（适用于 Q_D Q_Q 模式）
// 用于在类的实现文件中实现属性的 setter 和 getter。你需要在类的头文件中使用 Q_PROPERTY_CREATE_Q_H 宏来声明属性
#define Q_PROPERTY_CREATE_Q_CPP(CLASS, TYPE, M) \
void CLASS::set##M(TYPE M)                  \
{                                           \
        Q_D(CLASS);                             \
        if (d->_p##M != M) {                    \
            d->_p##M = M;                       \
            Q_EMIT p##M##Changed();             \
    }                                       \
}                                           \
    TYPE CLASS::get##M() const                  \
{                                           \
        Q_D(const CLASS);                       \
        return d->_p##M;                        \
}

// 指针变量实现宏（适用于 Q_D Q_Q 模式）
// 在类的实现文件中实现指针类型私有变量的 setter 和 getter。你需要在类的头文件中使用 Q_PRIVATE_CREATE_Q_H 宏来声明变量
#define Q_PRIVATE_CREATE_Q_CPP(CLASS, TYPE, M) \
void CLASS::set##M(TYPE M)                 \
{                                          \
        Q_D(CLASS);                            \
        if (d->_p##M != M) {                   \
            d->_p##M = M;                      \
            Q_EMIT p##M##Changed();            \
    }                                      \
}                                          \
    TYPE CLASS::get##M() const                 \
{                                          \
        Q_D(const CLASS);                      \
        return d->_p##M;                       \
}

// 私有成员变量声明宏
#define Q_PROPERTY_CREATE_D(TYPE, M) \
private:                             \
    TYPE _p##M;

// 指针类型私有成员变量声明宏
#define Q_PRIVATE_CREATE_D(TYPE, M) \
private:                            \
    TYPE _p##M;

// 普通成员变量创建宏
// 创建一个普通的成员变量，并自动生成 setter 和 getter
#define Q_PRIVATE_CREATE(TYPE, M) \
public:                           \
    void set##M(TYPE M)           \
{                             \
        _p##M = M;                \
}                             \
    TYPE get##M() const           \
{                             \
        return _p##M;             \
}                             \
    private:                          \
    TYPE _p##M;

// Q_Q 创建宏
// 创建一个 Q_Q 模式的类，即使用 QScopedPointer 来管理私有数据
#define Q_Q_CREATE(CLASS)                               \
protected:                                              \
    CLASS(CLASS##Private& dd, CLASS* parent = nullptr); \
    QScopedPointer<CLASS##Private> d_ptr;               \
    private:                                                \
    Q_DISABLE_COPY(CLASS)                               \
    Q_DECLARE_PRIVATE(CLASS);

// Q_D 创建宏
// 创建一个 Q_D 模式的类，即使用 Q_DECLARE_PRIVATE 和 Q_DECLARE_PUBLIC 来管理私有数据
#define Q_D_CREATE(CLASS) \
protected:                \
    CLASS* q_ptr;         \
    private:                  \
    Q_DECLARE_PUBLIC(CLASS);

#endif // STDAFX_H
