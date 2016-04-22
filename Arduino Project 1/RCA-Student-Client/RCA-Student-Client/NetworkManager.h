//
//  NetworkManager.h
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void(^CompletionBlock)(NSError *error);

@interface NetworkManager : NSObject

@property (strong, nonatomic) NSString *port;

+ (id)sharedManager;

- (void)connectWithCompletion:(CompletionBlock)completion;
- (void)disconnectWithCompletion:(CompletionBlock)completion;
- (void)sendData:(NSDictionary *)info withCompletion:(CompletionBlock)completion;

@end
