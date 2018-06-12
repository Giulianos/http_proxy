#ifndef CONFIG_H
#define CONFIG_H

/**
 * @param file_name
 * initializes the configurations reading from a file
 * @return
 */
int config_initialize_from_file(const char* file_name);

/**
 * recieves a name that represents a configuration and a value to set
 * if the configuration exists, changes the value
 * if the configuration does not exists, creates it
 * returns 0 if it was set correctly
 * returns <0 if it failed
 */

int config_create(const char* name, const char* value);

/**
 * @param index the position in configurations
 * @param value the value that needs to be set
 * @return 0 if it was set correctly, <0 if config was not found
 */
int config_set_from_index(int index, char* value);

/**
 * recieves a name that represents a configuration and returns the value
 */

char* config_get(const char* name);

/**
 * @param index the position in configurations
 * @return the value of the configuration
 */
char* config_get_from_index(int index);

/**
 * @param number number of the config in configurations array
 * @return the name of that configuration
 */
char* config_get_name(unsigned char number);

/**
 * @return the quantity of configs
 */
int config_get_size();
#endif
