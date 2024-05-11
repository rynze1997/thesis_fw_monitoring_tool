
#if defined(DISABLE_CRITICAL_REGIONS) || defined(UNITTEST)

#define ENTER_CRITICAL(_was_masked) do {\
} while(0)
#define EXIT_CRITICAL(_was_masked) do {\
} while(0)

#else


#define ENTER_CRITICAL(_was_masked) do {\
    __asm volatile ("MRS %0, primask" : "=r" (_was_masked) );\
    __asm volatile ("cpsid i" : : : "memory");\
} while(0)

#define EXIT_CRITICAL(_was_masked) do {\
    if (!_was_masked){\
        __asm volatile ("cpsie i" : : : "memory");\
    }\
} while(0)


#endif

#define CRITICAL_REGION(_code){     \
    uint32_t _was_masked;           \
    ENTER_CRITICAL(_was_masked);    \
    {_code}                         \
    EXIT_CRITICAL(_was_masked);     \
}
