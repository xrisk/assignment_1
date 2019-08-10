#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void write_message(const char *s) { write(STDOUT_FILENO, s, strlen(s)); }

#define BUFFER_SIZE (1024 * 1024)

#define die(x)                                                                 \
  do {                                                                         \
    write_message(x);                                                          \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define handle_error(x)                                                        \
  do {                                                                         \
    perror(x);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

size_t get_file_size(const int fd) {
  struct stat buf;
  if (fstat(fd, &buf) < 0)
    handle_error("fstat");
  return buf.st_size;
}

void check_reverse(const int f1, const int f2) {

  size_t fsize1 = get_file_size(f1);
  size_t fsize2 = get_file_size(f2);

  if (fsize1 != fsize2) {
    write_message("file contents not equal!\n");
    return;
  }

  char *buf1, *buf2;
  if ((buf1 = mmap(NULL, fsize1, PROT_READ, MAP_PRIVATE, f1, 0)) == MAP_FAILED)
    handle_error("mmap");
  if ((buf2 = mmap(NULL, fsize2, PROT_READ, MAP_PRIVATE, f2, 0)) == MAP_FAILED)
    handle_error("mmap");

  off_t pos = 0;

  int flag = 1;
  for (off_t idx = 0; idx < fsize1 / 2; ++idx) {
    if (buf1[idx] != buf2[fsize2 - idx - 1]) {
      flag = 0;
      break;
    }
  }
  if (flag)
    write_message("Whether file contents are reversed in new file: Yes\n");
  else
    write_message("Whether file contents are reversed in new file: No\n");

  if (munmap(buf1, fsize1) < 0)
    handle_error("munmap");
  if (munmap(buf2, fsize2) < 0)
    handle_error("munmap");
}

void display_perms(const int fd, const char *name) {
  struct stat buf;

  if (fstat(fd, &buf) < 0) {
    if (errno == EBADF) {
      write_message(name);
      write_message(" does not exist");
    } else
      handle_error(name);
    return;
  }

  char *categories[3] = {"user", "group", "others"};

  for (int i = 0; i < 3; i++) {
    mode_t rmask, wmask, xmask;

    write_message("*****************************\n");

    if (i == 0) {
      rmask = S_IRUSR, wmask = S_IWUSR, xmask = S_IXUSR;
    } else if (i == 1) {
      rmask = S_IRGRP, wmask = S_IWGRP, xmask = S_IXGRP;
    } else {
      rmask = S_IROTH, wmask = S_IWOTH, xmask = S_IXOTH;
    }

    write_message(categories[i]);
    write_message(" has read permissions on ");
    write_message(name);
    write_message(": ");
    if (buf.st_mode & rmask)
      write_message("Yes\n");
    else
      write_message("No\n");

    write_message(categories[i]);
    write_message(" has write permissions on ");
    write_message(name);
    write_message(": ");
    if (buf.st_mode & wmask)
      write_message("Yes\n");
    else
      write_message("No\n");

    write_message(categories[i]);
    write_message(" has execute permissions on ");
    write_message(name);
    write_message(": ");
    if (buf.st_mode & xmask)
      write_message("Yes\n");
    else
      write_message("No\n");
  }
}

void check_perms(const int dir_fd, const int fd_1, const int fd_2) {
  struct stat buf;

  if (fstat(dir_fd, &buf) == 0) {
    write_message("directory exists\n");
  } else
    write_message("directory does not exist\n");

  check_reverse(fd_1, fd_2);

  display_perms(fd_1, "oldfile");
  display_perms(fd_2, "newfile");
  display_perms(dir_fd, "directory");
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    die("usage: ./Q2 dir f1 f2\n");
  }

  int fd1, fd2, fd3;
  fd1 = open(argv[1], O_RDONLY);
  fd2 = open(argv[2], O_RDONLY);
  fd3 = open(argv[3], O_RDONLY);

  check_perms(fd1, fd2, fd3);
}

// vim: sts=2:sw=2
