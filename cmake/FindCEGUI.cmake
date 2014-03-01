# - Locate CEGUI LIBRARIES
# This module defines
#  CEGUI_FOUND, if false, do not try to link to CEGUI
#  CEGUI_INCLUDE_DIR, where to find headers.
#  CEGUI_LIBRARIES, the LIBRARIES to link against
#  CEGUI_BINARY_REL - location of the main CEGUI binary (win32 non-static only, release)
#  CEGUI_BINARY_DBG - location of the main CEGUI binaries (win32 non-static only, debug)
#
#
#    Modules :
#  CEGUI_${COMPONENT}_FOUND - ${COMPONENT} is available
#  CEGUI_${COMPONENT}_INCLUDE_DIRS - additional include directories for ${COMPONENT}
#  CEGUI_${COMPONENT}_LIBRARIES - link these to use ${COMPONENT}
#  CEGUI_${COMPONENT}_BINARY_REL - location of the component binary (win32 non-static only, release)
#  CEGUI_${COMPONENT}_BINARY_DBG - location of the component binary (win32 non-static only, debug)
#
#   WindowsRenderer:
#      Falagard
#   Renderer:
#      Direct3D9Renderer Direct3D10Renderer Direct3D11Renderer IrrlichtRenderer NullRenderer OgreRenderer OpenGLRenderer
#   ImageCodec:
#      CoronaImageCodec DevILImageCodec FreeImageImageCodec SILLYImageCodec STBImageCodec TGAImageCodec
#   Parser:
#      ExpatParser LibXMLParser RapidXMLParser TinyXMLParser XercesParser)
#   Script:
#      LuaScriptModule
     
include(FindPkgMacros)
include(PreprocessorUtils)

# Print component specific variables
macro(PRINT_COMPONENT_INFO COMPONENT)
    message(
          "${COMPONENT}_FOUND : " ${CEGUI_${COMPONENT}_FOUND} "\n"
          "${COMPONENT}_INCLUDE_DIRS: " ${CEGUI_${COMPONENT}_INCLUDE_DIRS} "\n"
          "${COMPONENT}_LIBRARIES: " ${CEGUI_${COMPONENT}_LIBRARIES} "\n"
          "${COMPONENT}_BINARY_REL: " ${CEGUI_${COMPONENT}_BINARY_REL} "\n"
          "${COMPONENT}_BINARY_DBG: " ${CEGUI_${COMPONENT}_BINARY_DBG} "\n"
    )
endmacro(PRINT_COMPONENT_INFO)

findpkg_begin(CEGUI)
set(CEGUI_FIND_REQUIRED 1)
 
# Get path, convert backslashes as ${ENV_${var}}
getenv_path(CEGUI_HOME)
getenv_path(CEGUI_DIR)
getenv_path(CEGUI_ROOT)
getenv_path(PROGRAMFILES)
 
# Determine whether to search for a dynamic or static build
if (CEGUI_STATIC)
  set(CEGUI_LIB_SUFFIX "_Static")
else ()
  set(CEGUI_LIB_SUFFIX "")
endif ()
 
set(CEGUI_LIBRARY_NAMES "CEGUIBase${CEGUI_LIB_SUFFIX}")
get_debug_names(CEGUI_LIBRARY_NAMES)
 
# construct search paths from environmental hints and
# OS specific guesses
if (WIN32)
  set(CEGUI_PREFIX_GUESSES
    ${ENV_PROGRAMFILES}/cegui
    ${ENV_PROGRAMFILES}/CEGUI
    C:/CEGUI-SDK
    C:/lib/cegui
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CEGUI_HOME]
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CEGUI_DIR]
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CEGUI_ROOT]
  )
elseif (UNIX)
  set(CEGUI_PREFIX_GUESSES
    /opt/cegui
    /opt/CEGUI
    /usr
    /usr/local
    $ENV{HOME}/cegui
    $ENV{HOME}/CEGUI
  )
endif ()
 
set(CEGUI_PREFIX_PATH
   $ENV{CEGUI_HOME} $ENV{CEGUI_DIR} $ENV{CEGUI_ROOT}
   ${CEGUI_PREFIX_GUESSES}
)
 
create_search_paths(CEGUI)
 
# redo search if any of the environmental hints changed
set(CEGUI_WINDOWSRENDERER_COMPONENTS
   CoreWindowRendererSet
)
set(CEGUI_RENDERER_COMPONENTS
   Direct3D9Renderer Direct3D10Renderer Direct3D11Renderer IrrlichtRenderer NullRenderer OgreRenderer OpenGLRenderer
)
set(CEGUI_IMAGECODEC_COMPONENTS
   CoronaImageCodec DevILImageCodec FreeImageImageCodec SILLYImageCodec STBImageCodec TGAImageCodec
)
set(CEGUI_PARSER_COMPONENTS
   ExpatParser LibXMLParser RapidXMLParser TinyXMLParser XercesParser
)
set(CEGUI_SCRIPT_COMPONENTS
   LuaScriptModule
)
set(CEGUI_COMPONENTS ${CEGUI_WINDOWSRENDERER_COMPONENTS} ${CEGUI_RENDERER_COMPONENTS} ${CEGUI_IMAGECODEC_COMPONENTS} ${CEGUI_PARSER_COMPONENTS} ${CEGUI_SCRIPT_COMPONENTS})
 
set(CEGUI_RESET_VARS
  CEGUI_CONFIG_INCLUDE_DIR CEGUI_INCLUDE_DIR
  CEGUI_LIBRARY_REL CEGUI_LIBRARY_DBG
)
 
foreach (comp ${CEGUI_COMPONENTS})
  set(CEGUI_RESET_VARS ${CEGUI_RESET_VARS}
    CEGUI_${comp}_INCLUDE_DIR CEGUI_${comp}_LIBRARY_FWK
    CEGUI_${comp}_LIBRARY_DBG CEGUI_${comp}_LIBRARY_REL
  )
endforeach (comp)
set(CEGUI_PREFIX_WATCH ${CEGUI_PREFIX_PATH})
clear_if_changed(CEGUI_PREFIX_WATCH ${CEGUI_RESET_VARS})
 
# try to locate CEGUI via pkg-config
use_pkgconfig(CEGUI_PKGC "CEGUI${CEGUI_LIB_SUFFIX}")

#message("CEGUI_INC_SEARCH_PATH:" ${CEGUI_INC_SEARCH_PATH})
#message("CEGUI_FRAMEWORK_INCLUDES:" ${CEGUI_FRAMEWORK_INCLUDES})
#message("CEGUI_PKGC_INCLUDE_DIRS:" ${CEGUI_PKGC_INCLUDE_DIRS})
 
# locate CEGUI include files
find_path(CEGUI_CONFIG_INCLUDE_DIR NAMES Config.h
          HINTS ${CEGUI_INC_SEARCH_PATH} ${CEGUI_FRAMEWORK_INCLUDES} ${CEGUI_PKGC_INCLUDE_DIRS}
        PATH_SUFFIXES "" "CEGUI" "cegui-0/CEGUI")

find_path(CEGUI_INCLUDE_DIR NAMES CEGUI.h
          HINTS ${CEGUI_INC_SEARCH_PATH} ${CEGUI_FRAMEWORK_INCLUDES} ${CEGUI_PKGC_INCLUDE_DIRS}
        PATH_SUFFIXES "" "CEGUI" "cegui-0/CEGUI")
set(CEGUI_INCOMPATIBLE FALSE)   
 
if (CEGUI_INCLUDE_DIR)
  # determine CEGUI version
  file(READ ${CEGUI_INCLUDE_DIR}/Version.h CEGUI_TEMP_VERSION_CONTENT)
  get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_MAJOR CEGUI_VERSION_MAJOR)
  get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_MINOR CEGUI_VERSION_MINOR)
  get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_PATCH CEGUI_VERSION_PATCH)
  get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_NAME CEGUI_VERSION_NAME)
  set(CEGUI_VERSION "${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}.${CEGUI_VERSION_PATCH}")
  pkg_message(CEGUI "Found CEGUI headers ${CEGUI_VERSION_NAME} (${CEGUI_VERSION})")
  pkg_message(CEGUI "Include dir: " ${CEGUI_INCLUDE_DIR})
 
  #set(CEGUI_VERSION "0.7.9")
  if ((${CEGUI_VERSION} VERSION_GREATER "0.8.0") OR (${CEGUI_VERSION} VERSION_EQUAL "0.8.0"))
     set(CEGUI_LIBRARY_NAMES "CEGUIBase-${CEGUI_VERSION_MAJOR}${CEGUI_LIB_SUFFIX}")
  else()
   set(CEGUI_INCOMPATIBLE TRUE)
   message(FATAL_ERROR "CEGUI version 0.8.0 or greater required !")
  endif()
 
  # determine configuration settings
  set(CEGUI_CONFIG_HEADERS
    ${CEGUI_CONFIG_INCLUDE_DIR}/Config.h
  )
  set(CEGUI_CONFIG_HEADER NOTFOUND)
  foreach(CFG_FILE ${CEGUI_CONFIG_HEADERS})
    if (EXISTS ${CFG_FILE})
      set(CEGUI_CONFIG_HEADER ${CFG_FILE}) # take the first config header from headers
      break()
    endif()
  endforeach()

  #message("Config header:" ${CEGUI_CONFIG_HEADER})
 
  if (CEGUI_CONFIG_HEADER)
    file(READ ${CEGUI_CONFIG_HEADER} CEGUI_TEMP_CONFIG_CONTENT)
    has_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_STATIC_LIB CEGUI_CONFIG_STATIC)
    get_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_THREAD_SUPPORT CEGUI_CONFIG_THREADS)
    get_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_THREAD_PROVIDER CEGUI_CONFIG_THREAD_PROVIDER)
    get_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_NO_FREEIMAGE CEGUI_CONFIG_FREEIMAGE)
    if (CEGUI_CONFIG_STATIC AND CEGUI_STATIC)
    elseif (CEGUI_CONFIG_STATIC OR CEGUI_STATIC)
      pkg_message(CEGUI "Build type (static, dynamic) does not match the requested one.")
      set(CEGUI_INCOMPATIBLE TRUE)
    endif ()
  else ()
    pkg_message(OGRE "Could not determine Ogre build configuration.")
    set(CEGUI_INCOMPATIBLE TRUE)
  endif ()
else ()
  set(CEGUI_INCOMPATIBLE FALSE)
endif ()

#message("Library names: " ${CEGUI_LIBRARY_NAMES})
#message("CEGUI_LIB_SEARCH_PATH: " ${CEGUI_LIB_SEARCH_PATH})
 
find_library(CEGUI_LIBRARY_REL NAMES ${CEGUI_LIBRARY_NAMES}
             HINTS ${CEGUI_LIB_SEARCH_PATH} ${CEGUI_PKGC_LIBRARY_DIRS} ${CEGUI_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

find_library(CEGUI_LIBRARY_DBG NAMES ${CEGUI_LIBRARY_NAMES_DBG}
             HINTS ${CEGUI_LIB_SEARCH_PATH} ${CEGUI_PKGC_LIBRARY_DIRS} ${CEGUI_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "debug")

#message("CEGUI_LIBRARY_REL : " ${CEGUI_LIBRARY_REL})
#message("CEGUI_FOUND : " ${CEGUI_FOUND})
make_library_set(CEGUI_LIBRARY)
 
if (CEGUI_INCOMPATIBLE)
  set(CEGUI_LIBRARY "NOTFOUND")
endif ()

#message("CEGUI_INCOMPATIBLE : " ${CEGUI_INCOMPATIBLE})
#message("CEGUI_LIBRARY : " ${CEGUI_LIBRARY})
 
set(CEGUI_INCLUDE_DIR ${CEGUI_CONFIG_INCLUDE_DIR} ${CEGUI_INCLUDE_DIR})
list(REMOVE_DUPLICATES CEGUI_INCLUDE_DIR)
findpkg_finish(CEGUI)
add_parent_dir(CEGUI_INCLUDE_DIRS CEGUI_INCLUDE_DIR)
 
mark_as_advanced(CEGUI_CONFIG_INCLUDE_DIR)
 
if (NOT CEGUI_FOUND)
  return()
endif ()
     
get_filename_component(CEGUI_LIBRARY_DIR_REL "${CEGUI_LIBRARY_REL}" PATH)
get_filename_component(CEGUI_LIBRARY_DIR_DBG "${CEGUI_LIBRARY_DBG}" PATH)
set(CEGUI_LIBRARY_DIRS ${CEGUI_LIBRARY_DIR_REL} ${CEGUI_LIBRARY_DIR_DBG})

#message("lib dirs0:" ${CEGUI_LIBRARY_DIRS})

# find binaries WIN32
if (NOT CEGUI_STATIC)
   #message("not static" ${CEGUI_BIN_SEARCH_PATH})
   if (WIN32)
      find_file(CEGUI_BINARY_REL NAMES "CEGUIBase-${CEGUI_VERSION_MAJOR}.dll"
                HINTS ${CEGUI_BIN_SEARCH_PATH}
                PATH_SUFFIXES "" release relwithdebinfo minsizerel)
      find_file(CEGUI_BINARY_DBG NAMES "CEGUIBase${CEGUI_VERSION_MAJOR}_d.dll"
                HINTS ${CEGUI_BIN_SEARCH_PATH}
                PATH_SUFFIXES "" debug )

      get_filename_component(CEGUI_BINARY_DIR_REL "${CEGUI_BINARY_REL}" PATH)
      get_filename_component(CEGUI_BINARY_DIR_DBG "${CEGUI_BINARY_DBG}" PATH)

      set(CEGUI_LIBRARY_DIRS ${CEGUI_BINARY_DIR_REL} ${CEGUI_BINARY_DIR_DBG})
     
   else () # UNIX and APPLE
      set(CEGUI_BINARY_REL NOTFOUND)
      set(CEGUI_BINARY_DBG NOTFOUND)
      set(CEGUI_BINARY_DIR_REL NOTFOUND)
      set(CEGUI_BINARY_DIR_DBG NOTFOUND)             
   endif()
      mark_as_advanced(CEGUI_BINARY_REL CEGUI_BINARY_DBG CEGUI_BINARY_DIR_REL CEGUI_BINARY_DIR_DBG)
endif()

#message("lib dirs1:" ${CEGUI_LIBRARY_DIRS})
 
# look for CoreWindowRendererSet component
findpkg_begin(CEGUI_CoreWindowRendererSet)
find_path(CEGUI_Falagard_INCLUDE_DIR NAMES NamedArea.h
          HINTS ${CEGUI_INCLUDE_DIRS}
          PATH_SUFFIXES falagard CEGUI/falagard)

find_path(CEGUI_CoreWindowRendererSet_INCLUDE_DIR NAMES Module.h
          HINTS ${CEGUI_INCLUDE_DIRS}
          PATH_SUFFIXES WindowRendererSets/Core CEGUI/WindowRendererSets/Core)

set(CEGUI_CoreWindowRendererSet_INCLUDE_DIR ${CEGUI_CoreWindowRendererSet_INCLUDE_DIR} ${CEGUI_Falagard_INCLUDE_DIR})
set(CEGUI_CoreWindowRendererSet_LIBRARY_NAMES "CEGUICoreWindowRendererSet${CEGUI_LIB_SUFFIX}")
get_debug_names(CEGUI_CoreWindowRendererSet_LIBRARY_NAMES)
find_library(CEGUI_CoreWindowRendererSet_LIBRARY_REL NAMES ${CEGUI_CoreWindowRendererSet_LIBRARY_NAMES}
             HINTS ${CEGUI_LIBRARY_DIR_REL} ${CEGUI_LIBRARY_DIR_REL}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
             PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

find_library(CEGUI_CoreWindowRendererSet_LIBRARY_DBG NAMES ${CEGUI_CoreWindowRendererSet_LIBRARY_NAMES_DBG}
             HINTS ${CEGUI_LIBRARY_DIR_DBG} ${CEGUI_LIBRARY_DIR_DBG}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
             PATH_SUFFIXES "" "debug")
make_library_set(CEGUI_CoreWindowRendererSet_LIBRARY)
findpkg_finish(CEGUI_CoreWindowRendererSet)

#PRINT_COMPONENT_INFO(CoreWindowRendererSet)
 
#Search renderer components
foreach (comp ${CEGUI_RENDERER_COMPONENTS})
    findpkg_begin(CEGUI_${comp})
    string(REPLACE "Renderer" "" compName ${comp})
    #message("compName:" ${compName})
    find_path(CEGUI_${comp}_INCLUDE_DIR NAMES Renderer.h
              HINTS ${CEGUI_INCLUDE_DIRS}
              PATH_SUFFIXES RendererModules/${compName} CEGUI/RendererModules/${compName})

    #message("comp incl dir:" ${CEGUI_${comp}_INCLUDE_DIR})
    set(CEGUI_${comp}_LIBRARY_NAMES "CEGUI${comp}-${CEGUI_VERSION_MAJOR}${CEGUI_LIB_SUFFIX}")
    get_debug_names(CEGUI_${comp}_LIBRARY_NAMES)

    #message("renderer stuff:" ${CEGUI_${comp}_LIBRARY_NAMES} "|" ${CEGUI_LIBRARY_DIR_REL})
    find_library(CEGUI_${comp}_LIBRARY_REL NAMES ${CEGUI_${comp}_LIBRARY_NAMES}
                 HINTS ${CEGUI_LIBRARY_DIR_REL}
                 PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

    find_library(CEGUI_${comp}_LIBRARY_DBG NAMES ${CEGUI_${comp}_LIBRARY_NAMES_DBG}
                 HINTS ${CEGUI_LIBRARY_DIR_DBG}
                 PATH_SUFFIXES "" "debug")

    make_library_set(CEGUI_${comp}_LIBRARY)
    findpkg_finish(CEGUI_${comp})
 
    #PRINT_COMPONENT_INFO(${comp})
endforeach (comp)

#Search image codec components
foreach (comp ${CEGUI_IMAGECODEC_COMPONENTS})
    findpkg_begin(CEGUI_${comp})
    find_path(CEGUI_${comp}_INCLUDE_DIR NAMES ImageCodec.h
              HINTS ${CEGUI_INCLUDE_DIRS}
              PATH_SUFFIXES ImageCodecModules/${comp} CEGUI/ImageCodecModules/${comp})

    set(CEGUI_${comp}_LIBRARY_NAMES "CEGUI${comp}${CEGUI_LIB_SUFFIX}")
    get_debug_names(CEGUI_${comp}_LIBRARY_NAMES)

    #message("image codec stuff: " ${CEGUI_${comp}_LIBRARY_NAMES} ${CEGUI_LIBRARY_DIR_REL})
    find_library(CEGUI_${comp}_LIBRARY_REL NAMES ${CEGUI_${comp}_LIBRARY_NAMES}
                 HINTS ${CEGUI_LIBRARY_DIR_REL} ${CEGUI_LIBRARY_DIR_REL}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
                 PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

    find_library(CEGUI_${comp}_LIBRARY_DBG NAMES ${CEGUI_${comp}_LIBRARY_NAMES_DBG}
                 HINTS ${CEGUI_LIBRARY_DIR_DBG} ${CEGUI_LIBRARY_DIR_DBG}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
                 PATH_SUFFIXES "" "debug")

    make_library_set(CEGUI_${comp}_LIBRARY)
    findpkg_finish(CEGUI_${comp})

    #PRINT_COMPONENT_INFO(${comp})
endforeach (comp)

#Search parser components
foreach (comp ${CEGUI_PARSER_COMPONENTS})
    findpkg_begin(CEGUI_${comp})
    find_path(CEGUI_${comp}_INCLUDE_DIR NAMES XMLParser.h
              HINTS ${CEGUI_INCLUDE_DIRS}
              PATH_SUFFIXES XMLParserModules/${comp} CEGUI/XMLParserModules/${comp} )
   
    set(CEGUI_${comp}_LIBRARY_NAMES "CEGUI${comp}${CEGUI_LIB_SUFFIX}")
    get_debug_names(CEGUI_${comp}_LIBRARY_NAMES)

    #message("parser stuff: " ${CEGUI_${comp}_LIBRARY_NAMES} )
    find_library(CEGUI_${comp}_LIBRARY_REL NAMES ${CEGUI_${comp}_LIBRARY_NAMES}
                 HINTS ${CEGUI_LIBRARY_DIR_REL} ${CEGUI_LIBRARY_DIR_REL}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
                 PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

    find_library(CEGUI_${comp}_LIBRARY_DBG NAMES ${CEGUI_${comp}_LIBRARY_NAMES_DBG}
                 HINTS ${CEGUI_LIBRARY_DIR_DBG} ${CEGUI_LIBRARY_DIR_DBG}/cegui-${CEGUI_VERSION_MAJOR}.${CEGUI_VERSION_MINOR}
                 PATH_SUFFIXES "" "debug")

    make_library_set(CEGUI_${comp}_LIBRARY)
    findpkg_finish(CEGUI_${comp})

    #PRINT_COMPONENT_INFO(${comp})
endforeach (comp)
     
foreach (comp ${CEGUI_SCRIPT_COMPONENTS})
    findpkg_begin(CEGUI_${comp})
    string(REPLACE "ScriptModule" "" compName ${comp})
   
#   message("comp: " ${comp})
#   message("compName: " ${compName})
   
    find_path(CEGUI_${comp}_INCLUDE_DIR NAMES ScriptModule.h
              HINTS ${CEGUI_INCLUDE_DIRS}
              PATH_SUFFIXES ScriptingModules/${compName} CEGUI/ScriptingModules/${compName})

    set(CEGUI_${comp}_LIBRARY_NAMES "CEGUI${comp}-${CEGUI_VERSION_MAJOR}${CEGUI_LIB_SUFFIX}")

    #message("script stuff: " ${CEGUI_${comp}_LIBRARY_NAMES} )
    get_debug_names(CEGUI_${comp}_LIBRARY_NAMES)
    find_library(CEGUI_${comp}_LIBRARY_REL NAMES ${CEGUI_${comp}_LIBRARY_NAMES}
                 HINTS ${CEGUI_LIBRARY_DIR_REL}
                 PATH_SUFFIXES "" "release" "relwithdebinfo" "minsizerel")

    find_library(CEGUI_${comp}_LIBRARY_DBG NAMES ${CEGUI_${comp}_LIBRARY_NAMES_DBG}
                 HINTS ${CEGUI_LIBRARY_DIR_DBG}
                 PATH_SUFFIXES "" "debug")

    make_library_set(CEGUI_${comp}_LIBRARY)
    findpkg_finish(CEGUI_${comp})

    #PRINT_COMPONENT_INFO(${comp})
endforeach (comp)
 
clear_if_changed(CEGUI_PREFIX_WATCH)

#stop the build to debug FindCEGUI.cmake
#message(FATAL_ERROR "Debug FindCEGUI")
