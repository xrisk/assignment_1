#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define handle_error(msg)                                                      \
  do {                                                                         \
    printf("Line %d", __LINE__);                                               \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

off_t check_file_size(int fd) {
  struct stat buf;
  if (fstat(fd, &buf) == -1)
    handle_error("stat");
  printf("optimal block size: %d\n", buf.st_blksize);
  return buf.st_size;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    const char *msg = "usage: ./reverse file_1 file_2";
    write(STDOUT_FILENO, msg, strlen(msg));
    return EXIT_FAILURE;
  }

  int source_fd = open(argv[1], O_RDONLY);

  if (source_fd < 0)
    handle_error("source_fd");

  off_t fsize = check_file_size(source_fd);

  int dest_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

  if (ftruncate(dest_fd, fsize) < 0)
    handle_error("ftruncate");

  if (dest_fd < 0)
    handle_error("dest_fd");

  char *ptr = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, source_fd, 0);

  if (ptr == MAP_FAILED)
    handle_error("mmap");

  char *ptr_dest =
      mmap(NULL, fsize, PROT_WRITE, MAP_FILE | MAP_SHARED, dest_fd, 0);

  if (ptr_dest == MAP_FAILED)
    handle_error("mmap");

  size_t progress_prev_mb = 0;
  size_t fsize_mb = fsize / (1024 * 1024);
  for (off_t pos = 0; pos < fsize; ++pos) {
    ptr_dest[fsize - pos - 1] = ptr[pos];
    size_t progress_cur_mb = pos / (1024 * 1024);
    if (progress_cur_mb != progress_prev_mb) {
      printf("\rprogress: %zuM/%zuM (%lu%%)", progress_cur_mb, fsize_mb,
             (progress_cur_mb * 100) / fsize_mb);
      fflush(stdout);
      progress_prev_mb = progress_cur_mb;
    }
  }
  printf("\n");

  if (msync(ptr_dest, fsize, MS_SYNC) < 0)
    handle_error("msync");

  if (munmap(ptr, fsize) < 0)
    handle_error("munmap");
  if (munmap(ptr_dest, fsize) < 0)
    handle_error("munmap");

  if (close(source_fd) < 0)
    handle_error("close(source_fd)");
  if (close(dest_fd) < 0)
    handle_error("close(dest_fd)");
}

// vim: sts=2:sw=2
