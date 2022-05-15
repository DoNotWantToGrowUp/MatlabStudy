function B=Affine(A)
%仿射变换（Affine Transformation）
%2022/5/15 by linyiting
%此函数将图像A置乱，输出置乱后的图像B
[M,N]=size(A);
for x=1:N
    for y=1:N
        if x<y   %计算(x,y)点映射到B图像的坐标(x1,y1)
            x1=x-y+N+1;
            y1=-x+N+1;
        else
            x1=x-y+1;
            y1=-x+N+1; 
        end
        B(x1,y1)=A(x,y);   %B图像的像素幅值
    end
end
end