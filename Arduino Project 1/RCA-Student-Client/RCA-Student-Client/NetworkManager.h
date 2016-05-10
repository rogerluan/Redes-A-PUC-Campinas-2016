//
//  NetworkManager.h
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void(^CompletionBlock)(NSError *error);

@protocol NetworkManagerDelegate <NSObject>

@optional
- (void)networkManagerDidConnect:(id)networkManager;
- (void)networkManagerDidDisconnect:(id)networkManager;
- (void)networkManager:(id)networkManager didReceiveError:(NSError *)error;
- (void)networkManager:(id)networkManager didReceiveData:(NSData *)data;

@end

@interface NetworkManager : NSObject

@property (assign, nonatomic) id<NetworkManagerDelegate>delegate;
@property (strong, nonatomic) NSString *port;

+ (id)sharedManager;

- (void)connectWithCompletion:(CompletionBlock)completion;
- (void)disconnectWithCompletion:(CompletionBlock)completion;
- (void)sendData:(NSDictionary *)info withCompletion:(CompletionBlock)completion;

@end
