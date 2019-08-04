/* Rishav Kundu 2019 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE (128 * 1024 * 1024)
/* 128M buffer size */

#define handle_error(x)                                                        \
  do {                                                                         \
    perror(x);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void write_message(const char *s) { write(STDOUT_FILENO, s, strlen(s)); }

void reverse_buffer(char *buffer, const size_t size) {
  for (size_t i = 0; i < size / 2; i++) {
    char tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}

ssize_t write_buffer(const int fd, const char *buffer, const size_t bsize) {
  return write(fd, buffer, bsize);
}

size_t get_file_size(const int fd) {
  struct stat buf;
  if (fstat(fd, &buf) < 0)
    handle_error("fstat");
  return buf.st_size;
}

int create_destination(const char *fname) {
  char *prefix = "Assignment/";

  size_t fname_len = strlen(fname);
  size_t prefix_len = strlen(prefix);

  if (mkdir(prefix, S_IRWXU) < 0 && errno != EEXIST)
    handle_error("mkdir");

  char *dest = calloc(1, prefix_len + fname_len + 1); // 1 null byte
  memcpy(dest, prefix, prefix_len);
  memcpy(dest + prefix_len, fname, fname_len);

  int fd;
  if ((fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
    handle_error("open(dest)");

  return fd;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    write_message("usage: ./reverse source_file");
    return EXIT_FAILURE;
  }
  char *source = argv[1];

  create_destination(source);
  char *dest = argv[2];

  int source_fd, dest_fd;

  if ((source_fd = open(source, O_RDONLY)) < 0)
    handle_error("open(source)");

  dest_fd = create_destination(source);

  off_t total_size = get_file_size(source_fd), processed_size = 0;

  char *buffer = malloc(BUFFER_SIZE);

  if (buffer == NULL)
    handle_error("malloc(BUFFER_SIZE");

  off_t prev_offset, new_offset;
  if ((prev_offset = lseek(source_fd, 0, SEEK_END)) < 0)
    handle_error("lseek");

  while (prev_offset != 0) {
    new_offset = prev_offset - BUFFER_SIZE;
    if (new_offset < 0)
      new_offset = 0;

    if (lseek(source_fd, new_offset, SEEK_SET) < 0)
      handle_error("lseek(source_fd)");

    size_t expected_b = prev_offset - new_offset;
    off_t actual_b = read(source_fd, buffer, expected_b);

    if (expected_b != actual_b) {
      char *b = "partial read\n";
      write(STDERR_FILENO, b, strlen(b));
      exit(EXIT_FAILURE);
    }

    prev_offset = new_offset;

    reverse_buffer(buffer, actual_b);
    if (write_buffer(dest_fd, buffer, actual_b) < 0)
      handle_error("write_buffer");

    processed_size += actual_b;

    printf("\rprogress: %lldM/%lldM bytes (%lld%%)",
           processed_size / (1024 * 1024), total_size / (1024 * 1024),
           (processed_size * 100) / total_size);
    fflush(stdout);
  }
  printf("\n");

  free(buffer);

  if (close(source_fd) < 0)
    handle_error("close(source_fd)");

  if (close(dest_fd) < 0)
    handle_error("close(dest_fd)");
}

// vim: sts=2:sw=2
