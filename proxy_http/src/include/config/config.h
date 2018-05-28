#ifndef CONFIG_H
#define CONFIG_H

/**
 * initializes the configurations reading from a file
 */

int
config_initialization(const char * file_name);

/**
 * recieves a name that represents a configuration and a value to set
 * returns 0 if it was set correctly
 * returns <0 if it failed
 */

int
config_set(const char * name, const char * value);

/**
 * recieves a name that represents a configuration and returns the value
 */

char *
config_get(const char * name);

#endif
