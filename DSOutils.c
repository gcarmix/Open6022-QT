#include "DSOutils.h"
#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif
void float2engStr(char* strout, double value)
{
    int pos = 0;
    if(value<1)
    {
        pos--;
        value = value*1000;
        if(value<1)
        {
            pos--;
            value = value*1000;
            if(value<1)
            {
                pos--;
                value = value*1000;
            }
        }
    }
    else if(value >=1000)
    {
        pos++;
        value = value/1000;
        if(value>=1000)
        {
            pos++;
            value = value/1000;
        }
    }

    switch(pos)
    {
    case 2:
        sprintf(strout,"%3.2fM",value);
        break;
    case 1:
        sprintf(strout,"%3.2fK",value);
        break;
    case 0:
        sprintf(strout,"%3.2f",value);
        break;
    case -1:
        sprintf(strout,"%3.2fm",value);
        break;
    case -2:
        sprintf(strout,"%3.2fu",value);
        break;
    case -3:
        sprintf(strout,"%3.2fn",value);
        break;

    }
}

#ifdef __cplusplus
    }
#endif
