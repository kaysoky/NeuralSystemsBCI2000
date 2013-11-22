###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: List of files belonging to the LibTiny utils library.

SET( TINY_DIR
  ${PROJECT_SRC_DIR}/shared/utils/Lib
)

SET( SRC_LIBTINY

  ${TINY_DIR}/Uncopyable.h
  ${TINY_DIR}/Lockable.h
  ${TINY_DIR}/Runnable.h
  ${TINY_DIR}/SharedPointer.h
  ${TINY_DIR}/LazyArray.h

  ${TINY_DIR}/Atomic.h
  ${TINY_DIR}/Synchronized.h
  ${TINY_DIR}/SynchronizedQueue.h
  ${TINY_DIR}/SpinLock.h

  ${TINY_DIR}/Waitable.cpp
  ${TINY_DIR}/Mutex.cpp
  ${TINY_DIR}/Semaphore.cpp

  ${TINY_DIR}/Thread.cpp
  ${TINY_DIR}/ReusableThread.cpp
  ${TINY_DIR}/ThreadLocal.h
  ${TINY_DIR}/ThreadUtils.cpp

  ${TINY_DIR}/FileUtils.cpp
  ${TINY_DIR}/ProcessUtils.cpp
  ${TINY_DIR}/DylibImports.cpp
  ${TINY_DIR}/SysError.cpp
  ${TINY_DIR}/SharedMemory.cpp

  ${TINY_DIR}/StringUtils.cpp  
  ${TINY_DIR}/EscapedString.cpp
  ${TINY_DIR}/ClassName.cpp
  ${TINY_DIR}/WildcardMatch.cpp
  ${TINY_DIR}/EnvVariable.cpp

  ${TINY_DIR}/PrecisionTime.cpp
  ${TINY_DIR}/Clock.cpp
  ${TINY_DIR}/StopWatch.h

  ${TINY_DIR}/NullStream.h
  ${TINY_DIR}/RedirectIO.cpp
  ${TINY_DIR}/SockStream.cpp
  ${TINY_DIR}/ThreadedSockbuf.cpp

  ${TINY_DIR}/ExceptionHandler.cpp
  ${TINY_DIR}/Exception.cpp
  ${TINY_DIR}/Debugging.cpp
  
  ${TINY_DIR}/LCRandomGenerator.cpp
  ${TINY_DIR}/Uuid.h
  ${TINY_DIR}/BinaryData.h
  ${TINY_DIR}/Numeric.h
)
