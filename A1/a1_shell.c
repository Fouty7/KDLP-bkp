#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s$ \n", cwd);
    return 0;
}
