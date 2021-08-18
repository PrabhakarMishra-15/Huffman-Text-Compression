#include<stdio.h>
#include<stdlib.h>
typedef unsigned char u_char;
struct Node
{
    int freq;
    u_char letter;
    struct Node *left,*right;
};

typedef struct Node Node;

int is_leaf_node(Node *node)
{
    return !(node->left || node->right);
}

Node* make_leaf_node(u_char letter,int freq)
{
    Node *temp = (Node *)malloc(sizeof(Node));

    temp->freq = freq; temp->letter = letter;
    temp->left = NULL; temp->right = NULL;

    return temp;
}

Node* make_internal_node(Node *left,Node *right)
{
    Node *temp = (Node *)malloc(sizeof(Node));

    temp->freq = left->freq + right->freq;
    temp->left = left; temp->right = right;

    return temp;
}

int *count_frequency(char *file_name)
{
    FILE *fptr = fopen(file_name,"rb");
    if(!fptr)
        return NULL;
    /* size is 257 since a one-byte character can
    have one of 256 values. The 257th element stores
    the number of characters with a non-zero count. */
    int *counts = (int *)calloc(257,sizeof(int));

    u_char c;
    while(fread(&c,1,1,fptr))
    {
        if(counts[c] == 0)
            counts[256]++;
        counts[c]++;
    }
    fclose(fptr);

    return counts;
}

void sort(u_char *uniq,int *freq,int size)
{
    int i,j,tempn;
    u_char tempc;
    for(i = 1; i < size; i++)
    {
        j = i - 1;
        tempn = freq[i]; tempc = uniq[i];
        while(j >= 0 && tempn < freq[j])
        {
            freq[j + 1] = freq[j];
            uniq[j + 1] = uniq[j];
            j--;
        }
        freq[j + 1] = tempn; uniq[j + 1] = tempc;
    }
}

/* ---------- Queue implementation ---------- */
typedef struct
{
    Node **queue;
    int front,rear;
    int size;
} Queue;

Queue* make_queue(int size)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));

    queue->queue = (Node **)malloc(size * sizeof(Node *));

    queue->front = queue->rear = -1;
    queue->size = size;

    return queue;
}

int full(Queue *q)
{
    return q->front == (q->rear + 1) % q->size;
}

int empty(Queue *q)
{
    return q->front == -1;
}

int one_node_left(Queue *q)
{
    return !empty(q) && q->front == q->rear;
}

int peek(Queue *q)
{
    return (q->queue)[q->front]->freq;
}

void enqueue(Queue *q,Node *node)
{
    if(full(q))
        return;

    if(empty(q))
        q->front = 0;
    q->rear = (q->rear + 1) % q->size;
    q->queue[q->rear] = node;
}

Node* dequeue(Queue *q)
{
    if(empty(q))
        return NULL;

    Node *x = q->queue[q->front];
    if(q->front == q->rear)
        q->front = q->rear = -1;
    else
        q->front = (q->front + 1) % q->size;
    return x;
}

/* ----------------------------------------- */
Node* extract_min(Queue *q1,Queue *q2)
{
    if(empty(q1))
        return dequeue(q2);
    if(empty(q2))
        return dequeue(q1);
    if(peek(q1) < peek(q2))
        return dequeue(q1);
    return dequeue(q2);
}

Queue* enqueue_leaves(u_char *uniq,int *freq,int size)
{
    Queue *queue = make_queue(size);
    Node *leaf;

    for(int i = 0;i < size;i++)
    {
        leaf = make_leaf_node(uniq[i],freq[i]);
        enqueue(queue,leaf);
    }

    return queue;
}

Node* make_tree(Queue *leaves)
{
    Queue *queue = make_queue(leaves->size);

    Node *left,*right,*internal_node;
    while((!empty(leaves)) || (!one_node_left(queue)))
    {
        left = extract_min(leaves,queue);
        right = extract_min(leaves,queue);

        internal_node = make_internal_node(left,right);
        enqueue(queue,internal_node);
    }
    return dequeue(queue);
}

int longest_path(Node *tree)
{
    if(is_leaf_node(tree))
        return 0;
    int left_path = 1 + longest_path(tree->left);
    int right_path = 1 + longest_path(tree->right);

    if(left_path > right_path)
        return left_path;
    return right_path;
}

void write_letter_to_file(FILE *fptr,u_char *x,int *bit)
{
    if(*bit == 0)
    {
        fwrite(x,1,1,fptr);
        *bit = 128; *x = 0;
    }
}

void encode_letter(FILE *fptr,u_char letter,u_char *x,int *bit)
{
    int a,char_bit = 128;

    for(a = 0;a < 8;a++)
    {
        if(letter & char_bit)
            *x += *bit;
        char_bit /= 2;
        *bit /= 2;
        write_letter_to_file(fptr,x,bit);
    }
}

void encode_tree(Node *tree,FILE *output)
{
    static u_char output_char = 0;
    static int bit = 128,depth = 0;

    if(is_leaf_node(tree))
    {
        output_char += bit; /* add a 1 to the binary output */
        bit /= 2;

        write_letter_to_file(output,&output_char,&bit);
        /* add the ASCII of the letter */
        encode_letter(output,tree->letter,&output_char,&bit);
        return;
    }
    bit /= 2; /* add a 0 to the binary output */
    write_letter_to_file(output,&output_char,&bit);

    depth++;
    encode_tree(tree->left,output);
    encode_tree(tree->right,output);
    depth--;

    /* write the last character to the file
    if it was not a complete byte */
    if(depth == 0 && bit < 128)
        fwrite(&output_char,1,1,output);
}

void encode_text(char *file_name,FILE *output,u_char *uniq,int **paths,int size)
{ /* to be written */
    FILE *input = fopen(file_name,"rb");
    u_char input_char,output_char = 0;
    int bit = 128;

    int i,j;
    while(fread(&input_char,1,1,input))
    {
        for(i = 0;i < size;i++)
            /* search for the current letter's index */
            if(uniq[i] == input_char)
                break;
        j = 0;
        /* the index of a letter in uniq is the same
            as that letter's path in paths. */
        while(paths[i][j] >= 0)
        {
            if(paths[i][j])
                output_char += bit;
            bit /= 2;
            write_letter_to_file(output,&output_char,&bit);
            j++;
        }
    }
    /* write incomplete byte, similar to encode_tree */
    if(bit < 128)
        fwrite(&output_char,1,1,output);
    fclose(input);
}

int find_path(Node *tree,u_char letter,int *path,int *index)
{
    if(is_leaf_node(tree))
        return tree->letter == letter;

    path[(*index)++] = 0;
    if(find_path(tree->left,letter,path,index))
       return 1;
    (*index)--;


    path[(*index)++] = 1;
    if(find_path(tree->right,letter,path,index))
        return 1;
    (*index)--;

    return 0;
}

void find_all_paths(int **paths,Node *tree,u_char *uniq,int size,int longest_path)
{
    /* paths is an array of arrays for storing all the paths */
    int i,index;
    for(i = 0;i < size;i++)
    {
        index = 0;
        find_path(tree,uniq[i],paths[i],&index);
        /* -1 is as a null is in a string */
        paths[i][index] = -1;
    }
}

void main()
{
    char file_name[100];
    printf("Enter the name of the file you want to compress:\n");
    gets(file_name);

    int *counts = count_frequency(file_name);
    if(counts)
    {
        int size = counts[256];

        u_char *uniq = (u_char *)malloc(size * sizeof(u_char));
        int *freq = (int *)malloc(size * sizeof(int));

        size = 0;
        int i;
        long long file_size = 0;
        for(i = 0;i < 256;i++)
        {
            if(counts[i])
            {
                uniq[size] = i; freq[size] = counts[i];
                size++; file_size += counts[i];
            }
        }
        free(counts);

        sort(uniq,freq,size);

        Queue *leaves = enqueue_leaves(uniq,freq,size);

        Node *tree = make_tree(leaves);
        int depth = longest_path(tree);
        free(leaves);

        int **paths = (int **)malloc(size * sizeof(int *));
        for(i = 0;i < size;i++)
            paths[i] = (int *)malloc(depth * sizeof(int));

        find_all_paths(paths,tree,uniq,size,depth);

        long long encoded_size = 0;
        int path_length;
        for(i = 0;i < size;i++)
        {
            path_length = 0;
            while(paths[i][path_length] >= 0)
                path_length++;
            encoded_size += (freq[i] * path_length);
        }
        free(freq);

        float ratio = (encoded_size * 12.5)/(file_size);

        printf("Compression ratio = %.2f %%\n",ratio);
        printf("Do you wish to proceed? y/n ");
        char c; scanf("%c",&c);

        if(c == 'y')
        {
            u_char padding = encoded_size % 8;

            int len = strlen(file_name);
            char outfile[len + 4];
            strcpy(outfile,file_name); strcpy(&outfile[len],".cmp");

            FILE *fptr = fopen(outfile,"wb");
            encode_tree(tree,fptr);
            fwrite(&padding,1,1,fptr);

            encode_text(file_name,fptr,uniq,paths,size);
            fclose(fptr);
        }
        free(uniq);

        for(i = 0;i < size;i++)
            free(paths[i]);
        free(paths);
    }
    else
        printf("Unable to open file or file does not exist.\n");
}


