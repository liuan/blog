#include<stdio.h>
#include<malloc.h>
#include<string.h>

void print(char **words,int len){
    if(words == NULL || len <= 0)
        return;
    int i;
    for(i = 0; i < len; ++i){
        while(*(words[i]) != ' ' && *(words[i]) != '\0'){
            printf("%c",*words[i]++);
        }
        if(i != len - 1)
            printf(" ");
    }
    printf("\n");
}

int getEachWord(char *str, char **words){
    if(str == NULL || words == NULL)
        return 0;
    int count = 0;
    int blank = 1;// means true
    while(*str != '\0'){ 
        if(blank == 1 && *str != ' '){
            ++count;
            *words++ = str;
            blank = 0;
        }else if(*str == ' '){
            blank = 1;
        }
        ++str;
    }

    return count;
}

void reverse(char **words, int count){
    if(words == NULL || count <= 0)
        return;
    int start = 0, end = count - 1;
    while(start < end){
        char *temp = words[start];
        words[start++] = words[end];
        words[end--] = temp;
    }
}

void reverseSentence(char *str){
    if(!str || strlen(str) == 0){
        printf("you should check the input.\n");
        return;
    }

    char **words = (char **)malloc(sizeof(char *)*strlen(str));
    int count = getEachWord(str,words);
    //print(words,count);
    reverse(words,count);
    print(words,count);

    free(words);
}

void main(){
    char str[] = {"I am a student."};   
    printf("%s\n",str);
    reverseSentence(str);
}
