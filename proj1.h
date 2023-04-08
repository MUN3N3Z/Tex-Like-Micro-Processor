// proj1.h                                          Stan Eisenstat (09/17/15)
//
// System header files and macros for proj1

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Write message to stderr using format FORMAT
#define WARN(format,...) fprintf (stderr, "proj1: " format "\n", __VA_ARGS__)

// Write message to stderr using format FORMAT and exit.
#define DIE(format,...)  WARN(format,__VA_ARGS__), exit (EXIT_FAILURE)

// Double the size of an allocated block PTR with NMEMB members and update
// NMEMB accordingly.  (NMEMB is only the size in bytes if PTR is a char *.)
#define DOUBLE(ptr,nmemb) realloc (ptr, (nmemb *= 2) * sizeof(*ptr))

#define LIST_CAPACITY 200

#define BUILT_IN_MACROS ["def", "undef", "ifdef", "if", "include", "expandafter"]

// Character struct node
typedef struct char_t
{
    char character;
    struct char_t* prev;
    struct char_t* next;
}char_t;

// String struct - character linked list
typedef struct string_t
{
    // Linked list of characters
    char_t* char_list; // Head
    char_t* char_list_tail; // Tail
    // Size of the string
    size_t size;
    // Function pointers
    char_t* (*put_char) (struct string_t* self, char s_char);
    void (*free) (struct string_t* self);
    void (*free_reversed) (struct string_t* self);

}string_t;

// Key:Value struct for the macros
typedef struct macro
{
    // Macro name
    char* key;
    // Macro value
    string_t* value;
    // Macro name size and capacity
    size_t key_size;
    // Flag for "#" in "VALUE"
    bool hash_present;
    // Next macro
    struct macro* next;
    // Prev macro
    struct macro* prev;
    
    // Function pointers
    void (*free) (struct macro* self);
    void (*insert_key)(struct macro* self, char* key);
    void (*insert_value)(struct macro* self, string_t* value);
}macro;

// Vector struct declaration
typedef struct vector_item
{   
    // Head of linked list
    macro* list;
    // Tail of linked list
    macro* list_tail;
    // Number of items in the list
    size_t size;
} vector_item;

typedef struct vector
{
    vector_item vectorItem;
    // Function pointers
    size_t (*size) (struct vector* self);
    void (*delete) (struct vector* self, macro* item);
    void (*push) (struct vector* self, macro* item);
    void (*free) (struct vector* self);
    macro* (*search) (struct vector* self, char* key);
    

}vector;

// Memory management array that stores pointers to char_t*
typedef struct mem_manager
{
    char_t** char_t_pointers;
    size_t size;
    size_t capacity;
    // Function pointers
    struct mem_manager* (*resize) (struct mem_manager* self);
    void (*point) (struct mem_manager* self, char_t* character);
    void (*destroy) (struct mem_manager* self);
}mem_manager;

 
// Implementation of "string_t" functions


/**
 * Adds a new character at the end of the string linked list
*/
char_t* string_put_char(string_t* self, char new_char){
    // Create a node for the first character and assign it the new character
    char_t* new_character_node = malloc(sizeof(char_t));
    new_character_node->character = new_char;
    
    // Edge case for first node in linked list (prev = NULL)
    if (self->size == 0)
    {
        new_character_node->next = NULL;
        new_character_node->prev = NULL;
        // Insert into linked list
        self->char_list = new_character_node;
        self->char_list_tail = new_character_node;
        // Keep count
        self->size ++;
    }
    else
    {
        // Perform insertion
        self->char_list_tail->next = new_character_node;
        new_character_node->prev = self->char_list_tail;
        self->char_list_tail = new_character_node; // New last node
        new_character_node->next = NULL;
        // Keep count
        self->size ++;
    }

    return new_character_node;
}

/**
 * Free the string_t struct and it's constituent string
*/
void string_free(string_t* self){
    // Error return
    if (self == NULL)
    {
        return;
    }
    if (self->size == 1)
    {
        free(self->char_list);
        self->char_list = NULL;
        self->char_list_tail = NULL;
    }
    else if(self->size > 1)
    {
        char_t* temp1 = self->char_list;
        char_t* temp2 = self->char_list->next;
        // Free character nodes
        while (temp2 != NULL)
        {
            free(temp1);
            // Keep count
            self->size --;
            // Advance pointers
            temp1 = temp2;
            temp2 = temp2->next;
        }
        // Free last character node
        free(temp1);
        // Keep count
        self->size --;
    }
    // Free string_t struct
    free(self);
}

/**
 * Free the string_t struct and it's constituent REVERSED string
*/
void string_special_free(string_t* self){

    if (self == NULL)
    {
        return;
    }
    
    if (self->size == 1)
    {
        free(self->char_list);
    }
    else if(self->size > 1)
    {
        char_t* temp1 = self->char_list;
        char_t* temp2 = self->char_list->prev;
        // Free character nodes
        while (temp2 != NULL)
        {
            free(temp1);
            // Keep count
            self->size --;
            // Advance pointers
            temp1 = temp2;
            temp2 = temp2->prev;
        }
        // Free last character node
        free(temp1);
        // Keep count
        self->size --;
    }
    // Free string_t struct
    free(self);
}

/**
 * Initialize a new instance of a string_t struct
*/
string_t* create_string(){
    // Allocate space for the new string structure
    string_t* new_string = malloc(sizeof(string_t));
    // Update size
    new_string->size = 0;
    // Initialize values
    new_string->char_list = NULL;
    new_string->char_list_tail = NULL;
    // Set function pointers
    new_string->put_char = string_put_char;
    new_string->free = string_free;
    new_string->free_reversed = string_special_free;

    return new_string;
}


// Implementation of "Macro" functions


/**
 * Free a macro struct and it's constituent key and value
*/
void macro_free(macro* self){

    if (self->key != NULL)
    {
        // Free key(char*)
        free(self->key);
    }
    if (self->value != NULL)
    {
        // Free value(string_t*)
        self->value->free(self->value);
    }
    // Free the actual struct
    free(self);
}

/**
 * Assign a key string to a macro
*/
void macro_insert_key(macro* self, char* key){
    // Insert macro name
    self->key = key;
    self->key_size = strlen(key);
}

/**
 * Assign a value string to a macro
*/
void macro_insert_value(macro* self, string_t* value){
    // Insert macro value
    self->value = value;
}

/**
 * Create a macro struct instance
*/
macro* create_macro(){
    // Allocate space for the actual struct, key and its value
    macro* new_macro = malloc(sizeof(macro));
    new_macro->key = NULL;
    new_macro->value = NULL;
    new_macro->hash_present = false;
    new_macro->next = NULL;
    new_macro->prev = NULL;

    // Set initial sizes and capacities of the constituent key and value 
    new_macro->key_size = 0;
    // Assign function pointer
    new_macro->free = macro_free;
    new_macro->insert_key = macro_insert_key;
    new_macro->insert_value = macro_insert_value;

    return new_macro;
}


// Implementation of "Vector" functions


/**
 * Return size of the vector
*/
size_t vector_size(vector* self){

    return self->vectorItem.size;
}

/**
 * Return index of a macro in the passed vector
*/
macro* vector_search(vector* self, char* key){

    macro* temp = self->vectorItem.list;
    while (temp != NULL)
    {
        if (strcmp(temp->key, key) == 0)
        {
            return temp;
        }
        else
        {
            temp = temp->next;
        }
    }
    // Macro not found
    return temp;
}

/**
 * Insert a macro struct at the end of the vector
*/
void vector_push(vector* self, macro* item){
    // Handle adding into an empty vecor
    if(self->size(self) != 0)
    {
        // Connect new macro node to the end of the linked list
        item->prev = self->vectorItem.list_tail;
        // Set new node as new next node for previous last node
        self->vectorItem.list_tail->next = item; 
    }
    else
    {
        // Set new head of list
        self->vectorItem.list = item;
    }
    self->vectorItem.list_tail = item;
    // Account for new node
    self->vectorItem.size ++;
}

/**
 * Delete an item in the vector
*/
void vector_delete(vector* self, macro* item){
    // Only one node is present
    if (self->size(self) == 1)
    {
        item->free(item);
        self->vectorItem.list = NULL;
        self->vectorItem.list_tail = NULL;
    }
    else
    {
        // If node is last on list
        if (item->next == NULL)
        {
            // Disconnect node from vector
            item->prev->next = NULL;
            self->vectorItem.list_tail = item->prev;
            // Free node
            item->free(item);
        }
        // If node is first on list
        else if (item->prev == NULL)
        {
            // Disconnect node
            self->vectorItem.list = item->next;
            item->next->prev = NULL;
            // Free node
            item->free(item);
        }
        // Node is in the middle of list
        else
        {
            // Disconnect node from list
            item->prev->next = item->next;
            item->next->prev = item->prev;
            // Free node
            item->free(item);
        }   
    }
    // Account for deleted node
    self->vectorItem.size --;
}

/**
 * Free an entire vector
*/
void vector_free(vector* self){
    // Free empty vector
    if (self->size(self) == 0)
    {
        free(self);
        return;
    }
    // Free list of assigned macros
    macro* temp2 = self->vectorItem.list->next;
    macro* temp1 = self->vectorItem.list;
    while (temp2 != NULL)
    {
        temp1->free(temp1);
        // Advance pointer
        temp1 = temp2; 
        temp2 = temp2->next;
        // Book keeping
        self->vectorItem.size --;
    }
    // Free last macro node
    free(temp1);
    // Free vector struct
    free(self);
}

/**
 * Initialize a vector struct
*/
vector* create_vector(){
    // Allocate space for the vector struct
    vector* self = malloc(sizeof(vector));
    self->vectorItem.list = NULL;
    self->vectorItem.list_tail = NULL;
    // Update size
    self->vectorItem.size = 0;

    // Assign function pointers
    self->delete = vector_delete;
    self->free = vector_free;
    self->push = vector_push;
    self->size = vector_size;
    self->search = vector_search;

    return self;
}
/**
 * Check next character without consuming it
*/
int peekchar(void)
{
    int c;

    c = getchar();
    if(c != EOF) ungetc(c, stdin);      /* puts it back */
    
    return c;
}

/**
 * COPY TESTER FUNCTION
*/
void clean_copy(FILE* stream, string_t* output){
    // Parse input
    int c;
    while((c = fgetc(stream)) != EOF)
    {
        output->put_char(output, c);
    }
}

/**
 * PRINT TESTER FUNCTION
*/
void clean_print(string_t* input)
{
    if (input == NULL)
    {
        return;
    }
    char_t* temp = input->char_list;
    while (temp != NULL)
    {
        fprintf(stdout, "%c", temp->character);
        temp = temp->next;
    }
}
/**
 * Forms a string from a REVERSED character linked list
*/
char* get_string(string_t* buffer){
    // Allocate a string to hold the buffer
    char* string_buffer = malloc(sizeof(char) * (buffer->size + 1));
    char_t* temp = buffer->char_list; //Pointer
    size_t i = 0; // Counter
    while (temp != NULL)
    {
        string_buffer[i] = temp->character;
        // Move pointer
        temp = temp->prev;
        i ++;
    }
    // End the string
    string_buffer[i] = '\0';

    return string_buffer;
    
}
/**
 * Returns true if character is a special character
*/
bool is_special(char c){
    if (c == '\\')
    {
        return true;
    }
    else if (c == '#')
    {
        return true;
    }
    else if (c == '%')
    {
        return true;
    }
    else if (c == '{')
    {
        return true;
    }
    else if (c == '}')
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**
 * Get string between backslash and opening curly brace
*/
char* get_arg(char_t* character, char limit){
    char_t* temp = character; // Pointer
    // Buffer for collected agrument
    string_t* buffer = create_string();
    while (temp->character != limit)
    {
        char curr_char = temp->character;
        if (curr_char != '}')// Handle corner case for nested user-defined macros
        {
            buffer->put_char(buffer, curr_char);
        }
        // Move pointer
        temp = temp->prev;
    }
    // Reverse string by switching head and tail of char_t linked list
    temp = buffer->char_list; // Avoid losing the head
    buffer->char_list = buffer->char_list_tail;
    buffer->char_list_tail = temp; // list is reversed so "next" and "prev" are switched
    // Construct a string for the argument
    char* argument = get_string(buffer);
    // Clean up
    buffer->free_reversed(buffer);

    return argument;
}

// MEMMANAGER FUNCTIONS
/**
 * Resize the array of pointers
*/
mem_manager* memmanager_resize(mem_manager* self){
    self->char_t_pointers = DOUBLE(self->char_t_pointers, self->capacity);
    return self;
}
/**
 * Add a new pointer to the array
*/
void memmanager_point(mem_manager* self, char_t* character){
    // Resize array if capacity is reached
    if ((self->size) >= (self->capacity))
    {
        self = self->resize(self);
    }
    self->char_t_pointers[self->size] = character;
    // Account for new pointer
    self->size ++;
}
/**
 * Free a memory manager
*/
void memmanager_destroy(mem_manager* self){
    // Free char_t nodes pointed by the array of pointers
    for (size_t i = 0; i < self->size; i++)
    {
        free(self->char_t_pointers[i]);
    }
    // Free list of pointers
    free(self->char_t_pointers);
    // Free the memmanager struct
    free(self);
    
}
/**
 * Create an instance of a memmanager
*/
mem_manager* create_memmanager(){
    mem_manager* self = malloc(sizeof(mem_manager));
    self->char_t_pointers = malloc(sizeof(char_t*) * LIST_CAPACITY);
    self->capacity = LIST_CAPACITY;
    self->size = 0;
    // Function pointers
    self->resize = memmanager_resize;
    self->point = memmanager_point;
    self->destroy = memmanager_destroy;

    return self;
}