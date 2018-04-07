/*************************************************************
 * C Code Sample - Command Line Calculator                   *
 * Author: Vincent Politzer <https://github.com/vjapolitzer> *
 *                                                           *
 * Evaluates a mathmatical expression contained within an    *
 * input string and outputs the result                       *
 *************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <float.h>

char* readline();
bool validOperand(const char*);
bool validOperator(const char* str);
int indexOfOp(const char*, char, int);
double applyOp(double, double, char);
void printResult(double);
void printAllocError();

/* evalExpression
 * ...Evaluate mathematical expression
 * ...Parameters:
 * ......char* exp -- string containing expression
 * ......double* result -- memory location to store result
 * ...Returns:
 * ......boolean true if sucessful, false otherwise
 * ...... answer is written to result if sucessful, 0.0 otherwise
 */
bool evalExpression(char* exp, double* result)
{
    double* operands = NULL;
    char* operators = NULL;
    bool* evaluated = NULL;

    int num_operands = 0;
    int num_operators = 0;

    bool parse_operand = true;
    bool error_occurred = false;

    char order_of_ops[] = {'*', '/', '+', '-'};
    char op_curr;
    int op_dex;
    double temp_val;

    char* token = strtok(exp, " ");

    // First, parse the string for the expression to be evaluated
    while(token != NULL)
    {
        if (parse_operand)
        {
            if (!validOperand(token))
            {
                printf("Invalid operand: %s\n", token);
                error_occurred = true;
                break;
            }
            operands = (double * )realloc(operands,
                                          sizeof(double) * ++num_operands);
            if (operands == NULL)
            {
                printAllocError();
                exit(EXIT_FAILURE);
            }

            operands[num_operands - 1] = atof(token);
        }
        else // parsing operator
        {
            if (!validOperator(token))
            {
                printf("Invalid operator: %s\n", token);
                error_occurred = true;
                break;
            }
            operators = (char *)realloc(operators,
                                        sizeof(char) * ++num_operators);
            if (operators == NULL)
            {
                printAllocError();
                exit(EXIT_FAILURE);
            }

            operators[num_operators - 1] = (char)token[0];
        }

        token = strtok(NULL, " ");
        parse_operand = !parse_operand;
    }

    // If parse was successful, check for extra operator
    if (!error_occurred && num_operands - num_operators != 1)
    {
        printf("Missing last operand\n");
        error_occurred = true;
    }

    evaluated = (bool *)calloc(num_operands, sizeof(bool));
    if (evaluated == NULL)
    {
        printAllocError();
        exit(EXIT_FAILURE);
    }

    if (!error_occurred)
    {   // Everything is good, proceed to evaluate the expression
        for (int i = 0; i < 4; i++)
        {
            op_curr = order_of_ops[i]; // evaluate according to order of ops
            while((op_dex = indexOfOp(operators, op_curr, num_operators)) != -1)
            {
                temp_val = applyOp(operands[op_dex],
                                   operands[op_dex + 1], op_curr);

                if (temp_val == DBL_MAX)
                {
                    printf("Divide by zero error\n");
                    error_occurred = true;
                    break;
                }

                evaluated[op_dex] = true;
                evaluated[op_dex + 1] = true;

                for (int j = 0; j < num_operands; j++)
                {
                    if (evaluated[j])
                        operands[j] = temp_val;
                }

                operators[op_dex] = 0; // operation completed, prevent repeats
            }
            if (error_occurred)
                break;
        }

        *result = error_occurred ? 0.0 : operands[0];
    }

    free(operands);
    free(operators);
    free(evaluated);

    return !error_occurred;
}

int main()
{
    char* input_str;
    double result;

    printf("Enter an expression to be evaluated!\n");
    printf("Valid operators are + - * /\n");
    printf("Valid operands are integers or floating point numbers.\n");
    printf("Operands and operators must be space-separated.\n");
    printf("Type quit and hit enter when you are finished.\n");

    while (true)
    {
        printf("\nInput expression: ");
        input_str = readline();

        if (strcmp(input_str, "quit") == 0)
            break;

        if (evalExpression(input_str, &result))
            printResult(result);

        free(input_str);
    }

    printf("Goodbye!\n");

    return 0;
}

/* readLine
 * ...Read a line from stdin, allocating the necessary memory
 * ...Returns:
 * ......char* line_data -- pointer to read in characters
 */
char* readline()
{
    size_t alloc_size = 128;
    size_t num_char = 0;
    char* line_data = (char *)malloc(alloc_size);
    char* cursor;
    char* temp_line_data;

    if (line_data == NULL)
    {
        printAllocError();
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        cursor = line_data + num_char;
        temp_line_data = fgets(cursor, alloc_size - num_char, stdin);

        if (temp_line_data == NULL)
        {
            printf("Error reading from stdin!\n");
            exit(EXIT_FAILURE);
        }

        num_char += strlen(cursor);

        if (num_char < alloc_size - 1 || line_data[num_char - 1] == '\n')
            break;

        alloc_size += 128;

        line_data = (char *)realloc(line_data, alloc_size);

        if (line_data == NULL)
        {
            printAllocError();
            exit(EXIT_FAILURE);
        }
    }

    if (line_data[num_char - 1] == '\n')
        line_data[num_char - 1] = '\0';

    line_data = (char *)realloc(line_data, num_char);

    return line_data;
}

/* validOperand
 * ...Check if given string only contains numbers
 * ...and no more than one decimal char
 * ...Parameters:
 * ......const char* str -- string to validate
 * ...Returns:
 * ......true if given string only contains numbers and
 * ...... no more than one decimal char, false otherwise
 */
bool validOperand(const char* str)
{
    unsigned int num_dec =  0;

    while (*str)
    {
        if (isdigit(*str) == 0
            && *str != '.')
            return false;

        else if (*str == '.')
            num_dec++;

        str++;
    }

    return num_dec <= 1;
}

/* validOperator
 * ...Check if given string is a valid operator
 * ...Parameters:
 * ......const char* str -- string to validate
 * ...Returns:
 * ......true if given string consists of only one char and
 * ...... that char is +, -, *, or /, false otherwise
 */
bool validOperator(const char* str)
{
    return strlen(str) == 1 && strpbrk(str, "+-*/") != NULL;
}

/* indexOfOp
 * ...Find the index of the first occurence of op char in operators
 * ...Parameters:
 * ......const char* operators -- operators from the provided expression
 * ......char op -- the operator character to find
 * ......int num_operators -- the number of operators to search
 * ...Returns:
 * ......the integer index of the first occurence of op in operators,
 * ...... -1 otherwise
 */
int indexOfOp(const char* operators, char op, int num_operators)
{
    for (int i = 0; i < num_operators; i++)
    {
        if (operators[i] == op)
            return i;
    }

    return -1;
}

/* applyOp
 * ...Apply operation op on a and b
 * ...Parameters:
 * ......double a -- the first number to operate on
 * ......double b -- the second number to operate on
 * ......char op -- char representing the operation to perform
 * ...Returns:
 * ......the result of the operation, DBL_MAX if divide by zero
 */
double applyOp(double a, double b, char op)
{
    double result = 0.0;
    switch (op)
    {
        case '+':
            result = a + b;
            break;

        case '-':
            result = a - b;
            break;

        case '*':
            result = a * b;
            break;

        case '/':
            if (fabs(b) < DBL_EPSILON)
                result = DBL_MAX; // divide by zero
            else
                result = a / b;
    }

    return result;
}

/* printResult
 * ...Print val to stdout, removing trailing zeros and decimal if val is int
 * ...Parameters:
 * ......double val -- value to print to stdout
 * ...Returns:
 * ......Nothing
 */
void printResult(double val)
{
    char* p;
    char* result_str = (char *)malloc(101); // 100 chars is plenty,
                                            // extra 1 for \0 char
    if (result_str != NULL)
    {
        snprintf(result_str, 100, "%.10f", val);
        p = strchr(result_str, '\0'); // pointer to last character in the str

        p--;
        while (*p == '0') // remove trailing zeros
            *p-- = '\0';


        if (*p == '.' || *p == ',')
            *p = '\0'; // remove decimal if val is an integer

        printf("Result: %s\n", result_str);

        free(result_str);
    }
    else
    {
        printAllocError();
        exit(EXIT_FAILURE);
    }
}

/* printAllocError
 * ...Print memory allocation error message to stdout
 * ...Returns:
 * ......Nothing
 */
void printAllocError()
{
    printf("Memory allocation error\n");
}
