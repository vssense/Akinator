//-------------------------------------------------------------------
//! THANKS TO
//! Nikita
//! - for team-brainstorming how to use tts the best way possible
//! - for mentoring my code and for saying it is shit
//-------------------------------------------------------------------


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#include "ReadFile.cpp"

const size_t DOT_CMD_SIZE = 40;
const size_t JPG_CMD_SIZE = 20;
const size_t ANSWER_SIZE = 64;
const size_t DEFINITION_SIZE = 64;
const size_t FILE_IS_NOT_EXIST = 0xFFFFFFFF;
const size_t NUM_STR_LEN = 5;
const size_t NOTATION = 10;

const char* STANDARD_DATA_NAME = "AkinatorData.txt";
const char* STANDARD_DOT_TXT_FILE_NAME = "Tree.txt";

#define AKI_SPEAK

#ifdef AKI_SPEAK
const size_t SAY_BUFFER_SIZE = 256;
const size_t SPEAK_CMD_SIZE = 121;
const char* SPEAK_CMD = "PowerShell -Command \"Add-Type -AssemblyName "
                        "System.Speech;(New-Object System.Speech.Synth"
                        "esis.SpeechSynthesizer).Speak('";
const char* END_OF_SPEAK_CMD = "');\"";
bool is_turned_on_voice = false;
#endif

struct AkiNode
{
    char* data = nullptr;

    AkiNode* parent = nullptr;

    AkiNode* left   = nullptr;
    AkiNode* right  = nullptr;
};

#include "stack.h"

struct AkiTree
{
    AkiNode* root = nullptr;
    AkiNode* NIL  = nullptr;

    bool is_changed = false;
    const char* file_name = nullptr;

    Stack* nodes_to_destruct = nullptr;

    char* first_buf = nullptr;
    char* second_buf = nullptr;
};

/////////////////////////////
//Binary tree
/////////////////////////////
void     Construct                 (AkiTree* tree);
AkiNode* NewNode                   (AkiTree* tree);
void     Destruct                  (AkiTree* tree);
void     DestructNodes             (AkiTree* tree, AkiNode* node);
void     Delete                    (AkiTree* tree);
void     DestructBuffers           (Stack* stack);
void     TreeDump                  (AkiTree* tree);
void     GetNames                  (char* dot_cmd, char* jpg_cmd);
size_t   GetJPGNumber              ();
void     PrintNodes                (AkiTree* tree, AkiNode* node, FILE* dump_file);


/////////////////////////////
//Work with data
/////////////////////////////
AkiTree* GetTree                   (const int argc, const char* argv);
void     GetData                   (AkiTree* tree, FILE* input_file);
void     GetNodes                  (AkiTree* tree, AkiNode* node, Text* text, size_t ofs);
void     GetString                 (AkiNode* node, const char* str);
void     SaveData                  (AkiTree* tree, AkiNode* node, FILE* DataFile);


/////////////////////////////
//Menu functions
/////////////////////////////
void     StartGame                 (AkiTree* tree, AkiNode* node);
bool     GetAnswer                 ();
void     EndGame                   (AkiTree* tree, AkiNode* node);
void     GetNewDefinition          (AkiTree* tree, AkiNode* node);
void     SetQuestion               (AkiTree* tree, char* buffer, AkiNode* node);
void     PrintDefinition           (AkiTree* tree);
AkiNode* Find                      (AkiTree* tree, AkiNode* node, char* definition);
void     WriteDefinition           (AkiTree* tree, AkiNode* node);
void     WriteDefinitionRecursively(Stack* stack, AkiNode* node);
void     CompareWords              (AkiTree* tree);
void     PrintDifferences          (Stack* stack1, Stack* stack2, char* definition1, char* definition2);
void     ChangeData                (AkiTree** tree);
void     Save                      (AkiTree* tree);
void     SwitchVoice               ();

/////////////////////////////
//other
/////////////////////////////
void     FillStack                 (Stack* stack, AkiTree* tree, AkiNode* node);
void     ScanString                (char* buffer, size_t max_size);
void     DeleteAllAfterChar        (char* buffer, char symbol);
void     Say                       (const char* format, ...);
void     FormatToSpeak             (char* buffer);
void     CheckBuffer               (char** buffer);



int main(const int argc, const char* argv[])
{
    AkiTree* tree = GetTree(argc, argv[1]);

    Say("\nHello! It is akinator. I can guess your word!\n");

    while (true)
    {
        printf("\n\n             MAIN MENU              \n"
               "data: \"%s\"                          \n\n"
               "Start game(you shoud make a word)  : [G]\n"
               "Print definition                   : [D]\n"
               "Compare two words                  : [C]\n"
               "Save Data                          : [S]\n"
               "Open akinator's tree(only with dot): [O]\n"
               "Load another data                  : [N]\n"
               "Exit                               : [X]\n", 
               tree->file_name);
        printf((is_turned_on_voice) ? 
               "Turn off voice                     : [T]\n\n" :
               "Turn on voice                      : [T]\n\n");

        char* command = (char*)calloc(DEFINITION_SIZE, sizeof(char));
        scanf("\n");
        fgets(command, DEFINITION_SIZE, stdin);
        command[0] = toupper(command[0]);
        
        switch (command[0])
        {
            case 'G':
            {
                StartGame(tree, tree->root);
                break;
            }
            case 'D' :
            {
                PrintDefinition(tree);
                break;
            }
            case 'C' :
            {
                CompareWords(tree);
                break;
            }
            case 'S':
            {
                Save(tree);
                break;
            }
            case 'O':
            {
                TreeDump(tree);
                break;
            }
            case 'N':
            {
                ChangeData(&tree);
                break;
            }
            case 'X':
            {
                printf("Goodbye!\n");
                Destruct(tree);
                return 0;
                break;
            }
            case 'T':
            {
                SwitchVoice();
                break;
            }
            default :
            {
                printf("Unknown command, please try again\n");
                break;
            }
        }

    }
    
    Destruct(tree);
    Delete(tree);

    return 0;
}

void Construct(AkiTree* tree)
{
    assert(tree);

    tree->NIL = (AkiNode*)calloc(1, sizeof(AkiNode));

    tree->NIL->right = tree->NIL;
    tree->NIL->left  = tree->NIL;

    tree->root = tree->NIL;

    tree->is_changed = false;

    tree->nodes_to_destruct = NewStack();
    CONSTRUCT(tree->nodes_to_destruct);

    tree->first_buf = nullptr;
    tree->second_buf = nullptr;
}

AkiNode* NewNode(AkiTree* tree)
{
    assert(tree);

    AkiNode* new_node = (AkiNode*)calloc(1, sizeof(AkiNode));

    new_node->left   = tree->NIL;
    new_node->right  = tree->NIL;
    new_node->parent = tree->NIL;

    return new_node;
}

void Destruct(AkiTree* tree)
{
    assert(tree);
 
    free(tree->root->data);
    DestructNodes(tree, tree->root);
    free(tree->NIL);

    if (tree->first_buf)
    {
        free(tree->first_buf);
        tree->first_buf = nullptr;
    }

    if (tree->second_buf)
    {
        free(tree->second_buf);
        tree->second_buf = nullptr;
    }

    DestructBuffers(tree->nodes_to_destruct);

    Destroy(tree->nodes_to_destruct);
    Delete(tree->nodes_to_destruct);
}

void DestructBuffers(Stack* stack)
{
    assert(stack);

    while (stack->size_ > 0)
    {
        AkiNode* node = top(stack);
        assert(node->data);
        free(node->data);

        pop(stack);
    }
}

void Delete(AkiTree* tree)
{
    assert(tree);

    free(tree);
}

void DestructNodes(AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(node);

    if (node != tree->NIL)
    {
        DestructNodes(tree, node->left);
        DestructNodes(tree, node->right);
        free(node);
    }
}   

void TreeDump(AkiTree* tree)
{
    assert(tree);

    FILE* dump_file = fopen(STANDARD_DOT_TXT_FILE_NAME, "w");

    fprintf(dump_file, "digraph G{\n");
    fprintf(dump_file, "node [shape=\"record\", style=\"filled\","
                       " fillcolor=\"#AED8D5\", color=\"#000000\"]\n");

    PrintNodes(tree, tree->root, dump_file);

    fprintf(dump_file, "}");

    fclose(dump_file);

    char dot_cmd[DOT_CMD_SIZE] = "";
    snprintf(dot_cmd, DOT_CMD_SIZE, "dot -Tjpg %s -o Dump", STANDARD_DOT_TXT_FILE_NAME);
    char jpg_cmd[JPG_CMD_SIZE] = "start Dump";
    
    GetNames(dot_cmd, jpg_cmd);

    system(dot_cmd);
    system(jpg_cmd);
}

void GetNames(char* dot_cmd, char* jpg_cmd)
{
    assert(dot_cmd);
    assert(jpg_cmd);

    size_t num = GetJPGNumber();

    char num_str[NUM_STR_LEN] = "";
    itoa(num, num_str, NOTATION);
    char extension[] = ".jpg";


    strcat(dot_cmd, num_str);
    strcat(dot_cmd, extension);
    
    strcat(jpg_cmd, num_str);
    strcat(jpg_cmd, extension);
}

size_t GetJPGNumber()
{
    FILE* numjpgs = fopen("numjpgs.txt", "r");

    size_t num = 0;
    
    if (numjpgs != nullptr)
    {
        fscanf(numjpgs, "%u", &num);
    }

    fclose(numjpgs);

    numjpgs = fopen("numjpgs.txt", "w");
    fprintf(numjpgs, "%u", num + 1);
    fclose(numjpgs);
    
    return num;
}

void PrintNodes(AkiTree* tree, AkiNode* node, FILE* dump_file)
{
    assert(tree);
    assert(node);
    assert(dump_file);

    if (node->right != tree->NIL && node->left != tree->NIL)
    {
        fprintf(dump_file, "\"%p\"[shape=\"record\", label=\"%s?\"]\n", node, node->data);
    }
    else
    {
        fprintf(dump_file, "\"%p\"[shape=\"pentagon\", label=\"%s\"]\n", node, node->data); 
    }
    
    if (node->left != tree->NIL)
    {
        fprintf(dump_file, "\"%p\":sw->\"%p\"[color=\"#210202\", label=\"no\"];\n", node, node->left);
        PrintNodes(tree, node->left, dump_file);
    }
    
    if (node->right != tree->NIL)
    {
        fprintf(dump_file, "\"%p\":se->\"%p\"[color=\"#011504\", label=\"yes\"];\n", node, node->right);
        PrintNodes(tree, node->right, dump_file);
    }
}

AkiTree* GetTree(const int argc, const char* argv)
{
    AkiTree* tree = (AkiTree*)calloc(1, sizeof(AkiTree));

    FILE* input = nullptr;

    if (argc - 1 == 0)
    {
        tree->file_name = STANDARD_DATA_NAME;
        input = fopen(STANDARD_DATA_NAME, "r");
        assert(input);
    }
    else 
    {
        tree->file_name = argv;
        input = fopen(argv, "r");
        assert(input);
    }

    Construct(tree);
    GetData(tree, input);

    fclose(input);
    return tree;
}

void GetData(AkiTree* tree, FILE* input_file)
{
    assert(tree);
    assert(input_file);

    Text text = {};
    ReadTextAndMakeLines(&text, input_file);

    if (text.num_str == 0)
    {
        printf("error : no data\n"
               "Please fill data file with at least one definition\n");
        return;
    }

    tree->root = NewNode(tree);
    size_t ofs = 0;

    GetString(tree->root, text.lines[ofs++].str);
    
    if (text.num_str > ofs)
    {
        if (strchr(text.lines[ofs++].str, '{'))
        {
            tree->root->right = NewNode(tree);
            tree->root->right->parent = tree->root;

            GetNodes(tree, tree->root->right, &text, ofs);
        }
    }
}

void GetNodes(AkiTree* tree, AkiNode* node, Text* text, size_t ofs)
{
    assert(tree);
    assert(node);
    assert(text);

    GetString(node, text->lines[ofs++].str);

    if (strchr(text->lines[ofs].str, '{'))
    {
        ofs++;
        node->right = NewNode(tree);
        node->right->parent = node;

        GetNodes(tree, node->right, text, ofs);
        return;
    }

    while (strchr(text->lines[ofs].str, '}') && ofs < text->num_str - 1)
    {
        node = node->parent;
        ofs++;
    }

    if (ofs < text->num_str - 1)
    {
        ofs++;
        node->left = NewNode(tree);
        node->left->parent = node;

        GetNodes(tree, node->left, text, ofs);
    }
}

void GetString(AkiNode* node, const char* str)
{
    assert(node);

    node->data = strchr(str, '"') + 1;
    char* tmp  = strchr(node->data, '"');
    if (tmp != nullptr) 
    {
        tmp[0] = '\0';
    }
}

void SaveData(AkiTree* tree, AkiNode* node, FILE* data_file)
{
    assert(tree);
    assert(node);


    fprintf(data_file, "\"%s\"\n", node->data);
    if (node->right != tree->NIL)
    {
        fprintf(data_file, "{\n");
        SaveData(tree, node->right, data_file);
        fprintf(data_file, "}\n");
    }

    if (node->left != tree->NIL)
    {
        fprintf(data_file, "{\n");
        SaveData(tree, node->left, data_file);
        fprintf(data_file, "}\n");
    }
}

void StartGame(AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(node);

    if (node->left == tree->NIL|| node->right == tree->NIL)
    {
        Say("Your word is %s\nAm I right?\n", node->data);
        EndGame(tree, node);
        return;
    }

    Say("Is it %s?\n", node->data);

    bool answer = GetAnswer();

    if (answer)
    {
        StartGame(tree, node->right);
    }
    else
    {
        StartGame(tree, node->left);
    }
}

bool GetAnswer()
{
    bool result = false;

    char* answer = (char*)calloc(ANSWER_SIZE, sizeof(char));
    while (true)
    {
        scanf("%s", answer);

        if (strcmpi(answer, "yes") == 0 || answer[0] == '+' || answer[0] == 'y')
        {
            result = true;
            break;
        }

        if (strcmpi(answer, "no") == 0 || answer[0] == '-' || answer[0] == 'n')
        {
            result = false;
            break;
        }

        printf("I don't understant, please write \"yes\\+\" or \"no\\-\"\n");
    }

    free(answer);
    return result;
}

void EndGame(AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(node);

    bool answer = GetAnswer();

    if (answer)
    {
        Say("I am a God");
    }
    else
    {
        Say("Ok, then what did you mean?\n");
        GetNewDefinition(tree, node);
    }
}

void GetNewDefinition(AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(node);

    char* definition = (char*)calloc(DEFINITION_SIZE, sizeof(char));
    ScanString(definition, DEFINITION_SIZE);

    AkiNode* tmp = Find(tree, tree->root, definition);
    
    if (tmp != tree->NIL)
    {
        Say("I think I already know this word!\n");
        WriteDefinition(tree, tmp);
        return;
    }

    node->right = NewNode(tree);
    node->right->parent = node;
    node->left = NewNode(tree);
    node->left->parent = node;

    node->left->data = node->data;
    node->right->data = definition;

    push(tree->nodes_to_destruct, node->right);

    Say("What are differences between %s and %s?\n", 
            node->right->data, node->left->data);
    Say("%s (is) ...\n", node->right->data);

    char* buffer = (char*)calloc(ANSWER_SIZE, sizeof(char));
    ScanString(buffer, ANSWER_SIZE);
    
    SetQuestion(tree, buffer, node);

    tree->is_changed = true;

    Say("Ok, you won, but now I know this word\n");
}

void SetQuestion(AkiTree* tree, char* buffer, AkiNode* node)
{ 
    assert(buffer);
    assert(node);

    if (strstr(buffer, "not") == buffer)
    {
        char* tmp = node->right->data;
        node->right->data = node->left->data;
        node->left->data = tmp;
        node->data = buffer + 4;

        pop(tree->nodes_to_destruct);
        push(tree->nodes_to_destruct, node->left);
    }
    else
    {
        node->data = buffer;
    }

    push(tree->nodes_to_destruct, node);
}

void PrintDefinition(AkiTree* tree)
{
    assert(tree);

    Say("Print word, please\n");
    //char* definition = (char*)calloc(DEFINITION_SIZE, sizeof(char));
    CheckBuffer(&tree->first_buf);
    ScanString(tree->first_buf, DEFINITION_SIZE);
   
    AkiNode* tmp = Find(tree, tree->root, tree->first_buf);

    if (tmp == tree->NIL)
    {
        Say("I do not know this word :(\n");
    }
    else
    {
        WriteDefinition(tree, tmp);
    }
}

AkiNode* Find(AkiTree* tree, AkiNode* node, char* definition)
{
    assert(tree);
    assert(node);

    if (node == tree->NIL)
    {
        return tree->NIL;
    }

    if (strcmpi(node->data, definition) == 0 && 
        (node->right == tree->NIL && node->left == tree->NIL))
    {
        return node;
    }
    
    AkiNode* tmp = Find(tree, node->left, definition);
    if (tmp != tree->NIL)
    {
        return tmp;
    }

    return Find(tree, node->right, definition);
}

void WriteDefinition(AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(node);

    Stack stack = {};
    CONSTRUCT(&stack);
    FillStack(&stack, tree, node);
    pop(&stack);

    Say("%s is ", node->data);
    WriteDefinitionRecursively(&stack, tree->root);
    Destroy(&stack);
    printf("\b\b \n");
}

void WriteDefinitionRecursively(Stack* stack, AkiNode* node)
{
    assert(stack);
    assert(node);

    if (stack->size_ == 0) return;

    AkiNode* child = top(stack);
    pop(stack);

    if (child == node->right)
    {
        Say("%s, ", node->data);
    }
    else
    {
        Say("not %s, ", node->data);
    }

    WriteDefinitionRecursively(stack, child);
}

void CompareWords(AkiTree* tree)
{
    assert(tree);

    Say("print two words with enter, please\n");
    
    printf("first: ");  
    CheckBuffer(&tree->first_buf);
    ScanString(tree->first_buf, DEFINITION_SIZE);
    
    printf("second: ");
    CheckBuffer(&tree->second_buf);
    ScanString(tree->second_buf, DEFINITION_SIZE);
    
    AkiNode* tmp1 = Find(tree, tree->root, tree->first_buf);
    if (tmp1 == tree->NIL)
    {
        Say("I do not know what is %s\n", tree->first_buf);
        return;
    }

    AkiNode* tmp2 = Find(tree, tree->root, tree->second_buf);
    if (tmp2 == tree->NIL)
    {
        Say("I do not know what is %s\n", tree->second_buf);
        return;
    }   

    Stack stack1 = {};
    Stack stack2 = {};

    CONSTRUCT(&stack1);
    CONSTRUCT(&stack2);
    FillStack(&stack1, tree, tmp1);
    FillStack(&stack2, tree, tmp2);

    PrintDifferences(&stack1, &stack2, tree->first_buf, tree->second_buf);

    Destroy(&stack1);
    Destroy(&stack2);
}

void PrintDifferences(Stack* stack1, Stack* stack2, char* definition1, char* definition2)
{
    assert(stack1);
    assert(stack2);
    assert(definition1);
    assert(definition2);

    AkiNode* node = top(stack1);
    pop(stack1);
    pop(stack2);

    if (top(stack1) == top(stack2))
    {   
        Say("Both are ");
    }

    while (top(stack1) == top(stack2) && 
           stack1->size_ > 0 && stack2->size_ > 0)
    {
        AkiNode* child = top(stack1);
        pop(stack1);
        pop(stack2);
    
        if (child == node->right)
        {
            Say("%s, ", node->data);
        }
        else
        {
            Say("not %s, ", node->data);
        }
        node = child;
    }
    printf("\b\b ");

    Say("but ");

    Say("%s is ", definition1);
    WriteDefinitionRecursively(stack1, node);
    Say("\b\b and ");

    Say("%s is ", definition2);
    WriteDefinitionRecursively(stack2, node);
    Say("\b\b \n");
}

void ChangeData(AkiTree** tree)
{
    assert(tree);

    char* name = (char*)calloc(DEFINITION_SIZE, sizeof(char));
    ScanString(name, DEFINITION_SIZE);
    
    if (GetFileAttributes(name) == FILE_IS_NOT_EXIST)
    {
        printf("This data file does not exist\n");
        free(name);
        return;
    }

    Destruct(*tree);
    *tree = GetTree(2, name);               
}

void Save(AkiTree* tree)
{
    assert(tree);

    if (tree->is_changed == true)
    {
        FILE* DataFile = fopen(tree->file_name, "w");
        SaveData(tree, tree->root, DataFile);
        fclose(DataFile);
        tree->is_changed = false;
        Say("Data was saved\n");
    }
    else
    {
        Say("Data is ready\n");
    }
}

void SwitchVoice()
{
    if (is_turned_on_voice)
    {
        is_turned_on_voice = false;
        Say("Voice is turned off");
    }
    else
    {
        is_turned_on_voice = true;
        Say("Voice is turned on");
    }
}

void FillStack(Stack* stack, AkiTree* tree, AkiNode* node)
{
    assert(tree);
    assert(stack);
    assert(node);

    while (node != tree->NIL)
    {
        push(stack, node);
        node = node->parent;
    }
}

void ScanString(char* buffer, size_t max_size)
{
    scanf("\n");
    fgets(buffer, max_size, stdin);
    DeleteAllAfterChar(buffer, '\n');
}

void DeleteAllAfterChar(char* buffer, char symbol)
{
    assert(buffer);

    char* tmp = strchr(buffer, symbol);

    if (tmp)
    {
        tmp[0] = '\0';
    }
}

void Say(const char* format, ...)
{
    va_list args = {};
    va_start(args, format);

    vprintf(format, args);

    if (is_turned_on_voice)
    {
        char* buffer = (char*)calloc(SAY_BUFFER_SIZE, sizeof(char));

        snprintf(buffer, SPEAK_CMD_SIZE, "%s", SPEAK_CMD);
        vsnprintf(buffer + SPEAK_CMD_SIZE - 1, SAY_BUFFER_SIZE - SPEAK_CMD_SIZE, format, args);
      
        FormatToSpeak(buffer);
        strcat(buffer, END_OF_SPEAK_CMD);
        
        system(buffer);

        free(buffer);
    }

    va_end(args);
}

void FormatToSpeak(char* buffer)
{
    assert(buffer);

    size_t pos = 0;

    for (size_t i = 0; buffer[i] != '\0'; ++i)
    {
        if (buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == '\b') 
        {
            continue;
        }

        buffer[pos++] = buffer[i];
    }
    buffer[pos] = '\0';
}

void CheckBuffer(char** buffer)
{
    if (*buffer != nullptr)
    {
        return;
    }

    *buffer = (char*)calloc(DEFINITION_SIZE, sizeof(char));
    assert(*buffer);
}
