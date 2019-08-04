#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE (1024 * 1024 * 1024)
/* 128M buffer size */

void write_message(const char *s) { write(STDOUT_FILENO, s, strlen(s)); }

void reverse_buffer(char *buffer, size_t size) {
  for (size_t i = 0; i < size / 2; i++) {
    char tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}

int write_buffer(int fd, const char *buffer, size_t bsize) {
  return write(fd, buffer, bsize);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    char *message = "usage ./reverse source_file dest_file";
    write(STDOUT_FILENO, message, strlen(message));
    return 0;
  }
  char *source = argv[1];
  char *dest = argv[2];

  int source_fd = open(source, O_RDONLY);
  int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

  if (source_fd < 0 || dest_fd < 0) {
    perror("main");
    return 1;
  }

  off_t offset = lseek(source_fd, 0, SEEK_END);
  char *buffer = malloc(BUFFER_SIZE);

  if (buffer == NULL) {
    write_message("failed to allocate temporary buffer\n");
    exit(1);
  }

  off_t total_size = offset;
  off_t processed_size = 0;

  while (offset > 0) {

    off_t prev_offset = offset;
    if (offset < BUFFER_SIZE)
      offset = lseek(source_fd, 0, SEEK_SET);
    else
      offset = lseek(source_fd, -BUFFER_SIZE, SEEK_CUR);

    off_t cur_block_offset = offset;

    off_t nbytes = read(source_fd, buffer, prev_offset - offset);

    offset = cur_block_offset;

    if (nbytes < 0) {
      perror(__FILE__);
      free(buffer);
      return 1;
    }

    reverse_buffer(buffer, nbytes);
    if (write_buffer(dest_fd, buffer, nbytes) < 0) {
      perror("write_buffer");
      free(buffer);
      exit(1);
    }

    processed_size += nbytes;

    printf("\rprogress: %lld/%lld bytes (%lld %%)", processed_size, total_size,
           processed_size * 100 / total_size);
    fflush(stdout);
  }
  printf("\n");

  free(buffer);
}

// vim: sts=2:sw=2
