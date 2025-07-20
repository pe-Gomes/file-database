#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                 char *addstring) {

  return 0;
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  return 0;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  return 0;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) { return 0; }

int create_db_header(int fd, struct dbheader_t **headerOut) { return 0; }
