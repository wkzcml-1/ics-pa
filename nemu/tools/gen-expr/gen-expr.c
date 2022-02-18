#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include <stdbool.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


// declaration
static void gen_rand_op(bool not_minus);

/* global variables */
int now_pos = 0;
char num_buf[10];
const int MAX_BUF = 65500;
const int MAX_NUM = 10000;

/* generate character */
static void gen(char c) {
  buf[now_pos++] = c;
  buf[now_pos] = '\0';
}

/* generate num */
static void gen_num(bool not_zero) {
  // get random num
  sprintf(num_buf, "%d", rand() % MAX_NUM + not_zero);

  int index = 0;

  while (num_buf[index] != '\0') {
    buf[now_pos++] = num_buf[index++];
    if (now_pos >= MAX_BUF) {
      buf[now_pos] = '\0';
      return;
    }
  }

  buf[now_pos] = '\0';
}

/* generate space */
static void gen_space() {
  if (rand() % 2) {
    buf[now_pos++] = ' ';
    buf[now_pos] = '\0';
  }
}

/* generate not zero expr */
static void gen_not0_expr() {
   // buf overflow
  if (now_pos >= MAX_BUF) {
    buf[now_pos++] = 1;
    buf[now_pos] = '\0';
  }
  /* Get a random number for 
    the corresponding operation */
  switch (rand() % 4) {
    case 0: 
      gen_num(true);
      break;
    case 1:
      gen('(');
      gen_not0_expr();
      gen(')');
      break;
    case 2:
      gen_space();
      gen_not0_expr();
      gen_space();
      break;
    default:
      gen_not0_expr();
      gen_space();
      gen_rand_op(true);
      break;
  }
}

static void gen_rand_expr() {
  // buf overflow
  if (now_pos >= MAX_BUF) {
    buf[now_pos++] = 1;
    buf[now_pos] = '\0';
  }
  /* Get a random number for 
    the corresponding operation */
  switch (rand() % 4) {
    case 0: 
      gen_num(false);
      break;
    case 1:
      gen('(');
      gen_rand_expr();
      gen(')');
      break;
    case 2:
      gen_space();
      gen_rand_expr();
      gen_space();
      break;
    default:
      gen_rand_expr();
      gen_space();
      gen_rand_op(false);
      break;
  }
}

/* generate opertions */
// + - * /
static void gen_rand_op(bool not_minus) {
  char c = '\0';
  bool not_zero = false;
  switch (rand() % 4) {
    case 0: c = '-'; 
      if(not_minus) c = '*';
      break;
    case 1: c = '+'; break;
    case 2: c = '*'; break;
    default: c = '/'; 
      not_zero = true;
  }
  buf[now_pos++] = c;
  if (not_zero) {
    gen_not0_expr();
  } else {
    gen_rand_expr();
  }
}


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    now_pos = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    if (fscanf(fp, "%d", &result));
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
