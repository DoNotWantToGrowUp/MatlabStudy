% 基于块加扰的图像解密函数
% 输入密文C，测试密钥K
% 输出明文D
% 2022/05/11 by linyiting
% LV 3.0 BlockScram Transformation
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
function D = BlockScram_DeAlgorithm(C,K,fenkuai)
    
    Bx=fenkuai;
    
    %% 使用密钥产生伪随机序列
    [m,n,~] = size(C);              %计算图像P的尺寸
    
    S_Len=m*n/(Bx*Bx);              %序列长度，这里各个伪随机序列的长度一样，故统一用S_Len表示序列长度
    
    rng(K(1));                      %控制随机数生成器，随机数生成器randi()的种子设置为0
    S1=randi([1,m*n/(Bx*Bx)],1,S_Len);  %返回一个由介于1和m*n/64之间的伪随机整数组成的1×S_Len数组

    rng(K(2));                      %控制随机数生成器，随机数生成器randi()的种子设置为1
    S2=randi([0,5],1,S_Len);       %返回一个由介于0和5之间的伪随机整数组成的1×S_Len数组

    rng(K(3));
    S3=randi([0,1],1,S_Len);

    rng(K(4));
    S4=randi([0,5],1,S_Len);
    
    %% 密文8×8分块 或 16×16分块
    
    Block=zeros(Bx,Bx,3,m/Bx,n/Bx);%5维数组，以8×8×3大小的数组作为5维数组的元素
    for i=1:m/Bx
        for j=1:n/Bx
            Block(:,:,:,i,j)=C((i-1)*Bx+1:i*Bx,(j-1)*Bx+1:j*Bx,:);
        end
    end
    
    
    %% 解密程序1，置乱颜色分量解密
    for i=1:m/Bx
        for j=1:n/Bx
            switch S4(i*j)
                case 1      %RBG->RGB，第2个颜色分量与第3个颜色分量相互交换，使用异或操作进行数据变换，
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                case 2      %GRB->RGB,第1个颜色分量与第2个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    
                case 3      %BGR->RGB,第1个颜色分量与第3个颜色分量相互交换
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    
                case 4      %BRG->BGR->RGB,分两步进行，首先第2个颜色分量与第3个颜色分量相互交换，再第1个颜色分量与第3个颜色分量相互交换
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,3,i,j));
                    
                case 5      %GBR->GRB->RGB,分两步进行，首先第2个颜色分量与第3个颜色分量相互交换，再第1个颜色分量与第2个颜色分量相互交换
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,3,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,2,i,j),Block(:,:,3,i,j));
                    
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,2,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    Block(:,:,1,i,j)=bitxor(Block(:,:,1,i,j),Block(:,:,2,i,j));
                    
                otherwise
                    Block(:,:,:,i,j)=Block(:,:,:,i,j);      %不做处理
            end
        end
    end
    
    
    %% 解密程序2，负-正变换的解密
    for i=1:m/Bx
        for j=1:n/Bx
           if S3(i*j)==1
               %Block(:,:,:,i,j)=bitxor(Block(:,:,:,i,j),A);
               Block(:,:,:,i,j)=255-Block(:,:,:,i,j);
           end
        end
    end
    
    
    %% 解密程序3，块旋转和翻转的解密
    for i=1:m/Bx
        for j=1:n/Bx
            % 若S2(i*j)==0，则不旋转块，即不做操作
            switch S2(i*j)
                case 1              %逆时针旋转90°
                    for k=1:Bx/2       
                        for l=1:Bx/2
                            t=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(l,Bx+1-k,:,i,j);
                            Block(l,Bx+1-k,:,i,j)=Block(Bx+1-k,Bx+1-l,:,i,j);
                            Block(Bx+1-k,Bx+1-l,:,i,j)=Block(Bx+1-l,k,:,i,j);
                            Block(Bx+1-l,k,:,i,j)=t;
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
                    
                case 3              %顺时针旋转90°
                    for k=1:Bx/2
                        for l=1:Bx/2
                            t=Block(k,l,:,i,j);
                            Block(k,l,:,i,j)=Block(Bx+1-l,k,:,i,j);
                            Block(Bx+1-l,k,:,i,j)=Block(Bx+1-k,Bx+1-l,:,i,j);
                            Block(Bx+1-k,Bx+1-l,:,i,j)=Block(l,Bx+1-k,:,i,j);
                            Block(l,Bx+1-k,:,i,j)=t;
                        end
                    end
                    
                case 4
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
                    
                case 5
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

    
    %% 解密程序4，块置乱解密
    for i=m*n/(Bx*Bx):-1:1  
        t=Block(:,:,:,S1(i));
        Block(:,:,:,S1(i))=Block(:,:,:,i);
        Block(:,:,:,i)=t;
    end
    
    D=zeros(m,n,3);
    for i=1:m/Bx
        for j=1:n/Bx
            D((i-1)*Bx+1:i*Bx,(j-1)*Bx+1:j*Bx,:)=Block(:,:,:,i,j);
        end
    end
    
end