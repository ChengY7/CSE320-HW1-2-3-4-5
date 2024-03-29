#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"
#include "function.h"
int argo_read_object(ARGO_VALUE *v, FILE *f);
ARGO_CHAR argo_read_char(FILE *f);
ARGO_CHAR argo_read_char_forString(FILE *f);
int argo_read_jsonLine(ARGO_VALUE *sentinel, FILE *f);
int add_to_linkedList(ARGO_VALUE *sentinel, ARGO_VALUE *new_value);
int argo_read_array(ARGO_VALUE *v, FILE *f);
int argo_convert_hex_dec(FILE *f);
int argo_write_object(ARGO_VALUE *v, FILE *f);
int argo_write_array(ARGO_VALUE *v, FILE *f);

/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */

ARGO_VALUE *argo_read_value(FILE *f) {
    ARGO_CHAR cursor = argo_read_char(f);        //read the first character
    if(cursor==ARGO_LBRACE) {  //If json is an object, initalize it 
        ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
        argo_next_value++;
        current_argo_value->type=4;   
        current_argo_value->next=current_argo_value;
        current_argo_value->prev=current_argo_value;
        current_argo_value->name.content=NULL;
        if(argo_read_object(argo_value_storage, f)==0) {   //and call argo_read_object
            cursor=argo_read_char(f);
            if(cursor==EOF)
                return current_argo_value; 
            else 
                return NULL;
        }                       //If successful then continue
        else 
            return NULL;                                //Else return null
    }
    else if (cursor==ARGO_LBRACK) {
        ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
        argo_next_value++;
        current_argo_value->type=5;
        current_argo_value->next=current_argo_value;
        current_argo_value->prev=current_argo_value;
        current_argo_value->name.content=NULL;
        if(argo_read_array(argo_value_storage, f)==0) {
            cursor=argo_read_char(f);
            if(cursor==EOF) {
                return current_argo_value;
            }
            else    
                return NULL;
        }
        else
            return NULL;
    }
    else if(argo_is_digit(cursor) || cursor==ARGO_MINUS) {
        ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
        argo_next_value++;
        current_argo_value->type=2;
        current_argo_value->next=current_argo_value;
        current_argo_value->prev=current_argo_value;
        current_argo_value->name.content=NULL;
        ungetc(cursor, f);
        if(argo_read_number(&(current_argo_value->content.number), f)==0) {
            cursor=argo_read_char(f);
            if(cursor==EOF) {
                return current_argo_value;
            }
            else    
                return NULL;
        }
        else    
            return NULL;
    }  
    else if(cursor==ARGO_QUOTE) {
        ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
        argo_next_value++;
        current_argo_value->type=3;
        current_argo_value->next=current_argo_value;
        current_argo_value->prev=current_argo_value;
        current_argo_value->name.content=NULL;
        if(argo_read_string(&(current_argo_value->content.string),f)==0) {
            cursor=argo_read_char(f);
            if(cursor==EOF) {
                return current_argo_value;
            }
            else    
                return NULL;
        }
        else    
       
        return NULL;
    }
    else if(cursor==*(ARGO_TRUE_TOKEN)) {
        cursor=argo_read_char(f);
        if(cursor==*(ARGO_TRUE_TOKEN+1)) {
            cursor=argo_read_char(f);
            if(cursor==*(ARGO_TRUE_TOKEN+2)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_TRUE_TOKEN+3)) {
                    ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
                    argo_next_value++;
                    current_argo_value->type=1;
                    current_argo_value->next=current_argo_value;
                    current_argo_value->prev=current_argo_value;
                    current_argo_value->name.content=NULL;
                    current_argo_value->content.basic=ARGO_TRUE;
                    cursor=argo_read_char(f);
                    if(cursor==EOF) {
                        return current_argo_value;
                    }
                    else {   
                        fprintf(stderr, "[Line: %d] Nothing should be after true\n", argo_lines_read+1); 
                        return NULL;
                    }
                }
            }
        }
        fprintf(stderr, "[Line: %d] Expecting true\n", argo_lines_read+1);
        return NULL;
    }
    else if (cursor==*(ARGO_FALSE_TOKEN)) {
        cursor=argo_read_char(f);
        if(cursor==*(ARGO_FALSE_TOKEN+1)) {
            cursor=argo_read_char(f);
            if(cursor==*(ARGO_FALSE_TOKEN+2)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_FALSE_TOKEN+3)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_FALSE_TOKEN+4)) {
                        ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
                        argo_next_value++;
                        current_argo_value->type=1;
                        current_argo_value->next=current_argo_value;
                        current_argo_value->prev=current_argo_value;
                        current_argo_value->name.content=NULL;
                        current_argo_value->content.basic=ARGO_FALSE;
                        cursor=argo_read_char(f);
                        if(cursor==EOF) {
                            return current_argo_value;
                        }
                        else {   
                            fprintf(stderr, "[Line: %d] Nothing should be after false\n", argo_lines_read+1); 
                            return NULL;
                        }
                    }
                }
            }
        }
        fprintf(stderr, "[Line: %d] Expecting false\n", argo_lines_read+1);
        return NULL;
    }
    else if (cursor==*(ARGO_NULL_TOKEN)) {
        cursor=argo_read_char(f);
        if(cursor==*(ARGO_NULL_TOKEN+1)) {
            cursor=argo_read_char(f);
            if(cursor==*(ARGO_NULL_TOKEN+2)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_NULL_TOKEN+3)) {
                    ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
                    argo_next_value++;
                    current_argo_value->type=1;
                    current_argo_value->next=current_argo_value;
                    current_argo_value->prev=current_argo_value;
                    current_argo_value->name.content=NULL;
                    current_argo_value->content.basic=ARGO_NULL;
                    cursor=argo_read_char(f);
                    if(cursor==EOF) {
                        return current_argo_value;
                    }
                    else{    
                        fprintf(stderr, "[Line: %d] Nothing should be after null\n", argo_lines_read+1);
                        return NULL;
                    }
                }
            }
        }
        fprintf(stderr, "[Line: %d] Expecting null\n", argo_lines_read+1);
        return NULL;
    }
    fprintf(stderr, "[Line: %d] Illegal start to json file\n", argo_lines_read+1);                                        
    return NULL;

}


int argo_read_array(ARGO_VALUE *v, FILE *f) {
    ARGO_VALUE *sentinel = (argo_value_storage+argo_next_value);
    argo_next_value++;
    v->content.array.element_list=sentinel;
    sentinel->next=sentinel;
    sentinel->prev=sentinel;
    sentinel->name.content=NULL;
    sentinel->type=0;
    ARGO_CHAR cursor = argo_read_char(f);
    while(cursor!=EOF) {
        if(cursor==ARGO_RBRACK) {
            return 0;
        }
        else if(cursor==ARGO_QUOTE) {
            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
            argo_next_value++;
            add_to_linkedList(sentinel, new_value);
            if(argo_read_string(&(new_value->content.string), f)==0) {
                new_value->type=3;
                cursor=argo_read_char(f);
                if(cursor==ARGO_COMMA) {
                    cursor=argo_read_char(f);
                    continue;
                }
                else if (cursor==ARGO_RBRACK) {
                    return 0;
                }
                else{
                    fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);
                    return -1;
                }
            }
            return -1;
        }
        else if(argo_is_digit(cursor) || cursor==ARGO_MINUS) {
            ungetc(cursor, f);
            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
            argo_next_value++;
            add_to_linkedList(sentinel, new_value);
            if(argo_read_number(&(new_value->content.number), f)==0) {
                new_value->type=2;
                cursor=argo_read_char(f);
                if(cursor==ARGO_COMMA) {
                    cursor=argo_read_char(f);
                    continue;
                }
                else if (cursor==ARGO_RBRACK) {
                    return 0;
                }
                else {  
                    fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);  
                    return -1;
                }
            }
            return -1;
        }
        else if(cursor==*(ARGO_TRUE_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_TRUE_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_TRUE_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_TRUE_TOKEN+3)) {
                            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
                            argo_next_value++;
                            add_to_linkedList(sentinel, new_value);
                            new_value->type=1;
                            new_value->content.basic=ARGO_TRUE;
                            cursor=argo_read_char(f);
                            if(cursor==ARGO_COMMA) {
                                cursor=argo_read_char(f);
                                continue;
                            }
                            else if (cursor==ARGO_RBRACK) {
                                return 0;
                            }
                            else {
                                fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);    
                                return -1;
                            }
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting true\n", argo_lines_read+1);
                return -1;
        }
        else if (cursor==*(ARGO_FALSE_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_FALSE_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_FALSE_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_FALSE_TOKEN+3)) {
                            cursor=argo_read_char(f);
                            if(cursor==*(ARGO_FALSE_TOKEN+4)) {
                                ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
                                argo_next_value++;
                                add_to_linkedList(sentinel, new_value);
                                new_value->type=1;
                                new_value->content.basic=ARGO_FALSE;
                                cursor=argo_read_char(f);
                                if(cursor==ARGO_COMMA) {
                                    cursor=argo_read_char(f);
                                    continue;
                                }
                                else if (cursor==ARGO_RBRACK) {
                                    return 0;
                                }
                                else {    
                                    fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting false\n", argo_lines_read+1);
                return -1;
        }
        else if (cursor==*(ARGO_NULL_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_NULL_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_NULL_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_NULL_TOKEN+3)) {
                            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
                            argo_next_value++;
                            add_to_linkedList(sentinel, new_value);
                            new_value->type=1;
                            new_value->content.basic=ARGO_NULL;
                            cursor=argo_read_char(f);
                            if(cursor==ARGO_COMMA) {
                                cursor=argo_read_char(f);
                                continue;
                            }
                            else if (cursor==ARGO_RBRACK) {
                                return 0;
                            }
                            else {  
                                fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);  
                                return -1;
                            }
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting null\n", argo_lines_read+1);
                return -1;
        }
        else if(cursor==ARGO_LBRACK) {
            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
            argo_next_value++;
            add_to_linkedList(sentinel, new_value);
            new_value->type=5;
            if(argo_read_array(new_value, f)==0) {
                cursor=argo_read_char(f);
                if(cursor==ARGO_COMMA) {
                    cursor=argo_read_char(f);
                    continue;
                }
                else if (cursor==ARGO_RBRACK) {
                    return 0;
                }
                else {   
                    fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);   
                    return -1;
                }
            }
            return -1;
        }
        else if(cursor==ARGO_LBRACE) {
            ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
            argo_next_value++;
            add_to_linkedList(sentinel, new_value);
            new_value->type=4;
            if(argo_read_object(new_value, f)==0) {
                cursor=argo_read_char(f);
                if(cursor==ARGO_COMMA) {
                    cursor=argo_read_char(f);
                    continue;
                }
                else if (cursor==ARGO_RBRACK) {
                    return 0;
                }
                else  {   
                    fprintf(stderr, "[Line: %d] Expecting closing array bracket or [Line: any] Expecting comma\n", argo_lines_read+1);  
                    return -1;
                }
            }
            return -1;
        }
        else {
            fprintf(stderr, "[Line: %d] Illegal start to an array\n", argo_lines_read+1);  
            return -1;
        }
    }
    fprintf(stderr, "[Line: %d] Illegal ending to json file\n", argo_lines_read+1);
    return -1;
}

int argo_read_object(ARGO_VALUE *v, FILE *f) {
    ARGO_VALUE *sentinel = (argo_value_storage+argo_next_value);  //initalize the sentinel
    argo_next_value++;
    v->content.object.member_list=sentinel;  //set argo value -> content -> member_list to sentinel
    sentinel->next=sentinel;
    sentinel->prev=sentinel;
    sentinel->name.content=NULL;
    sentinel->type=0;
    ARGO_CHAR cursor = argo_read_char(f);         //read next char and continue reading file
    while(cursor!=EOF) {
        if(cursor==ARGO_RBRACE) {  //If cursor at }
            return 0;          //If end of file return true
        }
        else if(cursor==ARGO_QUOTE) {    //If cursor is at "
            if(argo_read_jsonLine(sentinel, f)==0) {
                cursor=argo_read_char(f);
                if(cursor==ARGO_COMMA) {
                    cursor=argo_read_char(f);
                    continue;
                }
                else if (cursor==ARGO_RBRACE) {
                    return 0;
                }
                else {
                    fprintf(stderr, "[Line: %d] Expecting closing object brace or [Line: any] Expecting comma\n", argo_lines_read+1);
                    return -1;
                }
            }
            else    
                return -1;
        }
        else {
            fprintf(stderr, "[Line: %d] Illegal start to object\n", argo_lines_read+1);
            return -1;               //Else return 0;
        }
    }
    fprintf(stderr, "[Line: %d] Illegal ending to json file\n", argo_lines_read+1);
    return -1;
}
int argo_read_jsonLine(ARGO_VALUE *sentinel, FILE *f) {
    ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
    argo_next_value++;
    add_to_linkedList(sentinel, new_value);
    if(argo_read_string(&(new_value->name), f)==0) {
        ARGO_CHAR cursor=argo_read_char(f);
        if(cursor==ARGO_COLON) {
            cursor=argo_read_char(f);
            if(argo_is_digit(cursor) || cursor==ARGO_MINUS) {
                ungetc(cursor, f);
                new_value->type=2;
                if(argo_read_number(&(new_value->content.number), f)==0) {
                    return 0;
                }
                else {
                    return -1;
                }
            }
            else if (cursor==ARGO_LBRACE) {
                new_value->type=4;
                if(argo_read_object(new_value, f)==0) {
                    return 0;
                }
                return -1;
            }
            else if (cursor==ARGO_LBRACK) {
                new_value->type=5;
                if(argo_read_array(new_value, f)==0) {
                    return 0;
                }
                return -1;
            }
            else if(cursor==ARGO_QUOTE) {
                new_value->type=3;
                if(argo_read_string(&(new_value->content.string), f)==0) {
                    return 0;
                }
                else {
                    return -1;
                }
            }
            else if(cursor==*(ARGO_TRUE_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_TRUE_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_TRUE_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_TRUE_TOKEN+3)) {
                            new_value->type=1;
                            new_value->content.basic=ARGO_TRUE;
                            return 0;
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting true\n", argo_lines_read+1);
                return -1;
            }
            else if (cursor==*(ARGO_FALSE_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_FALSE_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_FALSE_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_FALSE_TOKEN+3)) {
                            cursor=argo_read_char(f);
                            if(cursor==*(ARGO_FALSE_TOKEN+4)) {
                                new_value->type=1;
                                new_value->content.basic=ARGO_FALSE;
                                return 0;
                            }
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting false\n", argo_lines_read+1);
                return -1;
            }
            else if (cursor==*(ARGO_NULL_TOKEN)) {
                cursor=argo_read_char(f);
                if(cursor==*(ARGO_NULL_TOKEN+1)) {
                    cursor=argo_read_char(f);
                    if(cursor==*(ARGO_NULL_TOKEN+2)) {
                        cursor=argo_read_char(f);
                        if(cursor==*(ARGO_NULL_TOKEN+3)) {
                            new_value->type=1;
                            new_value->content.basic=ARGO_NULL;
                            return 0;
                        }
                    }
                }
                fprintf(stderr, "[Line: %d] Expecting null\n", argo_lines_read+1);
                return -1;
            }
            else {
                fprintf(stderr, "[Line: %d] Illegal start after colon\n", argo_lines_read+1);
                return -1;
            }
        }
        else {
            fprintf(stderr, "[Line: %d] Expecting colon\n", argo_lines_read+1);
            return -1;
        }
    }
    else
        return -1;
    return 0;
}
int add_to_linkedList(ARGO_VALUE *sentinel, ARGO_VALUE *new_value) {
    if(sentinel->next==sentinel) {
        sentinel->next=new_value;
        sentinel->prev=new_value;      
        new_value->next=sentinel;
        new_value->prev=sentinel;
    }
    else {
        ARGO_VALUE *loop_cursor = sentinel->next;
        while(loop_cursor!=sentinel) {
            if(loop_cursor->next==sentinel) {
                loop_cursor->next=new_value;
                new_value->next=sentinel;
                sentinel->prev=new_value;
                new_value->prev=loop_cursor;
                break;
            }
            loop_cursor=loop_cursor->next;
        }
    }
    return 0;
}

ARGO_CHAR argo_read_char(FILE *f) {
    ARGO_CHAR cursor = fgetc(f);
    while(argo_is_whitespace(cursor)) {
        if(cursor==ARGO_LF)
            argo_lines_read++;
        argo_chars_read++;
        cursor=fgetc(f);
    }
    return cursor;
}
ARGO_CHAR argo_read_char_forString(FILE *f) {
    ARGO_CHAR cursor = fgetc(f);
    argo_chars_read++;
    if(cursor==ARGO_LF)
        argo_lines_read++;
    return cursor;
}
/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
 
 
int argo_read_string(ARGO_STRING *s, FILE *f) {
    // TO BE IMPLEMENTED.
    ARGO_CHAR cursor=argo_read_char_forString(f);    //read next character
    while(cursor!=EOF) {                   //continue reading file
        if(cursor==ARGO_QUOTE)             //If close quote " then return successful
            return 0;
        else if(argo_is_control(cursor)) {
            cursor=argo_read_char_forString(f);
            continue;
        }
        else if(cursor==ARGO_BSLASH) {
            cursor=argo_read_char_forString(f);
            if(cursor==ARGO_QUOTE) {
                argo_append_char(s, ARGO_QUOTE);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_BSLASH) {
                argo_append_char(s, ARGO_BSLASH);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_FSLASH) {
                argo_append_char(s, ARGO_FSLASH);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_B) {
                argo_append_char(s, ARGO_BS);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_F) {
                argo_append_char(s, ARGO_FF);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_N) {
                argo_append_char(s, ARGO_LF);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_R) {
                argo_append_char(s, ARGO_CR);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_T) {
                argo_append_char(s, ARGO_HT);
                cursor=argo_read_char_forString(f);
                continue;
            }
            else if(cursor==ARGO_U) {
                int x = argo_convert_hex_dec(f);
                if(x!=-1) {
                    argo_append_char(s, x);
                    cursor=argo_read_char_forString(f);
                    continue;
                }
                else 
                    return -1;
            }
            else {
                fprintf(stderr, "[Line: %d] Illegal character after back slash\n", argo_lines_read+1);
                return -1;
            }
        }
        else {
            argo_append_char(s, cursor);   //else append the character to the string
            cursor=argo_read_char_forString(f);      //move on to next character
            continue;
        }
    }
    fprintf(stderr, "[Line: %d] Illegal ending to json file\n", argo_lines_read+1);
    return -1;                              //return unsuccessful because of EOF
}
int argo_convert_hex_dec(FILE *f) {
    ARGO_CHAR cursor='0';
    int int_value=0;
    for(int i=0; i<4; i++) {
        cursor = argo_read_char_forString(f);
        if(argo_is_hex(cursor)){
            int current_hex=0;
            int sixteen_value=0;
            if(cursor>='0' && cursor<='9') 
                current_hex=cursor-'0';
            else if(cursor>='A' && cursor<='Z')
                current_hex=cursor-55;
            else {
                current_hex=cursor-87;
            }
            if(i==0) {
                sixteen_value=4096;
            }
            else if(i==1) {
                sixteen_value=256;
            }
            else if(i==2) {
                sixteen_value=16;
            }
            else if(i==3) {
                sixteen_value=1;
            }
            int_value=int_value+(current_hex*sixteen_value);
        }
        else {
            fprintf(stderr, "[Line: %d] Expecting hex\n", argo_lines_read+1);
            return -1;
        }
    }
    return int_value;
}
/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */


int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    char cursor = argo_read_char(f);
    int neg = 0;
    if(cursor==ARGO_MINUS) {
        argo_append_char(&(n->string_value), cursor);
        neg = 1;
        cursor=argo_read_char(f);
    }
    n->valid_string=1;
    n->valid_int=1;
    n->valid_float=1;
    while (cursor!=EOF) {
        if(argo_is_digit(cursor)) {
            argo_append_char(&(n->string_value), cursor);
            if(n->valid_int) {
                n->int_value=((n->int_value)*10)+(cursor-'0');
            }
            if(n->valid_float) {
                n->float_value=(double)(((n->float_value)*10)+(cursor-'0'));
            }
            cursor=argo_read_char(f);
            continue;
        }
        else if (argo_is_exponent(cursor)) {
            n->valid_int=0;
            n->int_value=0;
            argo_append_char(&(n->string_value), cursor);
            cursor=argo_read_char(f);
            int exponent=0;
            if(cursor==ARGO_MINUS) {
                argo_append_char(&(n->string_value), cursor);
                cursor=argo_read_char(f);
                while(argo_is_digit(cursor)) {
                    argo_append_char(&(n->string_value), cursor);
                    exponent=(exponent*10)+(cursor-'0');
                    cursor=argo_read_char(f);
                }
                if(cursor==ARGO_LF)
                    argo_lines_read--;
                argo_chars_read--;
                ungetc(cursor, f);
                for(int i=0; i<exponent; i++) {
                    n->float_value=n->float_value*0.1;
                }
                if(neg) {
                    n->float_value=n->float_value*-1;
                    n->int_value=n->int_value*-1;
                }
            }
            else {
                while(argo_is_digit(cursor)) {
                    argo_append_char(&(n->string_value), cursor);
                    exponent=(exponent*10)+(cursor-'0');
                    cursor=argo_read_char(f);
                }
                if(cursor==ARGO_LF)
                    argo_lines_read--;
                ungetc(cursor, f);
                for(int i=0; i<exponent; i++) {
                    n->float_value=n->float_value*10;
                }
                if(neg) {
                    n->float_value=n->float_value*-1;
                    n->int_value=n->int_value*-1;
                }
            }
            return 0;
        }
        else if (cursor==ARGO_PERIOD) {
            n->valid_int=0;
            n->int_value=0;
            argo_append_char(&(n->string_value), cursor);
            cursor = argo_read_char(f);
            int counter=0;
            while(argo_is_digit(cursor)) {
                argo_append_char(&(n->string_value), cursor);
                n->float_value=(double)((n->float_value*10)+(cursor-'0'));
                cursor=argo_read_char(f);
                counter++;
            }
            for (int i=0; i<counter; i++) {
                n->float_value=n->float_value*0.1;
            }
            if(argo_is_exponent(cursor)) 
                continue;
            if(neg) {
                    n->float_value=n->float_value*-1;
                    n->int_value=n->int_value*-1;
            }
            ungetc(cursor, f);
            return 0;
        }
        else if (cursor==ARGO_RBRACE || cursor==ARGO_RBRACK) {
            ungetc(cursor, f);
            return 0;
        }
        else if (argo_is_whitespace(cursor) || cursor==ARGO_COMMA) {
            if(neg) {
                    n->float_value=n->float_value*-1;
                    n->int_value=n->int_value*-1;
            }
            ungetc(cursor, f);
            return 0;
        }
        else {
            fprintf(stderr, "[Line: %d] Illegal number\n", argo_lines_read+1);
            return -1;
        }
    }
    return 0;
}


/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_value(ARGO_VALUE *v, FILE *f) {
    indent_level=0;
    if(v->type==1) { //basic
        if(v->content.basic==0) { 
            fputs(ARGO_NULL_TOKEN, f);
            return 0;
        }
        else if(v->content.basic==1) {
            fputs(ARGO_TRUE_TOKEN, f);
            return 0;
        }
        else if(v->content.basic==2) {
            fputs(ARGO_FALSE_TOKEN, f);
            return 0;
        }
        else return -1;
    }
    else if(v->type==2) { //number
        if(argo_write_number(&(v->content.number), f)!=-1) {
            return 0;
        }
        else return -1;
    }
    else if(v->type==3) { //string
        if(argo_write_string(&(v->content.string), f)!=-1) {
            return 0;
        }
        else return -1;
    }
    else if(v->type==4) { //object
        if(argo_write_object(v->content.object.member_list, f)!=-1) {
            return 0;
        }
        else return -1;
    }
    else if(v->type==5) { //array
        if(argo_write_array(v->content.array.element_list, f)!=-1) {
            return 0;
        }
        else return -1;
    }
    return -1;
}
int argo_write_array(ARGO_VALUE *v, FILE *f) {
    fputc(ARGO_LBRACK, f);
    if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
        indent_level++;
        fputc(ARGO_LF, f);
    }
    ARGO_VALUE *pointer = v->next;
    while(pointer!=v) {
        if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
            int space=global_options-CANONICALIZE_OPTION-PRETTY_PRINT_OPTION;
            space=space*indent_level;
            for(int i=0; i<space; i++)
                fputc(ARGO_SPACE, f);
        }
        if(pointer->type==1) {
            if(pointer->content.basic==0)  
                fputs(ARGO_NULL_TOKEN, f);
            else if(pointer->content.basic==1) {
                fputs(ARGO_TRUE_TOKEN, f);
            }
            else if(pointer->content.basic==2) {
                fputs(ARGO_FALSE_TOKEN, f);
            }
            else return -1;
        }
        else if(pointer->type==2) {
            if(argo_write_number(&(pointer->content.number), f)==-1) {
                return -1;
            }
        }
        else if(pointer->type==3) {
            if(argo_write_string(&(pointer->content.string), f)==-1) {
                return -1;
            }
        }
        else if(pointer->type==4) {
            if(argo_write_object(pointer->content.object.member_list, f)==-1) {
                return -1;
            }
        }
        else if(pointer->type==5) {
            if(argo_write_array(pointer->content.array.element_list, f)==-1) {
                return -1;
            }
        }
        else return -1;
        if(pointer->next!=v)
            fputc(ARGO_COMMA, f);
        if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
            fputc(ARGO_LF, f);
        }
        pointer=pointer->next;
    }
    if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
        indent_level--;
        int space=global_options-CANONICALIZE_OPTION-PRETTY_PRINT_OPTION;
        space=space*indent_level;
        for(int i=0; i<space; i++)
            fputc(ARGO_SPACE, f);
    }
    fputc(ARGO_RBRACK, f);
    return 0;
}
int argo_write_object(ARGO_VALUE *v, FILE *f) {
    fputc(ARGO_LBRACE, f);
    if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
        indent_level++;
        fputc(ARGO_LF, f);
    }
    ARGO_VALUE *pointer = v->next;
    while(pointer!=v) {
        if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
            int space=global_options-CANONICALIZE_OPTION-PRETTY_PRINT_OPTION;
            space=space*indent_level;
            for(int i=0; i<space; i++)
                fputc(ARGO_SPACE, f);
        }
        if(pointer->type==1) {
            if(argo_write_string(&(pointer->name), f)!=-1) {
                fputc(ARGO_COLON, f);
                fputc(ARGO_SPACE, f);
                if(pointer->content.basic==0)  
                    fputs(ARGO_NULL_TOKEN, f);
                else if(pointer->content.basic==1) {
                    fputs(ARGO_TRUE_TOKEN, f);
                }
                else if(pointer->content.basic==2) {
                    fputs(ARGO_FALSE_TOKEN, f);
                }
                else return -1;
            }
            else return -1;
        }
        else if(pointer->type==2) {
            if(argo_write_string(&(pointer->name), f)!=-1) {
                fputc(ARGO_COLON, f);
                fputc(ARGO_SPACE, f);
                if(argo_write_number(&(pointer->content.number), f)==-1)
                    return -1;
            }
        }
        else if(pointer->type==3) {
            if(argo_write_string(&(pointer->name), f)!=-1) {
                fputc(ARGO_COLON, f);
                fputc(ARGO_SPACE, f);
                if(argo_write_string(&(pointer->content.string), f)==-1)
                    return -1;
            }
        }
        else if(pointer->type==4) {
            if(argo_write_string(&(pointer->name), f)!=-1) {
                fputc(ARGO_COLON, f);
                fputc(ARGO_SPACE, f);
                if(argo_write_object(pointer->content.object.member_list,f)==-1)
                    return -1;
            }
        }
        else if(pointer->type==5) {
            if(argo_write_string(&(pointer->name), f)==-1) {
                fputc(ARGO_COLON, f);
                fputc(ARGO_SPACE, f);
                if(argo_write_array(pointer->content.array.element_list, f)==-1)
                    return -1;
            }
        }
        else return -1;
        if(pointer->next!=v)
            fputc(ARGO_COMMA, f);
        if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
            fputc(ARGO_LF, f);
        }
        pointer=pointer->next;
    }
    if(global_options>=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION) {
        indent_level--;
        int space=global_options-CANONICALIZE_OPTION-PRETTY_PRINT_OPTION;
        space=space*indent_level;
        for(int i=0; i<space; i++)
            fputc(ARGO_SPACE, f);
    }
    fputc(ARGO_RBRACE, f);
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */

 
int argo_write_string(ARGO_STRING *s, FILE *f) {
    ARGO_CHAR *cursor = s->content;
    fputc(ARGO_QUOTE, f);
    for(int i=0; i<s->length; i++) {
        if(*cursor>31 && *cursor<256) {
            if(*cursor==92) {
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_BSLASH, f);
                cursor++;
            }
            else if(*cursor==34) {
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_QUOTE, f);
                cursor++;
            }
            else {
                fputc(*cursor, f);
                cursor++;
            }
        }
        else if(*cursor>-1 && *cursor<65536) {
            if(*cursor==8) {
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_B, f);
                cursor++;
            }
            else if(*cursor==9){
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_T, f);
                cursor++;
            }
            else if(*cursor==10){
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_N, f);
                cursor++;
            }
            else if(*cursor==12){
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_F, f);
                cursor++;
            }
            else if(*cursor==13) {
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_R, f);
                cursor++;
            }
            else {
                double number= (double)*cursor;
                double remainder=0;
                number=number/16;
                remainder=number-(int)number;
                int one=(int)(remainder*16);
                number=number/16;
                remainder=number-(int)number;
                int two=(int)(remainder*16);
                number=number/16;
                remainder=number-(int)number;
                int three=(int)(remainder*16);
                number=number/16;
                remainder=number-(int)number;
                int four=(int)(remainder*16);
                if(one>9) 
                    one=one+87;
                else
                    one=one+'0';
                if(two>9) 
                    two=two+87;
                else
                    two=two+'0';
                if(three>9)
                    three=three+87;
                else
                    three=three+'0';
                if(four>9)
                    four=four+87;
                else
                    four=four+'0';
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_U, f);
                fputc(four, f);
                fputc(three, f);
                fputc(two, f);
                fputc(one, f);
            }
        }
        else    
            return -1;
    }
    fputc(ARGO_QUOTE, f);
    return 0;
}


/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */



int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    if(n->valid_int) {
        for(int i=0; i<n->string_value.length; i++) {
            fputc(*(n->string_value.content+i), f);
        }
        return 0;
    }
    else if(n->valid_float) {
        if((n->float_value>=0.1 && n->float_value<1) || (n->float_value<=-0.1 && n->float_value>-1) || n->float_value==0.0) {
            for(int i=0; i<n->string_value.length; i++) {
                fputc(*(n->string_value.content+i), f);
            }
            return 0;
        }
        else if(n->float_value>-1 && n->float_value<1) {
            int negative = 0;
            int exponent_counter=0;
            if(n->float_value<0) {
                negative=1;
                n->float_value=n->float_value*-1;
            }
            while(n->float_value<0.1) {
                n->float_value=n->float_value*10;
                exponent_counter++;
            }
            ARGO_CHAR *cursor = n->string_value.content;
            while(*cursor<'1' || *cursor>'9')
                cursor++;
            if(negative)
                fputc(ARGO_MINUS, f);
            fputc('0', f);
            fputc('.', f);
            while(argo_is_digit(*cursor) || *cursor==ARGO_PERIOD) {
                if(*cursor==ARGO_PERIOD) {
                    cursor++;
                    continue;
                }
                fputc(*cursor, f);
                cursor++;
            }
            putc(ARGO_E, f);
            putc(ARGO_MINUS,f);
            putc(exponent_counter+'0', f);
            return 0;
        }
        else {
            int negative=0;
            int exponent_counter=0;
            if(n->float_value<0) {
                negative=1;
                n->float_value=n->float_value*-1;
            }
            while(n->float_value>=1) {
                n->float_value=n->float_value/10;
                exponent_counter++;
            }
            ARGO_CHAR *cursor = n->string_value.content;
            while(*cursor<'1' || *cursor>'9')
                cursor++;
            if(negative)
                fputc(ARGO_MINUS, f);
            fputc('0', f);
            fputc('.', f);
            while(argo_is_digit(*cursor) || *cursor==ARGO_PERIOD) {
                if(*cursor==ARGO_PERIOD) {
                    cursor++;
                    continue;
                }
                fputc(*cursor, f);
                cursor++;
            }
            putc(ARGO_E, f);
            putc(exponent_counter+'0', f);
            return 0;
        }
    }
    fprintf( stderr, "Error: writing invalid number");
    return -1;
}

