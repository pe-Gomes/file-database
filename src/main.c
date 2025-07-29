#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-n <name,address,hours>] [-l] [-f <filename>] [-s <name>] "
         "[-d <id>]\n",
         argv[0]);
  printf("\t -n - create a new database\n");
  printf("\t -f - (required) path to database file\n");
  printf("\t -d - (required) employee id\n");
  printf("\t -l - list employees\n");
  printf("\t -s - search employees by name \n");

  return;
}

int main(int argc, char *argv[]) {
  int c = 0;

  bool newfile = false;
  bool list = false;
  bool delete = false;

  int dbfd = -1;
  char *filepath = NULL;
  char *string_id = NULL;
  struct dbheader_t *dbhdr = NULL;

  struct employee_t *employees = NULL;
  char *addstring = NULL;

  while ((c = getopt(argc, argv, "nlf:a:d:")) != -1) {
    switch (c) {
    case 'n':
      printf("Creating a new database file.\n");
      newfile = true;
      break;

    case 'f':
      filepath = optarg;
      break;

    case 'a':
      addstring = optarg;
      break;

    case 'l':
      list = true;
      break;

    case 'd':
      delete = true;
      string_id = optarg;
      break;

    case '?':
      printf("Unknown option: %c\n", optopt);
      print_usage(argv);
      break;

    default:
      printf("Unknown error occurred.\n");
      return -1;
    }
  }

  if (filepath == NULL) {
    printf("No file specified. Use -f <filename> to specify a file.\n");
    print_usage(argv);
  }

  if (newfile) {
    dbfd = create_db_file(filepath);

    if (dbfd == STATUS_ERROR) {
      printf("Error creating database file.\n");
      return -1;
    }

    if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Error creating database header.\n");
      return -1;
    }

  } else {
    dbfd = open_db_file(filepath);

    if (dbfd == STATUS_ERROR) {
      printf("Error opening database file.\n");
      return -1;
    }

    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Error validating database header.\n");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
    printf("Error reading employees from database.\n");
    return -1;
  }

  if (list) {
    list_employees(dbhdr, employees);
  }

  if (delete) {
    if (dbhdr->count == 0) {
      printf("No employees to delete.\n");
      return 0;
    }

    if (string_id == NULL) {
      printf("No ID specified for deletion. Use -d <id> to specify an ID.\n");
      return -1;
    }

    if (delete_employee(dbhdr, employees, string_id) == STATUS_ERROR) {
      printf("Error deleting employee with ID %s.\n", string_id);
      return -1;
    }
  }

  if (addstring) {
    dbhdr->count++;
    employees = realloc(employees, dbhdr->count * sizeof(struct employee_t));

    if (add_employee(dbhdr, employees, addstring) == STATUS_ERROR) {
      printf("Error adding employee to database.\n");
      return -1;
    }

    dbhdr->filesize =
        sizeof(struct dbheader_t) + (dbhdr->count * sizeof(struct employee_t));
  }

  if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR) {
    printf("Error writing database header to file.\n");
    return -1;
  }

  close(dbfd);

  return 0;
}
