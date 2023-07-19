#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_request.h"

int http_req_parse(char * content) {
    // printf("%s", content);

    http_data data;
    memset(&data, 0, sizeof(http_data));
    nextToken(content, " ", &(data.http_method));

    printf("http_method: %s\n", data.http_method);

    if (data.http_method)
    {
        free(data.http_method);
    }

    return 0;
}


int nextToken(char * inputStream, char * delim, char ** token) {
    int rc = 1;
    char * endpointer = strstr(inputStream, delim);
    size_t tokenSize = 0;
    if (endpointer != NULL)
    {
        tokenSize = endpointer - inputStream;
    }

    if (*token == NULL && tokenSize > 0)
    {
        *token = (char *) malloc(tokenSize + 1);
        memset(*token, 0, tokenSize + 1);
    }
    else
    {
        goto CLEANUP;
    }

    // printf("tokenSize: %lu\n", tokenSize);
    // printf("inputStream: %s\n", inputStream);
    // printf("endpointer: %s\n", endpointer);

    memcpy(*token, inputStream, tokenSize);
    rc = 0;

    CLEANUP:

    return rc;
}