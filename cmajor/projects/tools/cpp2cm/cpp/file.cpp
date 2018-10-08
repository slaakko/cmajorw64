#include <file.hpp>
#define BAR 0
#define BAZ 0
#if FOO || BAZ
#pragma jee
#
print("foo");
#elif BAR
print("bar");
#else
print("baz");
#endif
