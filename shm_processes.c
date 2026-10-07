#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void ParentProcess(int *ShmPTR);
void ChildProcess(int *ShmPTR);

int main(int argc, char *argv[])
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Allocate shared memory for 2 integers:
    // ShmPTR[0] -> BankAccount
    // ShmPTR[1] -> Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    // Attach shared memory
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (ShmPTR == (int *) -1) {  // or: if (ShmPTR == (void *) -1)
      printf("*** shmat error (server) ***\n");
      exit(1);
    }

    // Initialize BankAccount and Turn to 0
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn (0 = Parent turn, 1 = Child turn)

    printf("Server initialized BankAccount = %d, Turn = %d\n", ShmPTR[0], ShmPTR[1]);

    // Fork child process
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    }
    else if (pid == 0) {
        // Child Process (Poor Student)
        ChildProcess(ShmPTR);
        exit(0);
    }
    else {
        // Parent Process (Dear Old Dad)
        ParentProcess(ShmPTR);
    }

    // Wait for child to complete
    wait(&status);

    // Cleanup shared memory
    shmdt((void *) ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);

    printf("Server exits...\n");
    exit(0);
}

void ParentProcess(int *ShmPTR)
{
    int account;
    int balance;

    // Seed the pseudo-random number generator for Parent
    srand(time(NULL) ^ (getpid() << 16));

    for (int i = 0; i < 25; i++) {
        // 1. Sleep some random amount of time between 0 - 5 seconds
        sleep(rand() % 6);

        // 3. Busy-wait while Turn != 0 (Tanenbaum's strict alternation)
        while (ShmPTR[1] != 0) {
            // no-op
        }

        // 2. Copy the value in BankAccount to local variable account
        account = ShmPTR[0];

        // 4. Check if account is <= 100
        if (account <= 100) {
            // Deposit Money:
            // 1. Randomly generate balance between 0 and 100
            balance = rand() % 101;

            // 2. If balance is even: deposit amount
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } 
            // 3. If balance is odd: no deposit
            else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } 
        else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // 4. Copy value back to BankAccount
        ShmPTR[0] = account;

        // 5. Set Turn = 1
        ShmPTR[1] = 1;
    }
}

void ChildProcess(int *ShmPTR)
{
    int account;
    int balance;

    // Seed the pseudo-random number generator for Child
    srand(time(NULL) ^ (getpid() << 16));

    for (int i = 0; i < 25; i++) {
        // 1. Sleep some random amount of time between 0 - 5 seconds
        sleep(rand() % 6);

        // 3. Busy-wait while Turn != 1 (Tanenbaum's strict alternation)
        while (ShmPTR[1] != 1) {
            // no-op
        }

        // 2. Copy the value in BankAccount to local variable account
        account = ShmPTR[0];

        // 4. Randomly generate balance needed between 0 and 50
        balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);

        // 5. Check if balance needed <= account
        if (balance <= account) {
            // Withdraw amount
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } 
        // 6. Balance needed > account
        else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // 7. Copy value back to BankAccount
        ShmPTR[0] = account;

        // 8. Set Turn = 0
        ShmPTR[1] = 0;
    }
}