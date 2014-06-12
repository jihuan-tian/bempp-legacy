# Needs boost: include this after boost.lookup.
if(TARGET Boost)
  set(depends_on Boost)
endif()
set(default_common_URL
    http://www.dune-project.org/download/2.2.1/dune-common-2.2.1.tar.gz)
set(default_common_SHA256 6b16b2390af79e2ca93367c95d06ed536b58422034cf34e4437fc6201bb1ab85)
set(default_geometry_URL
    http://www.dune-project.org/download/2.2.1/dune-geometry-2.2.1.tar.gz)
set(default_geometry_SHA256 5ee2e8f514018f4e2f775c3c751764637895efd3851b33ab6eb91e2add5d9066)
set(default_grid_URL
    http://www.dune-project.org/download/2.2.1/dune-grid-2.2.1.tar.gz)
set(default_grid_SHA256 9f90e9a1f690a026f5dfee79c857f149cb219c10b6064fcb92a0e36c1d6364c0)
set(default_localfunctions_URL
    http://www.dune-project.org/download/2.2.1/dune-localfunctions-2.2.1.tar.gz)
set(default_localfunctions_SHA256 cf0fd7c486a2d417d4e959cf9246e13b73ce66702c4dc4c0b2eec3e8c0a50db9)

# Needed by dune
#enable_language(Fortran)
macro(find_program_or_fail VARIABLE)
    find_program(${VARIABLE} ${ARGN})
    if(NOT ${VARIABLE})
        message(FATAL_ERROR "Program needed for Dune was not found: ${VARIABLE}")
    endif()
endmacro()
find_package(PkgConfig REQUIRED)
find_program_or_fail(libtoolize_EXECUTABLE
    NAMES libtoolize glibtoolize)
find_program_or_fail(autoconf_EXECUTABLE autoconf)
find_program_or_fail(aclocal_EXECUTABLE aclocal)
find_program_or_fail(automake_EXECUTABLE automake)

macro(_get_arguments component)
    set(keyvalues  ${component}_URL;${component}_SHA256)
    cmake_parse_arguments(_ "" "${keyvalues}" "" ${Dune_ARGUMENTS})
    if(__${component}_URL AND NOT __${component}_SHA256)
        message(FATAL_ERROR "${component} given a URL but no SHA256 hash")
    elseif(__${component}_URL AND __${component}_SHA256)
        set(prefix "__")
    else()
        set(prefix "default_")
    endif()
    set(${component}_ARGUMENTS
        URL ${${prefix}${component}_URL}
        URL_HASH SHA256=${${prefix}${component}_SHA256}
    )
endmacro()

set(build_args
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
function(add_to_var VARIABLE PATH)
    include(CMakeParseArguments)
    cmake_parse_arguments(envvar "PREPEND" "" "" ${ARGN})
    if("${${VARIABLE}}" STREQUAL "")
        set(separator "")
    else()
        set(separator ":")
    endif()
    if(envvar_PREPEND)
        set(${${VARIABLE}} "${PATH}${separator}${${VARIABLE}}")
    else()
        set(${${VARIABLE}} "${${VARIABLE}}${separator}${PATH}")
    endif()
endfunction()


macro(_file_args OUTVAR withname)
    unset(result_var)
    foreach(filename ${ARGN})
        add_to_var(result_var "${filename}")
    endforeach()
    if(result_var)
        set(${OUTVAR} "--with-${withname}=\"${result_var}\"")
    endif()
endmacro()

_file_args(blas_args blas ${BLAS_LIBRARIES})
_file_args(lapack_args lapack ${LAPACK_LIBRARIES})

find_program(bash_EXECUTABLE bash REQUIRED)

# Create a script that cmake can call
# This should remove some issues that arises when cmake tries to build
# a complicated command line
function(write_configure_file path)
    get_filename_component(filename "${path}" NAME)
    file(WRITE "${PROJECT_BINARY_DIR}/CMakeFiles/external/${filename}"
        "#!${bash_EXECUTABLE}\n"
        "# Calls configure script for Dune packages\n"
        "export CC=${CMAKE_C_COMPILER}\n"
        "export CFLAGS=\"${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} "
    )
    if(BLAS_INCLUDE_DIR)
        file(APPEND "${PROJECT_BINARY_DIR}/CMakeFiles/external/${filename}"
            "-I${BLAS_INCLUDE_DIR}")
    endif()
    if(LAPACK_INCLUDE_DIR)
        file(APPEND "${PROJECT_BINARY_DIR}/CMakeFiles/external/${filename}"
            "-I${LAPACK_INCLUDE_DIR}")
    endif()
    file(APPEND "${PROJECT_BINARY_DIR}/CMakeFiles/external/${filename}"
        "\"\n"
        "export CXX=${CMAKE_CXX_COMPILER}\n"
        "export CXXFLAGS=\"${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\"\n"
        "export FC=${CMAKE_Fortran_COMPILER}\n"
        "export F77=${CMAKE_Fortran_COMPILER}\n"
        "export F90=${CMAKE_Fortran_COMPILER}\n"
        "export FCFLAGS=\"${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_RELEASE}\"\n"
        "export PKG_CONFIG_PATH=\"$ENV{PKG_CONFIG_PATH}\"\n"
        "\n"
        "./configure"
           " --enable-shared=no"
           " --enable-static=yes"
           " --with-pic"
           " --disable-documentation"
           " --enable-fieldvector-size-is-method"
           " --prefix=\"${EXTERNAL_ROOT}\""
           " --with-blas=\"${BLAS_LIBRARIES}\""
           " --with-blas=\"${LAPACK_LIBRARIES}\""
        ${ARGN}
    )
    file(COPY "${PROJECT_BINARY_DIR}/CMakeFiles/external/${filename}"
        DESTINATION "${EXTERNAL_ROOT}/src/"
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    )
endfunction()

set(configure_command "${EXTERNAL_ROOT}/src/dune_configure.sh")
write_configure_file("${configure_command}")

# Special stuff for dune-grid + ALUGrid
set(grid_configure_command "${configure_command}")
set(grid_depends "")
if(ALUGrid_FOUND OR TARGET ALUGrid)
    set(dune_grid_configure_command "${EXTERNAL_ROOT}/src/dune_grid_configure.sh")
    get_filename_component(ALUGrid_DIRECTORY "${ALUGrid_LIBRARIES}" PATH)
    get_filename_component(ALUGrid_DIRECTORY "${ALUGrid_DIRECTORY}" PATH)
    write_configure_file("${grid_configure_command}"
        "--with-alugrid=\"${ALUGrid_DIRECTORY}\""
    )
    if(TARGET ALUGrid)
        set(grid_depends ALUGrid)
    endif()
endif()


_get_arguments(common)
ExternalProject_Add(
    dune-common
    PREFIX ${EXTERNAL_ROOT}
    DEPENDS ${depends_on}
    ${common_ARGUMENTS}
    CONFIGURE_COMMAND ${configure_command}
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
    ${build_args}
)

_get_arguments(geometry)
ExternalProject_Add(
    dune-geometry
    DEPENDS dune-common
    PREFIX ${EXTERNAL_ROOT}
    ${geometry_ARGUMENTS}
    CONFIGURE_COMMAND ${configure_command}
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
    ${build_args}
)

include(PatchScript)
set(patchdir "${PROJECT_SOURCE_DIR}/cmake/patches/dune")
create_patch_script(Dune dune_patch_script
    CMDLINE "-p0"
    WORKING_DIRECTORY "${EXTERNAL_ROOT}/src"
    "${patchdir}/grid_yaspgrid.patch"
    "${patchdir}/grid_dgfparser.patch"
    "${patchdir}/grid_alugrid.patch"
)

_get_arguments(grid)
ExternalProject_Add(
    dune-grid
    DEPENDS dune-geometry dune-common ${grid_depends}
    PREFIX ${EXTERNAL_ROOT}
    ${grid_ARGUMENTS}
    PATCH_COMMAND ${dune_patch_script}
    CONFIGURE_COMMAND ${grid_configure_command}
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
    ${build_args}
)

_get_arguments(localfunctions)
ExternalProject_Add(
    dune-localfunctions
    DEPENDS dune-geometry dune-common
    PREFIX ${EXTERNAL_ROOT}
    ${localfunctions_ARGUMENTS}
    CONFIGURE_COMMAND ${configure_command}
    PATCH_COMMAND
        ${CMAKE_COMMAND} -DROOT=${EXTERNAL_ROOT}/src/dune-localfunctions
            -P ${CURRENT_LOOKUP_DIRECTORY}/patch-localfunctions.cmake
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
    ${build_args}
)

ExternalProject_Add(
    dune-foamgrid
    DEPENDS dune-grid dune-geometry dune-common
    PREFIX ${EXTERNAL_ROOT}
    URL ${PROJECT_SOURCE_DIR}/contrib/dune/dune-foamgrid
    PATCH_COMMAND
       ${CMAKE_COMMAND} -E copy_if_different
                        ${CURRENT_LOOKUP_DIRECTORY}/foamgrid-install.cmake
                        ${EXTERNAL_ROOT}/src/dune-foamgrid/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_ROOT}
)

# This file helps to create a fake dune project
# It answers the question posed by the duneproject script, including the subsidiary
# "this directory already exists..."
file(
  WRITE ${EXTERNAL_ROOT}/src/bempp.dune.input
  "dune-bempp
dune-common dune-geometry dune-grid dune-localfunctions
1
me@me
y
y
"
)

# Create fake dune project, with the sole goal of generating a config.h file!
# First, generate a new project
ExternalProject_Add(
    dune-bempp
    DEPENDS dune-foamgrid dune-grid dune-geometry dune-common dune-localfunctions
    PREFIX ${EXTERNAL_ROOT}
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ${configure_command}
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
               ${EXTERNAL_ROOT}/src/dune-bempp/config.h
               ${EXTERNAL_ROOT}/include/dune_config.h
    ${build_args}
)
ExternalProject_Add_Step(dune-bempp
    CREATE_PROJECT
    COMMAND dune-common/bin/duneproject < bempp.dune.input
    COMMAND dune-common/bin/dunecontrol --module=dune-bempp autogen
    WORKING_DIRECTORY ${EXTERNAL_ROOT}/src
    DEPENDERS configure
)

# Rerun cmake to capture new dune install
add_recursive_cmake_step(dune-bempp
    PACKAGE_NAME Dune
    FOUND_VAR Dune_FOUND
    DEPENDEES install
)
add_dependencies(lookup_dependencies dune-bempp)
