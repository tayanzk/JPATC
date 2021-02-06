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
#include "cmd.h"

/* wprintf, wscanf */

extern int wprintf(const wchar_t *__restrict __format, ...);
extern int wscanf(const wchar_t *__restrict __format, ...);

/*
** Macros
*/

static table_t g_tables[12];
static uint32_t g_tables_count;

static uint32_t g_alphabets           = kAlphabet_Cyrillic;
static uint32_t g_questions           = 7;
static uint32_t g_force_accuracy      = 1;
static uint32_t g_show_translation    = 1;
static uint32_t g_show_incorrect      = 1;

static uint32_t g_current = 0, g_last = 0;
static uint32_t g_inputs, g_incorrect;

/*
** Commands
*/

void arg_cmd_qe(int c, char **v) {
  char *buffer = NULL;
  
  assert(c == 2);
  g_questions = strtol(v[1], &buffer, 10);
}

void arg_cmd_fa(int c, char **v) {
  ((void)c, (void)v);
  g_force_accuracy = 1;
}

void arg_cmd_st(int c, char **v) {
  ((void)c, (void)v);
  g_show_translation = 1;
}

void arg_cmd_jp(int c, char **v) {
  int i, option;
  
  (void)v;
  assert(c >= 2);

  for(i = 0; i < c; i++) {
    option = 1;
    g_alphabets |= option;
  }
}

/*
** Entry
*/

void setup_tables() {
  if(g_alphabets & kAlphabet_Hiragana) {
    table_t hiragana_T = { JP1_ELEMENTS, &hiragana[0], &jp1_translations[0] };
    g_tables[g_tables_count++] = hiragana_T;
  }

  if(g_alphabets & kAlphabet_Katakana) {
    table_t katakana_T = { JP1_ELEMENTS, &katakana[0], &jp1_translations[0] };
    g_tables[g_tables_count++] = katakana_T;
  }

  if(g_alphabets & kAlphabet_Cyrillic) {
    table_t cyrillic_T = { CY_ELEMENTS, &cyrillic[0], &cy_translations[0] };
    g_tables[g_tables_count++] = cyrillic_T;
  }
}

const wchar_t *show_random() {
  table_t *table;
  uint32_t index;
  wchar_t c;

  index = rand() % g_tables_count;
  table = &g_tables[index];

  /* pick one that hasn't yet been chosen */
  do {
    g_current = rand() % table->elements;
  } while(g_current == g_last);
  g_last = g_current;

  c = table->alphabet[g_current];
  wprintf(L"%lc", c);

  if(g_show_translation) {
    wprintf(L" => %ls", table->translation[g_current]);
  }

  putwchar(L'\n');
  return table->translation[g_current];
}

void show_incorrect(const wchar_t *t, const wchar_t *i) {
  wprintf(L"Incorrect ");

  if(g_show_incorrect) {
    wprintf(L"[%ls => %ls]", i, t);
  }

  putwchar(L'\n');
}

void show_results() {
  float score = (((float)g_inputs - (float)g_incorrect) / ((float)g_inputs));

  wprintf(L"\n--------------------------\nQuestions\t%d\nMistakes\t%d\nScore\t\t%.1f\n--------------------------\n",
    g_inputs, g_incorrect, score * 100.0f);
}

int main(int argc, char *argv[]) {
  uint32_t q;
  wchar_t buffer[64], *input;

  /* 
  ** register_cmd("-fa", arg_cmd_fa);
  ** register_cmd("-st", arg_cmd_st);
  ** register_cmd("-jp", arg_cmd_jp);
  ** call_cmd(argc, argv);
  */
  ((void)argc, (void)argv);

  srand(time(NULL));
  setlocale(LC_CTYPE, "");
  setup_tables();

  for(q = 0; q < g_questions; q++) {
    const wchar_t *eng = show_random();

retry:
    input = &buffer[0];
    wprintf(L" > ");
    wscanf(L"%ls", input);
    g_inputs++;

    for (; *input; ++input)
      *input = towlower(*input);

    if(wcscmp(eng, &buffer[0])) {
      show_incorrect(eng, &buffer[0]);
      g_incorrect++;

      if(g_force_accuracy) {
        goto retry;
      }
    }

    putwchar('\n');
  }

  show_results();
  return 0;
}