/* 
 * File:   log_store.cpp
 * Author: Parfait Tokponnon <pafait.tokponnon@uclouvain.be>
 * The Log store : provide almost ready-to-be-used logs, so it does not create 
 * new log by resorting to new keyword. It can hold up to LOG_MAX logs and 
 * LOG_ENTRY_MAX log entries
 * 
 * Created on 17 octobre 2019, 19:50
 */

#include "log_store.hpp"
#include "log.hpp"
#include <cassert>

Log Logstore::logs[LOG_MAX];
Log_entry Logentrystore::logentries[LOG_ENTRY_MAX];
size_t Logstore::cursor, Logstore::start, Logstore::log_number, Logentrystore::cursor, Logentrystore::start;

Logstore::Logstore() {
}

Logstore::~Logstore() {
}

/**
 * Add new log. If the log at cursor index does have a string, it just replaces 
 * its content, if not, it creates a new one. The creation process is supposed to 
 * be called only on time
 * @param pd_name
 * @param ec_name
 */
void Logstore::add_log(const char* log){
    if(!Log::log_on)
        return;
    size_t curr = cursor%static_cast<size_t>(LOG_MAX);
    Log *l = &logs[curr];
    if(l->info) {
        l->info->replace_with(log);        
    } else {
        l->info = new String(log);
    }
    l->numero = log_number++;
    cursor++;
}

/**
 * Frees (100 - left) percent logs (if in_percent == true) or left logs (if in_percent == false)
 * in order to reclaim their memory. The function start by the oldest log. 
 * @param left
 * @param in_percent
 */
void Logstore::free_logs(size_t left, bool in_percent) {
    if(!log_number)
        return;
    if(in_percent){
        assert(left && left < 100);
        size_t log_number = cursor - start;
        left = left * log_number/100;
    }
// In no case should all logs be deleted, in order to avoid null pointer bug in logs queue         
    if(!left) 
        left = 1; 
    size_t log_max = static_cast<size_t>(LOG_MAX), 
            entry_start = 0, entry_end = 0, // to calculate free_logentries() args
            i_start = start%log_max, i_end = (cursor - left)%log_max;
    // If we reach the laxt index in the table, we will continue with index 0
    size_t s = i_start, e = i_start < i_end ? i_end : log_max; 
    for(size_t i = s; i < e; i++) {
        Log* l = &logs[i];
        if(!entry_start && l->log_size) // use the first log with entry to initialize entry_start
            entry_start = l->start_in_store;
        if(l->log_size) // entry_end will be the laxt entry of the last log
            entry_end = l->start_in_store + l->log_size;
        l->start_in_store = 0; // clear its start_in_store, log_size, numero and
        l->log_size = 0;        // free its memory
        l->numero = 0;
        l->info->free_block();
        if(i_start > i_end && i == log_max - 1){ // If we were to round from the last
            i = ~static_cast<size_t>(0ul);
            e = i_end;
        }
    }
    if(entry_end - entry_start) // if freed logs have entries
        Logentrystore::free_logentries(entry_start, entry_end);
    
    //Renumber the remaining logs
    i_start = e%log_max; i_end = (e + left)%log_max;
    s = i_start; e = i_start < i_end ? i_end : log_max;
    size_t numero = 0;
    for(size_t i = s; i < e; i++) {
        Log* l = &logs[i];
        l->numero = numero++;
        if(i_start > i_end && i == log_max - 1){
            i = ~static_cast<size_t>(0ul);
            e = i_end;
        }
    }
    log_number = numero;
    start = cursor - left;
}

/**
 * Frees consecutive logentries from f_start to f_end - 1
 * @param f_start
 * @param f_end
 */
void Logentrystore::free_logentries(size_t f_start, size_t f_end) {
    size_t log_entry_max = static_cast<size_t>(LOG_ENTRY_MAX), 
            j_start = f_start%log_entry_max, j_end = f_end%log_entry_max;
    size_t s = j_start, e = j_start < j_end ? j_end : log_entry_max;
    for(size_t j=s; j < e; j++){
        logentries[j].log_entry->free_block();
        if(j_start > j_end && j == log_entry_max - 1){
            j = ~static_cast<size_t>(0ul);
            e = j_end;
        }
    }
    start = f_end;
}

/**
 * 
 * @param funct_name : Where we come from
 * @param from_tail : From the first log (from_tail == false) or from the last
 * @param log_depth : the number of log to be printed; default is 5; we will print
 * all logs if this is 0
 */
void Logstore::dump(char const *funct_name, bool from_tail, size_t log_depth){   
    if(cursor == 0 && start == 0)
        return;
    size_t log_number = cursor - start, log_max = static_cast<size_t>(LOG_MAX),
        log_entry_number = Logentrystore::get_logentry_total_number();
    printf("%s Log %lu log entries %lu\n", funct_name, log_number, log_entry_number);
    if(from_tail){
        size_t i_start = (cursor-1)%log_max, i_end = log_depth ? (cursor - log_depth)%log_max : start%log_max;
        size_t s = i_start, e = i_start > i_end ? i_end : 0;
        for(size_t i = s; i >= e; i--) {
            logs[i].print(false);
            if(i_start < i_end && i == 0){
                i = log_max;
                e = i_end;
            }
        }
    } else {
        size_t i_start = start%log_max, i_end = log_depth ? (start + log_depth)%log_max : cursor%log_max;
        size_t s = i_start, e = i_start < i_end ? i_end : log_max;
//        printf("start %lu cursor %lu log_depth %lu i_start %lu i_end %lu s %lu e %lu\n", start, cursor, log_depth, i_start, i_end, s, e);
        for(size_t i = s; i < e; i++) {
            logs[i].print(false);
            if(i_start > i_end && i == log_max - 1){
                i = ~static_cast<size_t>(0ul);
                e = i_end;
            }
        }
    }    
}

/**
 * 
 * @param from_tail
 * @param from : start index
 * @param size : the number of entries to be printed
 */
void Logentrystore::dump(bool from_tail, size_t from, size_t size){
    if(!size)
        return;
    size_t log_entry_max = static_cast<size_t>(LOG_ENTRY_MAX);
    if(from_tail){
        size_t i_start = from%log_entry_max, i_end = (from - size)%log_entry_max;
        size_t s = i_start, e = i_start > i_end ? i_end : 0;
        for(size_t i = s; i >= e; i--) {
            logentries[i].print();
            if(from < size && i == 0){
                i = log_entry_max;
                e = i_end;
            }
        }
    } else {
        size_t i_start = from%log_entry_max, i_end = (from + size)%log_entry_max;
        size_t s = i_start, e = i_start < i_end ? i_end : log_entry_max;
        for(size_t i = s; i < e; i++) {
            logentries[i].print();
            if(i_start > i_end && i == log_entry_max - 1){
                i = ~static_cast<size_t>(0ul);
                e = i_end;
            }
        }
    }
}

/**
 * This will add a log entry with new string the first time the log at the cursor
 * is used, subsequent times it will just replace its content
 * @param log
 */
void Logstore::add_log_entry(const char* log){
    if(!Log::log_on)
        return;    
    size_t log_max = static_cast<size_t>(LOG_MAX);
    Log* l = &logs[(cursor-1)%log_max];
    assert(l->info->get_string());
    char buff[STR_MAX_LENGTH];
    String::print(buff, "%lu %s", l->log_size, log);
    if(!l->log_size)
        l->start_in_store = Logentrystore::cursor;
    l->log_size++;
    Logentrystore::add_log_entry(buff);
}

/**
 * Private function, to be called by Logstore::add_log_entry(); add entry for the 
 * last log in the logentries table
 * @param log
 */
void Logentrystore::add_log_entry(const char* log){
    size_t log_entry_max = static_cast<size_t>(LOG_ENTRY_MAX);
    size_t curr = cursor%log_entry_max;
    Log_entry *le = &logentries[curr];
    if(le->log_entry) {
        le->log_entry->replace_with(log);        
    } else {
        le->log_entry = new String(log);                
    }
    cursor++;
}    

/**
 * Append new string to the log info. It does this by destroying the last buffer
 * and allocating a new one, wide enough, to hold the the old and the new strings
 * @param s
 */
void Logstore::append_log_info(const char* s){
    if(!Log::log_on)
        return;    
    size_t log_max = static_cast<size_t>(LOG_MAX);
    Log* l = &logs[(cursor-1)%log_max];
    assert(l->info->get_string());
    l->info->append(s);
}

/**
 * store new log to the logs'buffer
 * @param s
 */
void Logstore::add_log_in_buffer(const char* s){
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
void Logstore::add_entry_in_buffer(const char* s){
    if(!Log::log_on)
        return;    
    size_t size = strlen(s)+1; // +1 for the final \n
    copy_string_nl(Log::entry_buffer_cursor, s, size);    
    Log::entry_buffer_cursor += size; 
}

/**
 * Commit log buffer and entries buffer
 */
void Logstore::commit_buffer(){
    if(!Log::log_on)
        return;    
    *Log::log_buffer_cursor = '\0';
    add_log(Log::log_buffer);
    memset(Log::log_buffer, 0, Log::log_buffer_cursor - Log::log_buffer + 1);
    Log::log_buffer_cursor = Log::log_buffer;
    
    *Log::entry_buffer_cursor = '\0';
    size_t log_max = static_cast<size_t>(LOG_MAX);
    Log* l = &logs[(cursor-1)%log_max];
    assert(l->info->get_string());
    if(!l->log_size)
        l->start_in_store = Logentrystore::cursor;
    l->log_size++;
    Logentrystore::add_log_entry(Log::entry_buffer);
    memset(Log::entry_buffer, 0, Log::entry_buffer_cursor - Log::entry_buffer + 1);
    Log::entry_buffer_cursor = Log::entry_buffer;
}