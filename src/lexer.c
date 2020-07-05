#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"

//TODO: make list of reserved IDENs. true, false, print, any builtin functions, etc. this might go in the parser instead.

// array of operators
static const char operators[] = {
    '#',
    '(',
    ')',
    '{',
    '}',
    '/',
    '?',
    '+',
    '-',
    '*',
    '=',
    '!',
    ',',
};

int isOperator(char character) {
    for (int i = 0; i < (int) sizeof(operators); i++) {
        if (character == operators[i]) {
            return 1;
        }
    }
    return 0;
}

void copyContent(char **content, char **currContent) {
    *content = malloc(strlen(*currContent) + 1);
    strcpy(*content, *currContent);
}

void resetValues(char **currContent, int lineLength, int *currCharacterNumber) {
    memset(*currContent, 0, lineLength * sizeof(char));
    *currCharacterNumber = 0;
}

void createAndStoreToken(int lineNumber, type tokenType, char *content, token **lineTokens, int *numLineTokens) {
    token t = {
        .line = lineNumber,
        .type = tokenType,
        .content = content,
    };

    (*lineTokens)[*numLineTokens] = t;
    (*numLineTokens)++;
}

void tokenizeLine(char line[], int lineNumber, int *numLineTokens, token **lineTokens, int *maxLineCapacity) {
    int lineLength = strlen(line);
    // strip of \n if line has it at the end
    if (lineLength > 0 && line[lineLength - 1] == '\n') {
        line[lineLength - 1] = 0;
        lineLength = strlen(line);
    }

    // realloc if length of line tokens is too small. lineLength serves as an upperbound
    // to the number of tokens we can have
    if (*maxLineCapacity < lineLength) {
        printf("reallocing lineTokens... old capacity: %d tokens, new capacity: %d tokens\n", *maxLineCapacity, lineLength);
        token *temp = realloc(*lineTokens, lineLength * sizeof(token));
        *maxLineCapacity = lineLength;
        if (temp == NULL) {
            // handle error when realloc isn't successful
        } else {
            *lineTokens = temp;
        }
    }

    // reset numLineTokens back to 0
    *numLineTokens = 0;
    
    char *currContent = malloc(lineLength * sizeof(char));
    int currCharacterNumber = 0;

    char* content;

    for (int i = 0; line[i] != '\0';) {
        char currChar = line[i];

        // check for string
        if (currChar == '"') {
            currContent[currCharacterNumber] = '"';
            currCharacterNumber++;
            for (int j = i + 1; line[j] != '\0'; j++) {
                currContent[currCharacterNumber] = line[j];
                currCharacterNumber++;
                //TODO: figure out escape characters
                if (line[j] == '"') {
                    copyContent(&content, &currContent);
                    createAndStoreToken(lineNumber, STR, content, lineTokens, numLineTokens);
                    resetValues(&currContent, lineLength, &currCharacterNumber);

                    i = j + 1;
                    break;
                }
            }
            // TODO: if it reaches here, then there was no closing " for a string. Error here
        
        // check for identifier (var name)
        } else if (isalpha(currChar)) {
            currContent[currCharacterNumber] = currChar;
            currCharacterNumber++;
            for (int j = i + 1;; j++) {
                if (isalnum(line[j])) {
                    currContent[currCharacterNumber] = line[j];
                    currCharacterNumber++;
                } else {
                    copyContent(&content, &currContent);
                    createAndStoreToken(lineNumber, IDEN, content, lineTokens, numLineTokens);
                    resetValues(&currContent, lineLength, &currCharacterNumber);

                    i = j;
                    break;
                }
            }
        } else if (isdigit(currChar)) {
            currContent[currCharacterNumber] = currChar;
            currCharacterNumber++;
            type numType = INT;
            for (int j = i + 1;; j++) {
                if (isdigit(line[j])) {
                    currContent[currCharacterNumber] = line[j];
                    currCharacterNumber++;
                } else if (line[j] == '.') {
                    // TODO; if . is last character in number, throw an error. 33.0 is fine, 33. is not
                    numType = FLT;
                    currContent[currCharacterNumber] = line[j];
                    currCharacterNumber++;
                } else {
                    copyContent(&content, &currContent);
                    createAndStoreToken(lineNumber, numType, content, lineTokens, numLineTokens);
                    resetValues(&currContent, lineLength, &currCharacterNumber);

                    i = j;
                    break;
                }
            }
        } else if (isOperator(currChar)) {
            currContent[currCharacterNumber] = currChar;
            currCharacterNumber++;
            copyContent(&content, &currContent);
            createAndStoreToken(lineNumber, OP, content, lineTokens, numLineTokens);
            resetValues(&currContent, lineLength, &currCharacterNumber);
            
            i++;
        } else {
            i++;
        }
    }

    free(currContent);
}

// NOTE: whatever calls this function must free tokens and the contents of the tokens after using them
void tokenize(char* fileName, token **tokens, int *numTokens) {
    FILE* file = fopen(fileName, "r");
    char line[256];
    int lineNumber = 1;
    
    int defaultSize = 10;
    int maxCapacity = defaultSize;
    *tokens = malloc(sizeof(token) * maxCapacity);
    *numTokens = 0;

    int defaultLineTokensSize = 10;
    int maxLineCapacity = defaultLineTokensSize;
    token *lineTokens = malloc(sizeof(token) * maxLineCapacity);
    int numLineTokens = 0;

    // read file line by line
    while (fgets(line, sizeof(line), file)) {
        // numLineTokens, lineTokens, and maxLineCapacity will be changed from tokenizeLine call.
        // numLineTokens will be the number of tokens in the line.
        // lineTokens will contain exactly numLineTokens number of tokens.
        // maxLineCapacity is the max number of tokens in a line, and will be increased if necessary.
        tokenizeLine(line, lineNumber, &numLineTokens, &lineTokens, &maxLineCapacity);
        // loop over all line tokens to add them into tokens array.
        for (int i = 0; i < numLineTokens; i++) {
            if (*numTokens >= maxCapacity) {
                printf("reallocing tokens... old size: %d, new size: %d\n", maxCapacity, maxCapacity * 2);
                maxCapacity *= 2;
                token *temp = realloc(*tokens, sizeof(token) * maxCapacity);
                if (temp == NULL) {
                    // handle error when realloc isn't successful
                } else {
                    *tokens = temp;
                }
            }

            (*tokens)[*numTokens] = lineTokens[i];
            (*numTokens)++;
        }

        lineNumber++;
    }

    free(lineTokens);
    fclose(file);
    return;
}
