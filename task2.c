#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")

#include<stdio.h>
#include<stdlib.h>
//#include<unistd.h>
#include<signal.h>
#include<time.h>
#include<math.h>

#define N 4096
#define swap(a,b) (a ^= b,b = a ^ b,a ^= b)

double max(double a,double b){return a>b?a:b;};
double min(double a,double b){return a<b?a:b;};

int group_num;
double weight[N],weight_sorted[N];
double Sum[16],Smax,Smin;
double Ave=0;
double score_best;

int Ans[N],Ans_best[N];
int n_weights=0;
char* L="ABCDEF";

//得点は各グループの分散にする
double scoring(){
    double maxval=Sum[0],minval=Sum[0];
    for(int i=1;i<group_num;++i){
        maxval=max(maxval,Sum[i]);
        minval=min(minval,Sum[i]);
    }
    return maxval-minval;
}

void greedy(){
    double ref;
    for(int i=0;i<n_weights;++i){
        double w=weight[i];
        for(int j=0;j<group_num;++j){
            Sum[Ans[i]]-=w;
            Sum[j]+=w;
            ref=scoring();
            if(ref<score_best){
                score_best=ref;
                Ans[i]=j;
            }else{
                Sum[Ans[i]]+=w;
                Sum[j]-=w;
            }
        }
    }
}

int main(int argc,char**argv){
    clock_t start=clock();//実行時間の計測開始
    clock_t end;
    long int itr=0;
    double TL=60*1000-10;//[msec]
    double T0=1e10;
    double T1=1e2;

    //乱数の初期化
    srand((unsigned int)time(NULL));

    group_num=atoi(argv[1]);    

    FILE *fp;
    char cbuf[1024];
    if ((fp = fopen(argv[2], "r")) == NULL) {
        //エラー出力；
        exit(1);
    }

    while (fgets(cbuf, 1024, fp)){
        weight[n_weights++] = atof(cbuf);
    }
    fclose(fp);

    //ランダムに初期化
    for(int i=0; i<n_weights; ++i)Ans[i]=rand()%group_num;

    //各グループの総和を求める
    for(int i=0; i<n_weights; ++i)Sum[Ans[i]]+=weight[i];

    //初期解のスコアを算出する
    score_best=scoring();
    printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
    greedy();
    score_best=scoring();

    printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
    for(int i=0;i<n_weights;++i)Ans_best[i]=Ans[i];
    end=clock();

    //焼きなまし
    while((double)(end-start)/CLOCKS_PER_SEC*1000<=TL){
        double score_ref;
        if(rand()%2==0){
            //1点交叉
            int cur=rand()%n_weights;
            int val=rand()%group_num;
            while(Ans[cur]==val)val=rand()%group_num;
            Sum[Ans[cur]]-=weight[cur];
            Sum[val]+=weight[cur];
            score_ref=scoring();

            if(score_ref>score_best){
                double rnd=rand()/(double)RAND_MAX;
                double t=(end-start)/TL;
                double T=powf(T0,1.0-t)*powf(T1,t);
                double p=expf((score_best-score_ref)/T);
                //rnd<=pなら採用, rnd>pならもとに戻す
                if(rnd>p){
                    Sum[Ans[cur]]+=weight[cur];
                    Sum[val]-=weight[cur];
                }else{
                    Ans[cur]=val;
                }
            }else{
                Ans[cur]=val;
            }
        }else{
            //2点swap
            int cur1=rand()%n_weights,cur2=rand()%n_weights;
            while(cur1==cur2||Ans[cur1]==Ans[cur2])cur2=rand()%n_weights;
            Sum[Ans[cur1]]+=weight[cur2]-weight[cur1];
            Sum[Ans[cur2]]+=weight[cur1]-weight[cur2];
            score_ref=scoring();

            if(score_ref>score_best){
                double rnd=rand()/(double)RAND_MAX;
                double t=(end-start)/TL;
                double T=powf(T0,1.0-t)*powf(T1,t);
                double p=expf((score_best-score_ref)/T);
                //rnd<=pなら採用, rnd>pならもとに戻す
                if(rnd>p){
                    Sum[Ans[cur1]]-=weight[cur2]-weight[cur1];
                    Sum[Ans[cur2]]-=weight[cur1]-weight[cur2];
                }else{
                    swap(Ans[cur1],Ans[cur2]);
                }
            }else{
                swap(Ans[cur1],Ans[cur2]);
            }
        }

        if(score_best>score_ref){
            score_best=score_ref;
            printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
            //最適解を保存しておく
            for(int i=0;i<n_weights;++i)Ans_best[i]=Ans[i];
        }

        ++itr;
        if(itr%100==0){
            end=clock();
        }
    }
    
    //最適解より悪いままなら最適解を採用
    if(scoring()>score_best)for(int i=0;i<n_weights;++i){
        Ans[i]=Ans_best[i];
    }
    for(int i=0;i<group_num;++i)Sum[i]=0;
    for(int i=0;i<n_weights;++i)Sum[Ans[i]]+=weight[i];
    Smax=Sum[0];
    Smin=Sum[0];
    for(int i=0;i<group_num;++i){
        Smax=max(Smax,Sum[i]);
        Smin=min(Smin,Sum[i]);
    }
    //printf("%.30lf, %.30lf\n",Smin,Smax);
    //解の出力
    for(int i=0; i<n_weights;++i){
        if(Ans[i]>=10)printf("%c",L[Ans[i]%10]);
        else printf("%d",Ans[i]);
    }
    printf("\n");
}