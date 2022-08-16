#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

//线程同步机制封装类，将互斥锁，条件变量，互斥锁进行封装


//互斥锁类成员：构造函数（初始化互斥锁）  析构函数（回收互斥锁）  lock（对互斥锁进行上锁）  unlock（对互斥锁进行解锁）
class locker {
public:
    locker() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0) {
            throw std::exception();
        }
    }
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    // 互斥锁进行上锁
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    // 互斥锁解锁
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t *get()
    {
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;

};

//条件变量类
class cond {
public:
    cond() {
        if(pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();  //如果初始化失败，则抛出异常 
        } 
    }
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }

    //线程等待信号触发，如果没有信号触发，无限期等待下去。
    bool wait(pthread_mutex_t * mutex) {
        return pthread_cond_wait(&m_cond, mutex) == 0; //等待条件变量m_cond得唤醒，并释放互斥锁mutex
    }

    //线程等待一定的时间，如果超时或有信号触发，线程唤醒。
    bool timedwait(pthread_mutex_t * mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
    }

    //唤醒一个线程
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }

    //唤醒所有线程
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond; // 条件变量
};

// 信号量类
class sem {
public:
    sem() {
        if(sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }

    sem(int num) {
        if(sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }

    ~sem() {
        sem_destroy(&m_sem);
    }

    //等待信号量
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }

    //增加信号量
    bool post() {
        return sem_post( &m_sem ) == 0;
    }
private:
    sem_t m_sem;
};

#endif