set(PROJECT_NAME pfc)

################################################################################
# Source groups
################################################################################
set(Doc
    "pfc-license.txt"
    "pfc-readme.txt"
)
source_group("Doc" FILES ${Doc})

set(Header_Files
    "alloc.h"
    "array.h"
    "audio_sample.h"
    "autoref.h"
    "avltree.h"
    "base64.h"
    "bigmem.h"
    "binary_search.h"
    "bit_array.h"
    "bit_array_impl.h"
    "bit_array_impl_part2.h"
    "bsearch.h"
    "bsearch_inline.h"
    "byte_order.h"
    "chain_list_v2.h"
    "charDownConvert.h"
    "cmd_thread.h"
    "com_ptr_t.h"
    "cpuid.h"
    "debug.h"
    "event.h"
    "filehandle.h"
    "fixed_map.h"
    "fpu.h"
    "guid.h"
    "instance_tracker_legacy.h"
    "int_types.h"
    "iterators.h"
    "killswitch.h"
    "list.h"
    "lockless.h"
    "map.h"
    "mem_block.h"
    "memalign.h"
    "nix-objects.h"
    "notifyList.h"
    "once.h"
    "order_helper.h"
    "other.h"
    "pathUtils.h"
    "pfc-fb2k-hooks.h"
    "pfc-lite.h"
    "pfc.h"
    "platform-objects.h"
    "pocket_char_ops.h"
    "pool.h"
    "pp-winapi.h"
    "primitives.h"
    "primitives_part2.h"
    "ptr_list.h"
    "ptrholder.h"
    "rcptr.h"
    "ref_counter.h"
    "SmartStrStr-table.h"
    "SmartStrStr-twoCharMappings.h"
    "SmartStrStr.h"
    "sort.h"
    "splitString.h"
    "splitString2.h"
    "stdsort.h"
    "string-compare.h"
    "string-conv-lite.h"
    "string-interface.h"
    "string-lite.h"
    "string-part.h"
    "string_base.h"
    "string_conv.h"
    "string_list.h"
    "string_simple.h"
    "suppress_fb2k_hooks.h"
    "syncd_storage.h"
    "synchro.h"
    "synchro_nix.h"
    "synchro_win.h"
    "targetver.h"
    "threads.h"
    "timers.h"
    "traits.h"
    "unicode-normalize.h"
    "wait_queue.h"
    "weakRef.h"
    "wildcard.h"
    "win-objects.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "audio_math.cpp"
    "audio_sample.cpp"
    "base64.cpp"
    "bigmem.cpp"
    "bit_array.cpp"
    "bsearch.cpp"
    "charDownConvert.cpp"
    "cpuid.cpp"
    "crashWithMessage.cpp"
    "filehandle.cpp"
    "guid.cpp"
#    "nix-objects.cpp"
    "other.cpp"
    "pathUtils.cpp"
#    "pfc-fb2k-hooks.cpp"
    "printf.cpp"
    "selftest.cpp"
    "SmartStrStr.cpp"
    "sort.cpp"
    "splitString2.cpp"
    "stdafx.cpp"
    "string-compare.cpp"
    "string-conv-lite.cpp"
    "string-lite.cpp"
    "string_base.cpp"
    "string_conv.cpp"
    "threads.cpp"
    "timers.cpp"
    "unicode-normalize.cpp"
    "utf8.cpp"
    "wildcard.cpp"
    "win-objects.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Doc}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pfc-lite.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE pfc)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebugDLL
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wtl"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /RTC1
        >
        $<$<CONFIG:Release>:
            /O2;
            /GF;
            /Oy;
            /d2notypeopt;
            /GS-
        >
        /permissive-;
        /MP;
        /std:c++17;
        /W3;
        /Zi;
        /fp:fast;
        /GR-;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(bsearch.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(guid.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(other.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(sort.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(stdafx.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/RTC1"
        ">"
    )
    source_file_compile_options(utf8.cpp ${FILE_CL_OPTIONS})
    target_link_options(${PROJECT_NAME} PRIVATE
        /NOLOGO
    )
endif()

