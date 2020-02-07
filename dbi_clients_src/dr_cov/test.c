/*
   Manul - test file
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

char *name = NULL;
char *log_name = "afl_test_log.txt";

void LOG(const char *msg) {
    FILE *f = fopen(log_name, "a");
    fprintf(f, "%s", msg);
    fclose(f);
}

void open_file() {
    char *buf = NULL;
    int size = 0;
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        printf("Couldn't open file specified %s", name);
        return;
    }
    printf("Opening %s\n", name);
    // obtain file size:
    fseek(fp , 0 , SEEK_END);
    size = ftell(fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    buf = (char*) malloc (sizeof(char ) * size);
    if (buf == NULL) {LOG("Unable to read file"); exit (-1);}

    // copy the file into the buffer:
    fread(buf, 1, size, fp);
    fclose(fp);

    if (buf[0] == 'P') {
        if (buf[1] == 'W') {
            if (buf[2] == 'N') {
                if (buf[3] == 'I') {
                    //if (buf[4] == 'T') {
                        LOG("Found it!\n");
                        ((void(*)())0x0)();
                    //}
                }
            }
        }
    }

    LOG("Parsing successfully completed\n");
    free(buf);
}

int main(int argc, char** argv)
{
    LOG("Session started\n");
    if(argc < 2) {
        LOG("Input filed was not provided.\n");
        exit(-1);
    }
    name = argv[1];
    open_file();
}
