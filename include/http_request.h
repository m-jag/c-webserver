typedef struct {
    char * http_method;
} http_data;

int http_req_parse(char * content);
int nextToken(char * inputStream, char * delim, char ** token);