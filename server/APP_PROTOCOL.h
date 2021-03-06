#ifndef SERVER_APP_PROTOCOL_H
#define SERVER_APP_PROTOCOL_H

#include <string>

static const int MAXCONTROLSIZE = 1;
static const int MAXCONTROLSIZE_DATA = 2;
static const int MAX_APPTS_SIZE_LENGTH = 10;

// Control Values (bytes)
static const int C_DISCONN_BIN = 0b0000;
static const int C_LOGIN_BIN = 0b0001;
static const int C_ADD_USER_BIN = 0b0010;
static const int C_GET_USER_APPTS_BIN = 0b0011;
static const int C_ADD_APPT_BIN = 0b0100;
static const int C_DEL_APPT_BIN = 0b0101;
static const int C_UPDATE_APPT_BIN = 0b0110;
static const int C_GET_USER_DATA_BIN = 0b0111;
static const int C_UPDATE_USER_DATA_BIN = 0b1000;

// Control Values (std::string)
static const std::string C_DISCONN = std::to_string(C_DISCONN_BIN);
static const std::string C_LOGIN = std::to_string(C_LOGIN_BIN);
static const std::string C_ADD_USER = std::to_string(C_ADD_USER_BIN);
static const std::string C_GET_USER_APPTS = std::to_string(C_GET_USER_APPTS_BIN);
static const std::string C_ADD_APPT = std::to_string(C_ADD_APPT_BIN);
static const std::string C_DEL_APPT = std::to_string(C_DEL_APPT_BIN);
static const std::string C_UPDATE_APPT = std::to_string(C_UPDATE_APPT_BIN);
static const std::string C_GET_USER_DATA = std::to_string(C_GET_USER_DATA_BIN);
static const std::string C_UPDATE_USER_DATA = std::to_string(C_UPDATE_USER_DATA_BIN);

#endif //SERVER_APP_PROTOCOL_H
