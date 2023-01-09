/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include <monero_seed/secure_random.hpp>
#include <stdexcept>
#include <cstdlib>

#if defined(_WIN32) || defined(__CYGWIN__)
	#define WINAPI
	#include <windows.h>
	#include <ntsecapi.h>
#elif defined __linux__ && defined __GLIBC__
	#define STRINGIFY(x) #x
	#define STR(x) STRINGIFY(x)
	#if __GLIBC__ > 2 || __GLIBC_MINOR__ > 24
		#define LINUX_GETENTROPY
		#include <sys/random.h>
	#else
		#pragma message("Warning: getentropy is not available in GLIBC " \
			STR(__GLIBC__) "." STR(__GLIBC_MINOR__))
		#define UNIX_FALLBACK
		#include <sys/syscall.h>
		#if defined(SYS_getrandom)
			#define LINUX_TRY_SYSCALL
			#include <sys/syscall.h>
			#include <errno.h>
		#else
			#pragma message("Warning: Kernel doesn't support SYS_getrandom")
		#endif
	#endif
#else
	#define UNIX_FALLBACK
#endif
#if defined(UNIX_FALLBACK)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define RANDOM_FILE "/dev/urandom"
#endif

void secure_random::gen_bytes(void* output, size_t size) {
#if defined(WINAPI)
	if (!RtlGenRandom(output, size)) {
		throw std::runtime_error("RtlGenRandom failed");
	}
#elif defined(LINUX_GETENTROPY)
	if (-1 == getentropy(output, size)) {
		throw std::runtime_error("getentropy failed");
	}
#else
#if defined(LINUX_TRY_SYSCALL)
	if (size <= 256) {
		if (0 == syscall(SYS_getrandom, output, size, 0)) {
			return;
		}
	}
#endif
	int fd = open(RANDOM_FILE, O_RDONLY);
	if (fd == -1) {
		throw std::runtime_error("Unable to open " RANDOM_FILE);
	}
	char* outptr = (char*)output;
	while (size) {
		ssize_t len = read(fd, outptr, size);
		if (len < 0) {
#ifndef __APPLE__
			if (errno != EINTR && errno != EAGAIN) {
				break;
			}
#endif
			continue;
		}
		outptr += len;
		size -= len;
	}
	close(fd);
	if (size) {
		throw std::runtime_error("Unable to read " RANDOM_FILE);
	}
#endif
}
