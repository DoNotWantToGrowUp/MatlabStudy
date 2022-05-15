% main函数
% 2022/05/11 by linyiting
% LV 3.0 BlockScram Transformation
%https://gitee.com/DoNotWantToGrowUp/matlab-study/

clc;clear;close all;

%读入图片
I=imread('.\image\lenaRGB256.bmp');
%密钥随机4位
key=[0 1 2 3];
%块置换块大小设置
fenkaui=4;%分块4X4 
tic
%% 块加密
EI=BlockScram_EnAlgorithm(I,key,fenkaui);
toc

tic
%% 块解密
DI=BlockScram_DeAlgorithm(EI,key,fenkaui);

%% 输出
figure(1)
subplot(2,3,1); imshow(uint8(I));title("原图");hold on;subplot(2,3,4);imhist(uint8(I));title("原图直方图");hold on;
subplot(2,3,2); imshow(uint8(EI));title("块置乱图");hold on;subplot(2,3,5);imhist(uint8(EI));title("块置乱直方图");hold on;
subplot(2,3,3); imshow(uint8(DI));title("解块置乱图");hold on;subplot(2,3,6);imhist(uint8(DI));title("解块置乱图直方图");hold on;
toc