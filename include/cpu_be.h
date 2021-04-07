#ifndef __CPU_BE_H__
#define __CPU_BE_H__

#define DRV_BIG_END             1
#define DRV_ENDIAN_TYPE         1

#ifdef __KERNEL__
#define DRV_CPU_TO_LE16(x)      __cpu_to_le16(x)
#define DRV_CPU_TO_LE32(x)      __cpu_to_le32(x)
#define DRV_LE16_TO_CPU(x)      __le16_to_cpu(x)
#define DRV_LE32_TO_CPU(x)      __le32_to_cpu(x)

#define DRV_CPU_TO_BE16(x)      __cpu_to_be16(x)
#define DRV_CPU_TO_BE32(x)      __cpu_to_be32(x)
#define DRV_BE16_TO_CPU(x)      __be16_to_cpu(x)
#define DRV_BE32_TO_CPU(x)      __be32_to_cpu(x)
#else
#define DRV_CPU_TO_LE16(x)      VA_SWAB16(x)
#define DRV_CPU_TO_LE32(x)      VA_SWAB32(x)

#define DRV_LE16_TO_CPU(x)      VA_SWAB16(x)
#define DRV_LE32_TO_CPU(x)      VA_SWAB32(x)

#define DRV_CPU_TO_BE16(x)      (x)
#define DRV_CPU_TO_BE32(x)      (x)
#define DRV_BE16_TO_CPU(x)      (x)
#define DRV_BE32_TO_CPU(x)      (x)

#endif /* __KERNEL__ */

#endif

