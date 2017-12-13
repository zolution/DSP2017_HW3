#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include "Ngram.h"
#include<fstream>
using namespace std;

Ngram *lm = NULL;
Vocab voc;
char to_insert_map[100005];
char to_do[100005];
inline unsigned int toKey(unsigned char a, unsigned char b){
    return ((unsigned int)a << 8) + b;
}

inline void toBig5(unsigned int a, unsigned char *goal){
    goal[0] = a >> 8;
    goal[1] = a & 255;
    return;
}

// Get P(W2 | W1) -- bigram
double getBigramProb(const unsigned char *tw1, const unsigned char *tw2){
    char w1[3];
    char w2[3];
    w1[0] = (char) tw1[0];
    w1[1] = (char) tw1[1];
    w1[2] = '\0';
    w2[0] = (char) tw2[0];
    w2[1] = (char) tw2[1];
    w2[2] = '\0';
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm->wordProb( wid2, context);
}

map<unsigned int, double> delta[2];
vector<map<unsigned int, unsigned int> > bt;

void Viterbi_init(int n, unsigned int* poss_list){
    delta[0].clear();
    delta[1].clear();
    bt.clear();
    for(int i=0;i<n;i++){
        VocabIndex context[] = {Vocab_None};
        unsigned char a[2];
        toBig5(poss_list[i],a);
        char b[3];
        b[0] = (char) a[0];
        b[1] = (char) a[1];
        b[2] = '\0';
        VocabIndex wid2 = voc.getIndex(b);
        if(wid2 == Vocab_None) wid2 = voc.getIndex(Vocab_Unknown);
        delta[0][poss_list[i]] = lm->wordProb(wid2, context);
    }
    return;
}

void Viterbi_next(int n, unsigned int* poss_list){
    delta[1].clear();
    bt.push_back(map<unsigned int, unsigned int>());

    for(int i=0;i<n;i++){
        unsigned char ch[2];
        toBig5(poss_list[i],ch);
//        VocabIndex id = voc.getIndex(ch);
//        if(id == Vocab_None)
//            id = voc.getIndex(Vocab_Unknown);
        
        double maxi = -10000000.0;
        unsigned int maxi_w = 0;
        for (map<unsigned int, double>::iterator it=delta[0].begin(); it!=delta[0].end(); it++){
            unsigned char tch[2];
            toBig5(it->first, tch);
            //VocabIndex tid = voc.getIndex(tch);
//            if(tid == Vocab_None)
//                tid = voc.getIndex(Vocab_Unknown);
            double p = getBigramProb(ch,tch);
            if(fabs(p)<=1e-9) continue;
            if(p + it->second > maxi){
                maxi = p + it->second;
                maxi_w = it->first;
            }
        
            if(maxi_w != 0){
                delta[1][poss_list[i]] = maxi;
                bt.back()[poss_list[i]] = maxi_w;
            }
            else{
                cerr<<"YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY!!!!!!!!!!!!!"<<endl;
            }
        }
    }
    delta[0] = delta[1];
    return;
}

vector<unsigned int> Viterbi_bt(){
    double maxi = -10000000.0;
    unsigned int maxi_w = 0;
    for (map<unsigned int, double>::iterator it=delta[0].begin(); it!=delta[0].end(); it++){
        if(it->second > maxi){
            maxi_w = it->first;
            maxi = it->second;
        }
    }
    vector<unsigned int> wlist;
    wlist.push_back(maxi_w);

    while(!bt.empty()){
        wlist.push_back(bt.back()[maxi_w]);
        maxi_w = bt.back()[maxi_w];
        bt.pop_back();
    }
    reverse(wlist.begin(),wlist.end());
    return wlist;
}


int main(int argc, char **argv){
    
    int ngram_order = 2;
    char *lm_filename = argv[1];
    char *map_filename = argv[2];
    char *input_filename = argv[3];
    //Init
    lm = new Ngram( voc, ngram_order );
    {
        File lmFile( lm_filename, "r" );
        lm->read(lmFile);
        lmFile.close();
    }

    //Construct Map
    map <unsigned int, vector<unsigned int> > mymap;
    ifstream map_file(map_filename);
//    map_file.open(map_filename);
    string temp;
    while(getline(map_file, temp)){
        strcpy(to_insert_map, temp.c_str());
        vector<unsigned int> goal;
        for(int ptr = 3; to_insert_map[ptr-1]!='\n' && to_insert_map[ptr-1]!='\0'; ptr+=3){
            goal.push_back(toKey(to_insert_map[ptr], to_insert_map[ptr+1]));
        }
        mymap[toKey(to_insert_map[0],to_insert_map[1])] = goal;
    }

    //Viterbi
    ifstream input_file(input_filename);
//    input_file.open(input_filename)
    while(getline(input_file, temp)){
        strcpy(to_do, temp.c_str());
        int len = strlen(to_do);
        int ptr = 0;
        while(isspace(to_do[ptr]) && ptr<len) ptr++;
        bool inited = false;
        
        while(ptr<len){
            if(ptr>=len) break;
            unsigned int Key = toKey(to_do[ptr],to_do[ptr+1]);
            vector<unsigned int>& possible = mymap[Key];
            if(!inited){
                Viterbi_init(possible.size(), &possible[0]);
                inited = true;
            }
            else{
                Viterbi_next(possible.size(), &possible[0]);
            }
            ptr+=2;
            while(isspace(to_do[ptr]) && ptr<len) ptr++;
        }
    
        vector<unsigned int> ans = Viterbi_bt();

        //print
        
        printf("<s> ");
        int len_ans = ans.size();
        for(int i=0;i<len_ans;i++){
            unsigned char a[2];
            toBig5(ans[i],a);
            printf("%c%c ",a[0],a[1]);
        }
        printf("</s> \n");
    }
    return 0;
}

/*

    VocabIndex wid = voc.getIndex("囧");
    if(wid == Vocab_None) {
        printf("No word with wid = %d\n", wid);
        printf("where Vocab_None is %d\n", Vocab_None);
    }

    wid = voc.getIndex("患");
    VocabIndex context[] = {voc.getIndex("癮") , Vocab_None};
    printf("log Prob(患者|毒-癮) = %f\n", lm.wordProb(wid, context)); 
*/

