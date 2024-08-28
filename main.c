#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char  name[64];       // node's name string
    char  type;
    struct node *child, *sibling, *parent;
} NODE;

NODE *root; 
NODE *cwd;
char *cmd[10] = {"pwd", "rmdir", "ls", "mkdir", "cd", "creat", "rm", "save", "reload", "quit"};
char* pathNames[64];
char* pwdStore[100];
char* command = NULL;
char* pathName = NULL;
char outputFileName[100];
char inputFileName[100];
// fill with list of commands
// other global variables

int initialize() 
{
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Filesystem initialized!\n");
    return 0; // Indicate success
}
// tokenizer used to sperate a path with /, the delim is /
int tokenizer(char* name) 
{
    // everytime the function is called we clear the pathNames
    if(pathNames[0] != NULL) {
        for(int j = 0; j != 65; j++) {
            pathNames[j] = NULL;
        }
    }

    int i = 0;
    char* token;

    const char* rootCheck = "/";
    // if the user inputted nothing then we just return 0
    if(name == NULL)
    {
        return i;
    }
    // tokenize the string and check if the first character is a / to know if they inputted a path or not
    if (name[0] == '/') {
        pathNames[i] = rootCheck;
        i++;
        token = strtok(name, "/");
    } 
    else 
    {
        token = strtok(name, "/");
        while (token != NULL) {
            pathNames[i] = token;
            i++;
            token = strtok(NULL, "/");
        }
        return i; // return the number of tokens found
    }

    // handle case where the input string starts with '/'
    while (token != NULL) {
        // skip the first token if it's "cd"
        if (strcmp(token, "cd") != 0) {
            pathNames[i] = token;
            i++;
        }
        token = strtok(NULL, "/");
    }
    return i; // return the number of tokens found
}
// save function that saves the current file system tree into the file
void save(NODE *node, FILE *file, char *basePath) 
{
    static int checking = 0;
    // if we find that the node is null when we move around then we just return and do nothing with it
    if (node == NULL)
        return;

    // write node type and path and if checking is 0 then we know we are at the root and rpint it
    if(checking == 0)
    {
        fprintf(file,"%c / \n", node->type);
        checking = 1;
    }
    else if(strcmp(basePath,"/") == 0)
    {
        fprintf(file, "%c /%s\n", node->type, node->name);
    }
    else
    {
        fprintf(file, "%c %s/%s\n", node->type, basePath, node->name);
    }


    // write child nodes 
if (node->child != NULL) 
{
    char newPath[256];
    // if checking is over 2 when know that we need to add a / because its a path
    if (checking > 2) 
    {
        strcpy(newPath, basePath);
        strcat(newPath, "/");
        strcat(newPath, node->name);
        save(node->child, file, newPath);
    } 
    else 
    {
        checking++;
        strcpy(newPath, basePath);
        strcat(newPath, node->name);
        save(node->child, file, newPath);
    }
}


    // write sibling nodes
    if (node->sibling != NULL) 
    {
        checking = 2;
        save(node->sibling, file, basePath);
    }
}
// make node function to call over and over for when im creating a node for the tree
NODE* makeNode(char* name, char type)
{
	NODE* newNode = (NODE *) malloc(sizeof(NODE));
	strcpy(newNode->name, name);
	newNode->type = type;
	newNode->sibling = newNode->parent = newNode->child = NULL;
	return newNode;
}
// reload function that reads from the file and creates the tree
void reload(FILE* file) 
{
    char line[256];
    char pathSave[256];
    int check = 0;
    while (fgets(line, sizeof(line), file) != NULL) 
    {
        char type;
        char path[256];
        sscanf(line, "%c %s", &type, path);
        strcpy(pathSave,path);
        check = tokenizer(path);
        // Split path into components
        char* token;
        token = strtok(path, "/");
        // if check is 2 we know we are at the name and check if its a directory or a file 
        if(check == 2 && type == 'D')
        {
            mkdir(token);
        }
        else if(check == 2 && type == 'F')
        {
            creat(token);
        }
        else if(token != NULL) 
        {
            if(type == 'D')
            {
                mkdir(pathSave);
            }
            else
            {
                creat(pathSave);
            }
        }
    }
}
// function to find the command
int find_commad(char* user_command)
{
    int i = 0;
    while(cmd[i])
    {
        if(strcmp(user_command,cmd[i]) == 0)
        {
            return i;
            i++;
        }
        return -1;
    }
}
// function to print the commands
void printCommands() 
{
    printf("Available commands:\n");
    for (int i = 0; i < 10; i++) 
    {
        printf("%d: %s\n", i, cmd[i]);
    }
}
// ls function to list the directories and files within the searched path
int ls(char* name)
{
    int check = 0;
    int i = 0;
    int found = 0;
    const char* rootCheck = "/";
    check = tokenizer(name); // /WSU/360/HW1 -> pathNames[64] WSU,360,HW1,NULL,...
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    // check if pathNames is NULL and if it is then we know they just typed ls, so we just print the directories and files connected to the cwd
    if(pathNames[0] == NULL)
    {
        if(pCur->child != NULL)
        {
            pCur = pCur->child;
            printf("%c %s\n", pCur->type,pCur->name);
            pCur = pCur->sibling;
            while(pCur != NULL)
            {
                printf("%c %s\n", pCur->type,pCur->name);
                pCur = pCur->sibling;
            }
            return 1;
        }
        else
        {
            return 1;
        }
    }
    else // the else statement checks and loops through the tree till it find the correct path that the user was looking for 
    {
    if (strcmp(pathNames[0], cwd->name) == 0) 
    {
        i++;
        while(pCur != NULL)
        {
            pCur = pCur->child;
            check--;
            while(pCur != NULL)
            {
                if(strcmp(pathNames[i],pCur->name) == 0)
                {
                    pPrev = pCur;
                    pCur = pCur->child;
                    i++;
                    check--;
                    if(check == 0)
                    {
                        pCur = pPrev->child;
                        printf("%c %s\n", pCur->type,pCur->name);
                        pCur = pCur->sibling;
                        while(pCur != NULL)
                        {
                            printf("%c %s\n", pCur->type,pCur->name);
                            pCur = pCur->sibling;

                        }
                        return 1;
                    }
                }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0) // path doesn't exist if we have a NULL pCur 
                    {
                        if(pCur == NULL)
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling;
                    }
            }
        }
    }

printf("Path Doesn't exist! \n"); // if nothing fits then we know the path doesn't exist
return 0;
// -------------------------------------------------------------
    }
}
// rmdir function that only removes directories entered by the user
int rmdir(char* name)
{
    int check = 0;
    int i = 0;
    int found = 0;
    const char* rootCheck = "/";
    check = tokenizer(name); // /WSU/360/HW1 -> pathNames[64] WSU,360,HW1,NULL,...
    NODE* pPrevChild = NULL;
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    // checks if a pathname was given and return 0 if it was not entered
    if(name == NULL)
    {
        printf("Pathname wasn't given!\n");
        return 0;
    }
    // if it enters this if statement we know that it was just a a single word
    if(check == 1)
    {
            if(cwd->child == NULL) // if it enters then we know that it doesn't go anywhere so it doesn't exist
            {
                printf("Directory doesn't exists!\n");
                return 0;
            }
            else
            {
                pPrev = pCur;
                pCur = cwd->child;
                if(strcmp(pathNames[0], pCur->name) == 0) // we found the correct node to remove
                {
                    if(pCur->child == NULL) // we know that its empty
                    {
                        if(pCur->sibling != NULL) // if it has a sibling then we need to make it the child
                        {
                            if(pCur->type == 'F') //  checks if its a file or not
                            {
                                printf("Can't remove file with rmdir!\n");
                                return 0;
                            }
                            pPrev->child = pCur->sibling;
                            pCur->parent = NULL;
                            pCur->sibling = NULL;
                            free(pCur);
                            return 1;
                        }
                        else // otherwise we just delete it
                        {
                            if(pCur->type == 'F')
                            {
                                printf("Can't remove file with rmdir!\n");
                                return 0;
                            }
                            pPrev->child = NULL;
                            pCur->parent = NULL;
                            free(pCur);
                            return 1;
                        }
                    }
                    else // we know that its not empty if it has a child
                    {
                        printf("Directory isn't empty!\n");
                        return 0;
                    }
                }
                while(pCur->sibling != NULL) // move through the siblings
                {
                    pPrevChild = pCur;
                    pCur = pCur->sibling;
                    if(strcmp(pathNames[0], pCur->name) == 0) // if we find it then we check the same checks
                    {
                        if(pCur->child == NULL)
                        {
                            if(pCur->sibling != NULL) // if there's a sibling after then we have to connect it
                            {
                                if(pCur->type == 'F') // cant remove a file in rmdir and if its not then we delete it
                                {
                                    printf("Can't remove file with rmdir!\n");
                                    return 0;
                                }
                                pPrevChild->sibling = pCur->sibling;
                                pCur->parent = NULL;
                                pCur->sibling = NULL;
                                free(pCur);
                                return 1;
                            }
                            else // if not then we can just delete the node
                            {
                                if(pCur->type == 'F')
                                {
                                    printf("Can't remove file with rmdir!\n");
                                    return 0;
                                }
                                pPrevChild->sibling = NULL;
                                pCur->parent = NULL;
                                pCur->sibling = NULL;
                                free(pCur);
                                return 1;
                            }
                        }
                        else
                        {
                            printf("Directory isn't empty!\n"); // if the checks above don't get it then we know its not empty
                            return 0;
                        }
                    }
                }
                printf("Directory doesn't exists!\n"); // the directory name wasn't in the tree
                return 0;
            }
    }
    else // else statement is for if the user inputted a path, it has the same checks as above
    {
       if (strcmp(pathNames[0], cwd->name) == 0) 
       {
        i++;
            while(pCur != NULL)
            {
                pCur = pCur->child;
                check--;
                while(pCur != NULL)
                {
                    if(strcmp(pathNames[i],pCur->name) == 0)
                    {
                        pPrev = pCur;
                        pCur = pCur->child;
                        i++;
                        check--;
                        if(check == 1)
                        {
                            if(pPrev->child == NULL)
                            {
                                printf("Directory doesn't exists!\n");
                                return 0;
                            }
                            else
                            {
                                pPrevChild = pCur;
                                pCur = pPrev->child;
                                if(strcmp(pathNames[i], pCur->name) == 0)
                                {
                                    if(pCur->child == NULL)
                                    {
                                        if(pCur->sibling != NULL)
                                        {
                                            if(pPrev->child == pCur)
                                            {
                                                if(pCur->type == 'F')
                                                {
                                                    printf("Can't remove file with rmdir!\n");
                                                    return 0;
                                                }
                                                pPrev->child = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                            else
                                            {
                                                if(pCur->type == 'F')
                                                {
                                                    printf("Can't remove file with rmdir!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            if(pCur->type == 'F')
                                            {
                                                printf("Can't remove file with rmdir!\n");
                                                return 0;
                                            }
                                            pPrev->child = NULL;
                                            pCur->parent = NULL;
                                            pCur->sibling = NULL;
                                            free(pCur);
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        printf("Directory isn't empty!\n");
                                        return 0;
                                    } 
                                }
                                while(pCur->sibling != NULL)
                                {
                                    pPrevChild = pCur;
                                    pCur = pCur->sibling;
                                    if(strcmp(pathNames[i], pCur->name) == 0)
                                    {
                                        if(pCur->child == NULL)
                                        {
                                            if(pCur->sibling != NULL)
                                            {
                                                if(pCur->type == 'F')
                                                {
                                                    printf("Can't remove file with rmdir!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                            else
                                            {
                                                if(pCur->type == 'F')
                                                {
                                                    printf("Can't remove file with rmdir!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = NULL;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            printf("Directory isn't empty!\n");
                                            return 0;
                                        }
                                    }
                                }
                                printf("Directory doesn't exists!\n");
                                return 0;
                            }
                        }
                    }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0)
                    {
                        if(pCur == NULL)
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling;
                    }
                }
            }
       }
    }
    printf("Path Doesn't exist! \n");
    return 0;
    // -------------------------------------------------------------
}
// creat function that creates a file
int creat (char* name)
{
    int check = 0;
    int i = 0;
    int found = 0;
    const char* rootCheck = "/";
    check = tokenizer(name); // /WSU/360/HW1 -> pathNames[64] WSU,360,HW1,NULL,...
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    // checks if a pathname was given and return 0 if it was not entered
    if(name == NULL)
    {
        printf("Pathname wasn't given!\n");
        return 0;
    }
    if(check == 1)
    {
        if(cwd->child == NULL)  // we know theres a free space to create a node thats a file
        {
            NODE* newDirect = makeNode(pathNames[i],'F');
            cwd->child = newDirect;
            newDirect->parent = cwd;
            newDirect->sibling = NULL;
        }
        else // we need to go through the siblings
        {
            pPrev = pCur;
            pCur = cwd->child;
            if(strcmp(pathNames[0], pCur->name) == 0) // if we find the same name then we know that it exists already
            {
                printf("Directory already exists!\n");
                return 0;
            }
            while(pCur->sibling != NULL) // we move through the siblings till its NULL and make a node
            {
                pCur = pCur->sibling;
                if(strcmp(pathNames[0], pCur->name) == 0) // if one of the siblings has the same name then we dont make a node
                {
                    printf("Directory already exists!\n");
                    return 0;
                }
            }
            NODE* newDirect = makeNode(pathNames[i],'F');
            pCur->sibling = newDirect;
            newDirect->parent = pPrev;
        }
    }
    else
    {
       if (strcmp(pathNames[0], cwd->name) == 0) // check that were at the root 
       {
            i++;
            while(pCur != NULL) 
            {
                pCur = pCur->child;
                check--; // we go down childs till check is equal to 1 to know we found the level we're suppose to be at
                while(pCur != NULL)
                {
                    if(strcmp(pathNames[i],pCur->name) == 0) 
                    {
                        pPrev = pCur;
                        pCur = pCur->child;
                        i++;
                        check--;
                        if(check == 1)
                        {
                            if(pPrev->child == NULL) // check if the prev is NULL and if it is then we make the node
                            {
                                if(pPrev->type == 'F') // check to see if the parent is a file and if it is then we cant make that node
                                {
                                    printf("Can't create file from a file\n");
                                    return 0;
                                }
                                NODE* newDirect = makeNode(pathNames[i],'F');
                                newDirect->parent = pPrev;
                                pPrev->child = newDirect;
                                newDirect->sibling = NULL;
                                return 1;
                            }
                            else
                            {
                                pCur = pPrev->child;
                                if(strcmp(pathNames[i], pCur->name) == 0) //  compare the names and if they are the same we know it exists already
                                {
                                    printf("File already exists!\n");
                                    return 0;
                                }
                                while(pCur->sibling != NULL)
                                {
                                    pCur = pCur->sibling;
                                    if(strcmp(pathNames[i], pCur->name) == 0) //  compare the names and if they are the same we know it exists already
                                    {
                                        printf("File already exists!\n");
                                        return 0;
                                    }
                                }
                                if(pPrev->type == 'F') 
                                {
                                    printf("Can't create file from a file\n");
                                    return 0;
                                }
                                NODE* newDirect = makeNode(pathNames[i],'F');
                                newDirect->parent = pPrev;
                                pCur->sibling = newDirect;
                                newDirect->sibling = NULL;
                                return 1;
                            }
                        }
                    }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0)
                    {
                        if(pCur == NULL)
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling;
                    }
                }
            }

printf("Path Doesn't exist! \n");
return 0;
// -------------------------------------------------------------
    }
}
}
// rm function that removes a file
int rm(char* name)
{
    int check = 0;
    int i = 0;
    int found = 0;
    const char* rootCheck = "/";
    check = tokenizer(name); // /WSU/360/HW1 -> pathNames[64] WSU,360,HW1,NULL,...
    NODE* pPrevChild = NULL;
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    if(name == NULL) // if a path wasnt given by the user then we return 0
    {
        printf("Pathname wasn't given!\n");
        return 0;
    }
    if(check == 1) // if the name is only one word 
    {
            if(cwd->child == NULL) // if theres no child from the cwd then we know that it doesn't exist
            {
                printf("Directory doesn't exists!\n");
                return 0;
            }
            else
            {
                pPrev = pCur;
                pCur = cwd->child; // move down through the child node
                if(strcmp(pathNames[0], pCur->name) == 0) // if the names match then we found the node
                {
                    if(pCur->child == NULL) // if it doesn't have a child then we enter
                    {
                        if(pCur->sibling != NULL) // if sibling is not NULL then we need to connect a new child to the parent
                        {
                            if(pCur->type == 'D') // if the node is a directory we cant remove it with rm
                            {
                                printf("Can't remove directory with rm!\n");
                                return 0;
                            }
                            pPrev->child = pCur->sibling;
                            pCur->parent = NULL;
                            pCur->sibling = NULL;
                            free(pCur);
                            return 1;
                        }
                        else
                        {
                            if(pCur->type == 'D') // if the node is a directory we cant remove it with rm
                            {
                                printf("Can't remove directory with rm!\n");
                                return 0;
                            }
                            pPrev->child = NULL;
                            pCur->parent = NULL;
                            free(pCur);
                            return 1;
                        }
                    }
                    else
                    {
                        printf("Directory isn't empty!\n");
                        return 0;
                    }
                }
                while(pCur->sibling != NULL)
                {
                    pPrevChild = pCur;
                    pCur = pCur->sibling; // move through the siblings till we find the right node
                    if(strcmp(pathNames[0], pCur->name) == 0)
                    {
                        if(pCur->child == NULL)
                        {
                            if(pCur->sibling != NULL) // we need to connect the siblings so all the siblings connect
                            {
                                if(pCur->type == 'D')
                                {
                                    printf("Can't remove directory with rm!\n"); // if the node is a directory we cant remove it with rm
                                    return 0;
                                }
                                pPrevChild->sibling = pCur->sibling;
                                pCur->parent = NULL;
                                pCur->sibling = NULL;
                                free(pCur);
                                return 1;
                            }
                            else
                            {
                                if(pCur->type == 'D')
                                {
                                    printf("Can't remove directory with rm!\n"); // if the node is a directory we cant remove it with rm
                                    return 0;
                                }
                                pPrevChild->sibling = NULL;
                                pCur->parent = NULL;
                                pCur->sibling = NULL;
                                free(pCur);
                                return 1;
                            }
                        }
                        else
                        {
                            printf("Directory isn't empty!\n");
                            return 0;
                        }
                    }
                }
                printf("Directory doesn't exists!\n");
                return 0;
            }
    }
    else // this has the same logic but the user inputted a path so if needs to go down till check is 1 then we in the correct level of the tree
    {
       if (strcmp(pathNames[0], cwd->name) == 0) 
       {
        i++;
            while(pCur != NULL)
            {
                pCur = pCur->child;
                check--;
                while(pCur != NULL)
                {
                    if(strcmp(pathNames[i],pCur->name) == 0)
                    {
                        pPrev = pCur;
                        pCur = pCur->child;
                        i++;
                        check--;
                        if(check == 1)
                        {
                            if(pPrev->child == NULL)
                            {
                                printf("Directory doesn't exists!\n");
                                return 0;
                            }
                            else
                            {
                                pPrevChild = pCur;
                                pCur = pPrev->child;
                                if(strcmp(pathNames[i], pCur->name) == 0)
                                {
                                    if(pCur->child == NULL)
                                    {
                                        if(pCur->sibling != NULL)
                                        {
                                            if(pPrev->child == pCur)
                                            {
                                                if(pCur->type == 'D')
                                                {
                                                    printf("Can't remove directory with rm!\n");
                                                    return 0;
                                                }
                                                pPrev->child = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                            else
                                            {
                                                if(pCur->type == 'D')
                                                {
                                                    printf("Can't remove directory with rm!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            if(pCur->type == 'D')
                                            {
                                                printf("Can't remove directory with rm!\n");
                                                return 0;
                                            }
                                            pPrev->child = NULL;
                                            pCur->parent = NULL;
                                            pCur->sibling = NULL;
                                            free(pCur);
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        printf("Directory isn't empty!\n");
                                        return 0;
                                    } 
                                }
                                while(pCur->sibling != NULL)
                                {
                                    pPrevChild = pCur;
                                    pCur = pCur->sibling;
                                    if(strcmp(pathNames[i], pCur->name) == 0)
                                    {
                                        if(pCur->child == NULL)
                                        {
                                            if(pCur->sibling != NULL)
                                            {
                                                if(pCur->type == 'D')
                                                {
                                                    printf("Can't remove directory with rm!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = pCur->sibling;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                            else
                                            {
                                                if(pCur->type == 'D')
                                                {
                                                    printf("Can't remove directory with rm!\n");
                                                    return 0;
                                                }
                                                pPrevChild->sibling = NULL;
                                                pCur->parent = NULL;
                                                pCur->sibling = NULL;
                                                free(pCur);
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            printf("Directory isn't empty!\n");
                                            return 0;
                                        }
                                    }
                                }
                                printf("Directory doesn't exists!\n");
                                return 0;
                            }
                        }
                    }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0)
                    {
                        if(pCur == NULL)
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling;
                    }
                }
            }
       }
    }
    printf("Path Doesn't exist! \n");
    return 0;
    // -------------------------------------------------------------
}
// pwd function that prints the absolute path
int pwd(NODE* CWD)
{
    int check = 0;
    int i = 0;
    int j = 0;
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    const char* rootCheck = "/"; 
    printf("/");
    if(CWD->name == root->name) // check to see if the cwd is the root then we just return 1 and / is all thats printed
    {
        return 1;
    }
    else
    {
        while(pCur != root) // store the whole path into pwdStore to later print out
        {
            pwdStore[i] = pCur->name;
            i++;
            pCur = pCur->parent;
        }
        i--;
        while(i != -1) // later we print the whole path till we get to the end of pwdStore
        {
            printf("%s", pwdStore[i]);
            i--;
            if(i != -1)
            {
                printf("/");
            }
        }
        // printf("\n");
        return 1;
    }

}
// cd function changes the directory
int cd(char* name) 
{
    int check = 0;
    int i = 0;
    check = tokenizer(name);
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    const char* rootCheck = "/";
    if(name == NULL) // if the user inputs nothing then we go back to the root
    {
        cwd = root;
        return 1;
    }
    if(check == 1) // if the check is 1 then we directory change is one level lower 
    { 
        if(pCur->child == NULL) 
        {
            printf("Path doesn't exist!\n");
            return 0;
        }
        pPrev = pCur;
        pCur = pCur->child;
        while(pCur != NULL) 
        {
            if(strcmp(pathNames[i], pCur->name) == 0) // we found the name we want
            {
                if(pCur->type == 'F') // but if its a file then we dont change the cwd and return 0
                {
                    printf("Can't make cwd a file!\n");
                    return 0;
                }
                cwd = pCur; // otherwise we do change it
                return 1;
            }
            pPrev = pCur;
            pCur = pCur->sibling; // go through the siblings till we find it
        }
        if(pCur == NULL)  // but if pCur is ever NULL then we know the path doesn't exist
        {
            printf("Path doesn't exist!\n");
            return 0;
        }
    }
    else
    {
       if (strcmp(pathNames[0], cwd->name) == 0)
       {
            i++;
            while(pCur != NULL) 
            {
                pCur = pCur->child; // move down each child till we get to the correct level
                check--;
                while(pCur != NULL)
                {
                    if(strcmp(pathNames[i],pCur->name) == 0) // if we find a name thats the same we move down the tree
                    {
                        pPrev = pCur;
                        pCur = pCur->child;
                        i++;
                        check--;
                        if(check == 1) // if check is 1 then we know that we're at the right level
                        {
                            if(pPrev->child == NULL) 
                            {
                                printf("Path Doesn't exist! \n"); // if the parent doesn't have a kid we know that path doesn't exist
                                return 0;
                            }
                            else
                            {
                                pCur = pPrev->child;
                                if(strcmp(pathNames[i], pCur->name) == 0) // if the names match we know we found the node
                                {
                                    if(pCur->type == 'F') // if the node is a file then we cant make it the cwd
                                    {
                                        printf("Can't make cwd a file!\n");
                                        return 0;
                                    }
                                    cwd = pCur; // otherwise we make it the cwd
                                    return 1;
                                }
                                while(pCur->sibling != NULL) // we move down the siblings till we find the right one
                                {
                                    pCur = pCur->sibling;
                                    if(strcmp(pathNames[i], pCur->name) == 0) // node found if names match
                                    {
                                        if(pCur->type == 'F') // if the node is a file then we cant make it the cwd
                                        {
                                            printf("Can't make cwd a file!\n");
                                            return 0;
                                        }
                                        cwd = pCur; // otherwise we make it the cwd
                                        return 1;
                                    }
                                }
                                printf("Path Doesn't exist! \n");
                                return 0;
                            }
                        }
                    }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0) 
                    {
                        if(pCur == NULL) //  if pCur is ever NULL then we know the path doesn't exist
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling; // go through the siblings till we find one the compares
                    }
                }
            }
       }

printf("Path Doesn't exist! \n");
return 0;
// -------------------------------------------------------------
    }
}
// mkdir function that creates a directory
int mkdir (char* name)
{
    int check = 0;
    int i = 0;
    int found = 0;
    const char* rootCheck = "/";
    check = tokenizer(name); // /WSU/360/HW1 -> pathNames[64] WSU,360,HW1,NULL,...
    NODE* pCur = cwd;
    NODE* pPrev = NULL;
    if(name == NULL) // if name is NULL then user didn't input anything
    {
        printf("Pathname wasn't given!\n");
        return 0;
    }
    if(check == 1) // user only inputted one word
    {
            if(cwd->child == NULL) // if the child of the cwd is NULL then we make a node 
            {
                    NODE* newDirect = makeNode(pathNames[i],'D');
                    cwd->child = newDirect;
                    newDirect->parent = cwd;
                    newDirect->sibling = NULL;
            }
            else
            {
                pPrev = pCur;
                pCur = cwd->child; // otherwise we move down to the child node and check the siblings 
                if(strcmp(pathNames[0], pCur->name) == 0) // if any name matches then we know that the directory exits
                {
                    printf("Directory already exists!\n");
                    return 0;
                }
                while(pCur->sibling != NULL)
                {
                    pCur = pCur->sibling;
                    if(strcmp(pathNames[0], pCur->name) == 0) // if any name matches then we know that the directory exits
                    {
                        printf("Directory already exists!\n");
                        return 0;
                    }
                }
                NODE* newDirect = makeNode(pathNames[i],'D');
                pCur->sibling = newDirect;
                newDirect->parent = pPrev;
            }
    }
    else
    {
       if (strcmp(pathNames[0], cwd->name) == 0) // check that were at the root 
       {
            i++;
            while(pCur != NULL)
            {
                pCur = pCur->child;
                check--; // we go down childs till check is equal to 1 to know we found the level we're suppose to be at
                while(pCur != NULL)
                {
                    if(strcmp(pathNames[i],pCur->name) == 0)
                    {
                        pPrev = pCur;
                        pCur = pCur->child;
                        i++;
                        check--; // we go down childs till check is equal to 1 to know we found the level we're suppose to be at
                        if(check == 1)
                        {
                            if(pPrev->child == NULL) // check if the prev is NULL and if it is then we make the node
                            {
                                if(pPrev->type == 'F') // check to see if the parent is a file and if it is then we cant make that node
                                {
                                    printf("Can't create directory from a file\n");
                                    return 0;
                                }
                                NODE* newDirect = makeNode(pathNames[i],'D');
                                newDirect->parent = pPrev;
                                pPrev->child = newDirect;
                                newDirect->sibling = NULL;
                                return 1;
                            }
                            else
                            {
                                pCur = pPrev->child;
                                if(strcmp(pathNames[i], pCur->name) == 0) // compare the names and if they are the same we know it exists already
                                {
                                    printf("Directory already exists!\n");
                                    return 0;
                                }
                                while(pCur->sibling != NULL)
                                {
                                    pCur = pCur->sibling;
                                    if(strcmp(pathNames[i], pCur->name) == 0) // compare the names and if they are the same we know it exists already
                                    {
                                        printf("Directory already exists!\n");
                                        return 0;
                                    }
                                }
                                if(pPrev->type == 'F') // check to see if the parent is a file and if it is then we cant make that node
                                {
                                    printf("Can't create directory from a file\n");
                                    return 0;
                                }
                                NODE* newDirect = makeNode(pathNames[i],'D');
                                newDirect->parent = pPrev;
                                pCur->sibling = newDirect;
                                newDirect->sibling = NULL;
                                return 1;
                            }
                        }
                    }
                    if(pCur == NULL || strcmp(pathNames[i],pCur->name) != 0)
                    {
                        if(pCur == NULL)
                        {
                            printf("Path Doesn't exist! \n");
                            return 0;
                        }
                        pPrev = pCur;
                        pCur = pCur->sibling;
                    }
                }
            }
       }

printf("Path Doesn't exist! \n");
return 0;
// -------------------------------------------------------------
    }
}

int main() 
{
    int j = 0;
    int input = 0;
    int value = 0;
    int checker = 0;
    char inputName[100];
    initialize();

    while (command == NULL || strcmp(command,"quit") != 0) 
    {
        j = 0;
        // printf("Enter command (type the index):\n");
        // printCommands();
        checker = 0;
        printf("C:");
        value = pwd(cwd); 
        printf(">");
        scanf(" %[^\n]", inputName); 
        command = strtok(inputName, " ");
        pathName = strtok(NULL, " ");
        if(strcmp(command,"pwd") == 0)
        {
            value = pwd(cwd);
            printf("\n");
            checker = 1;  
        }
        if(strcmp(command,"rmdir") == 0)
        {
            value = rmdir(pathName);  
            checker = 1;
        }
        if(strcmp(command,"ls") == 0)
        {
            value = ls(pathName);  
            checker = 1;
        }
        if(strcmp(command,"mkdir") == 0)
        {
            value = mkdir(pathName); 
            checker = 1; 
        }
        if(strcmp(command,"cd") == 0)
        {
            value = cd(pathName);   
            checker = 1;
        }
        if(strcmp(command,"creat") == 0)
        {
            value = creat(pathName);   
            checker = 1;
        }
        if(strcmp(command,"rm") == 0)
        {
            value = rm(pathName);   
            checker = 1;
        }
        if(strcmp(command,"save") == 0)
        {
            FILE *file = fopen("fssim_Lopez-Garcia.txt", "w");
            save(root, file, "");
            checker = 1;
        }
        if(strcmp(command,"reload") == 0)
        {
            FILE *file = fopen("fssim_Lopez-Garcia.txt", "r");
            reload(file);
            checker = 1;
        }
        if(strcmp(command,"quit") == 0)
        {
            FILE *file = fopen("fssim_Lopez-Garcia.txt", "w");
            save(root, file, "");
            return 0;
        }
        if(checker == 0)
        {
            printf("Invalid command index\n");
        }
    }
    return 0;
}

