/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   log_store.hpp
 * Author: parfait
 *
 * Created on 17 octobre 2019, 19:50
 */
#pragma once

#include "config.hpp"
#include "log.hpp"


class Logentrystore {
    friend class Logstore;
private:
    static Log::Log_entry logentries[LOG_ENTRY_MAX];
    static size_t cursor, start, end;
    
public:
    Logentrystore();
    Logentrystore(const Logentrystore& orig);
    ~Logentrystore();
    static void add_log_entry(const char*);
    static size_t get_logentry_total_number() {
        return start - cursor;
    }
    static void free_logentries(size_t, size_t);
};

class Logstore {
private:
    static Log logs[LOG_MAX];
    static size_t cursor, start, end;
    
public:
    Logstore();
    Logstore(const Logstore& orig);
    ~Logstore();
    
    static void add_log(const char*, const char*);
        
    static void free_logs(size_t=0, bool=false);
        
    static void dump(char const*, bool = true, uint32 = 5);
            
    static void add_log_entry(const char*);
    
    static void append_log_info(const char*);
        
};
