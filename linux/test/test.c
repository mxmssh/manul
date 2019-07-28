/*
   Manul - test file for Linux
   -------------------------------------
   Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>

   Copyright 2019 Salesforce.com, inc. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:
     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    char *buf = NULL;
    int size = 0;
    if(argc < 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        exit(-1);
    }
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Couldn't open file specified %s", argv[1]);
        exit(-1);
    }
    printf("Opening %s\n", argv[1]);
    // obtain file size:
    fseek(fp , 0 , SEEK_END);
    size = ftell(fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    buf = (char*) malloc (sizeof(char ) * size);
    if (buf == NULL) {printf("Unable to read file"); exit (-1);}

    // copy the file into the buffer:
    fread(buf, 1, size, fp);
    fclose(fp);

    if (buf[0] == 'P') {
        if (buf[1] == 'W') {
            if (buf[2] == 'N') {
                if (buf[3] == 'I') {
                    if (buf[4] == 'T') {
                        printf("Found it!\n");
                        ((void(*)())0x0)();
                    }
                }
            }
        }
    }

    printf("Parsed %d bytes\n", size);
    free(buf);
}
