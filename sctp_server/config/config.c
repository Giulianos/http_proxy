#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_NAME 10
#define MAX_CONFIG 100

#define PREALLOC_QUANTITY 10

struct metric {
    char name[MAX_NAME+1];
    char * value;
};

static struct metric configurations[MAX_CONFIG];

static int config_size = 0;

int
config_create(const char * name, const char * value)
{
  int i = 0;

  for(i; i < config_size; i++) {
    if(strncmp(name, configurations[i].name, MAX_NAME) == 0) {
      configurations[i].value = realloc(configurations[i].value, strlen(value));

      strncpy(configurations[i].value, value, strlen(value)+1);
      return 0;
    }
  }

  /** if i didn't return, i need to add the configuration */
  if(config_size + 1 > MAX_CONFIG)
    return -1;

  strncpy(configurations[config_size].name, name, MAX_NAME);
  configurations[i].value = malloc(strlen(value));
  strncpy(configurations[i].value, value, strlen(value)+1);

  config_size++;
  return 0;
}

int
config_set_from_index(int index, char * value)
{
  if(index >= config_size)
    return -1;
  configurations[index].value = value;
  return 0;
}

char *
config_get(const char *name)
{
  int i = 0;

  for(i; i < config_size; i++) {
    if(strncmp(name, configurations[i].name, MAX_NAME) == 0)
      return configurations[i].value;
  }

  return NULL;
}

char *
config_get_from_index(int index)
{
  if(index >= config_size)
    return NULL;
  return configurations[index].value;
}

char *
config_get_name(unsigned char number)
{
  return configurations[number].name;
}

int
config_get_size()
{
  return config_size;
}

int
config_initialization(const char * file_name)
{
  FILE * config_file = fopen(file_name, "r");
  int c;
  size_t current = 0;
  char name_buf [MAX_NAME+1];
  char * value_buf = NULL;

  enum {START,NAME,EQUAL,VALUE} state = START;

  while((c = fgetc(config_file)) != EOF){
    switch(state) {
      case START:
        if(isalpha(c) || isdigit(c) || c == '-' || c == '_') {
          name_buf[current] = (char)c;
          current++;
          state = NAME;
        } else {
          return -1;
        }
        break;
      case NAME:
        if(isalpha(c) || isdigit(c) || c == '-' || c == '_') {
          name_buf[current] = (char)c;
          current++;
        } else if(c == '=') {
          name_buf[current] = '\0';
          state = EQUAL;
        } else {
          return -1;
        }
        break;
      case EQUAL:
        if(isalpha(c) || isdigit(c) || c == '-' || c == '_') {
          current = 0;
          if(current % PREALLOC_QUANTITY == 0) {
            value_buf = realloc(value_buf, current + PREALLOC_QUANTITY);
          }
          value_buf[current] = (char) c;
          current++;
          state = VALUE;
        } else {
          return -1;
        }
        break;
      case VALUE:
        if(isalpha(c) || isdigit(c) || c == '-' || c == '_') {
          if(current % PREALLOC_QUANTITY == 0) {
            value_buf = realloc(value_buf, current + PREALLOC_QUANTITY);
          }
          value_buf[current] = (char) c;
          current++;
        } else if(c == '\n') {
          if(current % PREALLOC_QUANTITY == 0) {
            value_buf = realloc(value_buf, current + PREALLOC_QUANTITY);
          }
          value_buf[current] = '\0';
          config_create(name_buf,value_buf);
          current = 0;
          free(value_buf);
          value_buf = NULL;
          state = START;
        } else {
          return -1;
        }
        break;
    }
  }
  if(value_buf != NULL) {
    if(current % PREALLOC_QUANTITY == 0) {
      value_buf = realloc(value_buf, current + PREALLOC_QUANTITY);
    }
    value_buf[current] = '\0';
    config_create(name_buf,value_buf);
    free(value_buf);
  }
  return 0;
}
