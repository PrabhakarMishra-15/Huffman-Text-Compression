#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef unsigned char u_char;
struct Node
{
    u_char letter;
    struct Node *left,*right;
};

typedef struct Node Node;

int is_leaf_node(Node *node)
{
    return !(node->left || node->right);
}

void read_new_char(FILE *text,int *i,u_char *c)
{
    if(*i == 0)
    {
        *i = 128;
        fread(c,1,1,text);
        //printf("%d ",*c);
    }
}

Node* make_leaf_node(FILE *text,int *i,u_char *c)
{
    //Allocate dynamic memory for Node pointer
    Node *temp = (Node *)malloc(sizeof(Node));

    u_char ch = 0;
    int power = 128;

    //printf("Decoding a new letter:\n");
    for(int x = 0;x < 8;x++)
    {
        (*i) /= 2;
        read_new_char(text,i,c);
        //printf("%d %d\n",*c,*i);
        if((*c) & (*i))
        {
            //printf("+%d\n",power);
            ch += power;
        }
        power /= 2;
    }
    //printf("Decoded letter ASCII = %d\n\n",ch);
    temp->letter = ch;
    temp->left = NULL; temp->right = NULL;

    return temp;
}

Node* make_internal_node(Node *left,Node *right)
{
    //Allocate dynamic memory for Node pointer
    Node *temp = (Node *)malloc(sizeof(Node));

    //Allocate dynamic memory for the left and right Node pointers
    temp->left = (Node *)malloc(sizeof(Node));
    temp->right = (Node *)malloc(sizeof(Node));

    temp->left = left; temp->right = right;

    return temp;
}

Node *make_tree_from_text(FILE *text)
{
    static int i = 0;
    static u_char c;

    i /= 2;
    read_new_char(text,&i,&c);

    if((c & i) == 0)
    {
        //printf("New internal node:\n");
        Node *left = make_tree_from_text(text);
        Node *right = make_tree_from_text(text);

        return make_internal_node(left,right);
    }
    //printf("New leaf node:\n");
    return make_leaf_node(text,&i,&c);
}

void decode_text(Node *tree,FILE *input,FILE *output)
{
    Node *root = tree;
    u_char input_char,temp;
    int power;

    u_char padding;
    fread(&padding,1,1,input);

    fread(&input_char,1,1,input);
    while(1)
    {
        if(!fread(&temp,1,1,input))
            break;
        power = 128;
        while(power > 0)
        {
            if(input_char & power)
                tree = tree->right;
            else
                tree = tree->left;
            if(is_leaf_node(tree))
            {
                fwrite(&(tree->letter),1,1,output);
                tree = root;
            }
            power /= 2;
        }
        input_char = temp;
    }
    power = 128;
    if(padding == 0)
        padding = 8;

    while(padding > 0)
    {
        if(input_char & power)
            tree = tree->right;
        else
            tree = tree->left;
        if(is_leaf_node(tree))
        {
            fwrite(&(tree->letter),1,1,output);
            tree = root;
        }

        power /= 2;
        padding--;
    }
    fclose(output);
}

void main()
{
    char file_name[100];
    printf("Enter the name of the file to be decompressed:\n");
    gets(file_name);

    FILE *input = fopen(file_name,"rb");
    Node *tree = make_tree_from_text(input);

    file_name[strlen(file_name) - 4] = '\0';
    FILE *output = fopen(file_name,"wb");
    //printf("Traversing the tree:\n");
    decode_text(tree,input,output);
}
