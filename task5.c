#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Dictionary.h"

int main(int argc, char *argv[])
{
	Dictionary *dict = Dictionary_New();

	if (argc != 2)
	{
		printf("usage: \">task5 file\"\n");
		printf("or \">task5 --test\" to run tests\n");
		dict->release(dict);
		return 0;
	}

	if (strcmp(argv[1], "--test") == 0)
	{
		dict->testing(dict);
		dict->release(dict);
		return 0;
	}

	char load_successful = 0;
	char buffer[2 * 1024];

	load_successful = dict->load_from_file(dict, argv[1]);

	if (!load_successful)
	{
		printf("can not read dictionary from file %s", argv[1]);
	}
	else
	{
		while (1)
		{
			char * word = fgets(buffer, sizeof(buffer), stdin);
			int len = strlen(word);

			assert(len > 0); // at least '\n' is present
			
			if (word[len - 1] != '\n')
			{
				printf("we do not support words longer than %lu symbols. sorry about that.\n", sizeof(buffer) - 1);
				continue;
			}

			while (len > 0 && isspace(word[len - 1]))
			{
				len--;
				word[len] = 0;
			}

			if (strcmp(word, "exit") == 0)
			{
				break;
			}

			char is_word_preset = dict->is_word_present(dict, word);
			printf("%s\n", is_word_preset ? "YES" : "NO");
		}
	}
	
	dict->release(dict);
	return 0;
}
