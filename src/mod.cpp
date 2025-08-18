#include <mod/mod.h>
#include <hde64.h>

void write_memory(void* dst, void* src, size_t size);

void patch_create(const s_patch_create_parameters* parameters) {
	auto src_ptr = parameters->src_buffer;
	auto ori_ptr = parameters->ori_buffer;

	for (size_t i = 0; i < parameters->size; ++i) {
		auto lob = parameters->lob[i];
		auto dst = parameters->dst[i];
		auto src = parameters->src[i];

		parameters->patch[i] = s_patch{
			.dst = dst,
			.src = src_ptr,
			.ori = ori_ptr,
			.size = lob,
		};

		memcpy(src_ptr, src, lob);
		memcpy(ori_ptr, dst, lob);

		src_ptr += lob;
		ori_ptr += lob;
	}
}

void patch_vftable_create(const s_patch_vftable_create_parameters* parameters) {
	for (size_t i = 0; i < parameters->size; ++i) {
		auto lob = parameters->lob[i];
		auto dst = parameters->dst[i];
		auto src = parameters->src[i];
		auto ori = parameters->ori[i];

		parameters->patch[i] = s_patch{
			.dst = dst,
			.src = src,
			.ori = ori,
			.size = lob,
		};

		write_memory(ori, dst, lob);
	}
}

void patch_detour_create(const s_patch_detour_create_parameters* parameters) {
	auto src_ptr = parameters->src_buffer;
	auto ori_ptr = parameters->ori_buffer;
	auto code_ptr = parameters->code_buffer;

	for (int i = 0; i < parameters->size; ++i) {
		auto src = parameters->src[i];
		auto dst = parameters->dst[i];

		// generate an absolute indirect jmp
		auto lob = jmp(src_ptr, src, dst);

		auto code_lob = lob;

		// backup original code
		memcpy(ori_ptr, dst, lob);

		parameters->patch[i] = s_patch{
			.dst = dst,
			.src = src_ptr,
			.ori = ori_ptr,
			.size = lob,
		};

		// todo: analysis code
		//memcpy(code_ptr, dst, code_lob);

		parameters->original_function[i] = code_ptr;

		src_ptr += lob;
		ori_ptr += lob;
		code_ptr += code_lob;
	}
}

void patch_enable(s_patch* in, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		auto& patch = in[i];
		write_memory(patch.dst, patch.src, patch.size);
	}
}

void patch_disable(s_patch* in, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		auto& patch = in[i];
		write_memory(patch.dst, patch.ori, patch.size);
	}
}

#ifdef _WINDOWS
#include <Windows.h>

void write_memory(void* dst, void* src, size_t size) {
	DWORD old_protect;

	auto result = VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &old_protect);

	assert(("Failed to change protection", result));

	memcpy(dst, src, size);

	result = VirtualProtect(dst, size, old_protect, &old_protect);

	assert(("Failed to restore protection", result));
}
#else
static_assert(false, "Unsupported platform.");
#endif