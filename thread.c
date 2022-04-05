#include<stdio.h>
#include<string.h>
#include<stdlib.h>

const char END_OF_EXPRESSION[] = "";

enum Type {
    INT,
    FUNCTION,
    STRING,
    ERROR
};

struct Bind {
    char* name;
    int value;
};

struct Bind* new_bind(char name[], int value) {
    struct Bind b = {
        name: name,
        value: value
    };
    struct Bind* bp = malloc(sizeof(struct Bind));
    *bp = b;
    return bp;
}

struct Function {
    int argl;
    char args[100][100];
    char expr[100][100];
};

struct FunctionBind {
    char name[100];
    struct Function f;
};

void free_bind(struct Bind* binds[]) {
    for(int i=0; binds[i] != NULL; i++) {
        free(binds[i]);
    }
}

void free_fbind(struct FunctionBind* fbinds[]) {
    for(int i=0; fbinds[i] != NULL; i++) {
        free(fbinds[i]);
    }
}

struct FunctionBind* new_fbind(char* name, struct Function f) {
    struct FunctionBind fbind = {
        name: "",
        f: f
    };
    strcpy(fbind.name, name);
    struct FunctionBind* fbp = malloc(sizeof(struct FunctionBind));
    *fbp = fbind;
    return fbp;
}

void append_fbind(struct FunctionBind* fbinds[], struct FunctionBind* fb) {
    int l = 0;
    while(fbinds[l] != NULL) {
        l++;
    }
    fbinds[l] = fb;
    fbinds[l + 1] = NULL;
}

union TokenContent {
    int i;
    struct Function f;
    char str[100];
};

struct Token {
    enum Type type;
    union TokenContent content;
};

struct Token eval(char tp[][100], struct Bind* binds[], struct FunctionBind* fbinds[]);

struct Stack {
    struct Token data;
    struct Stack* next;
};

struct Token apply(struct Function f, int args[], struct FunctionBind* fbinds[]) {
    struct Bind* binds[f.argl + 1];
    for(int i=0; i < f.argl; i++) {
        struct Bind* b = new_bind(f.args[i], args[i]);
        binds[i] = b;
    }
    binds[f.argl] = NULL;
    return eval(f.expr, binds, fbinds);
}

void print_token(struct Token t);

struct Stack* new_stack(struct Stack* next, struct Token data) {
    struct Stack* sp = malloc(sizeof(struct Stack));
    struct Stack s = {
        data: data,
        next: next
    };
    *sp = s;
    return sp;
}

void push(struct Stack* s, struct Token data) {
    struct Stack* s1 = new_stack(s->next, s->data);
    struct Stack tmp = {data: data, next: s1};
    *s = tmp;
}

struct Token pop(struct Stack* s) {
    struct Token data = s->data;
    struct Stack* next = s->next;
    *s = *s->next;
    free(next);
    return data;
}

void free_stack(struct Stack* s) {
    while(s != NULL) {
        struct Stack* tmp = s->next;
        free(s);
        s = tmp;
    }
}

// bindsの最後にはNULLが入っている前提．
// 識別子にnameを持つBindを探し，そのポインタを返す
// 見つからなければNULLを返す
struct Bind* find_bind(struct Bind* binds[], char name[]) {
    for(int i=0; binds[i] != NULL; ++i) {
        if(!strcmp(binds[i]->name, name)) {
            return binds[i];
        }
    }
    return NULL;
}

// bindsの最後にはNULLが入っている前提．
// 識別子にnameを持つFunctionBindを探し，そのポインタを返す
// 見つからなければNULLを返す
struct FunctionBind* find_fbind(struct FunctionBind* fbinds[], char name[]) {
    for(int i=0; fbinds[i] != NULL; ++i) {
        if(!strcmp(fbinds[i]->name, name)) {
            return fbinds[i];
        }
    }
    return NULL;
}

struct Token itot(int i) {
    union TokenContent tc = {i: i};
    struct Token t = {
        type: INT,
        content: tc
    };
    return t;
}

struct Token stot(char* str) {
    union TokenContent tc;
    strcpy(tc.str, str);
    struct Token t = {
        type: STRING,
        content: tc
    };
    return t;
}

struct Token ftot(struct Function f) {
    union TokenContent tc = {f: f};
    struct Token t = {
        type: FUNCTION,
        content: tc
    };
    return t;
}

// トークンを整数に変換
int ttoi(struct Token t) {
    if(t.type == STRING) {
        return atoi(t.content.str);
    } else {
        return t.content.i;
    }
}

void print_token(struct Token t) {
    switch(t.type) {
        case ERROR:
            printf("ERROR\n");
            break;
        case STRING:
            printf("%s\n", t.content.str);
            break;
        case INT:
            printf("%d\n", t.content.i);
            break;
        case FUNCTION:
            printf("FUNCTION\n");
            break;
    }
}

void tokenize(char str[], char* tp[]) {
    int l = 0;
    while(tp[l] = strtok(l ? NULL : str, " ")) {
        l++;
    }
}

void strarrcpy(char dest[][100], char* src[], int len) {
    int l = 0;
    while(src[l] != NULL && l < len) {
        strcpy(dest[l], src[l]);
        l++;
    }
    strcpy(dest[l], END_OF_EXPRESSION);
}

struct Token eval(char tp[][100], struct Bind* binds[], struct FunctionBind* fbinds[]) {
    struct Token t_err = {
        type: ERROR,
        content: {i: 0}
    };
    struct Stack* s = new_stack(NULL, t_err);
    
    for(int i=0; strcmp(tp[i], END_OF_EXPRESSION); i++) {
        struct Bind* b;
        if((b = find_bind(binds, tp[i])) != NULL) {
            sprintf(tp[i], "%d", b->value);
        }
    
        struct FunctionBind* fb;
        if(!strcmp(tp[i], "!")) {
            struct Function f = pop(s).content.f;
            int args[f.argl];
            for(int i=f.argl-1; i >= 0; --i) {
                args[i] = ttoi(pop(s));
            }
            push(s, apply(f, args, fbinds));
        } else if(!strcmp(tp[i], "?")) {
            char* name = pop(s).content.str;
            struct Function f = pop(s).content.f;
            struct FunctionBind* fb = new_fbind(name, f);
            append_fbind(fbinds, fb);
            push(s, ftot(f));
        } else if((fb = find_fbind(fbinds, tp[i])) != NULL) {
            struct Token t = ftot(fb->f);
            push(s, t);
        } else if(!strcmp(tp[i], "[")) {
            int argl = 0;
            struct Function f;

            while(strcmp(tp[++i], ":")) {
                strcpy(f.args[argl++], tp[i]);
            }
            f.argl = argl;
    
            int expr_l = 0;
            while(strcmp(tp[++i], "]")){
                strcpy(f.expr[expr_l++], tp[i]);
            }
            strcpy(f.expr[expr_l], END_OF_EXPRESSION);
            
            struct Token t = ftot(f);
            push(s, t);
        } else if(!strcmp(tp[i], "+")) {
            int i1 = ttoi(pop(s));
            int i2 = ttoi(pop(s));
            struct Token t = itot(i1 + i2);
            push(s, t);
        } else if(!strcmp(tp[i], "-")) {
            int i1 = ttoi(pop(s));
            int i2 = ttoi(pop(s));
            struct Token t = itot(i2 - i1);
            push(s, t);
        } else if(!strcmp(tp[i], "*")) {
            int i1 = ttoi(pop(s));
            int i2 = ttoi(pop(s));
            struct Token t = itot(i1 * i2);
            push(s, t);
        } else if(!strcmp(tp[i], "if")) {
            struct Token t1 = pop(s);
            struct Token t2 = pop(s);
            int i = ttoi(pop(s));
            struct Token t = i == 0 ? t2 : t1;
            push(s, t);
        } else {
            struct Token t = stot(tp[i]);
            push(s, t);
        }
    }

    struct Token ret = pop(s);
    free_stack(s);
    return ret;
}

struct Token eval_str(char str[], struct Bind* binds[], struct FunctionBind* fbinds[]) {
    char* tp[100];
    char tp_cpy[100][100];
    tokenize(str, tp);
    strarrcpy(tp_cpy, tp, 100);
    struct Token ret = eval(tp_cpy, binds, fbinds);
    return ret;
}

int main() {
    char* fact_args[] = {"x"};
    char* fact_expr[100];
    char fact_str[] = "x 1 - x 1 - [ _ : 1 ] fact if ! x *";
    tokenize(fact_str, fact_expr);
    struct Function fact = {
        argl: 1
    };
    strarrcpy(fact.args, fact_args, fact.argl);
    strarrcpy(fact.expr, fact_expr, 100);
    struct FunctionBind* fact_bind = new_fbind("fact", fact);

    char* add_args[] = {"x", "y"};
    char* add_expr[100];
    char add_str[] = "x y +";
    tokenize(add_str, add_expr);
    struct Function add = {
        argl: 2
    };
    strarrcpy(add.args, add_args, add.argl);
    strarrcpy(add.expr, add_expr, 100);
    struct FunctionBind* add_bind = new_fbind("add", add);

    struct FunctionBind* fbinds[] = {fact_bind, add_bind, NULL};

    char str[100];
    struct Bind* binds[] = {NULL};
    while(fgets(str, sizeof(str), stdin)) {
        str[strlen(str) - 1] = '\0';
        print_token(eval_str(str, binds, fbinds));
    }

    free_bind(binds);
    free_fbind(fbinds);
    return 0;
}