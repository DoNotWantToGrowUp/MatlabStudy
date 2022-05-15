% 基于块加扰的图像加密函数
% 2022/05/11 by linyiting
% LV 3.0 BlockScram Transformation
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
% 输入明文P，测试密钥K
% 输出密文C
function C = BlockScram_EnAlgorithm(I,K,fenkuai)
  
  Bx=fenkuai;
      
    %先把图像补成行数和列数均为8的倍数的矩阵
    [m0,n0,~] = size(I);    %计算图像P的尺寸
    m1=Bx-mod(m0,Bx);         %计算需要增加的行数
        if m1==Bx
            m1=0;
        end
    n1=Bx-mod(n0,Bx);         %计算需要增加的列数
        if n1==Bx
            n1=0;
        end   
    m=m0+m1;
    n=n0+n1;
    P=zeros(m,n,3);          %矩阵P1行数和列数均为8的倍数
    P(1:m0,1:n0,:)=I;
    
    %明文分块
    Block=zeros(Bx,Bx,3,m/Bx,n/Bx);%5维数组，以8×8×3大小的数组作为5维数组的元素
    for i=1:m/Bx
        for j=1:n/Bx
            Block(:,:,:,i,j)=P((i-1)*Bx+1:i*Bx,(j-1)*Bx+1:j*Bx,:);
        end
    end
    
    %5D数组Block转成3D图像矩阵P2
    P1=zeros(m,n,3);
    for i=1:m/Bx
        for j=1:n/Bx
            P1((i-1)*Bx+1:i*Bx,(j-1)*Bx+1:j*Bx,:)=Block(:,:,:,i,j);
        end
    end
    


    %% 使用密钥产生伪随机序列
    
    S_Len=m*n/(Bx*Bx);              %序列长度，这里各个伪随机序列的长度一样，故统一用S_Len表示序列长度
    
    rng(K(1));                      %控制随机数生成器，随机数生成器randi()的种子设置为0
    S1=randi([1,m*n/(Bx*Bx)],1,S_Len);  %返回一个由介于1和m*n/64之间的伪随机整数组成的1×S_Len数组

    rng(K(2));                      %控制随机数生成器，随机数生成器randi()的种子设置为1
    S2=randi([0,5],1,S_Len);       %返回一个由介于0和5之间的伪随机整数组成的1×S_Len数组

    rng(K(3));
    S3=randi([0,1],1,S_Len);

    rng(K(4));
    S4=randi([0,5],1,S_Len);
    
    %% 加密程序1，块置乱加密
    
    %Block的子块根据随机序列S1置乱
    for i=1:m*n/(Bx*Bx)  
        t=Block(:,:,:,i);
        Block(:,:,:,i)=Block(:,:,:,S1(i));
        Block(:,:,:,S1(i))=t;
    end
    
    
    %% 加密程序2，块旋转和反转加密
    for i=1:m/Bx
        for j=1:n/Bx
            % 若S2(i*j)==0，则不旋转块，即不做操作
            switch S2(i*j)
                case 1              %顺时针旋转90°
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(Bx+1-l,k,:,i,j);
                            Block(Bx+1-l,k,:,i,j)=Block(Bx+1-k,Bx+1-l,:,i,j);
                            Block(Bx+1-k,Bx+1-l,:,i,j)=Block(l,Bx+1-k,:,i,j);
                            Block(l,Bx+1-k,:,i,j)=t;
                        end
                    end
                    
                case 2              %顺时针旋转180°
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t1=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(Bx+1-k,Bx+1-l,:,i,j);
                            Block(Bx+1-k,Bx+1-l,:,i,j)=t1;

                            t2=Block(k,l+Bx/2,:,i,j);
                            Block(k,l+Bx/2,:,i,j)=Block(Bx+1-k,Bx/2+1-l,:,i,j);
                            Block(Bx+1-k,Bx/2+1-l,:,i,j)=t2;
                        end
                    end
                    
                case 3              %顺时针旋转270°
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(l,Bx+1-k,:,i,j);
                            Block(l,Bx+1-k,:,i,j)=Block(Bx+1-k,Bx+1-l,:,i,j);
                            Block(Bx+1-k,Bx+1-l,:,i,j)=Block(Bx+1-l,k,:,i,j);
                            Block(Bx+1-l,k,:,i,j)=t;
                        end
                    end
                    
                case 4              %水平翻转
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t1=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(k,Bx+1-l,:,i,j);
                            Block(k,Bx+1-l,:,i,j)=t1;

                            t2=Block(Bx/2+k,l,:,i,j);
                            Block(Bx/2+k,l,:,i,j)=Block(Bx/2+k,Bx+1-l,:,i,j);
                            Block(Bx/2+k,Bx+1-l,:,i,j)=t2;
                        end
                    end
                    
                case 5              %垂直翻转
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t1=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(Bx+1-k,l,:,i,j);
                            Block(Bx+1-k,l,:,i,j)=t1;

                            t2=Block(k,Bx/2+l,:,i,j);
                            Block(k,Bx/2+l,:,i,j)=Block(Bx+1-k,Bx/2+l,:,i,j);
                            Block(Bx+1-k,Bx/2+l,:,i,j)=t2;
                        end
                    end
                    
                otherwise 
                    
            end 
        end
    end
    
    
    %% 加密程序3，负-正变换
    
    %A=repmat(255,Bx,Bx,3);
    for i=1:m/Bx
        for j=1:n/Bx
           if S3(i*j)==1
               %Block(:,:,:,i,j)=bitxor(Block(:,:,:,i,j),A);
               Block(:,:,:,i,j)=255-Block(:,:,:,i,j);
           end
        end
    end
    

    %% 加密程序4，置乱颜色分量
    for i=1:m/Bx
        for j=1:n/Bx
            switch S4(i*j)
                case 1      %RGB->RBG，颜色分量按RBG排布，第2个颜色分量与第3个颜色分量相互交换，使用异或操作进行数据变换，
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                case 2      %RGB->GRB,颜色分量按GRB排布，第1个颜色分量与第2个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    
                case 3      %RGB->BGR,颜色分量按BGR排布，第1个颜色分量与第3个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    
                case 4      %RGB->BGR->BRG,颜色分量按BRG排布，分两步进行，首先第1个颜色分量与第3个颜色分量相互交换，再第2个颜色分量与第3个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                case 5      %RGB->GRB->GBR,颜色分量按GBR排布，分两步进行，首先第1个颜色分量与第2个颜色分量相互交换，再第2个颜色分量与第3个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                otherwise
                    Block(:,:,:,i,j)=Block(:,:,:,i,j);      %不做处理
            end
        end
    end
    
    %% 密文
    for i=1:m/Bx
        for j=1:n/Bx
            C((i-1)*Bx+1:i*Bx,(j-1)*Bx+1:j*Bx,:)=Block(:,:,:,i,j);
        end
    end
    
end

%密文以无损压缩方式保存，则解密不会不出现任何问题，但是如果密文使用.jpg格式存储，即：
%imwrite(uint8(C1),'房屋密文.jpg');
%则会出现问题，后面会说