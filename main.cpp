#include <iostream>
#include <mysql.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <limits>
#include <algorithm>

using namespace std;
using namespace chrono;

string getCurrentDateTime() {
    time_t now = time(0);
    tm current_time;

#ifdef _MSC_VER
    localtime_s(&current_time, &now);
#else
    localtime_r(&now, &current_time);
#endif

    stringstream ss;
    ss << put_time(&current_time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

const char* HOST = "localhost";
const char* USER = "root";
const char* PW = " ";
const char* DB = "bank";

float* current_amount;
class Customers {
private:
    string Name;
    string Surname;
    string PhNumber;
    string Account_id;
    string Password;

public:
    Customers(string N, string Sur, string Ph, string Ac_id, string Pw)
        : Name(N), Surname(Sur), PhNumber(Ph), Account_id(Ac_id), Password(Pw) {}

    void setName(string N) { Name = N; }
    string getName() { return Name; }
    void setSurname(string Sur) { Surname = Sur; }
    string getSurname() { return Surname; }
    void setPhNumber(string Ph) { PhNumber = Ph; }
    string getPhNumber() { return PhNumber; }
    void setAccount_id(string Ac_id) { Account_id = Ac_id; }
    string getAccount_id() { return Account_id; }
    void setPassword(string Pw) { Password = Pw; }
    string getPassword() { return Password; }
};

class Accounts : public Customers {
private:
    string Account_type;
    string Currency;

public:
    Accounts(string N, string Sur, string Ph, string Ac_id, string Pw, string Ac_type, string Cur)
        : Customers(N, Sur, Ph, Ac_id, Pw), Account_type(Ac_type), Currency(Cur) {}

    void setAccount_type(string Ac_type) { Account_type = Ac_type; }
    string getAccount_type() { return Account_type; }
    void setCurrency(string Cur) { Currency = Cur; }
    string getCurrency() { return Currency; }
};

class Transactions :public Customers {
private:
    string Trantype;
    float Amount;
    string Date;
    string Currency;
    int Eftcode;

public:
    Transactions(string N, string Sur, string Ph, string Ac_id, string Pw, string T_type, float amount, string date, string Cur, int eft_code)
        : Customers(N, Sur, Ph, Ac_id, Pw), Trantype(T_type), Amount(amount), Date(date), Currency(Cur), Eftcode(eft_code) {}

    void setTrantype(string T_type) { Trantype = T_type; }
    string getTrantype() { return Trantype; }
    void setAmount(float amount) { Amount = amount; }
    float getAmount() { return Amount; }
    void setDate(string date) { Date = date; }
    string getDate() { return Date; }
    void setCurrency(string Cur) { Currency = Cur; }
    string getCurrency() { return Currency; }
    void setEFTcode(int eft_code) { Eftcode = eft_code; }
    int getEFTcode() { return Eftcode; }
};

void addCustomers(MYSQL* conn, Accounts Reg);
void Accounts_balance(MYSQL* conn, Accounts Reg);
void Transaction_insert(MYSQL* conn, Transactions Transac);
int password_control(MYSQL* conn, const string& id, const string& pw);
void balance_update(MYSQL* conn, Transactions Transac, float last_balance);

void phone_number_control(string& phone);
void Account_id_control(string& id);
void name_control(string& name);
void Surname_control(string& surname);
void Password_control(string& pw);

float balance(float amount, int value, MYSQL* conn, Transactions Transac);
float checkBalance(MYSQL* conn, const string& account_id, float amount);
int show_customer_menu();
int show_customer_menu2();
int show_customer_menu3();
int show_account_menu();

int main() {
    MYSQL* conn;
    conn = mysql_init(NULL);
    string name, surname, phone, pw, id, type, currency, trantype, inject_balance;
    if (!mysql_real_connect(conn, HOST, USER, PW, DB, 3306, NULL, 0)) {
        cout << "Error: " << mysql_error(conn) << endl;
        return 1;
    }
    else {
        cout << "Logged to DATABASE" << endl;
    }

    int val = show_customer_menu();

    if (val == 1) {

        name_control(name);
        Surname_control(surname);
        phone_number_control(phone);
        Account_id_control(id);
        Password_control(pw);
        val = show_customer_menu2();

        switch (val) {
        case 1: type = "Credits Account"; break;
        case 2: type = "Bank Account"; break;
        default: cout << "Enter 1 or 2." << endl;
        }

        val = show_customer_menu3();
        switch (val) {
        case 1: currency = "DOLLAR"; break;
        case 2: currency = "EURO"; break;
        case 3: currency = "TL"; break;
        default: cout << "Enter 1, 2 or 3." << endl;
        }

        Customers Custom(name, surname, phone, id, pw);
        Accounts Reg(name, surname, phone, id, pw, type, currency);
        addCustomers(conn, Reg);
        Accounts_balance(conn, Reg);
    }


    else if (val == 2) {
        float amount = 0, last_balance = 0, current_amount = 0;
        int val2;

        bool pass_cnt = false;
        do {
            Account_id_control(id);
            Password_control(pw);

            if (password_control(conn, id, pw) == 1) {
                cout << "Access granted. Proceeding with further operations." << endl << endl;
                pass_cnt = true;
            }
            else {
                cout << "Invalid account ID or password." << endl << endl;
            }
        } while (!pass_cnt);


        val2 = show_account_menu();

        switch (val2) {
        case 1: trantype = "Deposit"; break;
        case 2: trantype = "Withdrawal"; break;
        case 3: trantype = "Paying off debt"; break;
        case 4: trantype = "Money Transfer"; break;
        }

        if (val2 == 1) {
            cout << "Enter Amount: ";
            cin >> amount;
            current_amount = amount;


            string query1 = "SELECT Currency FROM Accounts WHERE Account_id='" + id + "'";
            if (mysql_query(conn, query1.c_str())) {
                cout << "Query failed: " << mysql_error(conn) << endl;
                return 1;
            }

            MYSQL_RES* res1 = mysql_store_result(conn);
            MYSQL_ROW row1 = mysql_fetch_row(res1);

            if (row1 != NULL) {
                currency = row1[0];
            }
            else {
                cout << "Currency information not found." << endl;
                return 1;
            }

            mysql_free_result(res1);


        }

        else if (val2 >= 2 && val2 <= 4) {

            cout << "Enter Amount: ";
            cin >> amount;


            current_amount = checkBalance(conn, id, amount);

            string query = "SELECT BALANCE FROM Accounts_balance WHERE Account_id='" + id + "'";

            if (mysql_query(conn, query.c_str())) {
                cout << "Query failed: " << mysql_error(conn) << endl;
                return 1;
            }

            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row = mysql_fetch_row(res);

            if (row == NULL) {
                cout << "Account not found." << endl;
                return 1;
            }

            string query1 = "SELECT Currency FROM Accounts WHERE Account_id='" + id + "'";
            if (mysql_query(conn, query1.c_str())) {
                cout << "Query failed: " << mysql_error(conn) << endl;
                return 1;
            }

            MYSQL_RES* res1 = mysql_store_result(conn);
            MYSQL_ROW row1 = mysql_fetch_row(res1);

            if (row1 != NULL) {
                inject_balance = row[0];
                currency = row1[0];

                cout << "Balance: " << balance << endl;
                cout << "Currency: " << currency << endl;
            }
            else {
                cout << "Currency information not found." << endl;
                return 1;
            }

            mysql_free_result(res);
            mysql_free_result(res1);

        }


        Transactions Transac(name, surname, phone, id, pw, trantype, current_amount, getCurrentDateTime(), currency, 1234);
        last_balance = balance(current_amount, val2, conn, Transac);
        cout << "Current Balance:" << last_balance << endl << endl;
        balance_update(conn, Transac, last_balance);
        Transaction_insert(conn, Transac);

    }

    else if (val == 0) {
        cout << "Exiting..." << endl;
    }


    mysql_close(conn);
    return 0;
}


int  show_customer_menu() {

    int val;
    cout << "<<<<<----MENU---->>>>>" << endl;
    cout << "1. Register New Customer:" << endl;
    cout << "2. Money Transactions" << endl;
    cout << "0. Exit" << endl;
    cout << "Enter value: ";
    cin >> val;
    if (val != 2 && (val != 1 && val != 0)) {
        show_customer_menu();
    }
    else { return val; }
}

int show_customer_menu2() {
    int val;
    cout << endl << "1. Credits Account" << endl;
    cout << "2. Bank Account" << endl;
    cout << "Enter value: ";
    cin >> val;
    cout << endl;
    if (val != 1 && val != 2) {
        cout << endl;
        cout << "Please Input 1 or 2 " << endl;
        show_customer_menu2();
        cout << endl;
    }
    else { return val; }

}

int show_customer_menu3() {

    int val;
    cout << "1. USD" << endl;
    cout << "2. EURO" << endl;
    cout << "3. TL" << endl;
    cout << "Enter value: ";
    cin >> val;


    if (val >= 1 && val <= 3) {
        return val;
    }
    else {
        cout << endl;
        cout << "Please Input 1,2 or 3" << endl;
        show_customer_menu3();
    }

}

int show_account_menu() {
    int val;
    cout << "1. Deposit" << endl;
    cout << "2. Withdrawal" << endl;
    cout << "3. Paying off the debt" << endl;
    cout << "4. Money Transfer" << endl;
    cout << "Please Select Transaction: ";
    cin >> val;
    cout << endl;
    if (val >= 1 && val <= 4) {
        return val;
    }
    else {
        cout << endl;
        cout << "Please choose between 1 - 4. " << endl;
        show_account_menu();
    }


}


void addCustomers(MYSQL* conn, Accounts Reg) {
    string query = "INSERT INTO Customers (Account_id, Name, Surname, Phone, psswd) VALUES ('" + Reg.getAccount_id() + "', '" + Reg.getName() + "', '" + Reg.getSurname() + "', '" + Reg.getPhNumber() + "', '" + Reg.getPassword() + "')";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        exit(1);
    }
    cout << "Account Registered Successfully." << endl;

    query = "INSERT INTO Accounts (Account_type, Currency, Account_id) VALUES ('" + Reg.getAccount_type() + "', '" + Reg.getCurrency() + "', '" + Reg.getAccount_id() + "')";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        exit(1);
    }
    cout << "Account Information Saved Successfully." << endl;
}

void Accounts_balance(MYSQL* conn, Accounts Reg) {
    string query = "INSERT INTO accounts_balance (Account_id, BALANCE,Currency_balance ) VALUES ('" + Reg.getAccount_id() + "', '" + to_string(0) + "', '" + Reg.getCurrency() + "')";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        exit(1);
    }

    cout << "The account has been opened." << endl;
}

void Transaction_insert(MYSQL* conn, Transactions Transac) {

    string query = "INSERT INTO money_transactions (Account_id,Trantype, Amount, transaction_date, Currency, Eftcode) VALUES ('" + Transac.getAccount_id() + "','" + Transac.getTrantype() + "', '" + to_string(Transac.getAmount()) + "', '" + Transac.getDate() + "', '" + Transac.getCurrency() + "', '" + to_string(Transac.getEFTcode()) + "')";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        exit(1);
    }

    cout << "Transaction successful." << endl;
}

int password_control(MYSQL* conn, const string& id, const string& pw) {
    MYSQL_ROW row;
    MYSQL_RES* res;

    string query = "SELECT * FROM Customers WHERE Account_id = '" + id + "' AND psswd= '" + pw + "'";
    mysql_query(conn, query.c_str());

    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);

    return (row == NULL) ? 0 : 1;
}

float balance(float amount, int value, MYSQL* conn, Transactions Transac) {////////////////////////////////////

    MYSQL_ROW row;
    MYSQL_RES* res;

    string query = "SELECT BALANCE FROM Accounts_balance WHERE Account_id = '" + Transac.getAccount_id() + "'";

    mysql_query(conn, query.c_str());
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);

    if (row == NULL) {
        cout << "Account not found." << endl;
        exit(1);
    }

    float current_balance = stof(row[0]);

    switch (value) {
    case 1: { return current_balance + amount; }
    case 2: { return current_balance - amount; }
    case 3: { return current_balance - amount; }
    case 4: { return current_balance - amount; }

    }
}

void balance_update(MYSQL* conn, Transactions Transac, float last_balance) {
    string query = "UPDATE Accounts_balance SET BALANCE = '" + to_string(last_balance) + "' WHERE Account_id = '" + Transac.getAccount_id() + "'";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        exit(1);
    }
}


float checkBalance(MYSQL* conn, const string& account_id, float amount) {

    string query = "SELECT BALANCE FROM Accounts_balance WHERE Account_id='" + account_id + "'";


    if (mysql_query(conn, query.c_str())) {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return 0;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        cout << "Failed to retrieve result: " << mysql_error(conn) << endl;
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        cout << "Account not found." << endl;
        mysql_free_result(res);
        return 0;
    }
    float balance = stof(row[0]);

    if (balance <= amount) {
        cout << "Your Balance:" << balance << endl;
        cout << "Insufficient balance. " << endl << endl;
        cout << "Amount:";
        cin >> amount;

        cout << endl;
        amount = checkBalance(conn, account_id, amount);

    }
    else {
        cout << "Sufficient balance." << endl;
        return amount;

    }
    return  amount;

    mysql_free_result(res);
}


bool isValidName(const string& name) {
    return all_of(name.begin(), name.end(), ::isalpha);
}

void name_control(string& name) {
    while (true) {
        cout << "Name: ";
        cin >> name;

        if (isValidName(name)) {
            cout << "Entered name: " << name << endl << endl;
            break;
        }
        else {
            cout << "Wrong entry." << endl << endl;
        }
    }
}

bool isValidSurname(const string& surname) {
    return all_of(surname.begin(), surname.end(), ::isalpha);
}

void Surname_control(string& surname) {
    while (true) {
        cout << "Surname: ";
        cin >> surname;

        if (isValidSurname(surname)) {
            cout << "Entered Surname: " << surname << endl << endl;
            break;
        }
        else {
            cout << "Wrong entry." << endl << endl;
        }
    }
}

bool isValidPhoneNumber(const string& phone) {
    return phone.length() == 10 && all_of(phone.begin(), phone.end(), ::isdigit);
}

void phone_number_control(string& phone) {
    while (true) {
        cout << "Enter the phone number (10 digits): ";
        cin >> phone;

        if (isValidPhoneNumber(phone)) {
            cout << "Entered phone number: " << phone << endl << endl;
            break;
        }
        else {
            cout << "Incorrect entry. The phone number must consist of 10 digits." << endl << endl;
        }
    }
}

bool isValidAccountId(const string& id) {
    return id.length() == 6 && all_of(id.begin(), id.end(), ::isdigit);
}

void Account_id_control(string& id) {
    while (true) {
        cout << "Enter your account id (6 digits):";
        cin >> id;

        if (isValidAccountId(id)) {
            cout << "Account id number:" << id << endl << endl;
            break;
        }
        else {
            cout << "Wrong entry. The account id must consist of 6 digits." << endl << endl;
        }
    }
}

bool isValidPassword(const string& pw) {
    return pw.length() == 6 && all_of(pw.begin(), pw.end(), ::isdigit);
}

void Password_control(string& pw) {
    while (true) {
        cout << "Password (6 digits): ";
        cin >> pw;

        if (isValidPassword(pw)) {
            cout << "Password number:" << pw << endl << endl;
            break;
        }
        else {
            cout << "Wrong entry. The password must consist of 6 digits." << endl << endl;
        }
    }
}




