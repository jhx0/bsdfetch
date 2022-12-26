/*
 * Copyright (c) 2022 jhx <jhx0x00@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__MidnightBSD__)
#include <time.h>
#include <sys/vmmeter.h>
#include <vm/vm_param.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/time.h>
#endif
#ifdef __OpenBSD__
#include "sysctlbyname.h"
#include <sys/sensors.h>
#else
#include <machine/param.h>
#endif

#define _PRG_NAME "bsdfetch"
#define _VERSION "1.0.2"

/* Bright red */
#define COLOR_RED "\x1B[91m"
/* Bright green */
#define COLOR_GREEN "\x1B[92m"
/* Reset color */
#define COLOR_RESET "\x1B[0m"

#define CELSIUS 273.15

#define _SILENT (void)

typedef unsigned int uint;

char buf[256] = {0};
size_t buf_size = 0;

int ret = 0;
int color_flag = 1;

static void die(int err_num, int line);
static void error(const char *msg, int line);
static void show(const char *entry, const char *text);
static void get_shell(void);
static void get_user(void);
static void get_cpu(void);
static void get_loadavg(void);
static void get_packages(void);
static void get_uptime(void);
static void get_memory(void);
static void get_hostname(void);
static void get_arch(void);
static void get_sysinfo(void);
static void version(void);
static void usage(void);

static void die(int err_num, int line) {
	_SILENT fprintf(stderr, "[ERROR] [Line: %d - Errno: %d] %s\n", line, err_num, strerror(err_num));

	exit(EXIT_FAILURE);
}

static void error(const char *msg, int line) {
	_SILENT fprintf(stderr, "[ERROR] [Line: %d] %s\n", line, msg);

	exit(EXIT_FAILURE);
}

static void show(const char *entry, const char *text) {
	if(color_flag) {
		_SILENT fprintf(stdout, "%s%s:%s %s\n",
						COLOR_RED, entry, COLOR_RESET,
						text);
	} else {
		_SILENT fprintf(stdout, "%s: %s\n",
						entry,
						text);
	}
}

static void get_shell(void) {
	char *sh;
	char *p;
	const char c = '/';
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);

	if (getenv("SHELL")) {
		sh = getenv("SHELL");
	} else {
		if ((sh = getenv("SHELL")) == NULL || *sh == '\0') {
			if (pw == NULL)
				die(errno, __LINE__);
			sh = pw->pw_shell;
		}
	}

	if ((p = strrchr(sh, c)) != NULL && *(p+1) != '\0')
		sh = ++ p;

	show("Shell", sh);
}

static void get_user(void) {
	char *user;
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);

	if (getenv("USER")) {
		user = getenv("USER");
	} else {
		if ((user = getenv("USER")) == NULL || *user == '\0') {
			if (pw == NULL)
				die(errno, __LINE__);
			user = pw->pw_name;
		}
	}

	show("User", user);
}

static void get_cpu(void) {
	size_t cpu_type_size = 0;
	uint ncpu = 0;
	uint ncpu_max = 0;
	char cpu_type[200] = {0};

	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	ncpu_max = sysconf(_SC_NPROCESSORS_CONF);

	if (ncpu_max <= 0 || ncpu <= 0)
		error("No CPU's found", __LINE__);

	cpu_type_size = sizeof(char) * 200;
	if(sysctlbyname("machdep.cpu_brand", &cpu_type, &cpu_type_size, NULL, 0) == -1)
		if(sysctlbyname("hw.model", &cpu_type, &cpu_type_size, NULL, 0) == -1)
			die(errno, __LINE__);

	show("CPU", cpu_type);

	buf_size = sizeof(buf);

	ret = snprintf(buf, buf_size, "%d of %d processors online", ncpu, ncpu_max);
	if (ret < 0)
		error("Could not write data to buffer", __LINE__);

	show("Cores", buf);

#if defined(__FreeBSD__) || defined(__MidnightBSD__) || defined(__DragonFly__)
	for(uint i = 0; i < ncpu; i++) {
		size_t temp_size = 0;
		int temp = 0;
		int ret_t = 0;

		temp_size = sizeof(buf);
		ret_t = snprintf(buf, temp_size, "dev.cpu.%d.temperature", i);
		if (ret_t < 0 || (size_t) ret_t >= buf_size)
			error("Could not write data to buffer", __LINE__);

		if(sysctlbyname(buf, &temp, &temp_size, NULL, 0) == -1)
			return;

		_SILENT fprintf(stdout, " %s->%s %sCore [%d]:%s %.1f °C\n",
						COLOR_GREEN, COLOR_RESET,
						COLOR_RED, i + 1, COLOR_RESET,
						(temp * 0.1) - CELSIUS);
	}
#elif defined(__OpenBSD__)
	int mib[5];
	char temp[10] = {0};
	size_t size = 0;
	size_t temp_size = 0;
	struct sensor sensors;
	int ret_t = 0;

	mib[0] = CTL_HW;
	mib[1] = HW_SENSORS;
	mib[2] = 0;
	mib[3] = SENSOR_TEMP;
	mib[4] = 0;

	size = sizeof(sensors);
	temp_size = sizeof(temp);

	if(sysctl(mib, 5, &sensors, &size, NULL, 0) < 0)
		return;

	ret_t = snprintf(temp, temp_size, "%d °C", (int)((float)(sensors.value - 273150000) / 1E6));
	if (ret_t < 0 || (size_t) ret_t >= temp_size)
		error("Could not write data to buffer", __LINE__);

	show("CPU Temp", temp);

#elif defined(__NetBSD__)
	const char del[] = ".";
	char *temp = "";
	int ret_t = 0;

	FILE *f = NULL;
	f = popen("/usr/sbin/envstat | awk '/cpu[0-9]/ {printf $3}'", "r");
	if (f == NULL)
		die(errno, __LINE__);

	if (fgets(buf, buf_size, f) != NULL)
		if ((temp = strtok(buf, del)) != NULL && *temp != '\0')
	if (pclose(f) != 0)
		die(errno, __LINE__);

	ret_t = snprintf(buf, buf_size, "%s °C", temp);
	if (ret_t < 0 || (size_t) ret_t >= buf_size)
		error("Could not write data to buffer", __LINE__);

	show("CPU Temp", buf);

#endif
}

static void get_loadavg(void) {
	double lavg[3] = { 0.0 };

	(void)getloadavg(lavg, 3);

	ret = snprintf(buf, buf_size, "%.2lf %.2lf %.2lf", lavg[0], lavg[1], lavg[2]);
	if (ret < 0 || (size_t) ret >= buf_size)
		error("Could not write data to buffer", __LINE__);

	show("Loadavg", buf);
}

static void get_packages(void) {
	FILE *f = NULL;
	size_t npkg = 0;

#if defined(__OpenBSD__) || defined(__NetBSD__)
	f = popen("/usr/sbin/pkg_info", "r");
#elif defined( __DragonFly__)
	f = popen("/usr/sbin/pkg info", "r");
#elif defined(__FreeBSD__)
	if(access("/usr/local/sbin/pkg", F_OK) == 0) {
		f = popen("/usr/local/sbin/pkg info", "r");
	} else {
		npkg = 0;
		goto no_pkg;
	}
#elif defined( __MidnightBSD__)
	f = popen("/usr/sbin/mport list", "r");
#else
	error("Could not determine BSD variant", __LINE__);
#endif
	if(f == NULL)
		die(errno, __LINE__);

	while (fgets(buf, sizeof buf, f) != NULL)
		if (strchr(buf, '\n') != NULL)
			npkg++;
	if (pclose(f) != 0)
		die(errno, __LINE__);

#ifdef __FreeBSD__
no_pkg:
#endif
	ret = snprintf(buf, buf_size, "%zu", npkg);
	if (ret < 0 || (size_t) ret >= buf_size)
		error("Could not write data to buffer", __LINE__);

	show("Packages", buf);
}

static void get_uptime(void) {
	int up = 0;
	int res = 0;
	int days = 0;
	int hours = 0;
	int minutes = 0;
	struct timespec t;
	size_t tsz = sizeof t;

	if ((res = sysctlbyname("kern.boottime", &t, &tsz, NULL, 0)) == -1)
		die(errno, __LINE__);

	up = time(NULL) - t.tv_sec + 30;
	days = up / 86400;
	up %= 86400;
	hours = up / 3600;
	up %= 3600;
	minutes = up / 60;

	ret = snprintf(buf, buf_size, "%dd %dh %dm", days, hours, minutes);
	if (ret < 0 || (size_t) ret >= buf_size)
		error("Could not write data to buffer", __LINE__);

	show("Uptime", buf);
}

static void get_memory(void) {
	unsigned long long buff = 0;
	unsigned long long mem = 0;
	const long pgsize = sysconf(_SC_PAGESIZE);
	const long pages = sysconf(_SC_PHYS_PAGES);

	if (pgsize == -1 || pages == -1)
		error("Could not determine pagesize", __LINE__);
	else
		buff = (uint64_t)pgsize * (uint64_t)pages;

	if (buff <= 0)
		error("Pagesize is less then zero", __LINE__);
	else
		mem = buff / 1048576;

	ret = snprintf(buf, buf_size, "%llu MB", mem);
	if (ret < 0 || (size_t) ret >= buf_size)
		error("Could not write data to buffer", __LINE__);

	show("RAM", buf);
}

static void get_hostname(void) {
	long host_size_max = 0;
	char hostname[_POSIX_HOST_NAME_MAX + 1];

	host_size_max = sysconf(_SC_HOST_NAME_MAX) + 1;
	if(gethostname(hostname, host_size_max) == -1)
		die(errno, __LINE__);

	show("Host", hostname);
}

static void get_arch(void) {
	struct utsname un;

	if(uname(&un))
		die(errno, __LINE__);

	show("Arch", un.machine);
}

static void get_sysinfo(void) {
	struct utsname un;

	if(uname(&un))
		die(errno, __LINE__);

	show("OS", un.sysname);
	show("Release", un.release);
	show("Version", un.version);
}

static void version(void) {
	_SILENT fprintf(stdout, "%s - version %s (%s)\n",
			_PRG_NAME,
			__DATE__
			);

	exit(EXIT_SUCCESS);
}

static void usage(void) {
	_SILENT fprintf(stdout, "USAGE: %s [-n|-v|-h]\n", _PRG_NAME);
	_SILENT fprintf(stdout, "   -n  Turn off colors\n");
	_SILENT fprintf(stdout, "   -v  Show version\n");
	_SILENT fprintf(stdout, "   -h  Show this help text\n");

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

	buf_size = sizeof(buf);

	int is_a_tty = 0;

	is_a_tty = isatty(1);

	if(is_a_tty) {
		if(argc == 2 && (strcmp(argv[1], "-n") == 0)) {
			color_flag = 0;
		}
	} else {
		color_flag = 0;
	}

	if(argc == 2 && (strcmp(argv[1], "-v") == 0))
		version();

	if(argc == 2 && (strcmp(argv[1], "-h") == 0))
		usage();

	get_sysinfo();
	get_hostname();
	get_arch();
	get_shell();
	get_user();
	get_packages();
	get_uptime();
	get_memory();
	get_loadavg();
	get_cpu();

	return EXIT_SUCCESS;
}
