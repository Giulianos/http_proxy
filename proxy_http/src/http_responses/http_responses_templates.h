#ifndef HTTP_RESPONSES_TEMPLATES_H
#define HTTP_RESPONSES_TEMPLATES_H

const char* http_response_version = "HTTP/1.1 ";
const char* http_response_connection_header = "\r\nConnection: close\r\n";
const char* http_response_content_length_header = "Content-Length: ";
const char* http_response_body_pretitle = "\r\n\r\n<html><head><title>";
const char* http_response_body_preh1 =
  "</title></head><body bgcolor=\"white\"><center><h1>";
const char* http_response_body_end =
  "</h1></center><hr><center>nginx/1.10.3</center></body></html>";

#endif
