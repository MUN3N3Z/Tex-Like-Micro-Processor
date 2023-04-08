#include "stdio.h"
#include "string.h"
#include "strings.h"

int main(int argc, char const *argv[])
{
    char name[5] = "Tony";

    char name2[5] = NULL;
    if (strcmp(name, name2) == 0)
    {
        printf("Wrong evaluation");
    }
    else
    {
        printf("EUREKA!!");
    }
    
    
    return 0;
}
