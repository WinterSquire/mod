#include <mod/assembly.h>

size_t call(char* buffer, void* RIP, int32_t rel32) {
	auto instruction = reinterpret_cast<s_call_rel32*>(buffer);

	instruction->opcode = 0xE8;
	instruction->offset = rel32 - sizeof(s_call_rel32);

	return sizeof(s_call_rel32);
}

size_t call(char* buffer, void* RIP, int64_t m64) {
	auto instruction = reinterpret_cast<s_invoke_absolute_indirect*>(buffer);

	instruction->Opcode = 0xFF;
	instruction->ModeRM.rm = 0b101;   // memory
	instruction->ModeRM.reg = 0b010;  // CALL NEAR (or 0b100 for JMP NEAR)
	instruction->ModeRM.mod = 0b00;   // memory, no displacement
	instruction->DISP32 = 0;

	auto address = reinterpret_cast<int64_t*>(buffer + sizeof(s_invoke_absolute_indirect));
	*address = m64;

	return sizeof(s_invoke_absolute_indirect) + sizeof(int64_t);
}

size_t call(char* buffer, void* RIP, void* DST) {

	return 0;
}

size_t jmp(char* buffer, void* RIP, int8_t rel8) {
	return sizeof(s_jmp_rel8);
}

size_t jmp(char* buffer, void* RIP, int32_t rel32) {

	return sizeof(s_jmp_rel32);
}

size_t jmp(char* buffer, void* RIP, int64_t m64) {
	auto instruction = reinterpret_cast<s_invoke_absolute_indirect*>(buffer);

	instruction->Opcode = 0xFF;
	instruction->ModeRM.rm = 0b101;   // memory
	instruction->ModeRM.reg = 0b100;  // JMP NEAR
	instruction->ModeRM.mod = 0b00;   // memory, no displacement
	instruction->DISP32 = 0;

	auto address = reinterpret_cast<int64_t*>(buffer + sizeof(s_invoke_absolute_indirect));
	*address = m64;

	return sizeof(s_invoke_absolute_indirect) + sizeof(int64_t);
}

size_t jmp(char* buffer, void* RIP, void* DST) {
	auto offset = reinterpret_cast<int64_t>(RIP) - reinterpret_cast<int64_t>(DST);

	return 0;
}