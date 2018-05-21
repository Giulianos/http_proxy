#include <config/config.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NAME 10
#define MAX_CONFIG 100

struct config {
    char name[MAX_NAME];
    char * value;
};

static struct config configurations[MAX_CONFIG];

static int config_size = 0;

int
set_config(const char * name, const char * value)
{
  int i = 0;

  for(i; i < config_size; i++) {
    if(strncmp(name, configurations[i].name, MAX_NAME) == 0) {
      configurations[i].value = realloc(configurations[i].value, strlen(value));

      strncpy(configurations[i].value, value, strlen(value));
      return 0;
    }
  }

  /** if i didn't return, i need to add the configuration */
  if(config_size + 1 > MAX_CONFIG)
    return -1;

  strncpy(configurations[config_size].name, name, MAX_NAME);
  configurations[i].value = malloc(strlen(value));
  strncpy(configurations[i].value, value, strlen(value));

  config_size++;
  return 0;
}


char *
get_config(const char * name)
{
  int i = 0;

  for(i; i < config_size; i++) {
    if(strncmp(name, configurations[i].name, MAX_NAME) == 0)
      return configurations[i].value;
  }

  return NULL;
}
