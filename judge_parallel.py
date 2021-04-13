import subprocess
import time
import os

#windowsかUNIXかを取得(実行時に叩くコマンドが変わるため)
osname=os.name
#ジャッジするコードのファイル名を入力してもらう
#sourcefile=input("input sourcefile's name: ")
sourcefile="task1.c"
#テストケースの取得
testcase_path="in/"
files=os.listdir(testcase_path)
testcases=[os.path.join(testcase_path,f) for f in files if os.path.isfile(os.path.join(testcase_path,f))]
testcases.sort()

#得点を格納しておく
score_all=[0 for j in range(len(testcases))]

#コンパイル
#g++にしてるのはWindows系でgccを叩くとウイルスバスターが悪さをしてバイナリを消してしまうことがあるため
subprocess.run("g++ -o a_task1 -O2 "+sourcefile,shell=True)

#実行コマンドの指定
if osname=="nt":
    cmd_prefix=".\\a_task1 "
else:
    cmd_prefix="./a_task1 "

procs=[]
for i in testcases:
    procs.append(subprocess.Popen(cmd_prefix+i,shell=True,
    stdout=subprocess.PIPE, stderr=subprocess.PIPE))

Ans=[0 for i in range(len(testcases))]
Time=[]
for i in range(len(procs)):
    stdout,stderr=procs[i].communicate()
    Ans[i]=stdout.decode()
"""
for proc in procs:
    stdout,stderr=proc.communicate()
    Ans.append(stdout.decode())
"""
Ans=[i[:-2] for i in Ans]
for i in range(len(testcases)):
    with open(testcases[i],mode="r") as f:
        weight=f.readlines()
    if len(Ans[i])!=len(weight):
        print(len(Ans[i]),len(weight))