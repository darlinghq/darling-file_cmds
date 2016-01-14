project(file_cmds)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include(darling_exe)
add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
# add_definitions(-DSUPPORT_UTMPX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_POSIX_C_SOURCE -D__DARWIN_UNIX03 -nostdinc -fblocks -fPIC -ggdb -O0")
set(CMAKE_EXE_LINKER_FLAGS "-nodefaultlibs -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")
#set(CMAKE_SHARED_LINKER_FLAGS "-nodefaultlibs -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include/FreeBSD)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include/NetBSD)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/gen)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/darwin)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/stdtime/FreeBSD)
include_directories(${DARLING_TOP_DIRECTORY}/src/launchd/liblaunch)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/membership.subproj)
include_directories(${DARLING_TOP_DIRECTORY}/src/copyfile)
include_directories(${DARLING_TOP_DIRECTORY}/src/libutil)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/zlib)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/bzip2)
include_directories(${DARLING_TOP_DIRECTORY}/src/CommonCrypto)
include_directories(${DARLING_TOP_DIRECTORY}/src/libremovefile)

include(InstallSymlink)
set(BINARY_PACKAGING_MODE ON)

add_darling_executable(chflags chflags/chflags.c)
add_darling_executable(chmod chmod/chmod.c chmod/chmod_acl.c)
add_darling_executable(chown chown/chown.c)
add_darling_executable(cksum cksum/cksum.c  cksum/crc32.c  cksum/crc.c  cksum/print.c  cksum/sum1.c  cksum/sum2.c)
add_darling_executable(compress compress/compress.c  compress/zopen.c)
add_darling_executable(cp cp/cp.c cp/utils.c)
add_darling_executable(dd dd/args.c  dd/conv.c  dd/conv_tab.c  dd/dd.c  dd/misc.c  dd/position.c)
add_darling_executable(du du/du.c)
# add_darling_executable(gzip gzip/gzip.c) # TODO: missing liblzma
add_darling_executable(install-bin install/xinstall.c)
set_target_properties(install-bin PROPERTIES OUTPUT_NAME install)
add_darling_executable(ipcrm ipcrm/ipcrm.c)
#add_darling_executable(ipcs ipcs/ipcs.c) # TODO: Doesn't build
add_darling_executable(ln ln/ln.c)
add_darling_executable(ls ls/cmp.c  ls/ls.c  ls/print.c  ls/util.c)
target_link_libraries(ls util)
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

install(TARGETS chflags chmod chown cksum compress cp dd du
	install-bin ipcrm ln ls mkdir mkfifo mknod mv pathchk
	pax rm rmdir stat touch
	DESTINATION libexec/darling/bin)
install(FILES compress/zcat.sh RENAME zcat
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION libexec/darling/bin)
install(FILES shar/shar.sh RENAME shar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION libexec/darling/bin)
InstallSymlink(chown libexec/darling/bin/chgrp)
InstallSymlink(rm libexec/darling/bin/unlink)
InstallSymlink(stat libexec/darling/bin/readlink)

