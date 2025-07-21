#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  for (int i = 0; i < dbhdr->count; i++) {
    printf("Employee %d:\n\t Name: %s, Address: %s, Hours: %d\n", i,
           employees[i].name, employees[i].address, employees[i].hours);
  }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                 char *addstring) {
  char *name = strtok(addstring, ",");
  char *address = strtok(NULL, ",");
  char *hours = strtok(NULL, ",");

  int last_employee_count = dbhdr->count - 1;

  strncpy(employees[last_employee_count].name, name,
          sizeof(employees[last_employee_count].name));

  strncpy(employees[last_employee_count].address, address,
          sizeof(employees[last_employee_count].address));

  employees[last_employee_count].hours = atoi(hours);

  return STATUS_SUCCESS;
}

int delete_employee_by_id(struct dbheader_t *dbhdr,
                          struct employee_t *employees, int id) {}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Invalid file descriptor.\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  if (count == 0) {
    printf("No employees in the database.\n");
    return STATUS_SUCCESS;
  }

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }

  if (read(fd, employees, sizeof(struct employee_t) * count) !=
      sizeof(struct employee_t) * count) {
    perror("read");
    free(employees);
    return STATUS_ERROR;
  }

  for (int i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  if (fd < 0) {
    printf("Invalid file descriptor.\n");
    return STATUS_ERROR;
  }

  struct dbheader_t hdr_to_write = *dbhdr;
  hdr_to_write.magic = htonl(dbhdr->magic);
  hdr_to_write.filesize = htonl(dbhdr->filesize);
  hdr_to_write.count = htons(dbhdr->count);
  hdr_to_write.version = htons(dbhdr->version);

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("lseek");
    return STATUS_ERROR;
  }

  if (write(fd, &hdr_to_write, sizeof(struct dbheader_t)) !=
      sizeof(struct dbheader_t)) {
    perror("write");
    return STATUS_ERROR;
  }

  for (int i = 0; i < dbhdr->count; i++) {
    struct employee_t emp_to_write = employees[i];
    emp_to_write.hours = htonl(employees[i].hours);
    if (write(fd, &emp_to_write, sizeof(struct employee_t)) !=
        sizeof(struct employee_t)) {
      perror("write");
      return STATUS_ERROR;
    }
  }

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Invalid file descriptor.\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

  if (header == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) !=
      sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    printf("Invalid database header magic number: %x\n", header->magic);
    free(header);
    return STATUS_ERROR;
  }

  if (header->version != APP_VERSION) {
    printf("Unsupported database version: %d\n", header->version);
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};

  if (fstat(fd, &dbstat) == -1) {
    perror("fstat");
    free(header);
    return STATUS_ERROR;
  }

  if (header->filesize != dbstat.st_size) {
    printf("Currupted database file: expected size %d, got %ld\n",
           header->filesize, dbstat.st_size);
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

  if (header == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }

  header->magic = HEADER_MAGIC;
  header->version = APP_VERSION;
  header->count = 0;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
