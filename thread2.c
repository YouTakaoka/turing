#include<stdio.h>
#include<string.h>
#include<stdlib.h>

enum Type {
    INT,
    FUNCTION,
    STRING,
    ERROR
};

enum FuncType {
    USER,
    PRESET
};

struct Bind {
    char* name;
    int value;
};

struct Bind new_bind(char name[], int value) {
    struct Bind b = {
        name: name,
        value: value
    };
    return b;
}

struct PresetFunction {
    int argl;
    struct Token (*func)(struct Token[]);
};

struct UserFunction {
    int argl;
    char** args;
    char* expr;
};

struct Function {
    enum FuncType type;
    union Func f;
};

union Func {
    struct UserFunction user;
    struct PresetFunction preset;
};

struct FunctionBind {
    char* name;
    struct UserFunction f;
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

struct Token eval(char* str, struct Bind* binds[], struct FunctionBind* fbinds[]);

struct Stack {
    struct Token data;
    struct Stack* next;
};

struct Token apply(struct Function f, int args[], struct FunctionBind* fbinds[]) {
    if(f.type == PRESET) {
        return f.f.preset.func(args);
    } else {
        int argl = f.f.user.argl;
        struct UserFunction uf = f.f.user;
        struct Bind* binds[argl];
        for(int i=0; i < argl; ++i) {
            struct Bind b = new_bind(uf.args[i], args[i]);
            binds[i] = &b;
        }
        return eval(uf.expr, binds, fbinds);
    }
}

struct Stack* new_stack(struct Token data, struct Stack* next) {
    struct Stack* sp = malloc(sizeof(struct Stack));
    struct Stack s = {
        data: data,
        next: next
    };
    *sp = s;
    return sp;
}

void print_token(struct Token t);

struct Ret {
    struct Function f;
    struct Token args[100];
    int count;
};

struct Token idf(struct Token args[]) {
    return args[0];
}

struct Token add(struct Token args[]) {
    int i1 = args[0].content.i;
    int i2 = args[1].content.i;
    return itot(i1 + i2);
}

struct Token sub(struct Token args[]) {
    int i1 = args[0].content.i;
    int i2 = args[1].content.i;
    return itot(i2 - i1);
}

struct Token prod(struct Token args[]) {
    int i1 = args[0].content.i;
    int i2 = args[1].content.i;
    return itot(i1 * i2);
}

struct Token if_func(struct Token args[]) {
    int i = args[2].content.i;
    return i == 0 ? args[1] : args[0];
}

struct Ret push(struct Token data, struct Bind binds[], struct FunctionBind fbinds[]) {
    char* tp = strtok(NULL, " ");
    struct Bind* b;
    if((b = find_bind(binds, tp)) != NULL) {
        sprintf(tp, "%d", b->value);
    }

    struct FunctionBind* fb;
    if(!strcmp(tp, "!")) {
        struct Function f = data.content.f;
        int argl;
        if(f.type == USER) {
            argl = f.f.user.argl;                
        } else {
            argl = f.f.preset.argl;
        }
        int arguments[argl];
        struct Ret ret = {
            f: f,
            count: argl,
            args: NULL
        };
        return ret;
    } else if((fb = find_fbind(fbinds, tp)) != NULL) {
        struct Token t = uftot(fb->f);
        return push(t, binds, fbinds);
    } else if(!strcmp(tp, "?")) {
        int argl = 0;
        char* args[100];
        while(tp = strtok(NULL, " ")) {
            if(!strcmp(tp, ":")) {
                break;
            }
            args[++argl] = tp;
        }

        char* expr = strtok(NULL, ":");
        struct UserFunction f = {
            argl: argl,
            args: args,
            expr: expr
        };
        struct Token t = uftot(f);
        return push(t, binds, fbinds);
    } else if(!strcmp(tp, "+")) {
        struct PresetFunction pf = {
            argl: 2,
            func: add
        };
        return push(pftot(pf), binds, fbinds);
    } else if(!strcmp(tp, "-")) {
        struct PresetFunction pf = {
            argl: 2,
            func: sub
        };
        return push(pftot(pf), binds, fbinds);
    } else if(!strcmp(tp, "*")) {
        struct PresetFunction pf = {
            argl: 2,
            func: prod
        };
        return push(pftot(pf), binds, fbinds);
    } else if(!strcmp(tp, "if")) {
        struct PresetFunction pf = {
            argl: 3,
            func: if_func
        };
        return push(pftot(pf), binds, fbinds);
    } else {
        struct Token t = itot(atoi(tp));
        push(s, t);
    }

    struct Ret ret = {
        f: idf,
        args: {data},
        count: 0
    };
    return ret;
}

struct Token pop(struct Stack* s) {
    print_token(s->data);
    struct Token data = s->data;
    *s = *s->next;
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

struct Token uftot(struct UserFunction uf) {
    union Func func = {
        user: uf
    };
    struct Function f = {
        type: USER,
        f: func
    };
    return ftot(f);
}

struct Token pftot(struct PresetFunction pf) {
    union Func func = {
        preset: pf
    };
    struct Function f = {
        type: PRESET,
        f: func
    };
    return ftot(f);
}

struct Token eval(char* str, struct Bind* binds[], struct FunctionBind* fbinds[]) {
    char* tp;
    union TokenContent tc = {i: 0};
    struct Token t = {
        type: ERROR,
        content: tc
    };
    
    return pop(s);
}

int main() {
    char str[100];
    struct Bind* binds[] = {NULL};

    char* fact_args[] = {"x"};
    struct UserFunction fact = {
        args: fact_args,
        expr: "x 1 - ? x : 1 : fact if !"
    };
    struct FunctionBind fact_bind = {
        name: "fact",
        f: fact
    };

    char* add_args[] = {"x", "y"};
    struct UserFunction add = {
        args: add_args,
        expr: "x y +"
    };
    struct FunctionBind add_bind = {
        name: "add",
        f: add
    };

    struct FunctionBind* fbinds[] = {NULL, &fact_bind, &add_bind};

    struct Stack* s = new_stack(itot(0), NULL);

    while(fgets(str, sizeof(str), stdin)) {
        str[strlen(str) - 1] = '\0';
        print_token(eval(str, binds, fbinds));
    }

    return 0;
}