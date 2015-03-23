#include "Dictionary.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node represents state in FSM
typedef struct Node
{
	char is_terminal;
	char character;
	char count_next_nodes;
	struct DictNode *next_begin;
} Node;

typedef struct BuildNode
{
	int begin;
	int end;
} BuildNode;

typedef struct DictNode
{
	union
	{
		Node node;
		BuildNode build_node;
	};
} DictNode;

static void release_data(struct Dictionary *self)
{
	if (self->nodes)
	{
		free(self->nodes);
	}
	self->nodes = NULL;
}

static void release(struct Dictionary *self)
{
	release_data(self);
	free(self);
}

static char load_from_file(struct Dictionary *self, const char *fileName)
{
	FILE *file = fopen(fileName, "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *data = malloc(fsize + 1);
	if (!data) return 0;

	long bytes_read = fread(data, fsize, 1, file);
	fclose(file);
	data[fsize] = 0;

	int num_of_words = 0;
	int num_characters = 0;

	for (int i = 0; i < fsize; ++i)
	{
		if (data[i] > 127 || data[i] < 32)
		{
			data[i] = 0;
		}
		else
		{
			++num_characters;
		}

		if (data[i] != 0 && (i == 0 || data[i - 1] == 0))
		{
			++num_of_words;
		}
	}

	char **array = malloc(sizeof(char*) * num_of_words);
	num_of_words = 0;

	for (int i = 0; i < fsize; i++)
	{
		if (data[i] != 0 && (i == 0 || data[i - 1] == 0))
		{
			array[num_of_words] = &data[i];
			++num_of_words;
		}
	}

	char res = self->load_from_array(self, num_of_words, array, num_characters);
	free(array);
	free(data);

	return res;
}

static int strCompareForSorting(const void * a, const void * b) {
	const char *pa = *(const char**)a;
	const char *pb = *(const char**)b;
	return strcmp(pa, pb);
}

static char load_from_array(struct Dictionary *self, int num_words, char **words, int total_characters)
{
	release_data(self);

	int *depthAll = malloc(sizeof(int) * (total_characters + 1));
	self->nodes = malloc(sizeof(DictNode) * (total_characters + 1));

	if (!self->nodes || !depthAll)
	{
		free(self);
		self = NULL;
		free(depthAll);
		depthAll = NULL;

		return 0; // false
	}

	qsort(words, num_words, sizeof(char *), strCompareForSorting);

	self->nodes[0].build_node.begin = 0;
	self->nodes[0].build_node.end = num_words;
	depthAll[0] = 0;

	int current_node = 0;
	int next_free_node = 1;

	while (current_node < next_free_node)
	{
		int begin = self->nodes[current_node].build_node.begin;
		int end = self->nodes[current_node].build_node.end;
		int depth = depthAll[current_node];
		int i;

		Node * node = &self->nodes[current_node].node;

		if (current_node > 0)
		{
			node->character = words[begin][depth - 1];
		}

		node->next_begin = &self->nodes[next_free_node];
		node->count_next_nodes = 0;
		node->is_terminal = 0;

		for (i = begin; i < end; i++)
		{
			if (i == begin || words[i - 1][depth] != words[i][depth])
			{
				// next char

				if (words[i][depth] == 0) 
				{
					// mark this word as present in dictionary
					node->is_terminal = 1;
				}
				else
				{
					self->nodes[next_free_node].build_node.begin = i;
					depthAll[next_free_node] = depth + 1;
					next_free_node++;
					node->count_next_nodes++;

					if (node->count_next_nodes > 1)
					{
						self->nodes[next_free_node - 2].build_node.end = i;
					}
				}
			}
		}

		if (node->count_next_nodes > 0)
		{
			self->nodes[next_free_node - 1].build_node.end = end;
		}

		current_node++;
	}
	
	free(depthAll);
	depthAll = NULL;

	return 1; // true
}

static char is_word_present(struct Dictionary *self, const char *word)
{
	DictNode *currentState = self->nodes;
	int current_pos = -1;

	while (1)
	{
		++current_pos;
		char current_char = word[current_pos];

		if (current_char == 0)
		{
			return currentState->node.is_terminal;
		}

		DictNode *nodes = currentState->node.next_begin;
		int begin = 0;
		int end = currentState->node.count_next_nodes;

		if (end == 0) return 0;

		while (begin + 1 != end)
		{
			int center = (begin + end) / 2;
			if (nodes[center].node.character <= current_char)
			{
				begin = center;
			}
			else
			{
				end = center;
			}
		}

		if (nodes[begin].node.character == current_char)
		{
			currentState = &nodes[begin];
		}
		else
		{
			return 0;
		}
	}
}

static void testing(struct Dictionary *self);

Dictionary *Dictionary_New()
{
	Dictionary *self = malloc(sizeof(Dictionary));
	self->load_from_file = load_from_file;
	self->load_from_array = load_from_array;
	self->is_word_present = is_word_present;
	self->testing = testing;
	self->release = release;

	self->nodes = NULL;
	return self;
}

#define TEST_TRUE(x) do { if (x) printf("ok\n"); else printf("fail\n"); } while (0)
#define TEST_FALSE(x) do { if (!(x)) printf("ok\n"); else printf("fail\n"); } while (0)

static void testing(struct Dictionary *self)
{
	{
		Dictionary *d = Dictionary_New();		
		char *data[] = { "test1", "aaa", "test2", "test22", "test222"};

		d->load_from_array(d, sizeof(data) / sizeof(char *), data, 1000000);

		TEST_TRUE(d->is_word_present(d, "test1"));
		TEST_TRUE(d->is_word_present(d, "test22"));
		TEST_TRUE(d->is_word_present(d, "aaa"));

		TEST_FALSE(d->is_word_present(d, "test"));
		TEST_FALSE(d->is_word_present(d, "test3"));
		TEST_FALSE(d->is_word_present(d, ""));
	}
}
