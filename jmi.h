//
// Created by 0x41c on 2022-06-26.
//

#ifndef JMI_H
#define JMI_H

#include <cstdint>
#include "jmi_hierarchy.h"


// Oop hierarchy
class oopDesc {
public:
    oopDesc() = delete;
    ~oopDesc() = delete;

    volatile markWord _mark;
    union _metadata {
        Klass *_klass;
        narrowKlass _compressed_klass;
    } _metadata;
};

class instanceOopDesc: public oopDesc {};
class arrayOopDesc: public oopDesc {};
class objArrayOopDesc: public arrayOopDesc {};
class typeArrayOopDesc: public arrayOopDesc {};

class stackChunkOopDesc: public instanceOopDesc {
public:
    static const uint8_t FLAG_HAS_INTERPRETED_FRAMES;
    static const uint8_t FLAG_CLAIM_RELATIVISE;
    static const uint8_t FLAG_NOTIFY_RELATIVISE;
    static const uint8_t FLAG_GC_MODE;
    static const uint8_t FLAG_HAS_BITMAP;
};

// Metadata hierarchy
class MetaspaceObj {
public:
    MetaspaceObj() = delete;
    ~MetaspaceObj() = delete;

    static void *_shared_metaspace_base;
    static void *_shared_metaspace_top;

    enum Type { // We don't need to create a macro here, that's lazy
        ClassType,
        SymbolType,
        TypeArrayU1Type,
        TypeArrayU2Type,
        TypeArrayU4Type,
        TypeArrayU8Type,
        TypeArrayOtherType,
        MethodType,
        ConstMethodType,
        MethodDataType,
        ConstantPoolType,
        ConstantPoolCacheType,
        AnnotationsType,
        MethodCountersType,
        RecordComponentType,
        _number_of_types
    };

};

template <typename T>
class Array: public MetaspaceObj {
public:
    int _length;
    T _data[1];
};

class ConstMethod: MetaspaceObj {
public:
    typedef enum { NORMAL, OVERPASS } MethodType;
    volatile uint64_t _fingerprint;
    ConstantPool *_constants;
    Array<u1> *_stackmap_data;
    int _constMethod_size;
    u2 _flags;
    u1 _result_size;
    u2 _code_size;
    u2 _name_index;
    u2 _signature_index;
    u2 _method_idnum;
    u2 _max_stack;
    u2 _max_locals;
    u2 _size_of_parameters;
    u2 _orig_id_num;
};

class ConstantPoolCache: public MetaspaceObj {
public:
    int _length;
    ConstantPool *_constant_pool;
    OopHandle _resolved_references;
    Array<u2> *_reference_map;
    int _archived_references_idx; // TODO: This may not be enabled, verify.
};

class Symbol: MetaspaceObj {
    volatile uint32_t _hash_and_ref;
    u2 length;
    u1 body[2];
    static Symbol *_vm_symbols[];
};

typedef Array<u1> AnnotationArray;

class Annotations: public MetaspaceObj {
public:
    AnnotationArray *_class_annotations;
    Array<AnnotationArray *> *_fields_annotations;
    AnnotationArray *_class_type_annotations;
    Array<AnnotationArray *> *_fields_type_annotations;
};

class RecordComponent: public MetaspaceObj {
public:
    AnnotationArray *_annotations;
    AnnotationArray *_type_annotations;
    u2 _name_index;
    u2 _descriptor_index;
    u2 _attributes_count;
};

// I really hope these virtual functions work and the VTable gets read properly
class Metadata: MetaspaceObj {
public:
    int identity_hash()                { return (int)(uintptr_t)this; }
    virtual bool is_metadata()           const;
    virtual bool is_klass()              const;
    virtual bool is_method()             const;
    virtual bool is_methodData()         const;
    virtual bool is_constantPool()       const;
    virtual bool is_methodCounters()     const;
    virtual int  size()                  const;
    virtual MetaspaceObj::Type type()    const;
    virtual const char* internal_name()  const;
};


class MethodData: public Metadata {
public:
    uintptr_t _method;
    int _size;
    int _hint_di;
    // TODO: Mutex
};

class AccessFlags {
public:
    jint _flags;
};

typedef u1 address;

class Method: public Metadata {
public:
    ConstMethod *_constMethod;
    uintptr_t _method_data;
    uintptr_t _method_counters;
    uintptr_t _adapter;
    AccessFlags _access_flags;
    int _vtable_index;
    u2 _intrinsic_id;
    mutable u2 _flags;
    address _i2i_entry;
    volatile address _from_compiled_entry;
    volatile uintptr_t _code;
    volatile address _from_interpreted_entry;
};

class ConstantPool: public Metadata {
public:
    Array<u1> *_tags;
    ConstantPoolCache *_cache;
    InstanceKlass *_pool_holder;
    Array<u2> *_operands;
    Array<Klass *> *_resolved_klasses;
    u2 _major_version;
    u2 _minor_version;
    u2 _generic_signature_idx;
    u2 _source_file_name_idx;
    u2 _flags;
    int _length;
    union {
        int _resolved_reference_length;
        int _version;
    } _saved;
};

template <MEMFLAGS f>
class CHeapObj {
public:
    CHeapObj() = delete;
    ~CHeapObj() = delete;
}; // Forward declare the forward declaration lmao
class CompiledICHolder: public CHeapObj<MEMFLAGS::mtCompiler> {}; // TODO: Define the structure

class JNIid: public CHeapObj<MEMFLAGS::mtClass> {
public:
    Klass *_holder;
    JNIid *_next;
    int _offset;
    bool _is_static_field_id; // FIXME: I don't think assert matters here
};

// Native method bucket.
class nmethodBucket: public CHeapObj<MEMFLAGS::mtClass> {
public:
    uintptr_t _nmethod;
    volatile int _count;
    volatile nmethodBucket *_next;
    volatile nmethodBucket *_purge_list_next;
};


// Klass hierarchy

enum KlassKind {
    InstanceKlassKind,
    InstanceRefKlassKind,
    InstanceMirrorKlassKind,
    InstanceClassLoaderKlassKind,
    InstanceStackChunkKlassKind,
    TypeArrayKlassKind,
    ObjArrayKlassKind
};

const uint KLASS_KIND_COUNT = ObjArrayKlassKind + 1;

typedef u8 traceid;

class Klass {
public:
    Klass() = delete;
    ~Klass() = delete;

    enum { _primary_super_limit = 8 };
    jint _layout_helper;
    const KlassKind _kind;
    jint _modifier_flags;
    juint _super_check_offset;
    Symbol *_name;
    Klass *_secondary_super_cache;
    Array<Klass *> *_secondary_supers;
    Klass *_primary_supers[_primary_super_limit];
    OopHandle _java_mirror;
    Klass *super;
    volatile Klass *_subklass;
    volatile Klass *_next_sibling;
    Klass *_next_link;
    uintptr_t _class_loader_data; // TODO: ClassLoaderData
    int _vtable_len;
    AccessFlags _access_flags;
    mutable traceid _trace_id;
    jshort _shared_class_path_idx;
    u2 _shared_class_flags;
    int _archived_mirror_idx;
};

class InstanceKlass: public Klass {
public:
    static const KlassKind Kind = InstanceKlassKind;
    enum ClassState : u1 {
        allocated,
        loaded,
        being_linked,
        linked,
        being_initialized,
        fully_initialized,
        initialization_error
    };
    Annotations *_annotations;
    uintptr_t _package_entry;
    volatile ObjArrayKlass *_array_klasses;
    ConstantPool *_constants;
    Array<jushort> *_inner_classes;
    Array<jushort> *_nest_members;
    InstanceKlass *_nest_host;
    Array<jushort> *_permitted_subclasses;
    Array<RecordComponent *> *_record_components;
    const char *_source_debug_extension;
    int _nonstatic_field_size;
    int _static_field_size;
    int _nonstatic_oop_map_size;
    int _itable_len;
    u2 _nest_host_index;
    u2 _this_class_index;
    u2 _static_oop_field_count;
    volatile u2 _idnum_allocated_count;
    bool _is_marked_dependant;
    ClassState _init_state;
    u2 _reference_type;

    enum { // Yes I copy and pasted this enum; Sue me.
        _misc_rewritten                           = 1 << 0,  // methods rewritten.
        _misc_has_nonstatic_fields                = 1 << 1,  // for sizing with UseCompressedOops
        _misc_should_verify_class                 = 1 << 2,  // allow caching of preverification
        _misc_unused                              = 1 << 3,  // not currently used
        _misc_is_contended                        = 1 << 4,  // marked with contended annotation
        _misc_has_nonstatic_concrete_methods      = 1 << 5,  // class/superclass/implemented interfaces has non-static, concrete methods
        _misc_declares_nonstatic_concrete_methods = 1 << 6,  // directly declares non-static, concrete methods
        _misc_has_been_redefined                  = 1 << 7,  // class has been redefined
        _misc_shared_loading_failed               = 1 << 8,  // class has been loaded from shared archive
        _misc_is_scratch_class                    = 1 << 9,  // class is the redefined scratch class
        _misc_is_shared_boot_class                = 1 << 10, // defining class loader is boot class loader
        _misc_is_shared_platform_class            = 1 << 11, // defining class loader is platform class loader
        _misc_is_shared_app_class                 = 1 << 12, // defining class loader is app class loader
        _misc_has_contended_annotations           = 1 << 13  // has @Contended annotation
    };

    u2 _misc_flags;
    uintptr_t _init_monitor;
    uintptr_t _init_thread;
    volatile uintptr_t _oop_map_cache;
    JNIid *_jni_ids;
    volatile jmethodID *_methods_jmethod_ids;
    volatile nmethodBucket *_dep_context;
    volatile uint64_t _dep_context_last_cleaned;
    uintptr_t _osr_nmethods_head;
    uintptr_t _breakpoints;
    InstanceKlass *_previous_versions;
    uintptr_t _cached_class_file;
    uintptr_t _jvmti_cached_class_field_map;
    Array<Method *> *_methods;
    Array<Method *> *_default_methods;
    Array<InstanceKlass *> *_local_interfaces;
    Array<InstanceKlass *> *_transitive_interfaces;
    Array<int> *_method_ordering;
    Array<int> *_default_vtable_indices;
    Array<u2> *fields;
    static bool _disable_method_binary_search;
    static bool _finalization_enabled;
};

class InstanceMirrorKlass: public InstanceKlass {
public:
    static int _offset_of_static_fields;
};

class InstanceClassLoaderKlass: public InstanceKlass {};
class InstanceRefKlass: public InstanceKlass {};
class InstanceStackChunkKlass: public InstanceKlass {
    static int _offset_of_stack;
};

class ArrayKlass: public Klass {
public:
    int _dimension;
    volatile Klass *_higher_dimension;
    volatile Klass *_lower_dimension;
};

class ObjArrayKlass: public ArrayKlass {
    Klass *_element_class;
    Klass *_bottom_class;
};

class TypeArrayKlass: public ArrayKlass {
public:
    jint _max_length;
};

#endif //JMI_H
