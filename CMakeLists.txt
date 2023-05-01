project(file_cmds)

cmake_minimum_required(VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# add_definitions(-DSUPPORT_UTMPX)

add_definitions(
	-DCOLORLS=1
	-D__DARWIN_UNIX03
	-U_POSIX_C_SOURCE
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_POSIX_C_SOURCE -D__DARWIN_UNIX03 -fblocks -fPIC -ggdb -O0")

add_definitions(-nostdinc)

set(BINARY_PACKAGING_MODE ON)

add_darling_executable(chflags chflags/chflags.c)
add_darling_executable(chmod chmod/chmod.c chmod/chmod_acl.c)
add_darling_executable(chown chown/chown.c)
add_darling_executable(cksum cksum/cksum.c  cksum/crc32.c  cksum/crc.c  cksum/print.c  cksum/sum1.c  cksum/sum2.c)
add_darling_executable(compress compress/compress.c  compress/zopen.c)
add_darling_executable(cp cp/cp.c cp/utils.c)
add_darling_executable(dd dd/args.c  dd/conv.c  dd/conv_tab.c  dd/dd.c  dd/misc.c  dd/position.c)
add_darling_executable(df df/df.c df/vfslist.c)
add_darling_executable(du du/du.c)

add_darling_executable(gzip gzip/gzip.c gzip/futimens.c)
target_link_libraries(gzip lzma z bz2)
target_compile_definitions(gzip PRIVATE -DGZIP_APPLE_VERSION="264.30.2")
InstallSymlink("gzip" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/gunzip")
InstallSymlink("gzip" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/gzcat")

add_darling_executable(install-bin install/xinstall.c)
#set_target_properties(install-bin PROPERTIES OUTPUT_NAME install)
add_darling_executable(ipcrm ipcrm/ipcrm.c)
#add_darling_executable(ipcs ipcs/ipcs.c) # TODO: Doesn't build
add_darling_executable(ln ln/ln.c)
add_darling_executable(ls ls/cmp.c  ls/ls.c  ls/print.c  ls/util.c)
target_link_libraries(ls util ncurses)
add_darling_executable(mkdir mkdir/mkdir.c)
add_darling_executable(mkfifo mkfifo/mkfifo.c)
add_darling_executable(mknod mknod/mknod.c)
#add_darling_executable(mtree mtree/commoncrypto.c  mtree/compare.c  mtree/create.c  mtree/excludes.c  mtree/misc.c  mtree/mtree.c  mtree/spec.c  mtree/specspec.c  mtree/verify.c) # TODO: incomplete CommonCrypto
add_darling_executable(mv mv/mv.c)
add_darling_executable(pathchk pathchk/pathchk.c)
add_darling_executable(pax pax/ar_io.c    pax/buf_subs.c  pax/cpio.c       pax/ftree.c     pax/getoldopt.c  pax/pat_rep.c  pax/pax_format.c  pax/tables.c  pax/tty_subs.c
	pax/ar_subs.c  pax/cache.c     pax/file_subs.c  pax/gen_subs.c  pax/options.c    pax/pax.c      pax/sel_subs.c    pax/tar.c)
add_darling_executable(rm rm/rm.c)
add_darling_executable(rmdir rmdir/rmdir.c)
#add_darling_executable(rmt rmt/rmt.c) # Not included in OS X
add_darling_executable(stat stat/stat.c)
add_darling_executable(touch touch/touch.c)

target_link_libraries(df util)

install(TARGETS chmod cksum cp dd df du
	ln ls mkdir mknod mv
	pax rm rmdir stat
	DESTINATION libexec/darling/bin)
install(TARGETS gzip pathchk compress chflags ipcrm
	mkfifo touch
	DESTINATION libexec/darling/usr/bin)

install(TARGETS install-bin DESTINATION libexec/darling/usr/bin)
install(CODE "execute_process(COMMAND mv install-bin install WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin)")
install(FILES compress/zcat.sh RENAME zcat
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION libexec/darling/bin)
	install(FILES gzip/zdiff gzip/zmore gzip/znew gzip/zforce gzip/gzexe
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
		                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		DESTINATION libexec/darling/bin)
install(FILES shar/shar.sh RENAME shar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION libexec/darling/bin)

install(TARGETS gzip DESTINATION libexec/darling/usr/bin)

install(TARGETS chown DESTINATION libexec/darling/usr/sbin)
InstallSymlink(../sbin/chown libexec/darling/usr/bin/chgrp)
InstallSymlink(rm libexec/darling/bin/unlink)
InstallSymlink(stat libexec/darling/bin/readlink)

InstallSymlink(gzip libexec/darling/usr/bin/gunzip)
InstallSymlink(gzip libexec/darling/usr/bin/gzcat)
InstallSymlink(gzip libexec/darling/usr/bin/zcat)
InstallSymlink(gzip libexec/darling/usr/bin/gunzip)
InstallSymlink(zdiff libexec/darling/usr/bin/zcmp)
InstallSymlink(zless libexec/darling/usr/bin/zmore)

# Manpages
install(FILES
	mkdir/mkdir.1
	mv/mv.1
	dd/dd.1
	touch/touch.1
	chmod/chmod.1
	df/df.1
	ln/link.1
	ln/ln.1
	compress/uncompress.1
	compress/compress.1
	gzip/zforce.1
	gzip/zdiff.1
	gzip/zmore.1
	gzip/gzexe.1
	gzip/znew.1
	gzip/gzip.1
	cp/cp.1
	ls/ls.1
	rm/unlink.1
	rm/rm.1
	stat/stat.1
	stat/readlink.1
	chflags/chflags.1
	chown/chgrp.1
	rmdir/rmdir.1
	shar/shar.1
	pathchk/pathchk.1
	#pax/cpio.1
	pax/pax.1
	ipcs/ipcs.1
	mkfifo/mkfifo.1
	du/du.1
	ipcrm/ipcrm.1
	install/install.1
	cksum/cksum.1
	cksum/sum.1

	DESTINATION libexec/darling/usr/share/man/man1
)

install(FILES ln/symlink.7 DESTINATION libexec/darling/usr/share/man/man7)

install(FILES
	mtree/mtree.8
	mknod/mknod.8
	chown/chown.8

	DESTINATION libexec/darling/usr/share/man/man8
)
