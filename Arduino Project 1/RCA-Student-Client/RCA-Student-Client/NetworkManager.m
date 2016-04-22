//
//  NetworkManager.m
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import "NetworkManager.h"
#import "ErrorManager.h"

#define IP_ADDRESS @"192.168.1.101"
#define PORT 8080

@interface NetworkManager() <NSStreamDelegate>

@property (strong, nonatomic) NSInputStream *inputStream;
@property (strong, nonatomic) NSOutputStream *outputStream;

@end

@implementation NetworkManager

#pragma mark - Init Methods -

+ (id)sharedManager {
    static NetworkManager *sharedMyManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyManager = [[self alloc] init];
    });
    return sharedMyManager;
}

- (id)init {
    if (self = [super init]) {
        [self.inputStream setDelegate:self];
        [self.outputStream setDelegate:self];
    }
    return self;
}

#pragma mark - Stream Delegate Methods -

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode {
    NSLog(@"Stream triggered.");
    
    switch(eventCode) {
        case NSStreamEventHasSpaceAvailable: {
            if (stream == self.outputStream) {
                NSLog(@"outputStream is ready.");
                
//                uint8_t *readBytes = (uint8_t *)[_data mutableBytes];
//                readBytes += byteIndex; // instance variable to move pointer
//                int data_len = [_data length];
//                unsigned int len = ((data_len - byteIndex >= 1024) ?
//                                    1024 : (data_len-byteIndex));
//                uint8_t buf[len];
//                (void)memcpy(buf, readBytes, len);
//                len = [stream write:(const uint8_t *)buf maxLength:len];
//                byteIndex += len;
//                break;
            }
            break;
        }
        case NSStreamEventHasBytesAvailable: {
            if(stream == self.inputStream) {
                NSLog(@"inputStream is ready.");
                
                uint8_t buf[1024];
                NSInteger len = 0;
                
                len = [self.inputStream read:buf maxLength:1024];
                
                if(len > 0) {
                    NSMutableData* data=[[NSMutableData alloc] initWithLength:0];
                    
                    [data appendBytes: (const void *)buf length:len];
                    
                    NSString *s = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
                    
                    NSLog(@"Reading in the following:");
                    NSLog(@"%@", s);
                }
            } 
            break;
        }
        case NSStreamEventErrorOccurred: {
            NSLog(@"Can not connect to the host!");
            break;
        }
        case NSStreamEventOpenCompleted: {
            NSLog(@"Stream opened.");
            break;
        }
        default: {
            NSLog(@"Stream is sending an Event: %lu", (unsigned long)eventCode);
            break;
        }
    }
}

- (void)connectWithCompletion:(CompletionBlock)completion {
    
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)@"localhost", PORT, &readStream, &writeStream);
    
    if (!CFWriteStreamOpen(writeStream)) {
        NSLog(@"Error opening writeStream.");
        completion([ErrorManager errorForErrorIdentifier:ERROR_OPENING_STREAM]);
    } else {
        self.inputStream = (__bridge_transfer NSInputStream *)readStream;
        self.outputStream = (__bridge_transfer NSOutputStream *)writeStream;
        
        [self.inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        [self.inputStream open];
        [self.outputStream open];
        
        NSLog(@"Status of outputStream: %lu", (unsigned long)[self.outputStream streamStatus]);
        completion(nil);
    }
}

- (void)disconnectWithCompletion:(CompletionBlock)completion {
    NSLog(@"Closing streams.");
    
    [self.inputStream close];
    [self.outputStream close];
    
    [self.inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [self.outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [self.inputStream setDelegate:nil];
    [self.outputStream setDelegate:nil];
    
    self.inputStream = nil;
    self.outputStream = nil;
    completion(nil);
}

- (void)sendData:(NSDictionary *)info withCompletion:(CompletionBlock)completion {
    NSData *data = [[NSData alloc] initWithData:[[self serializeObject:info] dataUsingEncoding:NSASCIIStringEncoding]];
    
    NSLog(@"Output Stream Error: %@",self.outputStream.streamError);
    if ([self.outputStream hasSpaceAvailable]) {
        NSInteger writeLength = [self.outputStream write:(const uint8_t*)[data bytes] maxLength:[data length]];
        if (writeLength != -1) {
            completion(nil);
        } else {
            completion([ErrorManager errorForErrorIdentifier:ERROR_WRITING_DATA]);
        }
    } else {
        NSLog(@"Output stream doesn't have space available.");
    }
    
}

#pragma mark - Helpers - 

- (NSString*)serializeObject:(NSDictionary *)object {
    return [NSString stringWithFormat:@"RA:%@,isDoor:0,isPresent:%@\n",[object objectForKey:@"RA"],[object objectForKey:@"isPresent"]];
}


@end
