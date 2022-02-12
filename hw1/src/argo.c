#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"
#include "function.h"
int argo_read_object(ARGO_VALUE *v, FILE *f);
int argo_read_char(FILE *f);
int argo_read_jsonLine(ARGO_VALUE *sentinel, FILE *f);
int add_to_linkedList(ARGO_VALUE *sentinel, ARGO_VALUE *new_value);

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
    while(cursor!=EOF) {                //read throught the file
        if(cursor==ARGO_LBRACE) {  //If json is an object, initalize it 
            ARGO_VALUE *current_argo_value=(argo_value_storage+argo_next_value);
            argo_next_value++;
            current_argo_value->type=4;   
            current_argo_value->next=current_argo_value;
            current_argo_value->prev=current_argo_value;
            current_argo_value->name.content=NULL;
            if(argo_read_object(argo_value_storage, f))     //and call argo_read_object
                continue;                                   //If successful then continue
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
        }
    }                                              
    /*
    ARGO_CHAR current;
    while((current = fgetc(f)) != EOF) {
        printf("%c", current-0);
        //printf("%s", ", ");
    }
    */
    return NULL;

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
            cursor=argo_read_char(f);
            if (cursor==EOF)
                return 1;          //If end of file return true
            else    
                return 0;          //Else return false
        }
        else if(argo_is_whitespace(cursor)) {
            cursor=argo_read_char(f);
            continue;              //If white space then check next char
        }
        else if(cursor==ARGO_QUOTE) {    //If cursor is at " then we know a key is about to start so we make a new ARGO_VALUE
                if(argo_read_string(&(new_value->name),f)) { //read the string
                    cursor=argo_read_char(f);
                    while(cursor!=EOF) {
                        if (argo_is_whitespac(cursor)) {
                            cursor=argo_read_char(f);
                            continue;
                        }
                        else if (cursor==ARGO_COLON) {

                        }
                        else
                            return 0;
                    }
                }   
                else
                    return 0;
            }
            else {
                ARGO_VALUE *loop_cursor=sentinel->next;
                while(loop_cursor!=sentinel) {
                    if(loop_cursor->next==sentinel) {
                        loop_cursor->next=new_value;
                        new_value->next=sentinel;
                        sentinel->prev=new_value;
                        new_value->prev=loop_cursor;
                        break;
                    }
                }
                if(argo_read_string(&(new_value->name), f)) {
                    cursor=argo_read_char(f);
                    while(cursor!=EOF) {
                        if (argo_is_whitespac(cursor)) {
                            cursor=argo_read_char(f);
                            continue;
                        }
                        else if (cursor==ARGO_COLON) {

                        }
                        else
                            return 0;
                    }
                }
                else 
                    return 0;
            }
        }
        else
            return 0;               //Else return 0;
    }
    return 0;
}
int argo_read_jsonLine(ARGO_VALUE *sentinel, FILE *f) {
    ARGO_VALUE *new_value=(argo_value_storage+argo_next_value);
    argo_next_value++;
    if (sentinel->next==sentinel) {
    sentinel->next=new_value;
    sentinel->prev=new_value;      //insert the new value
    new_value->next=sentinel;
    new_value->prev=sentinel;

}
int add_to_linkedList(ARGO_VALUE *sentinel, ARGO_VALUE *new_value) {

}

ARGO_CHAR argo_read_char(FILE *f) {
    ARGO_CHAR current = fgetc(f);
    argo_chars_read++;
    if(current==ARGO_LF)
        argo_lines_read++;
    return current;
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
    ARGO_CHAR cursor=argo_read_char(f);    //read next character
    while(cursor!=EOF) {                   //continue reading file
        if(cursor==ARGO_QUOTE)             //If close quote " then return successful
            return 1;
        else {
            argo_append_char(s, cursor);   //else append the character to the string
            cursor=argo_read_char(f);      //move on to next character
            continue;
        }
    }
    return 0;                              //return unsuccessful because of EOF
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

 /*
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}
*/

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

 /*
int argo_write_value(ARGO_VALUE *v, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}
*/

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

 /*
int argo_write_string(ARGO_STRING *s, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}
*/

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

 /*
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}
*/
