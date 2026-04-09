/*
 * bank.c - Basic Bank Account System in C
 *          WITH Login, Password Auth, Change Password & Transaction History
 *
 * Features:
 *   - Account creation (with password setup)
 *   - Login authentication (account number + password)
 *   - Change password
 *   - Deposit, Withdraw (only after login)
 *   - Check Balance, Account Summary
 *   - Mini Statement (last 5 transactions)
 *   - View full Transaction History
 *   - Delete Account (requires password)
 *
 * Data files:
 *   accounts.dat     — binary file storing all Account records
 *   transactions.txt — text file storing all transaction records
 *
 * Compile:  gcc bank.c -o bank
 * Run:      ./bank
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ===================== STRUCT DEFINITIONS ===================== */

/*
 * Account struct — stores all data for one bank account.
 * Password is stored as plaintext (beginner-level, for learning only).
 */
typedef struct Account {
    int    account_number;   /* Unique 6-digit auto-generated number */
    char   name[100];        /* Account holder's full name           */
    double balance;          /* Current balance                      */
    char   password[50];     /* Login password (min 6 characters)    */
} Account;

/*
 * Transaction struct — records each deposit or withdrawal.
 */
typedef struct Transaction {
    int    account_number;
    char   type[10];         /* "DEPOSIT" or "WITHDRAW" */
    double amount;
    char   timestamp[50];
} Transaction;

/* ===================== CONSTANTS ===================== */

#define MAX_ACCOUNTS      100
#define ACCOUNTS_FILE     "accounts.dat"
#define TRANS_FILE        "transactions.txt"
#define MIN_PASSWORD_LEN  6     /* Minimum password length */
#define MINI_STMT_COUNT   5     /* How many recent transactions to show */

/* ===================== FUNCTION PROTOTYPES ===================== */

/* Auth functions */
int  login(Account *loggedIn);
int  authenticate(int accNumber, const char *password,
                  Account accounts[], int count, int *foundIdx);
void changePassword(Account *loggedIn);

/* Account functions */
void createAccount();
void deleteAccount(Account *loggedIn);
void accountSummary(Account *loggedIn);

/* Banking functions (require login) */
void deposit(Account *loggedIn);
void withdraw(Account *loggedIn);
void checkBalance(Account *loggedIn);
void miniStatement(Account *loggedIn);
void viewTransactions(Account *loggedIn);

/* Transaction helpers */
void saveTransaction(int accNumber, const char *type, double amount);

/* File/utility helpers */
int  loadAccounts(Account accounts[], int *count);
int  saveAccounts(Account accounts[], int count);
int  findAccount(Account accounts[], int count, int accNumber);
int  generateAccountNumber(Account accounts[], int count);
void getCurrentTimestamp(char *buffer, int bufSize);
void printMainMenu();
void printBankingMenu(const char *name);
void clearInputBuffer();
void readPassword(char *buffer, int maxLen);

/* ===================== MAIN ===================== */

int main() {
    int choice;
    Account loggedIn;      /* Holds the currently logged-in account */
    int isLoggedIn = 0;    /* 0 = not logged in, 1 = logged in      */

    printf("\n");
    printf("================================================\n");
    printf("      WELCOME TO THE BASIC BANK SYSTEM         \n");
    printf("================================================\n");

    while (1) {
        if (!isLoggedIn) {
            /* ---- NOT logged in: show main menu ---- */
            printMainMenu();
            printf("Enter your choice: ");
            if (scanf("%d", &choice) != 1) {
                printf("Invalid input!\n");
                clearInputBuffer();
                continue;
            }
            clearInputBuffer();

            switch (choice) {
                case 1:
                    createAccount();
                    break;
                case 2:
                    isLoggedIn = login(&loggedIn);
                    break;
                case 0:
                    printf("\nGoodbye! Thank you for banking with us.\n");
                    return 0;
                default:
                    printf("\nInvalid option. Choose 0-2.\n");
            }

        } else {
            /* ---- LOGGED IN: show banking menu ---- */
            printBankingMenu(loggedIn.name);
            printf("Enter your choice: ");
            if (scanf("%d", &choice) != 1) {
                printf("Invalid input!\n");
                clearInputBuffer();
                continue;
            }
            clearInputBuffer();

            switch (choice) {
                case 1: accountSummary(&loggedIn);  break;
                case 2: deposit(&loggedIn);         break;
                case 3: withdraw(&loggedIn);        break;
                case 4: checkBalance(&loggedIn);    break;
                case 5: miniStatement(&loggedIn);   break;
                case 6: viewTransactions(&loggedIn);break;
                case 7:
                    changePassword(&loggedIn);
                    break;
                case 8:
                    deleteAccount(&loggedIn);
                    isLoggedIn = 0;   /* Force logout after deletion */
                    break;
                case 0:
                    printf("\nLogged out successfully.\n");
                    isLoggedIn = 0;
                    break;
                default:
                    printf("\nInvalid option. Choose 0-8.\n");
            }
        }
    }

    return 0;
}

/* ===================== PRINT MENUS ===================== */

/* Main menu (before login) */
void printMainMenu() {
    printf("\n------------------------------------------------\n");
    printf("                 MAIN MENU                     \n");
    printf("------------------------------------------------\n");
    printf("  1. Create New Account\n");
    printf("  2. Login to Your Account\n");
    printf("  0. Exit\n");
    printf("------------------------------------------------\n");
}

/* Banking menu (after login) */
void printBankingMenu(const char *name) {
    printf("\n------------------------------------------------\n");
    printf("  Logged in as: %s\n", name);
    printf("------------------------------------------------\n");
    printf("  1. Account Summary\n");
    printf("  2. Deposit Money\n");
    printf("  3. Withdraw Money\n");
    printf("  4. Check Balance\n");
    printf("  5. Mini Statement (last %d transactions)\n", MINI_STMT_COUNT);
    printf("  6. Full Transaction History\n");
    printf("  7. Change Password\n");
    printf("  8. Delete Account\n");
    printf("  0. Logout\n");
    printf("------------------------------------------------\n");
}

/* ===================== UTILITY HELPERS ===================== */

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void getCurrentTimestamp(char *buffer, int bufSize) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, bufSize, "%Y-%m-%d %H:%M:%S", t);
}

/*
 * readPassword() — reads a password from stdin.
 * In a real system you'd hide the characters; here we keep it simple.
 */
void readPassword(char *buffer, int maxLen) {
    fgets(buffer, maxLen, stdin);
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
}

/* ===================== FILE HELPERS ===================== */

int loadAccounts(Account accounts[], int *count) {
    FILE *file = fopen(ACCOUNTS_FILE, "rb");
    if (file == NULL) { *count = 0; return 0; }
    fread(count, sizeof(int), 1, file);
    fread(accounts, sizeof(Account), *count, file);
    fclose(file);
    return 1;
}

int saveAccounts(Account accounts[], int count) {
    FILE *file = fopen(ACCOUNTS_FILE, "wb");
    if (file == NULL) { printf("Error: Cannot save accounts!\n"); return 0; }
    fwrite(&count, sizeof(int), 1, file);
    fwrite(accounts, sizeof(Account), count, file);
    fclose(file);
    return 1;
}

int findAccount(Account accounts[], int count, int accNumber) {
    int i;
    for (i = 0; i < count; i++)
        if (accounts[i].account_number == accNumber) return i;
    return -1;
}

int generateAccountNumber(Account accounts[], int count) {
    int newNum, unique, i;
    do {
        newNum = 100000 + rand() % 900000;
        unique = 1;
        for (i = 0; i < count; i++)
            if (accounts[i].account_number == newNum) { unique = 0; break; }
    } while (!unique);
    return newNum;
}

/*
 * saveTransaction() — appends one line to transactions.txt.
 * Format:  ACCOUNT_NUMBER|TYPE|AMOUNT|TIMESTAMP
 */
void saveTransaction(int accNumber, const char *type, double amount) {
    FILE *file = fopen(TRANS_FILE, "a");
    if (file == NULL) { printf("Warning: Cannot save transaction.\n"); return; }
    char ts[50];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(file, "%d|%s|%.2f|%s\n", accNumber, type, amount, ts);
    fclose(file);
}

/* ===================== AUTHENTICATION ===================== */

/*
 * authenticate() — checks if the given account number + password match.
 *
 * Returns 1 (success) or 0 (failure).
 * Sets *foundIdx to the position in the array if found.
 */
int authenticate(int accNumber, const char *password,
                 Account accounts[], int count, int *foundIdx) {
    int idx = findAccount(accounts, count, accNumber);
    if (idx == -1) return 0;                        /* Account not found */
    if (strcmp(accounts[idx].password, password) != 0) return 0; /* Wrong pw */
    *foundIdx = idx;
    return 1;
}

/*
 * login() — prompts for account number and password,
 *            then verifies credentials.
 *
 * Returns 1 on success (and fills *loggedIn), 0 on failure.
 */
int login(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    int accNumber, idx;
    char password[50];

    printf("\n========== LOGIN ==========\n");

    loadAccounts(accounts, &count);
    if (count == 0) {
        printf("No accounts exist yet. Please create one first.\n");
        return 0;
    }

    printf("Account Number: ");
    if (scanf("%d", &accNumber) != 1) {
        printf("Invalid account number!\n");
        clearInputBuffer();
        return 0;
    }
    clearInputBuffer();

    printf("Password: ");
    readPassword(password, sizeof(password));

    /* Verify credentials */
    if (authenticate(accNumber, password, accounts, count, &idx)) {
        *loggedIn = accounts[idx];   /* Copy account data to session */
        printf("\n✓ Login successful! Welcome, %s.\n", loggedIn->name);
        return 1;
    } else {
        printf("\n✗ Login failed. Incorrect account number or password.\n");
        return 0;
    }
}

/* ===================== CHANGE PASSWORD ===================== */

/*
 * changePassword() — verifies the current password, then sets a new one.
 *
 * Rules:
 *   - New password must be at least MIN_PASSWORD_LEN characters.
 *   - New password must not be the same as the current one.
 */
void changePassword(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    char currentPw[50], newPw[50];
    int idx;

    printf("\n========== CHANGE PASSWORD ==========\n");

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);
    if (idx == -1) { printf("Error: Account not found.\n"); return; }

    printf("Enter current password: ");
    readPassword(currentPw, sizeof(currentPw));

    /* Verify the current password */
    if (strcmp(accounts[idx].password, currentPw) != 0) {
        printf("✗ Incorrect current password.\n");
        return;
    }

    printf("Enter new password (min %d characters): ", MIN_PASSWORD_LEN);
    readPassword(newPw, sizeof(newPw));

    /* Validate new password length */
    if ((int)strlen(newPw) < MIN_PASSWORD_LEN) {
        printf("✗ Password too short! Must be at least %d characters.\n",
               MIN_PASSWORD_LEN);
        return;
    }

    /* Make sure it's different from the old one */
    if (strcmp(currentPw, newPw) == 0) {
        printf("✗ New password must be different from the current one.\n");
        return;
    }

    /* Update and save */
    strncpy(accounts[idx].password, newPw, sizeof(accounts[idx].password) - 1);
    strncpy(loggedIn->password, newPw, sizeof(loggedIn->password) - 1);

    if (saveAccounts(accounts, count)) {
        printf("✓ Password changed successfully!\n");
    }
}

/* ===================== CREATE ACCOUNT ===================== */

/*
 * createAccount() — creates a new account with name, initial deposit,
 *                   and a password (min MIN_PASSWORD_LEN chars).
 */
void createAccount() {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    Account newAcc;

    printf("\n========== CREATE NEW ACCOUNT ==========\n");

    loadAccounts(accounts, &count);

    if (count >= MAX_ACCOUNTS) {
        printf("Error: Maximum accounts reached.\n");
        return;
    }

    /* Get name */
    printf("Enter your full name: ");
    fgets(newAcc.name, sizeof(newAcc.name), stdin);
    int len = strlen(newAcc.name);
    if (len > 0 && newAcc.name[len - 1] == '\n') newAcc.name[len - 1] = '\0';

    if (strlen(newAcc.name) == 0) {
        printf("Error: Name cannot be empty.\n");
        return;
    }

    /* Get initial deposit */
    printf("Initial deposit amount ($0 minimum): $");
    if (scanf("%lf", &newAcc.balance) != 1 || newAcc.balance < 0) {
        printf("Error: Invalid deposit amount.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    /* Set password */
    do {
        printf("Set a password (min %d characters): ", MIN_PASSWORD_LEN);
        readPassword(newAcc.password, sizeof(newAcc.password));

        if ((int)strlen(newAcc.password) < MIN_PASSWORD_LEN) {
            printf("Password too short! Try again.\n");
        }
    } while ((int)strlen(newAcc.password) < MIN_PASSWORD_LEN);

    /* Assign account number */
    newAcc.account_number = generateAccountNumber(accounts, count);
    accounts[count] = newAcc;
    count++;

    if (saveAccounts(accounts, count)) {
        /* Record opening deposit */
        if (newAcc.balance > 0)
            saveTransaction(newAcc.account_number, "DEPOSIT", newAcc.balance);

        printf("\n✓ Account created successfully!\n");
        printf("------------------------------------------\n");
        printf("  Account Number : %d\n", newAcc.account_number);
        printf("  Name           : %s\n",  newAcc.name);
        printf("  Opening Balance: $%.2f\n", newAcc.balance);
        printf("------------------------------------------\n");
        printf("⚠  Save your account number — you need it to log in.\n");
    }
}

/* ===================== ACCOUNT SUMMARY ===================== */

/*
 * accountSummary() — displays name, account number, and current balance
 *                    as a compact summary card.
 */
void accountSummary(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    int idx;

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);

    /* Refresh balance from file */
    if (idx != -1) loggedIn->balance = accounts[idx].balance;

    printf("\n========== ACCOUNT SUMMARY ==========\n");
    printf("  Account Holder : %s\n",  loggedIn->name);
    printf("  Account Number : %d\n",  loggedIn->account_number);
    printf("  Current Balance: $%.2f\n", loggedIn->balance);
    printf("======================================\n");
}

/* ===================== DEPOSIT ===================== */

void deposit(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    double amount;
    int idx;

    printf("\n========== DEPOSIT MONEY ==========\n");

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);
    if (idx == -1) { printf("Error: Account not found.\n"); return; }

    printf("Current Balance: $%.2f\n", accounts[idx].balance);
    printf("Enter deposit amount: $");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Error: Amount must be greater than $0.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    double prev = accounts[idx].balance;
    accounts[idx].balance += amount;
    loggedIn->balance = accounts[idx].balance;

    if (saveAccounts(accounts, count)) {
        saveTransaction(loggedIn->account_number, "DEPOSIT", amount);
        printf("\n✓ Deposit successful!\n");
        printf("  Deposited    : $%.2f\n", amount);
        printf("  Prev Balance : $%.2f\n", prev);
        printf("  New Balance  : $%.2f\n", accounts[idx].balance);
    }
}

/* ===================== WITHDRAW ===================== */

void withdraw(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    double amount;
    int idx;

    printf("\n========== WITHDRAW MONEY ==========\n");

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);
    if (idx == -1) { printf("Error: Account not found.\n"); return; }

    printf("Current Balance: $%.2f\n", accounts[idx].balance);
    printf("Enter withdrawal amount: $");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Error: Amount must be greater than $0.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    /* ⚠️ Insufficient balance alert */
    if (amount > accounts[idx].balance) {
        printf("\n⚠️  INSUFFICIENT BALANCE!\n");
        printf("  Requested : $%.2f\n", amount);
        printf("  Available : $%.2f\n", accounts[idx].balance);
        printf("  Shortfall : $%.2f\n", amount - accounts[idx].balance);
        return;
    }

    double prev = accounts[idx].balance;
    accounts[idx].balance -= amount;
    loggedIn->balance = accounts[idx].balance;

    if (saveAccounts(accounts, count)) {
        saveTransaction(loggedIn->account_number, "WITHDRAW", amount);
        printf("\n✓ Withdrawal successful!\n");
        printf("  Withdrawn    : $%.2f\n", amount);
        printf("  Prev Balance : $%.2f\n", prev);
        printf("  New Balance  : $%.2f\n", accounts[idx].balance);
    }
}

/* ===================== CHECK BALANCE ===================== */

void checkBalance(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    int idx;

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);
    if (idx != -1) loggedIn->balance = accounts[idx].balance;

    printf("\n========== BALANCE ==========\n");
    printf("  %s  (#%d)\n", loggedIn->name, loggedIn->account_number);
    printf("  Balance: $%.2f\n", loggedIn->balance);
    printf("=============================\n");
}

/* ===================== MINI STATEMENT ===================== */

/*
 * miniStatement() — shows the last MINI_STMT_COUNT transactions
 *                   plus the current balance.
 *
 * Reads the entire transactions file, keeps only the last N matching lines,
 * then prints them in order (oldest → newest).
 */
void miniStatement(Account *loggedIn) {
    char lines[MINI_STMT_COUNT][200];   /* Circular buffer of last N lines */
    int  lineCount = 0;                 /* How many we've filled */
    char buf[200];
    int  lineAccNum;
    char lineType[10];
    double lineAmount;
    char lineTimestamp[50];
    int  i;

    printf("\n========== MINI STATEMENT (Last %d) ==========\n",
           MINI_STMT_COUNT);

    FILE *file = fopen(TRANS_FILE, "r");
    if (file == NULL) {
        printf("No transactions found.\n");
        return;
    }

    /* Scan through the file, keeping a rolling window of the last N */
    while (fgets(buf, sizeof(buf), file) != NULL) {
        if (sscanf(buf, "%d|%9[^|]|%lf|%49[^\n]",
                   &lineAccNum, lineType, &lineAmount, lineTimestamp) == 4) {
            if (lineAccNum == loggedIn->account_number) {
                /* Shift left if we already have N entries */
                if (lineCount == MINI_STMT_COUNT) {
                    for (i = 0; i < MINI_STMT_COUNT - 1; i++)
                        strncpy(lines[i], lines[i + 1], sizeof(lines[i]));
                    lineCount--;
                }
                strncpy(lines[lineCount], buf, sizeof(lines[lineCount]) - 1);
                lineCount++;
            }
        }
    }
    fclose(file);

    /* Refresh current balance */
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    loadAccounts(accounts, &count);
    int idx = findAccount(accounts, count, loggedIn->account_number);
    if (idx != -1) loggedIn->balance = accounts[idx].balance;

    printf("Account: %s (#%d)\n", loggedIn->name, loggedIn->account_number);
    printf("Balance: $%.2f\n\n", loggedIn->balance);

    if (lineCount == 0) {
        printf("No transactions yet.\n");
    } else {
        printf("  %-10s  %-12s  %-22s\n", "Type", "Amount", "Date & Time");
        printf("  ----------------------------------------\n");
        for (i = 0; i < lineCount; i++) {
            if (sscanf(lines[i], "%d|%9[^|]|%lf|%49[^\n]",
                       &lineAccNum, lineType, &lineAmount, lineTimestamp) == 4) {
                if (strcmp(lineType, "DEPOSIT") == 0)
                    printf("  %-10s  +$%-10.2f  %s\n",
                           lineType, lineAmount, lineTimestamp);
                else
                    printf("  %-10s  -$%-10.2f  %s\n",
                           lineType, lineAmount, lineTimestamp);
            }
        }
    }
    printf("==============================================\n");
}

/* ===================== FULL TRANSACTION HISTORY ===================== */

void viewTransactions(Account *loggedIn) {
    char line[200];
    int  lineAccNum, found = 0;
    char lineType[10];
    double lineAmount;
    char lineTimestamp[50];

    printf("\n========== FULL TRANSACTION HISTORY ==========\n");
    printf("Account: %s (#%d)\n", loggedIn->name, loggedIn->account_number);
    printf("  %-10s  %-12s  %-22s\n", "Type", "Amount", "Date & Time");
    printf("  ----------------------------------------\n");

    FILE *file = fopen(TRANS_FILE, "r");
    if (file == NULL) { printf("No transactions found.\n"); return; }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%d|%9[^|]|%lf|%49[^\n]",
                   &lineAccNum, lineType, &lineAmount, lineTimestamp) == 4) {
            if (lineAccNum == loggedIn->account_number) {
                found = 1;
                if (strcmp(lineType, "DEPOSIT") == 0)
                    printf("  %-10s  +$%-10.2f  %s\n",
                           lineType, lineAmount, lineTimestamp);
                else
                    printf("  %-10s  -$%-10.2f  %s\n",
                           lineType, lineAmount, lineTimestamp);
            }
        }
    }

    fclose(file);
    if (!found) printf("  No transactions yet.\n");
    printf("===============================================\n");
}

/* ===================== DELETE ACCOUNT ===================== */

/*
 * deleteAccount() — removes the account after password confirmation.
 *
 * Asks for password, then shifts all remaining accounts left in the array
 * and saves the updated list (effectively deleting the record).
 */
void deleteAccount(Account *loggedIn) {
    Account accounts[MAX_ACCOUNTS];
    int count = 0;
    char password[50];
    int idx, i;

    printf("\n========== DELETE ACCOUNT ==========\n");
    printf("⚠️  This action is PERMANENT and cannot be undone.\n");
    printf("Confirm your password to continue: ");
    readPassword(password, sizeof(password));

    loadAccounts(accounts, &count);
    idx = findAccount(accounts, count, loggedIn->account_number);

    if (idx == -1) { printf("Error: Account not found.\n"); return; }

    if (strcmp(accounts[idx].password, password) != 0) {
        printf("✗ Incorrect password. Account not deleted.\n");
        return;
    }

    /* Shift accounts left to fill the gap */
    for (i = idx; i < count - 1; i++) {
        accounts[i] = accounts[i + 1];
    }
    count--;

    if (saveAccounts(accounts, count)) {
        printf("\n✓ Account #%d has been deleted.\n",
               loggedIn->account_number);
    }
}
