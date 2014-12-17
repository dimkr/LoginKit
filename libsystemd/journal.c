#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include "journal.h"

__attribute__((visibility("default")))
int sd_journal_printv(int priority, const char *format, va_list ap)
{
	vsyslog(priority, format, ap);
	return 0;
}

__attribute__((visibility("default")))
int sd_journal_print(int priority, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = sd_journal_printv(priority, format, ap);
	va_end(ap);

	return ret;
}

__attribute__((visibility("default")))
int sd_journal_perror(const char *message)
{
	char fmt[MAX_FMT_LEN];
	int len;
	char *err;

	err = strerror(errno);
	if (NULL == err)
		return -EINVAL;

	if (NULL == message)
		return sd_journal_print(LOG_ERR, err);

	len = snprintf(fmt, sizeof(fmt), "%s: %%s", message);
	if ((0 >= len) || (sizeof(fmt) <= len))
		return -EINVAL;

	return sd_journal_print(LOG_ERR, fmt, err);
}
