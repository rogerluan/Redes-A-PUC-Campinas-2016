//
//  LoginViewController.m
//  RCA-Student-Client
//
//  Created by Roger Luan on 4/21/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#import "LoginViewController.h"
#import "ActionViewController.h"
#import "NetworkManager.h"
#import "ErrorManager.h"

@interface LoginViewController ()

@property (strong, nonatomic) IBOutlet UITextField *RATextField;

@end

@implementation LoginViewController

#pragma mark - Lifecycle -

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

#pragma mark - Helpers -

- (BOOL)shouldConnect {
    NSLog(@"RA before normalizing: %@",self.RATextField.text);
    self.RATextField.text = [[self.RATextField.text componentsSeparatedByCharactersInSet:
                            [[NSCharacterSet decimalDigitCharacterSet] invertedSet]]
                           componentsJoinedByString:@""];
//    self.RATextField.text = [self.RATextField.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    NSLog(@"RA after normalizing: %@",self.RATextField.text);
    if (![self.RATextField hasText]) {
        return NO;
    } else if (self.RATextField.text.length < 8) {
        return NO;
    }
    return YES;
}

#pragma mark - IBActions -

- (IBAction)connectAction:(id)sender {
    if ([self shouldConnect]) {
        [[NetworkManager sharedManager] connectWithCompletion:^(NSError *error) {
            if (!error) {
                [self performSegueWithIdentifier:@"ConnectionSegue" sender:nil];
            } else {
                [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
            }
        }];
    } else {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Invalid RA",nil) message:NSLocalizedString(@"Please provide a valid RA so we can connect you to the server.",nil) preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", @"OK") style:UIAlertActionStyleCancel handler:nil];
        [alert addAction:cancelAction];
        [self presentViewController:alert animated:YES completion:nil];
    }
}

#pragma mark - Navigation -

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"ConnectionSegue"]) {
        ActionViewController *viewController = segue.destinationViewController;
        viewController.RA = self.RATextField.text;
    }
}

@end
