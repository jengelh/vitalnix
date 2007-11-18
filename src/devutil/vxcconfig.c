#include <stdio.h>
#include <stdlib.h>
#include <vitalnix/config.h>

int main(void)
{
	printf(
		"Product:          %s\n"
		"Release version:  %s\n"
		"Snapshot version: %s\n"
		"--sysconfdir:     %s\n"
		"--libdir:         %s\n"
		,
		PACKAGE_NAME,
		PACKAGE_VERSION_SIMPLE,
		PACKAGE_VERSION,
		CONFIG_SYSCONFDIR,
		CONFIG_LIBDIR
	);
	return EXIT_SUCCESS;
}
