function v=joseph_traverse(picture,space)
%约瑟夫问题置换图像
%2022/5/14 by linyiting
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
t=picture;
start=1;
[a,b]=size(t);
t=reshape(t',1,a*b);
value=a*b;
m=zeros(1,value);%存储标记序列
e=zeros(1,value);%存储置乱序列
cnt1=0;%用于记录下标
cnt2=0;%用于记录距离
flag=1;
while flag==1
    start=mod(start,value);
    if start==0%下标越界处理
        start=value;
    end
    while cnt2~=space
        %跳过标记区
        while m(1,start)==1
            start=start+1;
            start=mod(start,value);
            if start==0
                start=value;
            end
        end
        cnt2=cnt2+1;
        start=start+1;
        start=mod(start,value);
        if start==0
            start=value;
        end
    end
    start=mod(start-1+value,value);
    if start==0
        start=value;
    end
    cnt2=0;%清除距离，一遍下一次遍历计数
    cnt1=cnt1+1;
    m(1,start)=1;%已遍历过该点，标记防止重复遍历
    e(1,cnt1)=t(1,start);
    %遍历结束
    if cnt1==value
        flag=0;
    end
end
v=reshape(e,a,b);