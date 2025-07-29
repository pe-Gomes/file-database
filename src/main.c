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

struct command_line_args {
  bool newfile;
  bool list;
  bool delete;
  char *filepath;
  char *string_id;
  char *addstring;
  char *search_name;
};

void parse_args(int argc, char *argv[], struct command_line_args *args) {
  int c;
  while ((c = getopt(argc, argv, "nlf:a:d:s:")) != -1) {
    switch (c) {
    case 'n':
      args->newfile = true;
      break;
    case 'l':
      args->list = true;
      break;
    case 'd':
      args->delete = true;
      args->string_id = optarg;
      break;
    case 'f':
      args->filepath = optarg;
      break;
    case 'a':
      args->addstring = optarg;
      break;
    case 's':
      args->search_name = optarg;
      break;
    case '?':
      print_usage(argv);
      exit(0);
    }
  }
}

int main(int argc, char *argv[]) {
  struct command_line_args args = {0};
  parse_args(argc, argv, &args);

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  if (args.filepath == NULL) {
    printf("No file specified. Use -f <filename> to specify a file.\n");
    print_usage(argv);
  }

  if (args.newfile) {
    dbfd = create_db_file(args.filepath);

    if (dbfd == STATUS_ERROR) {
      printf("Error creating database file.\n");
      return -1;
    }

    if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Error creating database header.\n");
      return -1;
    }

  } else {
    dbfd = open_db_file(args.filepath);

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

  if (args.list) {
    list_employees(dbhdr, employees);
  }

  if (args.delete) {
    if (dbhdr->count == 0) {
      printf("No employees to delete.\n");
      return 0;
    }

    if (args.string_id == NULL) {
      printf("No ID specified for deletion. Use -d <id> to specify an ID.\n");
      return -1;
    }

    if (delete_employee(dbhdr, employees, args.string_id) == STATUS_ERROR) {
      printf("Error deleting employee with ID %s.\n", args.string_id);
      return -1;
    }
  }

  if (args.addstring) {
    dbhdr->count++;
    employees = realloc(employees, dbhdr->count * sizeof(struct employee_t));

    if (add_employee(dbhdr, employees, args.addstring) == STATUS_ERROR) {
      printf("Error adding employee to database.\n");
      return -1;
    }

    dbhdr->filesize =
        sizeof(struct dbheader_t) + (dbhdr->count * sizeof(struct employee_t));
  }

  if (args.search_name) {
    search_employees(dbhdr, employees, args.search_name);
  }

  if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR) {
    printf("Error writing database header to file.\n");
    free(employees);
    free(dbhdr);
    close(dbfd);
    return -1;
  }

  free(employees);
  free(dbhdr);
  close(dbfd);

  return 0;
}
