set(PROJECT_NAME libPPUI)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "libPPUI-license.txt"
    "libPPUI-readme.txt"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "AutoComplete.h"
    "CButtonLite.h"
    "CDialogResizeHelper.h"
    "CDialogResizeHelperCompat.h"
    "CEditWithButtons.h"
    "CEnumString.h"
    "CFlashWindow.h"
    "CHeaderCtrlEx.h"
    "CIconOverlayWindow.h"
    "clipboard.h"
    "CListAccessible.h"
    "CListControl-Cell.h"
    "CListControl-Cells-Compat.h"
    "CListControl-Cells.h"
    "CListControl-Subst.h"
    "CListControl.h"
    "CListControl_EditImpl.h"
    "CListControlComplete.h"
    "CListControlHeaderImpl.h"
    "CListControlOwnerData.h"
    "CListControlSimple.h"
    "CListControlTruncationTooltipImpl.h"
    "CListControlUserOptions.h"
    "CListControlWithSelection.h"
    "CListViewCtrlEx.h"
    "CMiddleDragImpl.h"
    "CMiddleDragLite.h"
    "CMiddleDragOverlay.h"
    "commandline_parser.h"
    "Controls.h"
    "CPopupTooltipMessage.h"
    "CPowerRequest.h"
    "CPropVariant.h"
    "CWindowCreateAndDelete.h"
    "DarkMode-CHyperLink.h"
    "DarkMode.h"
    "DarkModeEx.h"
    "gdi-types-portable.h"
    "gdiplus-helpers-webp.h"
    "gdiplus_helpers.h"
    "GDIUtils.h"
    "gesture.h"
    "hookWindowMessages.h"
    "IDataObjectUtils.h"
    "ImageEncoder.h"
    "ImplementOnFinalMessage.h"
    "InPlaceEdit.h"
    "InPlaceEditTable.h"
    "link-CommonControls6.h"
    "listview_helper.h"
    "PaintUtils.h"
    "pp-COM-macros.h"
    "ReStyleWnd.h"
    "SmartStrStr.h"
    "stdafx.h"
    "targetver.h"
    "TreeMultiSel.h"
    "TypeFind.h"
    "win32_op.h"
    "win32_utility.h"
    "windowLifetime.h"
    "wtl-pp.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "AutoComplete.cpp"
    "CDialogResizeHelper.cpp"
    "CEditWithButtons.cpp"
    "clipboard.cpp"
    "CListAccessible.cpp"
    "CListControl-Cells.cpp"
    "CListControl-Subst.cpp"
    "CListControl.cpp"
    "CListControlHeaderImpl.cpp"
    "CListControlTruncationTooltipImpl.cpp"
    "CListControlWithSelection.cpp"
    "CMiddleDragImpl.cpp"
    "commandline_parser.cpp"
    "Controls.cpp"
    "CPowerRequest.cpp"
    "DarkMode.cpp"
    "gdiplus_helpers.cpp"
    "GDIUtils.cpp"
    "IDataObjectUtils.cpp"
    "ImageEncoder.cpp"
    "InPlaceCombo.cpp"
    "InPlaceEdit.cpp"
    "InPlaceEditTable.cpp"
    "listview_helper.cpp"
    "PaintUtils.cpp"
    "stdafx.cpp"
    "TypeFind.cpp"
    "win32_op.cpp"
    "win32_utility.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE libPPUI)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/..;"
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
            /Zi
        >
        $<$<CONFIG:Release>:
            /O1;
            /Oi;
            /Oy;
            /Gy;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /d2notypeopt
        >
        /permissive-;
        /MP;
        /std:c++17;
        /W3;
        /fp:fast;
        /GR-;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /GS-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

