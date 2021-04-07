#ifndef __VA_HASH__
#define __VA_HASH__

#define VA_LONG_BITS                    2
#define VA_LONG_SIZE                    4

#define VA_GOLDEN_RATIO_PRIME           0x9e370001UL  /* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */

static inline U32 VA_Hash32(U32 u32Val, U32 u32Bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    U32 u32Hash = u32Val * VA_GOLDEN_RATIO_PRIME;

    /* High bits are more random, so use them. */
    return u32Hash >> (32 - u32Bits);
}

static inline ULONG VA_HashPtr(VOID *pPtr, U32 u32Bits)
{
    return VA_Hash32(((ULONG)pPtr) >> VA_LONG_BITS, u32Bits);
}

static inline U32 VA_Hash64(U64 u64Val, U32 u32Bits)
{
    COMP_U64_U stVal;
    U32 u32Val;

    stVal.u64Val = u64Val;
    u32Val = stVal.u32Val0 + stVal.u32Val1;

    return VA_Hash32(u32Val, u32Bits);
}

#endif //__VA_HASH__
