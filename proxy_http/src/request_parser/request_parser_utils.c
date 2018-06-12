#include "request_parser_private.h"
#include <request_parser/request_parser.h>
#include <string.h>
#include <stdio.h>

char url_special_chars[] = {'-', '.', '_', '~', ':', '/', '?', '#', '[', ']', '@', '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '='};
char header_name_special_chars[] = {'!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};


bool
is_url_char(char c)
{
  if(isalpha (c) || isdigit (c)) {
    return true;
  }

  size_t i;
  for(i=0; i< sizeof (url_special_chars)/ sizeof (url_special_chars[0]); i++) {
    if(url_special_chars[i] == c)
      return true;
  }

  return false;
}

bool
is_header_name_char(char c)
{
  if(isalpha (c) || isdigit (c)) {
      return true;
    }

  size_t i;
  for(i=0; i< sizeof (header_name_special_chars)/ sizeof (header_name_special_chars[0]); i++) {
      if(header_name_special_chars[i] == c)
        return true;
    }

  return false;
}

bool
is_header_value_char(char c)
{
  return c!='\n' && c!='\r';
}

bool
is_version_char(char c)
{
  return isalpha (c) || isdigit (c) || c == '.' || c == '/' ;
}

bool
check_temp_space(char ** space, size_t index)
{
  if(index + TEMP_PREALLOC > MAX_TEMP_SIZE) {
    return false;
  }

  if(index % TEMP_PREALLOC == 0) {
    *space = realloc(*space, index + TEMP_PREALLOC);

    if(*space == NULL) {
      return false;
    }
  }

  return true;
}

bool
check_request_has_body(const char * method)
{
  if(strncmp (method, "POST", MAX_METHOD_LEN + 1) == 0||
     strncmp (method, "PUT", MAX_METHOD_LEN + 1) == 0 ||
      strncmp (method, "DELETE", MAX_METHOD_LEN + 1) == 0 ||
       strncmp (method, "PATCH", MAX_METHOD_LEN + 1) == 0 ) {
    return true;
  }
  return false;
}

int
strncmp_case_insensitive(const char * str1, const char * str2, size_t n)
{
  size_t i=0;
  while(n>0 && (str1[i] != '\0' || str2[i]!='\0')) {
      if(tolower(str1[i]) != tolower(str2[i]))
        return tolower(str1[i]) - tolower(str2[i]);
      dprintf (STDERR_FILENO, "[STRNCMP_INSENSITIVE_INFO](n=%lu)%c matched against %c\n", n, str1[i], str2[i]);
      n--;
      i++;
    }
  dprintf (STDERR_FILENO, "[STRNCMP_INSENSITIVE_INFO](n=%lu)Ended, return value:%d\n", n, str1[i-1]-str2[i-1]);
  return tolower(str1[i]) - tolower(str2[i]);
}

void
split_host_port(host_details_t host_details)
{
  if(host_details == NULL)
    return;
  if(host_details->host == NULL)
    return;

  switch(host_details->type) {
    case FQDN:
    case IPV4:
    case IPV6:
      return;
    case FQDN_AND_PORT:
      host_details->type = FQDN;
      break;
    case IPV4_AND_PORT:
      host_details->type = IPV4;
      break;
    case IPV6_AND_PORT:
      host_details->type = IPV6;
      break;
  }

  size_t index = 0;

  while(host_details->host[index] != '\0' && host_details->host[index] != ':') {
    index++;
  }

  if(host_details->host[index] == '\0') {
    host_details->port = 80;
  } else {
    host_details->host[index] = '\0';
    index++;
    host_details->port = (unsigned int)atoi (&host_details->host[index]);
  }
}

bool
fill_host_details(host_details_t host)
{
  if(host == NULL)
    return false;

  if(check_ipv6 (host->host)) {
    host->type = IPV6_AND_PORT;
  } else if(check_ipv4 (host->host)) {
    host->type = IPV4_AND_PORT;
  } else {
    host->type = FQDN_AND_PORT;
  }

  split_host_port (host);

  return true;
}

bool
extract_host_from_target(const char * target, host_details_t host_details)
{
  if(target == NULL)
    return false;
  if(host_details == NULL)
    return false;

  enum target_state {
    SCHEME,
    HOST,
  } state;

  size_t target_len = strlen(target)+1;
  size_t current = 0;
  size_t temp_index = 0;
  state = SCHEME;

  while(current<target_len) {
    switch(state) {
      case SCHEME:
        dprintf (STDERR_FILENO, "[EXTRACT_HOST_STATE]Scheme\n");
        if(strncmp_case_insensitive(&target[current], "http://", 7) == 0) {
          state = HOST;
          current+=7;
        } else {
          dprintf (STDERR_FILENO, "[EXTRACT_HOST_INFO]%7s doesn't match \"http://\"\n", &target[current]);
          return false;
        }
        break;
      case HOST:
        dprintf (STDERR_FILENO, "[EXTRACT_HOST_STATE]Host\n");
        if(target[current] == '\0' || target[current] == '/') {
          if(!check_temp_space (&host_details->host, temp_index)) {
            dprintf (STDERR_FILENO, "[EXTRACT_HOST_INFO]Mem error\n");
            return false;
          }
          host_details->host[temp_index] = '\0';
          fill_host_details (host_details);
          return true;
        } else {
          if(!check_temp_space (&host_details->host, temp_index)) {
            dprintf (STDERR_FILENO, "[EXTRACT_HOST_INFO]Mem error\n");
            return false;
          }
          host_details->host[temp_index] = target[current];
          temp_index++;
        }
        current++;
        break;
      }
  }
  dprintf (STDERR_FILENO, "[EXTRACT_HOST_INFO]Host not found"
                          "\n");
  return false;
}

bool
check_ipv6(const char * host)
{
  if(host == NULL)
    return false;
  /** naive implementation */
  if(host[0] == '[')
    return true;
  return false;
}

bool
check_ipv4(const char * host)
{
  if(host == NULL)

    return false;
  size_t index = 0;
  size_t current = 0;
  int i;
  for(i=0; i<4; i++) {
    index = 0;
    while(index<3 && host[current]!='\0' && isdigit (host[current])) {
      index++;
      current++;
    }
    if(index <= 0) {
      dprintf (STDERR_FILENO, "[CHECK_IPV4_ERROR]Expected digit\n");
      return false;
    }
    if(i<3 && host[current]!='.') {
      dprintf (STDERR_FILENO, "[CHECK_IPV4_ERROR]Expected dot\n");
      return false;
    }
    current++;
  }

  if(host[current-1] == ':' || host[current-1] == '\0')
    return true;

  dprintf (STDERR_FILENO, "[CHECK_IPV4_ERROR]Expected colon or null, found: %c\n", host[current-1]);

  return false;
}
