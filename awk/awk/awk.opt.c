#include "apue.h"
#include <errno.h>
#include <limits.h>

static void	pr_sysconf(char *, int);
static void	pr_pathconf(char *, char *, int);

int
main(int argc, char *argv[])
{
	if (argc != 2)
		err_quit("usage: a.out <dirname>");

#ifdef _POSIX_ADVISORY_INFO
	printf("_POSIX_ADVISORY_INFO defined to be %ld\n", (long)_POSIX_ADVISORY_INFO+0);
#else
	printf("no symbol for _POSIX_ADVISORY_INFO\n");
#endif
#ifdef _SC_ADVISORY_INFO
	pr_sysconf("_POSIX_ADVISORY_INFO =", _SC_ADVISORY_INFO);
#else
	printf("no symbol for _SC_ADVISORY_INFO\n");
#endif
#ifdef _POSIX_ASYNCHRONOUS_IO
	printf("_POSIX_ASYNCHRONOUS_IO defined to be %ld\n", (long)_POSIX_ASYNCHRONOUS_IO+0);
#else
	printf("no symbol for _POSIX_ASYNCHRONOUS_IO\n");
#endif
#ifdef _SC_ASYNCHRONOUS_IO
	pr_sysconf("_POSIX_ASYNCHRONOUS_IO =", _SC_ASYNCHRONOUS_IO);
#else
	printf("no symbol for _SC_ASYNCHRONOUS_IO\n");
#endif
#ifdef _POSIX_BARRIERS
	printf("_POSIX_BARRIERS defined to be %ld\n", (long)_POSIX_BARRIERS+0);
#else
	printf("no symbol for _POSIX_BARRIERS\n");
#endif
#ifdef _SC_BARRIERS
	pr_sysconf("_POSIX_BARRIERS =", _SC_BARRIERS);
#else
	printf("no symbol for _SC_BARRIERS\n");
#endif
#ifdef _POSIX_CLOCK_SELECTION
	printf("_POSIX_CLOCK_SELECTION defined to be %ld\n", (long)_POSIX_CLOCK_SELECTION+0);
#else
	printf("no symbol for _POSIX_CLOCK_SELECTION\n");
#endif
#ifdef _SC_CLOCK_SELECTION
	pr_sysconf("_POSIX_CLOCK_SELECTION =", _SC_CLOCK_SELECTION);
#else
	printf("no symbol for _SC_CLOCK_SELECTION\n");
#endif
#ifdef _POSIX_CPUTIME
	printf("_POSIX_CPUTIME defined to be %ld\n", (long)_POSIX_CPUTIME+0);
#else
	printf("no symbol for _POSIX_CPUTIME\n");
#endif
#ifdef _SC_CPUTIME
	pr_sysconf("_POSIX_CPUTIME =", _SC_CPUTIME);
#else
	printf("no symbol for _SC_CPUTIME\n");
#endif
#ifdef _POSIX_FSYNC
	printf("_POSIX_FSYNC defined to be %ld\n", (long)_POSIX_FSYNC+0);
#else
	printf("no symbol for _POSIX_FSYNC\n");
#endif
#ifdef _SC_FSYNC
	pr_sysconf("_POSIX_FSYNC =", _SC_FSYNC);
#else
	printf("no symbol for _SC_FSYNC\n");
#endif
#ifdef _POSIX_IPV6
	printf("_POSIX_IPV6 defined to be %ld\n", (long)_POSIX_IPV6+0);
#else
	printf("no symbol for _POSIX_IPV6\n");
#endif
#ifdef _SC_IPV6
	pr_sysconf("_POSIX_IPV6 =", _SC_IPV6);
#else
	printf("no symbol for _SC_IPV6\n");
#endif
#ifdef _POSIX_JOB_CONTROL
	printf("_POSIX_JOB_CONTROL defined to be %ld\n", (long)_POSIX_JOB_CONTROL+0);
#else
	printf("no symbol for _POSIX_JOB_CONTROL\n");
#endif
#ifdef _SC_JOB_CONTROL
	pr_sysconf("_POSIX_JOB_CONTROL =", _SC_JOB_CONTROL);
#else
	printf("no symbol for _SC_JOB_CONTROL\n");
#endif
#ifdef _POSIX_MAPPED_FILES
	printf("_POSIX_MAPPED_FILES defined to be %ld\n", (long)_POSIX_MAPPED_FILES+0);
#else
	printf("no symbol for _POSIX_MAPPED_FILES\n");
#endif
#ifdef _SC_MAPPED_FILES
	pr_sysconf("_POSIX_MAPPED_FILES =", _SC_MAPPED_FILES);
#else
	printf("no symbol for _SC_MAPPED_FILES\n");
#endif
#ifdef _POSIX_MEMLOCK
	printf("_POSIX_MEMLOCK defined to be %ld\n", (long)_POSIX_MEMLOCK+0);
#else
	printf("no symbol for _POSIX_MEMLOCK\n");
#endif
#ifdef _SC_MEMLOCK
	pr_sysconf("_POSIX_MEMLOCK =", _SC_MEMLOCK);
#else
	printf("no symbol for _SC_MEMLOCK\n");
#endif
#ifdef _POSIX_MEMLOCK_RANGE
	printf("_POSIX_MEMLOCK_RANGE defined to be %ld\n", (long)_POSIX_MEMLOCK_RANGE+0);
#else
	printf("no symbol for _POSIX_MEMLOCK_RANGE\n");
#endif
#ifdef _SC_MEMLOCK_RANGE
	pr_sysconf("_POSIX_MEMLOCK_RANGE =", _SC_MEMLOCK_RANGE);
#else
	printf("no symbol for _SC_MEMLOCK_RANGE\n");
#endif
#ifdef _POSIX_MEMORY_PROTECTION
	printf("_POSIX_MEMORY_PROTECTION defined to be %ld\n", (long)_POSIX_MEMORY_PROTECTION+0);
#else
	printf("no symbol for _POSIX_MEMORY_PROTECTION\n");
#endif
#ifdef _SC_MEMORY_PROTECTION
	pr_sysconf("_POSIX_MEMORY_PROTECTION =", _SC_MEMORY_PROTECTION);
#else
	printf("no symbol for _SC_MEMORY_PROTECTION\n");
#endif
#ifdef _POSIX_MESSAGE_PASSING
	printf("_POSIX_MESSAGE_PASSING defined to be %ld\n", (long)_POSIX_MESSAGE_PASSING+0);
#else
	printf("no symbol for _POSIX_MESSAGE_PASSING\n");
#endif
#ifdef _SC_MESSAGE_PASSING
	pr_sysconf("_POSIX_MESSAGE_PASSING =", _SC_MESSAGE_PASSING);
#else
	printf("no symbol for _SC_MESSAGE_PASSING\n");
#endif
#ifdef _POSIX_MONOTONIC_CLOCK
	printf("_POSIX_MONOTONIC_CLOCK defined to be %ld\n", (long)_POSIX_MONOTONIC_CLOCK+0);
#else
	printf("no symbol for _POSIX_MONOTONIC_CLOCK\n");
#endif
#ifdef _SC_MONOTONIC_CLOCK
	pr_sysconf("_POSIX_MONOTONIC_CLOCK =", _SC_MONOTONIC_CLOCK);
#else
	printf("no symbol for _SC_MONOTONIC_CLOCK\n");
#endif
#ifdef _POSIX_PRIORITIZED_IO
	printf("_POSIX_PRIORITIZED_IO defined to be %ld\n", (long)_POSIX_PRIORITIZED_IO+0);
#else
	printf("no symbol for _POSIX_PRIORITIZED_IO\n");
#endif
#ifdef _SC_PRIORITIZED_IO
	pr_sysconf("_POSIX_PRIORITIZED_IO =", _SC_PRIORITIZED_IO);
#else
	printf("no symbol for _SC_PRIORITIZED_IO\n");
#endif
#ifdef _POSIX_PRIORITY_SCHEDULING
	printf("_POSIX_PRIORITY_SCHEDULING defined to be %ld\n", (long)_POSIX_PRIORITY_SCHEDULING+0);
#else
	printf("no symbol for _POSIX_PRIORITY_SCHEDULING\n");
#endif
#ifdef _SC_PRIORITY_SCHEDULING
	pr_sysconf("_POSIX_PRIORITY_SCHEDULING =", _SC_PRIORITY_SCHEDULING);
#else
	printf("no symbol for _SC_PRIORITY_SCHEDULING\n");
#endif
#ifdef _POSIX_RAW_SOCKETS
	printf("_POSIX_RAW_SOCKETS defined to be %ld\n", (long)_POSIX_RAW_SOCKETS+0);
#else
	printf("no symbol for _POSIX_RAW_SOCKETS\n");
#endif
#ifdef _SC_RAW_SOCKETS
	pr_sysconf("_POSIX_RAW_SOCKETS =", _SC_RAW_SOCKETS);
#else
	printf("no symbol for _SC_RAW_SOCKETS\n");
#endif
#ifdef _POSIX_READER_WRITER_LOCKS
	printf("_POSIX_READER_WRITER_LOCKS defined to be %ld\n", (long)_POSIX_READER_WRITER_LOCKS+0);
#else
	printf("no symbol for _POSIX_READER_WRITER_LOCKS\n");
#endif
#ifdef _SC_READER_WRITER_LOCKS
	pr_sysconf("_POSIX_READER_WRITER_LOCKS =", _SC_READER_WRITER_LOCKS);
#else
	printf("no symbol for _SC_READER_WRITER_LOCKS\n");
#endif
#ifdef _POSIX_REALTIME_SIGNALS
	printf("_POSIX_REALTIME_SIGNALS defined to be %ld\n", (long)_POSIX_REALTIME_SIGNALS+0);
#else
	printf("no symbol for _POSIX_REALTIME_SIGNALS\n");
#endif
#ifdef _SC_REALTIME_SIGNALS
	pr_sysconf("_POSIX_REALTIME_SIGNALS =", _SC_REALTIME_SIGNALS);
#else
	printf("no symbol for _SC_REALTIME_SIGNALS\n");
#endif
#ifdef _POSIX_REGEXP
	printf("_POSIX_REGEXP defined to be %ld\n", (long)_POSIX_REGEXP+0);
#else
	printf("no symbol for _POSIX_REGEXP\n");
#endif
#ifdef _SC_REGEXP
	pr_sysconf("_POSIX_REGEXP =", _SC_REGEXP);
#else
	printf("no symbol for _SC_REGEXP\n");
#endif
#ifdef _POSIX_SAVED_IDS
	printf("_POSIX_SAVED_IDS defined to be %ld\n", (long)_POSIX_SAVED_IDS+0);
#else
	printf("no symbol for _POSIX_SAVED_IDS\n");
#endif
#ifdef _SC_SAVED_IDS
	pr_sysconf("_POSIX_SAVED_IDS =", _SC_SAVED_IDS);
#else
	printf("no symbol for _SC_SAVED_IDS\n");
#endif
#ifdef _POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES defined to be %ld\n", (long)_POSIX_SEMAPHORES+0);
#else
	printf("no symbol for _POSIX_SEMAPHORES\n");
#endif
#ifdef _SC_SEMAPHORES
	pr_sysconf("_POSIX_SEMAPHORES =", _SC_SEMAPHORES);
#else
	printf("no symbol for _SC_SEMAPHORES\n");
#endif
#ifdef _POSIX_SHARED_MEMORY_OBJECTS
	printf("_POSIX_SHARED_MEMORY_OBJECTS defined to be %ld\n", (long)_POSIX_SHARED_MEMORY_OBJECTS+0);
#else
	printf("no symbol for _POSIX_SHARED_MEMORY_OBJECTS\n");
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
	pr_sysconf("_POSIX_SHARED_MEMORY_OBJECTS =", _SC_SHARED_MEMORY_OBJECTS);
#else
	printf("no symbol for _SC_SHARED_MEMORY_OBJECTS\n");
#endif
#ifdef _POSIX_SHELL
	printf("_POSIX_SHELL defined to be %ld\n", (long)_POSIX_SHELL+0);
#else
	printf("no symbol for _POSIX_SHELL\n");
#endif
#ifdef _SC_SHELL
	pr_sysconf("_POSIX_SHELL =", _SC_SHELL);
#else
	printf("no symbol for _SC_SHELL\n");
#endif
#ifdef _POSIX_SPAWN
	printf("_POSIX_SPAWN defined to be %ld\n", (long)_POSIX_SPAWN+0);
#else
	printf("no symbol for _POSIX_SPAWN\n");
#endif
#ifdef _SC_SPAWN
	pr_sysconf("_POSIX_SPAWN =", _SC_SPAWN);
#else
	printf("no symbol for _SC_SPAWN\n");
#endif
#ifdef _POSIX_SPIN_LOCKS
	printf("_POSIX_SPIN_LOCKS defined to be %ld\n", (long)_POSIX_SPIN_LOCKS+0);
#else
	printf("no symbol for _POSIX_SPIN_LOCKS\n");
#endif
#ifdef _SC_SPIN_LOCKS
	pr_sysconf("_POSIX_SPIN_LOCKS =", _SC_SPIN_LOCKS);
#else
	printf("no symbol for _SC_SPIN_LOCKS\n");
#endif
#ifdef _POSIX_SPORADIC_SERVER
	printf("_POSIX_SPORADIC_SERVER defined to be %ld\n", (long)_POSIX_SPORADIC_SERVER+0);
#else
	printf("no symbol for _POSIX_SPORADIC_SERVER\n");
#endif
#ifdef _SC_SPORADIC_SERVER
	pr_sysconf("_POSIX_SPORADIC_SERVER =", _SC_SPORADIC_SERVER);
#else
	printf("no symbol for _SC_SPORADIC_SERVER\n");
#endif
#ifdef _POSIX_SYNCHRONIZED_IO
	printf("_POSIX_SYNCHRONIZED_IO defined to be %ld\n", (long)_POSIX_SYNCHRONIZED_IO+0);
#else
	printf("no symbol for _POSIX_SYNCHRONIZED_IO\n");
#endif
#ifdef _SC_SYNCHRONIZED_IO
	pr_sysconf("_POSIX_SYNCHRONIZED_IO =", _SC_SYNCHRONIZED_IO);
#else
	printf("no symbol for _SC_SYNCHRONIZED_IO\n");
#endif
#ifdef _POSIX_THREAD_ATTR_STACKADDR
	printf("_POSIX_THREAD_ATTR_STACKADDR defined to be %ld\n", (long)_POSIX_THREAD_ATTR_STACKADDR+0);
#else
	printf("no symbol for _POSIX_THREAD_ATTR_STACKADDR\n");
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
	pr_sysconf("_POSIX_THREAD_ATTR_STACKADDR =", _SC_THREAD_ATTR_STACKADDR);
#else
	printf("no symbol for _SC_THREAD_ATTR_STACKADDR\n");
#endif
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	printf("_POSIX_THREAD_ATTR_STACKSIZE defined to be %ld\n", (long)_POSIX_THREAD_ATTR_STACKSIZE+0);
#else
	printf("no symbol for _POSIX_THREAD_ATTR_STACKSIZE\n");
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
	pr_sysconf("_POSIX_THREAD_ATTR_STACKSIZE =", _SC_THREAD_ATTR_STACKSIZE);
#else
	printf("no symbol for _SC_THREAD_ATTR_STACKSIZE\n");
#endif
#ifdef _POSIX_THREAD_ATTR_CPUTIME
	printf("_POSIX_THREAD_ATTR_CPUTIME defined to be %ld\n", (long)_POSIX_THREAD_ATTR_CPUTIME+0);
#else
	printf("no symbol for _POSIX_THREAD_ATTR_CPUTIME\n");
#endif
#ifdef _SC_THREAD_CPUTIME
	pr_sysconf("_POSIX_THREAD_ATTR_CPUTIME =", _SC_THREAD_CPUTIME);
#else
	printf("no symbol for _SC_THREAD_CPUTIME\n");
#endif
#ifdef _POSIX_THREAD_PRIO_INHERIT
	printf("_POSIX_THREAD_PRIO_INHERIT defined to be %ld\n", (long)_POSIX_THREAD_PRIO_INHERIT+0);
#else
	printf("no symbol for _POSIX_THREAD_PRIO_INHERIT\n");
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
	pr_sysconf("_POSIX_THREAD_PRIO_INHERIT =", _SC_THREAD_PRIO_INHERIT);
#else
	printf("no symbol for _SC_THREAD_PRIO_INHERIT\n");
#endif
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
	printf("_POSIX_THREAD_PRIORITY_SCHEDULING defined to be %ld\n", (long)_POSIX_THREAD_PRIORITY_SCHEDULING+0);
#else
	printf("no symbol for _POSIX_THREAD_PRIORITY_SCHEDULING\n");
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
	pr_sysconf("_POSIX_THREAD_PRIORITY_SCHEDULING =", _SC_THREAD_PRIORITY_SCHEDULING);
#else
	printf("no symbol for _SC_THREAD_PRIORITY_SCHEDULING\n");
#endif
#ifdef _POSIX_THREAD_PROCESS_SHARED
	printf("_POSIX_THREAD_PROCESS_SHARED defined to be %ld\n", (long)_POSIX_THREAD_PROCESS_SHARED+0);
#else
	printf("no symbol for _POSIX_THREAD_PROCESS_SHARED\n");
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
	pr_sysconf("_POSIX_THREAD_PROCESS_SHARED =", _SC_THREAD_PROCESS_SHARED);
#else
	printf("no symbol for _SC_THREAD_PROCESS_SHARED\n");
#endif
#ifdef _POSIX_THREAD_ROBUST_PRIO_INHERIT
	printf("_POSIX_THREAD_ROBUST_PRIO_INHERIT defined to be %ld\n", (long)_POSIX_THREAD_ROBUST_PRIO_INHERIT+0);
#else
	printf("no symbol for _POSIX_THREAD_ROBUST_PRIO_INHERIT\n");
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_INHERIT
	pr_sysconf("_POSIX_THREAD_ROBUST_PRIO_INHERIT =", _SC_THREAD_ROBUST_PRIO_INHERIT);
#else
	printf("no symbol for _SC_THREAD_ROBUST_PRIO_INHERIT\n");
#endif
#ifdef _POSIX_THREAD_ROBUST_PRIO_PROTECT
	printf("_POSIX_THREAD_ROBUST_PRIO_PROTECT defined to be %ld\n", (long)_POSIX_THREAD_ROBUST_PRIO_PROTECT+0);
#else
	printf("no symbol for _POSIX_THREAD_ROBUST_PRIO_PROTECT\n");
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_PROTECT
	pr_sysconf("_POSIX_THREAD_ROBUST_PRIO_PROTECT =", _SC_THREAD_ROBUST_PRIO_PROTECT);
#else
	printf("no symbol for _SC_THREAD_ROBUST_PRIO_PROTECT\n");
#endif
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
	printf("_POSIX_THREAD_SAFE_FUNCTIONS defined to be %ld\n", (long)_POSIX_THREAD_SAFE_FUNCTIONS+0);
#else
	printf("no symbol for _POSIX_THREAD_SAFE_FUNCTIONS\n");
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
	pr_sysconf("_POSIX_THREAD_SAFE_FUNCTIONS =", _SC_THREAD_SAFE_FUNCTIONS);
#else
	printf("no symbol for _SC_THREAD_SAFE_FUNCTIONS\n");
#endif
#ifdef _POSIX_THREAD_SPORADIC_SERVER
	printf("_POSIX_THREAD_SPORADIC_SERVER defined to be %ld\n", (long)_POSIX_THREAD_SPORADIC_SERVER+0);
#else
	printf("no symbol for _POSIX_THREAD_SPORADIC_SERVER\n");
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
	pr_sysconf("_POSIX_THREAD_SPORADIC_SERVER =", _SC_THREAD_SPORADIC_SERVER);
#else
	printf("no symbol for _SC_THREAD_SPORADIC_SERVER\n");
#endif
#ifdef _POSIX_THREADS
	printf("_POSIX_THREADS defined to be %ld\n", (long)_POSIX_THREADS+0);
#else
	printf("no symbol for _POSIX_THREADS\n");
#endif
#ifdef _SC_THREADS
	pr_sysconf("_POSIX_THREADS =", _SC_THREADS);
#else
	printf("no symbol for _SC_THREADS\n");
#endif
#ifdef _POSIX_TIMEOUTS
	printf("_POSIX_TIMEOUTS defined to be %ld\n", (long)_POSIX_TIMEOUTS+0);
#else
	printf("no symbol for _POSIX_TIMEOUTS\n");
#endif
#ifdef _SC_TIMEOUTS
	pr_sysconf("_POSIX_TIMEOUTS =", _SC_TIMEOUTS);
#else
	printf("no symbol for _SC_TIMEOUTS\n");
#endif
#ifdef _POSIX_TIMERS
	printf("_POSIX_TIMERS defined to be %ld\n", (long)_POSIX_TIMERS+0);
#else
	printf("no symbol for _POSIX_TIMERS\n");
#endif
#ifdef _SC_TIMERS
	pr_sysconf("_POSIX_TIMERS =", _SC_TIMERS);
#else
	printf("no symbol for _SC_TIMERS\n");
#endif
#ifdef _POSIX_TYPED_MEMORY_OBJECTS
	printf("_POSIX_TYPED_MEMORY_OBJECTS defined to be %ld\n", (long)_POSIX_TYPED_MEMORY_OBJECTS+0);
#else
	printf("no symbol for _POSIX_TYPED_MEMORY_OBJECTS\n");
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
	pr_sysconf("_POSIX_TYPED_MEMORY_OBJECTS =", _SC_TYPED_MEMORY_OBJECTS);
#else
	printf("no symbol for _SC_TYPED_MEMORY_OBJECTS\n");
#endif
#ifdef _POSIX_VERSION
	printf("_POSIX_VERSION defined to be %ld\n", (long)_POSIX_VERSION+0);
#else
	printf("no symbol for _POSIX_VERSION\n");
#endif
#ifdef _SC_VERSION
	pr_sysconf("_POSIX_VERSION =", _SC_VERSION);
#else
	printf("no symbol for _SC_VERSION\n");
#endif
#ifdef # _POSIX_V7_ILP32_OFF32
	printf("# _POSIX_V7_ILP32_OFF32 defined to be %ld\n", (long)# _POSIX_V7_ILP32_OFF32+0);
#else
	printf("no symbol for # _POSIX_V7_ILP32_OFF32\n");
#endif
#ifdef _SC_V7_ILP32_OFF32
	pr_sysconf("# _POSIX_V7_ILP32_OFF32 =", _SC_V7_ILP32_OFF32);
#else
	printf("no symbol for _SC_V7_ILP32_OFF32\n");
#endif
#ifdef # _POSIX_V7_ILP32_OFFBIG
	printf("# _POSIX_V7_ILP32_OFFBIG defined to be %ld\n", (long)# _POSIX_V7_ILP32_OFFBIG+0);
#else
	printf("no symbol for # _POSIX_V7_ILP32_OFFBIG\n");
#endif
#ifdef _SC_V7_ILP32_OFFBIG
	pr_sysconf("# _POSIX_V7_ILP32_OFFBIG =", _SC_V7_ILP32_OFFBIG);
#else
	printf("no symbol for _SC_V7_ILP32_OFFBIG\n");
#endif
#ifdef # _POSIX_V7_LP64_OFF64
	printf("# _POSIX_V7_LP64_OFF64 defined to be %ld\n", (long)# _POSIX_V7_LP64_OFF64+0);
#else
	printf("no symbol for # _POSIX_V7_LP64_OFF64\n");
#endif
#ifdef _SC_V7_LP64_OFF64
	pr_sysconf("# _POSIX_V7_LP64_OFF64 =", _SC_V7_LP64_OFF64);
#else
	printf("no symbol for _SC_V7_LP64_OFF64\n");
#endif
#ifdef # _POSIX_V7_LPBIG_OFFBIG
	printf("# _POSIX_V7_LPBIG_OFFBIG defined to be %ld\n", (long)# _POSIX_V7_LPBIG_OFFBIG+0);
#else
	printf("no symbol for # _POSIX_V7_LPBIG_OFFBIG\n");
#endif
#ifdef _SC_V7_LPBIG_OFFBIG
	pr_sysconf("# _POSIX_V7_LPBIG_OFFBIG =", _SC_V7_LPBIG_OFFBIG);
#else
	printf("no symbol for _SC_V7_LPBIG_OFFBIG\n");
#endif
#ifdef _XOPEN_CRYPT
	printf("_XOPEN_CRYPT defined to be %ld\n", (long)_XOPEN_CRYPT+0);
#else
	printf("no symbol for _XOPEN_CRYPT\n");
#endif
#ifdef _SC_XOPEN_CRYPT
	pr_sysconf("_XOPEN_CRYPT =", _SC_XOPEN_CRYPT);
#else
	printf("no symbol for _SC_XOPEN_CRYPT\n");
#endif
#ifdef _XOPEN_ENH_I18N
	printf("_XOPEN_ENH_I18N defined to be %ld\n", (long)_XOPEN_ENH_I18N+0);
#else
	printf("no symbol for _XOPEN_ENH_I18N\n");
#endif
#ifdef _SC_XOPEN_ENH_I18N
	pr_sysconf("_XOPEN_ENH_I18N =", _SC_XOPEN_ENH_I18N);
#else
	printf("no symbol for _SC_XOPEN_ENH_I18N\n");
#endif
#ifdef _XOPEN_REALTIME
	printf("_XOPEN_REALTIME defined to be %ld\n", (long)_XOPEN_REALTIME+0);
#else
	printf("no symbol for _XOPEN_REALTIME\n");
#endif
#ifdef _SC_XOPEN_REALTIME
	pr_sysconf("_XOPEN_REALTIME =", _SC_XOPEN_REALTIME);
#else
	printf("no symbol for _SC_XOPEN_REALTIME\n");
#endif
#ifdef _XOPEN_REALTIME_THREADS
	printf("_XOPEN_REALTIME_THREADS defined to be %ld\n", (long)_XOPEN_REALTIME_THREADS+0);
#else
	printf("no symbol for _XOPEN_REALTIME_THREADS\n");
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
	pr_sysconf("_XOPEN_REALTIME_THREADS =", _SC_XOPEN_REALTIME_THREADS);
#else
	printf("no symbol for _SC_XOPEN_REALTIME_THREADS\n");
#endif
#ifdef _XOPEN_SHM
	printf("_XOPEN_SHM defined to be %ld\n", (long)_XOPEN_SHM+0);
#else
	printf("no symbol for _XOPEN_SHM\n");
#endif
#ifdef _SC_XOPEN_SHM
	pr_sysconf("_XOPEN_SHM =", _SC_XOPEN_SHM);
#else
	printf("no symbol for _SC_XOPEN_SHM\n");
#endif
#ifdef # obsolete _XOPEN_STREAMS
	printf("# obsolete _XOPEN_STREAMS defined to be %ld\n", (long)# obsolete _XOPEN_STREAMS+0);
#else
	printf("no symbol for # obsolete _XOPEN_STREAMS\n");
#endif
#ifdef _SC_XOPEN_STREAMS
	pr_sysconf("# obsolete _XOPEN_STREAMS =", _SC_XOPEN_STREAMS);
#else
	printf("no symbol for _SC_XOPEN_STREAMS\n");
#endif
#ifdef _XOPEN_UNIX
	printf("_XOPEN_UNIX defined to be %ld\n", (long)_XOPEN_UNIX+0);
#else
	printf("no symbol for _XOPEN_UNIX\n");
#endif
#ifdef _SC_XOPEN_UNIX
	pr_sysconf("_XOPEN_UNIX =", _SC_XOPEN_UNIX);
#else
	printf("no symbol for _SC_XOPEN_UNIX\n");
#endif
#ifdef _XOPEN_UUCP
	printf("_XOPEN_UUCP defined to be %ld\n", (long)_XOPEN_UUCP+0);
#else
	printf("no symbol for _XOPEN_UUCP\n");
#endif
#ifdef _SC_XOPEN_UUCP
	pr_sysconf("_XOPEN_UUCP =", _SC_XOPEN_UUCP);
#else
	printf("no symbol for _SC_XOPEN_UUCP\n");
#endif
#ifdef _XOPEN_VERSION
	printf("_XOPEN_VERSION defined to be %ld\n", (long)_XOPEN_VERSION+0);
#else
	printf("no symbol for _XOPEN_VERSION\n");
#endif
#ifdef _SC_XOPEN_VERSION
	pr_sysconf("_XOPEN_VERSION =", _SC_XOPEN_VERSION);
#else
	printf("no symbol for _SC_XOPEN_VERSION\n");
#endif
#ifdef _POSIX_CHOWN_RESTRICTED
	printf("_POSIX_CHOWN_RESTRICTED defined to be %ld\n", (long)_POSIX_CHOWN_RESTRICTED+0);
#else
	printf("no symbol for _POSIX_CHOWN_RESTRICTED\n");
#endif
#ifdef _PC_CHOWN_RESTRICTED
	pr_pathconf("_POSIX_CHOWN_RESTRICTED =", argv[1], _PC_CHOWN_RESTRICTED);
#else
	printf("no symbol for _PC_CHOWN_RESTRICTED\n");
#endif
#ifdef _POSIX_NO_TRUNC
	printf("_POSIX_NO_TRUNC defined to be %ld\n", (long)_POSIX_NO_TRUNC+0);
#else
	printf("no symbol for _POSIX_NO_TRUNC\n");
#endif
#ifdef _PC_NO_TRUNC
	pr_pathconf("_POSIX_NO_TRUNC =", argv[1], _PC_NO_TRUNC);
#else
	printf("no symbol for _PC_NO_TRUNC\n");
#endif
#ifdef _POSIX_VDISABLE
	printf("_POSIX_VDISABLE defined to be %ld\n", (long)_POSIX_VDISABLE+0);
#else
	printf("no symbol for _POSIX_VDISABLE\n");
#endif
#ifdef _PC_VDISABLE
	pr_pathconf("_POSIX_VDISABLE =", argv[1], _PC_VDISABLE);
#else
	printf("no symbol for _PC_VDISABLE\n");
#endif
#ifdef _POSIX_ASYNC_IO
	printf("_POSIX_ASYNC_IO defined to be %ld\n", (long)_POSIX_ASYNC_IO+0);
#else
	printf("no symbol for _POSIX_ASYNC_IO\n");
#endif
#ifdef _PC_ASYNC_IO
	pr_pathconf("_POSIX_ASYNC_IO =", argv[1], _PC_ASYNC_IO);
#else
	printf("no symbol for _PC_ASYNC_IO\n");
#endif
#ifdef _POSIX_PRIO_IO
	printf("_POSIX_PRIO_IO defined to be %ld\n", (long)_POSIX_PRIO_IO+0);
#else
	printf("no symbol for _POSIX_PRIO_IO\n");
#endif
#ifdef _PC_PRIO_IO
	pr_pathconf("_POSIX_PRIO_IO =", argv[1], _PC_PRIO_IO);
#else
	printf("no symbol for _PC_PRIO_IO\n");
#endif
#ifdef _POSIX_SYNC_IO
	printf("_POSIX_SYNC_IO defined to be %ld\n", (long)_POSIX_SYNC_IO+0);
#else
	printf("no symbol for _POSIX_SYNC_IO\n");
#endif
#ifdef _PC_SYNC_IO
	pr_pathconf("_POSIX_SYNC_IO =", argv[1], _PC_SYNC_IO);
#else
	printf("no symbol for _PC_SYNC_IO\n");
#endif
#ifdef _POSIX2_SYMLINKS
	printf("_POSIX2_SYMLINKS defined to be %ld\n", (long)_POSIX2_SYMLINKS+0);
#else
	printf("no symbol for _POSIX2_SYMLINKS\n");
#endif
#ifdef _PC_2_SYMLINKS
	pr_pathconf("_POSIX2_SYMLINKS =", argv[1], _PC_2_SYMLINKS);
#else
	printf("no symbol for _PC_2_SYMLINKS\n");
#endif
	exit(0);
}

static void
pr_sysconf(char *mesg, int name)
{
	long	val;

	fputs(mesg, stdout);
	errno = 0;
	if ((val = sysconf(name)) < 0) {
		if (errno != 0) {
			if (errno == EINVAL)
				fputs(" (not supported)\n", stdout);
			else
				err_sys("sysconf error");
		} else {
			fputs(" (no limit)\n", stdout);
		}
	} else {
		printf(" %ld\n", val);
	}
}

static void
pr_pathconf(char *mesg, char *path, int name)
{
	long	val;

	fputs(mesg, stdout);
	errno = 0;
	if ((val = pathconf(path, name)) < 0) {
		if (errno != 0) {
			if (errno == EINVAL)
				fputs(" (not supported)\n", stdout);
			else
				err_sys("pathconf error, path = %s", path);
		} else {
			fputs(" (no limit)\n", stdout);
		}
	} else {
		printf(" %ld\n", val);
	}
}
