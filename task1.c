//#pragma GCC target("avx2")
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,avx2,tune=native")

#include<stdio.h>
#include<stdlib.h>
//#include<unistd.h>
#include<signal.h>
#include<time.h>
#include<math.h>

#define N 4096
#define swap(a,b) (a ^= b,b = a ^ b,a ^= b)

double weight[N],weight_sorted[N];
double Sum[2];
int Ans[N],Ans_best[N];
int n_weights=0;
double sum_1=0,sum_2=0;

int max(int a,int b){return a>b?a:b;};
int min(int a,int b){return a<b?a:b;};

int compare_double(const void *a,const void *b){
    if(*(double*)a-*(double*)b>0){
        return 1;
    }else if(*(double*)a-*(double*)b<0){
        return -1;
    }else return 0;
}

double scoring(){
    double sum1=0;
    double sum2=0;
    for(int i=0;i<n_weights;++i){
        if(Ans[i]==0)sum1+=weight[i];
        else sum2+=weight[i];
    }
    return fabs(sum1-sum2);
}

//初期解の生成
void greedy(){
    double diff1,diff2;
    for(int i=0;i<n_weights;++i){
        if(fabs(Sum[0]+weight[i]-Sum[1])<fabs(Sum[1]+weight[i]-Sum[0])){
            Ans[i]=0;
            Sum[0]+=weight[i];
        }else{
            Ans[i]=1;
            Sum[1]+=weight[i];
        }
    }
}

int main(int argc,char **argv){
    clock_t start=clock();//実行時間の計測開始
    clock_t end;
    long int itr=0;
    double TL=600*1000-10;//[msec]
    double T0=1e10;
    double T1=1e2;
    double score_best;

    FILE *fp;
    char cbuf[1024];
    if ((fp = fopen(argv[1], "r")) == NULL) {
        //エラー出力；
        exit(1);
    }

    while (fgets(cbuf, 1024, fp)){
        weight[n_weights++] = atof(cbuf);
    }
    fclose(fp);

    score_best=scoring();
    printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
    greedy();
    score_best=scoring();
    //printf("# %d: %1.30e\n",clock()-start,score_best);
    printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);

    //乱数の初期化
    srand((unsigned int)time(NULL));
    end=clock();

    //局所探索(焼きなまし)
    while((double)(end-start)/CLOCKS_PER_SEC*1000<=TL){
        //交叉方法
        //1点交叉: 任意の1点を選んで反転
        //多点交叉: 任意の幅を選んで反転
        //2点swap: 任意の2範囲(幅は2以上)を選んでswap
        double score_ref;
        if(rand()%3==0){
            //1点交叉
            //ランダムに1つ選んで符号を反転させる
            //double score_origin=fabs(Sum[0]-Sum[1]);
            int cursole=rand()%n_weights;
            Sum[Ans[cursole]]-=weight[cursole];
            Sum[(Ans[cursole]+1)%2]+=weight[cursole];
            Ans[cursole]=(Ans[cursole]+1)%2;
            score_ref=fabs(Sum[0]-Sum[1]);

            //よくなれば採用(変えたままにしてscore_bestを更新)

            //悪ければexp(delta/T), T=T0^(1-t)*T1^tの確率で採用
            if(score_ref>score_best){
                double rnd=rand()/(double)RAND_MAX;
                double t=(end-start)/TL;
                double T=powf(T0,1.0-t)*powf(T1,t);
                double p=expf((score_best-score_ref)/T);
                //rnd<=pなら採用, rnd>pならもとに戻す
                if(rnd>p){
                    Sum[Ans[cursole]]-=weight[cursole];
                    Sum[(Ans[cursole]+1)%2]+=weight[cursole];
                    Ans[cursole]=(Ans[cursole]+1)%2;
                }
            }
        }else if (rand()%3==1){
            //多点交叉
            int cur1,cur2;
            cur1=rand()%n_weights;
            cur2=rand()%n_weights;
            
            while(cur1==cur2)cur2=rand()%n_weights;
            //printf("%d\n",abs(cur1-cur2));
            //min(cur1,cur2)からmax(cur1,cur2)まで反転する
            for(int i=min(cur1,cur2);i<=max(cur1,cur2);++i){
                Sum[Ans[i]]-=weight[i];
                Ans[i]=(Ans[i]+1)%2;
                Sum[Ans[i]]+=weight[i];
            }

            score_ref=fabs(Sum[0]-Sum[1]);
            //悪ければexp(delta/T), T=T0^(1-t)*T1^tの確率で採用
            if(score_ref>score_best){
                double rnd=rand()/(double)RAND_MAX;
                double t=(end-start)/TL;
                double T=powf(T0,1.0-t)*powf(T1,t);
                double p=expf((score_best-score_ref)/T);
                //rnd<=pなら採用, rnd>pならもとに戻す
                if(rnd>p){
                    for(int i=min(cur1,cur2);i<=max(cur1,cur2);++i){
                        Sum[Ans[i]]-=weight[i];
                        Ans[i]=(Ans[i]+1)%2;
                        Sum[Ans[i]]+=weight[i];
                    }
                }
            }
        }else{
            //2点swap
            int cur1,cur2,width,width_ref;
            cur1=rand()%(n_weights-1);
            cur2=rand()%(n_weights-1);
            while(cur1==cur2 || abs(cur1-cur2)<2)cur2=rand()%(n_weights-1);
            width=abs(cur2-cur1);
            width_ref=min(width,n_weights-max(cur1,cur2));
            width=rand()%width_ref+1;
            while(width<2)width=rand()%width_ref+1;
            //swapする
            for(int i=0;i<width;++i){
                Sum[Ans[cur1+i]]-=weight[cur1+i];
                Sum[Ans[cur2+i]]-=weight[cur2+i];
                swap(Ans[cur1+i],Ans[cur2+i]);
                Sum[Ans[cur1+i]]+=weight[cur1+i];
                Sum[Ans[cur2+i]]+=weight[cur2+i];
            }

            score_ref=fabs(Sum[0]-Sum[1]);
            //悪ければexp(delta/T), T=T0^(1-t)*T1^tの確率で採用
            if(score_ref>score_best){
                double rnd=rand()/(double)RAND_MAX;
                double t=(end-start)/TL;
                double T=powf(T0,1.0-t)*powf(T1,t);
                double p=expf((score_best-score_ref)/T);
                //rnd<=pなら採用, rnd>pならもとに戻す
                if(rnd>p){
                    for(int i=0;i<width;++i){
                        Sum[Ans[cur1+i]]-=weight[cur1+i];
                        Sum[Ans[cur2+i]]-=weight[cur2+i];
                        swap(Ans[cur1+i],Ans[cur2+i]);
                        Sum[Ans[cur1+i]]+=weight[cur1+i];
                        Sum[Ans[cur2+i]]+=weight[cur2+i];
                    }
                }
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
    if(scoring()>score_best)for(int i=0;i<n_weights;++i)Ans[i]=Ans_best[i];
    for(int i=0;i<n_weights;++i)printf("%d",Ans[i]);
    printf("\n");
    //printf("%lf\n",scoring());
}