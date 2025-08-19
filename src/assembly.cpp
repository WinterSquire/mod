#include <mod/assembly.h>

size_t call(char* buffer, void* RIP, int32_t rel32) {
	auto instruction = reinterpret_cast<s_invoke_rel32*>(buffer);

	instruction->opcode = _opcode_call_rel32;
	instruction->offset = rel32 - sizeof(s_invoke_rel32);

	return sizeof(*instruction);
}

size_t call(char* buffer, void* RIP, int64_t m64) {
	auto ins_call = reinterpret_cast<s_invoke_absolute_indirect*>(buffer);

	ins_call->opcode = _opcode_group_FF;
	ins_call->ModeRM.rm = 0b101;   // memory
	ins_call->ModeRM.reg = 0b010;  // CALL NEAR (or 0b100 for JMP NEAR)
	ins_call->ModeRM.mod = 0b00;   // memory, no displacement
	ins_call->DISP32 = 2;

	auto ins_jmp = reinterpret_cast<s_jmp_rel8*>(buffer + sizeof(*ins_call));
	ins_jmp->opcode = _opcode_jmp_rel8;
	ins_jmp->offset = sizeof(int64_t);

	auto address = reinterpret_cast<int64_t*>(buffer + sizeof(*ins_call) + sizeof(*ins_jmp));
	*address = m64;

	return sizeof(*ins_call) + sizeof(*ins_jmp) + sizeof(*address);
}

size_t call(char* buffer, void* RIP, void* DST) {
	auto offset = reinterpret_cast<int64_t>(DST) - reinterpret_cast<int64_t>(RIP);
	auto abs_offset = static_cast<uint64_t>(offset < 0 ? -offset : offset);

	if (abs_offset < 0x80000000)
        return call(buffer, RIP, static_cast<int32_t>(offset));
	else
		return call(buffer, RIP, reinterpret_cast<int64_t>(DST));
}

size_t jmp(char* buffer, void* RIP, int8_t rel8) {
	auto insturction = reinterpret_cast<s_jmp_rel8*>(buffer);
	insturction->opcode = _opcode_jmp_rel8;
	insturction->offset = rel8 - sizeof(*insturction);
	return sizeof(*insturction);
}

size_t jmp(char* buffer, void* RIP, int32_t rel32) {
	auto instruction = reinterpret_cast<s_invoke_rel32*>(buffer);
	instruction->opcode = _opcode_jmp_rel32;
	instruction->offset = rel32 - sizeof(*instruction);
	return sizeof(*instruction);
}

size_t jmp(char* buffer, void* RIP, int64_t m64) {
	auto instruction = reinterpret_cast<s_invoke_absolute_indirect*>(buffer);

	instruction->opcode = _opcode_group_FF;
	instruction->ModeRM.rm = 0b101;   // memory
	instruction->ModeRM.reg = 0b100;  // JMP NEAR
	instruction->ModeRM.mod = 0b00;   // memory, no displacement
	instruction->DISP32 = 0;

	auto address = reinterpret_cast<int64_t*>(buffer + sizeof(s_invoke_absolute_indirect));
	*address = m64;

	return sizeof(*instruction) + sizeof(*address);
}

size_t jmp(char* buffer, void* RIP, void* DST) {
	auto offset = reinterpret_cast<int64_t>(DST) - reinterpret_cast<int64_t>(RIP);
	auto abs_offset = static_cast<uint64_t>(offset < 0 ? -offset : offset);

	if (abs_offset < 0x80)
		return jmp(buffer, RIP, static_cast<int8_t>(offset));
	else if (abs_offset < 0x80000000) 
		return jmp(buffer, RIP, static_cast<int32_t>(offset));
	else
		return jmp(buffer, RIP, reinterpret_cast<int64_t>(DST));
}