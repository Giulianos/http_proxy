#ifndef METRIC_H
#define METRIC_H

#define MAX_NAME 30
#define MAX_VALUE 30

enum metrics
{
  INST_CONCURRENT_CONNECTIONS = 0,
  MAX_CONCURRENT_CONNECTIONS,
  ACCESSES,
  TRANSFERED_BYTES,
  AVG_CONNECTION_TIME,
  METRICS_ENUM_SIZE
};

typedef struct connection_time* connection_time_t;

/**
 * creates it
 * returns 0 if it was set correctly
 * returns <0 if it failed
 */
int metric_create(enum metrics number, double value);

/**
 * @param index the position in metrics
 * @param value the char * where the string is going to be stored
 */
void metric_get_value_string(int index, char* value);

/**
 * @param number number of the metric in metrics array
 * @return the name of that metric or NULL if it does not exist
 */
char* metric_get_name(unsigned char number);

/**
 * @return the quantity of metrics
 */
int metric_get_size();

/**
 * Increments INST_CONCURRENT_CONNECTIONS and checks MAX_CURRENT_CONNECTIONS
 * @return connection_metrics_t with the init time initialized, NULL in case of
 * error
 */
connection_time_t metric_new_connection();

/**
 * Decrements INST_CONCURRENT_CONNECTIONS
 * Increments ACCESSES because of a client has finished its connection
 * Uses the connection_time struct to update the AVG_CONNECTION_TIME
 */
void metric_close_connection(connection_time_t contime);

/**
 * @param curr_transfered_bytes sums and updates TRANSFERED_BYTES
 */
void metric_add_transfered_bytes(double curr_transfered_bytes);

#endif
