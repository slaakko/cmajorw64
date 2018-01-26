#ifndef __BZ2_INTF_H__
#define __BZ2_INTF_H__
#include <stdint.h>

#ifndef CMRT_IMPORT
#define BZLIB_API __declspec(dllexport)
#else
#define BZLIB_API
#endif

#if defined (__cplusplus)
extern "C" {
#endif

BZLIB_API int32_t bz2_init(int32_t mode, int32_t compressionLevel, int32_t compressionWorkFactor, void** handle);
BZLIB_API void bz2_done(int32_t mode, void* handle);
BZLIB_API void bz2_set_input(void* inChunk, uint32_t inAvail, void* handle);
BZLIB_API int32_t bz2_compress(void* outChunk, uint32_t outChunkSize, uint32_t* have, uint32_t* outAvail, void* handle, int32_t action);
BZLIB_API int32_t bz2_decompress(void* outChunk, uint32_t outChunkSize, uint32_t* have, uint32_t* outAvail, uint32_t* inAvail, void* handle);
BZLIB_API const char* bz2_retval_str(int32_t retVal);

#if defined (__cplusplus)
}
#endif

#endif // __BZ2_INTF_H__
