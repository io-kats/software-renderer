#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <cstdlib>

#define ERS_DISABLE_MACROS
#if defined(ERS_DISABLE_MACROS)
	#define ERS_DISABLE_WARNINGS
	#define ERS_DISABLE_ASSERT
	#define ERS_DISABLE_HERE
#endif

// 'ERS_PANIC' and 'ERS_PANICF' for error logging and exiting program normally.
// No macro provided to disable it, so that the code in EXPR executes, so use with care.
#define ERS_PANICF(EXPR, FMT, ...) \
do { \
	if(!(EXPR)) { \
		fprintf(stderr, "********************\n"); \
		fprintf(stderr, "[ERROR] Assertion failed.\n"); \
		fprintf(stderr, "Expression: \n\n%s\n\n", #EXPR); \
		fprintf(stderr, FMT, __VA_ARGS__); \
		fprintf(stderr, "\nFile: %s\n", __FILE__); \
		fprintf(stderr, "Line: %d\n", __LINE__); \
		fprintf(stderr, "errno: %s\n", (errno == 0 ? "None" : strerror(errno))); \
		fprintf(stderr, "********************\n"); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

#define ERS_PANIC(EXPR) \
do { \
	if(!(EXPR)) { \
		fprintf(stderr, "********************\n"); \
		fprintf(stderr, "[ERROR] Assertion failed.\n"); \
		fprintf(stderr, "Expression: \n\n%s\n\n", #EXPR); \
		fprintf(stderr, "File: %s\n", __FILE__); \
		fprintf(stderr, "Line: %d\n", __LINE__); \
		fprintf(stderr, "errno: %s\n", (errno == 0 ? "None" : strerror(errno))); \
		fprintf(stderr, "********************\n"); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

// ERS_WARN has ERS_ASSERT's functionality,
// but without halting the program or writing to stderr.
// Define the following macro to disable it:
//     #define ERS_DISABLE_WARNINGS
#if defined(ERS_DISABLE_WARNINGS)
	#define ERS_WARNF(EXPR, MESSAGE, ...) ((void)0)	
	#define ERS_WARN(EXPR) ((void)0)	
#else
	#define ERS_WARNF(EXPR, FMT, ...) \
	do { \
		if(!(EXPR)) { \
			printf("********************\n"); \
			printf("[ERROR] Assertion failed.\n"); \
			printf("Expression: \n\n%s\n\n", #EXPR); \
			printf(FMT, __VA_ARGS__); \
			printf("\nFile: %s\n", __FILE__); \
			printf("Line: %d\n", __LINE__); \
			printf("errno: %s\n", (errno == 0 ? "None" : strerror(errno))); \
			printf("********************\n"); \
		} \
	} while (0)

	#define ERS_WARN(EXPR) \
	do { \
		if(!(EXPR)) { \
			printf("********************\n"); \
			printf("[ERROR] Assertion failed.\n"); \
			printf("Expression: \n\n%s\n\n", #EXPR); \
			printf("File: %s\n", __FILE__); \
			printf("Line: %d\n", __LINE__); \
			printf("errno: %s\n", (errno == 0 ? "None" : strerror(errno))); \
			printf("********************\n"); \
		} \
	} while (0)
#endif

// ERS_ASSERTF for ERS_ASSERT's and printf's functionality.
// Disable it along with 'assert' or by defining the following macro at the top of this header file:
//     #define ERS_DISABLE_ASSERT
#if defined(ERS_DISABLE_ASSERT)
	#define ERS_ASSERTF(EXPR, FMT, ...) ERS_WARNF(EXPR, FMT, __VA_ARGS__)
	#define ERS_ASSERT(EXPR) ERS_WARN(EXPR)
#else
	#define ERS_ASSERTF(EXPR, FMT, ...) ERS_PANICF(EXPR, FMT, __VA_ARGS__)
	#define ERS_ASSERT(EXPR) ERS_PANIC(EXPR)
#endif

#ifndef ERS_PTR_SIZE
	#if defined(__WORDSIZE)
		#define ERS_PTR_SIZE ((__WORDSIZE) / 8)
	#elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) || defined(__arch64__) || defined(__aarch64__) || defined(__mips64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
		#define ERS_PTR_SIZE 8
	#elif defined(__CC_ARM) && (__sizeof_ptr == 8)
		#define ERS_PTR_SIZE 8
	#else
		#define ERS_PTR_SIZE 4
	#endif
#endif

#define ERS_UNUSED(var) (void)(var)

#if defined(ERS_DISABLE_HERE)
	#define ERS_HERE()
#else
	#define ERS_HERE() printf("Here: %s, %d, %s\n", __FILE__, __LINE__, __func__)
#endif

#define GL_CHECK() \
do { \
    GLenum ec = glGetError(); \
    if (ec != GL_NO_ERROR) \
    { \
		ERS_HERE(); \
        switch(ec) \
        { \
        case GL_INVALID_ENUM: \
            printf("GL_INVALID_ENUM\n"); \
            break; \
        case GL_INVALID_VALUE: \
            printf("GL_INVALID_VALUE\n"); \
            break; \
        case GL_INVALID_OPERATION: \
            printf("GL_INVALID_OPERATION\n"); \
            break; \
        case GL_OUT_OF_MEMORY: \
            printf("GL_OUT_OF_MEMORY\n"); \
            break; \
        case GL_INVALID_FRAMEBUFFER_OPERATION: \
            printf("GL_INVALID_FRAMEBUFFER_OPERATION\n"); \
            break; \
        case GL_NO_ERROR: \
        default:  \
            break; \
        } \
        assert(false); \
    }   \
} while (0)

#define ERS_EVAL_CSTR(val) printf("%s = %s\n", #val, (val))
#define ERS_EVAL_BOOL(val) printf("%s = %s\n", #val, (val) ? "true" : "false")
#define ERS_EVAL_F32(val) printf("%s = %.10g\n", #val, (val))
#define ERS_EVAL_F64(val) printf("%s = %.10g\n", #val, (val))
#define ERS_EVAL_S32(val) printf("%s = %d\n", #val, (val))
#define ERS_EVAL_S64(val) printf("%s = %lld\n", #val, (val))
#define ERS_EVAL_U32(val) printf("%s = %u\n", #val, (val))
#define ERS_EVAL_U64(val) printf("%s = %llu\n", #val, (val))
#define ERS_EVAL_SIZET(val) printf("%s = %zu\n", #val, (val))
#define ERS_EVAL_V2F32(val) printf("%s = { %f, %f }\n", #val, (val).e[0], (val).e[1])
#define ERS_EVAL_V3F32(val) printf("%s = { %f, %f, %f }\n", #val, (val).e[0], (val).e[1], (val).e[2])
#define ERS_EVAL_V4F32(val) printf("%s = { %f, %f, %f, %f }\n", #val, (val).e[0], (val).e[1], (val).e[2], (val).e[3])
#define ERS_EVAL_V3S32(val) printf("%s = { %d, %d, %d }\n", #val, (val).e[0], (val).e[1], (val).e[2])

#define ERS_UNREACHABLE() ERS_ASSERT(false && "Unreachable code.")
