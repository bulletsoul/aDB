#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <vector>
#include <map>
#include <iostream>

/*
Authors: 
	 
Last Edited:

*/

#define MAXWORD 500 //maximum word size

using namespace std;

typedef enum{
    tokSELECT,tokFROM,tokWHERE,tokIS,tokNOT,tokBETWEEN,tokAND,tokINSERT,
    tokINTO,tokVALUES,tokDELETE, //reserved words
    tokADD,tokSUB,tokMULT,tokDIV, //arithmetic operators
    tokLPAR,tokRPAR,tokCOMMA,tokQUOTE, //delimeter
    tokATTR,
    NAtk,
    tokEND,tokTEXT,tokLET,tokVAL,tokEXPUNC,tokSEMICOL
    

}TokenType;

typedef struct tokenTag{
    char str[MAXWORD];
    TokenType type;
    int pos; //line number
}Token;

typedef enum{
    selectNode,attrNode,attr2Node,whereNode,tblNode
}NodeType;

typedef struct nodeTag{
    NodeType type;
    Token token;
    struct nodeTag *child1;
    struct nodeTag *child2;
    struct nodeTag *child3;
}Node;

char arithOperator[]={'*','/','+','-'};
char delimeter[]={'(',')',',','"','=',';'};
map<string,TokenType> reservedWord;
string code;
int offset=0,lineNum=0,tokcount=0;
Token *toks=NULL;
Token tk = {"N/A",NAtk,0};

void make_tokenmap(){
    reservedWord["SELECT"]=tokSELECT;
    reservedWord["FROM"]=tokFROM;
    reservedWord["WHERE"]=tokWHERE;
    reservedWord["IS"]=tokIS;
    reservedWord["NOT"]=tokNOT;
    reservedWord["BETWEEN"]=tokBETWEEN;
    reservedWord["AND"]=tokAND;
    reservedWord["INSERT"]=tokINSERT;
    reservedWord["INTO"]=tokINTO;
    reservedWord["VALUES"]=tokVALUES;
    reservedWord["DELETE"]=tokDELETE;

}

int isReservedChar(char c){
    if (c=='(' || c==')' || c==',' || c=='!' || c=='"' || c=='+' || c=='-' || 
	c=='*' || c=='/' || c=='%' || c=='=' || c=='<' || c=='>'){
	return 1;
    }else return 0;
}

int isDelimeter(char c){
    if (c=='(' || c==')' || c==',' || c=='"' || c==';'){
	return 1;
    }else return 0;
}

TokenType getDelimeterType(char c){
    if(c=='(') return tokLPAR;
    if(c==')') return tokRPAR;
    if(c==',') return tokCOMMA;
    if(c=='"') return tokQUOTE;
    if(c==';') return tokSEMICOL;
}

int isArithOperator(char c){  //char parameter
    if (c=='+' || c=='-' || c=='*' || c=='/' || c=='%')
	return 1;
    else return 0;
}

int isArithmeticOperator(string c){ //string parameter
    if (c=="+" || c=="-" || c=="*" || c=="/" || c=="%")
    return 1;
    else return 0;
}

TokenType getArithType(char c){
    if(c=='+') return tokADD;
    if(c=='-') return tokSUB;
    if(c=='*') return tokMULT;
    if(c=='/') return tokDIV;
    //if(c=='%') return tokMOD;
}

int isStmt(TokenType type){
   if(type==tokSELECT || type==tokINSERT || type==tokDELETE)
	return 1;
   else return 0;
}

int isRelOp(TokenType type){
   if(type==tokIS || type==tokBETWEEN || type==tokNOT)
	return 1;
   else
	return 0;
}

int scan_query(string linestream,int &lineNum){ //check for invalid characters
    int wordsize=0;
    char word[MAXWORD];
    for(int i=0;i<linestream.size();i++){
	if(linestream[i]=='\n'){ 
	    lineNum++;
	    continue;
	}

	/*if(linestream[i]=='-'){ 
	    if(linestream[i+1]=='-'){ //ignore comments
		lineNum++;
		continue;
	    }
	}*/

    	if(isalnum(linestream[i])){
	    word[wordsize++]=linestream[i];
	    if(wordsize==MAXWORD){
		  printf("ERROR: A word in line %d is too long. \n",lineNum);
		  return 1;
	    }
	}else if(isspace(linestream[i]) || isReservedChar(linestream[i])){
	    wordsize = 0;
	}else if(ispunct(linestream[i])){
	    continue;
	}else{
	    printf("Error: Invalid character '%c' in line %d. \n",linestream[i],lineNum);
	    return 1;
	}

	
    }
}

Token anlzr(){
    int idx=0;
    Token token; 
    string temp; 
    for(int i=offset;i<code.size();i++){
	if(code[i]=='\n'){
	    lineNum++;
	    continue; 
	}

     	/*if(code[i]=='-'){
	    if(code[i+1]=='-'){ //1-line comment
		while(code[i]!='\n'){i++;}
		lineNum++; 
		continue;
	    }
	}else if(code[i]=='<'){
	    if(code[i+1]=='!'){ //multi-line comment
		int lineidx=lineNum;
		while(!(code[i]=='!' && code[i+1]=='>')){
		    if(code[i]=='\n') lineNum++;
		    i++;
		    if(i==code.size()){ 
			printf("ERROR: The multi-line comment in line %d is not properly ended.\n",lineidx);
			exit(1);
		    }
		} i++;
		continue;
 	    }
	}else*/ if(code[i]=='"'){
	    idx=i+1; //do not include "
	    while(!(code[++i]=='"')){} //CONCAT print's text
	    temp=code.substr(idx,i-idx).c_str();
	    strcpy(token.str, temp.c_str());
	    token.pos = lineNum;
	    token.type = tokTEXT;
	    offset = i+1;
	    return token;
	}else if(code[i]=='\''){
	    token.str[0]=code[++i];
    	    token.str[1]='\0'; 
	    token.pos = lineNum;
	    token.type = tokLET;
	    if(code[++i]!='\''){ 
		printf("ERROR: Invalid syntax for character value in line %d.\n",lineNum);
		token.type = tokEND;
	    }
	    offset = ++i;
	    return token;
	}

        if(isalpha(code[i])){ //save keywords/attributes
	    idx=i;
	    while(isalnum(code[i]) || code[i]=='_'){code[i]=toupper(code[i++]);}
	    temp=code.substr(idx,i-idx).c_str();
            strcpy(token.str,temp.c_str());
	    token.pos = lineNum;
            if(reservedWord.find(temp)!=reservedWord.end()){ 
		  token.type = reservedWord.find(temp)->second;
	    }else{
		  token.type = tokATTR; //identifier
	    }
	    //printf("%s\n",temp.c_str());
	    offset=i; 
	    return token;
	}else if(isdigit(code[i])){
	    idx=i;
            int dotcount=0,maxdecplace=0;
  	    while(isdigit(code[i]) || code[i]=='.'){
		i++; 
		if(code[i]=='.'){
		    dotcount++;
		    maxdecplace=1;
		}
		if(maxdecplace) maxdecplace++;
	    }
	    if(code[i-1]=='.' || dotcount>1){
		printf("ERROR: Invalid numerical value in line %d.\n",lineNum);
		token.type = tokEND;
	    }
	    if(maxdecplace>5){
		printf("ERROR: The decimal places of a number in line %d exceeds the limit(4).\n",lineNum);
		token.type = tokEND;
	    }
	    temp=code.substr(idx,i-idx).c_str();
	    strcpy(token.str, temp.c_str());
	    token.pos = lineNum;
	    token.type = tokVAL;
	    //printf("%s\n",temp.c_str());
	    offset = i--;
	    return token;
	}else if(ispunct(code[i])){
    	    token.str[0]=code[i];
    	    token.str[1]='\0'; 
	    if(isDelimeter(code[i])){
    		token.pos = lineNum;
    		token.type = getDelimeterType(code[i]);
	    }else if(isArithOperator(code[i])){
    		token.pos = lineNum; 
    		token.type = getArithType(code[i]); 
	    }else{ 
    		token.pos = lineNum;
    		token.type = tokEXPUNC;
	    }
	    //printf("%s\n",token.str);
	    offset = ++i;
	    return token;
	}
    }
    token.type = tokEND; //duplicates the last token if not ';'
    return token;
}

Node* createNode(NodeType type){
    Node *node = (Node*) malloc(sizeof(Node));
    node->type = type;
    //node->token;
    node->child1 =node->child2 =node->child3 =NULL;
    return node;
}

Node* attr_more(){
    Node *node = createNode(attr2Node);
    if(tk.type==tokCOMMA){
	tk = toks[offset++]; 
	if(tk.type == tokATTR){
	    tk = toks[offset++];
 	    node->child1 = attr_more(); //more attributes in the project operation		
    	}else{
	    printf("ERROR: Expecting another attribute name in line %d.\n",tk.pos);
	    exit(1);
	}
    }
    return node;
}

Node* attrib(){
    Node *node = createNode(attrNode);
    if(tk.type == tokATTR){
	tk = toks[offset++];
 	node->child1 = attr_more();
    }else if(tk.type == tokMULT){
	tk = toks[offset++];
    }else{
	printf("ERROR: Invalid attribute name/s in the statement. %s\n",tk.str);
	exit(1);
    }
    return node;     
}

Node* tble_more(){
    Node *node = createNode(attrNode);
    if(tk.type==tokCOMMA){
 	tk = toks[offset++]; 
	if(tk.type == tokATTR){
	    tk = toks[offset++];
 	    node->child1 = tble_more(); //more tables		
    	}else{
	    printf("ERROR: Expecting another table name in line %d.\n",tk.pos);
	    exit(1);
	}
    }
    return node;

}

Node* tble(){
    Node *node = createNode(tblNode);
    if(tk.type == tokATTR){
	tk = toks[offset++];
	node->child1 = tble_more(); //more tables	
    }else{
	printf("ERROR: Invalid table name/s in the statement. %s\n",tk.str);
	exit(1);
    }
    return node;
}

Node* wherecond(){
    Node *node = createNode(whereNode);
    if(tk.type == tokWHERE){
	tk = toks[offset++];
    }
    return node;
}

Node* select(){
    Node *node = createNode(selectNode);
    if(tk.type==tokSELECT){ //open Program
	   tk = toks[offset++];
    }else{
       	printf("ERROR: Invalid start of the statement. %s\n",tk.str);
	exit(1);
    }

    node->child1 = attrib();
    
    if(tk.type==tokFROM){ //open Program
	   tk = toks[offset++];
    }else{
       	printf("ERROR: Invalid select statement. %s\n",tk.str);
	exit(1);
    }

    node->child2 = tble();
    node->child3 = wherecond();

    if(tk.type==tokSEMICOL){ //close Program
	    tk = toks[offset++];
	return node;
    }else{
   	    printf("ERROR: Invalid closing of the statement.\n");
	    exit(1);
    }
}

void parser(){
    tk = toks[offset++];
    Node *root = NULL;
    root = select();

    if(tk.type=tokEND) //tk is global so tk will be the end node if parsing works fine
	    printf("Successful parsing!\n");
    else{
	    exit(1);
    }
}

int main(){
    string linestream;

    ifstream fp1("query.txt");
    if(!fp1.is_open()){
	    printf("ERROR: Cannot open file.");
	return 1;
    }

    make_tokenmap();

    for(int i=0;getline(fp1,linestream);i++){
	if(scan_query(linestream,lineNum)){
	    printf("Cannot run the program. \n");	    
	}
	code += linestream + "\n";
    }

    toks = (Token*) malloc(tokcount*sizeof(Token));
    lineNum=1;
    
    //GET tokens
    do{
	    tokcount++;
	    toks = (Token*)realloc(toks,tokcount*sizeof(Token)); 
	    toks[tokcount-1]= anlzr(); 
	    printf("%s\t%d\n",toks[tokcount-1].str, toks[tokcount-1].type);
    }while(toks[tokcount-1].type != tokEND);

    offset=0;
    lineNum=0;
    parser();
}