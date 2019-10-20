/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Pe.cpp
 * Author: Parfait Tokponnon <pafait.tokponnon@uclouvain.be>
 * The Log : provide a doubled linked circular queue to hold all created logs
 * 
 * Created on 3 octobre 2018, 14:34
 */

#include "log.hpp"
#include "string.hpp"
#include "log_store.hpp"

size_t Log::log_number = 0, Log_entry::log_entry_number = 0;
bool Log::log_on;
Queue<Log> Log::logs;
char *Log::entry_buffer = reinterpret_cast<char*>(malloc(PAGE_SIZE)), 
        *Log::log_buffer = reinterpret_cast<char*>(malloc(PAGE_SIZE)),
        *Log::entry_buffer_cursor = Log::entry_buffer,
        *Log::log_buffer_cursor = Log::log_buffer;

Log::Log(const char* title) : prev(nullptr), next(nullptr){
    info = new String(title);
    numero = log_number++;
};

/**
 * Add new log. If the log at cursor index does have a string, it just replaces 
 * its content, if not, it creates a new one. The creation process is supposed to 
 * be called only on time
 * @param pd_name
 * @param ec_name
*/
void Log::add_log(const char* s){
    if(!log_on)
        return;
    if(log_number > LOG_MAX)
        free_logs(LOG_PERCENT_TO_BE_LEFT, true);
    Log* log = new Log(s);
    logs.enqueue(log);
}

/**
 * Frees (100 - left) percent logs (if in_percent == true) or left logs (if in_percent == false)
 * in order to reclaim their memory. The function start by the oldest log. 
 * @param left
 * @param in_percent
 */
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

//Renumber the remaining logs    
    size_t i = 0, log_entry_count = 0;
    log = logs.head();
    Log *n = nullptr;
    while(log) {
        log->numero = ++i;
        log_entry_count += log->log_size;
        n = log->next;
        log = (n == logs.head()) ? nullptr : n;
    }
    
    assert (log_number == left && Log_entry::log_entry_number == log_entry_count);
}

/**
 * 
 * @param funct_name : Where we come from
 * @param from_tail : From the first log (from_tail == false) or from the last
 * @param log_depth : the number of log to be printed; default is 5; we will print
 * all logs if this is 0
 */
void Log::dump(char const *funct_name, bool from_tail, size_t log_depth){   
    if(!logs.head())
        return;
    printf("%s Log %lu log entries %lu\n", funct_name, log_number, Log_entry::log_entry_number);
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


/**
 * This will add a log entry with new string the first time the log at the cursor
 * is used, subsequent times it will just replace its content
 * @param log
 */
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

/**
 * Append new string to the log info. It does this by destroying the last buffer
 * and allocating a new one, wide enough, to hold the the old and the new strings
 * @param s
 */
void Log::append_log_info(const char* i){
    if(!log_on)
        return;    
    Log *l = logs.tail();
    assert(l);
    l->info->append(i);
}

/**
 * Prints this log's entries, if queue were used, print from log_entries, else,
 * logstore was used, print from logstore, 
 * @param from_tail
 */
void Log::print(bool from_tail){
    printf("LOG %lu size %lu %s\n", numero, log_size, info->get_string());
    if(log_number) {
        Log_entry *log_info = from_tail ? log_entries.tail() : log_entries.head(), *end = from_tail ? 
            log_entries.tail() : log_entries.head(), 
            *n = nullptr;
        while(log_info) {
            log_info->print();
            n = from_tail ? log_info->prev : log_info->next;
            log_info = (n == end) ? nullptr : n;
        }
    } else {            
        Logentrystore::dump(from_tail, start_in_store, log_size);
    }
}

/**
 * Add a log entry. This constructor is to be used only for queue logentries. 
 * @param l
 */
Log_entry::Log_entry(char* l) {
    if(log_entry_number > LOG_ENTRY_MAX)
        Log::free_logs(LOG_PERCENT_TO_BE_LEFT, true);
    log_entry = new String(l);
    log_entry_number++;
}

/**
 * store new log to the logs'buffer
 * @param s
 */
void Log::add_log_in_buffer(const char* s){
    if(!Log::log_on)
        return;    
    size_t size = strlen(s); 
    copy_string(Log::log_buffer_cursor, s);    
    *(Log::log_buffer_cursor + size) = ' '; // replace the final '\0' by ' '
    Log::log_buffer_cursor += size + 1; 
}

/**
 * store new log entry to the entries'buffer
 * @param s
 */
void Log::add_entry_in_buffer(const char* s){
    if(!Log::log_on)
        return;    
    size_t size = strlen(s)+1; // +1 for the final \n
    copy_string_nl(Log::entry_buffer_cursor, s, size);    
    Log::entry_buffer_cursor += size; 
}

/**
 * Commit log buffer and entries buffer
 */
void Log::commit_buffer(){
    if(!Log::log_on)
        return;    
    *Log::log_buffer_cursor = '\0';
    add_log(Log::log_buffer);
    memset(Log::log_buffer, 0, Log::log_buffer_cursor - Log::log_buffer + 1);
    Log::log_buffer_cursor = Log::log_buffer;
    
    *Log::entry_buffer_cursor = '\0';
    Log* l = logs.tail();
    assert(l);
    Log_entry *log_info = new Log_entry(Log::entry_buffer);
    l->log_entries.enqueue(log_info);  
    l->log_size++;
    memset(Log::entry_buffer, 0, Log::entry_buffer_cursor - Log::entry_buffer + 1);
    Log::entry_buffer_cursor = Log::entry_buffer;
}