//
// Created by egeem on 25/02/2020.
//

#ifndef SIMPSIM_SPACECAT_VM_H
#define SIMPSIM_SPACECAT_VM_H

void parse_memory_pointer_load(const char* instruction);
void parse_value_load(const char* instruction);
void parse_register_pointer_load(const char* instruction);
void parse_pointer_store(const char* instruction);
void parse_register_store(const char* instruction);
void parse_move(const char* instruction);
void parse_ror(const char* instruction);
void parse_jmpeq(const char* instruction);
void parse_jmple(const char* instruction);
void parse_arithmatic(const char* instruction);
void run(const char* mem_load);
char return_stdout();

#endif //SIMPSIM_SPACECAT_VM_H
