/** \file os/vxworks5.5/code/os_stdlib_mkdir.c
 *  \brief wrapper to solve interface differents
 *
 */

os_int32
os_mkdir(
    const char *path,
    mode_t mode)
{
    return (mkdir(path, mode));
}

