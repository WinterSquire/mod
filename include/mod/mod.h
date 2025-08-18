#pragma once

#include <cassert>
#include <cstring>

#include "assembly.h"

struct s_patch {
	void* dst;
	void* src;
	void* ori;
	size_t size;
};

struct s_patch_create_parameters {
	size_t size;
	// IN
	void** dst;
	void** src;
	size_t* lob;
	// OUT
	s_patch* patch;
	char* src_buffer;
	char* ori_buffer;
};

struct s_patch_vftable_create_parameters {
	size_t size;
	// IN
	void** dst;
	void** src;
	void** ori;
	size_t* lob;
	// OUT
	s_patch* patch;
};

struct s_patch_detour_create_parameters {
	size_t size;
	// IN
	void** dst;
	void** src;
	// OUT
	char* src_buffer;
	char* ori_buffer;		
	char* code_buffer;
	s_patch* patch;	
	void** original_function;
};

void patch_create(const s_patch_create_parameters* parameters);
void patch_vftable_create(const s_patch_vftable_create_parameters* parameters);
void patch_detour_create(const s_patch_detour_create_parameters* parameters);

void patch_enable(s_patch* in, size_t size);
void patch_disable(s_patch* in, size_t size);