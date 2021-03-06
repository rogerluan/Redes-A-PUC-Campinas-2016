//
//  ErrorHandlerHelper.m
//  Patient
//
//  Created by Felipe Florencio Garcia on 10/24/15.
//  Copyright © 2015 GoDoctor. All rights reserved.
//

#import "ErrorManager.h"

@implementation ErrorManager

+ (NSError *)errorForErrorIdentifier:(NSInteger)errorIdentifier {
    return [NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier]
                               code:errorIdentifier
                           userInfo:[self userInfoForErrorIdentifier:errorIdentifier]];
}

+ (UIAlertController *)alertControllerFromError:(NSError *)error {
    NSLog(@"An error occured. Error description: %@. Possible failure reason: %@. Possible recovery suggestion: %@",error.localizedDescription,error.localizedFailureReason,error.localizedRecoverySuggestion);
    
    NSString *possibleRecoverySuggestion = @" ";
    if (error.localizedRecoverySuggestion) {
        possibleRecoverySuggestion = [NSString stringWithFormat:@"\nPossible recovery suggestion: %@",error.localizedRecoverySuggestion];
    }

    UIAlertController *alert = [UIAlertController alertControllerWithTitle:error.localizedDescription message:[NSString stringWithFormat:@"Possible reason: %@ %@",error.localizedFailureReason,possibleRecoverySuggestion] preferredStyle:UIAlertControllerStyleAlert];
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", @"OK") style:UIAlertActionStyleCancel handler:nil];
    [alert addAction:cancelAction];
    return alert;
}

+ (UIAlertController *)alertControllerFromErrorIdentifier:(NSInteger)errorIdentifier {
    return [self alertControllerFromError:[self errorForErrorIdentifier:errorIdentifier]];
}

#pragma mark - Helpers

+ (NSDictionary *)userInfoForErrorIdentifier:(NSInteger)error {
    
    switch (error) {
        case ERROR_WRITING_DATA:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"Error Writing Data", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"This occured when trying to write an output to the opened socket, when trying to send message to the server.", nil)
                     };
        case ERROR_OPENING_STREAM:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"Error Opening Stream", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"This occured when trying to open a stream.", nil)
                     };
        case ERROR_NO_CONNECTION_FOUND:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"There's no active connection.", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"This error occured when trying to close a connection, while there wasnt't a connection available.", nil)
                     };
        case ERROR_ALREADY_CONNECTED:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"There's a connection stablished already.", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"This error occured when trying to open a connection, while there was already a stablished connection.", nil)
                     };
        case ERROR_INVALID_RESPONSE_FORMAT:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"Invalid Response.", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"This error occurs when receiving a message with an unexpected format.", nil)
                     };
        default:
            return @{NSLocalizedDescriptionKey: NSLocalizedString(@"Invalid error", nil),
                     NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"The action generated an unexpected error.", nil)
                     };
    }
}


@end
