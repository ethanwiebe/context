#pragma once

#include <memory>
#include <stdint.h>

typedef uint8_t     u8;
typedef int8_t      s8;
typedef uint16_t    u16;
typedef int16_t     s16;
typedef uint32_t    u32;
typedef int32_t     s32;
typedef uint64_t    u64;
typedef int64_t     s64;

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T,typename ...Args>
constexpr Ref<T> MakeRef(Args&& ...args){
	return std::make_shared<T>(std::forward<Args>(args)...);
}


template <typename T>
using Handle = std::unique_ptr<T>;

template <typename T,typename ...Args>
constexpr Handle<T> MakeHandle(Args&& ...args){
	return std::make_unique<T>(std::forward<Args>(args)...);
}
