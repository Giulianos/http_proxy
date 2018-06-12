#include <ctype.h>
#include <stdio.h>
#include "response_parser_private.h"
#include <response_parser/response_parser.h>

static char header_name_special_chars[] = {'!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

bool
res_check_temp_space(char ** space, size_t index)
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
res_is_version_char(char c)
{
  return isalpha (c) || isdigit (c) || c == '.' || c == '/' ;
}

bool
res_is_text_char(char c)
{
  return c!='\n' && c!='\r';
}

bool
res_is_header_name_char(char c)
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
res_is_header_value_char(char c)
{
  return c!='\n' && c!='\r';
}

bool
res_check_response_has_body(char * code)
{
  if(code[0] == 1 ||
      (code[0] == 2 && code[1] == 0 && code[2] == 4) ||
      (code[0] == 3 && code[1] == 0 && code[2] == 4))
    return false;
  return true;
}

int
res_strncmp_case_insensitive(const char * str1, const char * str2, size_t n)
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