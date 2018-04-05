#include "client.h"

void prompt() {
    
    cout << yellow << bold << "\n";
    (state == LOGGED_IN) ? cout << "You " : cout << "> ";
    cout << regular;
    cout << flush;
}

void prepare_fd(fd_set &readSet, fd_set &writeSet, int sockfd) {
    
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);
    FD_SET(STDIN_FILENO, &readSet);
    
    FD_ZERO(&writeSet);
    FD_SET(sockfd, &writeSet);
}

void handle_incoming_msg(int sockfd) {
    
    string msg;
    int recvStatus = recv(sockfd, (char*)&msg, MAX, MSG_DONTWAIT);
    
    if(recvStatus < 0) {
        if(recvStatus == EAGAIN || recvStatus == EWOULDBLOCK)
            print_error("\rTry after some time", false);
        else {
            errno = recvStatus;
            print_error("\rCan't read from server");
        }
    }
    if(recvStatus == 0) {
        print_error("\rConnection Closed From Server", false);
        exit(0);
    }
    else {
        if(msg == "logged in" || msg == "signed up") {
            state = LOGGED_IN;
            cout << green << bold << "\rSuccessful\n" << regular;
        }
        else if(msg == "username taken") {
            state = LOGGED_OUT;
            print_error("\rUsername already taken", false);
        }
        else if(msg == "mismatch") {
            state = LOGGED_OUT;
            print_error("\rUsername and password doesn't match", false);
        }
        else
            cout << msg;
    }
}

void send_msg(int sockfd) {
    
    if(pending.empty())
        return;
    
    string input = pending.front();
    pending.pop();
    cout << "\nsending to server: " <<input<<endl;

    int sendStatus = send(sockfd, (char*)&input, input.size(), MSG_DONTWAIT);
    if(sendStatus < 0) {
        if(sendStatus == EMSGSIZE)
            print_error("\rMessage size too large", false);
        else if(sendStatus == EAGAIN || sendStatus == EWOULDBLOCK)
            print_error("\rTry after some time", false);
        else {
            errno = sendStatus;
            print_error("\rSend message Error");
        }
    }

    else if(input == "\\logout")
        state = LOGGED_OUT;
    
    else if(input.substr(0, 6) == "\\login") {
        state = WAITING_FOR_LOGIN_RESPONSE;
    }
    else if(input.substr(0, 7) == "\\signup") {
        state = WAITING_FOR_SIGNUP_RESPONSE;
    }
}
