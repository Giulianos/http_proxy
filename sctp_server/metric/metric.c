#include "metric.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_NAME 10
#define MAX_METRIC 100

struct metric {
    char name[MAX_NAME+1];
    char * value;
};

static struct metric metrics[MAX_METRIC];

static int metric_size = 0;

int
metric_create(const char * name, const char * value)
{
  int i = 0;

  for(i; i < metric_size; i++) {
    if(strncmp(name, metrics[i].name, MAX_NAME) == 0) {
      metrics[i].value = realloc(metrics[i].value, strlen(value));

      strncpy(metrics[i].value, value, strlen(value)+1);
      return 0;
    }
  }

  /** if i didn't return, i need to add the metric */
  if(metric_size + 1 > MAX_METRIC)
    return -1;

  strncpy(metrics[metric_size].name, name, MAX_NAME);
  metrics[i].value = malloc(strlen(value));
  strncpy(metrics[i].value, value, strlen(value)+1);

  metric_size++;
  return 0;
}

char *
metric_get(const char *name)
{
  int i = 0;

  for(i; i < metric_size; i++) {
    if(strncmp(name, metrics[i].name, MAX_NAME) == 0)
      return metrics[i].value;
  }

  return NULL;
}

char *
metric_get_from_index(int index)
{
  if(index >= metric_size)
    return NULL;
  return metrics[index].value;
}

char *
metric_get_name(unsigned char number)
{
  if(number >= metric_size)
    return NULL;
  return metrics[number].name;
}

int
metric_get_size()
{
  return metric_size;
}