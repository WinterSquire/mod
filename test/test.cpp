#include <mod/mod.h>
#include <gtest/gtest.h>
#include <cassert>
#include <cstring>

struct s_invoke_test_instance {
	char LOB;
	char AOB[31];
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

	// call create
	for (int i = 0; i < _countof(g_call_test_instances); ++i) {
		auto instance = g_call_test_instances + i;
		ASSERT_EQ(call(buffer, instance->RIP, instance->DST), instance->LOB);
		ASSERT_EQ(0, memcmp(buffer, instance->AOB, instance->LOB));
	}

	// todo: call convert(rel32 -> m64)
}

TEST(assembly, jmp_test) {
	char buffer[k_maximum_instruction_size + sizeof(void*)];

	// jmp create
	for (int i = 0; i < _countof(g_jmp_test_instances); ++i) {
		auto instance = g_jmp_test_instances + i;
		ASSERT_EQ(jmp(buffer, instance->RIP, instance->DST), instance->LOB);
		ASSERT_EQ(0, memcmp(buffer, instance->AOB, instance->LOB));
	}

	// todo: jmp convert(rel8 -> m64 | rel32 -> m64)
}

TEST(assembly, m64_test) {
	// todo: m64 copy
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

int get_value0() { return 0; }
int get_value1() { return 1; }

TEST(detour, test) {
#if 0
	void* dst[] = { get_value0 };
	void* src[] = { get_value1 };
	size_t lob[] = { 1 * sizeof(void*) };
	char src_buffer[32];
	char ori_buffer[32];
	s_patch patch[1];
	void* original_function[1];

	s_patch_detour_create_parameters parameters{
		.size = 1,
		.dst = dst,
		.src = src,
		.src_buffer = src_buffer,
		.ori_buffer = ori_buffer,
		.code_buffer = nullptr,
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
#endif
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
#endif

int main(int argc, char** argv) {
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}