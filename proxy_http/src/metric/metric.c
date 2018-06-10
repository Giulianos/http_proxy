#include <metric/metric.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

static double connection_time_sum = 0;

struct connection_time {
  time_t init_time;
  time_t end_time;
};

static double metrics[METRICS_ENUM_SIZE];

int
metric_create(enum metrics number, double value)
{
  if(number >= METRICS_ENUM_SIZE)
    return -1;

  metrics[number] = value;

  return 0;
}

void
metric_get_value_string(int index, char * value)
{
  if(index >= METRICS_ENUM_SIZE)
    return;

  sprintf(value, "%f", metrics[index]);
}

char *
metric_get_name(unsigned char number)
{
  enum metrics metric;
  if(number >= METRICS_ENUM_SIZE)
    return NULL;
  metric = (enum metrics) number;
  switch(metric) {
    case INST_CONCURRENT_CONNECTIONS:
      return "inst_concurrent_conections";
    case MAX_CONCURRENT_CONNECTIONS:
      return "max_concurrent_connections";
    case ACCESSES:
      return "acceses";
    case TRANSFERED_BYTES:
      return "transfered_bytes";
    case AVG_CONNECTION_TIME:
      return "avg_connection_time";
    default:
      return NULL;
  }
}

int
metric_get_size()
{
  return METRICS_ENUM_SIZE;
}

connection_time_t
metric_new_connection()
{
  connection_time_t contime;

  contime = malloc(sizeof(struct connection_time));
  if(contime == NULL)
    return NULL;

  contime->init_time = time(NULL);

  metrics[INST_CONCURRENT_CONNECTIONS] += 1;
  if(metrics[INST_CONCURRENT_CONNECTIONS] > metrics[MAX_CONCURRENT_CONNECTIONS]) {
    metrics[MAX_CONCURRENT_CONNECTIONS] = metrics[INST_CONCURRENT_CONNECTIONS];
  }

  return contime;
}

void
metric_close_connection(connection_time_t contime)
{
  contime->end_time = time(NULL);
  metrics[INST_CONCURRENT_CONNECTIONS] -= 1;
  metrics[ACCESSES] += 1;
  connection_time_sum += difftime(contime->end_time, contime->init_time);
  metrics[AVG_CONNECTION_TIME] = connection_time_sum / metrics[ACCESSES];
}

void
add_transfered_bytes(double curr_transfered_bytes)
{
  metrics[TRANSFERED_BYTES] += curr_transfered_bytes;
}