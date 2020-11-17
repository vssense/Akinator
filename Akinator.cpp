#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ReadFile.cpp"

const int DOT_CMD_SIZE = 40;
const int JPG_CMD_SIZE = 20;
const int ANSWER_SIZE = 50;
const int DEFINITION_SIZE = 25;

const char* STANDARD_NAME = "AkinatorData.txt";

enum TreeStatus
{
	UNCHANGED = 0,
	CHANGED = 1
};

struct Node
{
	char* data = nullptr;

	Node* parent = nullptr;

	Node* left   = nullptr;
	Node* right  = nullptr;
};

#include "stack.h"

struct AkiTree
{
	Node* root = nullptr;
	Node* NIL  = nullptr;

	TreeStatus status = UNCHANGED;
	const char* FileName = nullptr;
};

Node* NewNode(AkiTree* tree)
{
	Node* new_node = (Node*)calloc(1, sizeof(Node));

	new_node->left   = tree->NIL;
	new_node->right  = tree->NIL;
	new_node->parent = tree->NIL;

	return new_node;
}

void Construct(AkiTree* tree)
{
	tree->NIL = (Node*)calloc(1, sizeof(Node));

	tree->NIL->right = tree->NIL;
	tree->NIL->left  = tree->NIL;

	tree->root = tree->NIL;

	tree->status = UNCHANGED;
}

void GetString(Node* node, const char* str)
{
	node->data = strchr(str, '"') + 1;
	char* tmp  = strchr(node->data, '"');
	
	if (tmp != nullptr) 
	{
		tmp[0] = '\0';
	}
}

void GetNodes(AkiTree* tree, Node* node, Text* text, size_t ofs)
{
	assert(tree);

	GetString(node, text->lines[ofs++].str);

	if (strchr(text->lines[ofs].str, '{'))
	{
		ofs++;
		node->right = NewNode(tree);
		node->right->parent = node;

		GetNodes(tree, node->right, text, ofs);
		return;
	}

	while (strchr(text->lines[ofs].str, '}'))
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


void SaveData(AkiTree* tree, Node* node, FILE* DataFile)
{
	assert(tree);

	fprintf(DataFile, "\"%s\"\n", node->data);
	if (node->right != tree->NIL)
	{
		fprintf(DataFile, "{\n");
		SaveData(tree, node->right, DataFile);
		fprintf(DataFile, "}\n");
	}
	if (node->right != tree->NIL)
	{
		fprintf(DataFile, "{\n");
		SaveData(tree, node->left, DataFile);
		fprintf(DataFile, "}\n");
	}
}

void GetData(AkiTree* tree, FILE* input_file)
{
	assert(tree);

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

	if (strchr(text.lines[ofs++].str, '{'))
	{
		tree->root->right = NewNode(tree);
		tree->root->right->parent = tree->root;

		GetNodes(tree, tree->root->right, &text, ofs);
	}
}

AkiTree* GetTree(const int argc, const char* argv[])
{
	AkiTree* tree = (AkiTree*)calloc(1, sizeof(AkiTree));

	FILE* input = nullptr;

	if (argc - 1 == 0)
	{
		tree->FileName = STANDARD_NAME;
		input = fopen(STANDARD_NAME, "r");
		assert(input);
	}
	else 
	{
		tree->FileName = argv[1];
		input = fopen(argv[1], "r");
		assert(input);
	}
	
	Construct(tree);
	GetData(tree, input);

	fclose(input);
	return tree;
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

void GetNames(char* dot_cmd, char* jpg_cmd)
{
	size_t num = GetJPGNumber();

	char num_str[5] = "";
	itoa(num, num_str, 10);
	char extension[5] = ".jpg";


	strcat(dot_cmd, num_str);
	strcat(dot_cmd, extension);
	
	strcat(jpg_cmd, num_str);
	strcat(jpg_cmd, extension);
}

void PrintNodes(AkiTree* tree, Node* node, FILE* DumpFile)
{
	fprintf(DumpFile, "\"%p\"[label=\"%s\"]\n", node, node->data);
	
	if (node->left != tree->NIL)
	{
		fprintf(DumpFile, "\"%p\":sw->\"%p\"[color=\"#210202\", label=\"no\"];\n", node, node->left);
		PrintNodes(tree, node->left, DumpFile);
	}
	
	if (node->right != tree->NIL)
	{
		fprintf(DumpFile, "\"%p\":se->\"%p\"[color=\"#011504\", label=\"yes\"];\n", node, node->right);
		PrintNodes(tree, node->right, DumpFile);
	}
}

void TreeDump(AkiTree* tree)
{
	assert(tree);
    FILE* DumpFile = fopen("Tree.txt", "w");

	fprintf(DumpFile, "digraph G{\n");
	fprintf(DumpFile, "node [shape=\"record\", style=\"filled\", fillcolor=\"#AED8D5\", color=\"#000000\"]\n");

	PrintNodes(tree, tree->root, DumpFile);

	fprintf(DumpFile, "}");

	fclose(DumpFile);

	char dot_cmd[DOT_CMD_SIZE] = "dot -Tjpg Tree.txt -o Dump";
	char jpg_cmd[JPG_CMD_SIZE] = "start Dump";
	
	GetNames(dot_cmd, jpg_cmd);

	printf("%s\n", dot_cmd);
	printf("%s\n", jpg_cmd);
	
	system(dot_cmd);
	system(jpg_cmd);
}

void DestructNodes(AkiTree* tree, Node* node)
{
	assert(tree);

	if (node != tree->NIL)
	{
		DestructNodes(tree, node->left);
		DestructNodes(tree, node->right);
		free(node);
	}
}	

void Destruct(AkiTree* tree)
{
	assert(tree);
	free(tree->root->data);
	DestructNodes(tree, tree->root);
	free(tree->NIL);
	free(tree);
}

bool GetAnswer()
{
	bool result = false;

	char* answer = (char*)calloc(ANSWER_SIZE, sizeof(char));
	while (true)
	{
		scanf("%s", answer);

		if (strcmp(answer, "yes") == 0 || answer[0] == '+' || answer[0] == 'y')
		{
			result = true;
			break;
		}
		if (strcmp(answer, "no") == 0 || answer[0] == '-' || answer[0] == 'n')
		{
			result = false;
			break;
		}

		printf("I don't understant, please write \"yes\\+\" or \"no\\-\"\n");
	}

	free(answer);
	return result;
}

void DeleteAllAfterChar(char* buffer, char symbol)
{
	char* tmp = strchr(buffer, symbol);

	if (tmp)
	{
		tmp[0] = '\0';
	}
}

void GetNewDefinition(AkiTree* tree, Node* node)
{
	char* definition = (char*)calloc(DEFINITION_SIZE, sizeof(char));
	scanf("\n");
	fgets(definition, DEFINITION_SIZE, stdin);

	DeleteAllAfterChar(definition, '\n');
	
	node->right = NewNode(tree);
	node->right->parent = node;
	node->left = NewNode(tree);
	node->left->parent = node;

	node->left->data = node->data;
	node->right->data = definition;

	printf("What differences between %s and %s?\n", 
			node->right->data, node->left->data);
	printf("%s (is) ...\n", node->right->data);

	char* buffer = (char*)calloc(ANSWER_SIZE, sizeof(char));
	scanf("\n");
	fgets(buffer, ANSWER_SIZE, stdin);
	
	DeleteAllAfterChar(buffer, '\n');

	node->data = buffer;
	tree->status = CHANGED;

	printf("Ok, you won, but now I know this word\n");
}

void EndOfGame(AkiTree* tree, Node* node)
{
	bool answer = GetAnswer();

	if (answer)
	{
		printf("I'm a god");
	}
	else
	{
		printf("Ok, then what did you mean?\n");
		GetNewDefinition(tree, node);
	}
}

void StartGame(AkiTree* tree, Node* node)
{
	if (node->left == tree->NIL|| node->right == tree->NIL)
	{
		printf("Your word is %s\nAm I right?\n", node->data);
		EndOfGame(tree, node);
		return;
	}
	printf("Is it %s?\n", node->data);

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

Node* TraverceSearch(AkiTree* tree, Node* node, char* definition)
{
	if (node == tree->NIL)
	{
		return tree->NIL;
	}

	if (strcmp(node->data, definition) == 0 && 
		(node->right == tree->NIL && node->left == tree->NIL))
	{
		return node;
	}
	
	Node* tmp = TraverceSearch(tree, node->left, definition);
	if (tmp != tree->NIL)
	{
		return tmp;
	}
	return TraverceSearch(tree, node->right, definition);
}

void WriteDefinition(Stack* stack, Node* node)
{
	if (stack->size_ == 0) return;

	Node* child = top(stack);
	pop(stack);

	if (child == node->right)
	{
		printf("%s, ", node->data);
	}
	else
	{
		printf("not %s, ", node->data);
	}
	WriteDefinition(stack, child);
}

void FillStack(Stack* stack, AkiTree* tree, Node* node)
{
	while (node != tree->NIL)
	{
		push(stack, node);
		node = node->parent;
	}
}

void PrintDefinition(AkiTree* tree)
{
	char* definition = (char*)calloc(DEFINITION_SIZE, sizeof(char));
	scanf("\n");
	fgets(definition, DEFINITION_SIZE, stdin);
	DeleteAllAfterChar(definition, '\n');

	Node* tmp = TraverceSearch(tree, tree->root, definition);

	if (tmp == tree->NIL)
	{
		printf("I don't know this word\n");
	}
	else
	{
		Stack stack = {};
		CONSTRUCT(&stack);
      	FillStack(&stack, tree, tmp);
      	pop(&stack);

      	printf("%s is ", tmp->data);
      	WriteDefinition(&stack, tree->root);
      	Destroy(&stack);
      	printf("\b\b \n");
	}
	free(definition);
}

void WriteChanges(Stack* stack1, Stack* stack2, char* definition1, char* definition2)
{
	Node* node = top(stack1);
	pop(stack1);
	pop(stack2);

	while (top(stack1) == top(stack2) && 
		   stack1->size_ > 0 && stack2->size_ > 0)
	{
		Node* child = top(stack1);
		pop(stack1);
		pop(stack2);
	
		if (child == node->right)
		{
			printf("%s, ", node->data);
		}
		else
		{
			printf("not %s, ", node->data);
		}
		node = child;
	}
	printf("\b\b \n");

	printf("\nDifferences:\n");

	printf("\t%s is ", definition1);
	WriteDefinition(stack1, node);
	printf("\b\b \n");

	printf("\t%s is ", definition2);
	WriteDefinition(stack2, node);
	printf("\b\b \n");
}

void CompareWords(AkiTree* tree)
{
	char* definition1 = (char*)calloc(DEFINITION_SIZE, sizeof(char));
	scanf("\n");
	fgets(definition1, DEFINITION_SIZE, stdin);
	DeleteAllAfterChar(definition1, '\n');
	
	char* definition2 = (char*)calloc(DEFINITION_SIZE, sizeof(char));
	scanf("\n");
	fgets(definition2, DEFINITION_SIZE, stdin);
	DeleteAllAfterChar(definition2, '\n');

	Node* tmp1 = TraverceSearch(tree, tree->root, definition1);
	if (tmp1 == tree->NIL)
	{
		printf("I don't know what is %s\n", definition1);
		return;
	}

	Node* tmp2 = TraverceSearch(tree, tree->root, definition2);
	if (tmp2 == tree->NIL)
	{
		printf("I don't know what is %s\n", definition2);
		return;
	}	

	Stack stack1 = {};
	Stack stack2 = {};

	CONSTRUCT(&stack1);	
	CONSTRUCT(&stack2);	
	FillStack(&stack1, tree, tmp1);
	//printf("%s\n", top(&stack1)->data);
	FillStack(&stack2, tree, tmp2);
	//printf("%s\n", top(&stack2)->data);

	printf("Similarities: \n\t");
	WriteChanges(&stack1, &stack2, definition1, definition2);

	Destroy(&stack1);
	Destroy(&stack2);
	free(definition1);
	free(definition2);
}

int main(const int argc, const char* argv[])
{
	AkiTree* tree = GetTree(argc, argv);
	
	printf("\nHello! It is akinator. I can guess your word!\n");

	while (true)
	{
		printf("\n\n            MAIN MENU               \n"
		       "data: \"%s\"                          \n\n"
			   "Start game(you shoud make a word)  : [G]\n"
			   "Print definition                   : [P]\n"
			   "Compare two words                  : [C]\n"
			   "Save Data                          : [S]\n"
			   "Dump akinator's tree               : [D]\n"
			   "Exit                               : [X]\n\n", 
			   tree->FileName);

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
			case 'P' :
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
				if (tree->status == CHANGED)
				{
					FILE* DataFile = fopen(tree->FileName, "w");
					SaveData(tree, tree->root, DataFile);
					tree->status = UNCHANGED;
					fclose(DataFile);
					printf("Data was saved\n");
				}
				else
				{
					printf("Data is ready\n");
				}
				break;
			}
			case 'D' :
			{
				TreeDump(tree);
				break;
			}
			case 'X':
			{
				printf("Goodbye!\n");
				return 0;
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

	return 0;
}