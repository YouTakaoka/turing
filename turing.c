#include<stdio.h>
#include<string.h>

const char END_OF_STACK = '$';
const char PRE_START_STATE = '?';
const char INITIAL_STATE = '!';
const char BLANK = ' ';
char* final_states = "01";
// テープは標準入力，途中に一個だけ初期状態を含む文字列．初期状態の右側にある文字から始める
// 遷移関数はファイルから読み込む
// コマンドライン引数は file, final_states
int main(int argc, char *argv[]) {
    char halt_q = push_r(PRE_START_STATE, END_OF_STACK, END_OF_STACK);
    printf("%c", halt_q);
}

// 左に動くとき:
// 現在の値をスタックRにPUSHし，
// スタックLからPOPする（空ならBLANK）

// 右に動くとき:
// 現在の値をスタックLにPUSHし，
// スタックRからPOPする（空ならバッファから読み込む）

char get_input() {
    char c = getchar();
    if(c == EOF || c == NULL) {
        return BLANK;
    } else {
        return c;
    }
}

char next_state(char q, char c) {

}

char symbol_to_write(char q, char c) {

}

// 0ならleft, 1ならright
int left_or_right(char q, char c) {

}

int is_final_state(char q) {
    return strchr(final_states, (int)q) != NULL;
}

char get_l(char l) {
    if(l == END_OF_STACK) {
        return BLANK;
    } else {
        return l;
    }
}

char get_r(char r) {
    if(r == END_OF_STACK) {
        return get_input();
    } else {
        return r;
    }
}

char push_l(char q, char l, char r) {
    if(q == PRE_START_STATE) {
        if(r != INITIAL_STATE) {
            char c = get_r(r);
            return push_l(PRE_START_STATE, c, END_OF_STACK);
        }
    }

    q = next_state(q, r);
    char c = symbol_to_write(q, r);
    if(left_or_right(q, r)) {
        // right
        return push_l(q, c, );

    }
}

char push_r(char q, char l, char r) {
    if(q == PRE_START_STATE) {
        char c = get_input();
        return push_l(PRE_START_STATE, c, END_OF_STACK);
    }
}
