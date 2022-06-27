//
// Created by 0x41c on 2022-06-26.
//

#ifndef JMI_HIERARCHY_H
#define JMI_HIERARCHY_H

#include <cstdint>

typedef unsigned int uint; // Clion thinks "uint" has a definition lol

typedef unsigned char u1;
typedef char s1;
typedef unsigned short u2;
typedef short s2;
typedef unsigned int u4;
typedef int s4;

typedef int jint;
#ifdef _LP64
typedef long jlong;
typedef unsigned long u8;
typedef long s8;
#else
typedef long long jlong;
typedef unsigned long long u8;
typedef long long s8;
#endif
typedef signed char jbyte;
typedef unsigned char jboolean;
typedef unsigned short jchar;
typedef short jshort;
typedef float jfloat;
typedef double jdouble;
typedef jint jsize;
typedef jchar jushort;
typedef unsigned long juint;


// Port over https://github.com/openjdk/jdk/blob/master/src/hotspot/share/oops/oopsHierarchy.hpp
// We don't need the fancy stuff CHECK_UNHANDLED_OOPS provides

enum class narrowOop: uint32_t { null = 0 };
typedef juint narrowKlass;
typedef void *OopOrNarrowOopStar;
typedef uintptr_t markWord;

// Oop hierarchy
typedef class oopDesc *oop;
    typedef class instanceOopDesc *instanceOop;
        typedef class stackChunkOopDesc *stackChunkOop;
    typedef class arrayOopDesc *arrayOop;
        typedef class objArrayOopDesc *objArrayOop;
        typedef class typeArrayOopDesc *typeArrayOop;


// A pointer to a pointer to a pointer lmao
typedef oop* OopHandle;

template <typename T>
class Array;

// CHeapObj specific
enum class MEMFLAGS {
  mtJavaHeap,
  mtClass,
  mtThread,
  mtThreadStack,
  mtCode,
  mtGC,
  mtGCCardSet,
  mtCompiler,
  mtJVMCI,
  mtInternal,
  mtOther,
  mtSymbol,
  mtNMT,
  mtClassShared,
  mtChunk,
  mtTest,
  mtTracing,
  mtLogging,
  mtStatistics,
  mtArguments,
  mtModule,
  mtSafepoint,
  mtSynchronizer,
  mtServiceability,
  mtMetaspace,
  mtStringDedup,
  mtObjectMonitor,
  mtNone
};

// Metadata hierarchy
class MetaspaceObj;
    class Metadata;
        class MethodData;
        class ConstantPool;
    class ConstMethod;
    class ConstantPoolCache;
template <MEMFLAGS f> class CHeapObj;
    class CompiledICHolder;

// Klass hierarchy
class Klass;
    class InstanceKlass;
        class InstanceMirrorKlass;
        class InstanceClassLoaderKlass;
        class InstanceRefKlass;
        class InstanceStackChunkKlass;
    class ArrayKlass;
        class ObjArrayKlass;
        class TypeArrayKlass;


// Casting functions so generously provided for us
template <typename T> inline oop cast_to_oop(T value) {
    return (oopDesc*)value;
}
template <typename T> inline T cast_from_oop(oop o) {
    return (T)((oopDesc*)o);
}

// JNI specific types
// Yes, they are all basically typedefs of oopDesc**

typedef OopHandle jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jarray jobjectArray;

struct _jfieldID;
typedef struct _jfieldID *jfieldID;
struct _jmethodID;
typedef struct _jmethodID *jmethodID;



#endif //JMI_HIERARCHY_H
