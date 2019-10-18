/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Pe.hpp
 * Author: parfait
 *
 * Created on 3 octobre 2018, 14:34
 */

#pragma once
#include "types.hpp"
#include "compiler.hpp"
#include "queue.hpp"
#include "string.hpp"
#include <cassert>
#include <cstdio>

class Log {
    friend class Queue<Log>;
    friend class Logentrystore;
    friend class Logstore;
    static Queue<Log> logs;
    
    static size_t log_number;
    static size_t log_entry_number;
    class Log_entry {
        friend class Queue<Log_entry>;
        friend class Log;
        friend class Logentrystore;
        
        String *log_entry = nullptr;
        Log_entry *prev = nullptr, *next = nullptr;
//        size_t numero = 0;
        
//        ALWAYS_INLINE
//        static inline void *operator new (size_t) {return cache.alloc();}
//        
//        ALWAYS_INLINE
//        static inline void operator delete (void *ptr) {
//            cache.free (ptr);
//        }

        ~Log_entry() {
            delete log_entry;
            assert(log_entry_number);
            log_entry_number--;
        }
        Log_entry(){}
        
        Log_entry(const Log_entry& orig);
        
        Log_entry &operator=(Log_entry const &);

//        Log_entry(char* l, Log* log) {
        Log_entry(char* l) {
            if(log_entry_number > LOG_ENTRY_MAX)
                free_logs(LOG_PERCENT_TO_BE_LEFT, true);
            log_entry = new String(l);
            log_entry_number++;
//            numero = log_entry_number;
//            printf("   LOG (%lu %s) log_entry %lu %s\n", log->numero, log->info->get_string(), log_entry_number, l);
        }
        
        void print(){
            printf("   %s\n", log_entry->get_string());
        }
    };

    size_t start_in_store = 0;
    size_t log_size = 0;
    size_t numero = 0;
    String *info = nullptr;
    Queue<Log_entry> log_entries = {};
    Log* prev = nullptr;
    Log* next = nullptr;
    
public:
    static bool log_on;

    Log(){}

    Log(const char* , const char*, uint64);
    Log &operator = (Log const &);

//    ALWAYS_INLINE
//    static inline void *operator new (size_t) { return cache.alloc(); }
    
    Log(const Log& orig);    
    
    ~Log() {
        Log_entry *li = nullptr;
        while(log_entries.dequeue(li = log_entries.head())){
            delete li;
        } 
        delete info;
        assert(log_number);
        log_number--;
    } 
    
//    ALWAYS_INLINE
//    static inline void operator delete (void *ptr) {
//        cache.free (ptr);
//    }
    
    void print(bool from_tail = true){
        printf("LOG %lu size %lu %s\n", numero, log_size, info->get_string());        
        Log_entry *log_info = from_tail ? log_entries.tail() : log_entries.head(), *end = from_tail ? 
            log_entries.tail() : log_entries.head(), 
            *n = nullptr;
        while(log_info) {
            log_info->print();
            n = from_tail ? log_info->prev : log_info->next;
            log_info = (n == end) ? nullptr : n;
        }
    }
        
    static size_t get_number(){ return log_number; }
    
    static size_t get_total_log_size() { return log_entry_number; }
    
    static void add_log(const char*, const char*);
    
    static void free_logs(size_t=0, bool=false);
    
    static void dump(char const*, bool = true, uint32 = 5);
            
    static void add_log_entry(const char*);
    
    static void append_log_info(const char*);
};
