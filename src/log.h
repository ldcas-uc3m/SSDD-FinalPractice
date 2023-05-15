#ifndef _LOG_H_
#define _LOG_H_

// Activa o desactiva el sistema de logs
// #define LOG_ENABLE 1

// Se utiliza para inicializar el sistema de logging
void LogInit();
void lock_log();
void unlock_log();

#ifdef LOG_ENABLE
    #define Log(fmt, ...) \
        lock_log(); \
        printf("[%s:%d] " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
        unlock_log()
#else
    #define Log(fmt, ...)
#endif

#endif