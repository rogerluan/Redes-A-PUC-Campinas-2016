//
//  ActionViewController.m
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import "ActionViewController.h"
#import "NetworkManager.h"
#import "ErrorManager.h"

@interface ActionViewController () <NetworkManagerDelegate>

@property (strong, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicator;
@property (strong, nonatomic) IBOutlet UIButton *presentButton;

@end

@implementation ActionViewController

#pragma mark - Lifecycle

- (void)viewDidLoad {
    [super viewDidLoad];
    self.activityIndicator.hidden = YES;
    [self.navigationController setTitle:self.RA];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewWillDisappear:(BOOL)animated {
    [[NetworkManager sharedManager] disconnectWithCompletion:^(NSError *error) {
        if (error) {
            NSLog(@"An error occured: %@",error.description);
        }
    }];
    [[NetworkManager sharedManager] setDelegate:nil];
}

#pragma mark - Network Manager Delegate Methods - 

- (void)networkManagerDidDisconnect:(id)networkManager {
    NSLog(@"ActionViewController: Delegate method call: did disconnect.");
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)networkManager:(id)networkManager receivedError:(NSError *)error {
    NSLog(@"ActionViewController: Delegate method call: received error: %@", error);
    [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
}

- (void)networkManager:(id)networkManager didReceiveData:(NSData *)data {
    NSLog(@"ActionViewController: Delegate method call: didReceiveData: %@", data);
    if (data) {
        NSString *stringData = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
        NSLog(@"Reading in the following: %@",stringData);
        
        if ([self isDataValid:stringData]) {
            [[NetworkManager sharedManager] disconnectWithCompletion:^(NSError *error) {
                if (!error) {
                    __weak typeof(self) weakSelf = self;
                    UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Confirmed!",nil) message:NSLocalizedString(@"You have succesfully answered to the roll call. We'll disconnect you now.",nil) preferredStyle:UIAlertControllerStyleAlert];
                    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", @"OK") style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                        [weakSelf dismissViewControllerAnimated:YES completion:nil];
                    }];
                    [alert addAction:cancelAction];
                    [self presentViewController:alert animated:YES completion:nil];
                } else {
                    [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
                }
            }];
        } else {
            [self presentViewController:[ErrorManager alertControllerFromErrorIdentifier:ERROR_INVALID_RESPONSE_FORMAT] animated:YES completion:nil];
        }
    } else {
        NSLog(@"ActionViewController: Delegate method call: received invalid data. Error.");
    }
}

#pragma mark - IBActions -

- (IBAction)presentAction:(id)sender {
    NSDictionary *data = @{@"RA":self.RA,@"isPresent":@YES};
    
    [self animateActivityIndicator:YES];
    [[NetworkManager sharedManager] sendData:data withCompletion:^(NSError *error) {
        [self animateActivityIndicator:NO];
        if (error) {
            [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
        } else {
            /**
             *  TEST
             *
             */
//            NSString *testString = @"RA:12345678,recebeu:1";
//            NSData *testData = [[NSData alloc] initWithData:[testString dataUsingEncoding:NSASCIIStringEncoding]];
//            [self networkManager:[NetworkManager sharedManager] didReceiveData:testData];
        }
    }];
}

#pragma mark - Helpers - 

- (void)animateActivityIndicator:(BOOL)animate {
    self.activityIndicator.hidden = !animate;
    self.presentButton.enabled = !animate;
    animate ? [self.activityIndicator startAnimating] : [self.activityIndicator stopAnimating];
}

- (BOOL)isDataValid:(NSString *)data {
    /**
     *  Response format: `RA:12345678,recebeu:1` without quotes
     */
    data = [data stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    NSArray *componentsArray = [data componentsSeparatedByString:@":"];
    return [[componentsArray objectAtIndex:2] isEqualToString:@"1\0"];
}

@end
