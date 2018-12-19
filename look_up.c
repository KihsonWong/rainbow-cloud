#include "look_up.h"

int get_accepted_client_num(int *client, int max)
{
    int ret = 0, i;
    for(i=0;i<max;i++){
        if(client[i]!=-1)
            ret++;
    }
    return ret;
}
