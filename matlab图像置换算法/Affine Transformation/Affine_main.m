%仿射变换（Affine Transformation）
%2022/5/15 by linyiting
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
clear;
clc;
A=imread('.\image\lenaRGB256.bmp');
A=rgb2gray(A);
I=A;
% for K=1:16   %置乱16次
%     B(K)=Affine(A); 
%     figure,imshow(B);
%     title(['置乱',num2str(K),'次后的图像'])
%     A=B(K);
% end
 B1=Affine(A); 
 A=B1;
 B2=Affine(A); 
 A=B2;
 B3=Affine(A); 
 A=B3;
 B4=Affine(A); 
 A=B4;
 B5=Affine(A); 
 A=B5;
 B6=Affine(A); 
 A=B6;
 B7=Affine(A); 
 A=B7;
 B8=Affine(A); 
 A=B8;
 B9=Affine(A); 
 A=B9;
 B10=Affine(A); 
 A=B10;
 B11=Affine(A); 
 A=B11;
 B12=Affine(A); 
 A=B12;
 B13=Affine(A); 
 A=B13;
 B14=Affine(A); 
 A=B14;
 B15=Affine(A); 
 A=B15;
 B16=Affine(A); 
 A=B16;

figure(1)
subplot(3,3,1); imshow(uint8(I));title("原图");hold on;
subplot(3,3,2); imshow(uint8(B1));title("置乱1次后的图像");hold on;subplot(3,3,3);imshow(uint8(B2));title("置乱2次后的图像");hold on;
subplot(3,3,4); imshow(uint8(B3));title("置乱3次后的图像");hold on;subplot(3,3,5);imshow(uint8(B4));title("置乱4次后的图像");hold on;
subplot(3,3,6); imshow(uint8(B5));title("置乱5次后的图像");hold on;subplot(3,3,7);imshow(uint8(B6));title("置乱6次后的图像");hold on;
subplot(3,3,8); imshow(uint8(B7));title("置乱7次后的图像");hold on;subplot(3,3,9);imshow(uint8(B8));title("置乱8次后的图像");hold on;
figure(2)
subplot(3,3,1); imshow(uint8(I));title("原图");hold on;
subplot(3,3,2); imshow(uint8(B9));title("置乱9次后的图像");hold on;subplot(3,3,3);imshow(uint8(B10));title("置乱10次后的图像");hold on;
subplot(3,3,4); imshow(uint8(B11));title("置乱11次后的图像");hold on;subplot(3,3,5);imshow(uint8(B12));title("置乱12次后的图像");hold on;
subplot(3,3,6); imshow(uint8(B13));title("置乱13次后的图像");hold on;subplot(3,3,7);imshow(uint8(B14));title("置乱14次后的图像");hold on;
subplot(3,3,8); imshow(uint8(B15));title("置乱15次后的图像");hold on;subplot(3,3,9);imshow(uint8(B16));title("置乱16次后的图像");hold on;



