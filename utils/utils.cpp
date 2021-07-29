#include "../utils/utils.h"
#include "../utils/result.h"

#ifdef ARDUINO
// TODO(erd): EW
namespace std {
  void __throw_bad_alloc()
  {
    debugPrint("Unable to allocate memory");
    exit(1);
  }

  void __throw_length_error( char const*e )
  {
    debugPrint("Length Error :");
    debugPrint(e);
    exit(1);
  }
}
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
#else
int freeMemory() {
    return 0;
}
#endif
