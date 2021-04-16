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
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
//double max(double a,double b){return a>b?a:b;};
//double min(double a,double b){return a<b?a:b;};

int group_num;
int G[N][N],Ans[N],Ans_best[N];
int g1[N],g2[N],c1[N],c2[N];
double Sum[2];
double weight[N];
double Scores[N];
double score_best;

int n_weights=0;

double scoring(int A[N]){
    double S[N];
    for(int i=0;i<group_num;++i)S[i]=0;
    for(int i=0;i<n_weights;++i)S[A[i]]+=weight[i];
    double maxval=S[0],minval=S[0];
    for(int i=1;i<group_num;++i){
        maxval=max(maxval,S[i]);
        minval=min(minval,S[i]);
    }
    return maxval-minval;
}

void greedy(int A[N]){
    double ref,score_best=scoring(A);
    int tmp;
    for(int i=0;i<n_weights;++i){
        double w=weight[i];
        for(int j=0;j<group_num;++j){
            tmp=A[i];
            A[i]=j;
            ref=scoring(A);
            if(ref<score_best)score_best=ref;
            else A[i]=tmp;
        }
    }
}

int main(int argc,char**argv){
    clock_t start=clock();//実行時間の計測開始
    clock_t end;
    long int itr=0;
    double TL_genetic=200*1000;//[msec]
    double TL=600*1000-10;//[msec]
    double T0=1e10;
    double T1=1e2;
    int flg=1;//焼きなまし: 0, 山登り: 1

    //乱数の初期化
    srand((unsigned int)time(NULL));

    group_num=2;    

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

    //乱数で初期化
    //0 N個の個体をランダムに生成
    for(int i=0;i<N;++i){
        for(int j=0;j<n_weights;++j)G[i][j]=rand()%group_num;
        //1/10の確率で貪欲する
        if(rand()%10==0){
            greedy(G[i]);
        }
    }

    //得点計算
    for(int i=0;i<N;++i){
        Scores[i]=scoring(G[i]);
        if(i==0||Scores[i]<score_best)score_best=Scores[i];
    }

    int M=500;//1世代での交叉回数
    double alpha=1e-2;//突然変異を起こす確率
    printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
    end=clock();

    while((double)(end-start)/CLOCKS_PER_SEC*1000<=TL_genetic){
        int cur1=rand()%N;
        int cur2=rand()%N;
        while(cur1==cur2)cur2=rand()%N;
        //1: ランダムに2個体を集団から取り出し、コピーする
        for(int i=0;i<n_weights;++i){
            g1[i]=G[cur1][i];
            g2[i]=G[cur2][i];
        }
        double score_g1,score_g2,score_c1,score_c2;

        //取り出した個体の評価値を計算する
        score_g1=Scores[cur1];
        score_g2=Scores[cur2];

        //2: M回繰り返す
        for(int m=0;m<M;++m){
            //a: 交叉を行い2子個体を生成する
            int l,r;
            l=rand()%n_weights;
            r=rand()%n_weights;
            while(r==l)r=rand()%n_weights;
            if(r<l)swap(l,r);

            for(int i=0;i<n_weights;++i){
                if(l<=i&&i<=r){
                    c1[i]=g2[i];
                    c2[i]=g1[i];
                }else{
                    c1[i]=g1[i];
                    c2[i]=g2[i];
                }
                //c: 確率に応じて突然変異を行う
                double rnd=rand()/(double)RAND_MAX;
                int change;
                if(rnd<=alpha){
                    change=rand()%group_num;
                    while(change==c1[i])change=rand()%group_num;
                    c1[i]=change;
                }

                rnd=rand()/(double)RAND_MAX;
                change;
                if(rnd<=alpha){
                    change=rand()%group_num;
                    while(change==c2[i])change=rand()%group_num;
                    c2[i]=change;
                }
            }

            //d: 子の評価値がg1,g2より高ければ置き換える
            score_c1=scoring(c1);
            score_c2=scoring(c2);

            if(score_c1<score_g1){
                for(int i=0;i<n_weights;++i)g1[i]=c1[i];
                Scores[cur1]=score_c1;
            }

            if(score_c2<score_g2){
                for(int i=0;i<n_weights;++i)g2[i]=c2[i];
                Scores[cur2]=score_c2;
            }
        }

        //g1,g2を集団に戻す
        for(int i=0;i<n_weights;++i){
            G[cur1][i]=g1[i];
            G[cur2][i]=g2[i];
        }

        ++itr;
        if(itr%100==0)end=clock();
        //10000世代毎にベストスコアを計算・出力
        if(itr%1000==0){
            for(int i=0;i<N;++i)if(i==0||Scores[i]<score_best)score_best=Scores[i];
            printf("# %.0lf: %1.30e\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000,score_best);
            //しばらく改善が見られなかったら下位N/2個をランダムに初期化
        }
    }

    //最もよかったものをヒューリスティックに解く
    for(int i=0;i<N;++i)if(Scores[i]<=score_best){
        for(int j=0; j<n_weights;++j){
            Ans[j]=G[i][j];
            Ans_best[j]=G[i][j];
            Sum[Ans[j]]+=weight[j];
        }
        break;
    }
    end=clock();
    double score_best=fabs(Sum[1]-Sum[0]);

    printf("# %.lf: start heuristic\n",(double)(clock()-start)/CLOCKS_PER_SEC*1000);
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

    double score=0;
    Sum[0]=0;
    Sum[1]=0;
    for(int i=0;i<n_weights;++i)Sum[Ans[i]]+=weight[i];
    //最適解より悪いままなら最適解を採用
    if(fabs(Sum[1]-Sum[0])>score_best)for(int i=0;i<n_weights;++i)Ans[i]=Ans_best[i];
    for(int i=0;i<n_weights;++i)printf("%d",Ans[i]);
    printf("\n");
    //printf("%lf\n",scoring());

}