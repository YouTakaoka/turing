#include<stdio.h>
#include<string.h>
#include<stdlib.h>

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
    char** args;
    char** expr;
};

struct FunctionBind {
    char* name;
    struct Function f;
};

union TokenContent {
    int i;
    struct Function f;
    char* str;
};

struct Token {
    enum Type type;
    union TokenContent content;
};

struct Token eval(char* tp[], struct Bind* binds[], struct FunctionBind* fbinds[]);

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
    union TokenContent tc = {str: str};
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
    for(int i=0; tp[i] = strtok(i ? NULL : str, " "); i++) {}
}

struct Token eval(char* tp[], struct Bind* binds[], struct FunctionBind* fbinds[]) {
    char* tp_cpy[100];
    memcpy(tp_cpy, tp, sizeof(char*) * 100);

    struct Token t_err = {
        type: ERROR,
        content: {i: 0}
    };
    struct Stack* s = new_stack(NULL, t_err);
    
    for(int i=0; tp_cpy[i] != NULL; i++) {
        struct Bind* b;
        char tk[100];
        strcpy(tk, tp_cpy[i]);
        if((b = find_bind(binds, tk)) != NULL) {
            sprintf(tk, "%d", b->value);
        }
    
        struct FunctionBind* fb;
        if(!strcmp(tk, "!")) {
            struct Function f = pop(s).content.f;
            int args[f.argl];
            for(int i=f.argl-1; i >= 0; --i) {
                args[i] = pop(s).content.i;
            }
            push(s, apply(f, args, fbinds));
        } else if((fb = find_fbind(fbinds, tk)) != NULL) {
            struct Token t = ftot(fb->f);
            push(s, t);
        } else if(!strcmp(tk, "[")) {
            int argl = 0;
            char** args = &tp_cpy[i+1];
            while(strcmp(tp_cpy[++i], ":")) {
                argl++;
            }
    
            char** expr = &tp_cpy[++i];
            while(strcmp(tp_cpy[i], "]")){
                i++;
            }
            tp_cpy[i] = NULL;

            struct Function f = {
                argl: argl,
                args: args,
                expr: expr
            };
            struct Token t = ftot(f);
            push(s, t);
        } else if(!strcmp(tk, "+")) {
            int i1 = pop(s).content.i;
            int i2 = pop(s).content.i;
            struct Token t = itot(i1 + i2);
            push(s, t);
        } else if(!strcmp(tk, "-")) {
            int i1 = pop(s).content.i;
            int i2 = pop(s).content.i;
            struct Token t = itot(i2 - i1);
            push(s, t);
        } else if(!strcmp(tk, "*")) {
            int i1 = pop(s).content.i;
            int i2 = pop(s).content.i;
            struct Token t = itot(i1 * i2);
            push(s, t);
        } else if(!strcmp(tk, "if")) {
            struct Token t1 = pop(s);
            struct Token t2 = pop(s);
            int i = pop(s).content.i;
            struct Token t = i == 0 ? t2 : t1;
            push(s, t);
        } else {
            struct Token t = itot(atoi(tk));
            push(s, t);
        }
    }

    struct Token ret = pop(s);
    free_stack(s);
    return ret;
}

struct Token eval_str(char str[], struct Bind* binds[], struct FunctionBind* fbinds[]) {
    char* tp[100];
    tokenize(str, tp);
    struct Token ret = eval(tp, binds, fbinds);
    return ret;
}

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
        name: name,
        f: f
    };
    struct FunctionBind* fbp = malloc(sizeof(struct FunctionBind));
    *fbp = fbind;
    return fbp;
}

int main() {
    char* fact_args[] = {"x"};
    char* fact_expr[100];
    char fact_str[] = "x 1 - x 1 - [ _ : 1 ] fact if ! x *";
    tokenize(fact_str, fact_expr);
    struct Function fact = {
        argl: 1,
        args: fact_args,
        expr: fact_expr
    };
    struct FunctionBind* fact_bind = new_fbind("fact", fact);

    char* add_args[] = {"x", "y"};
    char* add_expr[100];
    char add_str[] = "x y +";
    tokenize(add_str, add_expr);
    struct Function add = {
        argl: 2,
        args: add_args,
        expr: add_expr
    };
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