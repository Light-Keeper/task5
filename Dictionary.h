#pragma once

struct DictNode;

typedef struct Dictionary
{
//public:
	char(*load_from_file) (struct Dictionary *self, const char *file);
	char(*load_from_array)(struct Dictionary *self, int num_words, char **words, int total_characters);

	char(*is_word_present)(struct Dictionary *self, const char *word);
	void(*testing) (struct Dictionary *self);

	void(*release)(struct Dictionary *self);

//pritate:
	struct DictNode *nodes;
} Dictionary;

Dictionary *Dictionary_New();

