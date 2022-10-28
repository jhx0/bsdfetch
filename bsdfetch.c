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
#ifndef __FreeBSD__
#error "This application will only work on FreeBSD!"
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#include <sys/utsname.h>
#include <vm/vm_param.h>

#define COLOR_RED "\x1B[31m"
#define COLOR_GREEN "\x1B[32m"
#define COLOR_RESET "\x1B[0m"

#define CLEAR(s) \
		memset(s, '\0', strlen(s));

#define CELSIUS 273.15

#define _SILENT (void)

typedef unsigned int uint;

static void die();
static void show(const char *entry, const char *text);
static void get_shell();
static void get_user();
static void get_cpu();
static void get_loadavg();
static void get_packages();
static void get_uptime();
static void get_memory();
static void get_hostname();
static void get_arch();
static void get_sysinfo();

static void die(int err_num) {
	fprintf(stderr, "Error: %s\n", strerror(err_num));

	exit(EXIT_FAILURE);
}

static void show(const char *entry, const char *text) {
	_SILENT fprintf(stdout, "%s%s:%s %s\n", 
					COLOR_RED, entry, COLOR_RESET, 
					text);
}

static void get_shell() {
	char home[128];

	CLEAR(home);

	show("Shell", getenv("SHELL"));
}

static void get_user() {
	char user[128];

	CLEAR(user);

	show("User", getenv("USER"));
}

static void get_cpu() {
	size_t num_cpu_size = 0;
	size_t cpu_type_size = 0;
	uint num_cpu = 0;
	char cpu_type[200];
	char tmp[100];

	num_cpu_size = sizeof(num_cpu);
	if(sysctlbyname("hw.ncpu", &num_cpu, &num_cpu_size, NULL, 0) == -1)
		die(errno);
	
	cpu_type_size = sizeof(char) * 200;
	if(sysctlbyname("hw.model", &cpu_type, &cpu_type_size, NULL, 0) == -1)
		die(errno);

	show("CPU", cpu_type);

	CLEAR(tmp);
	_SILENT sprintf(tmp, "%d", num_cpu);

	show("Cores", tmp);

	for(uint i = 0; i < num_cpu; i++) {
		size_t temperature_size = 0;
		char buf[100];
		int temperature = 0;

		CLEAR(buf);
		sprintf(buf, "dev.cpu.%d.temperature", i);	

		temperature_size = sizeof(buf);
		if(sysctlbyname(buf, &temperature, &temperature_size, NULL, 0) == -1)
			return;

		_SILENT fprintf(stdout, " %s->%s %sCore [%d]:%s %.1f °C\n", 
						COLOR_GREEN, COLOR_RESET,
						COLOR_RED, i + 1, COLOR_RESET,
						(temperature * 0.1) - CELSIUS);
	}
}

static void get_loadavg() {
	char tmp[20];
	double *lavg = NULL;

	lavg = malloc(sizeof(double) * 3);

	(void)getloadavg(lavg, -1);

	CLEAR(tmp);
	_SILENT sprintf(tmp, "%.2lf %.2lf %.2lf", lavg[0], lavg[1], lavg[2]);

	show("Loadavg", tmp);
}

static void get_packages() {
	FILE *f = NULL;
	char buf[10];

	f = popen("/usr/sbin/pkg info | wc -l | sed 's/ //g' | tr -d '\n'", "r");
	if(f == NULL)
		die(errno);

	CLEAR(buf);
	fgets(buf, sizeof(buf), f);
	pclose(f);

	show("Packages", buf);
}

static void get_uptime() {
	char buf[100];
	int up = 0;
	int ret = 0;
	int days = 0;
	int hours = 0;
	int minutes = 0;
	struct timespec t;

	ret = clock_gettime(CLOCK_UPTIME, &t);
	if(ret == -1)
		die(errno);

	up = t.tv_sec;
	days = up / 86400;
	up %= 86400;
	hours = up / 3600;
	up %= 3600;
	minutes = up / 60;

	CLEAR(buf);
	_SILENT sprintf(buf, "%dd %dh %dm", days, hours, minutes);

	show("Uptime", buf);
}

static void get_memory() {
	unsigned long long buf = 0;
	unsigned long long mem = 0;
	size_t buf_size = 0;
	char tmp[50];

	buf_size = sizeof(buf);
	if(sysctlbyname("hw.realmem", &buf, &buf_size, NULL, 0) == -1)
		die(errno);

	mem = buf / 1048576;

	CLEAR(tmp);
	_SILENT sprintf(tmp, "%llu MB", mem);

	show("RAM", tmp);
}

static void get_hostname() {
	long host_size_max = 0;
	char hostname[15];

	CLEAR(hostname);

	host_size_max = sysconf(_SC_HOST_NAME_MAX);
	if(gethostname(hostname, host_size_max) == -1)
		die(errno);

	show("Host", hostname);
}

static void get_arch() {
	char buf[10];
	size_t buf_size = 0;

	CLEAR(buf);

	buf_size = sizeof(buf);
	if(sysctlbyname("hw.machine_arch", &buf, &buf_size, NULL, 0) == -1)
		die(errno);

	show("Arch", buf);
}

static void get_sysinfo() {
	struct utsname un;
	
	if(uname(&un))
		die(errno);

	show("OS", un.sysname);
	show("Release", un.release);
	show("Version", un.version);
}

int main(void) {
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
