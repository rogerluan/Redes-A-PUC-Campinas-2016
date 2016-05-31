//
//  NetworkManager.m
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import "NetworkManager.h"
#import "ErrorManager.h"

#define IP_ADDRESS @"192.168.0.104"
#define PORT 5000

@interface NetworkManager() <NSStreamDelegate>

@property (strong, nonatomic) NSInputStream *inputStream;
@property (strong, nonatomic) NSOutputStream *outputStream;
@property (assign, nonatomic) BOOL isConnected;

@end

@implementation NetworkManager

#pragma mark - Init Methods -

+ (id)sharedManager {
    static NetworkManager *sharedMyManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyManager = [self new];
    });
    return sharedMyManager;
}

#pragma mark - Stream Delegate Methods -

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode {
    
    switch (eventCode) {
        case NSStreamEventHasSpaceAvailable: {
            if (stream == self.outputStream) {
                NSLog(@"OutputStream is ready.");
                if ([self.delegate respondsToSelector:@selector(networkManagerDidConnect:)]) {
                    [self.delegate networkManagerDidConnect:self];
                }
            }
            break;
        }
        case NSStreamEventHasBytesAvailable: {
            if (stream == self.inputStream) {
                NSLog(@"InputStream is ready.");
                
                uint8_t buf[1024];
                NSInteger len = 0;
                
                len = [self.inputStream read:buf maxLength:1024];
                
                if (len > 0) {
                    NSMutableData *data = [[NSMutableData alloc] initWithLength:0];
                    
                    [data appendBytes:(const void *)buf length:len];
                    
                    if ([self.delegate respondsToSelector:@selector(networkManager:didReceiveData:)]) {
                        [self.delegate networkManager:self didReceiveData:data];
                    }
                }
            } 
            break;
        }
        case NSStreamEventErrorOccurred: {
            //to-do: return an appropriate error message here.
            if ([self.delegate respondsToSelector:@selector(networkManager:didReceiveError:)]) {
                [self.delegate networkManager:self didReceiveError:nil];
            }
            
            NSLog(@"NSStreamEventErrorOccurred: Can not connect to the host!");
            break;
        }
        case NSStreamEventOpenCompleted: {
            if ([stream isEqual:self.inputStream]) {
                NSLog(@"Input stream has successfully opened.");
            } else if ([stream isEqual:self.outputStream]){
                NSLog(@"Output stream has successfully opened.");
            } else {
                NSLog(@"NSStreamEventOpenCompleted: an unknown stream has been opened.");
            }
            break;
        }
        case NSStreamEventEndEncountered: {
            NSLog(@"NSStreamEventEndEncountered");
            break;
        }
        case NSStreamEventNone: {
            NSLog(@"NSStreamEventNone");
            break;
        }
        default: {
            NSLog(@"Stream is sending an Event: %lu", (unsigned long)eventCode);
            break;
        }
    }
}

- (void)connectWithCompletion:(CompletionBlock)completion {
    
    if (!self.isConnected) {
        CFReadStreamRef readStream;
        CFWriteStreamRef writeStream;
        
#if (TARGET_IPHONE_SIMULATOR)
        CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)@"localhost", PORT, &readStream, &writeStream);
#else //iPhone Device
        CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)IP_ADDRESS, PORT, &readStream, &writeStream);
#endif
        
        if (!CFWriteStreamOpen(writeStream)) {
            NSLog(@"Error opening writeStream.");
            completion([ErrorManager errorForErrorIdentifier:ERROR_OPENING_STREAM]);
        } else {
            self.inputStream = (__bridge_transfer NSInputStream *)readStream;
            self.outputStream = (__bridge_transfer NSOutputStream *)writeStream;
            
            [self.inputStream scheduleInRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
            [self.outputStream scheduleInRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
            
            [self.inputStream open];
            [self.outputStream open];
            
            self.inputStream.delegate = self;
            self.outputStream.delegate = self;
            
            self.isConnected = YES;
            completion(nil);
        }
        
    } else {
        completion([ErrorManager errorForErrorIdentifier:ERROR_ALREADY_CONNECTED]);
    }
}

- (void)disconnectWithCompletion:(CompletionBlock)completion {
    if (self.isConnected) {
        NSLog(@"Closing streams.");
        
        [self.inputStream close];
        [self.outputStream close];
        
        [self.inputStream removeFromRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
        [self.outputStream removeFromRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
        
        [self.inputStream setDelegate:nil];
        [self.outputStream setDelegate:nil];
        
        self.inputStream = nil;
        self.outputStream = nil;
        self.isConnected = NO;
        
        if ([self.delegate respondsToSelector:@selector(networkManagerDidDisconnect:)]) {
            [self.delegate networkManagerDidDisconnect:self];
        }
        
        completion(nil);
    } else {
        completion([ErrorManager errorForErrorIdentifier:ERROR_NO_CONNECTION_FOUND]);
    }
}

- (void)sendData:(NSDictionary *)info withCompletion:(CompletionBlock)completion {
    NSData *data = [[NSData alloc] initWithData:[[self serializeObject:info] dataUsingEncoding:NSASCIIStringEncoding]];
    
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
