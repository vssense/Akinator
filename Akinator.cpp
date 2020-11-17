#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ReadFile.cpp"

const int DOT_CMD_SIZE = 40;
const int JPG_CMD_SIZE = 20;

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

struct AkiTree
{
	Node* root = nullptr;
	Node* NIL  = nullptr;

	TreeStatus status = UNCHANGED;
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

void GetNodes(AkiTree* tree, Node* node, Text* text, size_t ofs)
{
	node->data = strchr(text->lines[ofs++].str, '"') + 1;
	char* tmp = strchr(node->data, '"');
	if (tmp) 
	{
		tmp[0] = '\0';
	}

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

void GetData(AkiTree* tree, FILE* input_file)
{
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

	tree->root->data = strchr(text.lines[ofs++].str, '"') + 1;
	char* tmp = strchr(tree->root->data, '"');

	if (tmp)
	{
		tmp[0] = '\0';
	}

	if (strchr(text.lines[ofs++].str, '{'))
	{
		tree->root->right = NewNode(tree);
		tree->root->right->parent = tree->root;

		GetNodes(tree, tree->root->right, &text, ofs);
	}
}


void PrintNodes(AkiTree* tree, Node* node, FILE* DumpFile)
{
	fprintf(DumpFile, "\"%p\"[label=\"%s\"]", node, node->data);
	
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

void TreeDump(AkiTree* tree)
{
    FILE* DumpFile = fopen("tree.txt", "w");

	fprintf(DumpFile, "digraph G{\n");
	fprintf(DumpFile, "node [shape=\"record\", style=\"filled\", fillcolor=\"#AED8D5\", color=\"#000000\"]\n");

	PrintNodes(tree, tree->root, DumpFile);

	fprintf(DumpFile, "}");

	fclose(DumpFile);

	char dot_cmd[DOT_CMD_SIZE] = "dot -Tjpg tree.txt > ";
	char jpg_cmd[JPG_CMD_SIZE] = "start ";
	
	GetNames(dot_cmd, jpg_cmd);

	printf("%s\n", dot_cmd);
	printfs("%s\n", jpg_cmd);
	
	system(dot_cmd);
	system(jpg_cmd);
}

void DestructNodes(AkiTree* tree, Node* node)
{
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

	DestructNodes(tree, tree->root);
	free(tree->NIL);
}

int main()
{
	FILE* input = fopen("AkinatorData.txt", "r");
	assert(input);

	AkiTree tree = {};
	Construct(&tree);

	GetData(&tree, input);
	
	TreeDump(&tree);
	Destruct(&tree);

	return 0;
}