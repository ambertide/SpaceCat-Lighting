#include <stdio.h>
#include <string.h>
#include <math.h>
#include "spacecat_vm.h"

#define INSTRUCTION_SIZE 4
#define HEX_MAX_SIZE 2
char memory[126];
char *instruction_register = memory;
int program_counter = 0;
char registers[16];


void jump_to(int address) {
    instruction_register += address - program_counter;
    program_counter = address;
}

int parse_hex(const char* instruction_hex_start, const int size) {
    int sum_value = 0;
    for (int i = 0; i < size; i++) {
        if (*instruction_hex_start >= '0' && *instruction_hex_start < 'A') {
            sum_value += (*instruction_hex_start - '0') * (int) pow(16, size - (i + 1));
        } else if (*instruction_hex_start >= 'A') {
            sum_value += (*instruction_hex_start - 'A' + 10) * (int) pow(16, size - (i + 1));
        }
        instruction_hex_start++;
    }
    return sum_value;
}

void parse_memory_pointer_load(const char* instruction) {
    int register_address = parse_hex(&instruction[1], 1);
    int memory_address = parse_hex(&instruction[2], HEX_MAX_SIZE);
    registers[register_address] = memory[memory_address];
}

void parse_value_load(const char* instruction) {
    int register_address = parse_hex(&instruction[1], 1);
    int hex_value = parse_hex(&instruction[2], HEX_MAX_SIZE);
    registers[register_address] = (char) hex_value;
}

void parse_register_pointer_load(const char* instruction) {
    int register_address_recv = parse_hex(&instruction[2], 1);
    int register_address_ptr = parse_hex(&instruction[3], 1);
    registers[register_address_recv] = memory[registers[register_address_ptr]];
}

void parse_pointer_store(const char* instruction) {
    int register_address = parse_hex(&instruction[1], 1);
    int memory_address = parse_hex(&instruction[2], 2);
    memory[memory_address] = registers[register_address];
}

void parse_register_store(const char* instruction) {
    int register_address_hold = parse_hex(&instruction[2], 1);
    int register_address_mem_ptr = parse_hex(&instruction[3], 1);
    memory[registers[register_address_mem_ptr]] = registers[register_address_hold];
}

void parse_move(const char* instruction) {
    int register_address_recv = parse_hex(&instruction[2], 1);
    int register_address_hold = parse_hex(&instruction[3], 1);
    registers[register_address_recv] = registers[register_address_hold];
}

void parse_ror(const char* instruction) {}

void parse_jmpeq(const char* instruction) {
    int reg_to_check = parse_hex(&instruction[1], 1);
    int addr_to_jmp = parse_hex(&instruction[2], 2);
    if (registers[reg_to_check] == registers[0]) {
        jump_to(addr_to_jmp);
    }
}

void parse_jmple(const char* instruction) {
    int reg_to_check = parse_hex(&instruction[1], 1);
    int addr_to_jmp = parse_hex(&instruction[2], 2);
    if (registers[reg_to_check] <= registers[0]) {
        jump_to(addr_to_jmp);
    }
}

char add_float(char value_1, char value_2) {}

void parse_arithmatic(const char* instruction) {
    char* reg_recv_addr = &registers[parse_hex(&instruction[1], 1)];
    char reg_arithmatic_one = registers[parse_hex(&instruction[2], 1)];
    char reg_arithmatic_two = registers[parse_hex(&instruction[3], 1)];
    switch (*instruction) {
        case '5': *reg_recv_addr = (char) (reg_arithmatic_one + reg_arithmatic_two); break;
        case '6': *reg_recv_addr = add_float(reg_arithmatic_one, reg_arithmatic_two); break;
        case '7': *reg_recv_addr = (char) ((unsigned) reg_arithmatic_one | (unsigned) reg_arithmatic_two); break;
        case '8': *reg_recv_addr = (char) ((unsigned) reg_arithmatic_one & (unsigned) reg_arithmatic_two); break;
        case '9': *reg_recv_addr = (char) ((unsigned) reg_arithmatic_one ^ (unsigned) reg_arithmatic_two); break;
    }

}

char return_stdout() {
    return registers[15];
}

void parse_instruction(const char* instruction) {
    switch(*instruction) {
        case '1': parse_memory_pointer_load(instruction); break;
        case '2': parse_value_load(instruction); break;
        case '3': parse_pointer_store(instruction); break;
        case '4': parse_move(instruction);
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': parse_arithmatic(instruction); break;
        case 'A': parse_ror(instruction); break;
        case 'B': parse_jmpeq(instruction); break;
        case 'C': break;
        case 'D': parse_register_pointer_load(instruction); break;
        case 'E': parse_register_store(instruction);
        case 'F': parse_jmple(instruction);
    }
}

void load_mem(const char* payload, int addr) {
    for (const char* ptr = payload; *ptr != 0; ptr++) {
        memory[addr] = *ptr;
        addr++;
    }
}

void run(const char* mem_load) {
    load_mem(mem_load, 0);
    while (strncmp(instruction_register, "C000", 4) != 0) {
        parse_instruction(instruction_register);
        instruction_register += 4;
        program_counter += 4;
    }
}