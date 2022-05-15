function v=joseph_traverse(picture,space)
%解约瑟夫问题置换图像
%joseph traverse Transformation
%2022/5/14 by linyiting
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
a=picture;

s=space;
t=a;

[h,w]=size(t);
m=h*w;

for i=1:m 
    v(i)=0;
end

n=0;
x=1;
i=0;
while x~=m
    while n~=s
        i=i+1;

        if i>m
            i=1;
        end

        if v(i)==0
           n=n+1;
        end
     
        
    end
    n=0;
   v(i)=a(x);
   x=x+1;
    
       
end

for i=1:m
    if v(i)==0
        v(i)=a(m);
    end
end

i=1;
for h1=1:h
    for w1=1:w
        P(h1,w1)=v(i);
        i=i+1;
    end
end
v=P;
end