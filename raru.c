#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>

#ifndef VERSION
#define VERSION "Unknown version"
#endif

void usageQuit() {
        puts("Usage: raru <program> [arguments]\n\n"
             "Runs <program> as a random UID and GID (31337-96872).\n"
             "Recommended: alias raru='env -i PATH=$PATH raru'\n"
             "Version: "
             VERSION);
        _exit(2);
}

void failQuit(char *message) {
        fprintf(stderr, "raru critical failure: %s\n", message);
        _exit(1);
}

int main(int argc, char **argv) {

        char cwd[PATH_MAX];    /* Current directory for absolute path.  */
        id_t id;               /* UID and GID that we'll switch to.     */
        int fd;                /* File descriptor for /dev/urandom.     */
        unsigned short random; /* Our random offset for the UID/GIDs.   */

	if (argc == 1)
		usageQuit();
	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		failQuit("Unable to open /dev/urandom");
	if (read(fd, &random, sizeof(random)) != sizeof(random))
		failQuit("Unable to read from /dev/urandom");
	close(fd);

	/* Don't mess with NULL pointers. This is setuid, afterall.     */

	if (getcwd(cwd, sizeof(cwd)) == NULL)
		failQuit("getcwd() failed");

	/* Have to set gid before uid.                                  */

	id = 31337 + random;
	if (setgid(id) != 0 || setuid(id) != 0) {
		failQuit("Unable to setuid(), aborting.");
	}

	/* If we can't chdir() to the cwd, it means we shouldn't be     */
	/* there in the first place. Fall back to / if that's the       */
	/* case, for security reasons.                                  */
	/*                                                              */
	/* Being super paranoid, we test and abort on chdir("/")        */
	/* failures.                                                    */

	if (chdir(cwd) != 0)
		if (chdir("/") != 0)
			failQuit("Unable to chdir(\"/\")");

	/* exec the first argument with path hunting.		 	*/

	execvp(argv[1], argv+1);

	/* If we make it this far, something broke (not in PATH, etc)   */

	failQuit("Not able to execute.");
}
