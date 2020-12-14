#include <sys/sendfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>

static const char installdir[] = "/usr/local/bin/";
#define EFAIL EXIT_FAILURE

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		error(EFAIL, 0, "missing file operand");
	}

	int out = -1;
	{
		size_t pathlen = sizeof(installdir) + strlen(argv[1]); // sizeof accounts NULL so we have room for newpath NULL
		char *newpath = (char *)calloc(pathlen, sizeof(char));
		if (!newpath)
		{
			error(EFAIL, 0, "unable to allocate memory");
		}
		snprintf(newpath, pathlen, "%s%s", installdir, argv[1]);
		newpath[pathlen - 1] = '\0'; // Ensure it ends with NULL

		out = creat(newpath, 0777);
		free(newpath);
		newpath = NULL;
		if (out == -1)
		{
			if (errno == EACCES)
			{
				error(EFAIL, errno, "not in root");
			}
			else
			{
				error(EFAIL, errno, "failed to create a file in %s", installdir);
			}
		}
	}

	int in = open(argv[1], O_RDONLY);
	if (in == -1)
	{
		error(EFAIL, errno, "failed to access file \"%s\"", argv[1]);
	}

	{
		struct stat statbuf;
		if (fstat(in, &statbuf) == -1)
		{
			error(EFAIL, errno, "failed to read file size");
		}

		if (sendfile(out, in, NULL, statbuf.st_size) == -1)
		{
			error(EFAIL, errno, "failed to copy the file");
		}
	}

	if (close(in) == -1)
	{
		error(EFAIL, errno, "failed to close the file");
	}
	if (close(out) == -1)
	{
		error(EFAIL, errno, "failed to close the file");
	}

	printf("globalize: installed %s\n", argv[1]);

	return EXIT_SUCCESS;
}