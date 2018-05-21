#ifndef CONFIG_H
#define CONFIG_H

/**
 * recieves a name that represents a configuration and a value to set
 * returns 0 if it was set correctly
 * returns <0 if it failed
 */

int
set_config(const char * name, const char * value);

/**
 * recieves a name that represents a configuration and returns the value
 */

char *
get_config(const char * name);

#endif
