//
// Galactic Kitten, the new implementation.
//
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "spacecat_vm_gk.h"



typedef struct {
    /**
     * A type that defines a SVM cell.
     */
    unsigned char first_char;
    unsigned char second_char;
} svm_cell;

int svm_error_occured;

enum STDOUT_STATE {
    SLEEPING,
    ACTIVE
};

enum STDOUT_STATE  stdout_state = SLEEPING;

enum ERROR_CODE{
    RESERVED_OP_CODE_CALLED = 0,
    ILLEGAL_OP_CODE = 1
};

enum OP_CODE {
    RESERVED = '0',
    MEMORY_POINTER_LOAD = '1',
    VALUE_LOAD = '2',
    MEMORY_STORE = '3',
    MOVE = '4',
    ADDI = '5',
    ADDF = '6',
    OR = '7',
    AND = '8',
    XOR = '9',
    ROR = 'A',
    JMPEQ = 'B',
    HALT = 'C',
    REGISTER_POINTER_LOAD = 'D',
    REGISTER_POINTER_STORE = 'E',
    JMPLE = 'F'
};

void svm_cell_assign(svm_cell* cell_receiver, svm_cell* cell_sender) {
    cell_receiver -> first_char = cell_sender -> first_char;
    cell_receiver -> second_char = cell_sender -> second_char;
}

void svm_cell_assign_value(svm_cell* cell_receiver, unsigned int hex) {
    cell_receiver -> first_char = hex >> 4u;
    cell_receiver -> second_char = hex & 15u;
}

unsigned char svm_cell_retrieve_value(svm_cell* cell) {
    return ((cell -> first_char) << 4u) | (cell -> second_char);
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



void svm_create_cell(svm_cell* cell, unsigned char first_char, unsigned char second_char) {
    /*
     * Creates an SVM cell.
     */
    cell->first_char = first_char;
    cell->second_char = second_char;
}


svm_cell svm_memory[126];
svm_cell svm_registers[16];
unsigned char svm_instruction_register[4];
int svm_program_counter;

void initialise_machine() {
    for (int i = 0; i < 126; i++) {
        svm_cell* ptr = &svm_memory[i];
        svm_create_cell(ptr, 0, 0);
    }

    for (int j = 0; j < 16; j++) {
        svm_cell* ptr = &svm_registers[j];
        svm_create_cell(ptr, 0, 0);
    }
}

void svm_load_memory(const char* instructions, int address) {
    svm_cell* ptr = svm_memory;
    for (int i = address; instructions[i] != 0; i+=2) {
        svm_create_cell(ptr, instructions[i], instructions[i + 1]);
        ptr++;
    }
}

void jump(int address) {
    svm_program_counter = address - 2;
}

void parse_memory_pointer_load(const char* instruction) {
    int register_index = parse_hex(&instruction[1], 1);
    int memory_index = parse_hex(&instruction[2], 2);
    svm_cell_assign(&svm_registers[register_index], &svm_memory[memory_index]);
}

void parse_value_load(const char* instruction) {
    int register_index = parse_hex(&instruction[1], 1);
    int value = parse_hex(&instruction[2], 2);
    svm_cell_assign_value(&svm_registers[register_index], value);
}

void parse_register_pointer_load(const char* instruction) {
    int register_recv_index = parse_hex(&instruction[2], 1);
    int register_send_index  = parse_hex(&instruction[3], 1);
    unsigned int val_= svm_cell_retrieve_value(&svm_registers[register_send_index]);
    svm_cell_assign(&svm_registers[register_recv_index], &svm_memory[val_]);
}

void parse_pointer_store(const char* instruction) {
    int register_index = parse_hex(&instruction[1], 1);
    int memory_index = parse_hex(&instruction[2], 2);
    svm_cell_assign(&svm_memory[memory_index], &svm_registers[register_index]);
}

void parse_register_store(const char* instruction) {
    int register_index = parse_hex(&instruction[2], 1);
    int register_ptr_index = parse_hex(&instruction[3], 1);
    int memory_index = svm_cell_retrieve_value(&svm_registers[register_ptr_index]);
    svm_cell_assign(&svm_memory[memory_index], &svm_registers[register_index]);
}

void parse_move(const char* instruction) {
    int register_send = parse_hex(&instruction[2], 1);
    int register_recv = parse_hex(&instruction[3], 1);
    svm_cell_assign(&svm_registers[register_recv], &svm_registers[register_send]);
}

void parse_ror(const char* instruction) {
    unsigned rotate_n_times = parse_hex(&instruction[3], 1);
    unsigned rotation_count = rotate_n_times % 8;
    unsigned register_address = parse_hex(&instruction[1], 1);
    unsigned source = svm_cell_retrieve_value(&svm_registers[register_address]);
    unsigned carried_bits = source << (8 - rotation_count);
    unsigned shifted_bits = source >> (rotation_count);
    svm_cell_assign_value(&svm_registers[register_address], (char) (shifted_bits | carried_bits));
}


void parse_jmpeq(const char* instruction) {
    int value = svm_cell_retrieve_value(&svm_registers[parse_hex(&instruction[1], 1)]);
    int check_val = svm_cell_retrieve_value(&svm_registers[0]);
    if (check_val == value) {
        int jump_address = parse_hex(&instruction[2], 2);
        jump(jump_address);
    }
}
void parse_jmple(const char* instruction) {
    int value = svm_cell_retrieve_value(&svm_registers[parse_hex(&instruction[1], 1)]);
    int check_val = svm_cell_retrieve_value(&svm_registers[0]);
    if (check_val > value) {
        int jump_address = parse_hex(&instruction[2], 2);
        jump(jump_address);
    }
}

void parse_addf(const char* instruction) {}

void parse_arithmatic(const char* instruction) {
    int reg_addr_recv = parse_hex(&instruction[1], 1);
    int reg_operand_1 = parse_hex(&instruction[2], 1);
    int reg_operand_2 = parse_hex(&instruction[3], 1);
    if (reg_addr_recv == 15) {
        stdout_state = ACTIVE;
    }
    switch(instruction[0]) {
        case ADDI: svm_cell_assign_value(&svm_registers[reg_addr_recv], svm_cell_retrieve_value(&svm_registers[reg_operand_1]) + svm_cell_retrieve_value(&svm_registers[reg_operand_2])); break;
        case ADDF: parse_addf(instruction); break;
        case OR: svm_cell_assign_value(&svm_registers[reg_addr_recv], (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_1]) | (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_2])); break;
        case AND: svm_cell_assign_value(&svm_registers[reg_addr_recv], (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_1]) &  (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_2])); break;
        case XOR: svm_cell_assign_value(&svm_registers[reg_addr_recv], (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_1]) ^  (unsigned int) svm_cell_retrieve_value(&svm_registers[reg_operand_2])); break;
    }
}

void svm_error(enum ERROR_CODE code) {
    const char* error_message;
    switch (code) {
        case RESERVED_OP_CODE_CALLED:
        case ILLEGAL_OP_CODE: error_message = "Illegal OP code used."; break;
        default: error_message = "An unknown exception occurred."; break;
    }
    printf(error_message, 30);
    svm_error_occured = 1;
}

void parse_instruction(const char* instruction) {
    char op_code = instruction[0];
    if ((op_code == MEMORY_POINTER_LOAD || op_code == VALUE_LOAD) && parse_hex(&instruction[1], 1) == 15) {
        stdout_state = ACTIVE;
    } else if (op_code == MOVE && parse_hex(&instruction[3], 1) == 15) {
        stdout_state = ACTIVE;
    } else if (op_code == REGISTER_POINTER_LOAD && parse_hex(&instruction[2], 1) == 15) {
        stdout_state = ACTIVE;
    }
    switch(op_code) {
        case MEMORY_POINTER_LOAD: parse_memory_pointer_load(instruction); break;
        case VALUE_LOAD: parse_value_load(instruction); break;
        case MEMORY_STORE: parse_pointer_store(instruction); break;
        case MOVE: parse_move(instruction); break;
        case ADDI:
        case ADDF:
        case OR:
        case AND:
        case XOR: parse_arithmatic(instruction); break;
        case ROR: parse_ror(instruction); break;
        case JMPEQ: parse_jmpeq(instruction); break;
        case HALT: break;
        case REGISTER_POINTER_LOAD: parse_register_pointer_load(instruction); break;
        case REGISTER_POINTER_STORE: parse_register_store(instruction);
        case JMPLE: parse_jmple(instruction); break;
        case RESERVED: svm_error(RESERVED_OP_CODE_CALLED); break;
        default: svm_error(ILLEGAL_OP_CODE); break;
    }
}

unsigned char return_stdout() {
    return svm_cell_retrieve_value(&svm_registers[15]);
}

void svm_load_next_instruction() {
    for (int i = 0; i < 2; i++) {
        svm_instruction_register[2 * i] = svm_memory[svm_program_counter + i].first_char;
        svm_instruction_register[2 * i + 1] = svm_memory[svm_program_counter + i].second_char;
    }
}


void run(const char* instructions) {
    svm_load_memory(instructions, 0);
    svm_load_next_instruction();
    const char* ptr = svm_instruction_register;
    while (strncmp(svm_instruction_register, "C000", 4) != 0 && svm_error_occured == 0) {
        svm_load_next_instruction();
        parse_instruction(svm_instruction_register);
        if (stdout_state == ACTIVE) {
            printf("%c", return_stdout());
            stdout_state = SLEEPING;
        }
        svm_program_counter += 2;
    }
}