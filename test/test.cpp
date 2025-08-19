#include <mod/mod.h>
#include <gtest/gtest.h>
#include <cassert>
#include <cstring>

struct s_region {
	void* base;
	size_t size;
	size_t page_size;
	int number_of_code_page;
	int number_of_data_page;
	size_t page_used[16]; // page = code_page + data_page
};

static s_region g_region;

struct s_invoke_test_instance {
	size_t LOB;
	const char* AOB;
	void* RIP;
	void* DST;
};

s_invoke_test_instance g_jmp_test_instances[]{ {
	.LOB = 2, .AOB = "\xEB\x20",
	.RIP = (void*)0x180021178, .DST = (void*)0x18002119A
}, {
	.LOB = 5, .AOB = "\xE9\x60\xFF\xFF\xFF",
	.RIP = (void*)0x1800214AA, .DST = (void*)0x18002140F
}, {
	.LOB = 2, .AOB = "\xEB\x20",
	.RIP = (void*)0x180021178, .DST = (void*)0x18002119A
} };

s_invoke_test_instance g_call_test_instances[]{ {
	.LOB = 16, .AOB = "\xFF\x15\x02\x00\x00\x00\xEB\x08\x59\x56\x14\x2D\xFE\x7F\x00\x00",
	.RIP = (void*)0x18002144D, .DST = (void*)0x7FFE2D145659
}, {
	.LOB = 5, .AOB = "\xE8\x93\x78\x03\x00",
	.RIP = (void*)0x180021460, .DST = (void*)0x180058CF8
} };

TEST(assembly, call_test) {
	char buffer[k_maximum_instruction_size + sizeof(void*)];

	for (int i = 0; i < _countof(g_call_test_instances); ++i) {
		auto instance = g_call_test_instances + i;
		ASSERT_EQ(call(buffer, instance->RIP, instance->DST), instance->LOB);
		ASSERT_EQ(0, memcmp(buffer, instance->AOB, instance->LOB));
	}
}

TEST(assembly, jmp_test) {
	char buffer[k_maximum_instruction_size + sizeof(void*)];

	for (int i = 0; i < _countof(g_jmp_test_instances); ++i) {
		auto instance = g_jmp_test_instances + i;
		ASSERT_EQ(jmp(buffer, instance->RIP, instance->DST), instance->LOB);
		ASSERT_EQ(0, memcmp(buffer, instance->AOB, instance->LOB));
	}
}

struct s_unassemble_test_instance {
	void* RIP;
	size_t LIN;
	const char* IN;
	size_t LOUT;
	const char* OUT;
};

s_unassemble_test_instance g_unassemble_test_instances[] { 
{ // simple parse copy test
	.RIP = (void*)0x1800A78C0,
	.LIN = 32,
	.IN = "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\xC8\xFB\xFF\xFF\x48\x81\xEC\x38\x05\x00\x00\xFF\x15\xF6\x2B",
	.LOUT = 36,
	.OUT = "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\xC8\xFB\xFF\xFF\xFF\x25\x00\x00\x00\x00\xD5\x78\x0A\x80\x01\x00\x00\x00\xCC"
},
#if 0
{ // todo: call jz test
	.RIP = (void*)0x1800F01A0,
	.LIN = 32,
	.IN = "\x48\x83\xEC\x28\xE8\x3B\xFD\xFF\xFF\x32\xD2\x84\xC0\x74\x27\x8B\x0D\xE7\x9D\x94\x00\x65\x48\x8B\x04\x25\x58\x00\x00\x00\x41\xB8",
	.LOUT = 0,
	.OUT = ""
},
#endif
};

TEST(assembly, unassemble_test) {
	char buffer[256];
	
	for (int i = 0; i < _countof(g_unassemble_test_instances); ++i) {
		auto instance = g_unassemble_test_instances + i;
		ASSERT_EQ(unassemble(buffer, instance->IN, instance->RIP, 14), instance->LOUT);
		ASSERT_EQ(0, memcmp(buffer, instance->OUT, instance->LOUT));
	}
}

TEST(patch, test) {
	static int test_value = 1;
	int target_value = -1;

	void* dst[] = { &test_value };
	void* src[] = { &target_value };
	size_t lob[] = { sizeof(int) };
	s_patch patch[1];
	char ori_buffer[sizeof(int)];
	char src_buffer[sizeof(int)];

	s_patch_create_parameters parameters{
		.size = 1,
		.dst = dst,
		.src = src,
		.lob = lob,
		.patch = patch,
		.src_buffer = src_buffer,
		.ori_buffer = ori_buffer,
	};

	patch_create(&parameters);
	ASSERT_EQ(1, test_value);
	ASSERT_EQ(-1, target_value);
	ASSERT_EQ(1, *reinterpret_cast<int*>(ori_buffer));
	ASSERT_EQ(-1, *reinterpret_cast<int*>(src_buffer));

	patch_enable(patch, 1);
	ASSERT_EQ(-1, test_value);
	ASSERT_EQ(-1, target_value);
	ASSERT_EQ(1, *reinterpret_cast<int*>(ori_buffer));
	ASSERT_EQ(-1, *reinterpret_cast<int*>(src_buffer));

	patch_disable(patch, 1);
	ASSERT_EQ(1, test_value);
	ASSERT_EQ(-1, target_value);
	ASSERT_EQ(1, *reinterpret_cast<int*>(ori_buffer));
	ASSERT_EQ(-1, *reinterpret_cast<int*>(src_buffer));
}

int (*get_value_original)();

// make it static so the linker won't use ILT
static int get_value0() {
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	return 0;
}

static int get_value1() {
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	_mm_setzero_si128();
	return 1;
}

TEST(detour, test) {
	void* dst[] = { get_value0 };
	void* src[] = { get_value1 };
	size_t lob[] = { 1 * sizeof(void*) };
	char src_buffer[32];
	char ori_buffer[32];
	s_patch patch[1];
	void* original_function[1];
	auto code_buffer = reinterpret_cast<char*>(g_region.base);

	s_patch_detour_create_parameters parameters{
		.size = 1,
		.dst = dst,
		.src = src,
		.src_buffer = src_buffer,
		.ori_buffer = ori_buffer,
		.code_buffer = code_buffer,
		.patch = patch,
		.original_function = original_function
	};

	ASSERT_EQ(0, get_value0());
	ASSERT_EQ(1, get_value1());

	patch_detour_create(&parameters);
	get_value_original = reinterpret_cast<decltype(get_value_original)>(original_function[0]);
	ASSERT_EQ(0, get_value_original());
	ASSERT_EQ(0, get_value0());
	ASSERT_EQ(1, get_value1());

	patch_enable(patch, 1);
	ASSERT_EQ(0, get_value_original());
	ASSERT_EQ(1, get_value0());
	ASSERT_EQ(1, get_value1());

	patch_disable(patch, 1);
	ASSERT_EQ(0, get_value_original());
	ASSERT_EQ(0, get_value0());
	ASSERT_EQ(1, get_value1());
}

class i_person { public: virtual int get() = 0; };
class c_person : public i_person {public: virtual int get() { return 0; }};
class c_student : public i_person {public: virtual int get() { return 1; }};
class c_dummy : public i_person {public: virtual int get() { return 2; }};

TEST(vftable, test) {
	c_person person;
	c_student student;
	c_dummy dummy;
	
	void* dst[] = { *reinterpret_cast<void**>(&person) };
	void* src[] = { *reinterpret_cast<void**>(&student) };
	void* ori[] = { *reinterpret_cast<void**>(&dummy) };
	size_t lob[] = { 1 * sizeof(void*) };
	s_patch patch[1];

	s_patch_vftable_create_parameters parameters{
		.size = 1,
		.dst = dst,
		.src = src,
		.ori = ori,
		.lob = lob,
		.patch = patch
	};

	i_person* _person = &person;
	i_person* _student = &student;
	i_person* _dummy = &dummy;

	patch_vftable_create(&parameters);
	ASSERT_EQ(0, _person->get());
	ASSERT_EQ(1, _student->get());
	ASSERT_EQ(0, _dummy->get());

	patch_enable(patch, 1);
	ASSERT_EQ(1, _person->get());
	ASSERT_EQ(1, _student->get());
	ASSERT_EQ(0, _dummy->get());

	patch_disable(patch, 1);
	ASSERT_EQ(0, _person->get());
	ASSERT_EQ(1, _student->get());
	ASSERT_EQ(0, _dummy->get());
}

#ifdef _WINDOWS
#include <Windows.h>
#include <crtdbg.h>

SYSTEM_INFO g_system_info;

void initialize_region(s_region* region) {
	region->size = g_system_info.dwAllocationGranularity;
	region->page_size = g_system_info.dwPageSize;
	region->base = VirtualAlloc(NULL, region->size, MEM_RESERVE, PAGE_NOACCESS);
	region->number_of_code_page = 1;
	region->number_of_data_page = 1;

	assert(region->base != nullptr);

	auto page_address = reinterpret_cast<char*>(region->base);
	auto page_count = region->size / region->page_size;

	for (int i = 0; i < page_count; ++i) {
		auto page_type = PAGE_NOACCESS;

		if (i < region->number_of_code_page) {
			page_type = PAGE_EXECUTE_READWRITE;
		} else if (i < region->number_of_code_page + region->number_of_data_page) {
			page_type = PAGE_READWRITE;
		} else {
			break;
		}

		VirtualAlloc(page_address, region->page_size, MEM_COMMIT, page_type);
		page_address += region->page_size;
	}
}

void free_region(s_region* region) {
	VirtualFree(region->base, NULL, MEM_FREE);
}

int main(int argc, char** argv) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	GetSystemInfo(&g_system_info);

	GTEST_LOG_(INFO) 
		<< std::hex
		<< "System Info\n" 
		<< "Allocation Granularity: 0x" << g_system_info.dwAllocationGranularity << '\n'
		<< "Page Size: 0x" << g_system_info.dwPageSize << '\n'
		<< std::endl;

	initialize_region(&g_region);

	::testing::InitGoogleTest(&argc, argv);

	auto result = RUN_ALL_TESTS();

	free_region(&g_region); // we could also don't free the region

	return result;
}
#endif