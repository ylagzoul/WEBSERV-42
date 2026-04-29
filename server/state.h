#ifndef STATE_H
#define STATE_H

typedef enum e_state
{
    NEW_RS = 100,
    ACTIVE_RS,
    CLOSED_RS,
    NONE_RS,
    GET_RS,
    POST_LIST_RS,
    GET_LIST_RS,
    POST_RS,
    ERROR_RS

} t_state;

#endif