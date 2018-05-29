#ifndef METRIC_H
#define METRIC_H

/**
 * recieves a name that represents a metric and a value to set
 * if the metric exists, changes the value
 * if the metric does not exists, creates it
 * returns 0 if it was set correctly
 * returns <0 if it failed
 */

int
metric_create(const char * name, const char * value);

/**
 * @param index the position in metrics
 * @param value the value that needs to be set
 * @return 0 if it was set correctly, <0 if metric was not found
 */
int
metric_set_from_index(int index, char * value);

/**
 * recieves a name that represents a metric and returns the value
 */

char *
metric_get(const char * name);

/**
 * @param index the position in metrics
 * @return the value of the metric
 */
char *
metric_get_from_index(int index);

/**
 * @param number number of the metric in metrics array
 * @return the name of that metric
 */
char *
metric_get_name(unsigned char number);

/**
 * @return the quantity of metrics
 */
int
metric_get_size();

#endif
