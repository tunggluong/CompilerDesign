#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORDS           5000       // số mục từ tối đa
#define MAX_WORD_LEN        64
#define MAX_LINES_PER_WORD  200
#define MAX_STOPWORDS       1000
#define MAX_LINE_LEN        4096

typedef struct {
    char word[MAX_WORD_LEN];
    int  count;
    int  lines[MAX_LINES_PER_WORD];
    int  line_count;
} IndexEntry;

/* ==================== STOPWORD ==================== */

int load_stopwords(const char *filename,
                   char stopwords[][MAX_WORD_LEN],
                   int max_stopwords) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Khong mo duoc file stopword");
        return -1;
    }

    int n = 0;
    char buf[MAX_WORD_LEN];

    while (n < max_stopwords && fscanf(f, "%63s", buf) == 1) {
        int i;
        for (i = 0; buf[i] != 0 && i < MAX_WORD_LEN - 1; i++) {
            stopwords[n][i] = (char)tolower((unsigned char)buf[i]);
        }
        stopwords[n][i] = 0;
        n++;
    }

    fclose(f);
    return n;
}

int is_stopword(const char *word,
                char stopwords[][MAX_WORD_LEN],
                int stop_count) {
    for (int i = 0; i < stop_count; i++) {
        if (strcmp(word, stopwords[i]) == 0)
            return 1;
    }
    return 0;
}

/* ==================== INDEXING ==================== */

int find_word(IndexEntry *index, int n, const char *word) {
    for (int i = 0; i < n; i++) {
        if (strcmp(index[i].word, word) == 0)
            return i;
    }
    return -1;
}

int compare_index_entries(const void *a, const void *b) {
    const IndexEntry *ia = (const IndexEntry *)a;
    const IndexEntry *ib = (const IndexEntry *)b;
    return strcmp(ia->word, ib->word);
}

/* ==================== MAIN ==================== */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Cach dung:\n");
        printf("   %s <ten_file_vanban> <ten_file_stopword>\n", argv[0]);
        return 1;
    }

    const char *text_file = argv[1];
    const char *stopword_file = argv[2];

    char stopwords[MAX_STOPWORDS][MAX_WORD_LEN];
    int stop_count = load_stopwords(stopword_file, stopwords, MAX_STOPWORDS);
    if (stop_count < 0) return 1;

    FILE *fp = fopen(text_file, "r");
    if (!fp) {
        perror("Khong mo duoc file van ban");
        return 1;
    }

    IndexEntry *index = (IndexEntry *)malloc(MAX_WORDS * sizeof(IndexEntry));
    if (!index) {
        fprintf(stderr, "Khong cap phat duoc bo nho\n");
        fclose(fp);
        return 1;
    }

    int word_count = 0;
    int line_number = 0;
    char linebuf[MAX_LINE_LEN];

    while (fgets(linebuf, sizeof(linebuf), fp)) {
        line_number++;
        int len = strlen(linebuf);

        int i = 0;
        while (i < len) {
            while (i < len && !isalpha((unsigned char)linebuf[i])) i++;
            if (i >= len) break;

            int start = i;
            char first_char = linebuf[i];

            char prev_non_space = '.';
            for (int k = start - 1; k >= 0; k--) {
                if (!isspace((unsigned char)linebuf[k])) {
                    prev_non_space = linebuf[k];
                    break;
                }
            }

            while (i < len && isalpha((unsigned char)linebuf[i])) i++;
            int end = i;

            int wlen = end - start;
            if (wlen >= MAX_WORD_LEN) wlen = MAX_WORD_LEN - 1;

            char wordbuf[MAX_WORD_LEN];
            for (int j = 0; j < wlen; j++) {
                wordbuf[j] = linebuf[start + j];
            }
            wordbuf[wlen] = 0;

            int is_proper_noun =
                isupper((unsigned char)first_char) &&
                (prev_non_space != '.');

            char lower_word[MAX_WORD_LEN];
            for (int j = 0; j < wlen; j++) {
                lower_word[j] =
                    (char)tolower((unsigned char)wordbuf[j]);
            }
            lower_word[wlen] = 0;

            if (!is_proper_noun &&
                !is_stopword(lower_word, stopwords, stop_count) &&
                lower_word[0] != 0) {

                if (word_count >= MAX_WORDS) {
                    fprintf(stderr, "Vuot MAX_WORDS\n");
                    free(index);
                    fclose(fp);
                    return 1;
                }

                int idx = find_word(index, word_count, lower_word);
                if (idx == -1) {
                    strcpy(index[word_count].word, lower_word);
                    index[word_count].count = 1;
                    index[word_count].lines[0] = line_number;
                    index[word_count].line_count = 1;
                    word_count++;
                } else {
                    IndexEntry *e = &index[idx];
                    e->count++;
                    if (e->lines[e->line_count - 1] != line_number &&
                        e->line_count < MAX_LINES_PER_WORD) {
                        e->lines[e->line_count++] = line_number;
                    }
                }
            }
        }
    }

    fclose(fp);

    qsort(index, word_count, sizeof(IndexEntry), compare_index_entries);

    for (int i = 0; i < word_count; i++) {
        IndexEntry *e = &index[i];
        printf("%-15s%d", e->word, e->count);
        for (int j = 0; j < e->line_count; j++) {
            printf(", %d", e->lines[j]);
        }
        printf("\n");
    }

    free(index);
    return 0;
}
