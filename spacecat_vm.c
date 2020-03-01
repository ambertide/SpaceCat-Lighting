#include <stdio.h>
#include <string.h>
#include <math.h>
#include "spacecat_vm.h"

#define INSTRUCTION_SIZE 4
#define HEX_MAX_SIZE 2
#define INSTRUCTION_BIT_SIZE 8
typedef unsigned char bit;
char memory[126];
char *instruction_register = memory;
int program_counter = 0;
char svm_svm_registers[16];


typedef struct {
    bit bits[8];
} byte;

void create_byte(byte* byte) {
    for (int i = 0; i < 8; i++) {
        byte->bits[i] = 0;
    }
}

void jump_to(int address) {
    instruction_register += (address * 2  - program_counter);
    program_counter = address / 2;
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
    svm_svm_registers[register_address] = memory[memory_address];
}

void parse_value_load(const char* instruction) {
    int register_address = parse_hex(&instruction[1], 1);
    int hex_value = parse_hex(&instruction[2], HEX_MAX_SIZE);
    svm_svm_registers[register_address] = (char) hex_value;
}

void parse_register_pointer_load(const char* instruction) {
    int register_address_recv = parse_hex(&instruction[2], 1);
    int register_address_ptr = parse_hex(&instruction[3], 1);
    svm_svm_registers[register_address_recv] = memory[svm_svm_registers[register_address_ptr]];
}

void parse_pointer_store(const char* instruction) {
    int register_address = parse_hex(&instruction[1], 1);
    int memory_address = parse_hex(&instruction[2], 2);
    memory[memory_address] = svm_registers[register_address];
}

void parse_register_store(const char* instruction) {
    int register_address_hold = parse_hex(&instruction[2], 1);
    int register_address_mem_ptr = parse_hex(&instruction[3], 1);
    memory[svm_registers[register_address_mem_ptr]] = svm_registers[register_address_hold];
}

void parse_move(const char* instruction) {
    int register_address_recv = parse_hex(&instruction[3], 1);
    int register_address_supply = parse_hex(&instruction[2], 1);
    svm_registers[register_address_recv] = svm_registers[register_address_supply];
}

void parse_ror(const char* instruction) {
    unsigned rotate_n_times = parse_hex(&instruction[3], 1);
    unsigned rotation_count = rotate_n_times % INSTRUCTION_BIT_SIZE;
    unsigned register_address = parse_hex(&instruction[1], 1);
    unsigned source = (unsigned) svm_registers[register_address];
    unsigned carried_bits = source << (INSTRUCTION_BIT_SIZE - rotation_count);
    unsigned shifted_bits = source >> (rotation_count);
    svm_registers[register_address] = (char) (shifted_bits | carried_bits);
}

void parse_jmpeq(const char* instruction) {
    int reg_to_check = parse_hex(&instruction[1], 1);
    int addr_to_jmp = parse_hex(&instruction[2], 2);
    if (svm_registers[reg_to_check] == svm_registers[0]) {
        jump_to(addr_to_jmp);
    }
}

void parse_jmple(const char* instruction) {
    int reg_to_check = parse_hex(&instruction[1], 1);
    int addr_to_jmp = parse_hex(&instruction[2], 2);
    if (svm_registers[reg_to_check] <= svm_registers[0]) {
        jump_to(addr_to_jmp);
    }
}

char add_float(char value_1, char value_2) {
    byte value_one;
    create_byte(&value_one);
    byte value_two;
    create_byte(&value_two);
    bit temp_val;
    for (unsigned i = 7; i >= 0; i--) {
         temp_val = ((unsigned) value_1) >> i;
         temp_val &= (unsigned) 1;
         value_one.bits[7 - i] = temp_val;
    }

    for (unsigned i = 7; i >= 0; i--) {
        temp_val = ((unsigned) value_2) >> i;
        temp_val &= (unsigned) 1;
        value_two.bits[7 - i] = temp_val;
    }

    byte bigger;
    byte smaller;
    int same = 1;

    for (int i = 1; i < 4; i++) {
        if (value_one.bits[i] > value_two.bits[i]) {
            bigger = value_one;
            smaller = value_two;
            same = 0;
            break;
        } else if (value_two.bits[i] > value_one.bits[i]) {
            bigger = value_two;
            smaller = value_one;
            same = 0;
            break;
        }
    }

    if (same == 0) {
        unsigned bigger_exponent = value_one.bits[1] | value_one.bits[2] << 1 | value_one.bits[3] << 2;
        unsigned smaller_exponent = value_two.bits[1] |value_two.bits[2] << 1 | value_two.bits[3] << 2;

    }

}

void parse_arithmatic(const char* instruction) {
    int reg_recv_addr = parse_hex(&instruction[1], 1);
    int reg_arithmatic_one = parse_hex(&instruction[2], 1);
    int reg_arithmatic_two = parse_hex(&instruction[3], 1);
    switch (*instruction) {
        case '5': svm_registers[reg_recv_addr] = (char) (svm_registers[reg_arithmatic_one] + svm_registers[reg_arithmatic_two]); break;
        case '6': svm_registers[reg_recv_addr] = add_float(reg_arithmatic_one, reg_arithmatic_two); break;
        case '7': svm_registers[reg_recv_addr] = (char) ((unsigned) reg_arithmatic_one | (unsigned) reg_arithmatic_two); break;
        case '8': svm_registers[reg_recv_addr] = (char) ((unsigned) reg_arithmatic_one & (unsigned) reg_arithmatic_two); break;
        case '9': svm_registers[reg_recv_addr] = (char) ((unsigned) reg_arithmatic_one ^ (unsigned) reg_arithmatic_two); break;
    }
    printf(instruction);
    printf("  ->  R%X: %d which is R%X (%d) "
           "+ R%X (%d)\n", reg_recv_addr, svm_registers[reg_recv_addr], reg_arithmatic_one,  svm_registers[reg_arithmatic_one], reg_arithmatic_two, svm_registers[reg_arithmatic_two]);

}

char return_stdout() {
    return svm_registers[15];
}

void parse_instruction(const char* instruction) {
    switch(*instruction) {
        case '1': parse_memory_pointer_load(instruction); break;
        case '2': parse_value_load(instruction); break;
        case '3': parse_pointer_store(instruction); break;
        case '4': parse_move(instruction); break;
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