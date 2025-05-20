#include "compiler_globals.h"
#include "parser_semantic_programs.h"
struct Variable * VariableTable;
int vars = 0;
int tables = 1;

void VariableTable_init(){
    VariableTable = (struct Variable *)malloc(sizeof(struct Variable) * MAX_ID_PER_TABLE);
    struct Variable* v;
    for(int i = 0; i < MAX_ID_PER_TABLE; i++){
        v = &VariableTable[i];
        v->name = NULL;
        v->address = NULL;
        v->type = -1;
    }
}

void VariableTable_extend(){
    struct Variable* VT = VariableTable;
    for(int i = tables - 1; i > 0; i--){
        VT = VT[MAX_ID_PER_TABLE - 1].address;
    }
    VT[MAX_ID_PER_TABLE - 1].type = -2;
    VT[MAX_ID_PER_TABLE - 1].address = (struct Variable *)malloc(sizeof(struct Variable) * MAX_ID_PER_TABLE);
    VT = VT[MAX_ID_PER_TABLE - 1].address;

    struct Variable* v;
    for(int i = 0; i < MAX_ID_PER_TABLE; i++){
        v = &VT[i];
        v->name = NULL;
        v->address = NULL;
        v->type = -1;
    }
    tables++;
}

void VariableTable_print(){
    struct Variable* VT = VariableTable;
    struct Variable v;
    int count = 1;
    for(int i = 0; i < MAX_ID_PER_TABLE; i++){
        v = VT[i];
        if(v.type == -2){
            VT = v.address;
            i = -1;
            continue;
        }
        printf("%d: ", count);
        count++;
        if(v.name == NULL){
            printf("%s ", "NULL");
        }
        else{
            printf("%s ", v.name);
        }
        if(v.address == NULL){
            printf("%s ", "NULL");
        }
        else{
            printf("%p ", v.address);
        }
        printf("%d\n", v.type);
    }
}

int VariableTable_add(char* name, void* address, int type){
    if(VariableTable_find(name) == NULL){
        if((vars + 1) >= tables*MAX_ID_PER_TABLE){
            VariableTable_extend();
        }

        struct Variable* v = VariableTable_at(vars);
        v->name = strdup(name);
        v->address = address;
        v->type = type;

        vars++;
        return 1;
    }
    err_no = ERR_EXISTING_ID;
    err_throw();
    return 0;
}

struct Variable * VariableTable_at(int ind){
    int table_ind = ind / (MAX_ID_PER_TABLE-1);
    struct Variable* VT = VariableTable;
    struct Variable* v;
    while(table_ind > 0){
        VT = VariableTable[MAX_ID_PER_TABLE - 1].address;
        table_ind--;
    }
    v = &VT[ind % (MAX_ID_PER_TABLE - 1)];
    return v;
}

struct Variable* VariableTable_find(char* name){
    struct Variable* VT = VariableTable;
    struct Variable* v;
    for(int i = 0; i < MAX_ID_PER_TABLE && VT[i].type != -1; i++){
        v = &VT[i];
        if(v->type == -2){
            VT = v->address;
            i = -1;
            continue;
        }
        if(!strcmp(name, v->name)){
            return v;
        }
    }
    return NULL;
};
