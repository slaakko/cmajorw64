#ifndef __ZLIB_INTF_H__
#define __ZLIB_INTF_H__
#include <stdint.h>

#ifndef CMRT_IMPORT
#define ZLIB_API __declspec(dllexport)
#else
#define ZLIB_API
#endif

#if defined (__cplusplus)
extern "C" {
#endif

ZLIB_API int32_t zlib_init(int32_t mode, int32_t level, void** handle);
ZLIB_API void zlib_done(int32_t mode, void* handle);
ZLIB_API void zlib_set_input(void* inChunk, uint32_t inAvail, void* handle);
ZLIB_API int32_t zlib_deflate(void* outChunk, uint32_t outChunkSize, uint32_t* have, uint32_t* outAvail, void* handle, int32_t flush);
ZLIB_API int32_t zlib_inflate(void* outChunk, uint32_t outChunkSize, uint32_t * have, uint32_t * outAvail, uint32_t * inAvail, void* handle);
ZLIB_API const char* zlib_retval_str(int32_t retVal);

#if defined (__cplusplus)
}
#endif

#endif // __ZLIB_INTF_H__
