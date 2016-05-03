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

@interface ActionViewController ()

@end

@implementation ActionViewController

#pragma mark - Lifecycle

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.navigationController setTitle:self.RA];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

#pragma mark - IBActions

- (IBAction)presentAction:(id)sender {
    
    NSDictionary *data = @{@"RA":self.RA,@"isPresent":@YES};
    
    [[NetworkManager sharedManager] sendData:data withCompletion:^(NSError *error) {
        if (!error) {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Confirmed!",nil) message:NSLocalizedString(@"You have succesfully answered to the roll call. We'll disconnect you now.",nil) preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", @"OK") style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                [[NetworkManager sharedManager] disconnectWithCompletion:^(NSError *error) {
                    if (!error) {
                        [self dismissViewControllerAnimated:YES completion:nil];
                    } else {
                        [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
                    }
                }];
            }];
            [alert addAction:cancelAction];
            [self presentViewController:alert animated:YES completion:nil];
        } else {
            [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
        }
    }];
}


@end
