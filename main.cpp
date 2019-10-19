/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: parfait
 *
 * Created on 11 octobre 2019, 15:51
 */

#include <cstdlib>
#include <cstdio>
#include "log.hpp"
#include <csignal> 

using namespace std;

void signal_handler(int signal_num ) { 
   Log::dump("CTRL C", false, 0); 
   exit(0);
} 
  
/*
 * 
 */
int main(int argc, char** argv) {
    Log::log_on = true;
    signal(SIGINT, signal_handler);   
    int i = 0;
    char chaine[3][STR_MAX_LENGTH] = {"Première phrase courte", 
    "Deuxieme phrase plus longue que 1er", 
    "Troisième phrase encore plus longue 2e et 1er"};
    while(1){
        char s[STR_MAX_LENGTH];
        snprintf(s, STR_MAX_LENGTH, "PD %d EC %d", i, i);
        Log::add_log(s, " ");
        int n = rand()%10; 
        for(int j=0; j<n; j++){
            int k = rand()%3;
            char d[STR_MAX_LENGTH];
            snprintf(d, STR_MAX_LENGTH, "Log_entry %d %s", j, chaine[k]);
            Log::add_log_entry(d);
        }
        i++;
    }
}

