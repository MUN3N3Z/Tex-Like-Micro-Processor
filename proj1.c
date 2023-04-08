#include "proj1.h"

#include "string.h"
#include "strings.h"
#include "stdio.h"

/**
 * Receives input stream and returns the same without comments
*/
string_t* remove_comments(FILE* stream){
    // Enumerate states for the state machine
    typedef enum state_t {
        STATE_PLAIN_TEXT,
        STATE_REMOVE_COMMENT,
        STATE_NEWLINE,
    }state;
    // Declare struct to store output
    string_t* output = create_string();
    // State machine input
    int c;
    state current_state = STATE_PLAIN_TEXT;
    // Track new size of string after removing comments
    size_t new_size = 0;
    // Parse input
    while((c = fgetc(stream)) != EOF)
    {
        switch(current_state)
        {
        case STATE_PLAIN_TEXT:
            // Found a comment
            if(c == '%')
            {
                current_state = STATE_REMOVE_COMMENT;
            }
            else
            {
                output->put_char(output, c);
                new_size ++;
            }
            break;    

        case STATE_REMOVE_COMMENT:
            // Found a new line 
            if(c == '\n')
            {
                current_state = STATE_NEWLINE;
            }
            break;

        case STATE_NEWLINE:
            // Found first non-blank line
            if (peekchar() == '\n')
            {
                ungetc(c, stdin);
                current_state = STATE_PLAIN_TEXT;
            }
            else if((isspace(c) == 0))
            {
                ungetc(c, stdin);
                current_state = STATE_PLAIN_TEXT;
            }
            break;
        }
    }

    // Update string size
    output->size = new_size;

    return output;
}

/**
 * Forms a string from a REVERSED character linked list
*/
char* get_string_normal(string_t* buffer){
    // Allocate a string to hold the buffer
    char* string_buffer = malloc(sizeof(char) * (buffer->size + 1));
    char_t* temp = buffer->char_list; //Pointer
    size_t i = 0; // Counter
    while (temp != NULL)
    {
        string_buffer[i] = temp->character;
        // Move pointer
        temp = temp->next;
        i ++;
    }
    // End the string
    string_buffer[i] = '\0';

    return string_buffer;
    
}
/**
 * Return a copy of a string_t struct
*/
string_t* string_copy(string_t* src){
    // New copy
    string_t* copy = create_string();
    char_t* temp = src->char_list;
    char curr_char;
    while (temp != NULL)
    {
        curr_char = temp->character;
        copy->put_char(copy, curr_char);
        temp = temp->next;
    }
    return copy;
}
/**
 * DEF built-in function 
*/
char_t* DEF(char_t* character, vector* macro_vector){
    // Brace counter
    int brace_counter = 0;
    // Pointer 
    char_t* temp = character->next;
    // Initialize the new macro
    macro* new_macro = create_macro();
    // Get "NAME" argument
    string_t* name = create_string();
    while(temp->character != '}')
    {
        char curr_char = temp->character;
        // Ensure "NAME" arg is alphanumeric
        if(isalnum(curr_char) == 0)
        {
            WARN("\\def -> Non-alphanumeric character detected in ""NAME"" argument%s", " ");
            name->free(name);
            free(new_macro); // Macro is empty
            return NULL;

        }
        name->put_char(name, curr_char);
        // Move pointer
        temp = temp->next;
    }
    // Get string form of "NAME" argument
    char* name_string = get_string_normal(name);
    // Prevent user from redifining macro
    if (macro_vector->search(macro_vector, name_string) != NULL)
    {
        WARN("Cannot redifine macro%s", " ");
        // Clean up
        free(new_macro); // Macro is empty
        free(name_string);
        name->free(name);
        return NULL;
    }
    
    // Get "VALUE" argument
    // Ensure "NAME" is immediately followed by "VALUE"
    if(temp->next->character != '{')
    {
        WARN("usage -- def; expected '{'%s", " ");
        name->free(name);
        free(name_string);
        free(new_macro); // Macro is empty
        return NULL;
    }
    else
    {
        // Move pointer to opening curly brace
        temp = temp->next->next;
        // Update brace counter
        brace_counter ++;
    }
    // "VALUE" buffer
    string_t* value = create_string();
    while (brace_counter != 0)
    {
        // Return error if braces aren't balanced
        if (temp == NULL)
        {
            WARN("usage -- braces not balanced%s", " ");
            free(new_macro); // Macro is empty
            free(name_string);
            name->free(name);
            value->free(value);

        }
        char curr_char2 = temp->character;
        // Ensure curly braces are balanced
        if(curr_char2 == '{')
        {
            brace_counter ++;
        }
        if(curr_char2 == '}')
        {
            brace_counter --;
        }
        if (brace_counter != 0)
        {
            value->put_char(value, curr_char2);
        }
        // Check if hash is present
        if(curr_char2 == '#')
        {
            new_macro->hash_present = true;
        }
        
        // Move pointer
        temp = temp->next;
    }
    
    // Assign macro parameters
    new_macro->insert_key(new_macro, name_string);
    new_macro->insert_value(new_macro, value);
    // Insert macro into vector
    macro_vector->push(macro_vector, new_macro);
    // Clean up
    name->free(name);

    return temp->prev;
}
/**
 * UNDEF - built-in function
*/
char_t* UNDEF(char_t* character, vector* macro_vector){
    // Pointer
    char_t* temp = character;
    temp = temp->next; // Move temp to next character from opening brace
    string_t* buffer = create_string();
    while(temp->character != '}')
    {
        // Get current character
        char curr_char = temp->character;
        buffer->put_char(buffer, curr_char);
        temp = temp->next;
    }
    // Get string version of buffer
    char* buffer_string = get_string_normal(buffer);
    // No longer need buffer (linked list)
    buffer->free(buffer);
    // Delete macro with given NAME
    macro* macro_pointer = macro_vector->search(macro_vector, buffer_string);
    macro_vector->delete(macro_vector, macro_pointer);

    // Clean up
    free(buffer_string);
    return temp->next; // Next character after closing braces
}
/**
 * Expand user-defined macro
*/
char_t* USER_MACRO(char_t* cursor, vector* macro_vector, macro* mac){
    string_t* buffer = create_string();
    // Get argument after macro instance
    char_t* temp = cursor;
    temp = temp->next; // Move temp cursor one character away from opening brace (to start of argument)
    // Brace counter (Account for prev opening brace)
    int  brace_counter = 1;
    while (brace_counter != 0)
    {
        if (temp == NULL)
        {
            WARN("usage -- braces not balanced%s", " ");
            buffer->free(buffer);
            return NULL;
        }
        char curr_char = temp->character;
        if(curr_char == '{')
        {
            brace_counter ++;
        }
        if(curr_char == '}')
        {
            brace_counter --;
        }
        if (brace_counter != 0)
        {
            buffer->put_char(buffer, curr_char);
        }
        // Advance cursor
        temp = temp->next;
    }
    // Make a copy of macro value - to be inserted into output
    string_t* macro_value_copy = string_copy(mac->value);
    // Handle all possible cases
    // Find pound sign if present
    char_t* temp_pound = macro_value_copy->char_list;
    if (mac->hash_present)
    {
        while (temp_pound->character != '#') // Stop at pound sign
        {
            temp_pound = temp_pound->next;
        }
    }
    // Empty argument and no pound sign is absent in macro value
    if ((buffer->size == 0) && (!mac->hash_present))
    {
        macro_value_copy->char_list_tail->next = temp; // Connect VALUE  to the rest of the input stream
    }
    // Empty argument and pound sign is present in macro value
    else if ((buffer->size == 0) && (mac->hash_present))
    {
        // If pound sign is in the middle of the argument
        if (temp_pound->next != NULL)
        {
            // Connect previous character to next one, skipping the pound sign
            temp_pound->prev->next = temp_pound->next;
            temp_pound->next->prev = temp_pound->prev;
            macro_value_copy->char_list_tail->next = temp; // Connect VALUE  to the rest of the input stream
        }
        else
        {
            // Make prev character before pound sign to be last character in the string
            macro_value_copy->char_list_tail = temp_pound->prev;
            macro_value_copy->char_list_tail->next = temp; // Connect VALUE  to the rest of the input stream
        }
    }
    // Argument and pound sign both present
    else if ((buffer->size != 0) && (mac->hash_present))
    {
        // Handle case where only pound sign is given as argument
        if (temp_pound->next == NULL && temp_pound->prev == NULL)
        {
            // Only value in buffer is needed
            buffer->char_list_tail->next = temp;
            return buffer->char_list;
        }
        
        // Connect previous character to buffer string (arg)
        temp_pound->prev->next = buffer->char_list;
        buffer->char_list->prev = temp_pound->prev;
        buffer->char_list_tail->next = temp; // Connect VALUE  to the rest of the input stream
        return macro_value_copy->char_list; // Don't free buffer since it's part of output
    }
    else
    {
        macro_value_copy->char_list_tail->next = temp;
    }
    
    // Clean up
    buffer->free(buffer);

    return macro_value_copy->char_list;
}
/**
 * IF - built-in function
*/
char_t* USER_IF(char_t* cursor){
    char_t* temp = cursor;
    char_t* start_of_result;
    // Get "COND" 
    int brace_counter = 0;
    if (temp->character != '{')
    {
        WARN("Usage -- expected '{' in ""IF"" block%s", " ");
        return NULL;
    }
    else
    {
        // Account for opening brace
        brace_counter ++;
        // Move to next character
        temp = temp->next;
    }
    string_t* cond_buffer = create_string(); // "COND" buffer
    while (brace_counter != 0)
    {
        if (temp == NULL)
        {
            WARN("usage -- braces not balanced%s", " ");
            cond_buffer->free(cond_buffer);
            return NULL;
        }
        char curr_char = temp->character;
        if (curr_char == '{')
        {
            brace_counter ++;
        }
        if (curr_char == '}')
        {
            brace_counter --;
        }
        if (brace_counter != 0)
        {
            cond_buffer->put_char(cond_buffer, curr_char);
        }
        temp = temp->next; // Move cursor
    }
    // Ensure next character is opening brace of "THEN" block
    brace_counter = 0; // Reset brace counter
    if (temp->character != '{')
    {
        WARN("Usage -- expected '{' in ""IF"" block%s", " ");
        cond_buffer->free(cond_buffer); // Clean up
        return NULL;
    }
    else
    {
        // Account for opening brace
        brace_counter ++;
        // Move to next character
        temp = temp->next;
    }
    
    // Decide which condition to fulfill
    if (cond_buffer->size != 0)
    {
        // Expand "THEN"
        // Set cursor->next to point to beginning of replacement string
        start_of_result = temp;
        // Connect end of replacement string to character after "ELSE" string
        char_t* end_of_then;
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                cond_buffer->free(cond_buffer);
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            end_of_then = temp->prev;
            temp = temp->next;
        }
        // Get end of "ELSE" block
        brace_counter = 0; // Reset brace counter
        if (temp->character != '{')
        {
            WARN("Usage -- expected '{' in ""IF"" block%s", " ");
            cond_buffer->free(cond_buffer); // Clean up
            return NULL;
        }
        else
        {
            // Account for opening brace
            brace_counter ++;
            // Move to next character
            temp = temp->next;
        }
        // Ignore "ELSE" block
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                cond_buffer->free(cond_buffer);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            temp = temp->next;
        }
        // Connect end of "THEN" string to character after "ELSE" block
        end_of_then->next = temp;
        cond_buffer->free(cond_buffer);
        return start_of_result;
    }
    else
    {
        // EXPAND "ELSE" block
        // Connect end of replacement string to character after "ELSE" string
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                cond_buffer->free(cond_buffer);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            temp = temp->next;
        }
        // Ensure character is an opening brace
        brace_counter = 0;// Reset counter
        if (temp->character != '{')
        {
            WARN("Usage -- expected '{' in ""IF"" block%s", " ");
            cond_buffer->free(cond_buffer); // Clean up
            return NULL;
        }
        else
        {
            // Account for opening brace
            brace_counter ++;
            // Move to next character
            temp = temp->next;
        }
        // Connect cursor to start of "ELSE" string
        start_of_result = temp;
        // Brace balancing for the ELSE block
        char_t* temp2 = NULL;
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                cond_buffer->free(cond_buffer);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            temp2 = temp->prev;
            temp = temp->next;
        }
        // Connect character before closing brace to character after closing brace
        temp2->next = temp;
        cond_buffer->free(cond_buffer);
        return start_of_result;
    }
}
/**
 * IFDEF - built-in function
*/
char_t* USER_IF_DEF(char_t* temp, vector* macro_vector){
    // Brace balancing for the first argument
    int brace_counter = 1; // Account for current opening brace
    char_t* start_of_result;
    temp = temp->next;
    string_t* macro_name_buffer = create_string();
    while (brace_counter != 0)
    {
        if (temp == NULL)
        {
            WARN("usage -- braces not balanced%s", " ");
            macro_name_buffer->free(macro_name_buffer);
            return NULL;
        }
        char curr_char = temp->character;
        if (curr_char == '{')
        {
            brace_counter ++;
        }
        if (curr_char == '}')
        {
            brace_counter --;
        }
        if (brace_counter != 0)
        {
            if (isalnum(curr_char) == 1)
            {
                WARN("Usage -- \\Macro name is not alphanumeric%s", " ");
                macro_name_buffer->free(macro_name_buffer);
                return NULL;
            }
            macro_name_buffer->put_char(macro_name_buffer, curr_char);
        }
        temp = temp->next; // Move cursor
    }
    brace_counter = 0; // Reset brace count
    // Ensure opening brace for "THEN" is present
    if (temp->character != '{')
    {
        WARN("Usage -- ""IF_DEF"" Expected '{'%s", " ");
        macro_name_buffer->free(macro_name_buffer);
        return NULL;
    }
    else
    {
        // Account for opening brace
        brace_counter ++;
        temp = temp->next;
    }
    
    // Decide how to expand
    char* macro_name_string = get_string_normal(macro_name_buffer);
    if (macro_vector->search(macro_vector, macro_name_string) != NULL)
    {
        // Expand "THEN"
        // Set cursor->next to point to beginning of replacement string
        start_of_result = temp;
        // Connect end of replacement string to character after "ELSE" string
        char_t* end_of_then;
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                macro_name_buffer->free(macro_name_buffer);
                free(macro_name_string);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            end_of_then = temp->prev;
            temp = temp->next;
        }
        // Get end of "ELSE" block
        brace_counter = 0; // Reset brace counter
        if (temp->character != '{')
        {
            WARN("Usage -- expected '{' in ""IF"" block%s", " ");
            macro_name_buffer->free(macro_name_buffer); // Clean up
            return NULL;
        }
        else
        {
            // Account for opening brace
            brace_counter ++;
            // Move to next character
            temp = temp->next;
        }
        // Ignore "ELSE" block
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                macro_name_buffer->free(macro_name_buffer);
                free(macro_name_string);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            temp = temp->next;
        }
        // Connect end of "THEN" string to character after "ELSE" block
        end_of_then->next = temp;
        macro_name_buffer->free(macro_name_buffer);
        return start_of_result;
    }
    else
    {
        // EXPAND "ELSE" block
        // Connect end of replacement string to character after "ELSE" string
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                macro_name_buffer->free(macro_name_buffer);
                free(macro_name_string);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            temp = temp->next;
        }
        // Ensure character is an opening brace
        brace_counter = 0;// Reset counter
        if (temp->character != '{')
        {
            WARN("Usage -- expected '{' in ""IF"" block%s", " ");
            macro_name_buffer->free(macro_name_buffer); // Clean up
            return NULL;
        }
        else
        {
            // Account for opening brace
            brace_counter ++;
            // Move to next character
            temp = temp->next;
        }
        // Connect cursor to start of "ELSE" string
        start_of_result = temp;
        // Brace balancing for the ELSE block
        char_t* temp2 = NULL;
        while (brace_counter != 0)
        {
            if (temp == NULL)
            {
                WARN("usage -- braces not balanced%s", " ");
                macro_name_buffer->free(macro_name_buffer);
                free(macro_name_string);
                return NULL;
            }
            char curr_char = temp->character;
            if (curr_char == '{')
            {
                brace_counter ++;
            }
            if (curr_char == '}')
            {
                brace_counter --;
            }
            
            temp2 = temp->prev;
            temp = temp->next;
        }
        // Connect character before closing brace to character after closing brace
        temp2->next = temp;
        macro_name_buffer->free(macro_name_buffer);
        return start_of_result;
    }
    
}

char_t* INCLUDE(char_t* temp){
    string_t* buffer = create_string();
    // Brace balancing
    int brace_counter = 1; //Account for opening brace
    temp = temp->next;
    while (brace_counter != 0)
    {
        if (temp == NULL)
        {
            WARN("[INCLUDE]usage -- braces not balanced%s", " ");
            buffer->free(buffer);
            return NULL;
        }
        char curr_char = temp->character;
        if (curr_char == '{')
        {
            brace_counter ++;
        }
        if (curr_char == '}')
        {
            brace_counter --;
        }
        if (brace_counter != 0)
        {
            buffer->put_char(buffer, curr_char);
        }
        temp = temp->next;
    }
    // No file provided
    if (buffer->size == 0)
    {
        WARN("[INCLUDE]No file provided%s", " ");
        buffer->free(buffer);
        return NULL;
    }
    
    char* file_name = get_string_normal(buffer);
    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        WARN("Could not open \\INCLUDE file%s", " ");
        // Clean up
        free(file_name);
        buffer->free(buffer);
        return NULL;
    }
    string_t* no_comments = remove_comments(file);
    // Connect file contents to current stream
    no_comments->char_list_tail->next = temp;
    return no_comments->char_list;
}
/**
 * -------------------------------------------------------------STATE FUNCTIONS------------------------------------------------------------------------------------------------------------
*/



/**
 * Macro function - processes and stores macros then expands them on sight
*/

string_t* read_expand_macros(string_t* input, vector* macro_vector){
    // Enumerate states for the state machine
    typedef enum state_t {
        STATE_PLAIN_TEXT,
        STATE_MACRO,
    }state;
    // Output buffer
    string_t* output_buffer = create_string();
    char_t* temp = input->char_list; // Pointer
    // Initial state of state machine
    state current_state = STATE_PLAIN_TEXT;
    
    while (temp != NULL)
    {
        // Current character
        char c = temp->character;
        switch (current_state)
        {
        case STATE_PLAIN_TEXT:
            if (c == '\\')
            {
                // Escape special characters
                if (is_special(temp->next->character))
                {
                    // Push special character to output
                    output_buffer->put_char(output_buffer, temp->next->character);
                    // Move cursor to next character
                    temp = temp->next;
                }
                else if (isalnum(temp->next->character))
                {
                    // Argument is probably a macro
                    current_state = STATE_MACRO;
                }   
            }
            else
            {
                output_buffer->put_char(output_buffer, c);
            }
            break;

        case STATE_MACRO:
            if (c == '{')
            {
                // Decide what type of input we're dealing with to determine next state
                char* backslash_arg = get_arg(temp->prev, '\\');
                // Handle backslah options
                macro* mac;
                if((mac = macro_vector->search(macro_vector, backslash_arg)) != NULL)
                {
                    temp->next = USER_MACRO(temp, macro_vector, mac);
                }
                // DEF
                else if(strcmp(backslash_arg, "def") == 0)
                {
                    temp = DEF(temp, macro_vector);
                    // Handle internal DEF errors
                    if (temp == NULL)
                    {
                        // Clean up
                        free(backslash_arg);
                        return NULL;
                    }
                }
                // UNDEF
                else if (strcmp(backslash_arg, "undef") == 0)
                {
                    temp = UNDEF(temp, macro_vector);
                    if (temp == NULL)
                    {
                        // Clean up
                        free(backslash_arg);
                        return NULL;
                    }
                }
                else if (strcmp(backslash_arg, "if") == 0)
                {
                    temp->next = USER_IF(temp);
                    if (temp->next == NULL)
                    {
                        // Clean up
                        free(backslash_arg);
                        return NULL;
                    }
                }
                else if (strcmp(backslash_arg, "ifdef") == 0)
                {
                    temp->next = USER_IF_DEF(temp, macro_vector);
                    if (temp->next == NULL)
                    {
                        // Clean up
                        free(backslash_arg);
                        return NULL;
                    }
                }
                else if (strcmp(backslash_arg, "include") == 0)
                {
                    temp->next = INCLUDE(temp);
                    if(temp->next == NULL)
                    {
                        free(backslash_arg);
                        return NULL;
                    }
                }
                
                current_state = STATE_PLAIN_TEXT;
                // Free backslash_arg
                free(backslash_arg);
            }
            break;
        }
        // Move pointer
        temp = temp->next;
    }
    // Raise error if last state is Macro state
    if (current_state == STATE_MACRO)
    {
        WARN("Brace not balanced%s", " ");
        return NULL;
    }
    
    return output_buffer;
}

int main(int argc, char const *argv[])
{
    // Exit program is too few arguments are provided 
    // CURRENTLY HANDLES ONE FILE
    FILE* stream;
    if (argc < 2) 
    {
        stream = stdin;
    }
    else
    {
        stream = fopen(argv[1], "r");
    }
    
    string_t* output = remove_comments(stream);
    if (output != NULL)
    {
        if (output->char_list == NULL)
        {
            free(output);
            return 0;
        }   
    }
    
    // Create a vector to store macros
    vector* macro_vector = create_vector();
    string_t* new_output = NULL;
    new_output = read_expand_macros(output, macro_vector);

    if (new_output != NULL)
    {
        if (new_output->char_list != NULL)
        {
            clean_print(new_output);
        }
    }

    output->free(output);
    new_output->free(new_output);
    macro_vector->free(macro_vector);
    return 0;
}
