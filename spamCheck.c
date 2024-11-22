#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_WORDS 10000
#define MAX_SET 20
#define MAX_LINE_LENGTH 1024

// 구조체 정의
struct Word {
    char word[100];
    int count;
    double p;
    double q;
};

struct Set {
    int num;
    double r;
};

// 함수 선언
void readTrain(const char *filename, struct Word *words, int *count);
void readTest(const char *filename, const struct Word *trainWords, int trainCount, struct Set *set, char type);
void calcProbability(struct Word *spamWords, struct Word *hamWords, int spamCount, int hamCount);
double calculateProbability(const struct Word *trainWords, int trainCount, const struct Word *testWords, int testCount);

// 메인 함수
int main() {
    // 파일 경로 설정
    const char *trainSpamFile = "dataset_spam_train100.csv";
    const char *trainHamFile = "dataset_ham_train100.csv";
    const char *testSpamFile = "dataset_spam_test20.csv";
    const char *testHamFile = "dataset_ham_test20.csv";

    // 데이터 구조 초기화
    struct Word spamWords[MAX_WORDS] = {0};
    struct Word hamWords[MAX_WORDS] = {0};
    struct 
   Set spamRetset[MAX_SET] = {0};
    struct 
   Set hamRetset[MAX_SET] = {0};
    int spamCount = 0, hamCount = 0;

    // 학습 파일 읽기
    readTrain(trainSpamFile, spamWords, &spamCount);
    readTrain(trainHamFile, hamWords, &hamCount);

    // 확률 계산
    calcProbability(spamWords, hamWords, spamCount, hamCount);

    // 테스트 파일 읽고 결과 계산
    readTest(testSpamFile, spamWords, spamCount, spamRetset, 's');
    readTest(testHamFile, hamWords, hamCount, hamRetset, 'h');

    return 0;
}

// 학습 파일 읽기
void readTrain(const char *filename, struct Word *words, int *count) {
    FILE *file = fopen(filename, "r");
    // 파일 확인 
    if (!file) {
        perror("Error opening training file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, sizeof(line), file); // 헤더 스킵

    while (fgets(line, sizeof(line), file)) {
        char temp[MAX_LINE_LENGTH] = {0};
        int index = 0;

        // 소문자 변환 및 알파벳/공백만 유지
        for (int i = 0; i < strlen(line); i++) {
            if (isalpha(line[i])) {
                temp[index++] = tolower(line[i]);
            } else if (line[i] == ' ') {
                temp[index++] = ' ';
            }
        }

        // 단어 토큰화 및 카운트 증가
        char *token = strtok(temp, " ");
        while (token) {
            int found = 0;
            for (int i = 0; i < *count; i++) {
                if (strcmp(words[i].word, token) == 0) {
                    words[i].count++;
                    found = 1;
                    break;
                }
            }
            if (!found && *count < MAX_WORDS) {
                strcpy(words[*count].word, token);
                words[*count].count = 1;
                (*count)++;
            }
            token = strtok(NULL, " ");
        }
    }

    fclose(file);
}

// 확률 계산
void calcProbability(struct Word *spamWords, struct Word *hamWords, int spamCount, int hamCount) {
    for (int i = 0; i < spamCount; i++) {
        int hamMatchCount = 1; // 기본값 1
        for (int j = 0; j < hamCount; j++) {
            if (strcmp(spamWords[i].word, hamWords[j].word) == 0) {
                hamMatchCount = hamWords[j].count;
                break;
            }
        }
        spamWords[i].p = (double)spamWords[i].count / (spamWords[i].count + hamMatchCount);
        spamWords[i].q = 1 - spamWords[i].p;
    }

    for (int i = 0; i < hamCount; i++) {
        int spamMatchCount = 1; // 기본값 1
        for (int j = 0; j < spamCount; j++) {
            if (strcmp(hamWords[i].word, spamWords[j].word) == 0) {
                spamMatchCount = spamWords[j].count;
                break;
            }
        }
        hamWords[i].p = (double)hamWords[i].count / (hamWords[i].count + spamMatchCount);
        hamWords[i].q = 1 - hamWords[i].p;
    }
}

// 테스트 파일 처리
void readTest(const char *filename, const struct Word *trainWords, int trainCount, struct Set *set, char type) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening test file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, sizeof(line), file); // 헤더 스킵

    int setIndex = 0;

    while (fgets(line, sizeof(line), file) && setIndex < MAX_SET) {
        struct Word testWords[MAX_WORDS] = {0};
        int testCount = 0;

        // 문장 처리
        char temp[MAX_LINE_LENGTH] = {0};
        int index = 0;
        for (int i = 0; i < strlen(line); i++) {
            if (isalpha(line[i])) {
                temp[index++] = tolower(line[i]);
            } else if (line[i] == ' ') {
                temp[index++] = ' ';
            }
        }

        // 단어 추출
        char *token = strtok(temp, " ");
        while (token) {
            int found = 0;
            for (int i = 0; i < testCount; i++) {
                if (strcmp(testWords[i].word, token) == 0) {
                    testWords[i].count++;
                    found = 1;
                    break;
                }
            }
            if (!found && testCount < MAX_WORDS) {
                strcpy(testWords[testCount].word, token);
                testWords[testCount].count = 1;
                testCount++;
            }
            token = strtok(NULL, " ");
        }

        // 확률 계산
        double pprod = calculateProbability(trainWords, trainCount, testWords, testCount);
        double qprod = 1.0 - pprod;
        set[setIndex].r = pprod / (pprod + qprod);
        set[setIndex].num = setIndex + 1;
        setIndex++;
    }

    // 결과 출력
    printf("----------- %s -----------\n", (type == 's') ? "Spam" : "Ham");
    for (int i = 0; i < setIndex; i++) {
        printf("%c - %02d : %.3f\n", type, set[i].num, set[i].r);
    }

    fclose(file);
}

// 확률 계산 보조 함수
double calculateProbability(const struct Word *trainWords, int trainCount, const struct Word *testWords, int testCount) {
    double pprod = 1.0, qprod = 1.0;
    for (int i = 0; i < testCount; i++) {
        for (int j = 0; j < trainCount; j++) {
            if (strcmp(trainWords[j].word, testWords[i].word) == 0) {
                pprod *= trainWords[j].p;
                qprod *= trainWords[j].q;
                break;
            }
        }
    }
    return (pprod > 0) ? pprod : 0.0;
}
