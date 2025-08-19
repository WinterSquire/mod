#pragma once

#include <cstdint>

#if !defined(_WIN64) && !defined(__x86_64__)
	static_assert(false);
#endif

constexpr int k_maximum_instruction_size = 15;

#pragma pack(push, 1)

enum e_opcode : unsigned char {
	_opcode_call_rel32 = 0xE8,
	_opcode_jmp_rel32 = 0xE9,
	_opcode_jmp_rel8 = 0xEB,
	_opcode_group_FF = 0xFF
};

struct s_jmp_rel8 {
	e_opcode opcode;
	int8_t offset;
};

static_assert(sizeof(s_jmp_rel8) == 2);

struct s_invoke_rel32 {
	e_opcode opcode;
	int32_t offset;
};

static_assert(sizeof(s_invoke_rel32) == 5);

struct s_invoke_absolute_indirect {
	e_opcode opcode;
	struct {
		char rm : 3;
		char reg : 3;
		char mod : 2;
	} ModeRM;
	int32_t DISP32;
};

static_assert(sizeof(s_invoke_absolute_indirect) == 6);

#pragma pack(pop)

size_t call(char* buffer, void* RIP, int32_t rel32);
size_t call(char* buffer, void* RIP, int64_t m64);
size_t call(char* buffer, void* RIP, void* DST);

size_t jmp(char* buffer, void* RIP, int8_t rel8);
size_t jmp(char* buffer, void* RIP, int32_t rel32);
size_t jmp(char* buffer, void* RIP, int64_t m64);
size_t jmp(char* buffer, void* RIP, void* DST);