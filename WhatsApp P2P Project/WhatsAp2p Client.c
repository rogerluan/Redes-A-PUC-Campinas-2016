//
//  WhatsAp2p Client.c
//  WhatsAp2p Client
//
//  Created by Roger Luan on 6/3/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum protocol {connectionRequest = 1, infoRequest, disconnectionRequest};


#pragma mark - Read & Write Functions


/**
 *  Reads a string from keyboard input, with a maximum string size,
 *  and copy the resulting string to a specific buffer position.
 *
 *  @param buffer         Buffer in which the string is being copied on.
 *  @param bufferPosition The position of the buffer that the string is going to be copied on.
 *  @param maxStringSize  Max length of the string.
 *
 *  @return returns the resulting buffer position after copying the string on the buffer.
 */
int readString(char buffer[TAM_BUF], int bufferPosition, int maxLength) {
    char string[maxLength+1];
    int i;
    fgets(string, maxLength, stdin);
    for (i=0 ; string[i] ; i++) {
        if (string[i] == '\n') break;
        buffer[bufferPosition++] = string[i];
    }
    buffer[bufferPosition++] = '\0';
    return bufferPosition;
}

#pragma mark - Main

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
