/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   log_store.cpp
 * Author: parfait
 * 
 * Created on 17 octobre 2019, 19:50
 */

#include "log_store.hpp"
#include "log.hpp"
#include "util.hpp"
#include <cassert>

Log Logstore::logs[LOG_MAX];
size_t Logstore::cursor, Logstore::start, Logstore::end;
Log::Log_entry Logentrystore::logentries[LOG_ENTRY_MAX];
size_t Logentrystore::cursor, Logentrystore::start, Logentrystore::end;

Logstore::Logstore() {
}

Logstore::~Logstore() {
}

void Logstore::add_log(const char* pd_name, const char* ec_name){
    if(!Log::log_on)
        return;
    size_t curr = cursor%static_cast<size_t>(LOG_MAX);
    char buff[strlen(pd_name)+strlen(ec_name) + STR_MAX_LENGTH];
    String::print(buff, "Pe_num %u Pd %s Ec %s", 0, pd_name, ec_name);
    
    if(logs[curr].info)
        delete logs[curr].info;
    logs[curr].info = new String(buff);
    cursor++;
}

void Logstore::free_logs(size_t left, bool in_percent) {
    if(in_percent){
        assert(left && left < 100);
        size_t log_number = cursor - start;
        left = left * log_number/100;
    }
// In no case should all logs be deleted, in order to avoid null pointer bug in logs queue         
    if(!left) 
        left = 1; 
    size_t log_max = static_cast<size_t>(LOG_MAX), entry_start = 0, entry_end = 0,
            i_start = start%log_max, i_end = cursor%log_max - left;
    printf("Logstore Free left %lu start %lu cursor %lu istart %lu iend %lu\n", 
            left, start, cursor, i_start, i_end);
    for(size_t i = i_start; i < i_end; i++) {
        Log* l = &logs[i];
        delete l->info;
        if(!entry_start)
            entry_start = l->start_in_store;
        entry_end = l->start_in_store + l->log_size;
    }
    if(entry_end - entry_start)
        Logentrystore::free_logentries(entry_start, entry_end);
    start = cursor - left;
}

void Logentrystore::free_logentries(size_t f_start, size_t f_end) {
    size_t log_entry_max = static_cast<size_t>(LOG_ENTRY_MAX), 
            j_start = f_start%log_entry_max, j_end = f_end%log_entry_max;
    printf("Logentrystore Free fstart %lu fend %lu j_start %lu j_end %lu start "
            "%lu cursor %lu LOG_ENTRY_MAX %d\n", f_start, f_end, j_start, j_end, start, cursor, LOG_ENTRY_MAX);
    assert(start <= j_start && cursor > j_end);
    for(int j=j_start; j < j_end; j++){
        delete logentries[j].log_entry;
    }
    start = f_end;
}

void Logstore::dump(char const *funct_name, bool from_tail, uint32 log_depth){   
    if(cursor == 0 && start == 0)
        return;
    size_t log_number = cursor - start, log_max = static_cast<size_t>(LOG_MAX),
        log_entry_number = Logentrystore::get_logentry_total_number();
    printf("%s Log %lu log entries %lu\n", funct_name, log_number, log_entry_number);
    if(log_depth == 0)
        log_depth = 100000000ul;
    if(from_tail){
        size_t i_start = cursor%log_max, i_end = max(cursor%log_max - log_depth, start%log_max);
        for(size_t i = i_start; i > i_end; i--) {
            logs[i].print(false);
        }
    } else {
        size_t i_start = start%log_max, i_end = min(start%log_max + log_depth, cursor%log_max);
        for(size_t i = i_start; i < i_end; i++) {
            logs[i].print(false);
        }
    }
    
}

void Logstore::add_log_entry(const char* log){
    if(!Log::log_on)
        return;    
    size_t log_max = static_cast<size_t>(LOG_MAX);
    Log* l = &logs[cursor%log_max-1];
    assert(l->info);
    char buff[STR_MAX_LENGTH];
    String::print(buff, "%lu %s", l->log_size, log);
    l->start_in_store = Logentrystore::cursor;
    l->log_size++;
    Logentrystore::add_log_entry(buff);
}

void Logentrystore::add_log_entry(const char* log){
    size_t log_entry_max = static_cast<size_t>(LOG_ENTRY_MAX);
    size_t curr = cursor%log_entry_max;
    if(logentries[curr].log_entry)
        delete logentries[curr].log_entry;
    logentries[curr].log_entry = new String(log);
    cursor++;
}    

void Logstore::append_log_info(const char* i){
    if(!Log::log_on)
        return;    
    size_t log_max = static_cast<size_t>(LOG_MAX);
    Log* l = &logs[cursor%log_max - 1];
    assert(l->info);
    l->info->append(i);
}
