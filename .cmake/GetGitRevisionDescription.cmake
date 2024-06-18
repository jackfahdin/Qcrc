if(__get_git_revision_description)
    return()
endif()
set(__get_git_revision_description YES)

get_filename_component(_gitdescmoddir ${CMAKE_CURRENT_LIST_FILE} PATH)

# 查找git.exe
function(_find_git)
    if(NOT GIT_FOUND)
        find_package(Git QUIET)
    endif()
    if(NOT GIT_FOUND)
        message(FATAL_ERROR "Git not found")
    endif()
endfunction()

# 运行git命令
function(_run_git_command _output_var)
    message("command: ${ARGN}")
    execute_process(
        COMMAND ${ARGN}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE res
        OUTPUT_VARIABLE out
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(res EQUAL 0)
        string(STRIP "${out}" stripped_out)
        set(${_output_var} "${stripped_out}" PARENT_SCOPE)
    else()
        set(${_output_var} "GIT-COMMAND-FAILED" PARENT_SCOPE)
    endif()
endfunction()

# 查找最近的 .git 目录
function(_git_find_closest_git_dir _start_dir _git_dir_var)
    set(cur_dir "${_start_dir}")
    while(NOT EXISTS "${cur_dir}/.git")
        get_filename_component(cur_dir "${cur_dir}" DIRECTORY)
        if(cur_dir STREQUAL "${cur_dir}/..")
            set(${_git_dir_var} "" PARENT_SCOPE)
            return()
        endif()
    endwhile()
    set(${_git_dir_var} "${cur_dir}/.git" PARENT_SCOPE)
endfunction()

# 找到最近的 Git 目录，并获取当前 Git HEAD 的 refspec 和 hash
function(get_git_head_revision _refspecvar _hashvar)
    _git_find_closest_git_dir("${CMAKE_CURRENT_SOURCE_DIR}" GIT_DIR)

    set(ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR FALSE)
    if("${ARGN}" STREQUAL "ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR")
        set(ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR TRUE)
    endif()

    if(NOT "${GIT_DIR}" STREQUAL "")
        file(RELATIVE_PATH _relative_to_source_dir "${CMAKE_SOURCE_DIR}" "${GIT_DIR}")
        if("${_relative_to_source_dir}" MATCHES "[.][.]" AND NOT ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
            set(GIT_DIR "")
        endif()
    endif()

    if("${GIT_DIR}" STREQUAL "")
        set(${_refspecvar} "GITDIR-NOTFOUND" PARENT_SCOPE)
        set(${_hashvar} "GITDIR-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    set(HEAD_SOURCE_FILE "${GIT_DIR}/HEAD")
    if(NOT IS_DIRECTORY ${GIT_DIR})
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --show-superproject-working-tree
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE out
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT "${out}" STREQUAL "")
            file(READ ${GIT_DIR} submodule)
            string(REGEX REPLACE "gitdir: (.*)$" "\\1" GIT_DIR_RELATIVE ${submodule})
            string(STRIP ${GIT_DIR_RELATIVE} GIT_DIR_RELATIVE)
            get_filename_component(SUBMODULE_DIR ${GIT_DIR} PATH)
            get_filename_component(GIT_DIR ${SUBMODULE_DIR}/${GIT_DIR_RELATIVE} ABSOLUTE)
            set(HEAD_SOURCE_FILE "${GIT_DIR}/HEAD")
        else()
            file(READ ${GIT_DIR} worktree_ref)
            string(REGEX REPLACE "gitdir: (.*)$" "\\1" git_worktree_dir ${worktree_ref})
            string(STRIP ${git_worktree_dir} git_worktree_dir)
            _git_find_closest_git_dir("${git_worktree_dir}" GIT_DIR)
            set(HEAD_SOURCE_FILE "${git_worktree_dir}/HEAD")
        endif()
    endif()

    if(NOT EXISTS "${HEAD_SOURCE_FILE}")
        set(${_refspecvar} "HEAD-NOTFOUND" PARENT_SCOPE)
        set(${_hashvar} "HEAD-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    set(GIT_DATA "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/git-data")
    if(NOT EXISTS "${GIT_DATA}")
        file(MAKE_DIRECTORY "${GIT_DATA}")
    endif()

    set(HEAD_FILE "${GIT_DATA}/HEAD")
    configure_file("${HEAD_SOURCE_FILE}" "${HEAD_FILE}" COPYONLY)

    configure_file("${_gitdescmoddir}/GetGitRevisionDescription.cmake.in" "${GIT_DATA}/grabRef.cmake" @ONLY)
    include("${GIT_DATA}/grabRef.cmake")

    set(${_refspecvar} "${HEAD_REF}" PARENT_SCOPE)
    set(${_hashvar} "${HEAD_HASH}" PARENT_SCOPE)
endfunction()

function(git_latest_tag _var)
    _run_git_command(result ${GIT_EXECUTABLE} describe --abbrev=0 --tag)
    if(result STREQUAL "GIT-COMMAND-FAILED")
        set(result "0.0")
    endif()
    set(${_var} "${result}" PARENT_SCOPE)
endfunction()

function(git_commit_counts _var)
    _run_git_command(result ${GIT_EXECUTABLE} rev-list HEAD --count)
    if(result STREQUAL "GIT-COMMAND-FAILED")
        set(result "0")
    endif()
    set(${_var} "${result}" PARENT_SCOPE)
endfunction()

function(git_commit_new_tag_counts _var)
    # 获取最近的标签
    _run_git_command(latest_tag ${GIT_EXECUTABLE} describe --abbrev=0 --tags)

    if("${latest_tag}" STREQUAL "GIT-COMMAND-FAILED")
        # 如果没有标签，获取仓库的总提交数
        _run_git_command(commit_count ${GIT_EXECUTABLE} rev-list HEAD --count)
        if("${commit_count}" STREQUAL "GIT-COMMAND-FAILED")
            set(commit_count "0")
        endif()
    else()
        # 获取从最近标签到最新提交的提交个数
        _run_git_command(commit_count ${GIT_EXECUTABLE} rev-list ${latest_tag}..HEAD --count)
        if("${commit_count}" STREQUAL "GIT-COMMAND-FAILED")
            set(commit_count "0")
        endif()
    endif()

    set(${_var} "${commit_count}" PARENT_SCOPE)
endfunction()

function(git_describe _var)
    get_git_head_revision(refspec hash ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)

    if(NOT hash)
        set(${_var} "HEAD-HASH-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    _run_git_command(out ${GIT_EXECUTABLE} describe --tags --always ${hash} ${ARGN})

    if("${out}" STREQUAL "GIT-COMMAND-FAILED")
        set(out "${hash}-NOTFOUND")
    endif()

    set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_release_version _var)
    get_git_head_revision(refspec hash ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)

    if(NOT hash)
        set(${_var} "HEAD-HASH-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    _run_git_command(out ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD)

    if("${out}" STREQUAL "GIT-COMMAND-FAILED")
        set(${_var} "" PARENT_SCOPE)
        return()
    endif()

    if("${out}" MATCHES "^release/.+$")
        string(REPLACE "release/" "" tmp_out "${out}")
        set(${_var} "${tmp_out}" PARENT_SCOPE)
    else()
        set(${_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(git_describe_working_tree _var)
    _run_git_command(out ${GIT_EXECUTABLE} describe --dirty ${ARGN})

    if("${out}" STREQUAL "GIT-COMMAND-FAILED")
        set(out "${out}-NOTFOUND")
    endif()

    set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_get_exact_tag _var)
    _run_git_command(out ${GIT_EXECUTABLE} describe --exact-match ${ARGN})

    if("${out}" STREQUAL "GIT-COMMAND-FAILED")
        set(out "TAG-NOTFOUND")
    endif()

    set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_local_changes _var)
    get_git_head_revision(refspec hash)

    if(NOT hash)
        set(${_var} "HEAD-HASH-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    _run_git_command(res "${GIT_EXECUTABLE}" diff-index --quiet HEAD --)

    if("${res}" STREQUAL "GIT-COMMAND-FAILED" OR NOT res EQUAL 0)
        set(${_var} "DIRTY" PARENT_SCOPE)
    else()
        set(${_var} "CLEAN" PARENT_SCOPE)
    endif()
endfunction()

_find_git()
git_latest_tag(LATEST_TAG)
git_commit_counts(COMMIT_COUNTS)
git_commit_new_tag_counts(COMMIT_NEW_TAG_COUNTS)
git_describe(DESCRIBE)
git_release_version(RELEASE_VERSION)
git_describe_working_tree(DESCRIBE_WORKING_TREE)
git_get_exact_tag(GET_EXACT_TAG)
git_local_changes(LOCAL_CHANGES)
message(STATUS "LATEST_TAG : " ${LATEST_TAG})
message(STATUS "COMMIT_COUNTS : " ${COMMIT_COUNTS})
message(STATUS "COMMIT_NEW_TAG_COUNTS : " ${COMMIT_NEW_TAG_COUNTS})
message(STATUS "DESCRIBE : " ${DESCRIBE})
message(STATUS "RELEASE_VERSION : " ${RELEASE_VERSION})
message(STATUS "DESCRIBE_WORKING_TREE : " ${DESCRIBE_WORKING_TREE})
message(STATUS "GET_EXACT_TAG : " ${GET_EXACT_TAG})
message(STATUS "LOCAL_CHANGES : " ${LOCAL_CHANGES})
message(STATUS "RELEASE_VERSION : " ${RELEASE_VERSION})

string(REPLACE "." "," GIT_TAG_WITH_COMMA ${LATEST_TAG})
string(REGEX MATCH "[0-9]+\\.[0-9]+" GIT_SEMVER "${LATEST_TAG}")
string(REGEX MATCH "([0-9]+)\\.([0-9]+)" SEMVER_SPLITED "${GIT_SEMVER}")

message(STATUS "GIT_SEMVER : " ${GIT_SEMVER})
