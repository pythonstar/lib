#pragma once

//
// �����Դ��뼰���ֺ�
//
// ע��ܶ��ظ��ĵط���Ҫ���ȣ�����Ϊ
// �������ú���һ��ֵ�������ſ��Զ����Ψһ�ģ�
// ��ǰ��MFC�п��������Ƶ��÷������Ծ���������������
// 0xE8
//
// Apr 21th, 2009	- liusiyang
//

// ���������_ANITIDEBUG�����ɶҲ����
#ifdef _ANTIDEBUG
	// ..
	#define JUNK_CODE_ONE								\
		__asm {push eax}								\
		__asm {xor eax, eax}							\
		__asm {setpo al}								\
		__asm {push edx}								\
		__asm {xor edx, eax}							\
		__asm {sal edx, 2}								\
		__asm {xchg eax, edx}							\
		__asm {pop edx}									\
		__asm {or eax, ecx}								\
		__asm {pop eax}

	// ..
	#define JUNK_CODE_TWO_2(lineno, value)				\
		__asm {jz _1##lineno}							\
		__asm {jnz _1##lineno}							\
		__asm {_emit 0x##value}							\
		__asm {_1##lineno: }
	#define JUNK_CODE_TWO_1(name, value) JUNK_CODE_TWO_2(name, value)
	#define JUNK_CODE_TWO JUNK_CODE_TWO_1(__LINE__, __LINE__*1111%253)

	// ..
	#define JUNK_CODE_TWO_2_2(lineno)					\
		__asm {jz _112##lineno}							\
		__asm {jnz _112##lineno}						\
		__asm {_emit 0e8h}								\
		__asm {_112##lineno: }
	#define JUNK_CODE_TWO_1_2(name) JUNK_CODE_TWO_2_2(name)
	#define JUNK_CODE_TWO2 JUNK_CODE_TWO_1_2(__LINE__)

	// ..
	#define JUNK_CODE_TWO_2_3(lineno)					\
		__asm { xor eax, eax }							\
		__asm { test eax, eax }							\
		__asm {jz _1121##lineno}						\
		__asm {jnz _1120##lineno}						\
		__asm {_1120##lineno: }							\
		__asm {_emit 0e8h}								\
		__asm {_1121##lineno: }							\
		__asm { xor eax, 3 }							\
		__asm { add eax, 4 }							\
		__asm { xor eax, 5 }							
	#define JUNK_CODE_TWO_1_3(name) JUNK_CODE_TWO_2_3(name)
	#define JUNK_CODE_TWO3 JUNK_CODE_TWO_1_3(__LINE__)


	// ..
	#define JUNK_CODE_THREE_2(lineno, value1, value2)	\
		__asm {clc}										\
		__asm {jnb _3t##lineno}							\
		__asm {_emit 0x##value1}						\
		__asm {_emit 0x##value2}						\
		__asm {_3t##lineno: }
	#define JUNK_CODE_THREE_1(name, value1, value2) JUNK_CODE_THREE_2(name, value1, value2)
	#define JUNK_CODE_THREE JUNK_CODE_THREE_1(__LINE__, __LINE__*1222%253, __LINE__*1111%253)

	// ..
	#define JUNK_CODE_FOUR_2(lineno, value)				\
		__asm {jl _11f##lineno}							\
		__asm {_12f##lineno: }							\
		__asm {jmp _13f##lineno }						\
		__asm {_emit 0x##value }						\
		__asm {_11f##lineno: }							\
		__asm {jz _12f##lineno }						\
		__asm {_13f##lineno: }
	#define JUNK_CODE_FOUR_1(name, value) JUNK_CODE_FOUR_2(name, value)
	#define JUNK_CODE_FOUR JUNK_CODE_FOUR_1(__LINE__, __LINE__*1111%253)


	// ..
	#define JUNK_CODE_FIVE_2(lineno)					\
		__asm {pushf}									\
		__asm {push 0x0a}								\
		__asm {_51f##lineno: jnb _53f##lineno}			\
		__asm {jmp _52f##lineno}						\
		__asm {_52f##lineno: call _54f##lineno}			\
		__asm {_53f##lineno: jnb _52f##lineno}			\
		__asm {_54f##lineno: add esp,4}					\
		__asm {jmp _55f##lineno}						\
		__asm {_55f##lineno: }							\
		__asm {dec dword ptr [esp]}						\
		__asm {jno _56f##lineno}						\
		__asm {_56f##lineno: jns _51f##lineno}			\
		__asm {jp _57f##lineno}							\
		__asm {_57f##lineno: add esp,4}					\
		__asm {popf}									\
		__asm {jmp _58f##lineno}						\
		__asm {_58f##lineno: }
	#define JUNK_CODE_FIVE_1(name) JUNK_CODE_FIVE_2(name)
	#define JUNK_CODE_FIVE JUNK_CODE_FIVE_1(__LINE__)

#else
	#define JUNK_CODE_ONE
	#define JUNK_CODE_TWO
	#define JUNK_CODE_TWO2
	#define JUNK_CODE_TWO3
	#define JUNK_CODE_THREE
	#define JUNK_CODE_FOUR
	#define JUNK_CODE_FIVE
#endif


// ��ʼ�����Լ��
bool start_anti_debug();

// ֹͣ�����Լ��
bool stop_anti_debug();

// �Ƿ��ڵ�����
bool is_under_debug();

// �����Ƿ��ڵ����У����⿪һ�����Է�ʽ
bool test_under_debug();
