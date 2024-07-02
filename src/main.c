/*
** JPAT C
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <time.h>

#include "tables.h"

/*
** Macros
*/

static table_t  g_tables[12];
static uint32_t g_tables_count;

static uint32_t g_alphabets           = ALPHABET_NONE;
static uint32_t g_questions           = 7;
static uint32_t g_force_accuracy      = 1;
static uint32_t g_show_translation    = 1;
static uint32_t g_show_incorrect      = 1;

static uint32_t g_current = 0, g_last = 0;
static uint32_t g_inputs, g_incorrect;

/*
** Commands
*/

void arg_t(void)
{
  g_show_translation = !g_show_translation;
}

void arg_fa(void)
{
  g_force_accuracy = !g_force_accuracy;
}

void arg_l(void)
{
  g_show_incorrect = !g_show_incorrect;
}

void arg_a(const char *arg)
{
  uint32_t option = ALPHABET_NONE;

  switch (arg[0])
  {
  case 'k': option = ALPHABET_KATAKANA; break;
  case 'h': option = ALPHABET_HIRAGANA; break;
  case 'c': option = ALPHABET_CYRILLIC; break;
  }

  g_alphabets |= option;
}

void arg_q(const char *arg)
{
  g_questions = strtol(arg, NULL, 10);
}

/*
** Entry
*/

void setup_tables()
{
  if (g_alphabets & ALPHABET_HIRAGANA)
  {
    table_t table_hiragana = { JP1_ELEMENTS, &hiragana[0], &jp1_translations[0] };
    g_tables[g_tables_count++] = table_hiragana;
  }

  if (g_alphabets & ALPHABET_KATAKANA)
  {
    table_t table_katakana = { JP1_ELEMENTS, &katakana[0], &jp1_translations[0] };
    g_tables[g_tables_count++] = table_katakana;
  }

  if (g_alphabets & ALPHABET_CYRILLIC)
  {
    table_t table_cyrillic = { CY_ELEMENTS, &cyrillic[0], &cy_translations[0] };
    g_tables[g_tables_count++] = table_cyrillic;
  }
}

const wchar_t *show_random()
{
  table_t *table;
  uint32_t index;
  wchar_t c;

  index = rand() % g_tables_count;
  table = &g_tables[index];

  /* pick one that hasn't yet been chosen */
  do
  {
    g_current = rand() % table->elements;
  } while(g_current == g_last);
  g_last = g_current;

  c = table->alphabet[g_current];
  wprintf(L"%lc", c);

  if (g_show_translation)
    wprintf(L" => %ls", table->translation[g_current]);

  putwchar(L'\n');
  return table->translation[g_current];
}

void show_incorrect(const wchar_t *t, const wchar_t *i)
{
  wprintf(L"Incorrect ");

  if (g_show_incorrect)
    wprintf(L"[%ls => %ls]", i, t);

  putwchar(L'\n');
}

void show_results()
{
  float score = (((float) g_inputs - (float) g_incorrect) / ((float) g_inputs));

  wprintf(L"\n--------------------------"
          L"\nQuestions\t%d"
          L"\nMistakes\t%d"
          L"\nScore\t\t%.1f"
          L"\n--------------------------\n",
          g_inputs, g_incorrect, score * 100.0f);
}

int main(int argc, char *argv[])
{
  uint32_t i, q;
  wchar_t buffer[64], *input;

  for (i = 1; i < argc; i++)
  {
    if (!strncmp(argv[i], "-t", 2))
      arg_t();
  
    if (!strncmp(argv[i], "-fa", 3))
      arg_fa();

    if (!strncmp(argv[i], "-l", 2))
      arg_l();
    
    if (!strncmp(argv[i], "-a", 2))
      arg_a(argv[++i]);

    if (!strncmp(argv[i], "-q", 2))
      arg_q(argv[++i]);
  }


  srand(time(NULL));
  setlocale(LC_CTYPE, "");
  setup_tables();

  for(q = 0; q < g_questions; q++)
  {
    const wchar_t *eng = show_random();

retry:
    input = &buffer[0];
    wprintf(L" > ");
    wscanf(L"%64ls", input);
    g_inputs++;

    for (; *input; ++input)
      *input = towlower(*input);

    if(wcscmp(eng, &buffer[0]))
    {
      show_incorrect(eng, &buffer[0]);
      g_incorrect++;

      if(g_force_accuracy)
        goto retry;
    }

    putwchar('\n');
  }

  show_results();
  return 0;
}
