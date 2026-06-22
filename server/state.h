#ifndef STATE_H
#define STATE_H

typedef enum e_state
{
    //status
    NEW_RS = 100,
    ACTIVE_RS,
    CGI_ACTIVE,
    CLOSED_RS,
    ERROR_RS,

    //client_state
    DISCONNECTED,
    ALIVE,

    //methods
    NONE_RS,
    GET_RS,
    GET_CGI_RS,
    POST_LIST_RS,
    GET_LIST_RS,
    POST_RS,
    CGI_RDWR,
    NO_MD
    

} t_state;

#endif