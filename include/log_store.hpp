/* 
 * File:   log_store.hpp
 * Author: Parfait Tokponnon <pafait.tokponnon@uclouvain.be>
 * The Log store : provide almost ready-to-be-used logs, so it does not create 
 * new log by resorting to new keyword. It can hold up to LOG_MAX logs and 
 * LOG_ENTRY_MAX log entries
 * 
 * Created on 17 octobre 2019, 19:50
 */
#pragma once

#include "config.hpp"
#include "log.hpp"

class Logentrystore {
    friend class Logstore;
private:
    static Log_entry logentries[LOG_ENTRY_MAX];
// Next log index in the logentryies table, start index to save the table current start index*/ 
    static size_t cursor, start ;
    static void add_log_entry(const char*);
    
public:
    Logentrystore();
    Logentrystore(const Logentrystore& orig);
    ~Logentrystore();
    static size_t get_logentry_total_number() {
        return cursor - start;
    }
    static void free_logentries(size_t, size_t);
    static void dump(bool, size_t, size_t);
};

class Logstore {
private:
    static Log logs[LOG_MAX];
    static size_t cursor, start, log_number;
    
public:
    Logstore();
    Logstore(const Logstore& orig);
    ~Logstore();
    
    static void add_log(const char*, const char*);
        
    static void free_logs(size_t=0, bool=false);
        
    static void dump(char const*, bool = true, size_t = 5);
            
    static void add_log_entry(const char*);
    
    static void append_log_info(const char*);
        
};
