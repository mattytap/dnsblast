#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

int main()
{
    struct pollfd *pointer;
    int i;
    int noarrayelements;

    printf("Enter the size of the array: ");
    scanf("%d", &noarrayelements);
    printf("%d\n", noarrayelements);

    pointer = (struct pollfd *)calloc(noarrayelements, sizeof(struct pollfd));

    if (pointer == NULL)
    {
        printf("Memory allocation failed");
        exit(1); // exit the program
    }

    for (i = 0; i < noarrayelements; i++)
    {
        printf("Enter %d element: ", i);
        scanf("%d", pointer + i);
    }
    printf("\nprinting array of %d integers\n\n", noarrayelements);

    // calculate sum

    for (i = 0; i < noarrayelements; i++)
    {
        printf("%d ", *(pointer + i));
    }
    printf("\n");
    // signal to operating system program ran fine
    return 0;
}