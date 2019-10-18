/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Pe.cpp
 * Author: parfait
 * 
 * Created on 3 octobre 2018, 14:34
 */

#include "log.hpp"
#include "string.hpp"
size_t Log::log_number = 0, Log::log_entry_number = 0;
bool Log::log_on;
Queue<Log> Log::logs;

/**
 * @param pd_name
 * @param ec_name
 * @param n :
 */
Log::Log(const char* pd_name, const char* ec_name, uint64 n) : prev(nullptr), next(nullptr){
    char buff[strlen(pd_name)+strlen(ec_name) + STR_MAX_LENGTH];
    String::print(buff, "Pe_num %llu Pd %s Ec %s", n, pd_name, ec_name);
    info = new String(buff);
    numero = log_number++;
};

void Log::add_log(const char* pd_name, const char* ec_name){
    if(!log_on)
        return;
    if(log_number > LOG_MAX)
        free_logs(LOG_PERCENT_TO_BE_LEFT, true);
    Log* log = new Log(pd_name, ec_name, 0);
    logs.enqueue(log);
}

void Log::free_logs(size_t left, bool in_percent) {
    if(!log_number)
        return;
    Log *log = nullptr;
    
    if(in_percent){
        assert(left && left < 100);
        left = left * log_number/100;
    }
// In no case should all logs be deleted, in order to avoid null pointer bug in logs queue         
    if(!left) 
        left = 1; 
    
    while (left < log_number && logs.dequeue(log = logs.head())) {
        delete log;
    }
    
    size_t i = 0, log_entry_count = 0;
    log = logs.head();
    Log *n = nullptr;
    while(log) {
        log->numero = ++i;
        log_entry_count += log->log_size;
        n = log->next;
        log = (n == logs.head()) ? nullptr : n;
    }
    
    assert (log_number == left && log_entry_number == log_entry_count);
}

void Log::dump(char const *funct_name, bool from_tail, uint32 log_depth){   
    if(!logs.head())
        return;
    printf("%s Log %lu log entries %lu\n", funct_name, log_number, log_entry_number);
    Log *p = from_tail ? logs.tail() : logs.head(), *end = from_tail ? logs.tail() : logs.head(), 
            *n = nullptr;
    if(log_depth == 0)
        log_depth = 100000000ul;
    uint32 count = 0;
    while(p && count<log_depth) {
        p->print(false);
        n = from_tail ? p->prev : p->next;
        p = (n == end) ? nullptr : n;
        count++;
    }
}

void Log::add_log_entry(const char* log){
    if(!log_on)
        return;    
    Log *l = logs.tail();
    assert(l);
    char buff[STR_MAX_LENGTH];
    String::print(buff, "%lu %s", l->log_size, log);
    Log_entry *log_info = new Log_entry(buff);
    l->log_entries.enqueue(log_info);  
    l->log_size++;
}

void Log::append_log_info(const char* i){
    if(!log_on)
        return;    
    Log *l = logs.tail();
    assert(l);
    l->info->append(i);
}
