#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-n] [-f <filename>]\n", argv[0]);
  printf("\t -n - create a new database\n");
  printf("\t -f - (required) path to database file\n");

  return;
}

int main(int argc, char *argv[]) {
  int c = 0;

  bool newfile = false;
  char *filepath = NULL;
  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;

  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
    case 'n':
      printf("Creating a new database file.\n");
      newfile = true;
      break;

    case 'f':
      filepath = optarg;
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

  if (output_file(dbfd, dbhdr, NULL) == STATUS_ERROR) {
    printf("Error writing database header to file.\n");
    return -1;
  }

  return 0;
}
