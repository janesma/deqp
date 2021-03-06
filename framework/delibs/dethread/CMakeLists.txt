# dethread cmake file

if (NOT DE_DEFS)
	message(FATAL_ERROR "Include Defs.cmake")
endif ()

set(DETHREAD_SRCS
	deAtomic.c
	deAtomic.h
	deMutex.h
	deSemaphore.h
	deSingleton.c
	deSingleton.h
	deThread.h
	deThreadLocal.h
	deThreadTest.c
	deThreadTest.h
	)

set(DETHREAD_LIBS
	debase
	depool
	)

include_directories(
	../debase
	../depool
	${CMAKE_CURRENT_SOURCE_DIR}
	)

if (DE_OS_IS_WIN32 OR DE_OS_IS_WINCE)
	set(DETHREAD_SRCS
		${DETHREAD_SRCS}
		win32/deMutexWin32.c
		win32/deSemaphoreWin32.c
		win32/deThreadWin32.c
		win32/deThreadLocalWin32.c
		)

elseif (DE_OS_IS_UNIX OR DE_OS_IS_ANDROID OR DE_OS_IS_SYMBIAN)
	add_definitions(-D_XOPEN_SOURCE=600)
	set(DETHREAD_SRCS
		${DETHREAD_SRCS}
		unix/deMutexUnix.c
		unix/deSemaphoreUnix.c
		unix/deThreadUnix.c
		unix/deThreadLocalUnix.c
		)

	if (DE_OS_IS_UNIX)
		set(DETHREAD_LIBS ${DETHREAD_LIBS} pthread)
	elseif (DE_OS_IS_SYMBIAN)
		find_file(PTHREAD_LIB libpthread.dso PATHS ${SYMBIAN_LIB_DIR} NO_CMAKE_FIND_ROOT_PATH)
		set(DETHREAD_LIBS ${DETHREAD_LIBS} ${PTHREAD_LIB})
    endif ()

elseif (DE_OS_IS_OSX OR DE_OS_IS_IOS)
	add_definitions(-D_XOPEN_SOURCE=600)
	# \note OS X doesn't support unnamed semaphores.
	set(DETHREAD_SRCS
		${DETHREAD_SRCS}
		unix/deMutexUnix.c
		unix/deNamedSemaphoreUnix.c
		unix/deThreadUnix.c
		unix/deThreadLocalUnix.c
		)

else ()
	message(FATAL_ERROR "Unsupported os for dethread")
endif ()

add_library(dethread STATIC ${DETHREAD_SRCS})
target_link_libraries(dethread ${DETHREAD_LIBS})

set(DETHREAD_STANDALONE_TEST ON CACHE STRING "Build standalone binary for testing dethread.")

if (DETHREAD_STANDALONE_TEST AND (DE_OS_IS_WIN32 OR DE_OS_IS_UNIX OR DE_OS_IS_OSX))
	add_executable(dethread_test standalone_test.c)
	target_link_libraries(dethread_test dethread debase)
endif ()
