%约瑟夫问题置换图像
%2022/5/14 by linyiting
%TEST
%测试矩阵图像
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
clear;
clc;
I=imread('Lena.bmp');
%t=[1 2 3; 4 5 6 ;7 8 9 ] 测试矩阵
I=double(I);
a=joseph_traverse(I,2);
b=Dejoseph_traverse(a,2);

figure(1)
subplot(2,3,1); imshow(uint8(I));title("原图");hold on;subplot(2,3,4);imhist(uint8(I));title("原图直方图");hold on;
subplot(2,3,2); imshow(uint8(a));title("1图");hold on;subplot(2,3,5);imhist(uint8(a));title("1直方图");hold on;
subplot(2,3,3); imshow(uint8(b));title("2乱图");hold on;subplot(2,3,6);imhist(uint8(b));title("2图直方图");hold on;


