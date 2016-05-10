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

@interface LoginViewController () <NetworkManagerDelegate>

@property (strong, nonatomic) IBOutlet UITextField *RATextField;
@property (strong, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicator;
@property (strong, nonatomic) IBOutlet UIButton *connectButton;

@end

@implementation LoginViewController

#pragma mark - Lifecycle -

- (void)viewDidLoad {
    [super viewDidLoad];
    [self appearanceSetup];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [[NetworkManager sharedManager] setDelegate:self];
}

#pragma mark - Helpers -

- (BOOL)shouldConnect {
    self.RATextField.text = [[self.RATextField.text componentsSeparatedByCharactersInSet: [[NSCharacterSet decimalDigitCharacterSet]invertedSet]] componentsJoinedByString:@""];
    if (![self.RATextField hasText]) {
        return NO;
    } else if (self.RATextField.text.length < 8) {
        return NO;
    }
    return YES;
}

- (void)appearanceSetup {
    self.activityIndicator.hidden = YES;
    [self.navigationController.navigationBar setBarTintColor:[UIColor colorWithRed:0.154 green:0.413 blue:0.691 alpha:1.000]];
    [self.navigationController.navigationBar setTintColor:[UIColor colorWithRed:0.908 green:0.926 blue:0.932 alpha:1.000]];
    [self.navigationController.navigationBar setTranslucent:NO];
    [self.navigationController.navigationBar setTitleTextAttributes:@{NSForegroundColorAttributeName:[UIColor colorWithRed:0.908 green:0.926 blue:0.932 alpha:1.000]}];
    self.navigationController.navigationBar.barStyle = UIBarStyleBlack;
}

#pragma mark - NetworkManager Delegate Methods -

- (void)networkManagerDidConnect:(id)networkManager {
    NSLog(@"LoginViewController: Delegate method call: did connect.");
    [self animateActivityIndicator:NO];
    [self performSegueWithIdentifier:@"ConnectionSegue" sender:nil];
}

- (void)networkManagerDidDisconnect:(id)networkManager {
    NSLog(@"LoginViewController: Delegate method call: did disconnect.");
}

- (void)networkManager:(id)networkManager receivedError:(NSError *)error {
    NSLog(@"LoginViewController: Delegate method call: received error: %@",error);
    [self presentViewController:[ErrorManager alertControllerFromError:error] animated:YES completion:nil];
}

#pragma mark - IBActions -

- (IBAction)connectAction:(id)sender {
    if ([self shouldConnect]) {
        [self animateActivityIndicator:YES];
        [[NetworkManager sharedManager] connectWithCompletion:^(NSError *error) {
            if (error) {
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

- (void)animateActivityIndicator:(BOOL)animate {
    self.activityIndicator.hidden = !animate;
    self.connectButton.enabled = !animate;
    animate ? [self.activityIndicator startAnimating] : [self.activityIndicator stopAnimating];
}

#pragma mark - Navigation -

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"ConnectionSegue"]) {
        ActionViewController *viewController = segue.destinationViewController;
        viewController.RA = self.RATextField.text;
        [[NetworkManager sharedManager] setDelegate:viewController];
    }
}

@end
