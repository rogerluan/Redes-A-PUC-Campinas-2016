//
//  ErrorHandlerHelper.h
//  Patient
//
//  Created by Felipe Florencio Garcia on 10/24/15.
//  Copyright © 2015 GoDoctor. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#define ERROR_WRITING_DATA 1001
#define ERROR_OPENING_STREAM 1002
#define ERROR_NO_CONNECTION_FOUND 1003
#define ERROR_ALREADY_CONNECTED 1004
#define ERROR_INVALID_RESPONSE_FORMAT 1005

@interface ErrorManager : NSObject

+ (NSError*)errorForErrorIdentifier:(NSInteger)errorIdentifier;
+ (UIAlertController *)alertControllerFromError:(NSError *)error;
+ (UIAlertController *)alertControllerFromErrorIdentifier:(NSInteger)errorIdentifier;

@end
