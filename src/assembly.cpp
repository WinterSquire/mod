#include <mod/assembly.h>
#include <hde64.h>
#include <cstring>

size_t call(char* buffer, int32_t rel32) {
	auto instruction = reinterpret_cast<s_invoke_rel32*>(buffer);

	instruction->opcode = _opcode_call_rel32;
	instruction->offset = rel32 - sizeof(s_invoke_rel32);

	return sizeof(*instruction);
}

size_t call(char* buffer, int64_t m64) {
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
        return call(buffer, static_cast<int32_t>(offset));
	else
		return call(buffer, reinterpret_cast<int64_t>(DST));
}

size_t jmp(char* buffer, int8_t rel8) {
	auto insturction = reinterpret_cast<s_jmp_rel8*>(buffer);
	insturction->opcode = _opcode_jmp_rel8;
	insturction->offset = rel8 - sizeof(*insturction);
	return sizeof(*insturction);
}

size_t jmp(char* buffer, int32_t rel32) {
	auto instruction = reinterpret_cast<s_invoke_rel32*>(buffer);
	instruction->opcode = _opcode_jmp_rel32;
	instruction->offset = rel32 - sizeof(*instruction);
	return sizeof(*instruction);
}

size_t jmp(char* buffer, int64_t m64) {
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
		return jmp(buffer, static_cast<int8_t>(offset));
	else if (abs_offset < 0x80000000) 
		return jmp(buffer, static_cast<int32_t>(offset));
	else
		return jmp(buffer, reinterpret_cast<int64_t>(DST));
}

size_t unassemble(char* buffer, const char* instruction, void* RIP, size_t size) {
	hde64s hs;
	auto base = buffer;
	auto begin = instruction;
	auto end = instruction + size;

	do {
		if (*begin == _opcode_ret && begin + 1 < end) {
			return 0;
		}

		hde64_disasm(begin, &hs);

		if (hs.flags & F_ERROR) {
			return 0;
		}

		switch (hs.opcode) {
		default:
			memcpy(buffer, begin, hs.len);
			buffer += hs.len;
			begin += hs.len;
		}

	} while (begin < end);

	size_t ins_size = begin - instruction;
	int64_t jmp_addr = reinterpret_cast<int64_t>(RIP) + ins_size;
	buffer += jmp(buffer, jmp_addr);
	size_t total_size = buffer - base;

	constexpr int k_code_alignment = 4;
	constexpr int k_code_alignment_mask = ~(k_code_alignment - 1);

	size_t padding = ((total_size - 1) & k_code_alignment_mask) + k_code_alignment - total_size;

	if (padding)
		memset(buffer, _opcode_int3, padding);

	return total_size + padding;
}