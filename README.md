# bsdfetch

**bsdfetch** is a simple tool to show information about a running **FreeBSD/OpenBSD** system.

To build simply type **"make"** in the source directory. After that you can run it like so **"./bsdfetch"**.

**FreeBSD system example:**   
![freebsd-bsdfetch](https://user-images.githubusercontent.com/37046652/202207069-76dad8ea-1f4c-4b6e-818c-7af2f5252aa1.png)

**OpenBSD system example:**   
![openbsd-bsdfetch](https://user-images.githubusercontent.com/37046652/202207149-649af034-8497-418a-a864-520d9aa1d463.png)


**Thanks:**

**Felix Palmen** (https://twitter.com/8bitsound) - For suggestions regarding tty, initialization, color output handling. Also for contributing to the project (pkg).   
**Laurent Cheylus** (https://twitter.com/lcheylus) - For implementing "Arch" and "Memory" support for bsdfetch on OpenBSD (libsysctl). Also, for fixing a bug when no sensors are present on OpenBSD.
**Lucas Holt** (https://www.midnightbsd.org/) - For porting bsdfetch to MidnightBSD.
